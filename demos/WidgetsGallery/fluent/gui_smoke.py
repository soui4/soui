#!/usr/bin/env python3
"""PID-scoped Windows QA for SOUI demos; no desktop capture or global input.

Examples (coordinates are target logical client pixels, under its DPI context):
  python demos/fluent/gui_smoke.py --demo flowlayout_demo --toggle 543 30
  python demos/fluent/gui_smoke.py --demo demo --phase edited --toggle 864 18 \
      --action-file output/remaining-fluent/actions.json
  python demos/fluent/gui_smoke.py --demo wxDemo --single-theme

An action file is a JSON array, executed AFTER --click and BEFORE the light
baseline, in order. Example:
  [{"type":"click","x":100,"y":150},
   {"type":"text","text":"Fluent QA\\n"},
   {"type":"key","key":"HOME"},
   {"type":"drag","from":[100,150],"to":[200,150]},
   {"type":"wheel","x":300,"y":200,"delta":-240},
   {"type":"wait","seconds":1},
   {"type":"capture","label":"edited"}]

Keys are unmodified virtual keys (e.g. TAB, DOWN, ENTER, PAGEUP). Global
SendInput/clipboard/keyboard-state changes and modifier chords are deliberately
not used. Drag selects text without those global operations. Key/text actions
use this process's focused HWND, falling back to its main HWND. Other actions
use the main HWND; optional target="focus" or target="popup" remains PID-scoped.
A popup target must be unambiguous among this process's visible top-level HWNDs.
Click alone accepts "async": true (JSON boolean; default false) to queue its
owned-HWND mouse messages without waiting for a modal handler to return. Both
buttons and count=2 (double-click) retain their message order. Use a wait before
popup capture/key actions: queued input does not guarantee popup readiness.
Before an async non-popup click, the visible/enabled owned root host must be the
confirmed foreground HWND. SetForegroundWindow is requested at most once; a
refusal fails closed (no input-queue attachment, global input, or click retries).
Explicit popup targets are not reactivated, so an open native menu is not raised
behind its host. Async clicks emit pid-scoped-input JSON before/after diagnostics
(foreground PID, owned GUI-thread focus/capture/menu state, and owned popups).

--protect-roi X Y WIDTH HEIGHT uses full-window SCREENSHOT pixels, not client
coordinates. Protected regions are checked only when explicitly requested;
normal theme return is a relative visual comparison, NOT bit-exact equality.
All output goes to a unique run directory inside output/remaining-fluent.
This is a visual smoke check, not proof of control/model state preservation.
"""

from __future__ import annotations

import argparse
import ctypes
from ctypes import wintypes
from datetime import datetime, timezone
import hashlib
import json
import math
from pathlib import Path
import queue
import re
import subprocess
import sys
import threading
import time
import uuid
from dataclasses import asdict, dataclass
from typing import Any

from PIL import Image, ImageChops, ImageStat

REPO = Path(__file__).resolve().parents[2]
OUTPUT = REPO / "output" / "remaining-fluent"
DEMOS = (
    "demo", "demo2", "wxDemo", "flowlayout_demo", "framelayout_demo",
    "gridctrl_demo", "hellocharts_demo", "lightbutton_demo", "mclistview_demo",
    "propanimator_demo", "sci_demo", "wallpaper", "ipc_client", "ipc_server",
    "SoTool", "uieditor", "fun_test",
)
PROMPTS = {"select a render", "msgbox"}
PHASES = ("light", "dark", "light_return")
SAFE_LABEL = re.compile(r"[A-Za-z0-9][A-Za-z0-9_.-]{0,63}\Z")
KEYS = {
    "BACKSPACE": 0x08, "TAB": 0x09, "ENTER": 0x0D, "RETURN": 0x0D,
    "ESCAPE": 0x1B, "ESC": 0x1B, "SPACE": 0x20, "PAGEUP": 0x21,
    "PAGEDOWN": 0x22, "END": 0x23, "HOME": 0x24, "LEFT": 0x25,
    "UP": 0x26, "RIGHT": 0x27, "DOWN": 0x28, "INSERT": 0x2D,
    "DELETE": 0x2E,
}
KEYS.update({str(i): ord(str(i)) for i in range(10)})
KEYS.update({chr(i): i for i in range(ord("A"), ord("Z") + 1)})
KEYS.update({f"F{i}": 0x6F + i for i in range(1, 13)})
EXTENDED_KEYS = set(range(0x21, 0x29)) | {0x2D, 0x2E}


class SmokeError(RuntimeError):
    """Fail closed without falling back to desktop capture or global input."""


class OwnershipError(SmokeError):
    """An HWND is destroyed, stale, or not owned by the launched process."""


@dataclass(frozen=True)
class Thresholds:
    pixel_delta: int = 12
    min_changed_fraction: float = 0.12
    min_mean_delta: float = 8.0
    min_changed_tiles: float = 0.30
    min_nonblack_fraction: float = 0.02
    return_ratio: float = 0.75
    return_slack: float = 4.0
    roi_pixel_delta: int = 0
    roi_changed_fraction: float = 0.0


def safe_label(value: str) -> str:
    if not isinstance(value, str) or not SAFE_LABEL.fullmatch(value):
        raise ValueError("labels must be 1-64 ASCII letters/digits/._-, starting with a letter/digit")
    return value


def number(value: Any, name: str, low: float, high: float) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ValueError(f"{name} must be a number")
    value = float(value)
    if not math.isfinite(value) or not low <= value <= high:
        raise ValueError(f"{name} must be finite and between {low} and {high}")
    return value


def integer(value: Any, name: str, low: int, high: int) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or not low <= value <= high:
        raise ValueError(f"{name} must be an integer between {low} and {high}")
    return value


def point(value: Any, name: str = "point") -> tuple[int, int]:
    if not isinstance(value, (list, tuple)) or len(value) != 2:
        raise ValueError(f"{name} must be [x, y]")
    return tuple(integer(v, name, 0, 32767) for v in value)


def pack_point(x: int, y: int) -> int:
    # Mouse message coordinates are signed 16-bit, including wheel screen coords.
    integer(x, "x", -32768, 32767)
    integer(y, "y", -32768, 32767)
    return (x & 0xFFFF) | ((y & 0xFFFF) << 16)


def text_units(text: str) -> list[int]:
    data = text.replace("\r\n", "\n").replace("\n", "\r").encode("utf-16-le")
    return [int.from_bytes(data[i:i + 2], "little") for i in range(0, len(data), 2)]


