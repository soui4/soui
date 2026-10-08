#include <souistd.h>
#include <core/SWnd.h>
#include <core/SWndContainerImpl.h>
#include <res.mgr/SUiDef.h>
#include <control/SListCtrl.h>
#include <control/SListView.h>
#include <control/SMCListView.h>
#include <control/STreeCtrl.h>
#include <helper/SAdapterBase.h>
#include <gtest/gtest.h>
#include "common.h"

using namespace SOUI;
#ifdef _WIN32
#define WCHAR_ENC enc_utf16
#else
#define WCHAR_ENC enc_utf32
#endif
// ---------------------------------------------------------------------------
// Headless harness: application + in-memory builtin scrollbar skin + a test
// container so that SPanel-derived controls can be created and receive
// simulated mouse messages without a host window.
//
// SwndContainerImpl is deliberately abstract: drawing / host integration pure
// virtuals (OnRedraw, UpdateWindow, GetMsgLoop, ...) are implemented by real
// hosts such as SHostWnd. TestContainer stubs them out: redraws are no-ops and
// posted tasks run inline so the tests stay deterministic.
// ---------------------------------------------------------------------------
namespace
{
    class TestContainer : public SwndContainerImpl
    {
      public:
        STDMETHOD_(BOOL, OnFireEvent)(IEvtArgs *evt) OVERRIDE
        {
            return FALSE;
        }

        STDMETHOD_(IMessageLoop *, GetMsgLoop)(THIS) OVERRIDE
        {
            return NULL;
        }

        STDMETHOD_(IScriptModule *, GetScriptModule)() OVERRIDE
        {
            return NULL;
        }

        STDMETHOD_(HWND, GetHostHwnd)(CTHIS) SCONST OVERRIDE
        {
            return NULL;
        }

        STDMETHOD_(BOOL, IsTranslucent)(CTHIS) SCONST OVERRIDE
        {
            return FALSE;
        }

        STDMETHOD_(BOOL, IsSendWheel2Hover)(CTHIS) SCONST OVERRIDE
        {
            return FALSE;
        }

        STDMETHOD_(int, GetScale)(CTHIS) SCONST OVERRIDE
        {
            return 100;
        }

        STDMETHOD_(LPCWSTR, GetTranslatorContext)(CTHIS) SCONST OVERRIDE
        {
            return NULL;
        }

        STDMETHOD_(void, GetContainerRect)(CTHIS_ RECT *ret) SCONST OVERRIDE
        {
            ret->left = 0;
            ret->top = 0;
            ret->right = 4096;
            ret->bottom = 4096;
        }

        STDMETHOD_(void, UpdateRegion)(THIS_ IRegionS *rgn) OVERRIDE
        {
        }

        STDMETHOD_(void, OnRedraw)(THIS_ LPCRECT rc, BOOL bClip) OVERRIDE
        {
        }

        STDMETHOD_(BOOL, UpdateWindow)(THIS_ BOOL bForce DEF_VAL(TRUE)) OVERRIDE
        {
            return TRUE;
        }

        STDMETHOD_(void, UpdateTooltip)(THIS) OVERRIDE
        {
        }

        STDMETHOD_(void, SetToolTip)(THIS_ LPCRECT rc, UINT tipAlign, LPCTSTR pszTip) OVERRIDE
        {
        }

        STDMETHOD_(void, EnableIME)(THIS_ BOOL bEnable) OVERRIDE
        {
        }

        STDMETHOD_(void, OnUpdateCursor)(THIS) OVERRIDE
        {
        }

        STDMETHOD_(void, EnableHostPrivateUiDef)(THIS_ BOOL bEnable) OVERRIDE
        {
        }

        STDMETHOD_(BOOL, PostTask)(THIS_ IRunnable *runable, BOOL bAsync DEF_VAL(TRUE)) OVERRIDE
        {
            // Headless: no message loop to post to - run inline for determinism.
            if (runable)
                runable->run();
            return TRUE;
        }

        STDMETHOD_(int, RemoveTasksForObject)(THIS_ void *pObj) OVERRIDE
        {
            return 0;
        }

        STDMETHOD_(void, OnDropdownState)(THIS_ IHostWnd *pDropdown, BOOL bCreate) OVERRIDE
        {
        }

        void OnTimelineRequestChanged(BOOL bHasTimelineRequest) override {}
    };

    struct HeadlessApp
    {
        SComMgr2 comMgr;
        SAutoRefPtr<IRenderFactory> renderFactory;
        SApplication *app;

        HeadlessApp()
            : app(NULL)
        {
            EXPECT_TRUE(comMgr.CreateRender_GDI((IObjRef **)&renderFactory));
            app = new SApplication(renderFactory, NULL);

            // SPanel::OnCreate requires a builtin scrollbar skin. Inject a bare
            // SSkinScrollbar (no image is needed for headless message-level tests).
            static const wchar_t kSkinXml[] = L"<skin><scrollbar name=\"_skin.sys.scrollbar\"/></skin>";
            SXmlDoc skinDoc;
            // ASSERT_* cannot be used inside a constructor (it returns); EXPECT is fine.
            EXPECT_TRUE(skinDoc.load_buffer(kSkinXml, sizeof(kSkinXml) - sizeof(wchar_t), xml_parse_default, WCHAR_ENC));
            SXmlNode skinNode = skinDoc.root().child(L"skin");
            EXPECT_TRUE(skinNode);
            if (skinNode)
                GETUIDEF->GetBuiltinSkinPool()->LoadSkins(&skinNode);
        }

        ~HeadlessApp()
        {
            delete app;
        }
    };

    static BOOL CreateRootFromXml(SWindow &root, ISwndContainer *pContainer, const wchar_t *pszXml)
    {
        SXmlDoc doc;
        if (!doc.load_buffer(pszXml, (size_t)wcslen(pszXml) * sizeof(wchar_t), xml_parse_default, WCHAR_ENC))
            return FALSE;
        SXmlNode nodeRoot = doc.root().first_child();
        if (!nodeRoot)
            return FALSE;
        root.SetContainer(pContainer);
        return root.InitFromXml(&nodeRoot);
    }
}

