"""Check dpidemo chrome spacing without replacing demo action identities."""
from pathlib import Path
import unittest
import xml.etree.ElementTree as ET


DEMOS = Path(__file__).resolve().parent.parent


class TitlebarLayoutTest(unittest.TestCase):
    def test_theme_icon_has_no_overlaid_caption(self):
        root = ET.parse(DEMOS / "fun_test/uires/xml/dlg_main.xml")
        button = root.find(".//imgbtn[@name='fluent_theme_toggle']")
        self.assertIsNone(button.get("text"))
        self.assertEqual(button.get("tip"), "浅 / 深")
        self.assertEqual(button.get("skin"), "fluent_btn_icon_theme")

    def test_close_only_windows_keep_their_identity(self):
        sci = ET.parse(DEMOS / "sci_demo/uires/layout/sci.demo.xml").find(".//caption")
        self.assertEqual(sci.find("img").get("skin"), "logo")
        self.assertEqual(sci.find("img").get("size"), "16dp,16dp")
        self.assertEqual(sci.get("padding"), "8dp,0,0,0")
        self.assertEqual(sci.find("text").get("margin"), "5dp,0,8dp,0")
        student = ET.parse(DEMOS / "mclistview_demo/uires/layout/dlg.student.sms.xml").find(".//caption")
        self.assertEqual(student.find("text").get("pos"), "8dp,0,-98dp,-0")
        self.assertEqual(student.find("imgbtn[@name='fluent_theme_toggle']").get("pos"), "-86dp,8dp")
        for caption in (sci, student):
            self.assertIsNotNone(caption.find("imgbtn[@id='IDCANCEL']"))
            self.assertEqual(len(caption.findall("imgbtn")), 2)

    def test_absolute_main_caption_spacing(self):
        for project, layout in (("SoTool", "dlg_main"), ("wallpaper", "dlg_main"),
                                ("ipcdemo", "dlg_client"), ("ipcdemo", "dlg_server")):
            with self.subTest(project=project):
                caption = ET.parse(DEMOS / project / f"uires/xml/{layout}.xml").find(".//caption")
                icon = caption.find("icon")
                self.assertEqual(icon.get("pos"), "8dp,|0,@16dp,@16dp")
                self.assertEqual(caption.find("text").get("pos"), "29dp,0,-190dp,-0")
                self.assertEqual(caption.find("text").get("class"), "fluent_caption")
                theme = caption.find("imgbtn[@name='fluent_theme_toggle']")
                self.assertEqual(theme.get("pos"), "-178dp,8dp")
                for name, position in (("btn_close", "-46dp,4dp"),
                                       ("btn_max", "-92dp,4dp"),
                                       ("btn_restore", "-92dp,4dp"),
                                       ("btn_min", "-138dp,4dp")):
                    button = caption.find(f"imgbtn[@name='{name}']")
                    self.assertEqual(button.get("pos"), position)
                    self.assertEqual(button.get("size"), "46dp,40dp")
                self.assertEqual(caption.find("imgbtn[@name='btn_restore']").get("show"), "0")

    def test_linear_caption_spacing(self):
        projects = {"flowlayout_demo": "xml/dlg_main.xml",
                    "framelayout_demo": "xml/dlg_main.xml",
                    "propanimator_demo": "xml/dlg_main.xml",
                    "gridctrl_demo": "layout/dlg_main.xml",
                    "hellocharts_demo": "layout/dlg_main.xml",
                    "lightbutton_demo": "layout/dlg_main.xml"}
        for project, layout in projects.items():
            with self.subTest(project=project):
                caption = ET.parse(DEMOS / project / "uires" / layout).find(".//caption")
                self.assertEqual(caption.get("padding"), "8dp,0,0,0")
                self.assertEqual(caption.get("interval"), "0")
                self.assertEqual(caption.find("icon").get("size"), "16dp,16dp")
                self.assertEqual(caption.find("text").get("margin"), "5dp,0,8dp,0")


if __name__ == "__main__":
    unittest.main()
