"""All shipped demo shadows must match dpidemo, not a generator's old recipe."""
import unittest
import xml.etree.ElementTree as ET
from itertools import pairwise
from pathlib import Path

from PIL import Image, ImageColor

REPO = Path(__file__).resolve().parents[2]
DEMOS = (
    "demo", "demo2", "wxDemo", "dpidemo", "flowlayout_demo", "framelayout_demo",
    "gridctrl_demo", "hellocharts_demo", "lightbutton_demo", "mclistview_demo",
    "propanimator_demo", "sci_demo", "wallpaper", "ipcdemo", "SoTool", "fun_test", "uieditor",
)
POPUP_ONLY = {"lightbutton_demo"}


def shadows(demo, theme):
    uires = REPO / "demos" / demo / "uires"
    entries = {(group.tag.casefold(), n.get("name")): n.get("path").replace("\\", "/")
               for group in ET.parse(uires / "uires.idx").getroot() for n in group}
    palette = ET.parse(uires / "values" / f"fluent_{theme}.xml").getroot()
    for skin in palette:
        name = skin.get("name", "")
        if name in ("fluent_window_shadow", "fluent_popup_shadow", "ue_fluent_popup_shadow", "ue_fluent_window_shadow"):
            kind = "window" if "window" in name else "popup"
            resource_type, resource_name = skin.get("src").split(":", 1)
            yield kind, skin, uires / entries[(resource_type.casefold(), resource_name)]


class DemoShadowStyleTests(unittest.TestCase):
    def test_all_indexed_shadows_use_current_reference_alpha_and_preserve_surface(self):
        count = 0
        for demo in DEMOS:
            for theme in ("light", "dark"):
                uires = REPO / "demos" / demo / "uires"
                tokens = {n.tag: n.get("value") for n in ET.parse(
                    uires / "values" / f"theme_color_{theme}.xml").getroot().find("color")}
                prefix = "ue_fluent_" if demo == "uieditor" else "fluent_"
                with Image.open(REPO / "demos/dpidemo/uires/image" / f"window_{theme}.png") as reference:
                    expected_alpha = reference.getchannel("A").tobytes()
                assets = list(shadows(demo, theme))
                self.assertEqual(len(assets), 1 if demo in POPUP_ONLY else 2, demo)
                self.assertEqual({kind for kind, _, _ in assets},
                                 {'popup'} if demo in POPUP_ONLY else {'window', 'popup'})
                for kind, skin, _ in assets:
                    self.assertEqual(skin.get('name'), prefix + kind + '_shadow')
                for kind, skin, path in assets:
                    count += 1
                    with self.subTest(demo=demo, theme=theme, kind=kind), Image.open(path) as image:
                        self.assertEqual(skin.tag, "imgframe")
                        self.assertEqual(skin.get("margin"), "26,20,26,32")
                        self.assertEqual(image.mode, "RGBA")
                        self.assertEqual(image.size, (100, 100))
                        self.assertTrue(image.getchannel("A").tobytes() == expected_alpha,
                                        "Shadow alpha differs from shipped dpidemo window/menu style")
                        token = prefix + ("window_bg" if kind == "window" else "card_bg")
                        self.assertEqual(image.getpixel((50, 44)), (*ImageColor.getrgb(tokens[token]), 255))
        self.assertEqual(count, 66)

    def test_four_gutters_fade_without_hard_edges_or_excessive_spread(self):
        for demo in DEMOS:
            for theme in ("light", "dark"):
                for kind, _, path in shadows(demo, theme):
                    with self.subTest(demo=demo, theme=theme, kind=kind), Image.open(path) as image:
                        alpha = image.getchannel("A")
                        for edge in ((0, 0, 100, 1), (0, 99, 100, 100),
                                     (0, 0, 1, 100), (99, 0, 100, 100)):
                            self.assertEqual(alpha.crop(edge).getextrema(), (0, 0))
                        profiles = (
                            [alpha.getpixel((i, 44)) for i in range(16)],
                            [alpha.getpixel((99-i, 44)) for i in range(16)],
                            [alpha.getpixel((50, i)) for i in range(10)],
                            [alpha.getpixel((50, 99-i)) for i in range(22)],
                        )
                        for values in profiles:
                            self.assertEqual(values[0], 0)
                            self.assertLessEqual(values[1], 1)
                            self.assertGreaterEqual(len(set(values)), 5)
                            self.assertTrue(all(0 <= b-a <= 10 for a,b in pairwise(values)))
                            self.assertLessEqual(max(values[:max(1, len(values)-10)]), 2)


if __name__ == "__main__":
    unittest.main(verbosity=2)
