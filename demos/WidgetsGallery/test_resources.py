"""Offline contracts for gallery resources; runtime control checks live in fun_test.

Run directly with Python 3. No image libraries or generated snapshots are needed.
"""

from __future__ import annotations

import math
import re
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path

UIRES = Path(__file__).resolve().parent / "uires"
SKIN_ATTRIBUTES = {
    "skin", "ncskin", "tabskin", "btnskin", "focusskin", "itemskin", "checkskin",
    "sepskin", "arrowskin", "headerskin", "sbskin", "bandskin", "bkgndskin",
    "trackskin", "posskin", "filledskin", "iconskin", "toggleskin", "branchskin",
    "captionskin", "closebtnskin", "thumbskin", "lineskin",
    "skinforce", "prevskin", "nextskin", "dayskin", "weekskin", "upskin", "downskin",
    "sortskin", "dockbtnskin", "dropArrowSkin".casefold(),
}
COLOR_REFERENCE = re.compile(r"@color/([A-Za-z0-9_.-]+)")
SVG_NS = "http://www.w3.org/2000/svg"
REQUIRED_ENTRIES = {
    ("layout", "gallery"): "xml/gallery.xml",
    ("layout", "gallery_dialog"): "xml/dialog.xml",
    ("uidef", "xml_init"): "uidef/init.xml",
    ("values", "fluent_light"): "values/fluent_light.xml",
    ("values", "fluent_dark"): "values/fluent_dark.xml",
    ("values", "theme_color_light"): "values/theme_color_light.xml",
    ("values", "theme_color_dark"): "values/theme_color_dark.xml",
}


def indexed_resources() -> dict[tuple[str, str], Path]:
    entries: dict[tuple[str, str], Path] = {}
    root = ET.parse(UIRES / "uires.idx").getroot()
    for group in root:
        for entry in group:
            name, relative = entry.get("name", ""), entry.get("path", "")
            if not name or not relative:
                raise AssertionError(f"Unnamed or pathless index entry in {group.tag}")
            key = (group.tag.casefold(), name.casefold())
            if key in entries:
                raise AssertionError(f"Duplicate resource ID: {key}")
            path = (UIRES / relative.replace("\\", "/")).resolve()
            if not path.is_relative_to(UIRES.resolve()):
                raise AssertionError(f"Resource path escapes gallery uires: {relative}")
            if not path.is_file():
                raise AssertionError(f"Missing indexed resource {key}: {path}")
            entries[key] = path
    if not entries:
        raise AssertionError("Resource index is empty")
    return entries


def unique_named_nodes(root: ET.Element) -> dict[str, ET.Element]:
    named: dict[str, ET.Element] = {}
    for node in root:
        name = node.get("name", "")
        if not name:
            raise AssertionError(f"Unnamed {node.tag} definition")
        if name in named:
            raise AssertionError(f"Duplicate definition: {name}")
        named[name] = node
    return named


def svg_extent(root: ET.Element) -> tuple[float, float]:
    def dimension(value: str | None) -> float:
        if value is None:
            raise AssertionError("SVG width and height must be explicit")
        match = re.fullmatch(r"([0-9]+(?:\.[0-9]+)?)(?:px)?", value)
        if not match:
            raise AssertionError(f"Unsupported SVG extent: {value}")
        result = float(match.group(1))
        if not math.isfinite(result) or result <= 0:
            raise AssertionError(f"Invalid SVG extent: {value}")
        return result

    return dimension(root.get("width")), dimension(root.get("height"))


