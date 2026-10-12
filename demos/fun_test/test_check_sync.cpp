#include <souistd.h>
#include <core/SWnd.h>
#include <core/SWndContainerImpl.h>
#include <res.mgr/SUiDef.h>
#include <control/SListCtrl.h>
#include <control/STreeCtrl.h>
#include <gtest/gtest.h>
#include "common.h"
#include "test_container.h"

using namespace SOUI;

// ---------------------------------------------------------------------------
// Multi-selection checkbox sync: clicking the checkbox of a selected item
// applies its new check state to every selected item (Explorer semantics).
//   SListCtrl: selection and checkbox are two independent states (the
//   selection record lives in the control-level m_arrSelFlags array next to
//   the hover state, DXLVITEM.bCheck is the checkbox); a checkbox click
//   flips only check flags (multiSel syncs the whole selection;
//   checkbox-only mode keeps the independent toggle) and never moves the
//   selection or the cursor.
//   STreeCtrl: the selection lives in the rubber-band map; a checkbox click
//   on a selected item syncs every item of the map (children cascade, parents
//   recompute), a checkbox click on an unselected item keeps toggling only
//   that item.
// ---------------------------------------------------------------------------

namespace
{
    // Minimal checkSkin stand-in with a fixed 13x13 hit area. Headless builds
    // have no system skin resources (HeadlessApp only injects the scrollbar
    // skin), so the builtin checkbox skins would be NULL and neither
    // SListCtrl::HitCheckBox nor STreeCtrl::ItemHitTest could report a
    // checkbox hit.
    class SFakeCheckSkin : public SSkinObjBase
    {
        DEF_SOBJECT(SSkinObjBase, L"testcheckbox")

      public:
        STDMETHOD_(SIZE, GetSkinSize)(THIS) SCONST OVERRIDE
        {
            SIZE sz = { 13, 13 };
            return sz;
        }

      protected:
        void _DrawByIndex(IRenderTarget *pRT, LPCRECT prcDraw, int iState, BYTE byAlpha) const override
        {
        }
    };

    /**
     * @brief Register the fake checkbox skins in the builtin skin pool.
     * @note  Must run after HeadlessApp (needs the SApplication singleton)
     *        and before the control is created: the control constructor
     *        grabs GETBUILTINSKIN(SKIN_SYS_*_CHECKBOX) exactly once. The
     *        fake class needs an explicit factory registration, otherwise
     *        CreateSkinByName cannot instantiate it. SSkinPool dedupes by
     *        (name, scale), so repeated calls are no-ops.
     */
    void EnsureCheckSkins()
    {
        SApplication::getSingleton().RegisterSkinClass<SFakeCheckSkin>();
        static const wchar_t kSkinXml[] = L"<skin>"
                                          L"<testcheckbox name=\"_skin.sys.checkbox\"/>"
                                          L"<testcheckbox name=\"_skin.sys.tree.checkbox\"/>"
                                          L"</skin>";
        SXmlDoc doc;
        EXPECT_TRUE(doc.load_buffer(kSkinXml, sizeof(kSkinXml) - sizeof(wchar_t), xml_parse_default, WCHAR_ENC));
        SXmlNode skinNode = doc.root().child(L"skin");
        EXPECT_TRUE(skinNode);
        if (skinNode)
            GETUIDEF->GetBuiltinSkinPool()->LoadSkins(&skinNode);
    }

    /** Counts per-item selection state flips (SListCtrl::NotifyItemSelState). */
    struct SelStateSpy
    {
        int count;
        int lastItem;
        int lastState; // 0 deselected, 1 selected
        SelStateSpy()
            : count(0)
            , lastItem(-1)
            , lastState(-1)
        {
        }
        BOOL onItem(EventItemSelChanged *pEvt)
        {
            count++;
            lastItem = pEvt->iItem;
            lastState = pEvt->bSelected ? 1 : 0;
            return TRUE;
        }
    };

