"""Pure logic/mocked Win32 QA tests; NEVER launch a demo or capture a window."""

import contextlib
import ctypes
from ctypes import wintypes
import io
import json
from pathlib import Path
import subprocess
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import Mock, patch

from PIL import Image, ImageDraw

import gui_smoke as qa


def window(hwnd=0x123456789, pid=123, title="", size=(320, 240)):
    return {"hwnd": hwnd, "pid": pid, "title": title, "visible": True,
            "rect": [50, 50, 50 + size[0], 50 + size[1]],
            "client_rect": [0, 0, *size], "minimized": False, "maximized": False}


def triplet():
    frames = [{**window(), "label": label, "theme_baseline": True} for label in qa.PHASES]
    images = {"light": Image.new("RGB", (320, 240), (245, 245, 245)),
              "dark": Image.new("RGB", (320, 240), (32, 32, 32)),
              "light_return": Image.new("RGB", (320, 240), (245, 245, 245))}
    return frames, images


class ActionTests(unittest.TestCase):
    def test_tool_demo_launch_commands_are_scoped(self):
        for demo in ("SoTool", "uieditor", "fun_test"):
            args = qa.parser().parse_args(["--demo", demo, "--single-theme"])
            executable = Path("bin") / (demo + ".exe")
            expected = [str(executable)]
            if demo == "fun_test":
                expected.append("--gtest_filter=window.soui")
            self.assertEqual(qa.launch_command(executable, args.demo), expected)

    def test_repo_location(self):
        self.assertEqual(qa.REPO, Path(qa.__file__).resolve().parents[2])
        self.assertEqual(qa.OUTPUT, qa.REPO / "output" / "remaining-fluent")

    def test_all_supported_actions_keep_order(self):
        actions = [{"type": "click", "x": 5, "y": 6}, {"type": "key", "key": "DOWN", "count": 3},
                   {"type": "text", "text": "中文 😀"}, {"type": "wait", "seconds": 0.1},
                   {"type": "capture", "label": "edited"}, {"type": "wheel", "x": 5, "y": 6, "delta": -240},
                   {"type": "drag", "from": [5, 6], "to": [50, 6]}]
        self.assertEqual(qa.validate_actions(actions), actions)

    def test_bad_actions_fail_before_launch(self):
        bad = [{}, {"type": []}, {"type": "shell", "cmd": "anything"},
               {"type": "click", "x": -1, "y": 2}, {"type": "click", "x": True, "y": 2},
               {"type": "click", "x": 1, "y": 2, "hwnd": 999},
               {"type": "key", "key": "CTRL+A"}, {"type": "wait", "seconds": float("nan")},
               {"type": "wait", "seconds": -1}, {"type": "text", "text": 3},
               {"type": "capture", "label": "../outside"}, {"type": "text", "text": "x", "target": "desktop"}]
        for action in bad:
            with self.subTest(action=action), self.assertRaises(ValueError):
                qa.validate_actions([action])
        with self.assertRaises(ValueError):
            qa.validate_actions({"actions": []})

    def test_click_async_preserves_explicit_booleans_and_default(self):
        click = {"type": "click", "x": 5, "y": 6}
        for options in ({}, {"async": False}, {"async": True}):
            action = {**click, **options}
            with self.subTest(options=options):
                self.assertEqual(qa.validate_actions([action]), [action])

    def test_click_async_requires_bool(self):
        for value in (None, 0, 1, 0.0, 1.0, "true", "false", [], {}):
            with self.subTest(value=value), self.assertRaisesRegex(ValueError, "async must be a boolean"):
                qa.validate_actions([{"type": "click", "x": 5, "y": 6, "async": value}])

    def test_async_is_illegal_on_non_click_actions(self):
        actions = [{"type": "key", "key": "ESC"}, {"type": "text", "text": "test"},
                   {"type": "wait", "seconds": 0}, {"type": "capture", "label": "popup"},
                   {"type": "wheel", "x": 5, "y": 6, "delta": -120},
                   {"type": "drag", "from": [5, 6], "to": [50, 6]}]
        for action in actions:
            for value in (False, True):
                with self.subTest(action=action, value=value), self.assertRaisesRegex(ValueError, "unexpected fields"):
                    qa.validate_actions([{**action, "async": value}])

    def test_async_click_supports_existing_buttons_and_counts(self):
        for button in ("left", "right"):
            for count in (1, 2):
                action = {"type": "click", "x": 5, "y": 6, "async": True,
                          "button": button, "count": count, "target": "popup"}
                with self.subTest(button=button, count=count):
                    self.assertEqual(qa.validate_actions([action]), [action])

    def test_async_click_rejects_invalid_buttons_counts_and_double_field(self):
        for options in ({"button": "middle"}, {"button": None}, {"count": 0},
                        {"count": 3}, {"count": True}, {"double": True}):
            with self.subTest(options=options), self.assertRaises(ValueError):
                qa.validate_actions([{"type": "click", "x": 5, "y": 6, "async": True, **options}])

    def test_unicode_and_newlines(self):
        self.assertEqual(qa.text_units("A😀\r\nB\n"), [65, 0xD83D, 0xDE00, 13, 66, 13])

    def test_signed_wheel_coordinates(self):
        self.assertEqual(qa.pack_point(-20, -10), 0xFFF6FFEC)
        with self.assertRaises(ValueError):
            qa.pack_point(32768, 1)

    def test_default_three_themes_and_legacy_flags(self):
        args, actions, limits = qa.options(["--demo", "wxDemo", "--phase", "edited", "--toggle", "700", "20",
                                           "--click", "1", "2", "--click", "3", "4", "--dwell", "0.2"])
        self.assertFalse(args.single_theme)
        self.assertEqual(args.toggle, [700, 20])
        self.assertEqual([(a["x"], a["y"]) for a in actions], [(1, 2), (3, 4)])
        self.assertEqual(limits.min_changed_fraction, 0.12)

    def test_single_theme_is_explicit_opt_out(self):
        args, _, _ = qa.options(["--demo", "wxDemo", "--single-theme"])
        self.assertTrue(args.single_theme)

    def test_invalid_options_fail_without_launch(self):
        for extra in ([], ["--toggle", "1", "2", "--single-theme"],
                      ["--single-theme", "--dwell", "nan"], ["--single-theme", "--phase", "../bad"]):
            with self.subTest(extra=extra), contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as exc:
                qa.options(["--demo", "wxDemo", *extra])
            self.assertEqual(exc.exception.code, 2)


