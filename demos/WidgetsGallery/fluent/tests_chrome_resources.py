#!/usr/bin/env python3
"""Offline/additive chrome regressions, not runtime or GUI integration tests.

Acceptance/plan: first fail on the absent generator/assets, then reuse the
current dpidemo renderer, append only chrome skins/PNG indices, and verify all
13 palettes retain their private entries. Pixel checks cover both themes,
transparent perimeter, all four gutter profiles and panel fill/border. Temp
fixtures additionally cover custom colors, encoding/BOM/CRLF, conflict
preflight, exact additive edits, deterministic/no-op reruns and write scope.
Run: python -B demos/fluent/tests_chrome_resources.py
"""

from collections import Counter
import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import xml.etree.ElementTree as ET

from PIL import Image, ImageColor

REPO = Path(__file__).resolve().parents[2]
SCRIPT = Path(__file__).with_name("generate_chrome_resources.py")
REFERENCE = REPO / "demos/dpidemo/generate_popup_shadow.py"
TARGETS = (
    "demo", "demo2", "wxDemo", "flowlayout_demo", "framelayout_demo",
    "gridctrl_demo", "hellocharts_demo", "lightbutton_demo", "mclistview_demo",
    "propanimator_demo", "sci_demo", "wallpaper", "ipcdemo",
)
WINDOW_TARGETS = {
    "demo", "wxDemo", "flowlayout_demo", "framelayout_demo", "hellocharts_demo",
    "mclistview_demo", "propanimator_demo", "wallpaper", "ipcdemo",
}
# Generator fixtures keep their original scope; shipped hosts gained layered
# chrome independently. Do not infer permission from whatever files exist.
SHIPPED_WINDOW_TARGETS = WINDOW_TARGETS | {"demo2", "gridctrl_demo", "sci_demo"}
WINDOW_ALPHAS = {"light": (0.30, 0.08), "dark": (0.34, 0.10)}
MARGIN = "26,20,26,32"
PALETTE = ("<?xml version='1.0' encoding='utf-8'?>\r\n<skin>\r\n"
           "\t<!-- 私有颜色; <imgframe name='fluent_popup_shadow'/>; </skin> -->\r\n"
           "\t<shape name='custom_card' color='#174B79' cornerRadius='11' />\r\n"
           "\t<imglist name='private_toggle' src='SVG:PRIVATE' states='2'>\r\n"
           "\t\t<states><state index='0' value='normal'/></states>\r\n"
           "\t</imglist>\r\n</skin>\r\n<!-- keep trailer </skin> -->\r\n")
INDEX = ("<?xml version='1.0' encoding='utf-8'?>\r\n<resource>\r\n"
         "\t<values><file name='CUSTOM' path='values\\private.xml'/></values>\r\n"
         "\t<pNg>\r\n\t\t<!-- 私有索引; </pNg> -->\r\n"
         "\t\t<file name='PRIVATE' path='image\\private.png'/>\r\n"
         "\t</pNg>\r\n</resource>\r\n<!-- keep trailer </resource> -->\r\n")
NO_PNG_INDEX = ("<?xml version='1.0' encoding='utf-8'?>\r\n<resource>\r\n"
                "\t<values><file name='CUSTOM' path='values\\private.xml'/></values>\r\n"
                "</resource>\r\n")
CUSTOM_COLORS = {
    "light": {"fluent_window_bg": "#A7C3D9", "fluent_card_bg": "#E1DAC5",
              "fluent_border": "#3F7F9F"},
    "dark": {"fluent_window_bg": "#18212B", "fluent_card_bg": "#354453",
             "fluent_border": "#6280A0"},
}