// ---------------------------------------------------------------------------
// SListCtrl: dragging with multiSelection enabled draws a rubber band and the
// covered rows become checked. The band takes priority over drag scroll /
// fling (SPanel::HandleMouseDrag consults IsRubberBandSelEnabled first).
// ---------------------------------------------------------------------------
TEST(soui_rubberband, listctrl_band_selects_rows)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <listctrl name=\"lc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" headerHeight=\"20\" itemHeight=\"20\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"  </listctrl>"
        L"</window>"));
    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName(L"lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    for (int i = 0; i < 10; i++)
        pLc->InsertItem(-1, _T("item"));
    pLc->Move(CRect(0, 0, 200, 200));

    // Press on row 0 (client y=25, header is 20px high), drag down to y=95.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 25));
    pLc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 40));
    EXPECT_TRUE(pLc->IsRubberBandSelActive()) << L"band must start once the drag threshold is passed";
    pLc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 95));

    // rows 0..3 are covered by the band
    EXPECT_TRUE(pLc->GetCheckState(0));
    EXPECT_TRUE(pLc->GetCheckState(3));
    EXPECT_FALSE(pLc->GetCheckState(4));

    pLc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(50, 95));
    EXPECT_FALSE(pLc->IsRubberBandSelActive()) << L"band must end on mouse up";

    EXPECT_TRUE(pLc->GetCheckState(0));
    EXPECT_TRUE(pLc->GetCheckState(3));
    EXPECT_FALSE(pLc->GetCheckState(4));
    EXPECT_EQ(3, pLc->GetSelectedItem());

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SListCtrl: with multi-selection disabled, dragging must NOT start a rubber
// band (the legacy drag scroll path keeps working).
// ---------------------------------------------------------------------------
TEST(soui_rubberband, listctrl_band_disabled_without_multiselection)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <listctrl name=\"lc\" size=\"200,200\" headerHeight=\"20\" itemHeight=\"20\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"  </listctrl>"
        L"</window>"));
    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName(L"lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    for (int i = 0; i < 10; i++)
        pLc->InsertItem(-1, _T("item"));
    pLc->Move(CRect(0, 0, 200, 200));

    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 25));
    pLc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 40));
    pLc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 95));
    EXPECT_FALSE(pLc->IsRubberBandSelActive());
    pLc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(50, 95));
    EXPECT_FALSE(pLc->GetCheckState(3));
    EXPECT_TRUE(pLc->GetCheckState(0));

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SListCtrl: Ctrl+drag keeps (adds to) the previous selection.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, listctrl_ctrl_band_adds_selection)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <listctrl name=\"lc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" headerHeight=\"20\" itemHeight=\"20\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"  </listctrl>"
        L"</window>"));
    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName(L"lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    for (int i = 0; i < 10; i++)
        pLc->InsertItem(-1, _T("item"));
    pLc->Move(CRect(0, 0, 200, 200));

    // Pre-select row 5 through the click path.
    pLc->SetSelectedItem(5);
    EXPECT_TRUE(pLc->GetCheckState(5));

    // Ctrl+drag rows 0..2 with the mouse (band from y=25 to y=80).
    // The click path reads modifier keys from the message wParam, so the
    // simulated MK_CONTROL flag is all that is needed.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 25));
    pLc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 80));
    pLc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(50, 80));

    EXPECT_TRUE(pLc->GetCheckState(0));
    EXPECT_TRUE(pLc->GetCheckState(2));
    EXPECT_TRUE(pLc->GetCheckState(5)) << L"Ctrl+band must keep the previous selection";
    EXPECT_FALSE(pLc->GetCheckState(3));

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// STreeCtrl: rubber band multi-selection over root items.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, treectrl_band_selects_items)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" itemHeight=\"20\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);
    EXPECT_TRUE(pTc->GetMultiSel());

    SArray<HSTREEITEM> items;
    for (int i = 0; i < 6; i++)
    {
        SStringT str;
        str.Format(_T("item%d"), i);
        items.Add(pTc->InsertItem(str, STVI_ROOT, STVI_LAST));
    }
    pTc->Move(CRect(0, 0, 200, 200));

    // Press on row 0 (y=25 is inside row 1: rows are 0..20, 20..40, ...),
    // drag down to y=75: rows 1..3 are covered.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 25));
    pTc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 40));
    EXPECT_TRUE(pTc->IsRubberBandSelActive());
    pTc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 75));

    EXPECT_EQ(3, pTc->GetSelItemCount());
    EXPECT_TRUE(pTc->IsItemSelected(items[1]));
    EXPECT_TRUE(pTc->IsItemSelected(items[2]));
    EXPECT_TRUE(pTc->IsItemSelected(items[3]));
    EXPECT_FALSE(pTc->IsItemSelected(items[0]));
    EXPECT_FALSE(pTc->IsItemSelected(items[4]));

    pTc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(50, 75));
    EXPECT_FALSE(pTc->IsRubberBandSelActive());

    // The cursor follows the last item hit by the band (same convention as
    // the other multi-selection controls), so keyboard navigation continues
    // from the band's end.
    EXPECT_EQ(items[3], pTc->GetSelectedItem());

    // A plain selection replaces the rubber band selection set with the
    // clicked item (recorded in the map; the cursor moves to it).
    pTc->SelectItem(items[4], FALSE);
    EXPECT_EQ(1, pTc->GetSelItemCount());
    EXPECT_TRUE(pTc->IsItemSelected(items[4]));
    EXPECT_EQ(items[4], pTc->GetSelectedItem());

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// STreeCtrl: Ctrl+band must keep the existing selection. Regression: the
// Ctrl+click path used to run through SelectItem(), which cleared the whole
// multi-selection set before the band even started.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, treectrl_ctrl_band_keeps_selection)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" itemHeight=\"20\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);

    SArray<HSTREEITEM> items;
    for (int i = 0; i < 6; i++)
    {
        SStringT str;
        str.Format(_T("item%d"), i);
        items.Add(pTc->InsertItem(str, STVI_ROOT, STVI_LAST));
    }
    pTc->Move(CRect(0, 0, 200, 200));

    // Ctrl+click on row 5 (y=115 is inside row 5: 100..120) selects it via
    // the toggle path; the multi-selection set now contains item5 only.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 115));
    pTc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(50, 115));
    EXPECT_TRUE(pTc->IsItemSelected(items[5])) << L"Ctrl+click must select the clicked item";

    // Ctrl+drag rows 1..3 (band from y=25 to y=75) must keep item5.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 25));
    pTc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 40));
    EXPECT_TRUE(pTc->IsRubberBandSelActive());
    pTc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 75));
    pTc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(50, 75));

    EXPECT_TRUE(pTc->IsItemSelected(items[1]));
    EXPECT_TRUE(pTc->IsItemSelected(items[2]));
    EXPECT_TRUE(pTc->IsItemSelected(items[3]));
    EXPECT_TRUE(pTc->IsItemSelected(items[5])) << L"Ctrl+band must keep the previous selection";
    EXPECT_FALSE(pTc->IsItemSelected(items[0]));
    EXPECT_EQ(4, pTc->GetSelItemCount());

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SListCtrl: ESC while banding cancels the band and restores the selection
// that existed before the band started.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, listctrl_esc_cancels_band)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <listctrl name=\"lc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" headerHeight=\"20\" itemHeight=\"20\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"  </listctrl>"
        L"</window>"));
    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName(L"lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    for (int i = 0; i < 10; i++)
        pLc->InsertItem(-1, _T("item"));
    pLc->Move(CRect(0, 0, 200, 200));

    // The press selects row 0 through the click path, then the band covers
    // rows 0..3. The band-start snapshot therefore contains row 0 only.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 25));
    pLc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 40));
    EXPECT_TRUE(pLc->IsRubberBandSelActive());
    pLc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 95));
    EXPECT_TRUE(pLc->GetCheckState(0));
    EXPECT_TRUE(pLc->GetCheckState(3));

    // ESC undoes everything the band added, back to the band-start snapshot.
    pLc->SSendMessage(WM_KEYDOWN, VK_ESCAPE, 0);
    EXPECT_FALSE(pLc->IsRubberBandSelActive()) << L"ESC must cancel an active band";
    EXPECT_TRUE(pLc->GetCheckState(0)) << L"the click-selected row is part of the snapshot";
    EXPECT_FALSE(pLc->GetCheckState(3)) << L"ESC must undo the band selection";
    EXPECT_EQ(0, pLc->GetSelectedItem());

    // ESC again (band inactive) is a no-op and must not crash.
    pLc->SSendMessage(WM_KEYDOWN, VK_ESCAPE, 0);
    EXPECT_FALSE(pLc->IsRubberBandSelActive());

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// STreeCtrl: ESC while banding cancels the band and clears the band selection.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, treectrl_esc_cancels_band)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" itemHeight=\"20\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);
    EXPECT_TRUE(pTc->GetMultiSel());

    SArray<HSTREEITEM> items;
    for (int i = 0; i < 6; i++)
    {
        SStringT str;
        str.Format(_T("item%d"), i);
        items.Add(pTc->InsertItem(str, STVI_ROOT, STVI_LAST));
    }
    pTc->Move(CRect(0, 0, 200, 200));

    // Select item[0] first so the cancel has something to restore. In
    // multiSel mode a plain SelectItem records the item in the multi-
    // selection set and moves the keyboard cursor (m_hSelItem) to it.
    pTc->SelectItem(items[0], FALSE);
    EXPECT_TRUE(pTc->IsItemSelected(items[0]));
    EXPECT_EQ(items[0], pTc->GetSelectedItem());

    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 25));
    pTc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 75));
    EXPECT_TRUE(pTc->IsRubberBandSelActive());
    EXPECT_EQ(3, pTc->GetSelItemCount());
    // While the band is active the cursor sits on the last hit row.
    EXPECT_EQ(items[3], pTc->GetSelectedItem());

    pTc->SSendMessage(WM_KEYDOWN, VK_ESCAPE, 0);
    EXPECT_FALSE(pTc->IsRubberBandSelActive());
    // The press itself recorded the pressed row (item1) in the set before
    // the band started, so the snapshot holds item1: ESC undoes the rows the
    // band added and restores the pressed row. The cursor, which the band
    // moved to the last hit row, is restored to the pressed row as well.
    EXPECT_EQ(1, pTc->GetSelItemCount());
    EXPECT_TRUE(pTc->IsItemSelected(items[1]));
    EXPECT_FALSE(pTc->IsItemSelected(items[2]));
    EXPECT_EQ(items[1], pTc->GetSelectedItem());

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// Coordinate system regression: mouse message coordinates are HOST window
// coordinates, not control-local ones. Place the control at (100,50) inside
// the host and verify the band still maps onto the correct rows. Guards the
// SListView / SMCListView band mapping, which used to mix host coordinates
// with content coordinates (Position2Item expects content 0 = client origin).
// ---------------------------------------------------------------------------
TEST(soui_rubberband, listctrl_band_offset_within_host)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"400\" height=\"400\">"
        L"  <listctrl name=\"lc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" headerHeight=\"20\" itemHeight=\"20\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"  </listctrl>"
        L"</window>"));
    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName(L"lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    for (int i = 0; i < 10; i++)
        pLc->InsertItem(-1, _T("item"));
    // Place the control at (100,50) inside the host window.
    pLc->Move(CRect(100, 50, 300, 250));
    EXPECT_TRUE(pLc->GetClientRect() == CRect(100, 50, 300, 250));

    // All message coordinates below are HOST coordinates.
    // Rows live at host y = 50 + 20 + i*20 (header is 20px): row0 = 70..90.
    // Press inside row 0 (host y=75), drag down to host y=140: rows 0..3.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(150, 75));
    pLc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(150, 90));
    EXPECT_TRUE(pLc->IsRubberBandSelActive());
    pLc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(150, 140));

    EXPECT_TRUE(pLc->GetCheckState(0));
    EXPECT_TRUE(pLc->GetCheckState(3));
    EXPECT_FALSE(pLc->GetCheckState(4)) << L"band must not leak past the covered rows";
    EXPECT_EQ(3, pLc->GetSelectedItem());

    pLc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(150, 140));

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SListView (adapter based): same coordinate regression as above. The band
// mapping used to feed host coordinates directly into Position2Item(), which
// selects the wrong rows as soon as the view is not at the host origin.
// ---------------------------------------------------------------------------
namespace
{
    class TestLvAdapter : public SAdapterBase
    {
      public:
        STDMETHOD_(int, getCount)(THIS) OVERRIDE
        {
            return 10;
        }

        STDMETHOD_(void, getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate) OVERRIDE
        {
            if (pItem->GetChildrenCount() == 0)
            {
                SXmlNode xmlItem = xmlTemplate.child(L"itemTemp");
                if (xmlItem)
                    pItem->InitFromXml(&xmlItem);
            }
        }
    };
}

