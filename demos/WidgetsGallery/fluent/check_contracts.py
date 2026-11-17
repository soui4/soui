#!/usr/bin/env python3
"""Check the pre-migration demo contracts and paired Fluent resources.

SOUI accepts t: inline-template tags without an XML namespace; normalize only
those tags for ElementTree. Original double BOM/unescaped legacy menu text are
normalized while recording the baseline, not tolerated in newly written files.
"""
import argparse
from collections import Counter
import json
from pathlib import Path
import re
import xml.etree.ElementTree as ET
from titlebar_contract_migrations import WX_MAIN, WX_PAGES, wx_relocation

REPO = Path(__file__).resolve().parents[2]
CONTRACTS = Path(__file__).with_name('contracts.json')


def preserved_dimension(relative, scope, attr, original, actual):
    caption_growth = {
        ('demos/demo2/uires/xml/dlg_main.xml', 'root', 'height'): ('400', '448'),
        ('demos/wxDemo/uires/xml/dlg_main.xml', 'host', 'height'): ('640', '684'),
        ('demos/demo/uires/xml/dlg_formatmsg.xml', 'host', 'height'): ('300', '308'),
        ('demos/demo/uires/xml/dlg_skinset.xml', 'root', 'height'): ('500', '508'),
    }
    return actual == original or caption_growth.get((relative, scope, attr)) == (original, actual)


def parse_layout(data, legacy=False):
    text = data.decode('utf-8-sig') if isinstance(data, bytes) else data
    if legacy:
        text = text.lstrip('\ufeff')
        def deduplicate(match):
            seen = set()
            def attribute(item):
                if item[1] in seen:
                    return ''
                seen.add(item[1])
                return item[0]
            return re.sub(r'(\w+)="[^"]*"', attribute, match[0])
        text = re.sub(r'<[A-Za-z][^>]+>', deduplicate, text)
        text = re.sub(r'&(?!#\d+;|#x[\da-fA-F]+;|amp;|lt;|gt;|quot;|apos;)', '&amp;', text)
    text = re.sub(r'<(/?)t:', r'<\1t_', text)
    return ET.fromstring(text)


def named_nodes(root):
    return Counter((n.tag, n.get('name', ''), n.get('id', ''))
                   for n in root.iter() if n.get('name') or n.get('id'))


def check(demos, require_theme=True):
    baseline = json.loads(CONTRACTS.read_text(encoding='utf-8'))
    errors = []
    moved = Counter()
    if 'wxDemo' in demos and require_theme:
        try:
            moved = wx_relocation({p: (REPO / p).read_text(encoding='utf-8-sig')
                                   for p in (WX_MAIN, *WX_PAGES)})
        except (ValueError, OSError, ET.ParseError) as error:
            errors.append(f'wxDemo action relocation: {error}')
    layouts = 0
    for relative, expected in baseline['layouts'].items():
        if relative.split('/')[1] not in demos:
            continue
        layouts += 1
        try:
            root = parse_layout((REPO / relative).read_bytes(), legacy=not require_theme)
        except (OSError, ET.ParseError) as error:
            errors.append(f'{relative}: {error}')
            continue
        for attr, value in expected['size'].items():
            if not preserved_dimension(relative, 'host', attr, value, root.get(attr)):
                errors.append(f'{relative}: original {attr}={value} changed to {root.get(attr)}')
        if sum(n.tag == 'page' for n in root.iter()) != expected['pages']:
            errors.append(f'{relative}: original page count changed')
        body = root.find('root')
        for attr, value in expected.get('root_content_size', {}).items():
            if body is None or not preserved_dimension(relative, 'root', attr, value, body.get(attr)):
                errors.append(f'{relative}: original root content {attr}={value} changed')
        actual = named_nodes(root)
        if relative in WX_PAGES:
            actual.update(moved)
        for tag, name, ident, count in expected['named']:
            if actual[(tag, name, ident)] < count:
                errors.append(f'{relative}: removed original {tag} name={name} id={ident}')
    for demo in demos:
        uires = REPO / 'demos' / demo / 'uires'
        try:
            init = ET.parse(uires / 'uidef/init.xml').getroot()
            unit = init.find('unit')
            current_unit = unit.get('defUnit') if unit is not None else 'px'
            if current_unit != baseline['default_units'][demo]:
                errors.append(f'{demo}: original layout unit {baseline["default_units"][demo]} changed to {current_unit}')
            idx = ET.parse(uires / 'uires.idx').getroot()
            for group in idx:
                seen = set()
                for resource in group:
                    name = resource.get('name', '').casefold()
                    if name in seen:
                        errors.append(f'{demo}: duplicate {group.tag}:{name}')
                    seen.add(name)
                    path = uires / resource.get('path', '').replace('\\', '/')
                    if not path.is_file():
                        errors.append(f'{demo}: missing indexed resource {path}')
            if require_theme:
                for stem, attr in (('fluent', 'name'), ('theme_color', None)):
                    pools = [ET.parse(uires / 'values' / f'{stem}_{v}.xml').getroot() for v in ('light', 'dark')]
                    sets = [{n.get(attr) if attr else n.tag for n in (root if attr else root.find('color'))} for root in pools]
                    if sets[0] != sets[1] or not sets[0]:
                        errors.append(f'{demo}: {stem} light/dark names mismatch or empty')
        except (OSError, ET.ParseError) as error:
            errors.append(f'{demo}: {error}')
    return layouts, errors


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--demo', action='append', help='Explicit demo; defaults to all confirmed targets')
    parser.add_argument('--baseline', action='store_true', help='Original contract check, before paired palettes exist')
    args = parser.parse_args()
    selected = args.demo or json.loads(CONTRACTS.read_text(encoding='utf-8'))['demos']
    count, errors = check(selected, not args.baseline)
    for error in errors:
        print('ERROR:', error)
    print(f'{len(selected)} demos, {count} original layouts/menus: {len(errors)} errors')
    raise SystemExit(bool(errors))