def validate_actions(raw: Any) -> list[dict[str, Any]]:
    if not isinstance(raw, list):
        raise ValueError("action-file must contain a JSON array")
    allowed = {
        "click": {"x", "y", "button", "count", "async"}, "key": {"key", "count"},
        "text": {"text"}, "wait": {"seconds"}, "capture": {"label"},
        "wheel": {"x", "y", "delta"}, "drag": {"from", "to", "steps"},
    }
    actions = []
    for index, item in enumerate(raw):
        if not isinstance(item, dict) or not isinstance(item.get("type"), str) or item["type"] not in allowed:
            raise ValueError(f"action {index}: unknown or missing type")
        action = dict(item)
        kind = action["type"]
        if set(action) - allowed[kind] - {"type", "target"}:
            raise ValueError(f"action {index}: unexpected fields")
        if action.get("target", "main") not in ("main", "focus", "popup"):
            raise ValueError(f"action {index}: target must be main, focus, or popup")
        try:
            if kind in ("click", "wheel"):
                point([action["x"], action["y"]])
            if kind == "click":
                if not isinstance(action.get("async", False), bool):
                    raise ValueError("async must be a boolean")
                if action.get("button", "left") not in ("left", "right"):
                    raise ValueError("button must be left or right")
                integer(action.get("count", 1), "count", 1, 2)
            elif kind == "wheel":
                integer(action["delta"], "delta", -32768, 32767)
            elif kind == "key":
                if not isinstance(action["key"], str) or action["key"].upper() not in KEYS:
                    raise ValueError("key must be an unmodified named virtual key; use text for characters")
                integer(action.get("count", 1), "count", 1, 1000)
            elif kind == "text":
                if not isinstance(action["text"], str) or len(action["text"]) > 100000:
                    raise ValueError("text must be a string of at most 100000 characters")
                text_units(action["text"])
            elif kind == "wait":
                number(action["seconds"], "seconds", 0, 300)
            elif kind == "capture":
                safe_label(action["label"])
            elif kind == "drag":
                point(action["from"], "from")
                point(action["to"], "to")
                integer(action.get("steps", 12), "steps", 1, 1000)
        except (KeyError, ValueError, UnicodeError) as exc:
            raise ValueError(f"action {index}: {exc}") from exc
        actions.append(action)
    return actions


def rect_size(rect: list[int] | tuple[int, ...]) -> tuple[int, int]:
    return rect[2] - rect[0], rect[3] - rect[1]


def require_owner(expected_pid: int, actual_pid: int, alive: bool = True) -> None:
    if not alive or expected_pid != actual_pid:
        raise OwnershipError(f"refusing non-owned/stale HWND: expected PID {expected_pid}, got {actual_pid}")


def choose_main(windows: list[dict[str, Any]], pid: int,
                minimum: tuple[int, int], demo: str) -> dict[str, Any] | None:
    candidates = [w for w in windows if w["pid"] == pid and w["visible"]
                  and not w["minimized"]
                  and not (demo == "demo" and w["title"].casefold() in PROMPTS)
                  and rect_size(w["rect"])[0] >= minimum[0]
                  and rect_size(w["rect"])[1] >= minimum[1]]
    # Intentionally no nonempty-title test: wxDemo's real main window has no title.
    return max(candidates, key=lambda w: math.prod(rect_size(w["rect"])), default=None)


def geometry_checks(baseline: dict[str, Any], current: dict[str, Any]) -> dict[str, bool]:
    return {
        "same_pid": baseline["pid"] == current["pid"],
        "same_hwnd": baseline["hwnd"] == current["hwnd"],
        "same_size": rect_size(baseline["rect"]) == rect_size(current["rect"]),
        "same_client_size": rect_size(baseline["client_rect"]) == rect_size(current["client_rect"]),
        "not_minimized": not current["minimized"],
        "visible": current["visible"],
        "same_maximized_state": baseline["maximized"] == current["maximized"],
    }


def max_channel(image: Image.Image) -> Image.Image:
    r, g, b = image.convert("RGB").split()
    return ImageChops.lighter(ImageChops.lighter(r, g), b)


def fraction_above(image: Image.Image, threshold: int) -> float:
    histogram = image.histogram()
    return sum(histogram[threshold + 1:]) / math.prod(image.size)


def image_stats(image: Image.Image, limits: Thresholds) -> dict[str, Any]:
    fraction = fraction_above(max_channel(image), 8)
    return {"nonblack_fraction": fraction, "mean_rgb": ImageStat.Stat(image.convert("RGB")).mean,
            "nonblack": fraction >= limits.min_nonblack_fraction}


def compare_images(a: Image.Image, b: Image.Image, pixel_delta: int = 12) -> dict[str, Any]:
    if a.size != b.size:
        return {"same_size": False, "changed_fraction": 0.0, "mean_delta": 0.0,
                "changed_tiles_fraction": 0.0}
    diff = ImageChops.difference(a.convert("RGB"), b.convert("RGB"))
    strength = max_channel(diff)
    mask = strength.point(lambda value: 255 if value > pixel_delta else 0)
    # Spatial coverage rejects a single small hover/button/caret change.
    grid = mask.resize((8, 6), Image.Resampling.BOX)
    return {"same_size": True, "changed_fraction": fraction_above(strength, pixel_delta),
            "mean_delta": sum(ImageStat.Stat(diff).mean) / 3,
            "changed_tiles_fraction": sum(v >= 26 for v in grid.tobytes()) / 48}


def large_change(metrics: dict[str, Any], limits: Thresholds) -> bool:
    return (metrics["same_size"] and metrics["changed_fraction"] >= limits.min_changed_fraction
            and metrics["mean_delta"] >= limits.min_mean_delta
            and metrics["changed_tiles_fraction"] >= limits.min_changed_tiles)


