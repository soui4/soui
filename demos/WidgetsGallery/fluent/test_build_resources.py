#!/usr/bin/env python3
"""Real-builder regressions for index-only resource changes and safe publication."""
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

import build_resources as resources

BUILDER = resources.REPO / "tools" / ("uiresbuilder.exe" if sys.platform == "win32" else "uiresbuilder")


class BuildResourcesTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="soui-resource-test-")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.demo = self.root / "demos/wallpaper"
        (self.demo / "uires").mkdir(parents=True)
        (self.demo / "res").mkdir()
        (self.demo / "uires/main.xml").write_text(
            '<SOUI><root><button name="keep" id="101"/></root></SOUI>', encoding="utf-8")
        (self.demo / "uires/image.png").write_bytes(b"resource builder does not decode images")
        self.index(False)

    def index(self, png):
        image = '<PNG><file name="added" path="image.png"/></PNG>' if png else ''
        (self.demo / "uires/uires.idx").write_text(
            '<resource><LAYOUT><file name="main" path="main.xml"/></LAYOUT>' + image + '</resource>',
            encoding="utf-8")

    def run_builder(self, builder=BUILDER):
        with patch.object(resources, "REPO", self.root), patch.object(
                sys, "argv", ["build_resources.py", "--demo", "wallpaper", "--builder", str(builder)]):
            resources.main()

    def generated(self):
        return {name: (self.demo / "res" / name).read_bytes() for name in ("resource.h", "soui_res.rc2")}

    @unittest.skipUnless(BUILDER.is_file(), "real uiresbuilder not installed")
    def test_index_only_addition_and_removal_refresh_header_without_changing_ids(self):
        self.run_builder()
        original = self.generated()
        self.index(True)  # No layout/UIDEF timestamp changes: this is the builder cache regression.
        self.run_builder()
        added = self.generated()
        header = added["resource.h"].decode("utf-8-sig")
        self.assertIn('_T("PNG:added")', header)
        self.assertIn('DEFINE_UIRES(added,', added["soui_res.rc2"].decode("utf-8-sig"))
        self.assertIn('keep = 101', header)
        self.index(False)
        self.run_builder()
        removed = self.generated()
        self.assertNotIn('_T("PNG:added")', removed["resource.h"].decode("utf-8-sig"))
        self.assertEqual(original["resource.h"], removed["resource.h"])

    @unittest.skipUnless(BUILDER.is_file(), "real uiresbuilder not installed")
    def test_repeat_is_byte_identical_and_does_not_rewrite_outputs(self):
        self.index(True)
        self.run_builder()
        before = self.generated()
        mtimes = {name: (self.demo / "res" / name).stat().st_mtime_ns for name in before}
        self.run_builder()
        self.assertEqual(before, self.generated())
        self.assertEqual(mtimes, {name: (self.demo / "res" / name).stat().st_mtime_ns for name in before})

    def test_failed_builder_cannot_publish_partial_output(self):
        for name in ("resource.h", "soui_res.rc2"):
            (self.demo / "res" / name).write_bytes(b"last good " + name.encode())
        before = self.generated()

        def fail(command, **kwargs):
            # A failed builder may have already written one of its requested output files.
            output = Path(command[command.index("-r") + 1])
            if not output.is_absolute():
                output = Path(kwargs["cwd"]) / output
            output.write_bytes(b"partial output")
            raise subprocess.CalledProcessError(7, command)

        builder = self.root / "mock-builder"
        builder.write_bytes(b"")  # Only the existence check is real; execution is mocked.
        with patch.object(resources.subprocess, "run", side_effect=fail):
            with self.assertRaises(subprocess.CalledProcessError):
                self.run_builder(builder)
        self.assertEqual(before, self.generated())

    def test_success_without_both_outputs_cannot_publish_partial_output(self):
        for name in ("resource.h", "soui_res.rc2"):
            (self.demo / "res" / name).write_bytes(b"last good " + name.encode())
        before = self.generated()
        builder = self.root / "mock-builder"
        builder.write_bytes(b"")

        def omit_header(command, **kwargs):
            Path(command[command.index("-r") + 1]).write_bytes(b"partial output")
            return subprocess.CompletedProcess(command, 0)

        with patch.object(resources.subprocess, "run", side_effect=omit_header):
            with self.assertRaises(FileNotFoundError):
                self.run_builder(builder)
        self.assertEqual(before, self.generated())


if __name__ == "__main__":
    unittest.main(verbosity=2)
