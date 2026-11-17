"""Registry-to-live-example coverage, and generated usage synchronization."""
import json
import re
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path

import build_catalog

DEMO = Path(__file__).resolve().parent
REPO = DEMO.parents[1]


def generated_sources(header):
    # build_catalog.py emits C++03-compatible wide literals; decode the JSON
    # escaping back into the XML text used by the assertions.
    return [json.loads(text) for text in re.findall(r'^        L("(?:[^"\\]|\\.)*"),$', header, re.MULTILINE)]


class CatalogContracts(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = json.loads((DEMO / "control_catalog.json").read_text(encoding="utf-8"))
        cls.main = ET.parse(DEMO / "uires/xml/gallery.xml").getroot()

    def test_all_actual_default_registrations_are_accounted_for_once(self):
        source = (REPO / "SOUI/src/SApp.cpp").read_text(encoding="utf-8-sig")
        section = source.split("void SObjectDefaultRegister::RegisterWindows")[1].split("void SObjectDefaultRegister::RegisterSkins")[0]
        registered = re.findall(r"TplRegisterFactory<(\w+)>", section)
        controls = self.catalog["controls"]
        self.assertEqual({c["cpp"] for c in controls}, set(registered))
        self.assertEqual(len(controls), len(registered))
        self.assertEqual(len({c["tag"].casefold() for c in controls}), len(controls))
        recipes = [c["cpp"] for c in controls if c["scope"] == "platform_recipe"]
        self.assertEqual(recipes, ["SActiveX"])
        for entry in controls:
            definition = (REPO / entry["definition"]).read_text(encoding="utf-8-sig")
            definition = re.sub(r"/\*.*?\*/|//[^\n]*", "", definition, flags=re.DOTALL)
            decl = re.search(r"class\s+(?:SOUI_EXP\s+)?" + entry["cpp"] + r"\b[^;{]*\{", definition)
            self.assertIsNotNone(decl, entry["cpp"])
            tag = re.search(r'DEF_SOBJECT\([^,]+,\s*L"([^"]+)"', definition[decl.end():decl.end() + 900])
            self.assertEqual(tag[1], entry["tag"])

    def test_every_category_has_real_navigation_page_and_type_usage(self):
        categories = self.catalog["categories"]
        pages = self.main.findall('.//tabctrl[@name="gallery_pages"]/page')
        self.assertEqual([n.get("name") for n in pages], ["page_" + c["key"] for c in categories])
        for i, category in enumerate(categories):
            nav = self.main.find(f'.//gallerybutton[@name="nav_{category["key"]}"]')
            self.assertEqual(nav.get("id"), str(4100 + i))
            self.assertEqual(nav.find("text").get("text"), category["title"])
        modal = ET.parse(DEMO / "uires/xml/gallery_modal.xml").getroot()
        native = ET.parse(DEMO / "uires/xml/gallery_native.xml").getroot()
        named = {n.get("name"): n for root in (self.main, modal, native) for n in root.iter() if n.get("name")}
        for entry in self.catalog["controls"]:
            self.assertIn(entry["sample"], named)
            if entry["scope"] == "platform_recipe":
                continue
            expected = {"STabCtrl": "gallerytabs", "SComboEdit": "editStyle", "SRebar": "galleryrebar"}.get(entry["cpp"], entry["tag"])
            self.assertEqual(named[entry["sample"]].tag, expected, entry["cpp"])
            self.assertTrue(entry["style"], entry["cpp"])

    def test_templates_have_real_fields_and_headers_use_compound_style_nodes(self):
        for sample in ("sample_listctrl", "sample_mclistview"):
            view = next(n for n in self.main.iter() if n.get("name") == sample)
            self.assertIsNotNone(view.find("headerStyle/items"))
            self.assertIsNone(view.find("header"))
        for tag in ("listview", "mclistview", "tileview", "treeview"):
            view = self.main.find(".//" + tag)
            template = view.find("template")
            self.assertIsNotNone(template)
            ids = {n.get("id") for n in template.iter()}
            self.assertTrue({"1", "2"}.issubset(ids), tag)
        mc = self.main.find(".//mclistview/template")
        self.assertTrue({"1", "2", "3"}.issubset({n.get("id") for n in mc}))

    def test_comboedit_usage_keeps_real_editable_combobox_context(self):
        entry = next(c for c in self.catalog["controls"] if c["cpp"] == "SComboEdit")
        edit_style = next(n for n in self.main.iter() if n.get("name") == entry["sample"])
        parent = next(n for n in self.main.iter() if edit_style in list(n))
        self.assertEqual(parent.tag, "combobox")
        self.assertEqual(parent.get("dropdown"), "0")

        header = build_catalog.outputs()[DEMO / "GalleryCatalog.h"]
        sources = generated_sources(header)
        category = next(i for i, c in enumerate(self.catalog["categories"]) if c["key"] == entry["category"])
        examples = ET.fromstring("<examples>" + sources[category] + "</examples>")
        generated = examples.find(f'combobox[@name="{parent.get("name")}"]')
        self.assertIsNotNone(generated, "SComboEdit usage must include its real editable combobox")
        self.assertIsNotNone(generated.find(f'editStyle[@name="{entry["sample"]}"]'))
        self.assertIsNone(examples.find(f'editStyle[@name="{entry["sample"]}"]'))
        self.assertEqual(
            ET.canonicalize(ET.tostring(generated, encoding="unicode"), strip_text=True),
            ET.canonicalize(ET.tostring(parent, encoding="unicode"), strip_text=True),
        )

    def test_generated_header_and_matrix_are_current(self):
        for path, expected in build_catalog.outputs().items():
            self.assertEqual(path.read_text(encoding="utf-8"), expected, path.name)
        sources = "\n\n".join(generated_sources((DEMO / "GalleryCatalog.h").read_text(encoding="utf-8")))
        self.assertIn('name="sample_listview"', sources)
        self.assertIn('<template itemHeight="48dp"', sources)
        self.assertIn('buddy="sample_number"', sources)
        self.assertNotIn('<window name="sample"', sources)

    def test_new_state_skins_and_popup_sources_have_both_theme_coverage(self):
        counts = {"gallery_toggle": 8, "gallery_switch_thumb": 8, "gallery_image_button": 4,
                  "gallery_spinner": 12, "gallery_toolbar_icons": 3, "gallery_sort": 2, "gallery_tree_expander": 8}
        for variant in ("light", "dark"):
            skins = ET.parse(DEMO / f"uires/values/fluent_{variant}.xml").getroot()
            for name, states in counts.items():
                self.assertEqual(skins.find(f'*[@name="{name}"]').get("states"), str(states))
                if states == 8:
                    self.assertEqual(skins.find(f'*[@name="{name}"]').get("checkAsPushdown"), "0")
        for name in ("gallery_actions", "gallery_view"):
            root = ET.parse(DEMO / f"uires/xml/{name}.xml").getroot()
            self.assertEqual(root.tag, "menuRoot")
            self.assertTrue(root.findall("menuItem"))


if __name__ == "__main__":
    unittest.main()