class OwnershipGeometryTests(unittest.TestCase):
    def test_ownership_guard(self):
        qa.require_owner(123, 123)
        for actual, alive in ((456, True), (123, False)):
            with self.assertRaises(qa.OwnershipError):
                qa.require_owner(123, actual, alive)

    def test_blank_title_and_foreign_larger_window(self):
        own = window()
        foreign = window(hwnd=7, pid=456, title="private", size=(4000, 3000))
        self.assertEqual(qa.choose_main([foreign, own], 123, (320, 200), "wxDemo"), own)

    def test_main_demo_prompts_are_not_main(self):
        main = window(title="demo")
        prompts = [window(hwnd=9 + i, title=title, size=(800, 600)) for i, title in enumerate(qa.PROMPTS)]
        self.assertEqual(qa.choose_main([*prompts, main], 123, (320, 200), "demo"), main)

    def test_hidden_tiny_and_minimized_windows_ignored(self):
        windows = [{**window(), "visible": False}, window(size=(1, 1)), {**window(), "minimized": True}]
        self.assertIsNone(qa.choose_main(windows, 123, (320, 200), "wxDemo"))

    def test_moved_same_size_passes(self):
        before = window()
        after = {**before, "rect": [100, 200, 420, 440]}
        self.assertTrue(all(qa.geometry_checks(before, after).values()))

    def test_wrong_min_max_size_hwnd_fail(self):
        before = window()
        for change in ({"minimized": True}, {"maximized": True}, {"hwnd": 7}, {"pid": 456},
                       {"rect": [0, 0, 640, 480]}, {"client_rect": [0, 0, 200, 200]}, {"visible": False}):
            with self.subTest(change=change):
                self.assertFalse(all(qa.geometry_checks(before, {**before, **change}).values()))

    def test_snapshot_uses_ctypes_iszoomed_not_missing_pywin32(self):
        session = object.__new__(qa.WindowsSession)
        session.owner = Mock()
        session.process = SimpleNamespace(pid=123)
        session.gui = SimpleNamespace(GetWindowText=lambda _: "", GetWindowRect=lambda _: (0, 0, 910, 640),
                                      GetClientRect=lambda _: (0, 0, 910, 640), IsWindowVisible=lambda _: True,
                                      IsIconic=lambda _: False)
        session.user = SimpleNamespace(IsZoomed=Mock(return_value=False))
        session.dpi = SimpleNamespace(scope=lambda _: contextlib.nullcontext({"mode": "target-window-context"}))
        self.assertFalse(session.snapshot(0x123456789)["maximized"])
        session.user.IsZoomed.assert_called_once_with(0x123456789)


class ImageChecksTests(unittest.TestCase):
    def verdict(self, frames=None, images=None, rois=None, limits=None):
        defaults = triplet()
        return qa.evaluate(frames or defaults[0], images or defaults[1], limits or qa.Thresholds(), rois or [])

    def test_full_theme_roundtrip_passes(self):
        self.assertTrue(self.verdict()["ok"])

    def test_identical_images_cannot_be_theme_change(self):
        frames, images = triplet()
        images["dark"] = images["light"].copy()
        self.assertFalse(self.verdict(frames, images)["ok"])

    def test_hover_only_change_fails(self):
        frames, images = triplet()
        images["dark"] = images["light"].copy()
        ImageDraw.Draw(images["dark"]).rectangle((270, 5, 299, 34), fill=(0, 80, 160))
        self.assertFalse(self.verdict(frames, images)["ok"])

    def test_spatially_local_change_fails(self):
        frames, images = triplet()
        images["dark"] = images["light"].copy()
        ImageDraw.Draw(images["dark"]).rectangle((0, 0, 100, 100), fill=(32, 32, 32))
        self.assertFalse(self.verdict(frames, images)["ok"])

    def test_caret_and_selection_are_not_bitexact_requirements(self):
        frames, images = triplet()
        draw = ImageDraw.Draw(images["light_return"])
        draw.rectangle((5, 5, 160, 45), fill=(0, 100, 200))
        draw.line((100, 50, 100, 80), fill=(0, 0, 0), width=2)
        self.assertTrue(self.verdict(frames, images)["ok"])

    def test_return_still_dark_fails(self):
        frames, images = triplet()
        images["light_return"] = images["dark"].copy()
        self.assertFalse(self.verdict(frames, images)["ok"])

    def test_black_and_almost_black_fail(self):
        for color in ((0, 0, 0), (8, 8, 8)):
            self.assertFalse(qa.image_stats(Image.new("RGB", (20, 20), color), qa.Thresholds())["nonblack"])
        self.assertTrue(qa.image_stats(Image.new("RGB", (20, 20), (32, 32, 32)), qa.Thresholds())["nonblack"])

    def test_size_or_hwnd_changes_fail(self):
        frames, images = triplet()
        frames[1]["hwnd"] = 7
        self.assertFalse(self.verdict(frames, images)["ok"])
        frames, images = triplet()
        images["dark"] = images["dark"].resize((640, 480))
        self.assertFalse(self.verdict(frames, images)["ok"])

    def test_dynamic_roi_has_no_implicit_invariance_check(self):
        frames, images = triplet()
        ImageDraw.Draw(images["light_return"]).rectangle((30, 30, 90, 90), fill=(200, 80, 0))
        verdict = self.verdict(frames, images)
        self.assertTrue(verdict["ok"])
        self.assertEqual(verdict["protected_rois"], [])

    def test_explicit_protected_media_unchanged(self):
        frames, images = triplet()
        for image in images.values():
            ImageDraw.Draw(image).rectangle((10, 10, 49, 49), fill=(200, 80, 0))
        self.assertTrue(self.verdict(frames, images, [[10, 10, 40, 40]])["ok"])

    def test_explicit_protected_media_change_fails(self):
        self.assertFalse(self.verdict(rois=[[10, 10, 40, 40]])["ok"])

    def test_roi_bounds_do_not_silently_clip(self):
        for roi in ([300, 220, 50, 50], [-1, 0, 20, 20], [0, 0, 0, 20]):
            with self.subTest(roi=roi):
                self.assertFalse(self.verdict(rois=[roi])["ok"])

    def test_roi_tolerance_is_explicit(self):
        frames, images = triplet()
        for label, color in zip(qa.PHASES, (100, 101, 100)):
            ImageDraw.Draw(images[label]).rectangle((10, 10, 49, 49), fill=(color, color, color))
        self.assertFalse(self.verdict(frames, images, [[10, 10, 40, 40]])["ok"])
        self.assertTrue(self.verdict(frames, images, [[10, 10, 40, 40]], qa.Thresholds(roi_pixel_delta=1))["ok"])

    def test_single_theme_does_not_pretend_theme_verified(self):
        frames, images = triplet()
        verdict = qa.evaluate(frames[:1], {"light": images["light"]}, qa.Thresholds(), [], True)
        self.assertTrue(verdict["ok"])
        self.assertEqual(verdict["comparisons"], {})
        self.assertFalse(qa.evaluate(frames[:1], {"light": images["light"]}, qa.Thresholds(), [])["ok"])


