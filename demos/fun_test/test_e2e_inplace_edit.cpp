/**
 * @file       test_e2e_inplace_edit.cpp
 * @brief      Headless E2E tests for in-place label editing (STreeCtrl / SListCtrl).
 *
 * Covered headless: editor creation prefilled with the item text, commit
 * writing the text back, ESC-style discard, canceling via the begin event,
 * the end-event fields, F2 starting the edit, and the editLabel=0
 * behavior-preservation path.
 *
 * NOT covered headless (needs a real desktop / demo window, editLabel="1"):
 *  - double click on an item text starts the editor (and does not toggle /
 *    fire the double-click event)
 *  - committing by clicking another window (focus loss)
 *  - visual placement of the editor over the label
 * Manual steps in a demo app:
 *  1. double click an item text -> an edit opens over the label, text selected
 *  2. press F2 on the selected item -> same
 *  3. type, press RETURN (or TAB) -> the text is written back to the item
 *  4. type, press ESC -> the item text is unchanged
 *  5. type, click another window -> the text is written back
 *  6. while editing, scroll / resize / drag a column header (listctrl) ->
 *     the editor closes and the text is written back
 */

#include <souistd.h>
#include <core/SWnd.h>
#include <control/STreeCtrl.h>
#include <control/SListCtrl.h>
#include <control/SEdit.h>
#include <gtest/gtest.h>
#include "common.h"
#include "test_container.h"

using namespace SOUI;

namespace
{
    /// Records the fields of a tree end-edit event.
    struct TcEndRecorder
    {
        BOOL fired;
        HSTREEITEM hItem;
        SStringT strText;
        BOOL bCancel;

        TcEndRecorder()
            : fired(FALSE)
            , hItem(0)
            , bCancel(FALSE)
        {
        }

        BOOL OnEnd(IEvtArgs *pEvt)
        {
            EventTCEndEditLabel *p = static_cast<EventTCEndEditLabel *>(pEvt);
            fired = TRUE;
            hItem = p->hItem;
            strText = p->strText->c_str();
            bCancel = p->bCancel;
            return FALSE;
        }
    };

    /// Records the fields of a list end-edit event.
    struct LcEndRecorder
    {
        BOOL fired;
        int nItem;
        int nSubItem;
        SStringT strText;
        BOOL bCancel;

        LcEndRecorder()
            : fired(FALSE)
            , nItem(-1)
            , nSubItem(-1)
            , bCancel(FALSE)
        {
        }

        BOOL OnEnd(IEvtArgs *pEvt)
        {
            EventLCEndEditLabel *p = static_cast<EventLCEndEditLabel *>(pEvt);
            fired = TRUE;
            nItem = p->nItem;
            nSubItem = p->nSubItem;
            strText = p->strText->c_str();
            bCancel = p->bCancel;
            return FALSE;
        }
    };

    /// Cancels every tree begin-edit event.
    struct TcBeginCanceller
    {
        BOOL OnBegin(IEvtArgs *pEvt)
        {
            static_cast<EventTCBeginEditLabel *>(pEvt)->bCancel = TRUE;
            return FALSE;
        }
    };

    /// Cancels every list begin-edit event.
    struct LcBeginCanceller
    {
        BOOL OnBegin(IEvtArgs *pEvt)
        {
            static_cast<EventLCBeginEditLabel *>(pEvt)->bCancel = TRUE;
            return FALSE;
        }
    };

    /// Loads the inplace-edit layout into a container-backed root and lays it
    /// out manually: headless tests have no real host, so without an explicit
    /// Move the window rects stay empty and rect-based code would see a
    /// zero-size client. The container must be set before InitFromResId so
    /// events fired during creation reach subscribers.
    void InitRootWithLayout(SWindow &root, TestContainer &container)
    {
        root.SetContainer(&container);
        ASSERT_TRUE(root.InitFromResId(_T("layout:XML_INPLACE_EDIT_E2E")));
        CRect rcRoot(0, 0, 400, 320);
        root.Move(&rcRoot);
    }
}