TEST(soui_rubberband, listview_band_offset_within_host)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"400\" height=\"400\">"
        L"  <listview name=\"lv\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" dividerSize=\"0\">"
        L"    <template itemHeight=\"20\">"
        L"      <itemTemp>"
        L"        <text size=\"200,20\"/>"
        L"      </itemTemp>"
        L"    </template>"
        L"  </listview>"
        L"</window>"));
    SListView *pLv = sobj_cast<SListView>(root.FindChildByName(L"lv"));
    ASSERT_TRUE(pLv);

    TestLvAdapter adapter;
    ASSERT_TRUE(pLv->SetAdapter(&adapter));
    // Place the control at (100,50) inside the host window.
    pLv->Move(CRect(100, 50, 300, 250));

    // All message coordinates below are HOST coordinates. Rows live at host
    // y = 50 + i*20 (no header). Press inside row 0 (host y=60), drag to
    // host y=140: content band = 10..89 -> rows 0..4.
    pLv->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(150, 60));
    pLv->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(150, 75));
    EXPECT_TRUE(pLv->IsRubberBandSelActive());
    pLv->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(150, 140));

    EXPECT_TRUE(pLv->IsItemSelected(0));
    EXPECT_TRUE(pLv->IsItemSelected(4));
    EXPECT_FALSE(pLv->IsItemSelected(5)) << L"band must not leak past the covered rows";

    pLv->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(150, 140));
    EXPECT_FALSE(pLv->IsRubberBandSelActive());
    EXPECT_TRUE(pLv->IsItemSelected(4));

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SMCListView regression: rows are drawn BELOW the header (item top =
// rcClient.top + headerHeight + Item2Position - nPos), so the band -> content
// conversion must also step over the header height. Scrolled state + host
// offset + header all together used to shift the selected row range.
// ---------------------------------------------------------------------------
namespace
{
    class TestMcLvAdapter : public SMcAdapterBase
    {
      public:
        STDMETHOD_(int, getCount)(THIS) OVERRIDE
        {
            return 20;
        }

        STDMETHOD_(void, getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate) OVERRIDE
        {
            if (pItem->GetChildrenCount() == 0)
            {
                SXmlNode xmlItem = xmlTemplate.child(L"itemTemp");
                if (xmlItem)
                    pItem->InitFromXml(&xmlItem);
            }
        }
    };
}

TEST(soui_rubberband, mclistview_band_scrolled_offset_within_host)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"400\" height=\"400\">"
        L"  <mclistview name=\"mclv\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" headerHeight=\"20\" dividerSize=\"0\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"    <template itemHeight=\"20\">"
        L"      <itemTemp>"
        L"        <text size=\"200,20\"/>"
        L"      </itemTemp>"
        L"    </template>"
        L"  </mclistview>"
        L"</window>"));
    SMCListView *pMcLv = sobj_cast<SMCListView>(root.FindChildByName(L"mclv"));
    ASSERT_TRUE(pMcLv);

    TestMcLvAdapter adapter;
    ASSERT_TRUE(pMcLv->SetAdapter(&adapter));
    // Place the control at (100,50) inside the host window.
    pMcLv->Move(CRect(100, 50, 300, 250));
    // Scroll so content position 120 (row 6, 20px rows) sits at the top of
    // the list area (below the 20px header).
    ASSERT_TRUE(pMcLv->SetScrollPos(TRUE, 120, FALSE));
    EXPECT_EQ(120, pMcLv->GetScrollPos(TRUE));

    // All message coordinates below are HOST coordinates. Row i lives at
    // host y = 50 + 20 + i*20 - 120. Row 6 spans host 70..90.
    // Press inside row 6 (host y=75), drag to host y=140: content band =
    // 125..189 -> rows 6..9 (row 10 starts at host y=150).
    pMcLv->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(150, 75));
    pMcLv->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(150, 90));
    EXPECT_TRUE(pMcLv->IsRubberBandSelActive());
    pMcLv->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(150, 140));

    EXPECT_EQ(4, pMcLv->GetSelItemCount());
    EXPECT_TRUE(pMcLv->IsItemSelected(6));
    EXPECT_TRUE(pMcLv->IsItemSelected(9));
    EXPECT_FALSE(pMcLv->IsItemSelected(5)) << L"band must not leak above the covered rows";
    EXPECT_FALSE(pMcLv->IsItemSelected(10)) << L"band must not leak past the covered rows";

    pMcLv->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(150, 140));
    EXPECT_FALSE(pMcLv->IsRubberBandSelActive());
    EXPECT_EQ(4, pMcLv->GetSelItemCount());

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SMCListView: the band must be anchored in CONTENT coordinates. While edge
// auto-scrolling moves the content under a stationary mouse, everything the
// band has covered so far must stay selected (the band extends toward the
// scrolled direction instead of only covering the current viewport slice).
//
// The auto-scroll timer needs a host message loop, so this test drives the
// equivalent path directly: change the scroll position (SetScrollPos) and
// re-send the same mouse move - UpdateRubberBandSel re-runs with the new
// nPos exactly like an auto-scroll tick does.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, mclistview_band_union_during_scroll)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"400\" height=\"600\">"
        L"  <mclistview name=\"mclv\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" headerHeight=\"20\" dividerSize=\"0\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"    <template itemHeight=\"20\">"
        L"      <itemTemp>"
        L"        <text size=\"200,20\"/>"
        L"      </itemTemp>"
        L"    </template>"
        L"  </mclistview>"
        L"</window>"));
    SMCListView *pMcLv = sobj_cast<SMCListView>(root.FindChildByName(L"mclv"));
    ASSERT_TRUE(pMcLv);

    TestMcLvAdapter adapter;
    ASSERT_TRUE(pMcLv->SetAdapter(&adapter));
    pMcLv->Move(CRect(100, 50, 300, 250));

    // Row i lives at host y = 50 + 20 + i*20 - nPos. Press inside row 0
    // (host y=75), drag to host y=140 with nPos=0: content band 75..139,
    // rows 0..3.
    pMcLv->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(150, 75));
    pMcLv->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(150, 90));
    EXPECT_TRUE(pMcLv->IsRubberBandSelActive());
    pMcLv->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(150, 140));
    EXPECT_EQ(4, pMcLv->GetSelItemCount());
    EXPECT_TRUE(pMcLv->IsItemSelected(0));
    EXPECT_TRUE(pMcLv->IsItemSelected(3));

    // Simulate an auto-scroll tick: scroll down 3 rows (nPos 0 -> 60) with the
    // mouse standing still at host y=140. The content-anchored band keeps its
    // start at content y=75 (now above the viewport) and extends its end to
    // content y=200: rows 0..6, previously covered rows stay selected.
    ASSERT_TRUE(pMcLv->SetScrollPos(TRUE, 60, FALSE));
    pMcLv->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(150, 140));
    EXPECT_EQ(7, pMcLv->GetSelItemCount());
    EXPECT_TRUE(pMcLv->IsItemSelected(0)) << L"rows scrolled out of the viewport must stay selected";
    EXPECT_TRUE(pMcLv->IsItemSelected(3)) << L"previously covered rows must not be dropped";
    EXPECT_TRUE(pMcLv->IsItemSelected(6));
    EXPECT_FALSE(pMcLv->IsItemSelected(7));

    // Another tick: nPos 60 -> 120, band end reaches content y=260: rows 0..9.
    ASSERT_TRUE(pMcLv->SetScrollPos(TRUE, 120, FALSE));
    pMcLv->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(150, 140));
    EXPECT_EQ(10, pMcLv->GetSelItemCount());
    EXPECT_TRUE(pMcLv->IsItemSelected(0));
    EXPECT_TRUE(pMcLv->IsItemSelected(9));

    // Mouse moves back up into the viewport (host y=100): the band shrinks to
    // content 75..220 -> rows 0..7. The anchor above the viewport keeps the
    // scrolled-out rows selected.
    pMcLv->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(150, 100));
    EXPECT_EQ(8, pMcLv->GetSelItemCount());
    EXPECT_TRUE(pMcLv->IsItemSelected(0));
    EXPECT_TRUE(pMcLv->IsItemSelected(7));
    EXPECT_FALSE(pMcLv->IsItemSelected(8));

    // Mouse up keeps the accumulated selection.
    pMcLv->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(150, 100));
    EXPECT_FALSE(pMcLv->IsRubberBandSelActive());
    EXPECT_EQ(8, pMcLv->GetSelItemCount());

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// Plain click (no Ctrl/Shift) on a blank area below the items clears the
// selection. The click lands on WM_LBUTTONDOWN via the control's mouse
// handler (HitTest returns NULL there).
// ---------------------------------------------------------------------------
TEST(soui_rubberband, listview_blank_click_clears_selection)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"400\" height=\"400\">"
        L"  <listview name=\"lv\" size=\"200,300\" multiSel=\"1\" bandEnable=\"1\" dividerSize=\"0\">"
        L"    <template itemHeight=\"20\">"
        L"      <itemTemp>"
        L"        <text size=\"200,20\"/>"
        L"      </itemTemp>"
        L"    </template>"
        L"  </listview>"
        L"</window>"));
    SListView *pLv = sobj_cast<SListView>(root.FindChildByName(L"lv"));
    ASSERT_TRUE(pLv);

    TestLvAdapter adapter;
    ASSERT_TRUE(pLv->SetAdapter(&adapter));
    pLv->Move(CRect(100, 50, 300, 350));

    // 10 rows x 20px = 200px of rows; the control is 300px tall, so host
    // y in 250..350 is blank. Band over rows 0..3 first (host y 60..140).
    pLv->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(150, 60));
    pLv->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(150, 140));
    pLv->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(150, 140));
    EXPECT_GE(pLv->GetSelItemCount(), 1);

    // Plain click on the blank area clears the selection.
    pLv->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(150, 300));
    EXPECT_EQ(0, pLv->GetSelItemCount()) << L"blank click must clear the selection";
    EXPECT_FALSE(pLv->IsItemSelected(0));
    EXPECT_EQ(-1, pLv->GetSel());

    pLv->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(150, 300));
    EXPECT_EQ(0, pLv->GetSelItemCount());

    // Ctrl+click on blank must NOT clear (native behavior keeps the selection).
    pLv->SetSel(2, TRUE);
    EXPECT_TRUE(pLv->IsItemSelected(2));
    pLv->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(150, 300));
    pLv->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(150, 300));
    EXPECT_TRUE(pLv->IsItemSelected(2)) << L"Ctrl+click on blank must keep the selection";

    root.DestroyAllChildren();
}

