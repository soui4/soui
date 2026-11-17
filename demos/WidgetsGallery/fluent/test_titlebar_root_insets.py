"""Six-layout inset regression: geometry model plus unchanged business trees.

Coordinates are relative to the root panel, excluding the existing host margin
and sci_demo's one-pixel border margin. GUI smoke remains separate evidence.
"""
import hashlib
import json
from pathlib import Path
import unittest
import xml.etree.ElementTree as ET

DEMOS = Path(__file__).resolve().parents[1]
# Layout, old padding, old root interval, original business-tree fingerprint.
CASES = {
    "flowlayout_demo": ("xml/dlg_main.xml", "8dp", "0", "aa66f3c155a9bd492ab41891564e7fa1930d758315b9497bdf83c78b4cf552b4"),
    "framelayout_demo": ("xml/dlg_main.xml", "8dp", "0", "66fe4f983fdf93eef4f33754b36d2b3435644c47da396a2900bacda1b1a750f9"),
    "propanimator_demo": ("xml/dlg_main.xml", "5", "0", "d2fc2eeed08d3ae913233f9bb349793b50d0217e24bf1fbbea88ac69b41995e5"),
    "sci_demo": ("layout/sci.demo.xml", "5", "0", "fe18970220003c0e1422b2380ace251aa6cc337563cf0e3dc970975a4e5d3baa"),
    "lightbutton_demo": ("layout/dlg_main.xml", "5", "5", "0c64e572bba8ed05cdaa65ba815bd377d644c04d4fc45a814370d660ea85a0ea"),
    "ipcdemo": ("xml/dlg_client.xml", "5", "10", "c3e6e1c1dee23722e10fea6471328d4383c222557951fdfa0901d9b4b53c68e1"),
}


def px(value, scale, default_unit):
    if value.endswith("dp"):
        return float(value[:-2]) * scale
    if value.endswith("px"):
        return float(value[:-2])
    return float(value) * (scale if default_unit == "dp" else 1)


def padding(node, scale, default_unit):
    parts = node.get("padding", node.get("inset", "0")).split(",")
    if len(parts) == 1:
        parts *= 4
    return tuple(px(part, scale, default_unit) for part in parts)


def fingerprint(node):
    return [node.tag, sorted(node.attrib.items()), (node.text or "").strip(),
            [fingerprint(child) for child in node]]


class TitlebarRootInsetsTest(unittest.TestCase):
    def test_caption_effective_ancestor_insets(self):
        for demo, (layout, _, _, _) in CASES.items():
            unit = ET.parse(DEMOS / demo / "uires/uidef/init.xml").find("unit").get("defUnit")
            root = ET.parse(DEMOS / demo / "uires" / layout).find("root")
            caption = root.find(".//caption")
            parents = {child: parent for parent in root.iter() for child in parent}
            for scale in (1, 1.25, 1.5, 2):
                with self.subTest(demo=demo, scale=scale):
                    ancestors = []
                    node = caption
                    while node in parents:
                        node = parents[node]
                        ancestors.append(node)
                    effective = tuple(sum(padding(n, scale, unit)[i] for n in ancestors)
                                      for i in range(4))
                    self.assertEqual(effective, (0, 0, 0, 0))
                    self.assertIs(caption, root[0])
                    self.assertEqual(caption.get("size"), "-2,48dp")
                    if demo == "ipcdemo":
                        icon_x = px(caption.find("icon").get("pos").split(",")[0], scale, unit)
                        title_x = px(caption.find("text").get("pos").split(",")[0], scale, unit)
                    else:
                        icon_x = padding(caption, scale, unit)[0]
                        title_x = icon_x + 16 * scale + px(caption.find("text").get("margin").split(",")[0], scale, unit)
                    self.assertEqual(icon_x + effective[0], 8 * scale)
                    self.assertEqual(title_x + effective[0], 29 * scale)
                    self.assertEqual(effective[1] + (48 - 40) * scale / 2, 4 * scale)
                    self.assertEqual(effective[2], 0)  # close-button right inset

    def test_body_geometry_and_business_tree_are_preserved(self):
        dimensions = {"flowlayout_demo": ("600", "400"),
                      "framelayout_demo": ("600", "400"),
                      "propanimator_demo": ("800", "600"),
                      "sci_demo": ("760", "480"),
                      "lightbutton_demo": ("800", "600"),
                      "ipcdemo": ("600", "-1")}
        for demo, (layout, old_padding, old_interval, expected_hash) in CASES.items():
            unit = ET.parse(DEMOS / demo / "uires/uidef/init.xml").find("unit").get("defUnit")
            with self.subTest(demo=demo):
                host = ET.parse(DEMOS / demo / "uires" / layout).getroot()
                root = host.find("root")
                self.assertEqual(tuple(root.get(k, host.get(k)) for k in ("width", "height")), dimensions[demo])
                self.assertEqual(host.get("margin"), "5,5,5,5")
                self.assertIsNone(root.get("margin"))
                self.assertEqual(len(root), 2)
                body = root[1]
                self.assertEqual(body.tag, "window")
                self.assertIsNone(body.get("name"))
                self.assertIsNone(body.get("id"))
                self.assertEqual(body.get("layout"), "vbox")
                self.assertEqual(root.get("interval", "0"), "0")
                self.assertEqual(body.get("interval", "0"), old_interval)
                self.assertEqual(body.get("size"), "-2,-1" if demo == "ipcdemo" else "-2,0")
                self.assertEqual(body.get("weight"), None if demo == "ipcdemo" else "1")
                actual = [fingerprint(child) for child in body]
                self.assertEqual(hashlib.sha256(json.dumps(actual, ensure_ascii=True).encode()).hexdigest(), expected_hash)
                for scale in (1, 1.25, 1.5, 2):
                    p, gap = px(old_padding, scale, unit), px(old_interval, scale, unit)
                    left, top, right, bottom = padding(body, scale, unit)
                    self.assertEqual((left, top, right, bottom), (p, p + gap, p, p))
                    # Root vbox gives the wrapper all remaining height. Moving
                    # padding inside it must preserve body origin AND extent.
                    for width, height in ((600, 400), (1000, 720)):
                        before = (p, p + 48 * scale + gap, width - 2*p,
                                  height - 2*p - 48*scale - gap)
                        after = (left, 48*scale + top, width-left-right,
                                 height-48*scale-top-bottom)
                        self.assertEqual(after, before)
                if demo == "framelayout_demo":
                    frame = body[0]
                    self.assertEqual(frame.get("layout"), "frame")
                    self.assertEqual(frame.get("enableDockMode"), "any")
                    self.assertEqual(len(frame), 9)
                    names = {child.get("name") for child in frame}
                    for child in frame:
                        if child.get("dockRelativeTo"):
                            self.assertIn(child.get("dockRelativeTo"), names)


if __name__ == "__main__":
    unittest.main()