// ---------------------------------------------------------------------------
// STreeCtrl: BeginInplaceEdit creates a child editor prefilled with the item
// text; committing writes the edited text back and fires the end event.
// ---------------------------------------------------------------------------
TEST(soui_inplace_edit_e2e, treectrl_commit_writes_back)
{
    HeadlessApp app(_T("/e2e-res"));
    TestContainer container;
    SWindow root;
    InitRootWithLayout(root, container);

    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName("tc"));
    ASSERT_TRUE(pTc);

    HSTREEITEM hItem = pTc->InsertItem(_T("child"));
    ASSERT_TRUE(hItem != 0);

    TcEndRecorder rec;
    pTc->GetEventSet()->subscribeEvent(EventTCEndEditLabel::EventID, Subscriber(&TcEndRecorder::OnEnd, &rec));

    ASSERT_TRUE(pTc->BeginInplaceEdit(hItem));
    ASSERT_TRUE(pTc->IsInplaceEditing());

    // The editor is the control's only child window, prefilled with the
    // item text. Text must go through SEdit: the SWindow-level accessors
    // only touch the window attribute, not the edit engine.
    ASSERT_EQ(pTc->SWindow::GetChildrenCount(), 1u);
    // GetChild uses accessibility child ids: 0 = the window itself, 1 = first child.
    SEdit *pEdit = sobj_cast<SEdit>(pTc->GetChild(1));
    ASSERT_TRUE(pEdit != NULL);
    EXPECT_STREQ(pEdit->GetWindowText().c_str(), _T("child"));

    pEdit->SetWindowText(_T("edited"));
    pTc->EndInplaceEdit(TRUE);

    EXPECT_FALSE(pTc->IsInplaceEditing());
    EXPECT_EQ(pTc->SWindow::GetChildrenCount(), 0u);
    SStringT str;
    pTc->GetItemText(hItem, &str);
    EXPECT_STREQ(str.c_str(), _T("edited"));
    EXPECT_TRUE(pTc->IsFocused());

    EXPECT_TRUE(rec.fired);
    EXPECT_EQ(rec.hItem, hItem);
    EXPECT_FALSE(rec.bCancel);
    EXPECT_STREQ(rec.strText.c_str(), _T("edited"));

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// STreeCtrl: discarding (EndInplaceEdit(FALSE)) keeps the item text and fires
// the end event with bCancel=TRUE.
// ---------------------------------------------------------------------------
TEST(soui_inplace_edit_e2e, treectrl_cancel_discards)
{
    HeadlessApp app(_T("/e2e-res"));
    TestContainer container;
    SWindow root;
    InitRootWithLayout(root, container);

    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName("tc"));
    ASSERT_TRUE(pTc);

    HSTREEITEM hItem = pTc->InsertItem(_T("keep"));
    ASSERT_TRUE(hItem != 0);

    TcEndRecorder rec;
    pTc->GetEventSet()->subscribeEvent(EventTCEndEditLabel::EventID, Subscriber(&TcEndRecorder::OnEnd, &rec));

    ASSERT_TRUE(pTc->BeginInplaceEdit(hItem));
    SEdit *pEdit = sobj_cast<SEdit>(pTc->GetChild(1));
    ASSERT_TRUE(pEdit != NULL);
    pEdit->SetWindowText(_T("changed"));

    pTc->EndInplaceEdit(FALSE);

    EXPECT_FALSE(pTc->IsInplaceEditing());
    EXPECT_EQ(pTc->SWindow::GetChildrenCount(), 0u);
    SStringT str;
    pTc->GetItemText(hItem, &str);
    EXPECT_STREQ(str.c_str(), _T("keep"));

    EXPECT_TRUE(rec.fired);
    EXPECT_EQ(rec.hItem, hItem);
    EXPECT_TRUE(rec.bCancel);
    EXPECT_STREQ(rec.strText.c_str(), _T("changed"));

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// STreeCtrl: canceling inside the begin event prevents the editor from being
// created.
// ---------------------------------------------------------------------------
TEST(soui_inplace_edit_e2e, treectrl_begin_event_can_cancel)
{
    HeadlessApp app(_T("/e2e-res"));
    TestContainer container;
    SWindow root;
    InitRootWithLayout(root, container);

    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName("tc"));
    ASSERT_TRUE(pTc);

    HSTREEITEM hItem = pTc->InsertItem(_T("child"));
    ASSERT_TRUE(hItem != 0);

    TcBeginCanceller beg;
    pTc->GetEventSet()->subscribeEvent(EventTCBeginEditLabel::EventID, Subscriber(&TcBeginCanceller::OnBegin, &beg));

    EXPECT_FALSE(pTc->BeginInplaceEdit(hItem));
    EXPECT_FALSE(pTc->IsInplaceEditing());
    EXPECT_EQ(pTc->SWindow::GetChildrenCount(), 0u);

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// STreeCtrl: F2 on the selected item starts the edit session.
// ---------------------------------------------------------------------------
TEST(soui_inplace_edit_e2e, treectrl_f2_starts_edit)
{
    HeadlessApp app(_T("/e2e-res"));
    TestContainer container;
    SWindow root;
    InitRootWithLayout(root, container);

    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName("tc"));
    ASSERT_TRUE(pTc);

    HSTREEITEM hItem = pTc->InsertItem(_T("f2target"));
    ASSERT_TRUE(hItem != 0);
    pTc->SelectItem(hItem);

    EXPECT_FALSE(pTc->IsInplaceEditing());
    pTc->SSendMessage(WM_KEYDOWN, VK_F2, 0);
    EXPECT_TRUE(pTc->IsInplaceEditing());

    SEdit *pEdit = sobj_cast<SEdit>(pTc->GetChild(1));
    ASSERT_TRUE(pEdit != NULL);
    EXPECT_STREQ(pEdit->GetWindowText().c_str(), _T("f2target"));

    pTc->EndInplaceEdit(FALSE);
    EXPECT_FALSE(pTc->IsInplaceEditing());

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SListCtrl: BeginInplaceEdit creates a child editor prefilled with the
// subitem text; committing writes it back via the subitem API.
// ---------------------------------------------------------------------------
TEST(soui_inplace_edit_e2e, listctrl_commit_writes_back)
{
    HeadlessApp app(_T("/e2e-res"));
    TestContainer container;
    SWindow root;
    InitRootWithLayout(root, container);

    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName("lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    ASSERT_EQ(pLc->InsertItem(0, _T("item0")), 0);

    LcEndRecorder rec;
    pLc->GetEventSet()->subscribeEvent(EventLCEndEditLabel::EventID, Subscriber(&LcEndRecorder::OnEnd, &rec));

    ASSERT_TRUE(pLc->BeginInplaceEdit(0, 0));
    ASSERT_TRUE(pLc->IsInplaceEditing());

    // The header is the first child window (id 1); the editor is the last one.
    UINT nChildren = pLc->GetChildrenCount();
    ASSERT_EQ(nChildren, 2u);
    SEdit *pEdit = sobj_cast<SEdit>(pLc->GetChild((int)nChildren));
    ASSERT_TRUE(pEdit != NULL);
    EXPECT_STREQ(pEdit->GetWindowText().c_str(), _T("item0"));

    pEdit->SetWindowText(_T("renamed"));
    pLc->EndInplaceEdit(TRUE);

    EXPECT_FALSE(pLc->IsInplaceEditing());
    EXPECT_EQ(pLc->GetChildrenCount(), nChildren - 1);
    EXPECT_STREQ(pLc->GetSubItemText(0, 0).c_str(), _T("renamed"));

    EXPECT_TRUE(rec.fired);
    EXPECT_EQ(rec.nItem, 0);
    EXPECT_EQ(rec.nSubItem, 0);
    EXPECT_FALSE(rec.bCancel);
    EXPECT_STREQ(rec.strText.c_str(), _T("renamed"));

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// SListCtrl: discarding keeps the subitem text; the begin event can cancel.
// ---------------------------------------------------------------------------
TEST(soui_inplace_edit_e2e, listctrl_cancel_and_begin_cancel)
{
    HeadlessApp app(_T("/e2e-res"));
    TestContainer container;
    SWindow root;
    InitRootWithLayout(root, container);

    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName("lc"));
    ASSERT_TRUE(pLc);

    pLc->InsertColumn(0, _T("col"), 200, 0);
    ASSERT_EQ(pLc->InsertItem(0, _T("item0")), 0);

    // Discard path.
    LcEndRecorder rec;
    pLc->GetEventSet()->subscribeEvent(EventLCEndEditLabel::EventID, Subscriber(&LcEndRecorder::OnEnd, &rec));

    ASSERT_TRUE(pLc->BeginInplaceEdit(0, 0));
    SEdit *pEdit = sobj_cast<SEdit>(pLc->GetChild(pLc->GetChildrenCount()));
    ASSERT_TRUE(pEdit != NULL);
    pEdit->SetWindowText(_T("dropped"));

    pLc->EndInplaceEdit(FALSE);

    EXPECT_FALSE(pLc->IsInplaceEditing());
    EXPECT_STREQ(pLc->GetSubItemText(0, 0).c_str(), _T("item0"));
    EXPECT_TRUE(rec.fired);
    EXPECT_TRUE(rec.bCancel);
    EXPECT_STREQ(rec.strText.c_str(), _T("dropped"));

    // Begin-cancel path.
    LcBeginCanceller beg;
    pLc->GetEventSet()->subscribeEvent(EventLCBeginEditLabel::EventID, Subscriber(&LcBeginCanceller::OnBegin, &beg));

    EXPECT_FALSE(pLc->BeginInplaceEdit(0, 0));
    EXPECT_FALSE(pLc->IsInplaceEditing());

    root.DestroyAllChildren();
}

// ---------------------------------------------------------------------------
// Behavior preservation: with editLabel=0 BeginInplaceEdit is rejected.
// ---------------------------------------------------------------------------
TEST(soui_inplace_edit_e2e, editlabel_disabled_rejects_begin)
{
    HeadlessApp app(_T("/e2e-res"));
    TestContainer container;
    SWindow root;
    InitRootWithLayout(root, container);

    STreeCtrl *pTc = sobj_cast<STreeCtrl>(root.FindChildByName("tc_noedit"));
    ASSERT_TRUE(pTc);

    HSTREEITEM hItem = pTc->InsertItem(_T("x"));
    ASSERT_TRUE(hItem != 0);

    EXPECT_FALSE(pTc->BeginInplaceEdit(hItem));
    EXPECT_FALSE(pTc->IsInplaceEditing());
    EXPECT_EQ(pTc->SWindow::GetChildrenCount(), 0u);

    SListCtrl *pLc = sobj_cast<SListCtrl>(root.FindChildByName("lc"));
    ASSERT_TRUE(pLc);

    root.DestroyAllChildren();
}
