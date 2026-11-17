"""Local titlebar layout contracts (no GUI assertions)."""
from pathlib import Path
import unittest
import xml.etree.ElementTree as ET

UI = Path(__file__).resolve().parent / 'uires/xml'


class GalleryTitlebars(unittest.TestCase):
    def test_maximize_and_restore_share_one_explicit_hit_target(self):
        # SouiLayout without pos defaults to the previous sibling's right edge.
        # Hidden max still occupies its slot; restore must not follow it into close.
        caption = ET.parse(UI / 'gallery.xml').find('.//caption')
        maximum = caption.find('.//imgbtn[@name="btn_max"]')
        restore = caption.find('.//imgbtn[@name="btn_restore"]')
        slot = next(node for node in caption if maximum in list(node))
        self.assertIn(restore, list(slot))
        self.assertEqual(slot.get('size'), '46dp,40dp')
        for button in (maximum, restore):
            self.assertEqual(button.get('pos'), '0,0')
            self.assertEqual(button.get('size'), slot.get('size'))
        self.assertEqual(restore.get('show'), '0')
        self.assertEqual(list(caption)[list(caption).index(slot) + 1].get('name'), 'btn_close')

    def test_caption_and_system_button_extents(self):
        for name in ('gallery.xml', 'dialog.xml', 'gallery_native.xml'):
            with self.subTest(name=name):
                root = ET.parse(UI / name).getroot().find('root')
                caption = root.find('caption')
                self.assertEqual(caption.get('size'), '-2,48dp')
                self.assertEqual(caption.get('padding'), '8dp,0,0,0')
                self.assertEqual(caption.get('interval', '0'), '0')
                self.assertEqual(caption.get('font'), 'size:14dp,bold:1')
                for title in caption.iter('text'):
                    self.assertEqual(title.get('font'), 'size:14dp,bold:1')
                for button in caption.iter('imgbtn'):
                    expected = '40dp,32dp' if button.get('name') == 'fluent_theme_toggle' else '46dp,40dp'
                    self.assertEqual(button.get('size'), expected)
        caption = ET.parse(UI / 'gallery.xml').find('.//caption')
        self.assertEqual(caption.find('img').get('size'), '16dp,16dp')
        self.assertEqual(caption.find('text').get('extend_left'), '5dp')

    def test_dialog_body_capacity(self):
        self.assertEqual(ET.parse(UI / 'dialog.xml').find('root').get('size'), '460dp,278dp')
        native = ET.parse(UI / 'gallery_native.xml').find('root')
        self.assertEqual(native.get('size'), '560dp,308dp')
        self.assertIsNone(native.get('padding'))
        self.assertEqual(native.find('window').get('padding'), '20dp,32dp,20dp,20dp')
        self.assertEqual(native.find('.//realwnd').get('size'), '-2,120dp')


if __name__ == '__main__':
    unittest.main()