def evaluate(captures: list[dict[str, Any]], images: dict[str, Image.Image],
             limits: Thresholds, rois: list[list[int]], single_theme: bool = False) -> dict[str, Any]:
    checks: list[dict[str, Any]] = []
    comparisons: dict[str, Any] = {}

    def check(name: str, ok: bool, **detail: Any) -> None:
        checks.append({"name": name, "ok": bool(ok), **detail})

    required = PHASES[:1] if single_theme else PHASES
    frames = {frame["label"]: frame for frame in captures if frame.get("theme_baseline")}
    check("required_theme_captures", all(label in frames and label in images for label in required),
          expected=list(required), actual=list(frames))
    if not checks[-1]["ok"]:
        return {"ok": False, "checks": checks, "comparisons": comparisons, "protected_rois": []}
    first = frames["light"]
    for label in required:
        frame = frames[label]
        for name, ok in geometry_checks(first, frame).items():
            check(f"{label}.{name}", ok)
        check(f"{label}.image_matches_rect", images[label].size == rect_size(frame["rect"]))
        check(f"{label}.nonblack", image_stats(images[label], limits)["nonblack"])
        # Position is evidence, not a failure: moving a same-size owned HWND is safe.
        frame["same_rect_as_light"] = frame["rect"] == first["rect"]
        frame["same_size_as_light"] = rect_size(frame["rect"]) == rect_size(first["rect"])
    if not single_theme:
        for left, right in (("light", "dark"), ("dark", "light_return"), ("light", "light_return")):
            comparisons[f"{left}_to_{right}"] = compare_images(images[left], images[right], limits.pixel_delta)
        check("light_to_dark.large_change", large_change(comparisons["light_to_dark"], limits))
        check("dark_to_light_return.large_change", large_change(comparisons["dark_to_light_return"], limits))
        back = comparisons["light_to_light_return"]
        dark_distance = comparisons["dark_to_light_return"]["mean_delta"]
        # Caret, selection and modest dynamic media changes may differ after return.
        # Require return to resemble light substantially more than dark, not equality.
        check("return_resembles_light", back["same_size"]
              and back["mean_delta"] <= dark_distance * limits.return_ratio + limits.return_slack,
              allowed_mean_delta=dark_distance * limits.return_ratio + limits.return_slack,
              actual_mean_delta=back["mean_delta"], bitexact_required=False)
    protected = []
    for index, (x, y, width, height) in enumerate(rois):
        valid = (x >= 0 and y >= 0 and width > 0 and height > 0
                 and all(x + width <= images[label].width and y + height <= images[label].height
                         for label in required))
        check(f"roi_{index}.within_all_frames", valid, roi=[x, y, width, height])
        if not valid:
            continue
        box = (x, y, x + width, y + height)
        for label in required[1:]:
            metrics = compare_images(images["light"].crop(box), images[label].crop(box), limits.roi_pixel_delta)
            ok = metrics["changed_fraction"] <= limits.roi_changed_fraction
            protected.append({"roi": [x, y, width, height], "against": label, **metrics, "ok": ok})
            check(f"roi_{index}.unchanged_in_{label}", ok)
    return {"ok": all(item["ok"] for item in checks), "checks": checks,
            "comparisons": comparisons, "protected_rois": protected}


class GUIThreadInfo(ctypes.Structure):
    _fields_ = [("cbSize", wintypes.DWORD), ("flags", wintypes.DWORD)] + [
        (name, wintypes.HWND) for name in (
            "hwndActive", "hwndFocus", "hwndCapture", "hwndMenuOwner", "hwndMoveSize", "hwndCaret")
    ] + [("rcCaret", wintypes.RECT)]


class TargetDpi:
    """Adopt an owned HWND's coordinate space, restoring the calling thread."""

    def __init__(self, user: Any):
        self.get_context = getattr(user, "GetWindowDpiAwarenessContext", None)
        self.set_context = getattr(user, "SetThreadDpiAwarenessContext", None)
        self.get_awareness = getattr(user, "GetAwarenessFromDpiAwarenessContext", None)
        self.get_dpi = getattr(user, "GetDpiForWindow", None)
        if self.get_context:
            self.get_context.argtypes, self.get_context.restype = [wintypes.HWND], wintypes.HANDLE
        if self.set_context:
            self.set_context.argtypes, self.set_context.restype = [wintypes.HANDLE], wintypes.HANDLE
        if self.get_awareness:
            self.get_awareness.argtypes, self.get_awareness.restype = [wintypes.HANDLE], ctypes.c_int
        if self.get_dpi:
            self.get_dpi.argtypes, self.get_dpi.restype = [wintypes.HWND], wintypes.UINT
        self.missing = [name for name, function in (
            ("GetWindowDpiAwarenessContext", self.get_context),
            ("SetThreadDpiAwarenessContext", self.set_context)) if function is None]

    @property
    def support(self) -> dict[str, Any]:
        return {"mode": "target-window-context" if not self.missing else "process-context-fallback",
                "missing_apis": self.missing,
                "warning": None if not self.missing else
                "Target DPI context APIs unavailable; coordinate/image consistency needs manual verification. No cropping."}

    def scope(self, hwnd: int):
        from contextlib import contextmanager

        @contextmanager
        def adopt():
            record = {**self.support, "window_context": None, "awareness": None, "dpi": None,
                      "restored": None}
            if self.missing:
                yield record
                return
            context = self.get_context(hwnd)
            if not context:
                raise SmokeError("GetWindowDpiAwarenessContext failed for owned HWND")
            record["window_context"] = hex(context)
            old = self.set_context(context)
            if not old:
                raise SmokeError("SetThreadDpiAwarenessContext failed; refusing mixed-DPI coordinates")
            try:
                record["previous_thread_context"] = hex(old)
                if self.get_awareness:
                    awareness = self.get_awareness(context)
                    record["awareness"] = {-1: "invalid", 0: "unaware", 1: "system", 2: "per-monitor"}.get(awareness, str(awareness))
                if self.get_dpi:
                    record["dpi"] = self.get_dpi(hwnd)
                yield record
            finally:
                record["restored"] = bool(self.set_context(old))
                if not record["restored"]:
                    raise SmokeError("failed to restore thread DPI awareness context")

        return adopt()


