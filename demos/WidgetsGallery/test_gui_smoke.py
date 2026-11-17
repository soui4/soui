"""Offline regression tests for the Gallery-only navigation screenshot gate."""

import json
import tempfile
import unittest
from pathlib import Path

import gui_smoke
from PIL import Image, ImageDraw


class PageTitleCheckTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.captures = []
        for index, key in enumerate(gui_smoke.PAGE_KEYS):
            path = Path(self.directory.name) / f"{key}.png"
            image = Image.new("RGB", (1256, 876), "white")
            ImageDraw.Draw(image).text((280, 118), key, fill="black")
            image.save(path)
            self.captures.append({"label": f"page_{key}", "image": str(path)})

    def test_current_action_flows_are_valid_and_visit_the_complete_catalog(self):
        directory = Path(__file__).resolve().parent / "tests"
        for name in ("gui_pages.json", "gui_catalog_actions.json"):
            actions = gui_smoke.smoke.validate_actions(json.loads((directory / name).read_text(encoding="utf-8")))
            self.assertTrue(actions)
            if name == "gui_pages.json":
                labels = {item["label"] for item in actions if item["type"] == "capture"}
                self.assertEqual(labels, {"page_" + key for key in gui_smoke.PAGE_KEYS})

    def test_all_distinct_page_titles_pass(self):
        self.assertTrue(all(check["ok"] for check in gui_smoke.page_title_checks(self.captures)))

    def test_missed_navigation_with_duplicate_title_fails(self):
        self.captures[0]["image"] = self.captures[5]["image"]
        checks = gui_smoke.page_title_checks(self.captures)
        self.assertTrue(any(not check["ok"] and check.get("duplicate_of") == "home" for check in checks))

    def test_missing_page_fails(self):
        self.assertFalse(gui_smoke.page_title_checks(self.captures[:-1])[0]["ok"])

    def test_normal_action_flow_without_page_captures_is_not_rejected(self):
        self.assertEqual(gui_smoke.page_title_checks([{"label": "input", "image": "unused"}]), [])


if __name__ == "__main__":
    unittest.main()