class Function:
    def __init__(self, callback):
        self.callback = callback
        self.calls = []

    def __call__(self, *args):
        self.calls.append(args)
        return self.callback(*args)


def fake_dpi_user():
    user = SimpleNamespace(context=0x123456789ABCDEF, target=0xFEDCBA987654321)

    def set_context(value):
        old, user.context = user.context, value
        return old

    user.GetWindowDpiAwarenessContext = Function(lambda hwnd: user.target)
    user.SetThreadDpiAwarenessContext = Function(set_context)
    user.GetAwarenessFromDpiAwarenessContext = Function(lambda context: 0)
    user.GetDpiForWindow = Function(lambda hwnd: 96)
    return user


class DpiTests(unittest.TestCase):
    def test_pointer_width_signatures_and_restoration(self):
        user = fake_dpi_user()
        dpi = qa.TargetDpi(user)
        old = user.context
        self.assertEqual(user.GetWindowDpiAwarenessContext.argtypes, [wintypes.HWND])
        self.assertEqual(user.SetThreadDpiAwarenessContext.argtypes, [wintypes.HANDLE])
        self.assertEqual(ctypes.sizeof(user.GetWindowDpiAwarenessContext.restype), ctypes.sizeof(ctypes.c_void_p))
        self.assertEqual(ctypes.sizeof(user.SetThreadDpiAwarenessContext.restype), ctypes.sizeof(ctypes.c_void_p))
        with dpi.scope(0x123456789) as record:
            self.assertEqual(user.context, user.target)
            self.assertEqual(record["awareness"], "unaware")
        self.assertEqual(user.context, old)
        self.assertTrue(record["restored"])
        self.assertEqual(record["window_context"], hex(user.target))

    def test_exception_restores_old_context(self):
        user = fake_dpi_user()
        old = user.context
        with self.assertRaises(ValueError):
            with qa.TargetDpi(user).scope(1):
                raise ValueError("test")
        self.assertEqual(user.context, old)

    def test_nested_scopes_restore_in_order(self):
        user = fake_dpi_user()
        dpi, old = qa.TargetDpi(user), user.context
        with dpi.scope(1):
            with dpi.scope(2):
                self.assertEqual(user.context, user.target)
            self.assertEqual(user.context, user.target)
        self.assertEqual(user.context, old)

    def test_missing_api_fallback_is_explicit(self):
        dpi = qa.TargetDpi(SimpleNamespace())
        with dpi.scope(1) as record:
            self.assertEqual(record["mode"], "process-context-fallback")
            self.assertEqual(len(record["missing_apis"]), 2)
            self.assertIn("No cropping", record["warning"])

    def test_null_context_fails_closed(self):
        user = fake_dpi_user()
        user.GetWindowDpiAwarenessContext = Function(lambda hwnd: 0)
        with self.assertRaises(qa.SmokeError):
            with qa.TargetDpi(user).scope(1):
                self.fail("null context accepted")

    def test_context_set_failure_fails_closed(self):
        user = fake_dpi_user()
        user.SetThreadDpiAwarenessContext = Function(lambda context: 0)
        with self.assertRaises(qa.SmokeError):
            with qa.TargetDpi(user).scope(1):
                self.fail("set failure accepted")

    def test_restore_failure_is_not_hidden(self):
        user = fake_dpi_user()
        returns = iter([user.context, 0])
        user.SetThreadDpiAwarenessContext = Function(lambda context: next(returns))
        with self.assertRaises(qa.SmokeError):
            with qa.TargetDpi(user).scope(1):
                pass