class WindowsSession:
    """All Win32 handles are revalidated against the retained Popen process."""

    def __init__(self, process: subprocess.Popen, demo: str, args: argparse.Namespace):
        import win32con
        import win32gui
        import win32process

        self.process, self.demo, self.args = process, demo, args
        self.con, self.gui, self.proc = win32con, win32gui, win32process
        self.hwnd: int | None = None
        self.baseline: dict[str, Any] | None = None
        self.dialogs: list[dict[str, Any]] = []
        self.geometry_failures: list[dict[str, Any]] = []
        self.input_events: list[dict[str, Any]] = []
        self.user = ctypes.WinDLL("user32", use_last_error=True)
        self.gdi = ctypes.WinDLL("gdi32", use_last_error=True)
        self.dpi = TargetDpi(self.user)
        # Explicit pointer-sized signatures are essential on 64-bit Windows.
        self.user.PrintWindow.argtypes = [wintypes.HWND, wintypes.HDC, wintypes.UINT]
        self.user.PrintWindow.restype = wintypes.BOOL
        self.user.IsZoomed.argtypes = [wintypes.HWND]
        self.user.IsZoomed.restype = wintypes.BOOL
        self.user.SendMessageTimeoutW.argtypes = [wintypes.HWND, wintypes.UINT, ctypes.c_size_t,
                                                 ctypes.c_ssize_t, wintypes.UINT, wintypes.UINT,
                                                 ctypes.POINTER(ctypes.c_size_t)]
        self.user.SendMessageTimeoutW.restype = ctypes.c_ssize_t
        self.user.GetGUIThreadInfo.argtypes = [wintypes.DWORD, ctypes.c_void_p]
        self.user.GetGUIThreadInfo.restype = wintypes.BOOL
        self.user.GetForegroundWindow.argtypes = []
        self.user.GetForegroundWindow.restype = wintypes.HWND
        self.user.SetForegroundWindow.argtypes = [wintypes.HWND]
        self.user.SetForegroundWindow.restype = wintypes.BOOL
        self.user.GetAncestor.argtypes = [wintypes.HWND, wintypes.UINT]
        self.user.GetAncestor.restype = wintypes.HWND
        self.user.MapVirtualKeyW.argtypes = [wintypes.UINT, wintypes.UINT]
        self.user.MapVirtualKeyW.restype = wintypes.UINT
        self.gdi.CreateCompatibleDC.argtypes = [wintypes.HDC]
        self.gdi.CreateCompatibleDC.restype = wintypes.HDC
        self.gdi.CreateDIBSection.argtypes = [wintypes.HDC, ctypes.c_void_p, wintypes.UINT,
                                            ctypes.POINTER(ctypes.c_void_p), wintypes.HANDLE, wintypes.DWORD]
        self.gdi.CreateDIBSection.restype = wintypes.HBITMAP
        self.gdi.SelectObject.argtypes = [wintypes.HDC, wintypes.HANDLE]
        self.gdi.SelectObject.restype = wintypes.HANDLE
        self.gdi.DeleteObject.argtypes = [wintypes.HANDLE]
        self.gdi.DeleteDC.argtypes = [wintypes.HDC]
        self.gdi.DeleteObject.restype = self.gdi.DeleteDC.restype = wintypes.BOOL

    def owner(self, hwnd: int) -> None:
        if not self.gui.IsWindow(hwnd):
            raise OwnershipError("owned HWND was destroyed")
        require_owner(self.process.pid, self.proc.GetWindowThreadProcessId(hwnd)[1],
                      self.process.poll() is None)

    def snapshot(self, hwnd: int) -> dict[str, Any]:
        self.owner(hwnd)
        with self.dpi.scope(hwnd) as dpi:
            result = {"hwnd": int(hwnd), "pid": self.process.pid, "title": self.gui.GetWindowText(hwnd),
                      "rect": list(self.gui.GetWindowRect(hwnd)), "client_rect": list(self.gui.GetClientRect(hwnd)),
                      "visible": bool(self.gui.IsWindowVisible(hwnd)),
                      "minimized": bool(self.gui.IsIconic(hwnd)), "maximized": bool(self.user.IsZoomed(hwnd)),
                      "dpi_context": dpi}
        return result

    def windows(self) -> list[dict[str, Any]]:
        if self.process.poll() is not None:
            return []
        windows = []

        def visit(hwnd: int, _: Any) -> None:
            # Do not query other processes' titles/rects, let alone send/capture them.
            try:
                if self.proc.GetWindowThreadProcessId(hwnd)[1] == self.process.pid:
                    windows.append(self.snapshot(hwnd))
            except (self.gui.error, OwnershipError):
                pass  # A window can disappear during EnumWindows.

        self.gui.EnumWindows(visit, None)
        return windows

    def send(self, hwnd: int, message: int, wparam: int = 0, lparam: int = 0) -> None:
        self.owner(hwnd)
        with self.dpi.scope(hwnd):
            result = ctypes.c_size_t()
            ctypes.set_last_error(0)
            ok = self.user.SendMessageTimeoutW(hwnd, message, wparam, lparam,
                                              self.con.SMTO_ABORTIFHUNG | self.con.SMTO_BLOCK,
                                              2000, ctypes.byref(result))
            if not ok:
                raise SmokeError(f"owned HWND message {message:#x} timed out/failed (error {ctypes.get_last_error()})")

    def post(self, hwnd: int, message: int, wparam: int = 0, lparam: int = 0) -> None:
        self.owner(hwnd)
        with self.dpi.scope(hwnd):
            self.gui.PostMessage(hwnd, message, wparam, lparam)

    def handle_dialogs(self) -> None:
        if self.demo != "demo":
            return
        for window in self.windows():
            title = window["title"].casefold()
            if title in PROMPTS:
                hwnd = window["hwnd"]
                if title == "select a render":
                    self.post(hwnd, self.con.WM_COMMAND, self.con.IDYES)
                else:
                    self.post(hwnd, self.con.WM_CLOSE)
                if not any(d["hwnd"] == hwnd and d["title"] == title for d in self.dialogs):
                    self.dialogs.append({"hwnd": hwnd, "pid": self.process.pid, "title": title})

    def wait(self, seconds: float) -> None:
        deadline = time.monotonic() + seconds
        while True:
            if self.process.poll() is not None:
                raise SmokeError(f"{self.demo} exited early ({self.process.returncode})")
            self.handle_dialogs()
            if self.hwnd and self.baseline:
                current = self.snapshot(self.hwnd)
                bad = [name for name, ok in geometry_checks(self.baseline, current).items() if not ok]
                if bad:
                    self.geometry_failures.append({"baseline": self.baseline, "current": current,
                                                   "checks": geometry_checks(self.baseline, current)})
                    raise SmokeError(f"main-window geometry/state changed: {', '.join(bad)}; rect={current['rect']}")
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                return
            time.sleep(min(0.05, remaining))

    def find_main(self) -> None:
        deadline = time.monotonic() + self.args.timeout
        candidate = None
        since = time.monotonic()
        while time.monotonic() < deadline:
            self.wait(0)
            window = choose_main(self.windows(), self.process.pid, tuple(self.args.min_size), self.demo)
            key = (window["hwnd"], tuple(window["rect"])) if window else None
            if key != candidate:
                candidate, since = key, time.monotonic()
            if window and time.monotonic() - since >= self.args.startup_dwell:
                self.hwnd, self.baseline = window["hwnd"], window
                return
            time.sleep(0.1)
        raise SmokeError("no stable visible owned main window before --timeout (blank titles are accepted)")

    def _window_identity(self, hwnd: int | None) -> dict[str, Any]:
        # Foreign foreground/thread HWNDs are identifiers only: no title, rect,
        # capture, activation or input is requested for them.
        result = {"hwnd": int(hwnd or 0), "pid": None, "thread_id": None, "owned": False}
        if hwnd:
            try:
                tid, pid = self.proc.GetWindowThreadProcessId(hwnd)
                result.update(pid=pid, thread_id=tid, owned=pid == self.process.pid and tid != 0)
            except self.gui.error as exc:
                result["error"] = str(exc)
        return result

    def _thread_info(self, hwnd: int) -> dict[str, Any]:
        self.owner(hwnd)
        tid = self.proc.GetWindowThreadProcessId(hwnd)[0]
        info = GUIThreadInfo()
        info.cbSize = ctypes.sizeof(info)
        ctypes.set_last_error(0)
        if not self.user.GetGUIThreadInfo(tid, ctypes.byref(info)):
            return {"ok": False, "thread_id": tid, "error": ctypes.get_last_error()}
        return {"ok": True, "thread_id": tid, "flags": int(info.flags),
                "menu_mode": bool(info.flags & 0x1C),  # INMENU/SYSTEMMENU/POPUPMENU
                **{name: self._window_identity(getattr(info, name)) for name in (
                    "hwndActive", "hwndFocus", "hwndCapture", "hwndMenuOwner", "hwndMoveSize", "hwndCaret")}}

    def _host(self, hwnd: int) -> int:
        self.owner(hwnd)
        host = int(self.user.GetAncestor(hwnd, 2) or 0)  # GA_ROOT, not another owned window.
        self.owner(host)
        return host

    def input_diagnostics(self, hwnd: int | None = None) -> dict[str, Any]:
        hwnd = self.hwnd if hwnd is None else hwnd
        host = self._host(hwnd)
        popups = []
        for window in self.windows():
            if window["hwnd"] in (self.hwnd, host):
                continue
            try:
                self.owner(window["hwnd"])
                popups.append({"hwnd": window["hwnd"], "pid": self.process.pid,
                               "class": self.gui.GetClassName(window["hwnd"]),
                               "visible": window["visible"], "rect": window["rect"]})
            except (self.gui.error, OwnershipError):
                pass  # Diagnostic enumeration can race a disappearing menu/shadow.
        return {"pid": self.process.pid, "target_hwnd": int(hwnd), "host_hwnd": host,
                "foreground": self._window_identity(self.user.GetForegroundWindow()),
                "thread": self._thread_info(host), "owned_popups": popups}

    def _require_foreground(self, host: int, *, allow_owned_window: bool = False) -> None:
        self.owner(host)
        foreground = self._window_identity(self.user.GetForegroundWindow())
        if not foreground["owned"] or (foreground["hwnd"] != host and not allow_owned_window):
            raise SmokeError(f"owned host foreground lost before input: host={host}, foreground={foreground}")
        self.owner(foreground["hwnd"])

    def activate_host(self, hwnd: int, timeout: float = 0.75) -> dict[str, Any]:
        """One owned-host request, bounded observation; never bypass Windows policy."""
        timeout = number(timeout, "activation timeout", 0, 5)
        host = self._host(hwnd)
        if (not self.gui.IsWindowVisible(host) or self.gui.IsIconic(host)
                or not self.gui.IsWindowEnabled(host)):
            raise SmokeError(f"owned host is not interactive (hidden/minimized/disabled): {host}")
        thread = self._thread_info(host)
        if not thread["ok"]:
            raise SmokeError(f"owned host GUI thread state unavailable: {thread}")
        before = self._window_identity(self.user.GetForegroundWindow())
        result = {"host_hwnd": host, "pid": self.process.pid, "attempted": False,
                  "api_return": None, "before_foreground": before}
        if before["hwnd"] == host and before["owned"]:
            self._require_foreground(host)
            return result
        if thread["menu_mode"]:
            raise SmokeError(f"owned menu already active; refusing host reactivation: {thread}")
        self.owner(hwnd)
        self.owner(host)
        result["attempted"] = True
        result["api_return"] = bool(self.user.SetForegroundWindow(host))
        deadline = time.monotonic() + timeout
        while True:
            self.owner(hwnd)
            self.owner(host)
            foreground = self._window_identity(self.user.GetForegroundWindow())
            if foreground["hwnd"] == host and foreground["owned"]:
                return result  # Observed HWND/PID, not the BOOL return, proves activation.
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise SmokeError(f"owned host foreground activation not confirmed: {result}; foreground={foreground}")
            time.sleep(min(0.05, remaining))

    def focus(self) -> int:
        self.owner(self.hwnd)
        tid = self.proc.GetWindowThreadProcessId(self.hwnd)[0]
        info = GUIThreadInfo()
        info.cbSize = ctypes.sizeof(info)
        if self.user.GetGUIThreadInfo(tid, ctypes.byref(info)) and info.hwndFocus:
            self.owner(info.hwndFocus)  # Never use another process's foreground HWND.
            return int(info.hwndFocus)
        return self.hwnd

    def target(self, action: dict[str, Any]) -> int:
        default = "focus" if action["type"] in ("key", "text") else "main"
        target = action.get("target", default)
        if target == "focus":
            return self.focus()
        if target == "popup":
            popups = [w for w in self.windows() if w["visible"] and w["hwnd"] != self.hwnd
                      and not w["minimized"] and rect_size(w["rect"])[0] > 1
                      and rect_size(w["rect"])[1] > 1]
            if len(popups) != 1:
                raise SmokeError(f"popup target is ambiguous/missing ({len(popups)} owned candidates)")
            return popups[0]["hwnd"]
        self.owner(self.hwnd)
        return self.hwnd

    def position(self, hwnd: int, xy: tuple[int, int] | list[int]) -> int:
        self.owner(hwnd)
        with self.dpi.scope(hwnd):
            width, height = rect_size(self.gui.GetClientRect(hwnd))
        x, y = point(xy)
        if not (x < width and y < height):
            raise SmokeError(f"point {(x, y)} outside owned target client {(width, height)}")
        return pack_point(x, y)

    def act(self, action: dict[str, Any]) -> int | None:
        kind = action["type"]
        if kind == "wait":
            self.wait(action["seconds"])
            return None
        hwnd = self.target(action)
        with self.dpi.scope(hwnd):
            if kind != "click" or not action.get("async", False):
                self._input(hwnd, action)
            else:
                # Validate coordinates before even requesting foreground activation.
                self.position(hwnd, [action["x"], action["y"]])
                record = {"pid": self.process.pid, "target_hwnd": int(hwnd),
                          "action": dict(action), "transport": "PostMessage", "activation": None}
                self.input_events.append(record)
                try:
                    record["before"] = self.input_diagnostics(hwnd)
                    host = None
                    if action.get("target") != "popup":
                        record["activation"] = self.activate_host(hwnd)
                        host = record["activation"]["host_hwnd"]
                    self._input(hwnd, action, foreground_host=host)
                    self.wait(self.args.dwell)
                except Exception as exc:
                    record["error"] = f"{type(exc).__name__}: {exc}"
                    raise
                finally:
                    try:
                        record["after"] = self.input_diagnostics(hwnd)
                    except Exception as exc:
                        record["diagnostic_error"] = f"{type(exc).__name__}: {exc}"
                    print("pid-scoped-input " + json.dumps(record, ensure_ascii=False), flush=True)
                return int(hwnd)
        self.wait(self.args.dwell)
        return int(hwnd)

    def _input(self, hwnd: int, action: dict[str, Any], *, foreground_host: int | None = None) -> None:
        kind = action["type"]
        if kind == "click":
            pos = self.position(hwnd, [action["x"], action["y"]])
            right = action.get("button", "left") == "right"
            down = self.con.WM_RBUTTONDOWN if right else self.con.WM_LBUTTONDOWN
            up = self.con.WM_RBUTTONUP if right else self.con.WM_LBUTTONUP
            double = self.con.WM_RBUTTONDBLCLK if right else self.con.WM_LBUTTONDBLCLK
            flag = self.con.MK_RBUTTON if right else self.con.MK_LBUTTON
            # Modal click handlers may not return until their popup closes.
            # Keep async messages queued through the same owner-checked interface.
            transport = self.post if action.get("async", False) else self.send
            started = False

            def dispatch(*message: int) -> None:
                nonlocal started
                if foreground_host is not None:
                    self.owner(hwnd)
                    # A click can open an owned modal dialog between down/up.
                    # Finish this same-HWND sequence without reactivating its host.
                    self._require_foreground(foreground_host, allow_owned_window=started)
                transport(*message)
                started = True

            dispatch(hwnd, self.con.WM_MOUSEMOVE, 0, pos)
            for i in range(action.get("count", 1)):
                dispatch(hwnd, double if i else down, flag, pos)
                dispatch(hwnd, up, 0, pos)
            dispatch(hwnd, self.con.WM_MOUSELEAVE)
        elif kind == "key":
            vk = KEYS[action["key"].upper()]
            scan = self.user.MapVirtualKeyW(vk, 0)
            flags = 1 | (scan << 16) | ((1 << 24) if vk in EXTENDED_KEYS else 0)
            for _ in range(action.get("count", 1)):
                # Queue input to this HWND so the app's own TranslateMessage handles characters/TAB.
                self.post(hwnd, self.con.WM_KEYDOWN, vk, flags)
                self.post(hwnd, self.con.WM_KEYUP, vk, flags | (1 << 30) | (1 << 31))
        elif kind == "text":
            for unit in text_units(action["text"]):
                self.send(hwnd, self.con.WM_CHAR, unit, 1)
        elif kind == "wheel":
            pos = self.position(hwnd, [action["x"], action["y"]])
            self.send(hwnd, self.con.WM_MOUSEMOVE, 0, pos)
            screen = self.gui.ClientToScreen(hwnd, (action["x"], action["y"]))
            self.send(hwnd, self.con.WM_MOUSEWHEEL, (action["delta"] & 0xFFFF) << 16,
                      pack_point(*screen))
            self.send(hwnd, self.con.WM_MOUSELEAVE)
        elif kind == "drag":
            start, end = point(action["from"]), point(action["to"])
            self.position(hwnd, end)
            self.send(hwnd, self.con.WM_MOUSEMOVE, 0, self.position(hwnd, start))
            self.send(hwnd, self.con.WM_LBUTTONDOWN, self.con.MK_LBUTTON, self.position(hwnd, start))
            try:
                steps = action.get("steps", 12)
                for i in range(1, steps + 1):
                    xy = [round(a + (b - a) * i / steps) for a, b in zip(start, end)]
                    self.send(hwnd, self.con.WM_MOUSEMOVE, self.con.MK_LBUTTON, self.position(hwnd, xy))
                    self.wait(self.args.dwell / steps)
            finally:
                self.send(hwnd, self.con.WM_LBUTTONUP, 0, self.position(hwnd, end))
                self.send(hwnd, self.con.WM_MOUSELEAVE)
        else:
            raise SmokeError(f"unsupported input action: {kind}")

    def _bitmap(self, hwnd: int, size: tuple[int, int]) -> tuple[Image.Image, dict[str, Any]]:
        self.owner(hwnd)
        # The worker is a different thread: it too must adopt the target context.
        with self.dpi.scope(hwnd) as dpi:
            image = self._render_bitmap(hwnd, size)
        return image, dpi

    def _render_bitmap(self, hwnd: int, size: tuple[int, int]) -> Image.Image:
        class BitmapInfo(ctypes.Structure):
            _fields_ = [("biSize", wintypes.DWORD), ("biWidth", wintypes.LONG), ("biHeight", wintypes.LONG),
                        ("biPlanes", wintypes.WORD), ("biBitCount", wintypes.WORD),
                        ("biCompression", wintypes.DWORD), ("biSizeImage", wintypes.DWORD),
                        ("biXPelsPerMeter", wintypes.LONG), ("biYPelsPerMeter", wintypes.LONG),
                        ("biClrUsed", wintypes.DWORD), ("biClrImportant", wintypes.DWORD),
                        ("unusedColor", wintypes.DWORD)]

        self.owner(hwnd)
        width, height = size
        if width <= 0 or height <= 0 or width * height > 40_000_000:
            raise SmokeError(f"invalid/oversized capture dimensions {size}")
        dc = self.gui.GetWindowDC(hwnd)
        memory = bitmap = old = None
        try:
            if not dc:
                raise SmokeError("GetWindowDC failed for owned HWND")
            memory = self.gdi.CreateCompatibleDC(dc)
            info = BitmapInfo()
            info.biSize, info.biWidth, info.biHeight = 40, width, -height
            info.biPlanes, info.biBitCount = 1, 32
            bits = ctypes.c_void_p()
            bitmap = self.gdi.CreateDIBSection(dc, ctypes.byref(info), 0, ctypes.byref(bits), None, 0)
            if not memory or not bitmap or not bits.value:
                raise SmokeError("offscreen DIB allocation failed")
            old = self.gdi.SelectObject(memory, bitmap)
            if not old or old == ctypes.c_void_p(-1).value:
                raise SmokeError("SelectObject failed for offscreen DIB")
            ctypes.memset(bits, 0, width * height * 4)
            self.owner(hwnd)
            # PW_RENDERFULLCONTENT, full owned window. NEVER capture the desktop.
            if not self.user.PrintWindow(hwnd, memory, 2):
                raise SmokeError("owned HWND PrintWindow failed; no desktop fallback")
            self.owner(hwnd)
            return Image.frombytes("RGB", size, ctypes.string_at(bits, width * height * 4), "raw", "BGRX")
        finally:
            if old and old != ctypes.c_void_p(-1).value:
                self.gdi.SelectObject(memory, old)
            if bitmap:
                self.gdi.DeleteObject(bitmap)
            if memory:
                self.gdi.DeleteDC(memory)
            if dc:
                self.gui.ReleaseDC(hwnd, dc)  # A window DC is released, NOT DeleteDC'd.

    def capture(self, hwnd: int | None = None) -> tuple[dict[str, Any], Image.Image]:
        hwnd = self.hwnd if hwnd is None else hwnd
        before = self.snapshot(hwnd)
        if before["minimized"]:
            raise SmokeError("refusing capture of minimized owned window")
        self.send(hwnd, self.con.WM_NULL)
        result: queue.Queue = queue.Queue(maxsize=1)

        def worker() -> None:
            try:
                result.put((self._bitmap(hwnd, rect_size(before["rect"])), None))
            except Exception as exc:
                result.put((None, exc))

        # PrintWindow can hang. A bounded daemon worker owns all its GDI resources;
        # timeout cleanup closes/terminates only our Popen process. We never free
        # resources from under the worker or fall back to grabbing the screen.
        thread = threading.Thread(target=worker, daemon=True, name="owned-PrintWindow")
        thread.start()
        try:
            payload, error = result.get(timeout=self.args.capture_timeout)
        except queue.Empty as exc:
            raise SmokeError("owned HWND PrintWindow exceeded --capture-timeout") from exc
        if error:
            raise error
        image, dpi = payload
        after = self.snapshot(hwnd)
        if before["rect"] != after["rect"] or not all(geometry_checks(before, after).values()):
            raise SmokeError("owned window geometry changed during PrintWindow")
        after.update({"capture_method": "PrintWindow", "printwindow_flags": 2,
                      "printwindow_ok": True, "capture_dpi_context": dpi})
        return after, image

    def close(self) -> dict[str, Any]:
        result: dict[str, Any] = {"pid": self.process.pid, "terminated": False, "killed": False, "errors": []}
        result["exited_before_cleanup"] = self.process.poll() is not None
        try:
            if self.process.poll() is None:
                for window in self.windows():
                    try:
                        self.post(window["hwnd"], self.con.WM_CLOSE)
                    except Exception as exc:
                        result["errors"].append(str(exc))
                deadline = time.monotonic() + self.args.close_timeout
                while self.process.poll() is None and time.monotonic() < deadline:
                    self.handle_dialogs()  # Only this PID, including main demo's exit msgbox.
                    time.sleep(0.05)
        except Exception as exc:
            result["errors"].append(str(exc))
        # Enumeration/DPI/WM_CLOSE failures must not leave our launched demo alive.
        return terminate_owned(self.process, self.args.close_timeout, result)


