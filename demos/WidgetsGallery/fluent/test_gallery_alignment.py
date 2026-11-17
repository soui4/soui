"""Gallery-wide style checks plus pre-change UI behavior contracts (no SDK required)."""
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import unittest
import xml.etree.ElementTree as ET
from titlebar_contract_migrations import (COMPANION, WX_MAIN, WX_PAGES, WX_ACTIONS,
                                         migrated_behavior, validate_companion,
                                         validate_companion_creation, wx_relocation)

REPO = Path(__file__).resolve().parents[2]
DEMOS = REPO / 'demos'
BASELINE = Path(__file__).with_name('gallery_behavior.json')
PROJECTS = ('android-demo demo demo2 dpidemo flowlayout_demo framelayout_demo '
            'fun_test gridctrl_demo hellocharts_demo ipcdemo lightbutton_demo '
            'mclistview_demo ohos-demo propanimator_demo sci_demo screenpin SoTool '
            'SOUIWidgetsGalleryDemo uieditor wallpaper wxDemo').split()
MOBILE = {
    'android-demo': 'app/src/main/assets/uires',
    'ohos-demo': 'entry/src/main/resources/rawfile/uires',
}
MAIN = {
    'demo': 'xml/dlg_main.xml', 'dpidemo': 'xml/dlg_main.xml',
    'flowlayout_demo': 'xml/dlg_main.xml', 'framelayout_demo': 'xml/dlg_main.xml',
    'fun_test': 'xml/dlg_main.xml', 'gridctrl_demo': 'layout/dlg_main.xml',
    'hellocharts_demo': 'layout/dlg_main.xml', 'lightbutton_demo': 'layout/dlg_main.xml',
    'propanimator_demo': 'xml/dlg_main.xml', 'mclistview_demo': 'layout/dlg.student.sms.xml',
    'sci_demo': 'layout/sci.demo.xml', 'SoTool': 'xml/dlg_main.xml',
    'uieditor': 'xml/dlg_main.xml', 'wallpaper': 'xml/dlg_main.xml',
    'SOUIWidgetsGalleryDemo': 'xml/gallery.xml',
}

DIALOGS = {
    'demo': ('dlg_formatmsg', 'dlg_skinset'),
    'demo2': ('dlg_test',),
    'fun_test': ('wnd_soui',),
    'SOUIWidgetsGalleryDemo': ('gallery_native',),
    'uieditor': ('dlg_about', 'dlg_find', 'dlg_input', 'dlg_new_custom',
                 'dlg_new_skin', 'dlg_insert_element', 'dlg_skin_viewer',
                 'dlg_new_layout', 'messagebox'),
}


def uires(project):
    return DEMOS / project / MOBILE.get(project, 'uires')


def attributes(tag):
    return dict((m[1], m[3]) for m in re.finditer(r'([\w:.-]+)\s*=\s*([\'\"])(.*?)\2', tag, re.S))


def behavior(text):
    """Ignore presentation only; preserve IDs, labels, events, pages and host mode.

    Lexical parsing also covers the UIEditor's legacy snippets with XML examples
    inside attributes, which SOUI accepts but ElementTree cannot parse.
    """
    text = re.sub(r'<!--.*?-->', '', text, flags=re.S)
    nodes, labels, events, pages, hosts = [], [], [], [], []
    for m in re.finditer(r'<([\w:.-]+)\b([^<>]*)>', text):
        tag, a = m[1], attributes(m[2])
        if (a.get('name') or a.get('id')) and tag not in ('class',):
            kind = 'theme_action' if a.get('name') == 'fluent_theme_toggle' else tag
            nodes.append((kind, a.get('name', ''), a.get('id', '')))
        if tag not in ('class',) and a.get('text'):
            labels.append(a['text'])
        elif (a.get('name') == 'fluent_theme_toggle' and tag == 'imgbtn'
              and (a.get('tip'), a.get('skin')) in (
                  ('浅 / 深', 'fluent_btn_icon_theme'),
                  ('@string/ue_theme_toggle', 'ue_fluent_btn_icon_theme'))):
            labels.append(a['tip'])
        events.extend((tag, k, v) for k, v in a.items() if k.startswith('on_'))
        if tag == 'page':
            pages.append((a.get('name', ''), a.get('title', '')))
        if tag.lower() == 'soui':
            hosts.append({k: v for k, v in a.items() if k in (
                'wndType', 'translucent', 'appWnd', 'appwin', 'resizable',
                'sendWheel2Hover', 'toolwnd', 'toolWindow', 'enableIME')})
    return dict(nodes=sorted(nodes), labels=sorted(labels), events=sorted(events),
                pages=pages, hosts=hosts)


