from pathlib import Path
import unittest
import xml.etree.ElementTree as ET

DEMOS = Path(__file__).resolve().parents[1]


class ReportedWindowShells(unittest.TestCase):
    def test_grid_and_scintilla_can_present_the_alpha_shadow(self):
        for demo, layout in (('gridctrl_demo', 'layout/dlg_main.xml'),
                             ('sci_demo', 'layout/sci.demo.xml')):
            with self.subTest(demo=demo):
                host = ET.parse(DEMOS / demo / 'uires' / layout).getroot()
                self.assertEqual(host.get('translucent'), '1')
                root = host.find('root')
                self.assertIn(root.get('margin', '0'), ('0', '0,0,0,0'))
                self.assertIn(root.get('padding', '0'), ('0', '0,0,0,0'))
                for palette in ('light', 'dark'):
                    skin = ET.parse(DEMOS / demo / f'uires/values/fluent_{palette}.xml')
                    self.assertIsNotNone(skin.find(".//*[@name='fluent_window_shadow']"))

    def test_grid_long_title_keeps_window_actions_on_screen(self):
        caption = ET.parse(DEMOS / 'gridctrl_demo/uires/layout/dlg_main.xml').find('.//caption')
        title = caption.find('text')
        self.assertEqual(title.get('weight'), '1')
        self.assertEqual(title.get('size'), '0,-1')
        self.assertEqual(title.get('dotted'), '1')
        buttons = {button.get('name') for button in caption.iter('imgbtn')}
        self.assertTrue({'btn_close', 'btn_min', 'btn_max', 'btn_restore'} <= buttons)


if __name__ == '__main__':
    unittest.main()
