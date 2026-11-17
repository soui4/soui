"""Install only the current dpidemo window shadow without regenerating palettes."""
import argparse
from pathlib import Path
import shutil
import xml.etree.ElementTree as ET

DEMOS = Path(__file__).resolve().parents[1]


def install(demo):
    uires = DEMOS / demo / 'uires'
    index_path = uires / 'uires.idx'
    index = ET.parse(index_path)
    group = index.find('PNG')
    if group is None:
        group = ET.SubElement(index.getroot(), 'PNG')
    for mode in ('light', 'dark'):
        resource = 'FL_CHROME_WINDOW_' + mode.upper()
        relative = f'image/fluent_chrome/{resource}.png'
        target = uires / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(DEMOS / f'dpidemo/uires/image/window_{mode}.png', target)
        entry = next((node for node in group if node.get('name') == resource), None)
        if entry is None:
            entry = ET.SubElement(group, 'file', name=resource)
        entry.set('path', relative.replace('/', '\\'))
        palette_path = uires / f'values/fluent_{mode}.xml'
        palette = ET.parse(palette_path)
        skin = palette.find(".//*[@name='fluent_window_shadow']")
        if skin is None:
            skin = ET.SubElement(palette.getroot(), 'imgframe', name='fluent_window_shadow')
        skin.attrib.update(src='PNG:' + resource, states='1', margin='26,20,26,32', enableColorize='0')
        ET.indent(palette, space='  ')
        palette.write(palette_path, encoding='utf-8', xml_declaration=True)
    ET.indent(index, space='  ')
    index.write(index_path, encoding='utf-8', xml_declaration=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--demo', action='append', required=True,
                        choices=('gridctrl_demo', 'sci_demo'))
    for selected in parser.parse_args().demo:
        install(selected)
