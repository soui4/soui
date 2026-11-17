"""Bounded row-text policy regressions; stdlib only, no build or GUI."""

import contextlib
import importlib.util
import io
from pathlib import Path
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET

REPO = Path(__file__).resolve().parents[2]
SCRIPT = Path(__file__).with_name("sync_row_text.py")
CLASS_NAME = "fluent_row_label"
POLICY_CLASS = '<class name="fluent_row_label" padding="8dp,0,8dp,0"/>'
SOURCE = ('<?xml version="1.0" encoding="utf-8"?>\n'
          '<UIDEF>\n  <style>\n    ' + POLICY_CLASS + '\n  </style>\n'
          '  <objattr><listbox text-x="8dp" itemHeight="32dp"/></objattr>\n'
          '</UIDEF>\n')
INIT = ('<?xml version="1.0" encoding="utf-8"?>\n'
        '<UIDEF>\n  <unit defUnit="px"/>\n'
        '  <!-- <class name="fluent_row_label" padding="99"/> -->\n'
        '  <style>\n    <class name="custom" font="size:17"/>\n'
        '    <!-- 保留中文注释 Keep custom styles verbatim. -->\n  </style>\n'
        '  <objattr>\n'
        '    <!-- <listbox text-x="99"/> -->\n'
        '    <listbox itemHeight="30" font="size:13" skin="custom"/>\n'
        '    <text align="center"/>\n  </objattr>\n'
        '  <skin src="values:custom_palette"/>\n</UIDEF>\n')


class SourcePolicyTests(unittest.TestCase):
    def test_shared_row_class_changes_only_horizontal_padding(self):
        root = ET.parse(REPO / "demos/fluent/controls.xml").getroot()
        nodes = root.findall("./style/class[@name='fluent_row_label']")
        self.assertEqual(len(nodes), 1, "shared row label policy is missing/duplicated")
        self.assertEqual(nodes[0].attrib,
                         {"name": CLASS_NAME, "padding": "8dp,0,8dp,0"})
        self.assertEqual(root.find("./objattr/listbox").get("text-x"), "8dp")

    def test_demo2_actual_label_consumes_local_row_class(self):
        uires = REPO / "demos/demo2/uires"
        init = ET.parse(uires / "uidef/init.xml").getroot()
        nodes = init.findall("./style/class[@name='fluent_row_label']")
        self.assertEqual(len(nodes), 1, "demo2 local class is missing/duplicated")
        self.assertEqual(nodes[0].attrib,
                         {"name": CLASS_NAME, "padding": "8dp,0,8dp,0"})
        self.assertEqual(init.find("unit").get("defUnit"), "px")
        main = ET.parse(uires / "xml/dlg_main.xml").getroot()
        row = main.find(".//listview[@name='lv_test']/template")
        self.assertEqual(row.attrib, {"itemHeight": "50", "layout": "hbox",
                                     "gravity": "center", "skin": "skin_item_bk"})
        label = row.find("text")
        self.assertEqual(label.attrib, {"name": "txt_test", "class": CLASS_NAME,
                                       "colorText": "@color/fluent_text"})
        self.assertEqual(len(row), 1)
        self.assertEqual(init.find("./objattr/listbox").get("text-x"), "8dp")

    def test_wallpaper_category_inset_is_dp_but_geometry_stays_px(self):
        uires = REPO / "demos/wallpaper/uires"
        init = ET.parse(uires / "uidef/init.xml").getroot()
        self.assertEqual(init.find("unit").get("defUnit"), "px")
        self.assertEqual(init.find("./objattr/listbox").get("text-x"), "8dp")
        self.assertEqual(init.findall("./style/class[@name='fluent_row_label']"), [])
        main = ET.parse(uires / "xml/dlg_main.xml").getroot()
        category = main.find(".//listbox[@name='lb_catetory']")
        self.assertEqual(category.attrib, {"text-x": "8dp", "size": "128,-2",
                                          "name": "lb_catetory", "itemHeight": "30"})


class SyncTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        spec = importlib.util.spec_from_file_location("sync_row_text_under_test", SCRIPT)
        cls.sync = importlib.util.module_from_spec(spec)
        sys.modules[spec.name] = cls.sync
        spec.loader.exec_module(cls.sync)

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.repo = Path(self.temp.name)
        self.put("demos/fluent/controls.xml", SOURCE.encode("utf-8"))
        for demo in ("demo2", "wallpaper", "wxDemo"):
            self.put(f"demos/{demo}/uires/uidef/init.xml", INIT.encode("utf-8"))
            self.put(f"demos/{demo}/uires/values/fluent_light.xml", b"custom palette\n")
            self.put(f"demos/{demo}/uires/xml/dlg_main.xml", b"layout must not be rewritten\n")

    def put(self, relative, data):
        path = self.repo / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        return path

    def snapshot(self):
        return {p.relative_to(self.repo): p.read_bytes()
                for p in self.repo.rglob("*") if p.is_file()}

    def test_explicit_demo_only_and_no_palette_or_layout_rewrites(self):
        before = self.snapshot()
        self.assertTrue(self.sync.sync_demo("demo2", self.repo))
        after = self.snapshot()
        target = Path("demos/demo2/uires/uidef/init.xml")
        self.assertEqual({p for p in after if after[p] != before[p]}, {target})
        expected = INIT.replace("  </style>", "    " + POLICY_CLASS + "\n  </style>")
        expected = expected.replace('skin="custom"/>', 'skin="custom" text-x="8dp"/>')
        self.assertEqual(after[target], expected.encode("utf-8"))

    def test_second_sync_is_byte_identical_and_does_not_write(self):
        self.sync.sync_demo("demo2", self.repo)
        path = self.repo / "demos/demo2/uires/uidef/init.xml"
        before = self.snapshot()
        stamp = path.stat().st_mtime_ns
        self.assertFalse(self.sync.sync_demo("demo2", self.repo))
        self.assertEqual(self.snapshot(), before)
        self.assertEqual(path.stat().st_mtime_ns, stamp)

    def test_bom_crlf_comments_and_unmanaged_bytes_are_preserved(self):
        path = self.put("demos/demo2/uires/uidef/init.xml",
                        b"\xef\xbb\xbf" + INIT.replace("\n", "\r\n").encode("utf-8"))
        self.sync.sync_demo("demo2", self.repo)
        expected = INIT.replace("  </style>", "    " + POLICY_CLASS + "\n  </style>")
        expected = expected.replace('skin="custom"/>', 'skin="custom" text-x="8dp"/>')
        self.assertEqual(path.read_bytes(), b"\xef\xbb\xbf" + expected.replace("\n", "\r\n").encode("utf-8"))

    def test_existing_class_and_inset_update_values_not_other_attributes(self):
        old = INIT.replace('<class name="custom" font="size:17"/>',
                           "<class name='fluent_row_label' padding='2px,0,2px,0' tip='keep &amp; me'/>")
        old = old.replace('skin="custom"/>', "skin=\"custom\" text-x='2px'/>")
        path = self.put("demos/demo2/uires/uidef/init.xml", old.encode("utf-8"))
        self.sync.sync_demo("demo2", self.repo)
        self.assertEqual(path.read_bytes(), old.replace("padding='2px,0,2px,0'", "padding='8dp,0,8dp,0'")
                         .replace("text-x='2px'", "text-x='8dp'").encode("utf-8"))

    def test_wallpaper_only_syncs_listbox_default_not_unused_row_class(self):
        self.sync.sync_demo("wallpaper", self.repo)
        path = self.repo / "demos/wallpaper/uires/uidef/init.xml"
        expected = INIT.replace('skin="custom"/>', 'skin="custom" text-x="8dp"/>')
        self.assertEqual(path.read_bytes(), expected.encode("utf-8"))
        self.assertFalse(self.sync.sync_demo("wallpaper", self.repo))

    def test_compact_xml_remains_compact(self):
        old = '<UIDEF><style></style><objattr><listbox text-x="1"/></objattr></UIDEF>'
        path = self.put("demos/demo2/uires/uidef/init.xml", old.encode("utf-8"))
        self.sync.sync_demo("demo2", self.repo)
        expected = old.replace("</style>", POLICY_CLASS + "</style>").replace('text-x="1"', 'text-x="8dp"')
        self.assertEqual(path.read_bytes(), expected.encode("utf-8"))

    def test_missing_explicit_demo_and_unsupported_targets_fail_without_writes(self):
        before = self.snapshot()
        for args in ([], ["--demo", "wxDemo"], ["--demo", "../demo2"], ["--demo", "demo2", "--demo", "wxDemo"]):
            with self.subTest(args=args), contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as error:
                self.sync.main(args, self.repo)
            self.assertEqual(error.exception.code, 2)
        for demo in ("wxDemo", "../demo2"):
            with self.assertRaises(ValueError):
                self.sync.sync_demo(demo, self.repo)
        self.assertEqual(self.snapshot(), before)

    def test_check_mode_reports_drift_without_writing(self):
        before = self.snapshot()
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(self.sync.main(["--demo", "demo2", "--check"], self.repo), 1)
        self.assertEqual(self.snapshot(), before)
        self.sync.sync_demo("demo2", self.repo)
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(self.sync.main(["--demo", "demo2", "--check"], self.repo), 0)

    def test_malformed_or_ambiguous_xml_is_rejected_without_writing(self):
        fixtures = ["<UIDEF>",
                    INIT.replace("  </style>", "    " + POLICY_CLASS + "\n    " + POLICY_CLASS + "\n  </style>"),
                    INIT.replace('skin="custom"/>', 'skin="custom"/><listbox/>'),
                    INIT.replace("<style>", "<style/><!--").replace("</style>", "-->"),
                    INIT.replace("<listbox itemHeight", "<window itemHeight")]
        path = self.repo / "demos/demo2/uires/uidef/init.xml"
        for xml in fixtures:
            with self.subTest(xml=xml):
                path.write_bytes(xml.encode("utf-8"))
                before = self.snapshot()
                with self.assertRaises(ValueError):
                    self.sync.sync_demo("demo2", self.repo)
                self.assertEqual(self.snapshot(), before)

    def test_managed_values_are_read_from_the_shared_policy(self):
        source = SOURCE.replace('padding="8dp,0,8dp,0"', 'padding="12dp,0,12dp,0"')
        source = source.replace('text-x="8dp"', 'text-x="10dp"')
        self.put("demos/fluent/controls.xml", source.encode("utf-8"))
        self.sync.sync_demo("demo2", self.repo)
        root = ET.parse(self.repo / "demos/demo2/uires/uidef/init.xml").getroot()
        self.assertEqual(root.find("./style/class[@name='fluent_row_label']").get("padding"),
                         "12dp,0,12dp,0")
        self.assertEqual(root.find("./objattr/listbox").get("text-x"), "10dp")

    def test_both_explicit_targets_can_be_synced_and_rechecked(self):
        args = ["--demo", "demo2", "--demo", "wallpaper"]
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(self.sync.main(args, self.repo), 0)
            self.assertEqual(self.sync.main(args + ["--check"], self.repo), 0)


if __name__ == "__main__":
    unittest.main()