def load_script(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def kinds(demo, window_targets=WINDOW_TARGETS):
    return ("window", "popup") if demo in window_targets else ("popup",)


def resource_name(kind, theme):
    return f"FL_CHROME_{kind.upper()}_{theme.upper()}"


def resource_path(uires, kind, theme):
    return uires / "image/fluent_chrome" / (resource_name(kind, theme) + ".png")


def shipped_resource(demo, kind, theme):
    if demo == "demo2" and kind == "window":
        return f"D2_WINDOW_{theme.upper()}", f"image\\window_{theme}.png"
    name = resource_name(kind, theme)
    return name, f"image\\fluent_chrome\\{name}.png"


def skin_attrs(kind, theme):
    return {"name": f"fluent_{kind}_shadow",
            "src": "PNG:" + resource_name(kind, theme), "states": "1",
            "margin": MARGIN, "enableColorize": "0"}


def colors_at(uires, theme):
    root = ET.parse(uires / "values" / f"theme_color_{theme}.xml").getroot()
    return {node.tag: node.get("value") for node in root.find("color")}


def profiles(image, reference):
    alpha = image.getchannel("A")
    left, top, right, bottom = reference.PADDING
    x, y = left + reference.PANEL_SIZE // 2, top + reference.PANEL_SIZE // 2
    return {
        "left": [alpha.getpixel((i, y)) for i in range(left - 2)],
        "right": [alpha.getpixel((image.width - 1 - i, y)) for i in range(right - 2)],
        "top": [alpha.getpixel((x, i)) for i in range(top - 2)],
        "bottom": [alpha.getpixel((x, image.height - 1 - i)) for i in range(bottom - 2)],
    }


class ChromeAssertions:
    def assert_single_insertion(self, before, after):
        if before == after:
            return
        prefix = 0
        while prefix < len(before) and before[prefix] == after[prefix]:
            prefix += 1
        suffix = before[prefix:]
        self.assertGreater(len(after), len(before))
        self.assertTrue(after.endswith(suffix), "Original bytes were replaced, not appended")
        self.assertEqual(after[:prefix] + (after[-len(suffix):] if suffix else b""), before)

    def assert_panel(self, image, colors, kind, reference):
        self.assertEqual(image.mode, "RGBA")
        self.assertEqual(image.size, (100, 100))
        left, top, _, _ = reference.PADDING
        panel = reference.PANEL_SIZE
        x, y = left + panel // 2, top + panel // 2
        fill_key = "fluent_window_bg" if kind == "window" else "fluent_card_bg"
        self.assertEqual(image.getpixel((x, y)), (*ImageColor.getrgb(colors[fill_key]), 255))
        border = ImageColor.getrgb(colors["fluent_border"])
        # One-pixel border is antialiased by the reference's 4x Lanczos reduction.
        for point in ((left, y), (left + panel - 1, y), (x, top), (x, top + panel - 1)):
            rgba = image.getpixel(point)
            self.assertGreaterEqual(rgba[3], 235)
            self.assertLessEqual(max(abs(a - b) for a, b in zip(rgba[:3], border)), 20)

    def assert_reference_pixels(self, path, colors, kind, theme, reference):
        self.assertTrue(path.is_file(), str(path))
        alphas = WINDOW_ALPHAS[theme] if kind == "window" else reference.THEMES[theme][2:]
        fill_key = "fluent_window_bg" if kind == "window" else "fluent_card_bg"
        expected = reference.render_popup(colors[fill_key], colors["fluent_border"],
                                          *alphas, window=True)
        with Image.open(path) as image:
            self.assert_panel(image, colors, kind, reference)
            self.assertEqual(image.tobytes(), expected.tobytes(), "Not current dpidemo renderer")


class RepositoryChromeTests(ChromeAssertions, unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reference = load_script("chrome_reference_repository", REFERENCE)

    def assets(self):
        for demo in TARGETS:
            uires = REPO / "demos" / demo / "uires"
            for theme in ("light", "dark"):
                for kind in kinds(demo, SHIPPED_WINDOW_TARGETS):
                    _, relative = shipped_resource(demo, kind, theme)
                    yield demo, uires, theme, kind, uires / relative.replace('\\', '/')

    def test_exact_target_lists_and_current_window_alphas(self):
        generator = load_script("chrome_generator_contract", SCRIPT)
        self.assertEqual(set(generator.TARGETS), set(TARGETS))
        self.assertEqual(set(generator.WINDOW_TARGETS), WINDOW_TARGETS)
        self.assertEqual(generator.WINDOW_ALPHAS, WINDOW_ALPHAS)
        source_alphas = {theme: tuple(values[2:]) for theme, values in self.reference.THEMES.items()}
        self.assertEqual(generator.WINDOW_ALPHAS, source_alphas)
        self.assertEqual(self.reference.PADDING, (18, 12, 18, 24))
        self.assertEqual(self.reference.PANEL_SIZE, 64)
        self.assertEqual(self.reference.RADIUS, 8)

    def test_paired_skins_indices_and_popup_only_targets(self):
        for demo in TARGETS:
            uires = REPO / "demos" / demo / "uires"
            idx = ET.parse(uires / "uires.idx").getroot()
            png_groups = [n for n in idx if n.tag.casefold() == "png"]
            self.assertEqual(len(png_groups), 1, demo)
            png = png_groups[0]
            for theme in ("light", "dark"):
                pool = ET.parse(uires / "values" / f"fluent_{theme}.xml").getroot()
                for kind in kinds(demo, SHIPPED_WINDOW_TARGETS):
                    name, relative = shipped_resource(demo, kind, theme)
                    nodes = [n for n in pool if n.get("name") == f"fluent_{kind}_shadow"]
                    self.assertEqual(len(nodes), 1, (demo, theme, kind))
                    self.assertEqual(nodes[0].tag, "imgframe")
                    expected = skin_attrs(kind, theme)
                    expected['src'] = 'PNG:' + name
                    self.assertEqual(nodes[0].attrib, expected)
                    entries = [n for n in png if n.get("name") == name]
                    self.assertEqual(len(entries), 1, (demo, theme, kind))
                    self.assertEqual(entries[0].get("path"), relative)
                if demo not in SHIPPED_WINDOW_TARGETS:
                    self.assertFalse(any(n.get("name") == "fluent_window_shadow" for n in pool), demo)
                    self.assertFalse(resource_path(uires, "window", theme).exists(), demo)
                    self.assertFalse(any(n.get("name") == resource_name("window", theme) for n in png), demo)

    def test_all_50_assets_match_current_renderer_and_semantic_panel_colors(self):
        assets = list(self.assets())
        self.assertEqual(len(assets), 50)
        for demo, uires, theme, kind, path in assets:
            with self.subTest(demo=demo, theme=theme, kind=kind):
                self.assert_reference_pixels(path, colors_at(uires, theme), kind, theme, self.reference)

    def test_transparent_outer_perimeter_and_all_four_gutter_profiles(self):
        for demo, _, theme, kind, path in self.assets():
            with self.subTest(demo=demo, theme=theme, kind=kind):
                self.assertTrue(path.is_file(), str(path))
                with Image.open(path) as image:
                    alpha = image.getchannel("A")
                    for edge in ((0, 0, 100, 1), (0, 99, 100, 100),
                                 (0, 0, 1, 100), (99, 0, 100, 100)):
                        self.assertEqual(alpha.crop(edge).getextrema(), (0, 0))
                    for side, profile in profiles(image, self.reference).items():
                        with self.subTest(side=side):
                            self.assertEqual(profile[0], 0)
                            self.assertLessEqual(profile[1], 1)
                            self.assertTrue(all(0 <= b - a <= 10 for a, b in zip(profile, profile[1:])))
                            self.assertGreaterEqual(len(set(profile)), 5)
                            self.assertLessEqual(max(profile[:max(1, len(profile) - 10)]), 2)


class GeneratorTests(ChromeAssertions, unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.generator = load_script("chrome_generator_fixtures", SCRIPT)
        cls.reference = load_script("chrome_reference_fixtures", REFERENCE)

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.repo = Path(self.temp.name).resolve()
        for demo in TARGETS:
            for theme in ("light", "dark"):
                self.put(demo, f"values/fluent_{theme}.xml", PALETTE.encode("utf-8"))
                colors = "".join(f'<{key} value="{value}"/>' for key, value in CUSTOM_COLORS[theme].items())
                self.put(demo, f"values/theme_color_{theme}.xml",
                         f'<UIDEF><color>{colors}<private_color value="#135791"/></color></UIDEF>'.encode())
            self.put(demo, "uires.idx", (INDEX if demo == "wxDemo" else NO_PNG_INDEX).encode("utf-8"))
            for protected in ("xml/dlg_main.xml", "uidef/init.xml", "values/private.xml",
                              "image/private.png"):
                self.put(demo, protected, b"unmanaged bytes; do not modify\r\n")
        for protected in ("demos/fluent/controls.xml", "demos/fluent/FluentTheme.h",
                          "demos/demo/CMakeLists.txt", "demos/demo/MainDlg.cpp",
                          "demos/demo/res/resource.h", "demos/fluent/README.md"):
            path = self.repo / protected
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b"protected\r\n")

    def uires(self, demo="wxDemo"):
        return self.repo / "demos" / demo / "uires"

    def put(self, demo, relative, data):
        path = self.uires(demo) / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        return path

    def snapshot(self):
        return {path.relative_to(self.repo): path.read_bytes()
                for path in self.repo.rglob("*") if path.is_file()}

    def run_generator(self, demos=("wxDemo",), check=False):
        return self.generator.generate(demos, repo=self.repo, check=check)

    def test_default_supplies_44_assets_and_only_adds_authorized_resources(self):
        before = self.snapshot()
        changed = self.generator.generate(repo=self.repo)
        after = self.snapshot()
        self.assertEqual(len(changed), 83)  # 44 PNG + 26 palettes + 13 indices.
        self.assertEqual(len(set(after) - set(before)), 44)
        for relative, original in before.items():
            with self.subTest(path=relative):
                if relative.name in ("fluent_light.xml", "fluent_dark.xml", "uires.idx"):
                    self.assert_single_insertion(original, after[relative])
                else:
                    self.assertEqual(after[relative], original)
        for demo in TARGETS:
            for theme in ("light", "dark"):
                for kind in ("window", "popup"):
                    self.assertEqual(resource_path(self.uires(demo), kind, theme).exists(), kind in kinds(demo))

    def test_explicit_target_cannot_modify_other_demos(self):
        before = self.snapshot()
        self.run_generator()
        after = self.snapshot()
        for relative, original in before.items():
            if relative.parts[1] != "wxDemo":
                self.assertEqual(after[relative], original, str(relative))
        self.assertTrue(all(relative.parts[1] == "wxDemo" for relative in set(after) - set(before)))

    def test_custom_semantic_colors_drive_png_not_reference_or_private_skin_colors(self):
        self.run_generator()
        for theme in ("light", "dark"):
            for kind in kinds("wxDemo"):
                self.assert_reference_pixels(resource_path(self.uires(), kind, theme), CUSTOM_COLORS[theme],
                                             kind, theme, self.reference)

    def test_rerun_is_byte_deterministic_and_does_not_touch_mtimes(self):
        self.run_generator()
        before = self.snapshot()
        mtimes = {p: p.stat().st_mtime_ns for p in self.repo.rglob("*") if p.is_file()}
        self.assertEqual(self.run_generator(), [])
        self.assertEqual(self.snapshot(), before)
        self.assertEqual({p: p.stat().st_mtime_ns for p in mtimes}, mtimes)

    def test_private_palette_and_index_entries_survive_verbatim_with_comments(self):
        before = self.snapshot()
        self.run_generator()
        for relative in ("values/fluent_light.xml", "values/fluent_dark.xml", "uires.idx"):
            path = self.uires() / relative
            self.assert_single_insertion(before[path.relative_to(self.repo)], path.read_bytes())
        for theme in ("light", "dark"):
            root = ET.parse(self.uires() / "values" / f"fluent_{theme}.xml").getroot()
            self.assertEqual(root.find("shape").get("color"), "#174B79")
            self.assertEqual(len(root.find("imglist")), 1)
        idx = ET.parse(self.uires() / "uires.idx").getroot()
        self.assertEqual(idx.find("pNg/file").attrib, {"name": "PRIVATE", "path": "image\\private.png"})

    def test_preserves_utf8_bom_utf16_both_endians_and_gbk_with_crlf(self):
        for encoding, bom in (("utf-8", b"\xef\xbb\xbf"),
                              ("utf-16-le", b"\xff\xfe"), ("utf-16-be", b"\xfe\xff"),
                              ("gbk", b"")):
            with self.subTest(encoding=encoding):
                declaration = "utf-16" if encoding.startswith("utf-16") else encoding
                for relative, source in (("values/fluent_light.xml", PALETTE),
                                         ("values/fluent_dark.xml", PALETTE), ("uires.idx", INDEX)):
                    data = bom + source.replace("encoding='utf-8'", f"encoding='{declaration}'").encode(encoding)
                    self.put("wxDemo", relative, data)
                before = self.snapshot()
                self.run_generator()
                for relative in ("values/fluent_light.xml", "values/fluent_dark.xml", "uires.idx"):
                    path = self.uires() / relative
                    data = path.read_bytes()
                    self.assert_single_insertion(before[path.relative_to(self.repo)], data)
                    self.assertTrue(data.startswith(bom))
                    text = data[len(bom):].decode(encoding)
                    self.assertNotIn("\n", text.replace("\r\n", ""))
                    ET.fromstring(text)
                self.assertEqual(self.run_generator(), [])

    def test_mixed_existing_newlines_are_not_normalized(self):
        data = PALETTE.replace("</skin>\r\n", "</skin>\n", 1).encode()
        path = self.put("wxDemo", "values/fluent_light.xml", data)
        self.run_generator()
        self.assert_single_insertion(data, path.read_bytes())
        self.assertTrue(path.read_bytes().endswith(b"</skin>\n<!-- keep trailer </skin> -->\r\n"))

    def test_compact_and_self_closing_png_groups_keep_private_entries(self):
        data = b'<resource><values><file name="PRIVATE" path="values\\private.xml"/></values><PNG /></resource>'
        path = self.put("wxDemo", "uires.idx", data)
        self.run_generator()
        root = ET.parse(path).getroot()
        self.assertEqual(root.find("values/file").get("name"), "PRIVATE")
        self.assertEqual(len(root.findall("PNG")), 1)
        self.assertEqual(len(root.find("PNG")), 4)
        self.assertTrue(path.read_bytes().startswith(data.split(b"<PNG")[0]))
        self.assertEqual(self.run_generator(), [])

    def test_check_reports_differences_but_never_writes(self):
        before = self.snapshot()
        self.assertEqual(len(self.run_generator(check=True)), 7)
        self.assertEqual(self.snapshot(), before)
        self.run_generator()
        self.assertEqual(self.run_generator(check=True), [])

    def test_missing_or_invalid_theme_color_fails_before_any_write(self):
        path = self.uires("demo2") / "values/theme_color_dark.xml"
        original = path.read_bytes()
        for data in (original.replace(b'<fluent_border value="#6280A0"/>', b""),
                     original.replace(b"#6280A0", b"not-a-color")):
            path.write_bytes(data)
            before = self.snapshot()
            with self.assertRaises(ValueError):
                self.run_generator(("wxDemo", "demo2"))
            self.assertEqual(self.snapshot(), before)

    def test_conflicting_existing_shadow_skin_fails_instead_of_replacing_it(self):
        data = PALETTE.replace("</skin>\r\n", '<shape name="fluent_popup_shadow" color="#001122"/>\r\n</skin>\r\n', 1)
        self.put("demo2", "values/fluent_dark.xml", data.encode())
        before = self.snapshot()
        with self.assertRaises(ValueError):
            self.run_generator(("wxDemo", "demo2"))
        self.assertEqual(self.snapshot(), before)

    def test_conflicting_existing_png_index_fails_instead_of_retargeting_it(self):
        data = INDEX.replace("\t</pNg>\r\n", '\t\t<file name="FL_CHROME_POPUP_LIGHT" path="image\\custom.png"/>\r\n\t</pNg>\r\n', 1)
        self.put("wxDemo", "uires.idx", data.encode())
        before = self.snapshot()
        with self.assertRaises(ValueError):
            self.run_generator()
        self.assertEqual(self.snapshot(), before)

    def test_duplicate_chrome_skin_or_png_group_fails_without_deleting_entries(self):
        for relative, data in (("values/fluent_light.xml", PALETTE.replace(
                "</skin>\r\n", '<imgframe name="fluent_popup_shadow"/><imgframe name="fluent_popup_shadow"/></skin>\r\n', 1)),
                               ("uires.idx", INDEX.replace("</resource>\r\n", "<PNG/></resource>\r\n", 1))):
            with self.subTest(path=relative):
                path = self.put("wxDemo", relative, data.encode())
                before = self.snapshot()
                with self.assertRaises(ValueError):
                    self.run_generator()
                self.assertEqual(self.snapshot(), before)
                path.write_bytes((PALETTE if relative.startswith("values/") else INDEX).encode())

    def test_new_private_entries_and_custom_colors_survive_future_reruns(self):
        self.run_generator()
        for theme in ("light", "dark"):
            path = self.uires() / "values" / f"fluent_{theme}.xml"
            path.write_bytes(path.read_bytes().replace(b"</skin>\r\n", b'<shape name="later_private" color="#AB0123"/></skin>\r\n', 1))
        before = self.snapshot()
        self.assertEqual(self.run_generator(), [])
        self.assertEqual(self.snapshot(), before)

    def test_theme_color_change_updates_only_owned_pngs_not_palette_or_index(self):
        self.run_generator()
        path = self.uires() / "values/theme_color_light.xml"
        path.write_bytes(path.read_bytes().replace(b"#A7C3D9", b"#91AABB"))
        before = self.snapshot()
        changed = self.run_generator()
        self.assertEqual(changed, [resource_path(self.uires(), "window", "light")])
        after = self.snapshot()
        for relative, data in before.items():
            if relative != changed[0].relative_to(self.repo):
                self.assertEqual(after[relative], data)
        colors = dict(CUSTOM_COLORS["light"], fluent_window_bg="#91AABB")
        self.assert_reference_pixels(changed[0], colors, "window", "light", self.reference)

    def test_unsupported_or_path_traversal_targets_fail_without_writes(self):
        before = self.snapshot()
        for target in ("dpidemo", "../demo", "demo/../../", ""):
            with self.subTest(target=target), self.assertRaises(ValueError):
                self.run_generator((target,))
        self.assertEqual(self.snapshot(), before)

    def test_concurrent_palette_edit_is_not_overwritten_from_a_stale_plan(self):
        renderer = self.generator._renderer()
        render = renderer.render_popup
        calls = 0
        path = self.uires() / "values/fluent_light.xml"
        before = self.snapshot()
        concurrent = PALETTE.replace("</skin>\r\n", '<shape name="other_agent" color="#012ABC"/></skin>\r\n', 1).encode()

        def concurrent_render(*args, **kwargs):
            nonlocal calls
            calls += 1
            if calls == 3:  # Light palette is already planned; dark PNG is rendering.
                path.write_bytes(concurrent)
            return render(*args, **kwargs)

        with patch.object(renderer, "render_popup", side_effect=concurrent_render):
            with self.assertRaisesRegex(ValueError, "changed during planning"):
                self.run_generator()
        expected = dict(before)
        expected[path.relative_to(self.repo)] = concurrent
        self.assertEqual(self.snapshot(), expected)

    def test_current_real_palettes_keep_every_existing_entry_in_temp_copy(self):
        for demo in TARGETS:
            source = REPO / "demos" / demo / "uires"
            for relative in ("uires.idx", "values/fluent_light.xml", "values/fluent_dark.xml",
                             "values/theme_color_light.xml", "values/theme_color_dark.xml"):
                self.put(demo, relative, (source / relative).read_bytes())
        before = self.snapshot()
        self.generator.generate(repo=self.repo)
        after = self.snapshot()
        for demo in TARGETS:
            for relative in ("uires.idx", "values/fluent_light.xml", "values/fluent_dark.xml"):
                path = (self.uires(demo) / relative).relative_to(self.repo)
                self.assert_single_insertion(before[path], after[path])
                old = ET.fromstring(before[path])
                new = ET.fromstring(after[path])
                if relative == "uires.idx":
                    existing = Counter((g.tag, tuple(n.attrib.items())) for g in old for n in g)
                    actual = Counter((g.tag, tuple(n.attrib.items())) for g in new for n in g)
                else:
                    existing = Counter(ET.tostring(n, encoding="unicode").strip() for n in old)
                    actual = Counter(ET.tostring(n, encoding="unicode").strip() for n in new)
                self.assertFalse(existing - actual, (demo, relative, existing - actual))
        self.assertEqual(self.generator.generate(repo=self.repo), [])
        self.assertEqual(self.snapshot(), after)


if __name__ == "__main__":
    unittest.main(verbosity=2)