TEST(soui_rubberband, treectrl_blank_click_clears_selection)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" itemHeight=\"20\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);

    SArray<HSTREEITEM> items;
    for (int i = 0; i < 6; i++)
    {
        SStringT str;
        str.Format(_T("item%d"), i);
        items.Add(pTc->InsertItem(str, STVI_ROOT, STVI_LAST));
    }
    pTc->Move(CRect(0, 0, 200, 200));

    // 6 rows x 20px = 120px; y in 120..200 is blank. Band over rows 1..3.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 25));
    pTc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 75));
    pTc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(50, 75));
    EXPECT_GE(pTc->GetSelItemCount(), 1);

    // Plain click on the blank area clears the selection. The keyboard
    // cursor stays on the last band-hit row (item3) - only the selection
    // set is cleared.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 160));
    EXPECT_EQ(0, pTc->GetSelItemCount()) << L"blank click must clear the selection";
    EXPECT_FALSE(pTc->IsItemSelected(items[1]));
    EXPECT_EQ(items[3], pTc->GetSelectedItem());

    pTc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(50, 160));
    EXPECT_EQ(0, pTc->GetSelItemCount());

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// bandEnable="0" decouples the rubber band gesture from multi-select:
// - a multi-select control with the band disabled must NOT enter banding on
//   drag (it falls back to drag scroll / fling);
// - the multi-select click path (Ctrl+click toggle) keeps working.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, treectrl_band_disable_gesture_keeps_multisel)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" bandEnable=\"0\" itemHeight=\"20\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);
    EXPECT_FALSE(pTc->IsBandSelEnabled());

    SArray<HSTREEITEM> items;
    for (int i = 0; i < 6; i++)
    {
        SStringT str;
        str.Format(_T("item%d"), i);
        items.Add(pTc->InsertItem(str, STVI_ROOT, STVI_LAST));
    }
    pTc->Move(CRect(0, 0, 200, 200));

    // Ctrl+click selects item5 through the toggle path (not gated by bandEnable).
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 115));
    pTc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(50, 115));
    EXPECT_TRUE(pTc->IsItemSelected(items[5]));

    // A drag that used to band (rows 1..3) must not start the band any more.
    // Ctrl+down additionally toggles item1 (click semantics at DOWN).
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 25));
    pTc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 75));
    EXPECT_FALSE(pTc->IsRubberBandSelActive()) << L"bandEnable=0 must keep the band from starting";
    pTc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(50, 75));

    EXPECT_TRUE(pTc->IsItemSelected(items[5])) << L"the multi-selection set must survive the drag";
    EXPECT_FALSE(pTc->IsItemSelected(items[2])) << L"no band selection may be applied";
    EXPECT_FALSE(pTc->IsItemSelected(items[3]));

    root.DestroyAllChildren();
}

TEST(soui_rubberband, listview_band_disable_gesture_keeps_multisel)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"400\" height=\"400\">"
        L"  <listview name=\"lv\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" bandEnable=\"0\" dividerSize=\"0\">"
        L"    <template itemHeight=\"20\">"
        L"      <itemTemp>"
        L"        <text size=\"200,20\"/>"
        L"      </itemTemp>"
        L"    </template>"
        L"  </listview>"
        L"</window>"));
    SListView *pLv = sobj_cast<SListView>(root.FindChildByName(L"lv"));
    ASSERT_TRUE(pLv);

    TestLvAdapter adapter;
    ASSERT_TRUE(pLv->SetAdapter(&adapter));
    pLv->Move(CRect(0, 0, 200, 200));

    // A drag that used to band (rows 0..3) must not start the band. The plain
    // DOWN still selects the pressed row through the click path (item0), but
    // the band range must not be applied.
    pLv->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 10));
    pLv->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 75));
    EXPECT_FALSE(pLv->IsRubberBandSelActive()) << L"bandEnable=0 must keep the band from starting";
    pLv->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(50, 75));
    EXPECT_FALSE(pLv->IsItemSelected(3)) << L"no band selection may be applied";

    // Turning the gesture back on at runtime restores banding.
    pLv->EnableBandSel(TRUE);
    pLv->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 10));
    pLv->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 75));
    EXPECT_TRUE(pLv->IsRubberBandSelActive()) << L"EnableBandSel(TRUE) must restore the band gesture";
    pLv->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(50, 75));
    EXPECT_TRUE(pLv->IsItemSelected(0));
    EXPECT_TRUE(pLv->IsItemSelected(3));

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// Disabling multi-selection at runtime:
// - with several items selected the whole selection is cleared;
// - with a single item selected the selection is transferred to the
//   single-selection marker (m_iSelItem / m_hSelItem / m_nSelectItem), so
//   the single-selection APIs keep reporting it;
// - re-enabling multi-selection promotes the single-selection marker back
//   into the multi-selection map (adapter views).
// ---------------------------------------------------------------------------
TEST(soui_rubberband, listview_disable_multisel_clears_multiple)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"400\" height=\"400\">"
        L"  <listview name=\"lv\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" dividerSize=\"0\">"
        L"    <template itemHeight=\"20\">"
        L"      <itemTemp>"
        L"        <text size=\"200,20\"/>"
        L"      </itemTemp>"
        L"    </template>"
        L"  </listview>"
        L"</window>"));
    SListView *pLv = sobj_cast<SListView>(root.FindChildByName(L"lv"));
    ASSERT_TRUE(pLv);

    TestLvAdapter adapter;
    ASSERT_TRUE(pLv->SetAdapter(&adapter));
    pLv->Move(CRect(0, 0, 200, 200));

    // Band rows 0..3 -> four selected items.
    pLv->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 10));
    pLv->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 75));
    pLv->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(50, 75));
    EXPECT_GE(pLv->GetSelItemCount(), 4);

    // Turning multi-selection off clears the whole selection.
    pLv->SetMultiSel(FALSE);
    EXPECT_EQ(0, pLv->GetSelItemCount());
    EXPECT_FALSE(pLv->IsItemSelected(0));
    EXPECT_FALSE(pLv->IsItemSelected(3));

    // A single selection survives the toggle unchanged.
    pLv->SetMultiSel(TRUE);
    pLv->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 10));
    pLv->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(50, 10));
    EXPECT_EQ(1, pLv->GetSelItemCount());
    pLv->SetMultiSel(FALSE);
    EXPECT_EQ(1, pLv->GetSelItemCount()) << L"a single selection must be kept";
    EXPECT_TRUE(pLv->IsItemSelected(0));

    root.DestroyAllChildren();
}

TEST(soui_rubberband, treectrl_disable_multisel_clears_multiple)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" itemHeight=\"20\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);

    SArray<HSTREEITEM> items;
    for (int i = 0; i < 6; i++)
    {
        SStringT str;
        str.Format(_T("item%d"), i);
        items.Add(pTc->InsertItem(str, STVI_ROOT, STVI_LAST));
    }
    pTc->Move(CRect(0, 0, 200, 200));

    // Ctrl+click two items -> multi-selection set holds 2 entries.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 35));
    pTc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(50, 35));
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 55));
    pTc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(50, 55));
    EXPECT_GE(pTc->GetSelItemCount(), 2);

    // Turning multi-selection off clears the whole selection.
    pTc->EnableMultiSelection(FALSE);
    EXPECT_EQ(0, pTc->GetSelItemCount());
    EXPECT_EQ(0, (int)pTc->GetSelectedItem());

    // A single selection survives the toggle: in multi mode the plain click
    // records item1 in the map and moves the keyboard cursor to it, and
    // switching multi off transfers that record to the single-selection
    // anchor.
    pTc->EnableMultiSelection(TRUE);
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 35));
    pTc->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(50, 35));
    EXPECT_TRUE(pTc->IsItemSelected(items[1]));
    EXPECT_EQ(items[1], pTc->GetSelectedItem());
    pTc->EnableMultiSelection(FALSE);
    EXPECT_EQ(items[1], pTc->GetSelectedItem()) << L"a single selection must be kept";

    root.DestroyAllChildren();
}

// Switching multi-selection off with exactly one item in the map must
// transfer that item to the single-selection marker: the anchor-style
// GetSelectedItem / IsItemSelected keep working in single-selection mode.
TEST(soui_rubberband, listview_multisel_switch_transfers_selection)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"400\" height=\"400\">"
        L"  <listview name=\"lv\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" dividerSize=\"0\">"
        L"    <template itemHeight=\"20\">"
        L"      <itemTemp>"
        L"        <text size=\"200,20\"/>"
        L"      </itemTemp>"
        L"    </template>"
        L"  </listview>"
        L"</window>"));
    SListView *pLv = sobj_cast<SListView>(root.FindChildByName(L"lv"));
    ASSERT_TRUE(pLv);

    TestLvAdapter adapter;
    ASSERT_TRUE(pLv->SetAdapter(&adapter));
    pLv->Move(CRect(0, 0, 200, 200));

    // Band rows 0..1, then a plain click moves the (replacing) selection to
    // row 3: the map holds exactly one entry.
    pLv->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 10));
    pLv->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 35));
    pLv->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(50, 35));
    pLv->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 70));
    pLv->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(50, 70));
    EXPECT_EQ(1, pLv->GetSelItemCount());
    EXPECT_TRUE(pLv->IsItemSelected(3));

    // Switch to single-selection: the anchor takes over the map entry.
    pLv->SetMultiSel(FALSE);
    EXPECT_EQ(1, pLv->GetSelItemCount());
    EXPECT_TRUE(pLv->IsItemSelected(3)) << L"the transferred selection must stay selected";
    EXPECT_FALSE(pLv->IsItemSelected(0));

    // Switch back to multi-selection: the anchor is promoted into the map so
    // the map-based IsItemSelected keeps reporting it.
    pLv->SetMultiSel(TRUE);
    EXPECT_EQ(1, pLv->GetSelItemCount());
    EXPECT_TRUE(pLv->IsItemSelected(3));

    root.DestroyAllChildren();
}