class GalleryResourceContracts(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.entries = indexed_resources()
        cls.skins: dict[str, dict[str, ET.Element]] = {}
        cls.tokens: dict[str, dict[str, str]] = {}
        for variant in ("light", "dark"):
            skin_root = ET.parse(cls.entries[("values", f"fluent_{variant}")]).getroot()
            if skin_root.tag != "skin":
                raise AssertionError(f"Missing skin root in {variant} theme")
            cls.skins[variant] = unique_named_nodes(skin_root)
            colors = ET.parse(cls.entries[("values", f"theme_color_{variant}")]).getroot().find("color")
            if colors is None:
                raise AssertionError(f"Missing color definitions in {variant} theme")
            tokens: dict[str, str] = {}
            for node in colors:
                if node.tag in tokens:
                    raise AssertionError(f"Duplicate {variant} color token: {node.tag}")
                value = node.get("value", "")
                if not value:
                    raise AssertionError(f"Empty {variant} color token: {node.tag}")
                tokens[node.tag] = value
            cls.tokens[variant] = tokens

    def test_index_contains_entry_points_and_every_xml_parses(self) -> None:
        for key, relative in REQUIRED_ENTRIES.items():
            with self.subTest(resource=key):
                self.assertIn(key, self.entries)
                self.assertEqual(self.entries[key], (UIRES / relative).resolve())
        for key, path in self.entries.items():
            if path.suffix.casefold() == ".xml":
                with self.subTest(resource=key):
                    ET.parse(path)

    def test_svg_resources_are_real_local_vector_documents(self) -> None:
        svg_resources = {key: path for key, path in self.entries.items() if key[0] == "svg"}
        self.assertTrue(svg_resources, "The gallery must ship vector resources")
        visible_primitives = {"path", "rect", "circle", "ellipse", "line", "polygon", "polyline", "text"}
        for key, path in svg_resources.items():
            with self.subTest(resource=key):
                root = ET.parse(path).getroot()
                self.assertEqual(root.tag, f"{{{SVG_NS}}}svg")
                svg_extent(root)
                view_box = [float(value) for value in root.get("viewBox", "").replace(",", " ").split()]
                self.assertEqual(len(view_box), 4)
                self.assertTrue(all(math.isfinite(value) for value in view_box))
                self.assertGreater(view_box[2], 0)
                self.assertGreater(view_box[3], 0)
                nodes = list(root.iter())
                self.assertTrue(any(node.tag.rsplit("}", 1)[-1] in visible_primitives for node in nodes))
                for node in nodes:
                    tag = node.tag.rsplit("}", 1)[-1]
                    self.assertNotIn(tag, {"script", "foreignObject", "image"})
                    for attribute, value in node.attrib.items():
                        if attribute.rsplit("}", 1)[-1] == "href":
                            self.assertTrue(value.startswith("#"), "SVG assets must not depend on remote files")

    def test_both_themes_share_skin_and_token_names_but_not_surface_colors(self) -> None:
        self.assertTrue(self.skins["light"])
        self.assertTrue(self.tokens["light"])
        self.assertEqual(set(self.skins["light"]), set(self.skins["dark"]))
        self.assertEqual(set(self.tokens["light"]), set(self.tokens["dark"]))
        for token in ("fluent_window_bg", "fluent_card_bg", "fluent_text", "fluent_text_secondary"):
            with self.subTest(token=token):
                self.assertIn(token, self.tokens["light"])
                self.assertIn(token, self.tokens["dark"])
                self.assertNotEqual(self.tokens["light"][token], self.tokens["dark"][token])
        # SOUI-created dropdowns and RichEdit menus also use the active palette.
        for token in ("thm_color", "thm_border", "thm_txt_normal", "thm_txt_hover", "thm_txt_push", "thm_txt_disable"):
            self.assertIn(token, self.tokens["light"])
            self.assertIn(token, self.tokens["dark"])

    def test_layout_and_default_style_references_resolve_in_both_palettes(self) -> None:
        init = ET.parse(self.entries[("uidef", "xml_init")]).getroot()
        style_root = init.find("style")
        self.assertIsNotNone(style_root)
        styles = unique_named_nodes(style_root)
        skin_references: set[str] = set()
        color_references: set[str] = set()
        documents = [init]
        for key, path in self.entries.items():
            if key[0] in {"layout", "smenu"}:
                documents.append(ET.parse(path).getroot())
        for document in documents:
            # Host-private styles override defaults only within their SOUI
            # layout. A style in gallery.xml must not leak into dialog.xml.
            scoped_styles = dict(styles)
            private_styles = document.find("style")
            if document is not init and private_styles is not None:
                scoped_styles.update(unique_named_nodes(private_styles))
            for node in document.iter():
                for attribute, value in node.attrib.items():
                    if attribute.casefold() in SKIN_ATTRIBUTES and value.strip():
                        skin_references.add(value.strip())
                    color_references.update(COLOR_REFERENCE.findall(value))
                if node.text:
                    color_references.update(COLOR_REFERENCE.findall(node.text))
                if node.get("class"):
                    self.assertIn(node.get("class"), scoped_styles, f"Undefined style on {node.tag}: {node.get('name')}")
        self.assertTrue(skin_references)
        self.assertTrue(color_references)
        for variant in ("light", "dark"):
            with self.subTest(theme=variant):
                self.assertFalse(skin_references - set(self.skins[variant]), "Missing themed skins")
                self.assertFalse(color_references - set(self.tokens[variant]), "Missing semantic color tokens")

    def test_all_skin_sources_are_indexed_and_have_compatible_state_atlases(self) -> None:
        for variant, skins in self.skins.items():
            for name, skin in skins.items():
                source = skin.get("src")
                if not source:
                    continue
                with self.subTest(theme=variant, skin=name):
                    resource_type, separator, resource_name = source.partition(":")
                    self.assertTrue(separator, "Skin source must be a resource ID")
                    key = (resource_type.casefold(), resource_name.casefold())
                    self.assertIn(key, self.entries)
                    if key[0] != "svg":
                        continue
                    svg = ET.parse(self.entries[key]).getroot()
                    width, height = svg_extent(svg)
                    states = int(skin.get("states", "1"))
                    self.assertGreater(states, 0)
                    extent = height if skin.get("vertical", "0").casefold() in {"1", "true"} else width
                    self.assertGreaterEqual(extent / states, 1)
                    self.assertAlmostEqual(extent / states, round(extent / states), msg="State cells must not divide a vector pixel")

    def test_combo_dropdowns_request_translucency_at_creation(self) -> None:
        # The fluent chrome paints a per-pixel-alpha shadow around combo dropdowns.
        # Translucency has to be requested in the init XML because Linux cannot
        # switch an existing window to an ARGB visual when SetTranslucent runs later.
        found = 0
        for key, path in self.entries.items():
            if key[0] != "layout":
                continue
            for combo in ET.parse(path).getroot().iter():
                if combo.tag not in {"combobox", "comboview"}:
                    continue
                for node in combo.findall("dropdownStyle"):
                    found += 1
                    with self.subTest(resource=key, name=combo.get("name")):
                        self.assertIn(node.get("translucent", "").casefold(), {"1", "true"})
        self.assertTrue(found, "The gallery must exercise combo dropdowns")

    def test_primary_secondary_and_navigation_offer_all_four_interaction_states(self) -> None:
        for variant in ("light", "dark"):
            for name in ("fluent_btn_primary", "fluent_btn_secondary", "gallery_nav", "gallery_nav_selected"):
                with self.subTest(theme=variant, skin=name):
                    self.assertIn(name, self.skins[variant])
                    skin = self.skins[variant][name]
                    self.assertEqual(skin.get("states"), "4", "normal/hover/pressed/disabled must all be authored")
                    if skin.tag == "colorrect":
                        self.assertTrue(all(skin.get(state) for state in ("normal", "hover", "push", "disable")))
                    else:
                        self.assertTrue(skin.get("src"))


class ResourceBuilderTests(unittest.TestCase):
    def test_stamp_only_updates_do_not_change_semantic_payload(self):
        from build_resources import resource_payload

        first = b"\xef\xbb\xbf//stamp:0000000000000001\r\nint sample = 4100;\r\n"
        second = first.replace(b"0000000000000001", b"0000000000000002")
        self.assertEqual(resource_payload(first), resource_payload(second))
        self.assertNotEqual(resource_payload(first), resource_payload(second.replace(b"4100", b"4101")))
        self.assertTrue(resource_payload(first).startswith(b"\xef\xbb\xbf"))


if __name__ == "__main__":
    unittest.main(verbosity=2)
