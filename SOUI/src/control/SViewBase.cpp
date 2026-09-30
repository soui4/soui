#include "control/SViewBase.h"

SNSBEGIN

SViewBase::SViewBase(SPanel *pView)
    : SHostProxy(pView)
    , m_pView(pView)
    , m_bPendingUpdate(false)
    , m_iPendingUpdateItem(-2)
    , m_iPendingViewItem(-1)
    , m_iFirstVisible(0)
    , m_itemCapture(NULL)
    , m_iSelItem(-1)
    , m_iSelAnchor(-1)
    , m_pHoverItem(NULL)
    , m_bDatasetInvalidated(FALSE)
    , m_bWantTab(FALSE)
    , m_bMultiSel(FALSE)
{
}

SViewBase::~SViewBase()
{
    // Clean up item recycle
    for (int i = 0; i < m_itemRecycle.GetCount(); i++)
    {
        SList<SItemPanel *> *pList = m_itemRecycle[i];
        if (pList)
        {
            while (!pList->IsEmpty())
            {
                SItemPanel *pItem = pList->RemoveHead();
                if (pItem)
                {
                    pItem->Destroy();
                    delete pItem;
                }
            }
            delete pList;
        }
    }
    m_itemRecycle.RemoveAll();
}

void SViewBase::OnItemSetCapture(SOsrPanel *pItem, BOOL bCapture)
{
    if (bCapture)
    {
        m_pHost->GetContainer()->OnSetSwndCapture(m_pHost->GetSwnd());
        m_itemCapture = pItem;
    }
    else
    {
        m_pHost->GetContainer()->OnReleaseSwndCapture();
        m_itemCapture = NULL;
    }
}

BOOL SViewBase::OnItemGetRect(const SOsrPanel *pItem, CRect &rcItem) const
{
    if (!pItem)
        return FALSE;
    rcItem = pItem->GetWindowRect();
    return TRUE;
}

BOOL SViewBase::IsItemRedrawDelay() const
{
    return FALSE;
}

BOOL SViewBase::IsTimelineEnabled() const
{
    return FALSE;
}

void SViewBase::onDataSetChanged()
{
    m_bPendingUpdate = true;
    m_iPendingUpdateItem = -1;
    m_pHost->Invalidate();
    if (m_pView)
        m_pView->accNotifyEvent(EVENT_OBJECT_REORDER); // Visible set/data changed, notify screen reader to rebuild the child tree
}

void SViewBase::onDataSetInvalidated()
{
    m_bPendingUpdate = true;
    m_iPendingUpdateItem = -1;
    m_bDatasetInvalidated = TRUE;
    m_pHost->Invalidate();
    if (m_pView)
        m_pView->accNotifyEvent(EVENT_OBJECT_REORDER);
}

void SViewBase::onItemDataChanged(int iItem)
{
    m_bPendingUpdate = true;
    m_iPendingUpdateItem = iItem;
    m_pHost->Invalidate();
}

void SViewBase::DispatchMessage2Items(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    for (SPOSITION it = m_lstItems.GetHeadPosition(); it;)
    {
        ItemInfo &itInfo = m_lstItems.GetNext(it);
        itInfo.pItem->SDispatchMessage(uMsg, wParam, lParam);
    }
}

SWindow *SViewBase::GetVisibleAccChild(int iChild) const
{
    if (iChild < 1)
        return NULL;
    int n = 0;
    for (SPOSITION it = m_lstItems.GetHeadPosition(); it;)
    {
        const ItemInfo &itInfo = m_lstItems.GetNext(it);
        n++;
        if (n == iChild)
            return itInfo.pItem;
    }
    return NULL;
}

int SViewBase::GetVisibleAccSelIndex() const
{
    int n = 0;
    for (SPOSITION it = m_lstItems.GetHeadPosition(); it;)
    {
        const ItemInfo &itInfo = m_lstItems.GetNext(it);
        n++;
        if (itInfo.pItem && itInfo.pItem->GetItemIndex() == (LPARAM)m_iSelItem)
            return n;
    }
    return 0;
}