TEST(soui_rubberband, treectrl_multisel_switch_transfers_selection)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" itemHeight=\"20\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);

    SArray<HSTREEITEM> items;
    for (int i = 0; i < 6; i++)
    {
        SStringT str;
        str.Format(_T("item%d"), i);
        items.Add(pTc->InsertItem(str, STVI_ROOT, STVI_LAST));
    }
    pTc->Move(CRect(0, 0, 200, 200));

    // Ctrl+click two items, then clear everything by switching modes and
    // back: with a single anchor the selection must survive both switches.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 25));
    pTc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(50, 25));
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 65));
    pTc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(50, 65));
    EXPECT_GE(pTc->GetSelItemCount(), 2);

    // Two entries: switching off clears the whole selection.
    pTc->EnableMultiSelection(FALSE);
    EXPECT_EQ(0, (int)pTc->GetSelectedItem());

    // Single-selection anchor.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 45));
    pTc->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(50, 45));
    EXPECT_EQ(items[2], pTc->GetSelectedItem());

    // Re-enable: the single-selection anchor is promoted into the map and
    // stays on as the keyboard cursor; the item still reports selected.
    pTc->EnableMultiSelection(TRUE);
    EXPECT_TRUE(pTc->IsItemSelected(items[2]));

    // Ctrl+click toggles the promoted map entry off; the cursor follows the
    // clicked item (it is no longer a selection record, just the focus).
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 45));
    pTc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(50, 45));
    EXPECT_FALSE(pTc->IsItemSelected(items[2]));
    EXPECT_EQ(items[2], pTc->GetSelectedItem());

    // Ctrl+click one more item: exactly one map entry (items[4]).
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 85));
    pTc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(50, 85));
    EXPECT_EQ(1, pTc->GetSelItemCount());

    // Switch off: the single map entry transfers to the anchor.
    pTc->EnableMultiSelection(FALSE);
    EXPECT_EQ(items[4], pTc->GetSelectedItem())
        << L"the map entry must transfer to the single-selection anchor";
    EXPECT_TRUE(pTc->IsItemSelected(items[4]));
    EXPECT_FALSE(pTc->IsItemSelected(items[2]));

    root.DestroyAllChildren();
}

TEST(soui_rubberband, listctrl_multisel_switch_transfers_selection)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <listctrl name=\"lc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" headerHeight=\"20\" itemHeight=\"20\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"  </listctrl>"
        L"</window>"));
    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName(L"lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    for (int i = 0; i < 10; i++)
        pLc->InsertItem(-1, _T("item"));
    pLc->Move(CRect(0, 0, 200, 200));

    // Plain click selects row 0.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 25));
    pLc->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(50, 25));
    EXPECT_EQ(0, pLc->GetSelectedItem());

    // Ctrl+click row 2 adds it, Ctrl+click row 0 removes it again: exactly
    // one checked item (row 2) with the anchor left on row 0.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 65));
    pLc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(50, 65));
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_CONTROL, MAKELPARAM(50, 25));
    pLc->SSendMessage(WM_LBUTTONUP, MK_CONTROL, MAKELPARAM(50, 25));

    // Switch off: the anchor must follow the last remaining checked item.
    pLc->EnableMultiSelection(FALSE);
    EXPECT_EQ(2, pLc->GetSelectedItem())
        << L"the single checked item must become the single selection";

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// fullRowSel: with the default (full-row) mode a click anywhere on the item
// row selects it; with fullRowSel="0" only the item text region selects, and
// clicks beyond the text are treated as blank space.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, treectrl_fullrow_selects_beyond_text)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" itemHeight=\"20\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);

    SArray<HSTREEITEM> items;
    for (int i = 0; i < 6; i++)
    {
        SStringT str;
        str.Format(_T("item%d"), i);
        items.Add(pTc->InsertItem(str, STVI_ROOT, STVI_LAST));
    }
    pTc->Move(CRect(0, 0, 200, 200));

    // Full-row mode (default): a click far beyond the item text still selects.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(150, 25));
    pTc->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(150, 25));
    EXPECT_TRUE(pTc->IsItemSelected(items[1]));
    EXPECT_EQ(1, pTc->GetSelItemCount());

    root.DestroyAllChildren();
}

TEST(soui_rubberband, treectrl_textmode_click_beyond_text_no_select)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" fullRowSel=\"0\" itemHeight=\"20\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);

    SArray<HSTREEITEM> items;
    for (int i = 0; i < 6; i++)
    {
        SStringT str;
        str.Format(_T("item%d"), i);
        items.Add(pTc->InsertItem(str, STVI_ROOT, STVI_LAST));
    }
    pTc->Move(CRect(0, 0, 200, 200));

    // Clicking on the item text still selects it.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(30, 25));
    pTc->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(30, 25));
    EXPECT_TRUE(pTc->IsItemSelected(items[1]));
    EXPECT_EQ(1, pTc->GetSelItemCount());

    // Clicking beyond the text is blank space: it must not select that item
    // and clears the current selection instead. The keyboard cursor stays
    // on the last clicked row.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(150, 65));
    pTc->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(150, 65));
    EXPECT_EQ(items[1], pTc->GetSelectedItem()) << L"a click beyond the text must not select the item";
    EXPECT_EQ(0, pTc->GetSelItemCount());

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// STreeCtrl keyboard navigation. Arrow keys move the selection (single mode)
// or the cursor / selection set (multi mode), LEFT/RIGHT expand and collapse,
// SPACE toggles the item at the cursor in multi mode.
// Note: Ctrl+arrow / Shift+arrow / Ctrl+A need real modifier key state
// (GetKeyState) and cannot be simulated headless - they follow the SListView
// / STreeView semantics but are only verifiable in interactive sessions.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, treectrl_keyboard_arrow_moves_selection)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" itemHeight=\"20\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);

    SArray<HSTREEITEM> items;
    for (int i = 0; i < 6; i++)
    {
        SStringT str;
        str.Format(_T("item%d"), i);
        items.Add(pTc->InsertItem(str, STVI_ROOT, STVI_LAST));
    }
    pTc->Move(CRect(0, 0, 200, 200));

    // Single-selection mode: click item1, then arrows move the selection.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 25));
    pTc->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(50, 25));
    EXPECT_EQ(items[1], pTc->GetSelectedItem());

    pTc->SSendMessage(WM_KEYDOWN, VK_DOWN, 0);
    EXPECT_EQ(items[2], pTc->GetSelectedItem());
    EXPECT_TRUE(pTc->IsItemSelected(items[2]));
    EXPECT_FALSE(pTc->IsItemSelected(items[1]));

    pTc->SSendMessage(WM_KEYDOWN, VK_UP, 0);
    EXPECT_EQ(items[1], pTc->GetSelectedItem());

    // HOME selects the first visible item, END the last one.
    pTc->SSendMessage(WM_KEYDOWN, VK_HOME, 0);
    EXPECT_EQ(items[0], pTc->GetSelectedItem());
    pTc->SSendMessage(WM_KEYDOWN, VK_END, 0);
    EXPECT_EQ(items[5], pTc->GetSelectedItem());

    root.DestroyAllChildren();
}

TEST(soui_rubberband, treectrl_keyboard_left_right_expand_collapse)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" itemHeight=\"20\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);

    HSTREEITEM hRoot = pTc->InsertItem(_T("root"), STVI_ROOT, STVI_LAST);
    HSTREEITEM hChild = pTc->InsertItem(_T("child"), hRoot, STVI_LAST);
    pTc->Move(CRect(0, 0, 200, 200));

    // Select the root, then collapse it with LEFT.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 5));
    pTc->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(50, 5));
    EXPECT_EQ(hRoot, pTc->GetSelectedItem());

    pTc->SSendMessage(WM_KEYDOWN, VK_LEFT, 0);
    EXPECT_EQ(hRoot, pTc->GetSelectedItem());
    // Collapsed: DOWN has no next visible item, selection stays.
    pTc->SSendMessage(WM_KEYDOWN, VK_DOWN, 0);
    EXPECT_EQ(hRoot, pTc->GetSelectedItem());

    // Expand again with RIGHT, then DOWN reaches the child.
    pTc->SSendMessage(WM_KEYDOWN, VK_RIGHT, 0);
    pTc->SSendMessage(WM_KEYDOWN, VK_DOWN, 0);
    EXPECT_EQ(hChild, pTc->GetSelectedItem());

    root.DestroyAllChildren();
}

TEST(soui_rubberband, treectrl_keyboard_multisel_arrow_and_space)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" itemHeight=\"20\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);

    SArray<HSTREEITEM> items;
    for (int i = 0; i < 6; i++)
    {
        SStringT str;
        str.Format(_T("item%d"), i);
        items.Add(pTc->InsertItem(str, STVI_ROOT, STVI_LAST));
    }
    pTc->Move(CRect(0, 0, 200, 200));

    // Multi mode: click item1 (set = {item1}, cursor = item1), then a plain
    // DOWN replaces the set with item2 and moves the cursor.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 25));
    pTc->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(50, 25));
    EXPECT_EQ(1, pTc->GetSelItemCount());

    pTc->SSendMessage(WM_KEYDOWN, VK_DOWN, 0);
    EXPECT_EQ(1, pTc->GetSelItemCount());
    EXPECT_TRUE(pTc->IsItemSelected(items[2]));
    EXPECT_FALSE(pTc->IsItemSelected(items[1]));
    EXPECT_EQ(items[2], pTc->GetSelectedItem());

    // SPACE toggles the item at the cursor.
    pTc->SSendMessage(WM_KEYDOWN, VK_SPACE, 0);
    EXPECT_EQ(0, pTc->GetSelItemCount()) << L"SPACE must toggle the cursor item off";
    EXPECT_EQ(items[2], pTc->GetSelectedItem()) << L"toggling off must not move the cursor";

    pTc->SSendMessage(WM_KEYDOWN, VK_SPACE, 0);
    EXPECT_EQ(1, pTc->GetSelItemCount());
    EXPECT_TRUE(pTc->IsItemSelected(items[2]));

    root.DestroyAllChildren();
}