def layouts():
    for project in PROJECTS:
        for path in sorted(uires(project).rglob('*.xml')):
            if path.parent.name in ('xml', 'layout', 'smenu'):
                yield path


def parse(path):
    text = path.read_text(encoding='utf-8-sig').lstrip('\ufeff')
    return ET.fromstring(re.sub(r'<(/?)t:', r'<\1t_', text))


def caption_height(caption):
    if caption.get('size'):
        return caption.get('size').split(',')[1].removesuffix('dp')
    pos = caption.get('pos', '').split(',')
    if len(pos) == 4:
        bottom = pos[3].lstrip('@').removesuffix('dp')
        if pos[3].startswith('@'):
            return bottom
        return str(int(bottom) - int(pos[1].removesuffix('dp')))
    return caption.get('height', '').removesuffix('dp')


class GalleryAlignment(unittest.TestCase):
    def test_all_projects_in_scope(self):
        actual = {p.name for p in DEMOS.iterdir() if p.is_dir() and
                  ((p / 'uires').is_dir() or p.name in MOBILE)}
        self.assertEqual(actual, set(PROJECTS), 'new UI projects require an explicit alignment decision')

    def test_business_contracts(self):
        expected = json.loads(BASELINE.read_text(encoding='utf-8'))
        documents = {p.relative_to(REPO).as_posix(): p.read_text(encoding='utf-8-sig') for p in layouts()}
        self.assertEqual(set(expected) | {COMPANION}, set(documents))
        validate_companion(documents[COMPANION])
        for relative, digest in expected.items():
            with self.subTest(layout=relative):
                payload = json.dumps(migrated_behavior(relative, documents, behavior),
                                     sort_keys=True, ensure_ascii=False)
                self.assertEqual(hashlib.sha256(payload.encode()).hexdigest(), digest,
                                 'changed business IDs, labels, events, pages or host mode')

    def test_main_caption_and_window_actions(self):
        paths = [uires(p) / relative for p, relative in MAIN.items()]
        paths += [uires('ipcdemo') / 'xml' / f'dlg_{side}.xml' for side in ('client', 'server')]
        for path in paths:
            with self.subTest(layout=path.relative_to(REPO)):
                root = parse(path)
                caption = root.find('.//caption')
                self.assertIsNotNone(caption)
                self.assertEqual(caption_height(caption), '48')
                parents = {child: parent for parent in caption.iter() for child in parent}
                for n in caption.iter():
                    if n.tag not in ('imgbtn', 'button'): continue
                    if n.get('name') in ('fluent_theme_toggle', 'btn_theme'):
                        self.assertEqual(n.get('size'), '40dp,32dp')
                    elif (n.get('skin', '').startswith('_skin.sys.btn.') or
                          n.get('skin', '').startswith('ue_fluent_sys_btn_')):
                        size = n.get('size')
                        if size == '-2,-2':
                            size = parents[n].get('size')
                        self.assertEqual(size, '46dp,40dp')

    def test_demo2_layered_host_keeps_resize_and_system_actions(self):
        main = parse(uires('demo2') / 'xml/dlg_main.xml')
        self.assertEqual(main.get('resizable'), '1')
        self.assertEqual(main.get('translucent'), '1')
        self.assertIsNone(main.get('wndStyle'))
        self.assertIsNone(main.find('ncpainter'))
        caption = main.find('root/caption')
        self.assertEqual(caption_height(caption), '48')
        self.assertEqual({n.get('id') for n in caption.iter() if n.get('id')},
                         {'sysid_icon', 'sysid_title', 'sysid_min', 'sysid_max',
                          'sysid_restore', 'sysid_close', '65000'})
        source = (DEMOS / 'demo2/demo2.c').read_text(encoding='utf-8-sig')
        self.assertRegex(source, r'CreateEx\(hostWnd,NULL,WS_POPUP,')

    def test_layered_migrations_and_private_editor_companion(self):
        for project, path in (('demo2', 'xml/dlg_main.xml'),
                              ('gridctrl_demo', 'layout/dlg_main.xml'),
                              ('sci_demo', 'layout/sci.demo.xml')):
            document = parse(uires(project) / path)
            self.assertEqual(document.get('translucent'), '1')
            self.assertEqual(len(document.findall('.//caption')), 1)
        grid = parse(uires('gridctrl_demo') / 'layout/dlg_main.xml')
        title = grid.find('root/caption/text')
        self.assertEqual((title.get('size'), title.get('weight')), ('0,-1', '1'))
        sci = parse(uires('sci_demo') / 'layout/sci.demo.xml')
        self.assertIsNone(sci.find('root').get('margin'))
        editor = parse(uires('uieditor') / 'xml/dlg_main.xml')
        self.assertEqual(editor.get('translucent', '0'), '0')
        validate_companion((REPO / COMPANION).read_text(encoding='utf-8-sig'))
        index = parse(uires('uieditor') / 'uires.idx')
        entries = index.findall("LAYOUT/file[@name='ue_window_shadow']")
        self.assertEqual(len(entries), 1)
        self.assertEqual(entries[0].get('path').replace('\\', '/'), 'xml/window_shadow.xml')
        chrome = (DEMOS / 'uieditor/EditorChrome.h').read_text(encoding='utf-8-sig')
        self.assertIn('LAYOUT:ue_window_shadow', chrome)
        validate_companion_creation(chrome)

    def test_demo2_client_caption_preserves_content_space(self):
        document = parse(uires('demo2') / 'xml/dlg_main.xml')
        root = document.find('root')
        self.assertEqual(root.get('height'), '448')
        self.assertEqual(root.get('padding'), '0,0,0,0')
        content = root.find('window')
        self.assertEqual(content.get('pos'), '0,48dp,-0,-0')
        self.assertEqual(content.get('padding'), '16dp,12dp,16dp,16dp')
        self.assertEqual(content.get('interval'), '12dp')
        toolbar, slider, body = content.findall('window')
        self.assertEqual(toolbar.get('size'), '-2,36dp')
        self.assertEqual(toolbar.get('interval'), '8dp')
        for button in toolbar.findall('button'):
            self.assertEqual(button.get('size'), '0,36dp')
            self.assertEqual(button.get('weight'), '1')
        self.assertEqual(toolbar.find('testCtrl').get('size'), '100dp,36dp')
        self.assertEqual(body.get('interval'), '12dp')
        self.assertEqual(slider.get('size'), '-2,32dp')
        for button in body.findall('window/button'):
            self.assertEqual(button.get('size'), '-2,32dp')

    def test_dialog_caption_and_window_actions(self):
        for project, dialogs in DIALOGS.items():
            for dialog in dialogs:
                with self.subTest(project=project, dialog=dialog):
                    root = parse(uires(project) / 'xml' / f'{dialog}.xml')
                    self.assertEqual(caption_height(root.find('.//caption')), '48')
                    actions = [n for n in root.iter('imgbtn') if
                               n.get('skin', '') in ('_skin.sys.btn.close', 'ue_fluent_sys_btn_close')]
                    self.assertEqual(len(actions), 1)
                    self.assertEqual(actions[0].get('size'), '46dp,40dp')
                    if actions[0].get('pos'):
                        position = actions[0].get('pos')
                        self.assertIn(position, ('-46dp,4dp', '-46dp,|0'))
                        if position.endswith('|0'):
                            self.assertEqual(actions[0].get('offset'), '0,-0.5')

    def test_dialog_absolute_content_clears_caption(self):
        about = parse(uires('uieditor') / 'xml/dlg_about.xml')
        self.assertEqual(about.find("root/window/text").get('pos'), '25,56')
        input_dialog = parse(uires('uieditor') / 'xml/dlg_input.xml')
        self.assertEqual(input_dialog.find(".//*[@name='edtInput']").get('pos'), '56,59dp')
        new_layout = parse(uires('uieditor') / 'xml/dlg_new_layout.xml')
        self.assertEqual(new_layout.find('root/window').get('pos'), '10,53dp,-10,-10')
        self.assertEqual(new_layout.find(".//*[@name='NAME_UIDESIGNER_btn_OK']").get('pos'), '-100,-30,@70,@30')
        messagebox = parse(uires('uieditor') / 'xml/messagebox.xml')
        self.assertEqual(messagebox.find('root/window').get('pos'), '5,56dp,-5,-60')
        skinset = parse(uires('demo') / 'xml/dlg_skinset.xml')
        self.assertEqual(skinset.find("root/window/window").get('pos'), '0,48dp,-0,-0')

    def test_compact_new_layout_actions_do_not_cover_fields(self):
        root = parse(uires('uieditor') / 'xml/dlg_new_layout.xml')
        # The 150dp host has 5dp top/bottom frame margins. Resolve the
        # fixed/previous/bottom-relative Y coordinates used by this dialog.
        host_height = float(root.get('height'))
        margin = [float(v) for v in root.get('margin').split(',')]
        client_height = host_height - margin[1] - margin[3]
        content = root.find('root/window')
        coords = [v.strip().removesuffix('dp') for v in content.get('pos').split(',')]
        if coords[1].startswith('['):
            siblings = list(root.find('root'))
            previous = siblings[siblings.index(content) - 1]
            previous_y = float(previous.get('pos').split(',')[1].removesuffix('dp'))
            previous_h = float(previous.get('size').split(',')[1].removesuffix('dp'))
            top = previous_y + previous_h + float(coords[1][1:])
        else:
            top = float(coords[1])
        bottom = client_height + float(coords[3])
        height = bottom - top
        self.assertGreaterEqual(top, 45)
        last_bottom = 0
        for edit in content.findall('edit'):
            _, y, _, h = [v.strip().removesuffix('dp') for v in edit.get('pos').split(',')]
            y = last_bottom + float(y[1:]) if y.startswith('[') else float(y)
            self.assertGreaterEqual(y, last_bottom)
            last_bottom = y + float(h.removeprefix('@'))
        action = content.find("*[@name='NAME_UIDESIGNER_btn_OK']")
        _, y, _, h = action.get('pos').split(',')
        action_top = height + float(y) if y.startswith('-') else float(y)
        self.assertGreaterEqual(action_top - last_bottom, 1,
                                'save action covers the path field/browse button')
        self.assertLessEqual(action_top + float(h.removeprefix('@')), height)

    def test_wx_actions_move_once_to_main_caption(self):
        documents = {p: (REPO / p).read_text(encoding='utf-8-sig') for p in (WX_MAIN, *WX_PAGES)}
        self.assertEqual(sum(wx_relocation(documents).values()), 6)
        main = parse(REPO / WX_MAIN)
        caption = main.find('root/caption')
        self.assertEqual(caption_height(caption), '48')
        self.assertEqual(len(main.findall(".//*[@name='fluent_theme_toggle']")), 1)
        self.assertIsNotNone(caption.find(".//*[@name='fluent_theme_toggle']"))

    def test_wx_chrome_dpi_clearance(self):
        main = parse(REPO / WX_MAIN)
        self.assertEqual(main.get('height'), '684')
        self.assertEqual(main.get('minsize'), '910,684')
        self.assertEqual(main.get('width'), '910')
        self.assertEqual(main.find('root').get('padding'), '0')
        caption = main.find('root/caption')
        content = main.find("root/window[@name='main_content']")
        self.assertEqual(content.get('pos'), '8dp,56dp,-8dp,-8dp')
        self.assertEqual(content.find('caption').get('pos'), '0,0,@64,-0')
        self.assertIsNotNone(content.find("tabctrl[@name='tab_main_opt']"))
        actions = caption.find('window')
        self.assertEqual(actions.get('pos'), '-258dp,0,-0,-0')
        self.assertEqual(actions.get('layout'), 'hbox')
        self.assertEqual(actions.get('gravity'), 'center')
        self.assertEqual([n.get('name') for n in actions],
                         ['fluent_theme_toggle', 'btn_skin', 'btn_menu', 'btn_min', None, 'btn_close'])
        for n in actions:
            self.assertEqual(n.get('size'), '40dp,32dp' if n.get('name') in
                             ('fluent_theme_toggle', 'btn_skin', 'btn_menu') else '46dp,40dp')
        pair = actions.find('window')
        self.assertEqual([n.get('name') for n in pair], ['btn_max', 'btn_restore'])
        self.assertTrue(all(n.get('pos') == '0,0' and n.get('size') == '46dp,40dp' for n in pair))
        self.assertEqual(pair[1].get('show'), '0')
        for scale in (1, 1.5, 2):
            self.assertEqual((3*40 + 3*46)*scale, 258*scale)
            self.assertLessEqual(40*scale, 48*scale)
            self.assertGreaterEqual(910 - 258*scale, 29*scale)

    def test_normal_menu_density(self):
        for path in layouts():
            # This emoji grid is a picker, not a command menu.
            if 'emoji' in path.name: continue
            try: root = parse(path)
            except ET.ParseError: continue  # legacy designer examples are locked above
            if root.tag.lower() not in ('menu', 'menuroot'): continue
            with self.subTest(menu=path.relative_to(REPO)):
                self.assertEqual(root.get('itemHeight'), '34dp')

    def test_desktop_semantic_palette(self):
        tokens = ('fluent_window_bg', 'fluent_card_bg', 'fluent_text',
                  'fluent_text_secondary', 'fluent_border', 'fluent_accent')
        for theme in ('light', 'dark'):
            ref = parse(uires('SOUIWidgetsGalleryDemo') / 'values' / f'theme_color_{theme}.xml')
            expected = {n.tag: n.get('value').upper() for n in ref.find('color') if n.tag in tokens}
            for project in PROJECTS:
                if project in MOBILE or project == 'screenpin': continue
                root = parse(uires(project) / 'values' / f'theme_color_{theme}.xml')
                actual = {n.tag.removeprefix('ue_'): n.get('value').upper() for n in root.find('color')}
                for token, value in expected.items():
                    with self.subTest(project=project, theme=theme, token=token):
                        self.assertEqual(actual.get(token), value)

    def test_mobile_semantics_and_touch_targets(self):
        for project in MOBILE:
            with self.subTest(project=project):
                init = parse(uires(project) / 'uidef/init.xml')
                color = parse(uires(project) / 'values/color.xml')
                colors = {n.tag: n.get('value', '').upper() for n in (color if color.tag == 'color' else color.find('color'))}
                self.assertEqual(colors.get('fluent_window_bg'), '#F3F3F3')
                self.assertEqual(colors.get('fluent_accent'), '#0067C0')
                self.assertIsNotNone(init.find("objattr/button[@skin='btn_primary']"))
                main = parse(uires(project) / 'xml/dlg_main.xml')
                for n in main.iter('button'):
                    self.assertGreaterEqual(float(n.get('height', '48').removesuffix('dp')), 48)
                self.assertEqual(main.find(".//*[@name='btn_theme']").get('id'), '1002')

    def test_screenpin_gallery_surface(self):
        root = parse(uires('screenpin') / 'values/skin.xml')
        panel = root.find("*[@name='sp_panel']")
        self.assertEqual(panel.get('normal').upper(), '#FFFFFF')
        self.assertEqual(panel.get('normalBorder').upper(), '#E5E5E5')
        fill = root.find("*[@name='sp_slider_fill']")
        self.assertTrue(all(n.get('color').upper() == '#0067C0' for n in fill.iter('solid')))

    def test_mobile_surfaces_and_primary_states(self):
        reference = parse(uires('SOUIWidgetsGalleryDemo') / 'values/theme_color_light.xml')
        expected = {n.tag: n.get('value').upper() for n in reference.find('color')
                    if n.tag.startswith('fluent_')}
        for project in MOBILE:
            with self.subTest(project=project):
                root = uires(project)
                colors = {n.tag: n.get('value') for n in parse(root / 'values/color.xml')}
                for token, color in expected.items():
                    self.assertEqual(colors.get(token, '').upper(), color)
                for name, color in (('red', 'rgb(255,0,0)'), ('green', 'rgb(0,255,0)'),
                                    ('blue', 'rgb(0,0,255)'), ('white', 'rgb(255,255,255)')):
                    self.assertEqual(colors[name], color)
                init = parse(root / 'uidef/init.xml')
                self.assertEqual(init.find('unit').get('defUnit'), 'dp')
                self.assertEqual(init.find('deffont').get('value'),
                                 'face:Noto Sans CJK JP,size:30' if project == 'android-demo' else 'size:16')
                for button in (init.find('objattr/button'), init.find("style/class[@name='normalbtn']")):
                    for attr in ('colorText', 'colorTextHover', 'colorTextPushdown'):
                        self.assertEqual(button.get(attr), '@color/fluent_on_accent')
                    self.assertEqual(button.get('colorTextDisable'), '@color/fluent_text_disabled')
                main = parse(root / 'xml/dlg_main.xml')
                self.assertEqual(main.find('root').get('colorBkgnd'), '@color/fluent_window_bg')
                self.assertEqual(main.find('root').get('colorText'), '@color/fluent_text')
                self.assertEqual(main.find('.//combobox').get('colorBorder'), '@color/fluent_border')
                self.assertEqual(main.find(".//*[@name='title']").get('colorText'), '@color/fluent_text')
                self.assertEqual(main.find(".//*[@name='info']").get('colorText'), '@color/fluent_text_secondary')
                modal = parse(root / 'xml/model_view.xml')
                self.assertEqual(modal.get('colorBkgnd'), 'rgba(0,0,0,0.5)')
                view = modal.find('modalview')
                self.assertEqual(view.get('colorBkgnd'), '@color/fluent_card_bg')
                for attr, value in {'size': '-2,-1', 'pos': '0,0,6', 'offset': '0,-1',
                                    'enterAnimation': 'anim:modal_in', 'exitAnimation': 'anim:modal_out'}.items():
                    self.assertEqual(view.get(attr), value)
                self.assertTrue(all(n.get('size') == '-2,80' for n in view.findall('button')))
                svg = parse(root / 'svg/btn_primary.svg')
                rects = list(svg)
                self.assertEqual(svg.get('viewBox'), '0 0 200 20')
                self.assertEqual([n.get('fill').upper() for n in rects],
                                 ['#0067C0', '#1975C5', '#3183CA', '#EAEAEA'])
                self.assertEqual([n.get('x') for n in rects], ['0', '50', '100', '150'])
                self.assertTrue(all(n.get('width') == '50' and n.get('height') == '20' and
                                    n.get('rx') == '3' and n.get('ry') == '3' for n in rects))

    def test_screenpin_control_states_and_shadow_geometry(self):
        root = parse(uires('screenpin') / 'values/skin.xml')
        expected = {
            'sp_panel': ('#FFFFFF', '#FFFFFF', '#FFFFFF', '#F3F3F3'),
            'sp_field': ('#FFFFFF', '#F5F5F5', '#EBEBEB', '#EAEAEA'),
            'sp_button': ('#00000000', '#F5F5F5', '#EBEBEB', '#00000000'),
            'sp_active': ('#0067C026', '#0067C026', '#0067C026', '#00000000'),
            'sp_menu_item': ('#00000000', '#F5F5F5', '#EBEBEB', '#00000000'),
        }
        for name, colors in expected.items():
            skin = root.find(f"*[@name='{name}']")
            self.assertEqual(tuple(skin.get(a).upper() for a in
                                   ('normal', 'hover', 'pushdown', 'disable')), colors)
            if name in ('sp_panel', 'sp_field'):
                self.assertTrue(all(skin.get(a).upper() == '#E5E5E5' for a in
                                    ('normalBorder', 'hoverBorder', 'pushdownBorder', 'disableBorder')))
        thumb = root.find("*[@name='sp_slider_thumb']")
        self.assertTrue(all(n.get('color').upper() == '#0067C0' for n in thumb.iter('stroke')))
        self.assertEqual(root.find("*[@name='sp_shadow']").get('margin'), '22,20,22,26')
        self.assertEqual(root.find("*[@name='sp_popup_surface']").get('margin'), '20,18,20,24')

    def test_indexed_files_exist(self):
        for project in PROJECTS:
            root = uires(project)
            for group in parse(root / 'uires.idx'):
                names = set()
                for resource in group:
                    name = resource.get('name', '').casefold()
                    self.assertNotIn(name, names, f'{project}: {group.tag}:{name}')
                    names.add(name)
                    self.assertTrue((root / resource.get('path', '').replace('\\', '/')).is_file(),
                                    f'{project}: {resource.attrib}')

    def test_contract_catches_business_mutations(self):
        text = '<SOUI translucent="1"><root><button id="42" name="save" text="Save" on_command="save()"/></root></SOUI>'
        original = behavior(text)
        self.assertEqual(original, behavior(text.replace('<root>', '<root colorBkgnd="#202020">')))
        for a, b in (('id="42"', 'id="43"'), ('Save', 'Delete'), ('save()', 'delete()'),
                     ('translucent="1"', 'translucent="0"')):
            self.assertNotEqual(original, behavior(text.replace(a, b)))

    def test_theme_label_migration_preserves_only_the_named_action(self):
        original = '<imgbtn name="fluent_theme_toggle" skin="fluent_btn_icon_theme" text="浅 / 深"/>'
        migrated = original.replace('text=', 'tip=')
        self.assertEqual(behavior(original), behavior(migrated))
        self.assertNotEqual(behavior(original), behavior(migrated.replace('浅 / 深', 'other')))
        self.assertNotEqual(behavior(original), behavior(migrated.replace('fluent_theme_toggle', 'save')))
        self.assertNotEqual(behavior(original), behavior(migrated.replace('fluent_btn_icon_theme', 'other')))
        private = original.replace('浅 / 深', '@string/ue_theme_toggle').replace(
            'skin="fluent_btn_icon_theme"', 'skin="ue_fluent_btn_icon_theme"')
        self.assertEqual(behavior(private), behavior(private.replace('text=', 'tip=')))


if __name__ == '__main__':
    unittest.main()