def terminate_owned(process: subprocess.Popen, timeout: float,
                    result: dict[str, Any] | None = None) -> dict[str, Any]:
    """Never terminate by name/PID lookup, recurse into a tree, or touch peers."""
    if result is None:
        result = {"pid": process.pid, "terminated": False, "killed": False, "errors": []}
    if process.poll() is None:
        result["terminated"] = True
        process.terminate()
        try:
            process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            result["killed"] = True
            process.kill()
            process.wait(timeout=timeout)
    result["exit_code"] = process.poll()
    return result


def enable_dpi_awareness() -> str:
    user = ctypes.WinDLL("user32", use_last_error=True)
    try:
        user.SetProcessDpiAwarenessContext.argtypes = [wintypes.HANDLE]
        user.SetProcessDpiAwarenessContext.restype = wintypes.BOOL
        if user.SetProcessDpiAwarenessContext(ctypes.c_void_p(-4)):
            return "per-monitor-v2"
    except AttributeError:
        pass
    user.SetProcessDPIAware.restype = wintypes.BOOL
    return "system-aware" if user.SetProcessDPIAware() else "already-configured"


def launch_command(executable: Path, demo: str) -> list[str]:
    # fun_test contains interactive and noninteractive diagnostics. Launch only
    # the intended SOUI demo, never the entire Google Test suite during GUI QA.
    command = [str(executable)]
    if demo == "fun_test":
        command.append("--gtest_filter=window.soui")
    return command


def parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--demo", required=True, choices=DEMOS)
    p.add_argument("--phase", default="before", type=safe_label, help="filename/run label, not a theme selector")
    p.add_argument("--toggle", nargs=2, type=int, metavar=("X", "Y"), help="theme button in target logical client pixels")
    p.add_argument("--click", nargs=2, type=int, action="append", default=[], metavar=("X", "Y"))
    p.add_argument("--dwell", type=float, default=0.6, help="settle time after input, seconds")
    p.add_argument("--action-file", type=Path, help="ordered JSON actions before theme baseline")
    p.add_argument("--single-theme", action="store_true", help="opt out of default light/dark/light_return smoke")
    p.add_argument("--bin-dir", type=Path, default=REPO / "build" / "bin" / "Debug")
    p.add_argument("--timeout", type=float, default=60.0, help="main window discovery timeout")
    p.add_argument("--startup-dwell", type=float, default=1.5, help="main HWND/rect must be stable this long")
    p.add_argument("--capture-timeout", type=float, default=10.0)
    p.add_argument("--close-timeout", type=float, default=5.0)
    p.add_argument("--min-size", type=int, nargs=2, default=[320, 200], metavar=("WIDTH", "HEIGHT"))
    p.add_argument("--min-change", type=float, default=0.12, help="minimum changed-pixel fraction per theme switch")
    p.add_argument("--protect-roi", type=int, nargs=4, action="append", default=[],
                   metavar=("X", "Y", "WIDTH", "HEIGHT"), help="optional invariant region in screenshot pixels")
    p.add_argument("--roi-tolerance", type=int, default=0, help="allowed per-channel protected ROI pixel delta")
    p.add_argument("--roi-max-change", type=float, default=0.0, help="allowed changed fraction in protected ROIs")
    return p