void SViewBase::SetMultiSel(BOOL bMultiSel)
{
    if (!bMultiSel && m_bMultiSel)
    {
        // Turning multi-selection off. Note: m_mapSelItems must be inspected
        // directly - GetSelItemCount() switches semantics once m_bMultiSel
        // becomes FALSE.
        int nCount = m_mapSelItems.GetCount();
        if (nCount > 1)
        {
            // Several items selected: the whole selection is cleared,
            // including the single-selection anchor.
            ClearSelItems();
            if (m_iSelItem != -1)
            {
                SItemPanel *pItem = GetItemPanel(m_iSelItem);
                if (pItem)
                {
                    pItem->ModifyItemState(0, WndState_Check);
                    RedrawItem(pItem);
                }
                m_iSelItem = -1;
            }
            m_iSelAnchor = -1;
        }
        else if (nCount == 1)
        {
            // Exactly one item selected: transfer it to the single-selection
            // anchor. Single-selection mode reads m_iSelItem (see
            // IsItemSelected), not the map; the item's panel already carries
            // the checked state, so only ownership moves.
            int iItem = -1;
            BOOL bVal;
            for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos;)
            {
                m_mapSelItems.GetNextAssoc(pos, iItem, bVal);
            }
            m_mapSelItems.RemoveAll();
            m_iSelItem = iItem;
            m_iSelAnchor = iItem;
        }
        // nCount == 0: the single-selection anchor is kept unchanged.
    }
    else if (bMultiSel && !m_bMultiSel)
    {
        // Turning multi-selection on: the map becomes the only selection
        // record (all queries read it exclusively in multi mode). Make sure
        // it holds the anchor's item - a pure record move, no event, no
        // repaint (the item is already drawn selected). The anchor itself
        // keeps its cursor role (keyboard navigation, shift-range base),
        // but it is never a selection record in multi mode.
        if (m_iSelItem != -1)
            m_mapSelItems[m_iSelItem] = TRUE;
        m_iSelAnchor = m_iSelItem;
    }
    m_bMultiSel = bMultiSel;
}

BOOL SViewBase::GetMultiSel() const
{
    return m_bMultiSel;
}

void SViewBase::SelectItem(int iItem, BOOL bNotify)
{
    ILvAdapter *pAdapter = getAdapter();
    if (!pAdapter)
        return;
    if (iItem >= pAdapter->getCount())
        return;
    if (iItem < 0)
        iItem = -1;

    int nOldSel = m_iSelItem;
    int nNewSel = iItem;

    if (m_bMultiSel)
    {
        // Multi-selection mode: a plain selection replaces the whole set,
        // recorded in the map only. Per-item events report the flips - the
        // anchor-style LV SelChanging/SelChanged are a single-selection
        // protocol and are not fired here. The anchor only tracks the
        // cursor (keyboard navigation / shift-range base).
        if (nNewSel != -1 && IsItemSelected(nNewSel) && m_mapSelItems.GetCount() == 1)
        {
            // The set is already exactly {nNewSel}: only the cursor follows,
            // no per-item events, no repaint.
            m_iSelItem = nNewSel;
            m_iSelAnchor = nNewSel;
            return;
        }
        // Release item focus on the old cursor panel (parity with the
        // single-selection path) before the set is rebuilt.
        SItemPanel *pOldCursor = GetItemPanel(m_iSelItem);
        if (pOldCursor && m_iSelItem != nNewSel)
            pOldCursor->GetFocusManager()->ClearFocus();
        ClearSelItems();
        m_iSelItem = nNewSel;
        m_iSelAnchor = nNewSel;
        if (nNewSel != -1)
            AddSelItem(nNewSel);
        return;
    }

    // Single selection mode: the anchor m_iSelItem is the selection record
    // (IsItemSelected reads it); the map is the multi-selection record only.
    if (IsItemSelected(nNewSel))
        return;

    // Clearing an already-empty selection changes nothing: no visuals and
    // no SelChanging/SelChanged pair.
    if (nNewSel == -1 && nOldSel == -1)
        return;

    if (bNotify)
    {
        EventLVSelChanging evt(m_pView);
        evt.bCancel = FALSE;
        evt.iOldSel = nOldSel;
        evt.iNewSel = nNewSel;
        m_pView->FireEvent(evt);
        if (evt.bCancel)
            return;
    }

    // Clear old selection
    SItemPanel *pItem = GetItemPanel(m_iSelItem);
    if (pItem)
    {
        pItem->GetFocusManager()->ClearFocus();
        pItem->SetSelected(FALSE);
        RedrawItem(pItem);
    }

    // Set new selection. No per-item event here - the anchor-style
    // LVSelChanging/LVSelChanged pair is the single-selection feedback.
    m_iSelItem = nNewSel;
    m_iSelAnchor = nNewSel;
    pItem = GetItemPanel(nNewSel);
    if (pItem)
    {
        pItem->ModifyItemState(WndState_Check, 0);
        RedrawItem(pItem);
    }

    if (bNotify)
    {
        EventLVSelChanged evt(m_pView);
        evt.iOldSel = nOldSel;
        evt.iNewSel = nNewSel;
        m_pView->FireEvent(evt);
    }
}

