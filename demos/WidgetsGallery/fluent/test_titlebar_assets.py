"""Check indexed titlebar SVGs against dpidemo; no GUI/visual claims."""

from pathlib import Path
import unittest
import xml.etree.ElementTree as ET


DEMOS = Path(__file__).resolve().parents[1]
TARGETS = (
    "flowlayout_demo", "framelayout_demo", "gridctrl_demo", "hellocharts_demo",
    "lightbutton_demo", "mclistview_demo", "propanimator_demo", "sci_demo",
    "wallpaper", "ipcdemo", "SoTool", "SOUIWidgetsGalleryDemo",
    "demo", "demo2", "wxDemo", "fun_test", "uieditor",
)
SKINS = {
    "minimize": "_skin.sys.btn.minimize",
    "maximize": "_skin.sys.btn.maximize",
    "restore": "_skin.sys.btn.restore",
    "close": "_skin.sys.btn.close",
    "theme": "fluent_btn_icon_theme",
    "menu": "fluent_btn_icon_menu",
}


def indexed_resources(demo):
    root = DEMOS / demo / "uires"
    return {
        (group.tag.lower(), entry.attrib["name"]):
        root / entry.attrib["path"].replace("\\", "/")
        for group in ET.parse(root / "uires.idx").getroot()
        for entry in group
    }


def titlebar_assets(demo, theme):
    resources = indexed_resources(demo)
    skin_file = resources[("values", "fluent_" + theme)]
    skins = {node.get("name"): node for node in ET.parse(skin_file).iter()
             if node.get("src")}
    assets = {}
    for role, name in SKINS.items():
        if demo == "uieditor":
            name = (name.replace("_skin.sys.", "ue_fluent_sys_").replace(".", "_")
                    if name.startswith("_skin.sys.") else "ue_" + name)
        if name not in skins:
            continue
        skin = skins[name]
        kind, resource = skin.attrib["src"].split(":", 1)
        assert kind.lower() == "svg", (demo, theme, name, kind)
        path = resources[(kind.lower(), resource)]
        assert path.suffix.lower() == ".svg", path
        assets[role] = (path, skin.get("states"))
    return assets


class TitlebarAssetsTest(unittest.TestCase):
    def test_reference_covers_both_themes(self):
        for theme in ("light", "dark"):
            assets = titlebar_assets("dpidemo", theme)
            self.assertEqual(set(assets), set(SKINS))
            for path, _ in assets.values():
                self.assertEqual(ET.parse(path).getroot().tag,
                                 "{http://www.w3.org/2000/svg}svg")

    def test_indexed_assets_match_reference(self):
        for demo in TARGETS:
            for theme in ("light", "dark"):
                with self.subTest(demo=demo, theme=theme):
                    assets = titlebar_assets(demo, theme)
                    self.assertTrue(set(SKINS) - {"menu"} <= set(assets))
                    reference = titlebar_assets("dpidemo", theme)
                    for role, (path, states) in assets.items():
                        with self.subTest(role=role, path=str(path)):
                            expected, expected_states = reference[role]
                            self.assertEqual(states, expected_states)
                            self.assertEqual(ET.parse(path).getroot().tag,
                                             "{http://www.w3.org/2000/svg}svg")
                            self.assertEqual(path.read_text(encoding="utf-8"),
                                             expected.read_text(encoding="utf-8"),
                                             str(path.relative_to(DEMOS)))


if __name__ == "__main__":
    for demo in TARGETS:
        for theme in ("light", "dark"):
            missing = set(SKINS) - set(titlebar_assets(demo, theme))
            if missing:
                print(f"Missing corresponding resources: {demo}/{theme}: "
                      + ", ".join(sorted(missing)))
    unittest.main()