TEST(soui_rubberband, treectrl_keyboard_with_mode_switch)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" itemHeight=\"20\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);

    SArray<HSTREEITEM> items;
    for (int i = 0; i < 6; i++)
    {
        SStringT str;
        str.Format(_T("item%d"), i);
        items.Add(pTc->InsertItem(str, STVI_ROOT, STVI_LAST));
    }
    pTc->Move(CRect(0, 0, 200, 200));

    // Click item1 in multi mode, then switch to single: the sole selection
    // migrates to the anchor.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 25));
    pTc->SSendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(50, 25));
    EXPECT_TRUE(pTc->IsItemSelected(items[1]));
    pTc->EnableMultiSelection(FALSE);
    EXPECT_EQ(items[1], pTc->GetSelectedItem());

    // Back to multi: the anchor is promoted into the map and kept as the
    // keyboard cursor.
    pTc->EnableMultiSelection(TRUE);
    EXPECT_TRUE(pTc->IsItemSelected(items[1]));
    EXPECT_EQ(items[1], pTc->GetSelectedItem());

    // A plain arrow in multi mode replaces the set.
    pTc->SSendMessage(WM_KEYDOWN, VK_DOWN, 0);
    EXPECT_EQ(1, pTc->GetSelItemCount());
    EXPECT_TRUE(pTc->IsItemSelected(items[2]));
    EXPECT_FALSE(pTc->IsItemSelected(items[1]));

    // Switching off again migrates the sole selection back to the anchor.
    pTc->EnableMultiSelection(FALSE);
    EXPECT_EQ(items[2], pTc->GetSelectedItem()) << L"sole selection must migrate back to the anchor";
    // Mode-aware count: single-selection mode reports the anchor (1).
    EXPECT_EQ(1, pTc->GetSelItemCount());
    EXPECT_TRUE(pTc->IsItemSelected(items[2]));

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// Removing a node must drop exactly that node (and its descendants) from the
// multi-selection map. A pre-order walk from the last child would run past
// the subtree into the following siblings and wrongly deselect them.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, treectrl_removeitem_keeps_unrelated_selection)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" itemHeight=\"20\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);

    SArray<HSTREEITEM> items;
    for (int i = 0; i < 6; i++)
    {
        SStringT str;
        str.Format(_T("item%d"), i);
        items.Add(pTc->InsertItem(str, STVI_ROOT, STVI_LAST));
    }
    pTc->Move(CRect(0, 0, 200, 200));

    // Band rows 1..3 (y 25..75).
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 25));
    pTc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 75));
    pTc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(50, 75));
    EXPECT_TRUE(pTc->IsItemSelected(items[1]));
    EXPECT_TRUE(pTc->IsItemSelected(items[2]));
    EXPECT_TRUE(pTc->IsItemSelected(items[3]));

    // Remove item1: items[2]/items[3] come after it in pre-order and must
    // keep their selection.
    pTc->RemoveItem(items[1]);
    EXPECT_FALSE(pTc->IsItemSelected(items[1]));
    EXPECT_TRUE(pTc->IsItemSelected(items[2])) << L"sibling after the removed item must stay selected";
    EXPECT_TRUE(pTc->IsItemSelected(items[3])) << L"sibling after the removed item must stay selected";
    EXPECT_EQ(2, pTc->GetSelItemCount());

    // A removed subtree takes the parent and its descendants with it.
    HSTREEITEM hNode = pTc->InsertItem(_T("node"), STVI_ROOT, STVI_LAST);
    HSTREEITEM hKid = pTc->InsertItem(_T("kid"), hNode, STVI_LAST);
    pTc->Expand(hNode, TVE_EXPAND);
    // node/kid now sit at rows 5/6 (y 100..140); band over them replaces the
    // set. Non-full-row mode only hits the item's content width, so keep the
    // band inside the short "node" text (x=10).
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(10, 105));
    pTc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(10, 135));
    pTc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(10, 135));
    EXPECT_TRUE(pTc->IsItemSelected(hNode));
    EXPECT_TRUE(pTc->IsItemSelected(hKid));

    pTc->RemoveItem(hNode);
    EXPECT_FALSE(pTc->IsItemSelected(hKid)) << L"descendant of the removed node must be dropped";
    EXPECT_EQ(0, pTc->GetSelItemCount());

    root.DestroyAllChildren();
}
// ---------------------------------------------------------------------------
// Per-item selection state events (EventItemSelChanged / EventTreeItemSel-
// Changed): fired from AddSelItem / RemoveSelItem whenever a single item's
// selection state actually flips, in real time during a rubber band. The
// anchor-style SelChanged event keeps its single-selection semantics: it is
// never fired anywhere in the band path (start, moves, end or cancel).
// ---------------------------------------------------------------------------
namespace
{
    struct SelSpy
    {
        // per-item events
        int itemCount;
        int lastItem;
        int lastState; // -1 none, 0 deselected, 1 selected
        // anchor-style SelChanged events
        int selCount;
        int selLastNew;
        SelSpy()
            : itemCount(0)
            , lastItem(-1)
            , lastState(-1)
            , selCount(0)
            , selLastNew(-1)
        {
        }
        void reset()
        {
            itemCount = 0;
            lastItem = -1;
            lastState = -1;
            selCount = 0;
            selLastNew = -1;
        }
        BOOL onItem(EventItemSelChanged *pEvt)
        {
            itemCount++;
            lastItem = pEvt->iItem;
            lastState = pEvt->bSelected ? 1 : 0;
            return TRUE;
        }
        BOOL onTreeItem(EventTreeItemSelChanged *pEvt)
        {
            itemCount++;
            lastItem = (int)(INT_PTR)pEvt->hItem;
            lastState = pEvt->bSelected ? 1 : 0;
            return TRUE;
        }
        BOOL onLC(EventLCSelChanged *pEvt)
        {
            selCount++;
            selLastNew = pEvt->nNewSel;
            return TRUE;
        }
        BOOL onLV(EventLVSelChanged *pEvt)
        {
            selCount++;
            selLastNew = pEvt->iNewSel;
            return TRUE;
        }
        BOOL onTC(EventTCSelChanged *pEvt)
        {
            selCount++;
            selLastNew = (int)(INT_PTR)pEvt->hNewSel;
            return TRUE;
        }
    };
}