void SViewBase::SetSelRange(int iAnchor, int iCursor)
{
    if (!m_bMultiSel || iCursor < 0)
        return;
    if (iAnchor < 0)
        iAnchor = iCursor;
    int iLo = smin(iAnchor, iCursor);
    int iHi = smax(iAnchor, iCursor);

    // Deselect everything outside the anchored range (snapshot first:
    // RemoveSelItem mutates the map while firing events).
    SArray<int> arrSel;
    for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos;)
    {
        int iItem;
        BOOL bVal;
        m_mapSelItems.GetNextAssoc(pos, iItem, bVal);
        arrSel.Add(iItem);
    }
    for (int i = 0; i < (int)arrSel.GetCount(); i++)
    {
        if (arrSel[i] < iLo || arrSel[i] > iHi)
            RemoveSelItem(arrSel[i]);
    }
    // Select everything inside the range that is not selected yet.
    for (int i = iLo; i <= iHi; i++)
    {
        if (!IsItemSelected(i))
            AddSelItem(i);
    }
}

void SViewBase::AddSelItem(int iItem)
{
    if (iItem < 0)
        return;

    // Already in the multi-selection map: the state does not change and no
    // per-item event fires. (Single-selection mode relies on m_iSelItem; the
    // map is cleared right before by SetSelItem, so the add always proceeds.)
    const ItemSelectionMap::CPair *pPair = m_mapSelItems.Lookup(iItem);
    if (pPair && pPair->m_value)
        return;

    m_mapSelItems[iItem] = TRUE;

    // Update the visible item's state
    SItemPanel *pItem = GetItemPanel(iItem);
    if (pItem)
    {
        pItem->ModifyItemState(WndState_Check, 0);
        RedrawItem(pItem);
    }

    if (m_pView)
    {
        EventItemSelChanged evt(m_pView);
        evt.iItem = iItem;
        evt.bSelected = TRUE;
        m_pView->FireEvent(evt);
    }
}

void SViewBase::RemoveSelItem(int iItem)
{
    if (iItem < 0)
        return;

    // Not in the multi-selection map: nothing to remove, no event.
    if (!m_mapSelItems.RemoveKey(iItem))
        return;

    // Update the visible item's state
    SItemPanel *pItem = GetItemPanel(iItem);
    if (pItem)
    {
        pItem->ModifyItemState(0, WndState_Check);
        RedrawItem(pItem);
    }

    if (m_pView)
    {
        EventItemSelChanged evt(m_pView);
        evt.iItem = iItem;
        evt.bSelected = FALSE;
        m_pView->FireEvent(evt);
    }
}