def options(argv: list[str] | None = None) -> tuple[argparse.Namespace, list[dict[str, Any]], Thresholds]:
    p = parser()
    args = p.parse_args(argv)
    try:
        if not args.single_theme and args.toggle is None:
            raise ValueError("default three-theme smoke needs --toggle X Y; use --single-theme for capture-only")
        if args.single_theme and args.toggle is not None:
            raise ValueError("--toggle and --single-theme are mutually exclusive")
        if args.toggle is not None:
            point(args.toggle, "toggle")
        for xy in args.click:
            point(xy, "click")
        for value in args.min_size:
            integer(value, "min-size", 1, 32767)
        for name in ("dwell", "startup_dwell"):
            number(getattr(args, name), name, 0, 60)
        for name in ("timeout", "capture_timeout", "close_timeout"):
            number(getattr(args, name), name, 0.1, 300)
        number(args.min_change, "min-change", 0.01, 1)
        integer(args.roi_tolerance, "roi-tolerance", 0, 254)
        number(args.roi_max_change, "roi-max-change", 0, 1)
        for x, y, width, height in args.protect_roi:
            point([x, y], "protect-roi origin")
            integer(width, "protect-roi width", 1, 32767)
            integer(height, "protect-roi height", 1, 32767)
        actions = [{"type": "click", "x": x, "y": y} for x, y in args.click]
        if args.action_file:
            actions.extend(validate_actions(json.loads(args.action_file.read_text(encoding="utf-8-sig"))))
        limits = Thresholds(min_changed_fraction=args.min_change, roi_pixel_delta=args.roi_tolerance,
                            roi_changed_fraction=args.roi_max_change)
    except (ValueError, OSError) as exc:
        p.error(str(exc))
    return args, actions, limits


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="milliseconds")