    /** Counts per-item checkbox flips (EventItemCheckState, SListCtrl). */
    struct LcCheckSpy
    {
        int count;
        int lastItem;
        int lastCheck; // 0 unchecked, 1 checked
        LcCheckSpy()
            : count(0)
            , lastItem(-1)
            , lastCheck(-1)
        {
        }
        BOOL onCheck(EventItemCheckState *pEvt)
        {
            count++;
            lastItem = pEvt->iItem;
            lastCheck = pEvt->bCheck ? 1 : 0;
            return TRUE;
        }
    };

    /** Counts EventTCCheckState firings (STreeCtrl::ItemLButtonUp). */
    struct CheckStateSpy
    {
        int count;
        CheckStateSpy()
            : count(0)
        {
        }
        BOOL onCheck(EventTCCheckState *pEvt)
        {
            count++;
            return TRUE;
        }
    };
}

// ---------------------------------------------------------------------------
// SListCtrl: selection and checkbox are independent states. With multiSel on,
// clicking the checkbox of a selected item applies its new check state to
// every selected item while the selection set and the cursor stay untouched;
// clicking the checkbox of an unselected item toggles only that item.
// ---------------------------------------------------------------------------
TEST(soui_check_sync, listctrl_checkbox_click_syncs_selected_items)
{
    HeadlessApp app;
    EnsureCheckSkins();
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <listctrl name=\"lc\" size=\"200,200\" multiSel=\"1\" checkBox=\"1\" headerHeight=\"20\" itemHeight=\"20\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"  </listctrl>"
        L"</window>"));
    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName(L"lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    for (int i = 0; i < 5; i++)
        pLc->InsertItem(-1, _T("item"));
    pLc->Move(CRect(0, 0, 200, 200));

    SelStateSpy selSpy;
    ASSERT_TRUE(pLc->GetEventSet()->subscribeEvent(&SelStateSpy::onItem, &selSpy));
    LcCheckSpy chkSpy;
    ASSERT_TRUE(pLc->GetEventSet()->subscribeEvent(&LcCheckSpy::onCheck, &chkSpy));

    // Build the selection: select row 1, then Ctrl+click row 3 (x=100 is
    // beyond the checkbox column at x=[7,20]; rows are 20px tall below the
    // 20px header, so row i spans y=[20+20*i, 40+20*i)). Selecting must not
    // check anything.
    pLc->SetSelectedItem(1);
    EXPECT_TRUE(pLc->GetSelected(1));
    EXPECT_FALSE(pLc->GetCheck(1)) << L"selecting must not flip the check state";
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(100, 90));
    pLc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(100, 90));
    EXPECT_TRUE(pLc->GetSelected(3));
    EXPECT_EQ(2, pLc->GetSelectedCount());
    EXPECT_EQ(0, pLc->GetCheckedItemCount()) << L"selection must not flip the check state";
    EXPECT_EQ(2, selSpy.count);

    // Click the checkbox of the selected row 1 (x=10 hits the 13px checkbox;
    // y=50 is row 1's center): its new state (checked) must propagate to
    // every selected item, while the selection set and the cursor stay put.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(10, 50));
    EXPECT_TRUE(pLc->GetCheck(1));
    EXPECT_TRUE(pLc->GetCheck(3)) << L"selected items must follow the clicked item's new check state";
    EXPECT_FALSE(pLc->GetCheck(0));
    EXPECT_FALSE(pLc->GetCheck(4));
    EXPECT_EQ(2, pLc->GetSelectedCount()) << L"a checkbox click must not change the selection";
    EXPECT_TRUE(pLc->GetSelected(1));
    EXPECT_TRUE(pLc->GetSelected(3));
    EXPECT_EQ(3, pLc->GetSelectedItem()) << L"a checkbox click must not move the keyboard cursor";
    EXPECT_EQ(2, selSpy.count) << L"a checkbox click must not fire selection events";
    EXPECT_EQ(2, chkSpy.count);
    EXPECT_EQ(3, chkSpy.lastItem);
    EXPECT_EQ(1, chkSpy.lastCheck);
    pLc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(10, 50));

    // Clicking again syncs the whole selection back to unchecked.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(10, 50));
    EXPECT_FALSE(pLc->GetCheck(1));
    EXPECT_FALSE(pLc->GetCheck(3));
    EXPECT_EQ(2, pLc->GetSelectedCount());
    EXPECT_EQ(4, chkSpy.count);
    pLc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(10, 50));

    // The checkbox of an unselected item toggles only that item.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(10, 30));
    EXPECT_TRUE(pLc->GetCheck(0));
    EXPECT_FALSE(pLc->GetCheck(1));
    EXPECT_FALSE(pLc->GetCheck(3));
    EXPECT_EQ(2, pLc->GetSelectedCount());
    EXPECT_EQ(5, chkSpy.count);
    EXPECT_EQ(0, chkSpy.lastItem);
    EXPECT_EQ(2, selSpy.count);
    pLc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(10, 30));

    // A plain click replaces the selection without touching the check state.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(100, 70));
    EXPECT_EQ(1, pLc->GetSelectedCount());
    EXPECT_TRUE(pLc->GetSelected(2));
    EXPECT_TRUE(pLc->GetCheck(0)) << L"selection must not flip the check state";
    EXPECT_EQ(5, selSpy.count) << L"1 build + row 1 deselect + row 3 deselect + row 2 select";
    EXPECT_EQ(5, chkSpy.count);
    pLc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(100, 70));

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SListCtrl: MFC CListCtrl-style state APIs. The selection record lives in
// the control-level flag array (never inside DXLVITEM): GetItemState /
// SetItemState read and write the S_LVIS_ bits, GetSelItems fills the caller
// buffer in ascending order, and GetFirstSelectedItemPosition /
// GetNextSelectedItem walk the selection like MFC's POSITION iteration.
// ---------------------------------------------------------------------------
TEST(soui_check_sync, listctrl_mfc_style_state_apis)
{
    HeadlessApp app;
    EnsureCheckSkins();
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <listctrl name=\"lc\" size=\"200,200\" multiSel=\"1\" checkBox=\"1\" headerHeight=\"20\" itemHeight=\"20\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"  </listctrl>"
        L"</window>"));
    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName(L"lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    for (int i = 0; i < 6; i++)
        pLc->InsertItem(-1, _T("item"));
    pLc->Move(CRect(0, 0, 200, 200));

    // Empty state: no selected bits, iteration yields nothing.
    EXPECT_EQ(0u, pLc->GetSelectedCount());
    EXPECT_TRUE(pLc->GetFirstSelectedItemPosition() == NULL);
    EXPECT_EQ(0u, pLc->GetItemState(2, S_LVIS_SELECTED | S_LVIS_FOCUSED));

    // SetItemState builds the selection set and moves the cursor.
    pLc->SetItemState(1, S_LVIS_SELECTED, S_LVIS_SELECTED);
    pLc->SetItemState(4, S_LVIS_SELECTED, S_LVIS_SELECTED);
    pLc->SetItemState(4, S_LVIS_FOCUSED, S_LVIS_FOCUSED);
    EXPECT_EQ(2u, pLc->GetSelectedCount());
    EXPECT_EQ(S_LVIS_SELECTED, pLc->GetItemState(1, S_LVIS_SELECTED | S_LVIS_FOCUSED));
    EXPECT_EQ(S_LVIS_SELECTED | S_LVIS_FOCUSED, pLc->GetItemState(4, S_LVIS_SELECTED | S_LVIS_FOCUSED));
    EXPECT_EQ(0u, pLc->GetItemState(1, S_LVIS_FOCUSED)) << L"the cursor moved to item 4 only";

    // GetSelItems fills the buffer in ascending order and reports the count.
    int items[6] = { 0 };
    EXPECT_EQ(2, pLc->GetSelItems(6, items));
    EXPECT_EQ(1, items[0]);
    EXPECT_EQ(4, items[1]);
    EXPECT_EQ(1, pLc->GetSelItems(1, items)) << L"the count is capped by nMaxItems";
    EXPECT_EQ(1, items[0]);

    // POSITION iteration mirrors MFC: walk 1 then 4, then the cursor is NULL.
    SPOSITION pos = pLc->GetFirstSelectedItemPosition();
    ASSERT_TRUE(pos != NULL);
    EXPECT_EQ(1, pLc->GetNextSelectedItem(pos));
    EXPECT_EQ(4, pLc->GetNextSelectedItem(pos));
    EXPECT_TRUE(pos == NULL);
    EXPECT_EQ(-1, pLc->GetNextSelectedItem(pos));

    // Clearing a state bit removes the item from the selection.
    pLc->SetItemState(1, 0, S_LVIS_SELECTED);
    EXPECT_EQ(1u, pLc->GetSelectedCount());
    EXPECT_FALSE(pLc->GetSelected(1));
    EXPECT_TRUE(pLc->GetSelected(4));

    // GetCheck / SetCheck are the MFC aliases of the independent checkbox
    // state: neither direction touches the selection record.
    EXPECT_FALSE(pLc->GetCheck(2));
    EXPECT_TRUE(pLc->SetCheck(2, TRUE));
    EXPECT_TRUE(pLc->GetCheck(2));
    EXPECT_EQ(1u, pLc->GetSelectedCount()) << L"the checkbox state must not touch the selection";
    pLc->SetItemState(2, S_LVIS_SELECTED, S_LVIS_SELECTED);
    EXPECT_TRUE(pLc->GetCheck(2)) << L"the selection must not touch the checkbox state";
    EXPECT_EQ(2u, pLc->GetSelectedCount());

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SListCtrl: Shift+Click replaces the anchored range, but only the rows whose
// state actually changes are flipped: growing the range fires EventItemSelChanged
// for the newly added rows only, shrinking for the removed rows only; rows
// already in the desired state are never touched (no redraw, no event).
// ---------------------------------------------------------------------------
TEST(soui_check_sync, listctrl_shift_range_only_flips_changed_rows)
{
    HeadlessApp app;
    EnsureCheckSkins();
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <listctrl name=\"lc\" size=\"200,200\" multiSel=\"1\" headerHeight=\"20\" itemHeight=\"20\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"  </listctrl>"
        L"</window>"));
    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName(L"lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    for (int i = 0; i < 8; i++)
        pLc->InsertItem(-1, _T("item"));
    pLc->Move(CRect(0, 0, 200, 200));

    SelStateSpy selSpy;
    ASSERT_TRUE(pLc->GetEventSet()->subscribeEvent(&SelStateSpy::onItem, &selSpy));

    // Anchor on row 1, then Shift+click row 4 (row centers: y = 20 + 20*i + 10;
    // x=100 is beyond the checkbox column): rows 2..4 are added, row 1 was
    // already selected and must not fire again.
    pLc->SetSelectedItem(1);
    EXPECT_EQ(1, selSpy.count);
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_SHIFT, MAKELPARAM(100, 110));
    pLc->SSendMessage(WM_LBUTTONUP, MK_SHIFT, MAKELPARAM(100, 110));
    EXPECT_EQ(4, pLc->GetSelectedCount());
    EXPECT_EQ(4, selSpy.count) << L"1 anchor select + only the added rows 2..4, row 1 not re-flipped";
    EXPECT_EQ(4, selSpy.lastItem);
    EXPECT_EQ(1, selSpy.lastState);

    // Shift+click row 7 grows [1..4] to [1..7]: only rows 5..7 flip; rows
    // 1..4 are already in the desired state and must stay untouched.
    selSpy.count = 0;
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_SHIFT, MAKELPARAM(100, 170));
    pLc->SSendMessage(WM_LBUTTONUP, MK_SHIFT, MAKELPARAM(100, 170));
    EXPECT_EQ(7, pLc->GetSelectedCount());
    EXPECT_TRUE(pLc->GetSelected(1));
    EXPECT_TRUE(pLc->GetSelected(4));
    EXPECT_EQ(3, selSpy.count) << L"rows 1..4 must not be re-flipped when the range grows";
    EXPECT_EQ(7, selSpy.lastItem);
    EXPECT_EQ(1, selSpy.lastState);

    // Shift+click row 3 shrinks [1..7] back to [1..3]: only the removed rows
    // 4..7 flip, each exactly once.
    selSpy.count = 0;
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_SHIFT, MAKELPARAM(100, 90));
    pLc->SSendMessage(WM_LBUTTONUP, MK_SHIFT, MAKELPARAM(100, 90));
    EXPECT_EQ(3, pLc->GetSelectedCount());
    EXPECT_TRUE(pLc->GetSelected(1));
    EXPECT_TRUE(pLc->GetSelected(3));
    EXPECT_FALSE(pLc->GetSelected(4));
    EXPECT_FALSE(pLc->GetSelected(7));
    EXPECT_EQ(4, selSpy.count) << L"shrinking the range fires only for the removed rows 4..7";
    EXPECT_EQ(7, selSpy.lastItem);
    EXPECT_EQ(0, selSpy.lastState);

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SListCtrl: with multiSel off but checkBox on, checkbox clicks keep the
// independent-toggle behavior (no sync) - the sync is multi-selection-only,
// and the single selection does not follow the checkbox either.
// ---------------------------------------------------------------------------
TEST(soui_check_sync, listctrl_checkbox_without_multiselection_toggles_only_clicked)
{
    HeadlessApp app;
    EnsureCheckSkins();
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <listctrl name=\"lc\" size=\"200,200\" checkBox=\"1\" headerHeight=\"20\" itemHeight=\"20\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"  </listctrl>"
        L"</window>"));
    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName(L"lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    for (int i = 0; i < 4; i++)
        pLc->InsertItem(-1, _T("item"));
    pLc->Move(CRect(0, 0, 200, 200));

    LcCheckSpy chkSpy;
    ASSERT_TRUE(pLc->GetEventSet()->subscribeEvent(&LcCheckSpy::onCheck, &chkSpy));

    // Check rows 1 and 2 programmatically.
    EXPECT_TRUE(pLc->SetCheck(1, TRUE));
    EXPECT_TRUE(pLc->SetCheck(2, TRUE));
    EXPECT_TRUE(pLc->GetCheck(1));
    EXPECT_TRUE(pLc->GetCheck(2));
    EXPECT_EQ(-1, pLc->GetSelectedItem()) << L"checking must not move the selection";
    EXPECT_EQ(2, chkSpy.count);

    // A checkbox click on the checked row 1 unchecks only that item
    // (y=50 targets row 1) and leaves the (empty) selection alone.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(10, 50));
    EXPECT_FALSE(pLc->GetCheck(1));
    EXPECT_TRUE(pLc->GetCheck(2)) << L"without multiSel the other items must keep their check state";
    EXPECT_EQ(-1, pLc->GetSelectedItem());
    EXPECT_EQ(3, chkSpy.count);
    pLc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(10, 50));

    // A plain click selects row 0 without touching the check state.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(100, 30));
    pLc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(100, 30));
    EXPECT_EQ(0, pLc->GetSelectedItem());
    EXPECT_FALSE(pLc->GetCheck(0)) << L"selection must not flip the check state";
    EXPECT_TRUE(pLc->GetCheck(2));
    EXPECT_EQ(3, chkSpy.count);

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// STreeCtrl: with multiSel on, clicking the checkbox of a selected item syncs
// every selected item; clicking the checkbox of an unselected item selects it
// and toggles only it.
// ---------------------------------------------------------------------------
TEST(soui_check_sync, treectrl_checkbox_click_syncs_selected_items)
{
    HeadlessApp app;
    EnsureCheckSkins();
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" multiSel=\"1\" itemHeight=\"20\" checkBox=\"1\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);

    SArray<HSTREEITEM> items;
    for (int i = 0; i < 5; i++)
    {
        SStringT str;
        str.Format(_T("item%d"), i);
        items.Add(pTc->InsertItem(str, STVI_ROOT, STVI_LAST));
    }
    pTc->Move(CRect(0, 0, 200, 200));

    CheckStateSpy chkSpy;
    ASSERT_TRUE(pTc->GetEventSet()->subscribeEvent(&CheckStateSpy::onCheck, &chkSpy));

    // Build the selection with Ctrl+clicks on the text area (x=60 is beyond
    // the 13px checkbox column; headless has no toggle skin/lines, so leaf
    // rows expose the checkbox at x=[0,13)).
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(60, 30));
    pTc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(60, 30));
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(60, 70));
    pTc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(60, 70));
    EXPECT_EQ(2, pTc->GetSelItemCount());
    EXPECT_TRUE(pTc->IsItemSelected(items[1]));
    EXPECT_TRUE(pTc->IsItemSelected(items[3]));

    // Click the checkbox of the selected item 1 (x=6): both selected items
    // become checked and the selection set survives.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(6, 30));
    EXPECT_EQ(STVICheckBox_Checked, pTc->GetCheckState(items[1]));
    EXPECT_EQ(STVICheckBox_Checked, pTc->GetCheckState(items[3])) << L"selected items must follow the clicked item's new state";
    EXPECT_EQ(STVICheckBox_UnChecked, pTc->GetCheckState(items[0]));
    EXPECT_EQ(STVICheckBox_UnChecked, pTc->GetCheckState(items[4]));
    EXPECT_EQ(2, pTc->GetSelItemCount()) << L"a checkbox click must not change the selection set";
    pTc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(6, 30));
    EXPECT_EQ(1, chkSpy.count) << L"the click fires one CheckState event for the clicked item";

    // Clicking again syncs the whole selection back to unchecked.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(6, 30));
    EXPECT_EQ(STVICheckBox_UnChecked, pTc->GetCheckState(items[1]));
    EXPECT_EQ(STVICheckBox_UnChecked, pTc->GetCheckState(items[3]));
    EXPECT_EQ(2, pTc->GetSelItemCount());
    pTc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(6, 30));

    // The checkbox of an unselected item selects it and toggles only it.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(6, 50));
    EXPECT_EQ(STVICheckBox_Checked, pTc->GetCheckState(items[2]));
    EXPECT_EQ(STVICheckBox_UnChecked, pTc->GetCheckState(items[1]));
    EXPECT_EQ(STVICheckBox_UnChecked, pTc->GetCheckState(items[3]));
    EXPECT_EQ(1, pTc->GetSelItemCount());
    EXPECT_TRUE(pTc->IsItemSelected(items[2]));
    pTc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(6, 50));

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// STreeCtrl: the sync keeps the parent/child checkbox cascade - syncing a
// selected parent checks its selected child through the normal
// SetCheckState propagation.
// ---------------------------------------------------------------------------
TEST(soui_check_sync, treectrl_checkbox_sync_cascades_children)
{
    HeadlessApp app;
    EnsureCheckSkins();
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" multiSel=\"1\" itemHeight=\"20\" checkBox=\"1\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);

    HSTREEITEM hParent = pTc->InsertItem(_T("parent"), STVI_ROOT, STVI_LAST);
    HSTREEITEM hChild = pTc->InsertItem(_T("child"), hParent, STVI_LAST);
    pTc->InsertItem(_T("sibling"), STVI_ROOT, STVI_LAST);
    pTc->Move(CRect(0, 0, 200, 200));

    // Select parent (row 0) and child (row 1, indented by 18px but the
    // item-relative checkbox stays at x=[0,13)).
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(60, 10));
    pTc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(60, 10));
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(60, 30));
    pTc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(60, 30));
    EXPECT_EQ(2, pTc->GetSelItemCount());

    // Checking the parent's checkbox checks both (child via the cascade).
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(6, 10));
    EXPECT_EQ(STVICheckBox_Checked, pTc->GetCheckState(hParent));
    EXPECT_EQ(STVICheckBox_Checked, pTc->GetCheckState(hChild));
    EXPECT_EQ(2, pTc->GetSelItemCount());
    pTc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(6, 10));

    // Unchecking syncs the whole selection down again.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(6, 10));
    EXPECT_EQ(STVICheckBox_UnChecked, pTc->GetCheckState(hParent));
    EXPECT_EQ(STVICheckBox_UnChecked, pTc->GetCheckState(hChild));
    pTc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(6, 10));

    root.DestroyAllChildren();
}
