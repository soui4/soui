"""Exact caption migrations; frozen signatures still reject business mutations."""
import copy
import hashlib
import json
import unittest

from check_contracts import preserved_dimension
from test_gallery_alignment import BASELINE, REPO, behavior, layouts
from titlebar_contract_migrations import (COMPANION, DEMO2, LAYERED, WX_MAIN,
                                         WX_PAGES, migrated_behavior, validate_companion,
                                         validate_companion_creation, wx_relocation)


class CaptionGrowthTest(unittest.TestCase):
    def test_only_exact_dialog_height_migrations_are_allowed(self):
        cases = (("dlg_formatmsg", "host", "300", "308"),
                 ("dlg_skinset", "root", "500", "508"))
        for dialog, scope, old, new in cases:
            path = f"demos/demo/uires/xml/{dialog}.xml"
            self.assertTrue(preserved_dimension(path, scope, "height", old, old))
            self.assertTrue(preserved_dimension(path, scope, "height", old, new))
            self.assertFalse(preserved_dimension(path, scope, "height", old, str(int(new) + 1)))
            self.assertFalse(preserved_dimension(path, scope, "width", old, new))
            self.assertFalse(preserved_dimension(path.replace(dialog, "other"), scope, "height", old, new))
            self.assertFalse(preserved_dimension(path, "other", "height", old, new))

    def test_main_height_growth_is_exact_and_directional(self):
        for path, scope, old, new in ((DEMO2, 'root', '400', '448'),
                                      (WX_MAIN, 'host', '640', '684')):
            self.assertTrue(preserved_dimension(path, scope, 'height', old, new))
            for p, s, a, o, n in ((path, scope, 'height', new, old),
                                  (path, scope, 'height', old, str(int(new)+1)),
                                  (path, scope, 'width', old, new),
                                  (path, 'other', 'height', old, new),
                                  (path.replace('dlg_main', 'other'), scope, 'height', old, new)):
                self.assertFalse(preserved_dimension(p, s, a, o, n))


class MigrationGuardTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.documents = {p.relative_to(REPO).as_posix(): p.read_text(encoding='utf-8-sig') for p in layouts()}
        cls.baseline = json.loads(BASELINE.read_text(encoding='utf-8'))

    def digest(self, path, documents):
        value = migrated_behavior(path, documents, behavior)
        return hashlib.sha256(json.dumps(value, sort_keys=True, ensure_ascii=False).encode()).hexdigest()

    def rejected(self, path, old, new):
        docs = copy.copy(self.documents)
        self.assertIn(old, docs[path])
        docs[path] = docs[path].replace(old, new, 1)
        try:
            actual = self.digest(path, docs)
        except ValueError:
            return
        self.assertNotEqual(actual, self.baseline[path])

    def test_approved_migrations_match_unchanged_baselines(self):
        for path in (*LAYERED, WX_MAIN, *WX_PAGES):
            with self.subTest(path=path):
                self.assertEqual(self.digest(path, self.documents), self.baseline[path])

    def test_layered_permission_is_local_and_one_way(self):
        for path in LAYERED:
            self.rejected(path, 'translucent="1"', 'translucent="0"')
            self.rejected(path, 'resizable="1"', 'resizable="0"')
        other = 'demos/uieditor/uires/xml/dlg_main.xml'
        self.rejected(other, '<SOUI ', '<SOUI translucent="1" ')

    def test_unlisted_paths_get_no_migration_normalization(self):
        for path, text in self.documents.items():
            if path not in (*LAYERED, WX_MAIN, *WX_PAGES):
                with self.subTest(path=path):
                    self.assertEqual(migrated_behavior(path, self.documents, behavior), behavior(text))
        # Even an identical document under another path gets no host or label
        # exception. Permissions are exact paths, not demo/name/tag heuristics.
        docs = dict(self.documents)
        alias = DEMO2.replace('dlg_main', 'other')
        docs[alias] = docs[DEMO2]
        self.assertEqual(migrated_behavior(alias, docs, behavior), behavior(docs[alias]))
        self.assertNotEqual(self.digest(alias, docs), self.baseline[DEMO2])

    def test_wx_rejects_actions_moved_outside_caption_or_wrong_tag(self):
        import xml.etree.ElementTree as ET
        for mutation in ('outside', 'tag'):
            docs = dict(self.documents)
            root = ET.fromstring(docs[WX_MAIN])
            action = root.find(".//*[@name='btn_close']")
            if mutation == 'tag':
                action.tag = 'button'
            else:
                parent = next(n for n in root.iter() if action in list(n))
                parent.remove(action)
                root.find("root/window[@name='main_content']").append(action)
            docs[WX_MAIN] = ET.tostring(root, encoding='unicode')
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):
                wx_relocation(docs)

    def test_demo2_system_and_theme_identity_and_events_stay_locked(self):
        for old, new in (('id="sysid_min"', 'id="sysid_other"'),
                         ('id="65000"', 'id="65001"'),
                         ('tip="Light / Dark"', 'tip="Wrong"'),
                         ('name="btn_add"', 'name="btn_delete"'),
                         ('text="Add"', 'text="Delete"'),
                         ('name="btn_add"', 'name="btn_add" on_command="delete()"')):
            self.rejected(DEMO2, old, new)

    def test_wx_aggregate_rejects_lost_duplicate_retagged_and_rebound_actions(self):
        for old, new in (('name="btn_close"', 'name="lost_close"'),
                         ('name="btn_close"', 'name="btn_min"'),
                         ('name="btn_skin"', 'name="btn_skin" id="99"'),
                         ('name="btn_menu"', 'name="btn_menu" on_command="other()"'),
                         ('skin="skin_common_min"', 'skin="wrong"')):
            self.rejected(WX_MAIN, old, new)
        docs = copy.copy(self.documents)
        docs[WX_PAGES[0]] = docs[WX_PAGES[0]].replace('<include>', '<include><imgbtn name="btn_close"/>', 1)
        with self.assertRaises(ValueError):
            wx_relocation(docs)
        docs = copy.copy(self.documents)
        # Moving a shared action back to one page must not satisfy another page.
        docs[WX_MAIN] = docs[WX_MAIN].replace('name="btn_close"', 'name="missing"', 1)
        with self.assertRaises(ValueError):
            migrated_behavior(WX_PAGES[1], docs, behavior)
        self.rejected(WX_PAGES[0], 'name="edit_msg_search"', 'name="deleted_search"')
        self.rejected(WX_PAGES[1], 'name="btn_accept"', 'name="btn_accept" on_command="other()"')

    def test_companion_permits_no_business_nodes_events_or_host_changes(self):
        text = self.documents[COMPANION]
        validate_companion(text)
        for old, new in (('appWnd="0"', 'appWnd="1"'),
                         ('resizable="0"', 'resizable="1"'),
                         ('translucent="1"', 'translucent="0"'),
                         ('cache="1"', 'cache="1" on_command="run()"'),
                         ('ue_fluent_window_shadow', 'fluent_window_shadow'),
                         ('</SOUI>', '<button name="save"/></SOUI>')):
            with self.subTest(mutation=new), self.assertRaises(ValueError):
                validate_companion(text.replace(old, new, 1))

    def test_companion_createex_preserves_exact_creation_boundaries(self):
        source = (REPO / 'demos/uieditor/EditorChrome.h').read_text(encoding='utf-8-sig')
        validate_companion_creation(source)
        mutations = (
            ('CreateEx(NULL,', 'CreateEx(hwnd,'),
            ('CreateEx(NULL,', 'Create(NULL,'),
            ('WS_POPUP | WS_DISABLED', 'WS_POPUP'),
            ('WS_POPUP | WS_DISABLED', 'WS_CHILD | WS_DISABLED'),
            ('WS_EX_TRANSPARENT | ', ''),
            ('WS_EX_NOACTIVATE | ', ''),
            ('WS_EX_TOOLWINDOW,', 'WS_EX_APPWINDOW,'),
            ('0, 0, 0, 0)', '0, 0, 100, 100)'),
        )
        for old, new in mutations:
            self.assertIn(old, source)
            with self.subTest(mutation=new), self.assertRaises(ValueError):
                validate_companion_creation(source.replace(old, new, 1))
        safe = ('CreateEx(NULL, WS_POPUP | WS_DISABLED, WS_EX_TRANSPARENT | '
                'WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW, 0, 0, 0, 0)')
        for text in (source + '\n' + safe,
                     source + '\nCreateEx(GetOwner(), WS_POPUP, 0, 0, 0, 100, 100);',
                     '// ' + safe + '\nCreateEx(hwnd, WS_POPUP, 0, 0, 0, 100, 100);',
                     '/* ' + safe + ' */'):
            with self.subTest(source=text[-100:]), self.assertRaises(ValueError):
                validate_companion_creation(text)


if __name__ == "__main__":
    unittest.main()