def perform(session: WindowsSession, args: argparse.Namespace, actions: list[dict[str, Any]],
            limits: Thresholds, out: Path, evidence: dict[str, Any]) -> dict[str, Any]:
    images: dict[str, Image.Image] = {}

    def capture(label: str, filename: str, baseline: bool, hwnd: int | None = None) -> None:
        metadata, image = session.capture(hwnd)
        destination = out / filename
        image.save(destination)
        metadata.update({"label": label, "image": str(destination), "theme_baseline": baseline,
                         "captured_at": utc_now(), "sha256": hashlib.sha256(destination.read_bytes()).hexdigest(),
                         "statistics": image_stats(image, limits)})
        evidence["captures"].append(metadata)
        if baseline:
            images[label] = image
        if not metadata["statistics"]["nonblack"]:
            evidence["checks"].append({"name": f"{label}.nonblack", "ok": False})
            raise SmokeError(f"{label}: PrintWindow produced a black/nearly-black image")
        print(json.dumps({"demo": args.demo, "phase": args.phase, **metadata}, ensure_ascii=False), flush=True)

    session.find_main()
    evidence["initial_window"] = session.baseline
    evidence["target_dpi_support"] = session.dpi.support
    for index, action in enumerate(actions):
        entry = {"index": index, "stage": "before_light", **action, "started_at": utc_now()}
        evidence["actions"].append(entry)
        if action["type"] == "capture":
            entry["hwnd"] = session.target(action)
            capture(action["label"], f"action_{index:04d}_{action['label']}.png", False, entry["hwnd"])
        else:
            entry["hwnd"] = session.act(action)
        entry["completed_at"] = utc_now()
        entry["window_after"] = session.snapshot(session.hwnd)
    session.wait(args.dwell)
    capture("light", "light.png", True)
    if not args.single_theme:
        x, y = args.toggle
        for label in PHASES[1:]:
            entry = {"stage": f"switch_to_{label}", "type": "click", "x": x, "y": y,
                     "started_at": utc_now()}
            evidence["actions"].append(entry)
            entry["hwnd"] = session.act({"type": "click", "x": x, "y": y})
            entry["completed_at"] = utc_now()
            entry["window_after"] = session.snapshot(session.hwnd)
            capture(label, f"{label}.png", True)
    return evaluate(evidence["captures"], images, limits, args.protect_roi, args.single_theme)


def main(argv: list[str] | None = None) -> int:
    args, actions, limits = options(argv)
    out = OUTPUT / f"{args.demo}_{args.phase}_{datetime.now().strftime('%Y%m%d_%H%M%S')}_{uuid.uuid4().hex[:8]}"
    out.mkdir(parents=True, exist_ok=False)
    executable = args.bin_dir.resolve() / f"{args.demo}.exe"
    evidence: dict[str, Any] = {
        "schema_version": 1, "demo": args.demo, "phase": args.phase, "started_at": utc_now(),
        "executable": str(executable), "cwd": str(executable.parent), "status": "failed",
        "coordinate_spaces": {"input": "target HWND logical client pixels under its DPI context",
                              "protected_roi": "full-window screenshot pixels; no cropping"},
        "thresholds": asdict(limits), "options": {k: str(v) if isinstance(v, Path) else v for k, v in vars(args).items()},
        "captures": [], "actions": [], "checks": [], "comparisons": {}, "protected_rois": [],
    }
    process = session = None
    code = 1
    try:
        if sys.platform != "win32":
            raise SmokeError("GUI execution is Windows-only; pure logic tests are platform-independent")
        if not executable.is_file():
            raise SmokeError(f"demo executable does not exist: {executable}")
        # Resolve QA dependencies before launch so an import error cannot leak a demo.
        import win32con, win32gui, win32process  # noqa: F401
        evidence["process_dpi_awareness"] = enable_dpi_awareness()
        with (out / "process.log").open("wb") as log:
            process = subprocess.Popen(launch_command(executable, args.demo), cwd=str(executable.parent),
                                       stdout=log, stderr=subprocess.STDOUT,
                                       creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            evidence["pid"] = process.pid
            session = WindowsSession(process, args.demo, args)
            verdict = perform(session, args, actions, limits, out, evidence)
            evidence.update(verdict)
            evidence["status"] = "passed" if verdict["ok"] else "failed"
            code = 0 if verdict["ok"] else 1
    except KeyboardInterrupt:
        evidence["error"] = "interrupted"
        code = 130
    except Exception as exc:
        evidence["error"] = f"{type(exc).__name__}: {exc}"
    finally:
        try:
            if session:
                evidence["input_diagnostics"] = session.input_events
                evidence["cleanup"] = session.close()
                evidence["handled_dialogs"] = session.dialogs
                evidence["runtime_geometry_failures"] = session.geometry_failures
                if process.poll() is None:
                    raise SmokeError("launched demo still running after cleanup")
            elif process:
                # Constructor failure still closes only this retained Popen handle.
                evidence["cleanup"] = terminate_owned(process, args.close_timeout)
        except Exception as exc:
            evidence["cleanup_error"] = f"{type(exc).__name__}: {exc}"
            evidence["status"], code = "failed", 1
            if process and process.poll() is None:
                try:
                    evidence["cleanup_retry"] = terminate_owned(process, args.close_timeout)
                except Exception as retry:
                    evidence["cleanup_retry_error"] = str(retry)
        evidence["finished_at"] = utc_now()
        target = out / "evidence.json"
        target.write_text(json.dumps(evidence, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({"status": evidence["status"], "evidence": str(target),
                          "error": evidence.get("error"), "exit_code": code}, ensure_ascii=False), flush=True)
    return code


if __name__ == "__main__":
    raise SystemExit(main())