void SViewBase::ClearSelItems()
{
    // Route through RemoveSelItem so every deselected item fires its
    // per-item selection-state event and updates its panel state. Keys are
    // collected first: RemoveSelItem mutates the map while firing events.
    SArray<int> arrSel;
    for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos;)
    {
        int iItem;
        BOOL bVal;
        m_mapSelItems.GetNextAssoc(pos, iItem, bVal);
        arrSel.Add(iItem);
    }
    for (int i = 0; i < (int)arrSel.GetCount(); i++)
    {
        RemoveSelItem(arrSel[i]);
    }
}

void SViewBase::PruneSelItems(int nNewCount)
{
    if (nNewCount < 0)
        nNewCount = 0;

    // Multi-selection cursor becomes invalid when it points past the end.
    if (m_iSelItem != -1 && m_iSelItem >= nNewCount)
        m_iSelItem = -1;

    // The shift-range anchor can be a different (larger) index than the
    // cursor; normalize it independently so it never dangles past the end.
    if (m_iSelAnchor != -1 && m_iSelAnchor >= nNewCount)
        m_iSelAnchor = (m_iSelItem != -1) ? m_iSelItem : -1;

    // Drop multi-selection entries that fall beyond the new count so that
    // GetSelItems() never yields an out-of-range index to the adapter.
    // Removed rows no longer exist, so deselect them silently (no per-item
    // event), matching the tree views' pruning rule.
    if (m_bMultiSel)
    {
        SArray<int> arrStale;
        for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos;)
        {
            int iItem;
            BOOL bVal;
            m_mapSelItems.GetNextAssoc(pos, iItem, bVal);
            if (iItem >= nNewCount)
                arrStale.Add(iItem);
        }
        for (int i = 0; i < (int)arrStale.GetCount(); i++)
        {
            m_mapSelItems.RemoveKey(arrStale[i]);
        }
    }
}

void SViewBase::SnapshotSelItems()
{
    m_arrBandSnapshot.RemoveAll();
    int nCount = GetSelItemCount();
    if (nCount <= 0)
        return;
    int *pItems = new int[nCount];
    nCount = GetSelItems(pItems, nCount);
    for (int i = 0; i < nCount; i++)
    {
        m_arrBandSnapshot.Add(pItems[i]);
    }
    delete[] pItems;
}

void SViewBase::RestoreSelItems()
{
    ClearSelItems();
    for (int i = 0; i < (int)m_arrBandSnapshot.GetCount(); i++)
    {
        AddSelItem(m_arrBandSnapshot[i]);
    }
    m_arrBandSnapshot.RemoveAll();
}

BOOL SViewBase::IsItemSelected(int iItem) const
{
    if (iItem < 0)
        return FALSE;
    if (m_bMultiSel)
    {
        const ItemSelectionMap::CPair *pVal = m_mapSelItems.Lookup(iItem);
        return pVal && pVal->m_value;
    }
    else
    {
        // In single-selection mode, m_iSelItem holds the selected item
        return m_iSelItem == iItem;
    }
}

int SViewBase::GetSelItemCount() const
{
    if (m_bMultiSel)
    {
        return m_mapSelItems.GetCount();
    }
    else
    {
        // In single-selection mode, return 1 if there is a selected item, otherwise return 0
        return m_iSelItem != -1 ? 1 : 0;
    }
}

