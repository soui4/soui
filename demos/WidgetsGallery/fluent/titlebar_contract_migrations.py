"""Explicit migrations back to frozen business signatures, never new baselines."""
from collections import Counter
import re
import xml.etree.ElementTree as ET

WX_MAIN = 'demos/wxDemo/uires/xml/dlg_main.xml'
WX_PAGES = tuple(f'demos/wxDemo/uires/xml/page_{p}.xml' for p in ('msg', 'contact', 'collect'))
WX_ACTIONS = ('btn_skin', 'btn_menu', 'btn_min', 'btn_max', 'btn_restore', 'btn_close')
COMPANION = 'demos/uieditor/uires/xml/window_shadow.xml'
DEMO2 = 'demos/demo2/uires/xml/dlg_main.xml'
LAYERED = {DEMO2: '0', 'demos/gridctrl_demo/uires/layout/dlg_main.xml': None,
           'demos/sci_demo/uires/layout/sci.demo.xml': None}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def wx_relocation(documents):
    """One actual action per name in main; none left in any included page."""
    main = ET.fromstring(documents[WX_MAIN])
    caption = main.find("root/caption[@name='titlebar']")
    require(caption is not None and caption.get('pos') == '0,0,-0,@48dp', 'wx caption')
    require(main.find("root/window[@name='main_content']") is not None, 'wx body')
    for name in WX_ACTIONS:
        nodes = [n for n in main.iter() if n.get('name') == name]
        require(len(nodes) == 1 and nodes[0] in list(caption.iter()), 'wx missing/duplicate/misplaced ' + name)
        node = nodes[0]
        require(node.tag == 'imgbtn' and not node.get('id') and not node.get('text')
                and not any(k.startswith('on_') for k in node.attrib), 'wx changed action ' + name)
        expected_skin = 'skin_btn_' + name[4:] if name in ('btn_menu', 'btn_skin') else 'skin_common_' + name[4:]
        require(node.get('skin') == expected_skin, 'wx action skin ' + name)
    for path in WX_PAGES:
        root = ET.fromstring(documents[path])
        require(not any(n.get('name') in WX_ACTIONS for n in root.iter()), 'wx duplicate page actions ' + path)
    return Counter(('imgbtn', name, '') for name in WX_ACTIONS)


def validate_companion(text):
    root = ET.fromstring(text)
    require(root.tag == 'SOUI' and root.attrib == {
        'translucent': '1', 'appWnd': '0', 'resizable': '0', 'width': '100', 'height': '100'},
        'unexpected companion host')
    require(len(root) == 1 and root[0].tag == 'root' and root[0].attrib == {
        'skin': 'ue_fluent_window_shadow', 'cache': '1'} and len(root[0]) == 0,
        'companion must contain only the private shadow surface')
    require(not (root.text or '').strip() and not (root[0].text or '').strip()
            and not (root[0].tail or '').strip(), 'companion text')


def validate_companion_creation(source):
    """Static guard for the one approved unowned, input-disabled CreateEx call.

    This checks source arguments, not runtime Win32 behavior. Ignore comments
    so a commented-out safe call cannot cover an unsafe live call.
    """
    source = re.sub(r'/\*.*?\*/|//[^\n]*', '', source, flags=re.S)
    calls = re.findall(r'\b(Create(?:Ex)?)\s*\(([^()]*)\)', source)
    require(len(re.findall(r'\bCreate(?:Ex)?\s*\(', source)) == 1 and len(calls) == 1,
            'companion must have one creation call')
    method, arguments = calls[0]
    compact = re.sub(r'\s+', '', arguments)
    require(method == 'CreateEx' and compact == (
        'NULL,WS_POPUP|WS_DISABLED,'
        'WS_EX_TRANSPARENT|WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW,0,0,0,0'),
        'companion creation must stay unowned, disabled, nonactivating and initially empty')


def migrated_behavior(relative, documents, behavior):
    text = documents[relative]
    result = behavior(text)
    if relative in LAYERED:
        root = ET.fromstring(text)
        require(root.get('translucent') == '1', 'expected layered host ' + relative)
        require(len(result['hosts']) == 1, 'unexpected host count')
        original = LAYERED[relative]
        if original is None:
            del result['hosts'][0]['translucent']
        else:
            result['hosts'][0]['translucent'] = original
    if relative == DEMO2:
        root = ET.fromstring(text)
        require(root.find('ncpainter') is None, 'demo2 still has NC chrome')
        caption = root.find('root/caption')
        require(caption is not None, 'demo2 client caption missing')
        for ident in ('sysid_icon', 'sysid_title', 'sysid_min', 'sysid_max', 'sysid_restore', 'sysid_close'):
            nodes = [n for n in root.iter() if n.get('id') == ident]
            require(len(nodes) == 1 and nodes[0] in list(caption.iter()), 'demo2 system action ' + ident)
        theme = [n for n in root.iter() if n.get('name') == 'fluent_theme_toggle']
        require(len(theme) == 1 and theme[0] in list(caption.iter()) and theme[0].tag == 'imgbtn'
                and theme[0].get('id') == '65000' and theme[0].get('skin') == 'fluent_btn_icon_theme'
                and theme[0].get('tip') == 'Light / Dark' and theme[0].get('text') is None,
                'demo2 theme migration')
        result['labels'] = sorted(result['labels'] + ['Theme'])
    if relative in (WX_MAIN, *WX_PAGES):
        moved = wx_relocation(documents)
        nodes = Counter(result['nodes'])
        if relative == WX_MAIN:
            for item in (*moved, ('caption', 'titlebar', ''), ('window', 'main_content', '')):
                require(nodes[item] == 1, 'wx migration node multiplicity')
                del nodes[item]
        else:
            nodes.update(moved)
        result['nodes'] = sorted(nodes.elements())
    return result