// ---------------------------------------------------------------------------
// Family unification round (2026-09-29): clearing an already-empty single
// selection is a no-op - no visuals and no anchor-style SelChanging/SelChanged
// pair (previously SetSel(-1) on an empty view still fired the pair).
// The remaining unifications (SListCtrl Shift+Click append, STreeView band
// cursor restore + Shift page keys, virtual SetSel click/keyboard merge) are
// modifier / STreeView-adapter / acc dependent and covered by gate regression.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, listview_clear_empty_selection_no_events)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <listview name=\"lv\" size=\"200,200\" dividerSize=\"0\">"
        L"    <template itemHeight=\"20\">"
        L"      <itemTemp>"
        L"        <text size=\"200,20\"/>"
        L"      </itemTemp>"
        L"    </template>"
        L"  </listview>"
        L"</window>"));
    SListView *pLv = sobj_cast<SListView>(root.FindChildByName(L"lv"));
    ASSERT_TRUE(pLv);

    TestLvAdapter adapter;
    ASSERT_TRUE(pLv->SetAdapter(&adapter));
    pLv->Move(CRect(0, 0, 200, 200));

    SelSpy spy;
    ASSERT_TRUE(pLv->GetEventSet()->subscribeEvent(&SelSpy::onLV, &spy));

    // A real selection fires exactly one anchor-style pair.
    pLv->SetSel(2, TRUE);
    EXPECT_EQ(2, pLv->GetSel());
    EXPECT_EQ(1, spy.selCount);

    // Clearing a non-empty selection is a real change: one pair.
    pLv->SetSel(-1, TRUE);
    EXPECT_EQ(-1, pLv->GetSel());
    EXPECT_EQ(2, spy.selCount);

    // Clearing an already-empty selection: the event pair must not fire.
    pLv->SetSel(-1, TRUE);
    EXPECT_EQ(2, spy.selCount) << L"clearing an empty selection must not fire SelChanged";
    pLv->SetSel(-1, TRUE);
    EXPECT_EQ(2, spy.selCount);

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// Explorer-style anchored shift ranges: the plain click fixes the range
// anchor; Shift+Click selects exactly [anchor..clicked] (replacing the set),
// so a later Shift+Click closer to the anchor SHRINKS the range again.
// SListCtrl is the only control whose shift path reads the message flags
// (MK_SHIFT) instead of GetKeyState, so it is headless-testable.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, listctrl_shift_click_anchored_range)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"300\">"
        L"  <listctrl name=\"lc\" size=\"200,300\" multiSel=\"1\" bandEnable=\"1\" headerHeight=\"20\" itemHeight=\"20\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"  </listctrl>"
        L"</window>"));
    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName(L"lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    for (int i = 0; i < 10; i++)
        pLc->InsertItem(-1, _T("item"));
    pLc->Move(CRect(0, 0, 200, 300));

    // Rows live at client y = 20 (header) + i*20. Plain click on row 2:
    // it becomes the range anchor.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 20 + 2 * 20 + 10));
    EXPECT_TRUE(pLc->GetCheckState(2));
    EXPECT_EQ(1, pLc->GetCheckedItemCount());

    // Shift+Click row 5: anchored range 2..5.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_SHIFT, MAKELPARAM(50, 20 + 5 * 20 + 10));
    EXPECT_EQ(4, pLc->GetCheckedItemCount());
    EXPECT_TRUE(pLc->GetCheckState(2));
    EXPECT_TRUE(pLc->GetCheckState(5));
    EXPECT_FALSE(pLc->GetCheckState(1)) << L"anchored range must replace the set";
    EXPECT_FALSE(pLc->GetCheckState(6));

    // Shift+Click row 3: the anchor stays at row 2 - the range SHRINKS
    // to 2..3 (regression: Shift navigation used to only ever grow).
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_SHIFT, MAKELPARAM(50, 20 + 3 * 20 + 10));
    EXPECT_EQ(2, pLc->GetCheckedItemCount());
    EXPECT_TRUE(pLc->GetCheckState(2));
    EXPECT_TRUE(pLc->GetCheckState(3));
    EXPECT_FALSE(pLc->GetCheckState(4)) << L"shrinking must deselect items beyond the new cursor";
    EXPECT_FALSE(pLc->GetCheckState(5));

    // Plain click resets the anchor to the clicked row.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 20 + 7 * 20 + 10));
    EXPECT_EQ(1, pLc->GetCheckedItemCount());
    EXPECT_TRUE(pLc->GetCheckState(7));
    // Shift+Click row 9: new anchored range 7..9.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON | MK_SHIFT, MAKELPARAM(50, 20 + 9 * 20 + 10));
    EXPECT_EQ(3, pLc->GetCheckedItemCount());

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SListCtrl keyboard navigation (multi-selection): plain arrows replace the
// set with the cursor item, SPACE toggles the cursor item, HOME/END jump to
// the ends. Shift/Ctrl keyboard modifiers go through GetKeyState and are not
// reachable headless - they are covered by the GUI.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, listctrl_keyboard_multisel_navigation)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <listctrl name=\"lc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" headerHeight=\"20\" itemHeight=\"20\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"  </listctrl>"
        L"</window>"));
    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName(L"lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    for (int i = 0; i < 10; i++)
        pLc->InsertItem(-1, _T("item"));
    pLc->Move(CRect(0, 0, 200, 200));

    // With no cursor yet, DOWN selects item 0.
    pLc->SSendMessage(WM_KEYDOWN, VK_DOWN, 0);
    EXPECT_EQ(0, pLc->GetSelectedItem());
    EXPECT_EQ(1, pLc->GetSelItemCount());
    EXPECT_TRUE(pLc->GetCheckState(0));

    // Plain DOWN replaces the set with the new cursor item.
    pLc->SSendMessage(WM_KEYDOWN, VK_DOWN, 0);
    EXPECT_EQ(1, pLc->GetSelectedItem());
    EXPECT_FALSE(pLc->GetCheckState(0));
    EXPECT_TRUE(pLc->GetCheckState(1));

    // SPACE toggles the cursor item off / on without moving the cursor.
    pLc->SSendMessage(WM_KEYDOWN, VK_SPACE, 0);
    EXPECT_EQ(0, pLc->GetSelItemCount()) << L"SPACE must toggle the cursor item off";
    EXPECT_EQ(1, pLc->GetSelectedItem()) << L"toggling off must not move the cursor";
    pLc->SSendMessage(WM_KEYDOWN, VK_SPACE, 0);
    EXPECT_EQ(1, pLc->GetSelItemCount());
    EXPECT_TRUE(pLc->GetCheckState(1));

    // END jumps to the last item (replacing the set), HOME back to the first.
    pLc->SSendMessage(WM_KEYDOWN, VK_END, 0);
    EXPECT_EQ(9, pLc->GetSelectedItem());
    EXPECT_TRUE(pLc->GetCheckState(9));
    EXPECT_EQ(1, pLc->GetSelItemCount());
    pLc->SSendMessage(WM_KEYDOWN, VK_HOME, 0);
    EXPECT_EQ(0, pLc->GetSelectedItem());
    EXPECT_TRUE(pLc->GetCheckState(0));

    // UP at item 0 stays put.
    pLc->SSendMessage(WM_KEYDOWN, VK_UP, 0);
    EXPECT_EQ(0, pLc->GetSelectedItem());

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SListCtrl keyboard navigation (single-selection): arrows move the selection
// and fire the anchor-style event pair; SPACE is left to the dialog.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, listctrl_keyboard_single_selection)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <listctrl name=\"lc\" size=\"200,200\" headerHeight=\"20\" itemHeight=\"20\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"  </listctrl>"
        L"</window>"));
    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName(L"lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    for (int i = 0; i < 10; i++)
        pLc->InsertItem(-1, _T("item"));
    pLc->Move(CRect(0, 0, 200, 200));

    // DOWN from an empty selection selects item 0, further DOWN moves it.
    pLc->SSendMessage(WM_KEYDOWN, VK_DOWN, 0);
    EXPECT_EQ(0, pLc->GetSelectedItem());
    pLc->SSendMessage(WM_KEYDOWN, VK_DOWN, 0);
    EXPECT_EQ(1, pLc->GetSelectedItem());
    pLc->SSendMessage(WM_KEYDOWN, VK_UP, 0);
    EXPECT_EQ(0, pLc->GetSelectedItem());

    // Repeated DOWN at the last item must not fire spurious event pairs.
    pLc->SSendMessage(WM_KEYDOWN, VK_END, 0);
    EXPECT_EQ(9, pLc->GetSelectedItem());
    SelSpy spy;
    ASSERT_TRUE(pLc->GetEventSet()->subscribeEvent(&SelSpy::onLC, &spy));
    pLc->SSendMessage(WM_KEYDOWN, VK_DOWN, 0);
    EXPECT_EQ(9, pLc->GetSelectedItem());
    EXPECT_EQ(0, spy.selCount) << L"boundary key must be silent";

    // SPACE in single-selection mode is left to the dialog (not handled).
    pLc->SSendMessage(WM_KEYDOWN, VK_SPACE, 0);
    EXPECT_EQ(9, pLc->GetSelectedItem());

    root.DestroyAllChildren();
}

TEST(soui_rubberband, listctrl_band_fires_item_sel_changed)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <listctrl name=\"lc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" headerHeight=\"20\" itemHeight=\"20\">"
        L"    <headerStyle wndclass=\"header\"/>"
        L"  </listctrl>"
        L"</window>"));
    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName(L"lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    for (int i = 0; i < 10; i++)
        pLc->InsertItem(-1, _T("item"));
    pLc->Move(CRect(0, 0, 200, 200));

    SelSpy spy;
    ASSERT_TRUE(pLc->GetEventSet()->subscribeEvent(&SelSpy::onItem, &spy));
    ASSERT_TRUE(pLc->GetEventSet()->subscribeEvent(&SelSpy::onLC, &spy));

    // The press selects row 0 through the click path: one per-item event
    // (row 0 -> selected).
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 25));
    EXPECT_EQ(1, spy.itemCount);
    EXPECT_EQ(0, spy.lastItem);
    EXPECT_EQ(1, spy.lastState);

    // Band start re-covers row 0 (already checked: no flip, no event); the
    // move extends the band over row 1 and must report it in real time.
    pLc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 45));
    EXPECT_TRUE(pLc->IsRubberBandSelActive());
    EXPECT_EQ(2, spy.itemCount);
    EXPECT_EQ(1, spy.lastItem);
    EXPECT_EQ(1, spy.lastState);

    // Extending over rows 2..3: two more per-item events.
    pLc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 95));
    EXPECT_EQ(4, spy.itemCount);
    EXPECT_EQ(3, spy.lastItem);
    EXPECT_EQ(1, spy.lastState);

    // Moving inside the already covered range: no row flips, no event.
    pLc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 96));
    EXPECT_EQ(4, spy.itemCount);

    // The press fires no anchor-style SelChanged either: in multi-selection
    // mode the LC SelChanging/SelChanged pair is not fired at all, the click
    // reports through per-item events only. The band never fires it - not
    // during the moves, not at band end.
    EXPECT_EQ(0, spy.selCount) << L"multi-selection feedback is per-item only";
    pLc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(50, 96));
    EXPECT_EQ(0, spy.selCount) << L"band end must not fire SelChanged";
    EXPECT_EQ(4, spy.itemCount) << L"band end must not fire per-item events";

    root.DestroyAllChildren();
}

TEST(soui_rubberband, listview_band_fires_item_sel_changed_restore_on_cancel)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <listview name=\"lv\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" dividerSize=\"0\">"
        L"    <template itemHeight=\"20\">"
        L"      <itemTemp>"
        L"        <text size=\"200,20\"/>"
        L"      </itemTemp>"
        L"    </template>"
        L"  </listview>"
        L"</window>"));
    SListView *pLv = sobj_cast<SListView>(root.FindChildByName(L"lv"));
    ASSERT_TRUE(pLv);

    TestLvAdapter adapter;
    ASSERT_TRUE(pLv->SetAdapter(&adapter));
    pLv->Move(CRect(0, 0, 200, 200));

    SelSpy spy;
    ASSERT_TRUE(pLv->GetEventSet()->subscribeEvent(&SelSpy::onItem, &spy));
    ASSERT_TRUE(pLv->GetEventSet()->subscribeEvent(&SelSpy::onLV, &spy));

    // Pre-select row 0 with a plain click so the cancel has something to
    // restore. In multi mode the click reports through per-item events only:
    // the anchor-style LV SelChanged is a single-selection protocol.
    pLv->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 10));
    pLv->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(50, 10));
    EXPECT_TRUE(pLv->IsItemSelected(0));
    EXPECT_EQ(1, spy.itemCount);
    EXPECT_EQ(0, spy.selCount) << L"plain click in multi mode fires no LV SelChanged";
    spy.reset();

    // Band over rows 0..3: the replacing (!bAdd) path clears then refills on
    // EVERY select call - the 1x1 band start deselects+re-selects row 0
    // (2 events) and the extend deselects row 0 then adds rows 0..3 (5
    // events). Chatty by design: AddSelItem/RemoveSelItem report every map
    // transition, no set diffing.
    pLv->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 10));
    spy.reset();
    pLv->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 75));
    EXPECT_TRUE(pLv->IsRubberBandSelActive());
    EXPECT_EQ(7, spy.itemCount) << L"2 (band start) + 1 deselect + 4 selects";
    EXPECT_EQ(3, spy.lastItem);
    EXPECT_EQ(1, spy.lastState);
    EXPECT_EQ(0, spy.selCount) << L"no anchor-style SelChanged during the band";

    // ESC cancels the band and restores the pre-band selection: rows 0..3
    // are deselected (4 events) and row 0 re-selected (1 event). Cancel
    // fires no anchor-style SelChanged either - the restore is reported
    // entirely through per-item events.
    pLv->SSendMessage(WM_KEYDOWN, VK_ESCAPE, 0);
    EXPECT_FALSE(pLv->IsRubberBandSelActive());
    EXPECT_EQ(12, spy.itemCount) << L"4 deselects + 1 re-select on restore";
    EXPECT_EQ(0, spy.selCount) << L"cancel must not fire SelChanged";
    EXPECT_FALSE(pLv->IsItemSelected(3));
    EXPECT_TRUE(pLv->IsItemSelected(0));

    root.DestroyAllChildren();
}