class ClickDispatchTests(unittest.TestCase):
    def setUp(self):
        self.session = object.__new__(qa.WindowsSession)
        session = self.session
        session.hwnd, self.popup = 0x123456789, 0x987654321
        session.process = Mock(pid=123)
        session.process.poll.return_value = None
        session.args = SimpleNamespace(dwell=0, capture_timeout=1)
        session.con = SimpleNamespace(
            WM_NULL=0, WM_MOUSEMOVE=0x200, WM_MOUSELEAVE=0x2A3,
            WM_LBUTTONDOWN=0x201, WM_LBUTTONUP=0x202, WM_LBUTTONDBLCLK=0x203,
            WM_RBUTTONDOWN=0x204, WM_RBUTTONUP=0x205, WM_RBUTTONDBLCLK=0x206,
            MK_LBUTTON=1, MK_RBUTTON=2, WM_KEYDOWN=0x100, WM_KEYUP=0x101,
            SMTO_ABORTIFHUNG=2, SMTO_BLOCK=1)
        self.window_pids = {session.hwnd: 123, self.popup: 123}
        self.events, self.fail_message = [], None
        self.foreground, self.thread_flags = session.hwnd, 0
        self.thread_handles = {"hwndActive": session.hwnd, "hwndFocus": session.hwnd}
        session.input_events = []
        session.gui = SimpleNamespace(
            error=OSError,
            IsWindow=Mock(side_effect=lambda hwnd: hwnd in self.window_pids),
            IsWindowVisible=Mock(return_value=True), IsIconic=Mock(return_value=False),
            IsWindowEnabled=Mock(return_value=True),
            GetClassName=Mock(side_effect=lambda hwnd: "#32768" if hwnd == self.popup else "Host"),
            GetClientRect=Mock(side_effect=self.client_rect),
            PostMessage=Mock(side_effect=lambda *args: self.record_message("post", *args)))
        session.proc = SimpleNamespace(GetWindowThreadProcessId=Mock(
            side_effect=lambda hwnd: (7, self.window_pids[hwnd])))
        session.user = fake_dpi_user()
        self.original_context = session.user.context
        session.user.SendMessageTimeoutW = Mock(side_effect=self.send_message)
        session.user.MapVirtualKeyW = Mock(return_value=1)
        session.user.GetAncestor = Mock(return_value=session.hwnd)
        session.user.GetForegroundWindow = Mock(side_effect=lambda: self.foreground)
        session.user.SetForegroundWindow = Mock(side_effect=self.set_foreground)
        session.user.GetGUIThreadInfo = Mock(side_effect=self.thread_info)
        for name in ("SendInput", "AttachThreadInput", "SetFocus", "mouse_event", "keybd_event"):
            setattr(session.user, name, Mock(side_effect=AssertionError("global input is forbidden")))
        session.dpi = qa.TargetDpi(session.user)
        session.wait = Mock()
        session.windows = Mock(return_value=[window(session.hwnd), window(self.popup)])
        for name, value in (("set_last_error", None), ("get_last_error", 1460)):
            mock = patch.object(qa.ctypes, name, return_value=value, create=True)
            mock.start()
            self.addCleanup(mock.stop)
        self.output = io.StringIO()
        stdout = patch.object(qa.sys, "stdout", self.output)
        stdout.start()
        self.addCleanup(stdout.stop)

    def set_foreground(self, hwnd):
        self.foreground = hwnd
        return 1

    def thread_info(self, tid, pointer):
        info = ctypes.cast(pointer, ctypes.POINTER(qa.GUIThreadInfo)).contents
        info.flags = self.thread_flags
        for field, hwnd in self.thread_handles.items():
            setattr(info, field, hwnd)
        return 1

    def client_rect(self, hwnd):
        self.assertEqual(self.session.user.context, self.session.user.target)
        return (0, 0, 320, 240)

    def record_message(self, transport, hwnd, message, wparam, lparam):
        self.assertEqual(self.session.user.context, self.session.user.target)
        self.events.append((transport, hwnd, message, wparam, lparam))

    def send_message(self, hwnd, message, wparam, lparam, *args):
        self.record_message("send", hwnd, message, wparam, lparam)
        return int(message != self.fail_message)

    def expected_click(self, transport, button="left", count=1):
        down, up, double, flag = {"left": (0x201, 0x202, 0x203, 1),
                                  "right": (0x204, 0x205, 0x206, 2)}[button]
        pos = (6 << 16) | 5
        messages = [(0x200, 0, pos), (down, flag, pos), (up, 0, pos)]
        if count == 2:
            messages += [(double, flag, pos), (up, 0, pos)]
        messages += [(0x2A3, 0, 0)]
        return [(transport, self.session.hwnd, *message) for message in messages]

    def test_default_and_false_click_stay_synchronous(self):
        for options in ({}, {"async": False}):
            for button in ("left", "right"):
                for count in (1, 2):
                    with self.subTest(options=options, button=button, count=count):
                        self.events.clear()
                        result = self.session.act({"type": "click", "x": 5, "y": 6,
                                                   "button": button, "count": count, **options})
                        self.assertEqual(result, self.session.hwnd)
                        self.assertEqual(self.events, self.expected_click("send", button, count))
                        self.assertEqual(self.session.user.context, self.original_context)
        self.session.gui.PostMessage.assert_not_called()

    def test_async_click_posts_buttons_and_double_clicks_in_order(self):
        for button in ("left", "right"):
            for count in (1, 2):
                with self.subTest(button=button, count=count):
                    self.events.clear()
                    result = self.session.act({"type": "click", "x": 5, "y": 6, "async": True,
                                               "button": button, "count": count})
                    self.assertEqual(result, self.session.hwnd)
                    self.assertEqual(self.events, self.expected_click("post", button, count))
                    self.assertEqual(self.session.user.context, self.original_context)
        self.session.user.SendMessageTimeoutW.assert_not_called()
        self.assertEqual(self.session.gui.GetClientRect.call_count, 8)
        self.session.wait.assert_called_with(0)

    def test_sync_timeout_is_reported_without_post_fallback(self):
        self.fail_message = 0x202
        for options in ({}, {"async": False}):
            with self.subTest(options=options):
                self.events.clear()
                with self.assertRaisesRegex(qa.SmokeError, r"0x202.*error 1460"):
                    self.session.act({"type": "click", "x": 5, "y": 6, **options})
                self.assertEqual(self.events, self.expected_click("send")[:3])
                self.assertEqual(self.session.user.context, self.original_context)
        self.session.gui.PostMessage.assert_not_called()
        self.session.wait.assert_not_called()

    def test_async_click_never_calls_modal_sync_up(self):
        self.fail_message = 0x202
        self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
        self.assertEqual(self.events, self.expected_click("post"))
        self.session.user.SendMessageTimeoutW.assert_not_called()

    def test_async_click_checks_coordinates_before_any_message(self):
        for x, y in ((-1, 0), (320, 0), (0, 240), (32768, 0), (True, 0)):
            with self.subTest(x=x, y=y), self.assertRaises((qa.SmokeError, ValueError)):
                self.session.act({"type": "click", "x": x, "y": y, "async": True})
            self.assertEqual(self.events, [])
            self.assertEqual(self.session.user.context, self.original_context)
        self.session.user.SetForegroundWindow.assert_not_called()

    def test_async_click_rejects_foreign_stale_or_exited_owner(self):
        for state in ("foreign", "stale", "exited"):
            with self.subTest(state=state):
                self.window_pids[self.session.hwnd] = 456 if state == "foreign" else 123
                if state == "stale":
                    del self.window_pids[self.session.hwnd]
                self.session.process.poll.return_value = 1 if state == "exited" else None
                with self.assertRaises(qa.OwnershipError):
                    self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
                self.assertEqual(self.events, [])
        self.session.gui.GetClientRect.assert_not_called()
        self.assertEqual(self.session.user.GetWindowDpiAwarenessContext.calls, [])

    def test_async_click_revalidates_owner_between_each_post(self):
        for cutoff in (0x200, 0x201, 0x202):
            with self.subTest(cutoff=cutoff):
                self.events.clear()
                self.window_pids[self.session.hwnd] = 123

                def revoke_after_post(hwnd, message, wparam, lparam):
                    self.record_message("post", hwnd, message, wparam, lparam)
                    if message == cutoff:
                        self.window_pids[hwnd] = 456

                self.session.gui.PostMessage.side_effect = revoke_after_post
                with self.assertRaises(qa.OwnershipError):
                    self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
                expected = self.expected_click("post")
                length = next(i + 1 for i, event in enumerate(expected) if event[2] == cutoff)
                self.assertEqual(self.events, expected[:length])
                self.assertEqual(self.session.user.context, self.original_context)
        self.session.user.SendMessageTimeoutW.assert_not_called()

    def test_async_click_revalidates_liveness_between_posts(self):
        def exit_after_down(hwnd, message, wparam, lparam):
            self.record_message("post", hwnd, message, wparam, lparam)
            if message == 0x201:
                self.session.process.poll.return_value = 1

        self.session.gui.PostMessage.side_effect = exit_after_down
        with self.assertRaises(qa.OwnershipError):
            self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
        self.assertEqual(self.events, self.expected_click("post")[:2])
        self.assertEqual(self.session.user.context, self.original_context)
        self.session.user.SendMessageTimeoutW.assert_not_called()

    def test_async_click_dpi_failure_prevents_dispatch(self):
        self.session.user.GetWindowDpiAwarenessContext.callback = lambda hwnd: 0
        with self.assertRaises(qa.SmokeError):
            self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
        self.assertEqual(self.events, [])
        self.session.gui.GetClientRect.assert_not_called()

    def test_async_post_error_propagates_without_sync_retry(self):
        def fail_post(hwnd, message, wparam, lparam):
            self.record_message("post", hwnd, message, wparam, lparam)
            if message == 0x202:
                raise qa.SmokeError("queued mouse-up failed")

        self.session.gui.PostMessage.side_effect = fail_post
        with self.assertRaisesRegex(qa.SmokeError, "queued mouse-up failed"):
            self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
        self.assertEqual(self.events, self.expected_click("post")[:3])
        self.assertEqual(self.session.user.context, self.original_context)
        self.session.user.SendMessageTimeoutW.assert_not_called()
        self.session.wait.assert_not_called()

    def test_async_click_then_popup_wait_capture_and_escape(self):
        self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
        self.session.act({"type": "wait", "seconds": 0.1})
        popup = self.session.target({"type": "capture", "target": "popup", "label": "menu"})
        self.session.snapshot = Mock(side_effect=lambda hwnd: window(hwnd))
        image = Image.new("RGB", (320, 240), "white")
        self.session._bitmap = Mock(return_value=(image, {}))
        metadata, captured = self.session.capture(popup)
        self.assertEqual(metadata["hwnd"], self.popup)
        self.assertIs(captured, image)
        self.session.act({"type": "key", "key": "ESC", "target": "popup"})
        key_flags = 1 | (1 << 16)
        self.assertEqual(self.events, self.expected_click("post") + [
            ("send", self.popup, 0, 0, 0),
            ("post", self.popup, 0x100, 0x1B, key_flags),
            ("post", self.popup, 0x101, 0x1B, key_flags | (1 << 30) | (1 << 31))])
        self.session.wait.assert_any_call(0.1)
        self.assertEqual(self.session.user.context, self.original_context)


    def test_async_click_activates_only_validated_foreground_host_once(self):
        self.foreground = 0x111111111
        self.window_pids[self.foreground] = 456
        self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
        self.session.user.SetForegroundWindow.assert_called_once_with(self.session.hwnd)
        self.assertEqual(self.events, self.expected_click("post"))
        record = self.session.input_events[-1]
        self.assertEqual(record["before"]["foreground"]["pid"], 456)
        self.assertFalse(record["before"]["foreground"]["owned"])
        self.assertEqual(record["activation"]["host_hwnd"], self.session.hwnd)
        self.assertTrue(record["after"]["foreground"]["owned"])
        self.assertEqual(record["transport"], "PostMessage")
        lines = [line for line in self.output.getvalue().splitlines() if line.startswith("pid-scoped-input ")]
        self.assertEqual(json.loads(lines[-1].split(" ", 1)[1]), record)
        self.session.gui.GetClassName.assert_called_with(self.popup)
        self.assertNotIn(0x111111111, [call.args[0] for call in self.session.gui.GetClassName.call_args_list])

    def test_already_foreground_async_click_does_not_reactivate(self):
        self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
        self.session.user.SetForegroundWindow.assert_not_called()
        self.assertFalse(self.session.input_events[-1]["activation"]["attempted"])

    def test_sync_click_does_not_request_foreground(self):
        self.foreground = 0
        self.session.act({"type": "click", "x": 5, "y": 6})
        self.assertEqual(self.events, self.expected_click("send"))
        self.session.user.SetForegroundWindow.assert_not_called()
        self.session.user.GetForegroundWindow.assert_not_called()
        self.assertEqual(self.session.input_events, [])

    def test_foreground_denied_or_unconfirmed_never_dispatches_or_retries(self):
        self.foreground = 0x111111111
        self.window_pids[self.foreground] = 456
        for api_result in (0, 1):
            with self.subTest(api_result=api_result):
                self.session.user.SetForegroundWindow.reset_mock()
                self.session.user.SetForegroundWindow.side_effect = None
                self.session.user.SetForegroundWindow.return_value = api_result
                with patch.object(qa.time, "monotonic", side_effect=[0, 1]), self.assertRaisesRegex(
                        qa.SmokeError, "foreground.*not confirmed"):
                    self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
                self.session.user.SetForegroundWindow.assert_called_once_with(self.session.hwnd)
                self.assertEqual(self.events, [])
                self.assertIn("error", self.session.input_events[-1])
                self.assertFalse(self.session.input_events[-1]["after"]["foreground"]["owned"])
        self.session.user.SendMessageTimeoutW.assert_not_called()

    def test_activation_is_bounded_observation_not_repeated_requests(self):
        self.foreground = 0
        self.session.user.SetForegroundWindow.side_effect = None
        self.session.user.SetForegroundWindow.return_value = 1

        def settle(_):
            self.foreground = self.session.hwnd

        with patch.object(qa.time, "sleep", side_effect=settle) as sleep:
            result = self.session.activate_host(self.session.hwnd)
        self.assertTrue(result["attempted"])
        self.session.user.SetForegroundWindow.assert_called_once_with(self.session.hwnd)
        sleep.assert_called_once()

    def test_confirmed_foreground_is_checked_even_when_api_returns_false(self):
        self.foreground = 0

        def activated(hwnd):
            self.foreground = hwnd
            return 0

        self.session.user.SetForegroundWindow.side_effect = activated
        self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
        self.assertEqual(self.events, self.expected_click("post"))
        self.assertFalse(self.session.input_events[-1]["activation"]["api_return"])

    def test_unavailable_hidden_minimized_or_disabled_host_fails_closed(self):
        for api, bad in (("IsWindowVisible", False), ("IsIconic", True), ("IsWindowEnabled", False)):
            with self.subTest(api=api):
                function = getattr(self.session.gui, api)
                normal = function.return_value
                function.return_value = bad
                try:
                    with self.assertRaisesRegex(qa.SmokeError, "host.*interactive"):
                        self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
                finally:
                    function.return_value = normal
                self.assertEqual(self.events, [])
                self.session.user.SetForegroundWindow.assert_not_called()

    def test_foreign_or_destroyed_root_is_not_activated(self):
        for state in ("foreign", "destroyed"):
            with self.subTest(state=state):
                root = 0x222222222
                self.session.user.GetAncestor.return_value = root
                self.window_pids[root] = 456
                if state == "destroyed":
                    del self.window_pids[root]
                with self.assertRaises(qa.OwnershipError):
                    self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
                self.assertEqual(self.events, [])
                self.session.user.SetForegroundWindow.assert_not_called()

    def test_focus_child_activates_its_owned_root_not_the_child(self):
        child = 0x333333333
        self.window_pids[child] = 123
        self.thread_handles["hwndFocus"] = child
        self.foreground = 0
        result = self.session.act({"type": "click", "x": 5, "y": 6, "async": True, "target": "focus"})
        self.assertEqual(result, child)
        self.session.user.GetAncestor.assert_called_with(child, 2)
        self.session.user.SetForegroundWindow.assert_called_once_with(self.session.hwnd)
        self.assertTrue(all(event[1] == child for event in self.events))

    def test_popup_click_never_raises_host_over_open_menu(self):
        self.thread_flags = 0x14
        self.session.act({"type": "click", "x": 5, "y": 6, "async": True, "target": "popup"})
        self.session.user.SetForegroundWindow.assert_not_called()
        self.assertTrue(all(event[1] == self.popup for event in self.events))
        self.assertIsNone(self.session.input_events[-1]["activation"])

    def test_active_menu_is_not_reactivated_when_foreground_is_lost(self):
        self.thread_flags = 0x14
        self.thread_handles["hwndMenuOwner"] = self.session.hwnd
        self.foreground = 0
        with self.assertRaisesRegex(qa.SmokeError, "menu.*active"):
            self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
        self.session.user.SetForegroundWindow.assert_not_called()
        self.assertEqual(self.events, [])

    def test_foreground_loss_before_dispatch_prevents_all_messages(self):
        original = self.session.gui.GetClientRect.side_effect
        count = 0

        def steal(hwnd):
            nonlocal count
            count += 1
            if count == 2:
                self.foreground = 0
            return original(hwnd)

        self.session.gui.GetClientRect.side_effect = steal
        with self.assertRaisesRegex(qa.SmokeError, "foreground.*lost"):
            self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
        self.assertEqual(self.events, [])
        self.session.user.SetForegroundWindow.assert_not_called()

    def test_foreground_loss_between_posts_stops_without_global_fallback(self):
        def steal_after_move(hwnd, message, wparam, lparam):
            self.record_message("post", hwnd, message, wparam, lparam)
            self.foreground = 0

        self.session.gui.PostMessage.side_effect = steal_after_move
        with self.assertRaisesRegex(qa.SmokeError, "foreground.*lost"):
            self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
        self.assertEqual(self.events, self.expected_click("post")[:1])
        self.session.user.SendMessageTimeoutW.assert_not_called()
        self.session.user.SetForegroundWindow.assert_not_called()

    def test_activation_revalidates_liveness_and_owner_after_request(self):
        for state in ("foreign", "exited", "destroyed"):
            with self.subTest(state=state):
                self.foreground = 0
                self.window_pids[self.session.hwnd] = 123
                self.session.process.poll.return_value = None

                def revoke(hwnd):
                    if state == "foreign":
                        self.window_pids[hwnd] = 456
                    elif state == "destroyed":
                        del self.window_pids[hwnd]
                    else:
                        self.session.process.poll.return_value = 1
                    return 1

                self.session.user.SetForegroundWindow.side_effect = revoke
                with self.assertRaises(qa.OwnershipError):
                    self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
                self.assertEqual(self.events, [])
                self.assertIn("diagnostic_error", self.session.input_events[-1])

    def test_diagnostics_report_menu_flags_and_foreign_thread_handles_as_unowned(self):
        foreign = 0x444444444
        self.window_pids[foreign] = 456
        self.thread_flags = 0x14
        self.thread_handles.update(hwndCapture=foreign, hwndMenuOwner=self.session.hwnd)
        state = self.session.input_diagnostics()
        self.assertTrue(state["thread"]["menu_mode"])
        self.assertEqual(state["thread"]["flags"], 0x14)
        self.assertFalse(state["thread"]["hwndCapture"]["owned"])
        self.assertEqual(state["owned_popups"][0]["class"], "#32768")
        self.assertEqual(self.events, [])
        self.session.user.SetForegroundWindow.assert_not_called()

    def test_thread_query_failure_is_reported_without_activation(self):
        self.session.user.GetGUIThreadInfo.side_effect = None
        self.session.user.GetGUIThreadInfo.return_value = 0
        self.foreground = 0
        with self.assertRaisesRegex(qa.SmokeError, "thread.*unavailable"):
            self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
        self.assertEqual(self.session.input_events[-1]["before"]["thread"]["error"], 1460)
        self.session.user.SetForegroundWindow.assert_not_called()
        self.assertEqual(self.events, [])


    def test_owned_modal_foreground_handoff_keeps_original_click_order(self):
        def open_dialog(hwnd, message, wparam, lparam):
            self.record_message("post", hwnd, message, wparam, lparam)
            if message == self.session.con.WM_LBUTTONDOWN:
                self.foreground = self.popup

        self.session.gui.PostMessage.side_effect = open_dialog
        self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
        self.assertEqual(self.events, self.expected_click("post"))
        self.assertEqual(self.session.input_events[-1]["after"]["foreground"]["hwnd"], self.popup)
        self.session.user.SetForegroundWindow.assert_not_called()

    def test_new_native_menu_diagnostics_do_not_raise_host_on_followup_sync_click(self):
        self.session.windows.return_value = [window(self.session.hwnd)]

        def open_menu(hwnd, message, wparam, lparam):
            self.record_message("post", hwnd, message, wparam, lparam)
            if message == self.session.con.WM_LBUTTONUP:
                self.thread_flags = 0x14
                self.thread_handles["hwndMenuOwner"] = self.session.hwnd
                self.session.windows.return_value = [window(self.session.hwnd), window(self.popup)]

        self.session.gui.PostMessage.side_effect = open_menu
        self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
        state = self.session.input_events[-1]["after"]
        self.assertEqual(state["foreground"]["hwnd"], self.session.hwnd)
        self.assertTrue(state["foreground"]["owned"])
        self.assertTrue(state["thread"]["menu_mode"])
        self.assertEqual(state["thread"]["hwndMenuOwner"]["hwnd"], self.session.hwnd)
        self.assertEqual(state["owned_popups"][0]["class"], "#32768")
        foreground_reads = self.session.user.GetForegroundWindow.call_count
        self.session.act({"type": "click", "x": 5, "y": 6})
        self.assertEqual(self.session.user.GetForegroundWindow.call_count, foreground_reads)
        self.session.user.SetForegroundWindow.assert_not_called()
        self.assertEqual(self.events, self.expected_click("post") + self.expected_click("send"))

    def test_null_and_disappeared_foreground_are_read_only_diagnostics(self):
        state = self.session.input_diagnostics()
        self.foreground = 0
        null_state = self.session.input_diagnostics()
        self.assertEqual(null_state["foreground"]["hwnd"], 0)
        self.assertFalse(null_state["foreground"]["owned"])
        self.foreground = 0x555555555
        original = self.session.proc.GetWindowThreadProcessId.side_effect

        def stale(hwnd):
            if hwnd == self.foreground:
                raise OSError("foreground disappeared")
            return original(hwnd)

        self.session.proc.GetWindowThreadProcessId.side_effect = stale
        missing_state = self.session.input_diagnostics()
        self.assertFalse(missing_state["foreground"]["owned"])
        self.assertIn("disappeared", missing_state["foreground"]["error"])
        self.assertEqual(state["host_hwnd"], self.session.hwnd)
        self.session.user.SetForegroundWindow.assert_not_called()
        self.assertEqual(self.events, [])

    def test_secondary_diagnostic_failure_does_not_mask_input_error(self):
        before = self.session.input_diagnostics()
        self.session.input_diagnostics = Mock(side_effect=[before, qa.SmokeError("diagnostic unavailable")])
        self.session.gui.PostMessage.side_effect = qa.SmokeError("input failed")
        with self.assertRaisesRegex(qa.SmokeError, "input failed"):
            self.session.act({"type": "click", "x": 5, "y": 6, "async": True})
        self.assertEqual(self.session.input_events[-1]["error"], "SmokeError: input failed")
        self.assertIn("diagnostic unavailable", self.session.input_events[-1]["diagnostic_error"])
        self.session.user.SendMessageTimeoutW.assert_not_called()

    def test_popup_target_gate_still_rejects_missing_or_ambiguous_windows(self):
        for candidates in ([window(self.session.hwnd)],
                           [window(self.session.hwnd), window(self.popup), window(0x666666666)]):
            with self.subTest(count=len(candidates)):
                self.session.windows.return_value = candidates
                with self.assertRaisesRegex(qa.SmokeError, "popup target is ambiguous/missing"):
                    self.session.act({"type": "click", "x": 5, "y": 6, "async": True, "target": "popup"})
        self.assertEqual(self.events, [])
        self.session.user.SetForegroundWindow.assert_not_called()