int SViewBase::GetSelItems(int *pItems, int nMaxCount) const
{
    if (m_bMultiSel)
    {
        // In multi-selection mode, return all items in m_mapSelItems
        int i = 0;
        for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos && nMaxCount > 0;)
        {
            int iItem;
            BOOL bVal;
            m_mapSelItems.GetNextAssoc(pos, iItem, bVal);
            pItems[i++] = iItem;
            nMaxCount--;
        }
        return i;
    }
    else
    {
        // In single-selection mode, return the selected item if there is one
        if (m_iSelItem != -1 && nMaxCount > 0)
        {
            pItems[0] = m_iSelItem;
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

void SViewBase::HandleSelectionChange(int nOldSel, int nNewSel)
{
    if (nOldSel == nNewSel)
    {
        return;
    }

    // Clear old selection if in multi-select mode
    if (m_bMultiSel)
    {
        ClearSelItems();
    }

    // Update old selected item
    SItemPanel *pItem = GetItemPanel(nOldSel);
    if (pItem)
    {
        pItem->GetFocusManager()->ClearFocus();
        pItem->ModifyItemState(0, WndState_Check);
        RedrawItem(pItem);
    }

    // Update new selected item
    m_iSelItem = nNewSel;
    m_iSelAnchor = nNewSel;
    pItem = GetItemPanel(nNewSel);
    if (pItem)
    {
        pItem->ModifyItemState(WndState_Check, 0);
        RedrawItem(pItem);
    }

    // Add new selection if in multi-select mode
    if (m_bMultiSel && nNewSel != -1)
    {
        AddSelItem(nNewSel);
    }
#ifdef SOUI_ENABLE_ACC
    if (m_pView)
        m_pView->accNotifyEvent(EVENT_OBJECT_SELECTION); // Selection changed, notify screen reader
#endif
}

BOOL SViewBase::OnItemClick(IEvtArgs *pEvt)
{
    EventItemPanelClick *pClickEvt = sobj_cast<EventItemPanelClick>(pEvt);
    if (!pClickEvt)
        return TRUE;
    SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pEvt->Sender());
    int iItem = (int)pItemPanel->GetItemIndex();

    BOOL bCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    BOOL bShiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    BOOL bMultiSelMode = GetMultiSel();

    if (bMultiSelMode)
    {
        if (bCtrlPressed)
        {
            // Ctrl+Click: toggle selection of current item
            if (IsItemSelected(iItem))
            {
                RemoveSelItem(iItem);
            }
            else
            {
                AddSelItem(iItem);
            }
            m_iSelItem = iItem;
            m_iSelAnchor = iItem;
        }
        else if (bShiftPressed)
        {
            // Shift+Click: select the anchored range [anchor..clicked]
            // (Explorer semantics: the anchor stays fixed, the set is
            // replaced, so the range can shrink as well as grow).
            SetSelRange(m_iSelAnchor, iItem);
            m_iSelItem = iItem;
        }
        else
        {
            // Normal Click: select only current item. No IsItemSelected guard:
            // clicking an already-selected item in multi-select must collapse
            // the set to just that item (SetSel clears the rest), otherwise a
            // plain click keeps the whole existing selection.
            SelectItem(iItem, TRUE);
        }
    }
    else
    {
        // Single selection mode
        SelectItem(iItem, TRUE);
    }
    return TRUE;
}

BOOL SViewBase::OnItemClickUp(IEvtArgs *pEvt)
{
    SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pEvt->Sender());
    SASSERT(pItemPanel);
    int iItem = (int)pItemPanel->GetItemIndex();
    if (!m_bMultiSel)
        return TRUE;
    UINT uKeyFlags = GetKeyState(VK_CONTROL) & 0x8000 ? MK_CONTROL : 0;
    uKeyFlags |= GetKeyState(VK_SHIFT) & 0x8000 ? MK_SHIFT : 0;
    if (uKeyFlags == 0)
    {
        SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pEvt->Sender());
        int hItem = (int)pItemPanel->GetItemIndex();
        if (GetSelItemCount() > 1)
        {
            // remove other selected items
            SelectItem(iItem, TRUE);
        }
    }
    return TRUE;
}
SNSEND