TEST(soui_rubberband, treectrl_band_fires_item_sel_changed)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"200\" height=\"200\">"
        L"  <treectrl name=\"tc\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" itemHeight=\"20\"/>"
        L"</window>"));
    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName(L"tc"));
    ASSERT_TRUE(pTc);

    SArray<HSTREEITEM> items;
    for (int i = 0; i < 6; i++)
    {
        SStringT str;
        str.Format(_T("item%d"), i);
        items.Add(pTc->InsertItem(str, STVI_ROOT, STVI_LAST));
    }
    pTc->Move(CRect(0, 0, 200, 200));

    SelSpy spy;
    ASSERT_TRUE(pTc->GetEventSet()->subscribeEvent(&SelSpy::onTreeItem, &spy));
    ASSERT_TRUE(pTc->GetEventSet()->subscribeEvent(&SelSpy::onTC, &spy));

    // The press records item1 in the multi-selection set (one per-item
    // event; no anchor-style event fires). Reset so the assertions below
    // only see band-time events.
    pTc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 25));
    spy.reset();

    // Band: the 1x1 band start clears the press selection (1 deselect) and
    // re-adds item1 (1 event); the extend clears (1 deselect) and re-adds
    // rows 1..3 (3 events) - the clear+refill runs on every select call, no
    // set diffing.
    pTc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 75));
    EXPECT_TRUE(pTc->IsRubberBandSelActive());
    EXPECT_EQ(6, spy.itemCount);
    EXPECT_EQ((int)(INT_PTR)items[3], spy.lastItem);
    EXPECT_EQ(1, spy.lastState);
    EXPECT_EQ(0, spy.selCount) << L"no anchor-style SelChanged during the band";

    // ESC while the band is still active restores the band-start snapshot
    // {item1}: three deselects plus one re-select of the pressed row. Cancel
    // fires no anchor-style SelChanged either (see treectrl_esc_cancels_band).
    pTc->SSendMessage(WM_KEYDOWN, VK_ESCAPE, 0);
    EXPECT_FALSE(pTc->IsRubberBandSelActive());
    EXPECT_EQ(10, spy.itemCount) << L"3 deselects + 1 re-select on restore";
    EXPECT_EQ(1, spy.lastState);
    EXPECT_EQ(0, spy.selCount) << L"cancel must not fire SelChanged";
    EXPECT_EQ(1, pTc->GetSelItemCount());
    EXPECT_TRUE(pTc->IsItemSelected(items[1]));

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SListView regression: after the dataset shrinks, selected indices that fell
// beyond the new item count must be pruned. Otherwise getCount() can drop below
// a shift-range anchor (or a multi-selection entry), GetSelItems() yields
// out-of-range indices and feeding them to the adapter reads past the end.
// ---------------------------------------------------------------------------
namespace
{
    class ShrinkingLvAdapter : public SAdapterBase
    {
      public:
		ShrinkingLvAdapter():m_nCount(10){}

        STDMETHOD_(int, getCount)(THIS) OVERRIDE
        {
            return m_nCount;
        }

        STDMETHOD_(void, getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate) OVERRIDE
        {
            if (pItem->GetChildrenCount() == 0)
            {
                SXmlNode xmlItem = xmlTemplate.child(L"itemTemp");
                if (xmlItem)
                    pItem->InitFromXml(&xmlItem);
            }
        }

        void SetCount(int n)
        {
            m_nCount = n;
        }

      private:
        int m_nCount;
    };
}

TEST(soui_rubberband, listview_dataset_shrink_prunes_selection_beyond_count)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"400\" height=\"400\">"
        L"  <listview name=\"lv\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" dividerSize=\"0\">"
        L"    <template itemHeight=\"20\">"
        L"      <itemTemp>"
        L"        <text size=\"200,20\"/>"
        L"      </itemTemp>"
        L"    </template>"
        L"  </listview>"
        L"</window>"));
    SListView *pLv = sobj_cast<SListView>(root.FindChildByName(L"lv"));
    ASSERT_TRUE(pLv);

    ShrinkingLvAdapter adapter;
    ASSERT_TRUE(pLv->SetAdapter(&adapter));
    pLv->Move(CRect(0, 0, 200, 200));

    // Build a multi-selection spanning indices 4..9 (rows 5..9 will become
    // stale once the dataset shrinks to 5 rows).
    for (int i = 4; i <= 9; i++)
        pLv->AddSelItem(i);
    EXPECT_EQ(6, pLv->GetSelItemCount());

    // Shrink the dataset and re-notify. Re-setting the same adapter hits the
    // "same as previous adapter" branch which calls onDataSetChanged() without
    // resetting the selection built above.
    adapter.SetCount(5);
    ASSERT_TRUE(pLv->SetAdapter(&adapter));

    // Every reported selection index must be within the new item count.
    int items[16];
    int n = pLv->GetSelItems(items, 16);
    EXPECT_GE(n, 0);
    EXPECT_LE(n, 5);
    for (int i = 0; i < n; i++)
        EXPECT_LT(items[i], 5) << L"selection must not leak past the new item count";

    // The surviving in-range row stays selected; the pruned row is gone.
    EXPECT_TRUE(pLv->IsItemSelected(4));
    EXPECT_FALSE(pLv->IsItemSelected(5));

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SListView regression: a plain (no modifier) click on an already-selected
// item in multi-select mode must collapse the set to just that item. The set
// used to stay unchanged because the normal-click branch skipped SetSel when
// the item was already selected.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, listview_multisel_normal_click_collapses_selection)
{
    HeadlessApp app;
    TestContainer container;

    SWindow root;
    ASSERT_TRUE(CreateRootFromXml(root, &container,
        L"<window width=\"400\" height=\"400\">"
        L"  <listview name=\"lv\" size=\"200,200\" multiSel=\"1\" bandEnable=\"1\" dividerSize=\"0\">"
        L"    <template itemHeight=\"20\">"
        L"      <itemTemp>"
        L"        <text size=\"200,20\"/>"
        L"      </itemTemp>"
        L"    </template>"
        L"  </listview>"
        L"</window>"));
    SListView *pLv = sobj_cast<SListView>(root.FindChildByName(L"lv"));
    ASSERT_TRUE(pLv);

    TestLvAdapter adapter;
    ASSERT_TRUE(pLv->SetAdapter(&adapter));
    pLv->Move(CRect(0, 0, 200, 200));

    // Multi-select {0,1,2}.
    for (int i = 0; i <= 2; i++)
        pLv->AddSelItem(i);
    EXPECT_EQ(3, pLv->GetSelItemCount());

    // Plain click on already-selected row 2 (host y = 2*20 = 40): the set must
    // collapse to just that row, not keep {0,1,2}.
    pLv->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 40));
    pLv->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(50, 40));

    EXPECT_EQ(1, pLv->GetSelItemCount());
    EXPECT_TRUE(pLv->IsItemSelected(2));
    EXPECT_FALSE(pLv->IsItemSelected(0));
    EXPECT_FALSE(pLv->IsItemSelected(1));

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SListCtrl: check-box mode (m_bCheckBox) is unrelated to multi-selection.
// It only renders a check box inside each item; rubber-band selection is
// gated on m_bMultiSelection alone, so a check-box-only list must NOT start
// a band and a drag must never batch-check rows.
// ---------------------------------------------------------------------------
TEST(soui_rubberband, listctrl_checkbox_only_no_band_select)
{
    HeadlessApp app;
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
    for (int i = 0; i < 10; i++)
        pLc->InsertItem(-1, _T("item"));
    pLc->Move(CRect(0, 0, 200, 200));

    // Press on row 0 (client y=25) and drag down to y=95: without
    // multiSelection the band must never engage.
    pLc->SSendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(50, 25));
    pLc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 40));
    EXPECT_FALSE(pLc->IsRubberBandSelActive()) << L"band must not start in check-box-only mode";
    pLc->SSendMessage(WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(50, 95));
    EXPECT_FALSE(pLc->IsRubberBandSelActive());

    // Only the pressed row got checked by the plain click (existing
    // check-box click behavior); the drag must NOT batch-check rows 1..9.
    EXPECT_TRUE(pLc->GetCheckState(0));
    for (int i = 1; i < 10; i++)
        EXPECT_FALSE(pLc->GetCheckState(i)) << L"row " << i;

    pLc->SSendMessage(WM_LBUTTONUP, 0, MAKELPARAM(50, 95));
    EXPECT_FALSE(pLc->IsRubberBandSelActive());

    root.DestroyAllChildren();
}
