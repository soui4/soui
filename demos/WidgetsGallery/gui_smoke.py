#!/usr/bin/env python3
"""Gallery-only entry point for the existing PID-scoped Windows GUI verifier.

All input/capture/cleanup is scoped to the process launched by this command.
The shared verifier is not changed. Coordinates are logical client pixels.
"""
import ctypes
import hashlib
import importlib.util
import json
import sys
from ctypes import wintypes
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("gallery_fluent_gui", REPO / "demos/fluent/gui_smoke.py")
smoke = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = smoke
spec.loader.exec_module(smoke)
smoke.DEMOS = ("SOUIWidgetsGalleryDemo",)
smoke.OUTPUT = REPO / "output/gallery-gui"

class GallerySession(smoke.WindowsSession):
    def act(self, action):
        # Queue each click as one sequence, just like normal window input. With
        # cross-process SendMessage on down/up, capture can inject a real mouse
        # move between them and cancel the SOUI pressed state. Ownership and
        # foreground checks remain in the shared async transport.
        if action["type"] == "click":
            action = {"async": True, **action}
        return super().act(action)

    def cursor_position(self):
        point = wintypes.POINT()
        if not self.user.GetPhysicalCursorPos(ctypes.byref(point)):
            raise smoke.SmokeError("GetPhysicalCursorPos failed")
        return point.x, point.y

    def move_cursor(self, hwnd, point, foreground_host=None):
        self.position(hwnd, point)
        self._require_foreground(foreground_host or self._host(hwnd))
        screen = self.gui.ClientToScreen(hwnd, tuple(point))
        self.owner(self.gui.WindowFromPoint(screen))
        if not hasattr(self, "cursor_origin"):
            self.cursor_origin = self.cursor_position()
        # TrackMouseEvent and captured real mouse moves use the cursor, not a
        # synthetic message's lParam. Keep both at the same owned target.
        if not self.user.SetCursorPos(*screen):
            raise smoke.SmokeError("SetCursorPos failed for owned target")
        self.cursor_latest = self.cursor_position()

    def _input(self, hwnd, action, *, foreground_host=None):
        if action["type"] == "click":
            self.move_cursor(hwnd, [action["x"], action["y"]], foreground_host)
            self.wait(0.05)
        elif action["type"] == "drag":
            start, end = action["from"], action["to"]
            self.position(hwnd, end)
            self.move_cursor(hwnd, start)
            self.wait(0.05)
            self.send(hwnd, self.con.WM_MOUSEMOVE, 0, self.position(hwnd, start))
            self.send(hwnd, self.con.WM_LBUTTONDOWN, self.con.MK_LBUTTON, self.position(hwnd, start))
            try:
                steps = action.get("steps", 12)
                for index in range(1, steps + 1):
                    point = [round(a + (b - a) * index / steps) for a, b in zip(start, end)]
                    self.move_cursor(hwnd, point)
                    self.send(hwnd, self.con.WM_MOUSEMOVE, self.con.MK_LBUTTON, self.position(hwnd, point))
                    self.wait(self.args.dwell / steps)
            finally:
                self.send(hwnd, self.con.WM_LBUTTONUP, 0, self.position(hwnd, end))
                self.send(hwnd, self.con.WM_MOUSELEAVE)
            return
        return super()._input(hwnd, action, foreground_host=foreground_host)

    def close(self):
        restored = False
        try:
            if hasattr(self, "cursor_latest") and self.cursor_position() == self.cursor_latest:
                restored = bool(self.user.SetPhysicalCursorPos(*self.cursor_origin))
        finally:
            result = super().close()
        result["cursor_restored"] = restored
        return result


PAGE_KEYS = tuple(category["key"] for category in json.loads(
    (Path(__file__).resolve().parent / "control_catalog.json").read_text(encoding="utf-8"))["categories"])
TITLE_BOX = (274, 110, 760, 154)


def page_title_checks(captures):
    """Catch missed navigation, not merely successful/nonblack screenshots.

    This default-window-size check supplements (not replaces) visual review
    and headless assertions for the exact category/ID/page mapping.
    """
    from PIL import Image

    pages = {item["label"][5:]: item for item in captures if item["label"].startswith("page_")}
    if not pages:
        return []
    checks = [{"name": f"pages.all_{len(PAGE_KEYS)}_captured", "ok": set(pages) == set(PAGE_KEYS)}]
    hashes = {}
    for key, item in pages.items():
        with Image.open(item["image"]) as image:
            if image.size != (1256, 876):
                checks.append({"name": f"pages.{key}.default_capture_size", "ok": False})
                continue
            title = image.convert("RGB").crop(TITLE_BOX)
            digest = hashlib.sha256(title.tobytes()).hexdigest()
        duplicate = hashes.get(digest)
        checks.append({"name": f"pages.{key}.distinct_title", "ok": duplicate is None,
                       "duplicate_of": duplicate, "title_roi": list(TITLE_BOX), "sha256": digest})
        hashes[digest] = key
    return checks


shared_perform = smoke.perform


def perform(session, args, actions, limits, out, evidence):
    verdict = shared_perform(session, args, actions, limits, out, evidence)
    checks = page_title_checks(evidence["captures"])
    verdict["checks"].extend(checks)
    verdict["ok"] = verdict["ok"] and all(item["ok"] for item in checks)
    return verdict


smoke.WindowsSession = GallerySession
smoke.perform = perform

if __name__ == "__main__":
    raise SystemExit(smoke.main(["--demo", "SOUIWidgetsGalleryDemo", *sys.argv[1:]]))