class ForegroundSignatureTests(unittest.TestCase):
    def test_foreground_apis_keep_pointer_sized_handles(self):
        user, gdi = Mock(), Mock()
        modules = {name: SimpleNamespace() for name in ("win32con", "win32gui", "win32process")}
        with patch.dict(qa.sys.modules, modules), patch.object(qa.ctypes, "WinDLL", create=True,
                side_effect=[user, gdi]):
            session = qa.WindowsSession(Mock(pid=123), "SoTool", SimpleNamespace())
        self.assertEqual(session.input_events, [])
        self.assertEqual(user.GetForegroundWindow.argtypes, [])
        self.assertEqual(user.GetForegroundWindow.restype, wintypes.HWND)
        self.assertEqual(user.SetForegroundWindow.argtypes, [wintypes.HWND])
        self.assertEqual(user.SetForegroundWindow.restype, wintypes.BOOL)
        self.assertEqual(user.GetAncestor.argtypes, [wintypes.HWND, wintypes.UINT])
        self.assertEqual(user.GetAncestor.restype, wintypes.HWND)


class CleanupWorkflowTests(unittest.TestCase):
    def test_mocked_main_hides_command_window_and_persists_input_diagnostics(self):
        # Even main's dependencies/process/capture are mocked: this is not live GUI.
        qa.OUTPUT.mkdir(parents=True, exist_ok=True)
        for failed in (False, True):
            with self.subTest(failed=failed), tempfile.TemporaryDirectory(prefix="qa_logic_", dir=qa.OUTPUT) as tmp:
                root = Path(tmp).resolve()
                self.assertTrue(root.is_relative_to(qa.OUTPUT.resolve()))
                binary = root / "bin"
                binary.mkdir()
                (binary / "SoTool.exe").touch()
                process = Mock(pid=123, returncode=0)
                process.poll.return_value = 0
                diagnostics = [{"pid": 123, "target_hwnd": 0x123456789, "transport": "PostMessage"}]
                session = SimpleNamespace(input_events=diagnostics, dialogs=[], geometry_failures=[],
                                          close=Mock(return_value={"exit_code": 0}))
                modules = {name: SimpleNamespace() for name in ("win32con", "win32gui", "win32process")}
                with patch.object(qa, "OUTPUT", root / "evidence"), patch.dict(qa.sys.modules, modules), \
                        patch.object(qa.sys, "platform", "win32"), \
                        patch.object(qa, "enable_dpi_awareness", return_value="mocked"), \
                        patch.object(qa.subprocess, "CREATE_NO_WINDOW", 0x08000000, create=True), \
                        patch.object(qa.subprocess, "Popen", return_value=process) as launch, \
                        patch.object(qa, "WindowsSession", return_value=session), \
                        patch.object(qa, "perform", return_value={"ok": True},
                                     side_effect=qa.SmokeError("popup absent") if failed else None), \
                        contextlib.redirect_stdout(io.StringIO()):
                    code = qa.main(["--demo", "SoTool", "--single-theme", "--bin-dir", str(binary)])
                self.assertEqual(code, 1 if failed else 0)
                launch.assert_called_once()
                self.assertEqual(launch.call_args.kwargs["creationflags"], 0x08000000)
                self.assertNotIn("shell", launch.call_args.kwargs)
                evidence = json.loads(next((root / "evidence").rglob("evidence.json")).read_text(encoding="utf-8"))
                self.assertEqual(evidence["input_diagnostics"], diagnostics)
                self.assertEqual(evidence["status"], "failed" if failed else "passed")
                session.close.assert_called_once()
                process.terminate.assert_not_called()
                process.kill.assert_not_called()

    def test_terminate_retained_process_only(self):
        process = Mock(pid=123)
        process.poll.side_effect = [None, 1]
        result = qa.terminate_owned(process, 0.1)
        process.terminate.assert_called_once_with()
        process.kill.assert_not_called()
        self.assertTrue(result["terminated"])

    def test_already_exited_is_not_terminated(self):
        process = Mock(pid=123)
        process.poll.return_value = 0
        self.assertEqual(qa.terminate_owned(process, 0.1)["exit_code"], 0)
        process.terminate.assert_not_called()

    def test_kill_is_same_retained_process_on_timeout(self):
        process = Mock(pid=123)
        process.poll.side_effect = [None, 1]
        process.wait.side_effect = [subprocess.TimeoutExpired("fake", 0.1), 1]
        result = qa.terminate_owned(process, 0.1)
        process.kill.assert_called_once_with()
        self.assertTrue(result["killed"])

    def test_close_enumeration_error_still_cleans_own_process(self):
        session = object.__new__(qa.WindowsSession)
        session.process = Mock(pid=123)
        session.process.poll.side_effect = [None, None, None, 1]
        session.args = SimpleNamespace(close_timeout=0.1)
        session.windows = Mock(side_effect=qa.SmokeError("DPI query failed"))
        result = session.close()
        session.process.terminate.assert_called_once_with()
        self.assertIn("DPI query failed", result["errors"])

    def test_ordered_workflow_and_three_output_images_without_gui(self):
        args, _, limits = qa.options(["--demo", "wxDemo", "--toggle", "100", "20", "--dwell", "0"])
        actions = qa.validate_actions([{"type": "text", "text": "test"}, {"type": "key", "key": "HOME"},
                                       {"type": "capture", "label": "edited"}, {"type": "wait", "seconds": 0}])
        events, toggle = [], [0]
        frames, images = triplet()
        session = SimpleNamespace(hwnd=frames[0]["hwnd"], baseline=frames[0], dpi=SimpleNamespace(support={}))
        session.find_main = lambda: events.append("find_main")
        session.snapshot = lambda hwnd: window(hwnd)
        session.target = lambda action: session.hwnd
        session.wait = lambda seconds: events.append("wait")

        def act(action):
            events.append(action["type"])
            if action["type"] == "click":
                toggle[0] += 1
            return session.hwnd

        session.act = act
        session.capture = lambda hwnd=None: (window(), images[qa.PHASES[toggle[0]]].copy())
        qa.OUTPUT.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="qa_logic_", dir=qa.OUTPUT) as tmp:
            out = Path(tmp).resolve()
            self.assertTrue(out.is_relative_to(qa.OUTPUT.resolve()))  # Cleanup cannot reach peers.
            evidence = {"captures": [], "actions": [], "checks": []}
            with patch("subprocess.Popen", side_effect=AssertionError("tests must not launch anything")), contextlib.redirect_stdout(io.StringIO()):
                verdict = qa.perform(session, args, actions, limits, out, evidence)
            self.assertTrue(verdict["ok"])
            self.assertEqual(events[:3], ["find_main", "text", "key"])
            self.assertEqual([f["label"] for f in evidence["captures"] if f["theme_baseline"]], list(qa.PHASES))
            self.assertEqual(sorted(p.name for p in out.glob("*.png")),
                             ["action_0002_edited.png", "dark.png", "light.png", "light_return.png"])
            json.dumps({**evidence, **verdict})


if __name__ == "__main__":
    unittest.main()
