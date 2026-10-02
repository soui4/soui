#include "souistd.h"
#include "control/STreeView.h"

SNSBEGIN
class STreeViewDataSetObserver : public TObjRefImpl<ITvDataSetObserver> {
  public:
    STreeViewDataSetObserver(STreeView *pView)
        : m_pOwner(pView)
    {
    }

    STDMETHOD_(void, onBranchChanged)(THIS_ HSTREEITEM hBranch) OVERRIDE
    {
        m_pOwner->onBranchChanged(hBranch);
    }

    STDMETHOD_(void, onBranchInvalidated)
    (THIS_ HSTREEITEM hBranch, BOOL bInvalidParents, BOOL bInvalidChildren) OVERRIDE
    {
        m_pOwner->onBranchInvalidated(hBranch, bInvalidParents, bInvalidChildren);
    }

    STDMETHOD_(void, onBranchExpandedChanged)
    (THIS_ HSTREEITEM hBranch, BOOL bExpandedOld, BOOL bExpandedNew) OVERRIDE
    {
        m_pOwner->onBranchExpandedChanged(hBranch, bExpandedOld, bExpandedNew);
    }

    STDMETHOD_(void, notifyItemBeforeRemove)(THIS_ HSTREEITEM hItem) OVERRIDE
    {
        m_pOwner->onItemBeforeRemove(hItem);
    }

  protected:
    STreeView *m_pOwner;
};

///////////////////////////////////////////////////////////////////////
STreeViewItemLocator::STreeViewItemLocator(int nIndent)
    : m_nLineHeight(30)
    , m_nIndent(nIndent)
    , m_szDef(10, 50)
{
}

STreeViewItemLocator::~STreeViewItemLocator()
{
}

void STreeViewItemLocator::SetAdapter(ITvAdapter *pAdapter)
{
    m_adapter = pAdapter;
}

void STreeViewItemLocator::_InitBranch(HSTREEITEM hItem)
{
    if (hItem != ITEM_ROOT)
    {
        _SetItemHeight(hItem, m_szDef.cy);
        _SetItemWidth(hItem, m_szDef.cx);
    }
    else
    {
        _SetItemHeight(hItem, 0);
        _SetItemWidth(hItem, 0);
    }
    if (m_adapter->HasChildren(hItem))
    { // Has child node
        HSTREEITEM hChild = m_adapter->GetFirstChildItem(hItem);
        int nBranchHeight = 0;
        while (hChild != ITEM_NULL)
        {
            // Set offset
            _SetItemOffset(hChild, nBranchHeight);
            _InitBranch(hChild);
            nBranchHeight += _GetItemVisibleHeight(hChild);
            hChild = m_adapter->GetNextSiblingItem(hChild);
        }
        _SetBranchHeight(hItem, nBranchHeight);
        // Set default width
        _SetBranchWidth(hItem, m_szDef.cx + m_nIndent);
    }
    else
    { // No child node
        _SetBranchHeight(hItem, 0);
        _SetBranchWidth(hItem, 0);
    }
}

BOOL STreeViewItemLocator::_IsItemVisible(HSTREEITEM hItem) const
{
    return m_adapter->IsItemVisible(hItem);
}

HSTREEITEM STreeViewItemLocator::_Position2Item(int position, HSTREEITEM hParent, int nParentPosition) const
{
    if (position < nParentPosition || position >= (nParentPosition + _GetItemVisibleHeight(hParent)))
        return ITEM_NULL;

    int nItemHeight = GetItemHeight(hParent);
    int nPos = nParentPosition + nItemHeight;

    if (nPos > position)
        return hParent;

    SASSERT(IsItemExpanded(hParent));

    int nParentBranchHeight = _GetBranchHeight(hParent);

    if (position - nPos < nParentBranchHeight / 2)
    { // Search starting from first
        HSTREEITEM hItem = m_adapter->GetFirstChildItem(hParent);
        while (hItem)
        {
            int nBranchHeight = _GetItemVisibleHeight(hItem);
            if (nPos + nBranchHeight > position)
            {
                return _Position2Item(position, hItem, nPos);
            }
            nPos += nBranchHeight;
            hItem = m_adapter->GetNextSiblingItem(hItem);
        }
    }
    else
    { // Search starting from last
        nPos += nParentBranchHeight;

        HSTREEITEM hItem = m_adapter->GetLastChildItem(hParent);
        while (hItem)
        {
            int nBranchHeight = _GetItemVisibleHeight(hItem);
            nPos -= nBranchHeight;
            if (nPos <= position)
            {
                return _Position2Item(position, hItem, nPos);
            }
            hItem = m_adapter->GetPrevSiblingItem(hItem);
        }
    }

    SASSERT(FALSE); // Should not reach here
    return ITEM_NULL;
}

int STreeViewItemLocator::_GetItemVisibleWidth(HSTREEITEM hItem) const
{
    int nRet = GetItemWidth(hItem);
    if (m_adapter->IsItemExpanded(hItem) && m_adapter->GetFirstChildItem(hItem) != ITEM_NULL)
    { // branch wid includes indent size of its children.
        nRet = smax(nRet, _GetBranchWidth(hItem));
    }
    return nRet;
}

int STreeViewItemLocator::_GetItemVisibleHeight(HSTREEITEM hItem) const
{
    int nRet = GetItemHeight(hItem);
    if (IsItemExpanded(hItem))
        nRet += _GetBranchHeight(hItem);
    return nRet;
}

void STreeViewItemLocator::_SetItemHeight(HSTREEITEM hItem, int nHeight)
{
    m_adapter->SetItemDataByIndex(hItem, DATA_INDEX_ITEM_HEIGHT, nHeight);
}

void STreeViewItemLocator::_SetItemOffset(HSTREEITEM hItem, int nOffset)
{
    m_adapter->SetItemDataByIndex(hItem, DATA_INDEX_ITEM_OFFSET, nOffset);
}

int STreeViewItemLocator::_GetItemOffset(HSTREEITEM hItem) const
{
    return (int)m_adapter->GetItemDataByIndex(hItem, DATA_INDEX_ITEM_OFFSET);
}

void STreeViewItemLocator::_UpdateSiblingsOffset(HSTREEITEM hItem)
{
    int nOffset = _GetItemOffset(hItem);
    nOffset += _GetItemVisibleHeight(hItem);

    HSTREEITEM hSib = m_adapter->GetNextSiblingItem(hItem);
    while (hSib != ITEM_NULL)
    {
        _SetItemOffset(hSib, nOffset);
        nOffset += _GetItemVisibleHeight(hSib);
        hSib = m_adapter->GetNextSiblingItem(hSib);
    }
    // Note to update the offset of each level of parent nodes
    HSTREEITEM hParent = m_adapter->GetParentItem(hItem);
    if (hParent != ITEM_NULL && hParent != ITEM_ROOT && IsItemExpanded(hParent))
    {
        _UpdateSiblingsOffset(hParent);
    }
}

void STreeViewItemLocator::_UpdateBranchHeight(HSTREEITEM hItem, int nDiff)
{
    HSTREEITEM hParent = m_adapter->GetParentItem(hItem);
    while (hParent != ITEM_NULL)
    {
        int nBranchHeight = _GetBranchHeight(hParent);
        _SetBranchHeight(hParent, nBranchHeight + nDiff);
        hParent = m_adapter->GetParentItem(hParent);
    }
}

void STreeViewItemLocator::_SetBranchHeight(HSTREEITEM hItem, int nHeight)
{
    m_adapter->SetItemDataByIndex(hItem, DATA_INDEX_BRANCH_HEIGHT, nHeight);
}

int STreeViewItemLocator::_GetBranchHeight(HSTREEITEM hItem) const
{
    return (int)m_adapter->GetItemDataByIndex(hItem, DATA_INDEX_BRANCH_HEIGHT);
}

void STreeViewItemLocator::_SetItemWidth(HSTREEITEM hItem, int nWidth)
{
    m_adapter->SetItemDataByIndex(hItem, DATA_INDEX_ITEM_WIDTH, nWidth);
}

void STreeViewItemLocator::_SetBranchWidth(HSTREEITEM hBranch, int nWidth)
{
    m_adapter->SetItemDataByIndex(hBranch, DATA_INDEX_BRANCH_WIDTH, nWidth);
}

int STreeViewItemLocator::_GetBranchWidth(HSTREEITEM hBranch) const
{
    return (int)m_adapter->GetItemDataByIndex(hBranch, DATA_INDEX_BRANCH_WIDTH);
}

void STreeViewItemLocator::_UpdateBranchWidth(HSTREEITEM hItem, int nOldWidth, int nNewWidth)
{
    HSTREEITEM hParent = m_adapter->GetParentItem(hItem);
    if (hParent == ITEM_NULL)
        return;
    int nCurBranchWidth = _GetBranchWidth(hParent);

    int nIndent = hParent == ITEM_ROOT ? 0 : m_nIndent;
    if (nCurBranchWidth != nOldWidth + nIndent)
    { // The parent node's width is not controlled by the current node
        if (nCurBranchWidth < nNewWidth + nIndent)
        { // The new width extends the parent node's display width
            _SetBranchWidth(hParent, nNewWidth + nIndent);
            if (IsItemExpanded(hParent))
                _UpdateBranchWidth(hParent, nCurBranchWidth, nNewWidth + nIndent);
        }
    }
    else
    { // The parent node's width is exactly the display width of hItem
        int nNewBranchWidth;
        if (nNewWidth > nOldWidth)
        {
            nNewBranchWidth = nNewWidth + nIndent;
        }
        else
        {
            HSTREEITEM hSib = m_adapter->GetFirstChildItem(hParent);
            nNewBranchWidth = 0;
            while (hSib != ITEM_NULL)
            {
                nNewBranchWidth = smax(nNewBranchWidth, _GetItemVisibleWidth(hSib));
                hSib = m_adapter->GetNextSiblingItem(hSib);
            }
            nNewBranchWidth += nIndent;
        }
        _SetBranchWidth(hParent, nNewBranchWidth);
        if (IsItemExpanded(hParent))
            _UpdateBranchWidth(hParent, nCurBranchWidth, nNewBranchWidth);
    }
}

BOOL STreeViewItemLocator::IsItemExpanded(HSTREEITEM hItem) const
{
    return (BOOL)m_adapter->IsItemExpanded(hItem);
}

int STreeViewItemLocator::GetItemIndent(HSTREEITEM hItem) const
{
    int nRet = 0;
    for (;;)
    {
        hItem = m_adapter->GetParentItem(hItem);
        if (hItem == ITEM_ROOT)
            break;
        nRet += m_nIndent;
    }
    return nRet;
}

int STreeViewItemLocator::GetIndent() const
{
    return m_nIndent;
}

void STreeViewItemLocator::SetIndent(int nIndent)
{
    m_nIndent = nIndent;
}

int STreeViewItemLocator::GetItemHeight(HSTREEITEM hItem) const
{
    return (int)m_adapter->GetItemDataByIndex(hItem, DATA_INDEX_ITEM_HEIGHT);
}

int STreeViewItemLocator::GetItemWidth(HSTREEITEM hItem) const
{
    return (int)m_adapter->GetItemDataByIndex(hItem, DATA_INDEX_ITEM_WIDTH);
}

void STreeViewItemLocator::SetItemHeight(HSTREEITEM hItem, int nHeight)
{
    int nOldHeight = GetItemHeight(hItem);
    _SetItemHeight(hItem, nHeight);
    if (nOldHeight != nHeight)
    {
        _UpdateBranchHeight(hItem, nHeight - nOldHeight);
        _UpdateSiblingsOffset(hItem);
    }
}

void STreeViewItemLocator::SetItemWidth(HSTREEITEM hItem, int nWidth)
{
    int nOldWidth = GetItemWidth(hItem);
    if (nOldWidth == nWidth)
        return;
    int nOldBranchWidth = _GetItemVisibleWidth(hItem);
    _SetItemWidth(hItem, nWidth);
    int nNewBranchWidth = _GetItemVisibleWidth(hItem);
    if (nOldBranchWidth == nNewBranchWidth)
        return;
    _UpdateBranchWidth(hItem, nOldBranchWidth, nNewBranchWidth);
}

HSTREEITEM STreeViewItemLocator::Position2Item(int position) const
{
    return _Position2Item(position, ITEM_ROOT, 0);
}

int STreeViewItemLocator::Item2Position(HSTREEITEM hItem) const
{
    if (!_IsItemVisible(hItem))
    {
        SASSERT(FALSE);
        return -1;
    }

    int nRet = 0;
    // Get the parent node's start position
    HSTREEITEM hParent = m_adapter->GetParentItem(hItem);
    if (hParent != ITEM_NULL && hParent != ITEM_ROOT)
    {
        nRet = Item2Position(hParent);
        // Skip past the parent node
        nRet += GetItemHeight(hParent);
    }
    // Skip past preceding sibling nodes
    nRet += _GetItemOffset(hItem);

    return nRet;
}

int STreeViewItemLocator::GetScrollLineSize() const
{
    return m_nLineHeight;
}

void STreeViewItemLocator::SetDefItemHeight(int nHeight)
{
    m_nLineHeight = nHeight;
}

int STreeViewItemLocator::GetTotalWidth() const
{
    return (int)m_adapter->GetItemDataByIndex(ITEM_ROOT, DATA_INDEX_BRANCH_WIDTH);
}

int STreeViewItemLocator::GetTotalHeight() const
{
    return (int)m_adapter->GetItemDataByIndex(ITEM_ROOT, DATA_INDEX_BRANCH_HEIGHT);
}

void STreeViewItemLocator::OnBranchExpandedChanged(HSTREEITEM hItem, BOOL bExpandedOld, BOOL bExpandedNew)
{
    if (bExpandedNew == bExpandedOld)
        return;
    int nOldBranchWidth = _GetBranchWidth(hItem);
    int nBranchHei = _GetBranchHeight(hItem);
    if (nBranchHei == 0 && bExpandedNew)
    {
        HSTREEITEM hChild = m_adapter->GetFirstChildItem(hItem);
        while (hChild != ITEM_NULL)
        {
            _SetItemOffset(hChild, nBranchHei);
            _SetItemHeight(hChild, m_nLineHeight);
            nBranchHei += m_nLineHeight;
            hChild = m_adapter->GetNextSiblingItem(hChild);
        }
        _SetBranchHeight(hItem, nBranchHei);
    }
    HSTREEITEM hParent = m_adapter->GetParentItem(hItem);
    while (hParent != ITEM_NULL)
    {
        _SetBranchHeight(hParent, _GetBranchHeight(hParent) + nBranchHei * (bExpandedNew ? 1 : -1));
        if (!IsItemExpanded(hParent))
            break;
        hParent = m_adapter->GetParentItem(hParent);
    }
    _UpdateSiblingsOffset(hItem);

    int nNewBranchWidth = _GetItemVisibleWidth(hItem);
    _UpdateBranchWidth(hItem, nOldBranchWidth, nNewBranchWidth);
}

void STreeViewItemLocator::OnBranchChanged(HSTREEITEM hItem)
{
    // Initialize list item height and other data
    int nVisibleHeightOld = _GetItemVisibleHeight(hItem);
    _InitBranch(hItem);
    int nVisibleHeightNew = _GetItemVisibleHeight(hItem);
    int nDiff = nVisibleHeightNew - nVisibleHeightOld;
    if (nDiff == 0 || hItem == ITEM_ROOT)
        return;

    HSTREEITEM hParent = m_adapter->GetParentItem(hItem);
    while (hParent != ITEM_NULL)
    {
        _SetBranchHeight(hParent, _GetBranchHeight(hParent) + nDiff);
        hParent = m_adapter->GetParentItem(hParent);
    }
    _UpdateSiblingsOffset(hItem);
}

///////////////////////////////////////////////////////////////////////
STreeView::STreeView()
    : m_itemCapture(NULL)
    , m_pHoverItem(NULL)
    , m_hSelected(ITEM_NULL)
    , m_hSelAnchor(ITEM_NULL)
    , m_pVisibleMap(new VISIBLEITEMSMAP)
    , m_bWantTab(FALSE)
    , m_bMultiSel(FALSE)
    , m_hBandOldSel(ITEM_NULL)
    , m_pLineSkin(GETBUILTINSKIN(SKIN_SYS_TREE_LINES))
    , m_bHasLines(FALSE)
    , SHostProxy(this)
{
    m_bFocusable = TRUE;

    m_evtSet.addEvent(EVENTID(EventTVSelChanging));
    m_evtSet.addEvent(EVENTID(EventTVSelChanged));
    m_evtSet.addEvent(EVENTID(EventTreeItemSelChanged));
    m_observer.Attach(new STreeViewDataSetObserver(this));
    m_tvItemLocator.Attach(new STreeViewItemLocator);
}

STreeView::~STreeView()
{
    StopFlingAnimation();
    delete m_pVisibleMap;
}

BOOL STreeView::SetAdapter(ITvAdapter *adapter)
{
    if (m_adapter)
    {
        m_adapter->unregisterDataSetObserver(m_observer);
    }
    if (m_adapter == adapter)
    {
        SSLOGW() << "the new adapter is same to previous set adapter, same as onBranchChanged";
        if (m_adapter)
        {
            onBranchChanged(ITEM_ROOT);
        }
        return TRUE;
    }
    ClearItemPanels();
    m_adapter = adapter;
    if (m_adapter)
    {
        m_adapter->registerDataSetObserver(m_observer);
    }

    if (m_tvItemLocator)
        m_tvItemLocator->SetAdapter(adapter);

    if (adapter)
    {
        SXmlNode xmlNode = m_xmlTemplate.root().first_child();
        m_adapter->InitByTemplate(&xmlNode);
        for (int i = 0; i < m_adapter->getViewTypeCount(); i++)
        {
            m_itemRecycle.Add(new SList<SItemPanel *>());
        }
        onBranchChanged(ITEM_ROOT);
    }

    return TRUE;
}

BOOL STreeView::CreateChildren(SXmlNode xmlNode)
{
    SXmlNode xmlTemplate = xmlNode.child(STreeView_style::kStyle_template);
    if (xmlTemplate)
    {
        m_xmlTemplate.Reset();
        m_xmlTemplate.root().append_copy(xmlTemplate);
        if (m_tvItemLocator)
        {
            int defItemHeight = m_xmlTemplate.root().attribute(STreeView_style::kStyle_defItemHeight).as_int(30);
            if (defItemHeight > 0)
            {
                m_tvItemLocator->SetDefItemHeight(defItemHeight);
            }
        }
    }
    return TRUE;
}

void STreeView::OnPaint(IRenderTarget *pRT)
{
    SPainter painter;
    BeforePaint(pRT, painter);

    CRect rcClient;
    GetClientRect(&rcClient);
    pRT->PushClipRect(&rcClient, RGN_AND);

    CRect rcClip;
    pRT->GetClipBox(&rcClip);
    SAutoRefPtr<IRegionS> rgnClip;
    pRT->GetClipRegion(&rgnClip);

    CPoint pt(0, -1);
    float fMat[9];
    pRT->GetTransform(fMat);
    SMatrix mtx(fMat);

    int nIndent = m_tvItemLocator->GetIndent();
    for (SPOSITION pos = m_visible_items.GetHeadPosition(); pos;)
    {
        ItemInfo ii = m_visible_items.GetNext(pos);
        HSTREEITEM hItem = (HSTREEITEM)ii.pItem->GetItemIndex();
        if (pt.y == -1)
        {
            pt.y = m_tvItemLocator->Item2Position(hItem) - m_siVer.nPos;
        }
        pt.x = -m_siHoz.nPos;

        CSize szItem(m_tvItemLocator->GetItemWidth(hItem), m_tvItemLocator->GetItemHeight(hItem));
        if (m_bHasLines)
        {
            CRect rcItem(pt, CSize(rcClient.Width(), szItem.cy));
            rcItem.OffsetRect(rcClient.TopLeft());
            DrawLines(pRT, rcItem, hItem);
        }

        pt.x = m_tvItemLocator->GetItemIndent(hItem) - m_siHoz.nPos;

        CRect rcItem(pt, szItem);
        rcItem.OffsetRect(rcClient.TopLeft());
        if (m_bHasLines)
            rcItem.OffsetRect(nIndent, 0);
        if (SItemPanel::IsItemInClip(mtx, rcClip, rgnClip, rcItem))
        { // draw the item
            ii.pItem->Draw(pRT, rcItem);
        }
        pt.y += m_tvItemLocator->GetItemHeight(hItem);
    }

    pRT->PopClip();

    DrawRubberBandSel(pRT);
    AfterPaint(pRT, painter);
}

void STreeView::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);
    if (!m_adapter)
        return;
    UpdateScrollBar();
    UpdateVisibleItems();
}

void STreeView::OnDestroy()
{
    if (m_adapter)
    {
        m_adapter->unregisterDataSetObserver(m_observer);
    }
    ClearItemPanels();
    __baseCls::OnDestroy();
}

void STreeView::ClearItemPanels()
{
    if (m_itemCapture) {
        m_itemCapture->ReleaseCapture();
        m_itemCapture = NULL;
    }
    m_pHoverItem = NULL;
    // free all item panels in the recycle bin
    for (size_t i = 0; i < m_itemRecycle.GetCount(); i++)
    {
        SList<SItemPanel *> *lstItemPanels = m_itemRecycle.GetAt(i);
        SPOSITION pos = lstItemPanels->GetHeadPosition();
        while (pos)
        {
            SItemPanel *pItemPanel = lstItemPanels->GetNext(pos);
            pItemPanel->Release();
        }
        delete lstItemPanels;
    }
    m_itemRecycle.RemoveAll();

    // free all visible item panels
    SPOSITION pos = m_visible_items.GetHeadPosition();
    while (pos)
    {
        ItemInfo ii = m_visible_items.GetNext(pos);
        ii.pItem->Release();
    }
    m_visible_items.RemoveAll();
    m_pVisibleMap->RemoveAll();

    // reset per-item hover, capture and selection state
    m_hSelected = 0;
    m_hSelAnchor = 0;
    // The selection map holds handles owned by the old adapter; reuse them
    // across an adapter swap and lookups would hit dangling handles (or
    // false-match a recycled address), and a band cannot span an adapter swap.
    m_mapSelItems.RemoveAll();
    m_arrBandSnapshot.RemoveAll();
}

void STreeView::EnsureVisible(HSTREEITEM hItem)
{
    // Ensure hItem is correctly expanded
    HSTREEITEM hParent = m_adapter->GetParentItem(hItem);
    while (hParent != ITEM_ROOT)
    {
        m_adapter->ExpandItem(hParent, TVC_EXPAND);
        hParent = m_adapter->GetParentItem(hParent);
    }
    // Scroll view
    int nPos = m_tvItemLocator->Item2Position(hItem);
    int nHeight = m_tvItemLocator->GetItemHeight(hItem);
    if (nPos + nHeight <= m_siVer.nPos)
    {
        OnScroll(TRUE, SB_THUMBPOSITION, nPos);
    }
    else if (nPos > m_siVer.nPos + (int)m_siVer.nPage)
    {
        OnScroll(TRUE, SB_THUMBPOSITION, nPos + nHeight - m_siVer.nPage);
    }
    int nIndent = m_tvItemLocator->GetItemIndent(hItem);
    if (m_bHasLines)
        nIndent += m_tvItemLocator->GetIndent();
    int nWidth = m_tvItemLocator->GetItemWidth(hItem);

    if (nIndent + nWidth <= m_siHoz.nPos)
    {
        OnScroll(FALSE, SB_THUMBPOSITION, nIndent);
    }
    else if (nIndent < m_siHoz.nPos + (int)m_siHoz.nPage)
    {
        OnScroll(FALSE, SB_THUMBPOSITION, nIndent + nWidth - m_siHoz.nPage);
    }
}

void STreeView::SetSel(HSTREEITEM hItem, BOOL bNotify /**< =FALSE */)
{
    if (!m_adapter)
        return;

    if (!m_bMultiSel)
    {
        // Single selection mode: the anchor m_hSelected is the selection
        // record (IsItemSelected reads it); the map is the multi-selection
        // record only.
        if (IsItemSelected(hItem))
            return;
        // Clearing an already-empty selection changes nothing: no visuals
        // and no SelChanging/SelChanged pair.
        if (hItem == ITEM_NULL && m_hSelected == ITEM_NULL)
            return;
        HSTREEITEM hOldSel = m_hSelected;
        // The anchor-style TV SelChanging/SelChanged are a single-selection
        // protocol; multi-selection feedback is per-item only.
        if (bNotify)
        {
            EventTVSelChanging evt(this);
            evt.bCancel = FALSE;
            evt.hOldSel = hOldSel;
            evt.hNewSel = hItem;
            FireEvent(&evt);
            if (evt.bCancel)
            { // Cancel SetSel and restore selection state
                return;
            }
        }

        // Clear old selection
        SItemPanel *pItem = GetItemPanel(m_hSelected);
        if (pItem)
        {
            pItem->GetFocusManager()->ClearFocus();
            pItem->SetSelected(FALSE);
            RedrawItem(pItem);
        }

        // Set new selection. No per-item event here - the anchor-style
        // TVSelChanging/TVSelChanged pair is the single-selection feedback.
        m_hSelected = hItem;
        m_hSelAnchor = hItem;
        if (hItem != ITEM_NULL)
        {
            SItemPanel *pNewItem = GetItemPanel(hItem);
            if (pNewItem)
            {
                pNewItem->SetSelected(TRUE);
                RedrawItem(pNewItem);
            }
        }
        if (bNotify)
        {
            EventTVSelChanged evt(this);
            evt.hOldSel = hOldSel;
            evt.hNewSel = hItem;
            FireEvent(&evt);
        }
    }
    else
    {
        // Multi selection mode
        if (hItem == ITEM_NULL)
        {
            // Clear all selections
            ClearSelItems();
            m_hSelected = ITEM_NULL;
            m_hSelAnchor = ITEM_NULL;
        }
        else if (IsItemSelected(hItem) && m_mapSelItems.GetCount() == 1)
        {
            // The set is already exactly {hItem}: only the cursor follows,
            // no per-item events, no repaint (mirrors SViewBase::SetSel).
            m_hSelected = hItem;
            m_hSelAnchor = hItem;
        }
        else
        {
            // Clear old selection
            ClearSelItems();

            // Select new item
            m_hSelected = hItem;
            m_hSelAnchor = hItem;
            AddSelItem(hItem);
        }
    }
}

void STreeView::OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags)
{
    if (!m_adapter || nChar == VK_ESCAPE)
    {
        SetMsgHandled(FALSE);
        return;
    }

    if (m_hSelected != ITEM_NULL && m_bWantTab)
    {
        SItemPanel *pItem = GetItemPanel(m_hSelected);
        if (pItem)
        {
            pItem->DoFrameEvent(WM_KEYDOWN, nChar, MAKELONG(nFlags, nRepCnt));
            if (pItem->IsMsgHandled())
                return;
        }
    }

    SWindow *pOwner = GetOwner();
    if (pOwner && (nChar == VK_ESCAPE || nChar == VK_RETURN))
    {
        pOwner->SSendMessage(WM_KEYDOWN, nChar, MAKELONG(nFlags, nRepCnt));
        return;
    }

    BOOL bCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    BOOL bShiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    BOOL bMultiSelMode = GetMultiSel();

    // Handle space key to toggle selection
    if (nChar == VK_SPACE && m_hSelected != ITEM_NULL)
    {
        if (bMultiSelMode)
        {
            if (IsItemSelected(m_hSelected))
            {
                RemoveSelItem(m_hSelected);
            }
            else
            {
                AddSelItem(m_hSelected);
            }
            return;
        }
        else
        {
            SetMsgHandled(FALSE);
            return;
        }
    }

    // Handle Ctrl+A to select all
    if (bCtrlPressed && nChar == 'A')
    {
        if (bMultiSelMode)
        {
            ClearSelItems();
            // This would require additional logic to traverse all tree items
            // For simplicity, we'll just select the visible items for now
            HSTREEITEM hItem = m_adapter->GetFirstVisibleItem();
            while (hItem != ITEM_NULL)
            {
                AddSelItem(hItem);
                hItem = m_adapter->GetNextVisibleItem(hItem);
            }
            return;
        }
        else
        {
            SetMsgHandled(FALSE);
            return;
        }
    }

    HSTREEITEM nNewSelItem = ITEM_NULL;
    switch (nChar)
    {
    case VK_DOWN:
        nNewSelItem = (m_hSelected == ITEM_NULL) ? m_adapter->GetFirstVisibleItem() : m_adapter->GetNextVisibleItem(m_hSelected);
        break;
    case VK_UP:
        nNewSelItem = (m_hSelected == ITEM_NULL) ? m_adapter->GetFirstVisibleItem() : m_adapter->GetPrevVisibleItem(m_hSelected);
        break;
    case VK_PRIOR:
    {
        // Page keys move the selection/cursor too (mirrors STreeCtrl):
        // scroll first, then land on the item at the new top visible row.
        OnScroll(TRUE, SB_PAGEUP, 0);
        nNewSelItem = m_tvItemLocator->Position2Item(m_siVer.nPos);
        break;
    }
    case VK_NEXT:
    {
        OnScroll(TRUE, SB_PAGEDOWN, 0);
        CRect rcClient;
        GetClientRect(&rcClient);
        nNewSelItem = m_tvItemLocator->Position2Item(m_siVer.nPos + rcClient.Height() - 1);
        if (nNewSelItem == ITEM_NULL)
        {
            // position past the content end: fall back to the last visible item
            HSTREEITEM hItem = m_adapter->GetFirstVisibleItem();
            while (hItem != ITEM_NULL)
            {
                nNewSelItem = hItem;
                hItem = m_adapter->GetNextVisibleItem(hItem);
            }
        }
        break;
    }
    case VK_HOME:
        OnScroll(TRUE, SB_TOP, 0);
        nNewSelItem = m_adapter->GetFirstVisibleItem();
        break;
    case VK_END:
    {
        OnScroll(TRUE, SB_BOTTOM, 0);
        HSTREEITEM hItem = m_adapter->GetFirstVisibleItem();
        while (hItem != ITEM_NULL)
        {
            nNewSelItem = hItem;
            hItem = m_adapter->GetNextVisibleItem(hItem);
        }
        break;
    }
    case VK_LEFT:
        if (m_hSelected != ITEM_NULL)
        {
            if (m_adapter->HasChildren(m_hSelected) && m_adapter->IsItemExpanded(m_hSelected))
                m_adapter->ExpandItem(m_hSelected, TVC_COLLAPSE); // collapse the selected item
            else
                nNewSelItem = m_adapter->GetPrevVisibleItem(m_hSelected);
        }
        break;
    case VK_RIGHT:
        if (m_hSelected != ITEM_NULL)
        {
            if (m_adapter->HasChildren(m_hSelected) && !m_adapter->IsItemExpanded(m_hSelected))
                m_adapter->ExpandItem(m_hSelected, TVC_EXPAND); // collapse the selected item
            else
                nNewSelItem = m_adapter->GetNextVisibleItem(m_hSelected);
        }
        break;
    }

    if (nNewSelItem != ITEM_NULL)
    {
        EnsureVisible(nNewSelItem);

        if (bMultiSelMode)
        {
            if (bCtrlPressed)
            {
                // Ctrl + arrow: move focus without changing selection
                m_hSelected = nNewSelItem;
                m_hSelAnchor = nNewSelItem;
                // Update visible items to reflect the new focus
                UpdateVisibleItems();
            }
            else if (bShiftPressed)
            {
                // Shift + arrow: move the anchored span to the new cursor
                // (the anchor stays fixed, so the span can shrink as well
                // as grow - Explorer semantics).
                SetSelRange(m_hSelAnchor, nNewSelItem);
                m_hSelected = nNewSelItem;
            }
            else
            {
                // Normal arrow: clear selection and select new item
                SetSel(nNewSelItem, TRUE);
            }
        }
        else
        {
            // Single selection mode
            SetSel(nNewSelItem, TRUE);
        }
    }
    else
    {
        SetMsgHandled(FALSE);
    }
}

LRESULT STreeView::OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet = 0;
    SItemPanel *pItem = GetItemPanel(m_hSelected);
    if (pItem)
    {
        lRet = pItem->DoFrameEvent(uMsg, wParam, lParam);
        SetMsgHandled(pItem->IsMsgHandled());
    }
    else
    {
        SetMsgHandled(FALSE);
    }
    return lRet;
}

void STreeView::UpdateScrollBar()
{
    CSize szView;
    szView.cx = m_tvItemLocator->GetTotalWidth();
    if (m_bHasLines)
        szView.cx += m_tvItemLocator->GetIndent();
    szView.cy = m_tvItemLocator->GetTotalHeight();

    CRect rcClient;
    SWindow::GetClientRect(&rcClient); // Do not compute scrollbar size
    CSize size = rcClient.Size();
    // Close scroll bar
    m_wBarVisible = SSB_NULL;

    if (size.cy < szView.cy || (size.cy < szView.cy + GetSbWidth() && size.cx < szView.cx))
    {
        // Need vertical scroll bar
        m_wBarVisible |= SSB_VERT;
        m_siVer.nMin = 0;
        m_siVer.nMax = szView.cy - 1;
        m_siVer.nPage = rcClient.Height();

        if (size.cx - GetSbWidth() < szView.cx)
        {
            // If fit-to-width, it means item width needs updating; otherwise a horizontal scrollbar is needed
            if (m_adapter->isViewWidthMatchParent())
            {
                UpdateVisibleItems();
            }
            else
            {
                // Need horizontal scrollbar
                m_wBarVisible |= SSB_HORZ;
                m_siVer.nPage = size.cy - GetSbWidth() > 0 ? size.cy - GetSbWidth() : 0; // Note to also adjust the vertical scrollbar page info

                m_siHoz.nMin = 0;
                m_siHoz.nMax = szView.cx - 1;
                m_siHoz.nPage = (size.cx - GetSbWidth()) > 0 ? (size.cx - GetSbWidth()) : 0;
            }
        }
        else
        {
            // No horizontal scroll bar needed
            m_siHoz.nPage = size.cx;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = m_siHoz.nPage - 1;
            m_siHoz.nPos = 0;
        }
    }
    else
    {
        // No vertical scroll bar needed
        m_siVer.nPage = size.cy;
        m_siVer.nMin = 0;
        m_siVer.nMax = size.cy - 1;
        m_siVer.nPos = 0;

        if (size.cx < szView.cx)
        {
            // Need horizontal scrollbar
            m_wBarVisible |= SSB_HORZ;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = szView.cx - 1;
            m_siHoz.nPage = size.cx;
        }
        else
        {
            // No horizontal scroll bar needed
            m_siHoz.nPage = size.cx;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = m_siHoz.nPage - 1;
            m_siHoz.nPos = 0;
        }
    }

    // Adjust origin position as needed
    if (HasScrollBar(FALSE) && m_siHoz.nPos + (int)m_siHoz.nPage > szView.cx)
    {
        m_siHoz.nPos = szView.cx - m_siHoz.nPage;
    }

    if (HasScrollBar(TRUE) && m_siVer.nPos + (int)m_siVer.nPage > szView.cy)
    {
        m_siVer.nPos = szView.cy - m_siVer.nPage;
    }

    SetScrollPos(TRUE, m_siVer.nPos, TRUE);
    SetScrollPos(FALSE, m_siHoz.nPos, TRUE);

    // Recompute client and non-client areas
    SSendMessage(WM_NCCALCSIZE);

    InvalidateRect(NULL);
}

void STreeView::UpdateVisibleItems()
{
    if (!m_adapter || !GetContainer())
        return;
    SAutoEnableHostPrivUiDef enableUiDef(this);
    HSTREEITEM hItem = m_tvItemLocator->Position2Item(m_siVer.nPos);
    if (hItem == ITEM_NULL)
    {
        // If none is displayable, remove all items
        SPOSITION pos = m_visible_items.GetHeadPosition();
        while (pos)
        {
            ItemInfo ii = m_visible_items.GetNext(pos);

            if (ii.pItem == m_pHoverItem)
            {
                m_pHoverItem->DoFrameEvent(WM_MOUSELEAVE, 0, 0);
                m_pHoverItem = NULL;
                // SSLOGI() << "m_pHoverItem = " << m_pHoverItem;
            }

            ii.pItem->GetEventSet()->setMutedState(true);
            if (ii.pItem->IsSelected())
            {
                ii.pItem->SetSelected(FALSE, FALSE);
                ii.pItem->GetFocusManager()->ClearFocus();
            }
            ii.pItem->SetVisible(FALSE); // Prevent SItemPanel::OnTimeFrame() from executing
            ii.pItem->GetEventSet()->setMutedState(false);

            m_itemRecycle[ii.nType]->AddTail(ii.pItem);
        }
        m_visible_items.RemoveAll();
        m_pVisibleMap->RemoveAll();
        return;
    }

    CSize szOldView;
    szOldView.cx = m_tvItemLocator->GetTotalWidth();
    if (m_bHasLines)
        szOldView.cx += m_tvItemLocator->GetIndent();
    szOldView.cy = m_tvItemLocator->GetTotalHeight();

    VISIBLEITEMSMAP *pMapOld = m_pVisibleMap;
    m_pVisibleMap = new VISIBLEITEMSMAP;

    CRect rcClient = GetClientRect();
    int nOffset = m_tvItemLocator->Item2Position(hItem) - m_siVer.nPos;

    m_visible_items.RemoveAll();
    while (hItem != ITEM_NULL)
    {
        VISIBLEITEMSMAP::CPair *pFind = pMapOld->Lookup(hItem);
        ItemInfo ii;
        ii.nType = m_adapter->getViewType(hItem);
        BOOL bNewItem = FALSE;
        if (pFind && pFind->m_value.nType == ii.nType)
        { // re use the previous item;
            ii = pFind->m_value;
            pMapOld->RemoveKey(hItem);
        }
        else
        {
            SList<SItemPanel *> *lstRecycle = m_itemRecycle.GetAt(ii.nType);
            if (lstRecycle->IsEmpty())
            { // Create a new list item
                bNewItem = TRUE;
                ii.pItem = SItemPanel::Create(this, SXmlNode(), this);
                ii.pItem->GetEventSet()->subscribeEvent(EventItemPanelClick::EventID, Subscriber(&STreeView::OnItemClick, this));
                ii.pItem->GetEventSet()->subscribeEvent(EventItemPanelClickUp::EventID, Subscriber(&STreeView::OnItemClickUp, this));
            }
            else
            {
                ii.pItem = lstRecycle->RemoveHead();
            }
            ii.pItem->SetItemIndex(hItem);
        }
        m_pVisibleMap->SetAt(hItem, ii);
        ii.pItem->SetVisible(TRUE);

        ii.pItem->SetSelected(IsItemSelected(hItem), FALSE);

        if (m_pHoverItem && hItem == (HSTREEITEM)m_pHoverItem->GetItemIndex())
            ii.pItem->ModifyItemState(WndState_Hover, 0);
        else
            ii.pItem->ModifyItemState(0, WndState_Hover);

        SXmlNode xmlNode = m_xmlTemplate.root().first_child();
        ii.pItem->LockUpdate();
        m_adapter->getView(hItem, ii.pItem, &xmlNode);
        ii.pItem->UnlockUpdate();
        if (bNewItem)
        {
            ii.pItem->SDispatchMessage(UM_SETSCALE, GetScale(), 0);
            ii.pItem->SDispatchMessage(UM_SETLANGUAGE, 0, 0);
            ii.pItem->DoColorize(GetColorizeColor());
        }

        CSize szItem;
        CRect rcItem = GetClientRect();
        m_adapter->getViewDesiredSize(&szItem, hItem, ii.pItem, -1, -1);
        // Do not apply width
        if (m_adapter->isViewWidthMatchParent())
            szItem.cx = rcItem.Width();
        ii.pItem->Move(CRect(0, 0, szItem.cx, szItem.cy));
        m_tvItemLocator->SetItemWidth(hItem, szItem.cx);
        m_tvItemLocator->SetItemHeight(hItem, szItem.cy);

        m_visible_items.AddTail(ii);
        nOffset += szItem.cy;
        if (nOffset >= rcClient.Height())
            break;
        hItem = m_adapter->GetNextVisibleItem(hItem);
    }

    SPOSITION pos = pMapOld->GetStartPosition();
    while (pos)
    {
        ItemInfo ii = pMapOld->GetNextValue(pos);

        if (ii.pItem == m_pHoverItem)
        {
            m_pHoverItem->DoFrameEvent(WM_MOUSELEAVE, 0, 0);
            m_pHoverItem = NULL;
            // SSLOGI() << "m_pHoverItem = " << m_pHoverItem;
        }

        ii.pItem->GetEventSet()->setMutedState(true);
        if ((HSTREEITEM)ii.pItem->GetItemIndex() == m_hSelected)
        {
            // Recycle the panel's focus / check visuals only. The selection
            // record (the anchor in single mode, the map in multi mode) is
            // kept: the rebuilt panel re-applies it via IsItemSelected.
            ii.pItem->ModifyItemState(0, WndState_Check);
            ii.pItem->GetFocusManager()->ClearFocus();
        }
        ii.pItem->SetVisible(FALSE); // Prevent SItemPanel::OnTimeFrame() from executing
        ii.pItem->GetEventSet()->setMutedState(false);

        m_itemRecycle[ii.nType]->AddTail(ii.pItem);
    }
    delete pMapOld;

    CSize szNewView;
    szNewView.cx = m_tvItemLocator->GetTotalWidth();
    if (m_bHasLines)
        szNewView.cx += m_tvItemLocator->GetIndent();
    szNewView.cy = m_tvItemLocator->GetTotalHeight();
    if (szOldView != szNewView)
    { // update scroll range
        UpdateScrollBar();
        UpdateVisibleItems(); // Recompute the displayed list items based on the new scrollbar state
    }
    else
    {
        InvalidateRect(NULL);
    }
}

void STreeView::OnItemSetCapture(SOsrPanel *pItem, BOOL bCapture)
{
    if (bCapture)
    {
        GetContainer()->OnSetSwndCapture(m_swnd);
        m_itemCapture = pItem;
    }
    else
    {
        GetContainer()->OnReleaseSwndCapture();
        m_itemCapture = NULL;
    }
}

BOOL STreeView::OnItemGetRect(const SOsrPanel *pItem, CRect &rcItem) const
{
    HSTREEITEM hItem = (HSTREEITEM)pItem->GetItemIndex();
    if (m_pVisibleMap->Lookup(hItem) == NULL)
        return FALSE;

    int nOffset = m_tvItemLocator->Item2Position(hItem) - m_siVer.nPos;
    rcItem = GetClientRect();
    rcItem.top += nOffset;
    rcItem.bottom = rcItem.top + m_tvItemLocator->GetItemHeight(hItem);
    rcItem.left += m_tvItemLocator->GetItemIndent(hItem) - m_siHoz.nPos;
    rcItem.right = rcItem.left + m_tvItemLocator->GetItemWidth(hItem);
    if (m_bHasLines)
    {
        rcItem.OffsetRect(m_tvItemLocator->GetIndent(), 0);
    }
    return TRUE;
}

BOOL STreeView::IsItemRedrawDelay() const
{
    return TRUE;
}

BOOL STreeView::IsTimelineEnabled() const
{
    return !m_sbHorz.IsThumbTracking() && !m_sbVert.IsThumbTracking();
}

void STreeView::onBranchChanged(HSTREEITEM hBranch)
{
    if (m_adapter == NULL)
    {
        return;
    }
    if (m_tvItemLocator)
        m_tvItemLocator->OnBranchChanged(hBranch);
    UpdateScrollBar();
    UpdateVisibleItems();
}

void STreeView::onBranchInvalidated(HSTREEITEM hBranch, BOOL bInvalidParents, BOOL bInvalidChildren)
{
    if (m_adapter == NULL)
    {
        return;
    }
    SAutoEnableHostPrivUiDef enableUiDef(this);
    if (bInvalidParents)
    {
        HSTREEITEM hParent = m_adapter->GetParentItem(hBranch);
        while (hParent)
        {
            SItemPanel *pItem = GetItemPanel(hParent);
            if (pItem)
            {
                SXmlNode xmlNode = m_xmlTemplate.root().first_child();
                m_adapter->getView(hParent, pItem, &xmlNode);
                pItem->InvalidateRect(NULL);
            }
            hParent = m_adapter->GetParentItem(hParent);
        }
    }
    if (!bInvalidChildren)
    {
        SItemPanel *pItem = GetItemPanel(hBranch);
        if (pItem)
        {
            SXmlNode xmlNode = m_xmlTemplate.root().first_child();
            m_adapter->getView(hBranch, pItem, &xmlNode);
            pItem->InvalidateRect(NULL);
        }
    }
    else
    {
        SPOSITION pos = m_visible_items.GetHeadPosition();
        while (pos)
        {
            const ItemInfo &ii = m_visible_items.GetNext(pos);
            bool bInvalid = false;
            HSTREEITEM hItem = (HSTREEITEM)ii.pItem->GetItemIndex();
            HSTREEITEM hParent = hItem;
            while (hParent)
            {
                if (hParent == hBranch)
                {
                    bInvalid = true;
                    break;
                }
                hParent = m_adapter->GetParentItem(hParent);
            }
            if (bInvalid)
            {
                SXmlNode xmlNode = m_xmlTemplate.root().first_child();
                m_adapter->getView(hItem, ii.pItem, &xmlNode);
                ii.pItem->InvalidateRect(NULL);
            }
        }
    }
}

void STreeView::onBranchExpandedChanged(HSTREEITEM hBranch, BOOL bExpandedOld, BOOL bExpandedNew)
{
    if (m_adapter == NULL)
    {
        return;
    }
    if (m_tvItemLocator)
        m_tvItemLocator->OnBranchExpandedChanged(hBranch, bExpandedOld, bExpandedNew);
    UpdateScrollBar();
    UpdateVisibleItems();
}

void STreeView::onItemBeforeRemove(HSTREEITEM hItem)
{
    if (m_adapter == NULL)
    {
        return;
    }
    if (m_hSelected && (m_hSelected == hItem || m_adapter->IsDecendentItem(hItem, m_hSelected)))
    {
        m_hSelected = 0;
        m_hSelAnchor = 0;
    }

    // Drop the item (and its descendants) from the multi-selection map -
    // silently, the rows are going away (same convention as STreeCtrl).
    {
        SArray<HSTREEITEM> arrStale;
        for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos;)
        {
            HSTREEITEM hSel;
            BOOL bVal;
            m_mapSelItems.GetNextAssoc(pos, hSel, bVal);
            if (hSel == hItem || m_adapter->IsDecendentItem(hItem, hSel))
                arrStale.Add(hSel);
        }
        for (int i = 0; i < (int)arrStale.GetCount(); i++)
        {
            m_mapSelItems.RemoveKey(arrStale[i]);
        }
    }

    if (m_pHoverItem)
    {
        HSTREEITEM hHover = m_pHoverItem->GetItemIndex();
        if (hHover == hItem || m_adapter->IsDecendentItem(hItem, hHover))
        {
            m_pHoverItem->DoFrameEvent(WM_MOUSELEAVE, 0, 0);
            m_pHoverItem = NULL;
            // SSLOGI() << "m_pHoverItem = " << m_pHoverItem;
        }
    }
    if (m_itemCapture)
    {
        HSTREEITEM hCapture = m_itemCapture->GetItemIndex();
        if (hCapture == hItem || m_adapter->IsDecendentItem(hItem, hCapture))
        {
            m_itemCapture = NULL;
        }
    }
}

LRESULT STreeView::OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SetMsgHandled(FALSE);
    if (m_adapter == NULL)
    {
        return 0;
    }

    LRESULT lRet = 0;
    CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

    // === 1. Try drag handling first ===
    if (HandleMouseDrag(uMsg, wParam, lParam, lRet))
    {
        SetMsgHandled(TRUE);
        return 0;
    }

    if (m_itemCapture)
    {
        CRect rcItem = m_itemCapture->GetItemRect();
        pt.Offset(-rcItem.TopLeft());
        lRet = m_itemCapture->DoFrameEvent(uMsg, wParam, MAKELPARAM(pt.x, pt.y));
    }
    else
    {
        if (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN)
        { // Hand over to panel for handling
            CPoint pt2(pt);
            SItemPanel *pPanel = HitTest(pt2);
            if (!pPanel && m_hSelected) // hit in none-item area,so make item to killfocus
            {
                SItemPanel *pSelItem = GetItemPanel(m_hSelected);
                if (pSelItem)
                    pSelItem->DoFrameEvent(WM_KILLFOCUS, 0, 0);
            }
            if (!pPanel && uMsg == WM_LBUTTONDOWN && (wParam & (MK_CONTROL | MK_SHIFT)) == 0)
            {
                // Plain click on blank area clears the selection. Ctrl/
                // Shift+click and right/middle clicks leave it untouched.
                if (m_bMultiSel)
                {
                    ClearSelItems();
                    m_hSelected = ITEM_NULL; // the cursor goes with the set
                    m_hSelAnchor = ITEM_NULL;
                }
                else if (m_hSelected)
                {
                    // Single-selection mode: the anchor is the record.
                    SItemPanel *pSelItem = GetItemPanel(m_hSelected);
                    if (pSelItem)
                    {
                        pSelItem->ModifyItemState(0, WndState_Check);
                        RedrawItem(pSelItem);
                    }
                    m_hSelected = NULL;
                    m_hSelAnchor = NULL;
                }
            }

            __baseCls::ProcessSwndMessage(uMsg, wParam, lParam, lRet);
        }

        SOsrPanel *pHover = HitTest(pt);
        if (pHover != m_pHoverItem)
        {
            SOsrPanel *oldHover = m_pHoverItem;
            m_pHoverItem = pHover;
            // SSLOGI() << "m_pHoverItem = " << m_pHoverItem;

            if (oldHover)
            {
                oldHover->DoFrameEvent(WM_MOUSELEAVE, 0, 0);
                oldHover->InvalidateRect(NULL);
            }
            if (m_pHoverItem)
            {
                m_pHoverItem->DoFrameEvent(WM_MOUSEHOVER, 0, 0);
                m_pHoverItem->InvalidateRect(NULL);
            }
        }

        if (m_pHoverItem)
        {
            m_pHoverItem->DoFrameEvent(uMsg, wParam, MAKELPARAM(pt.x, pt.y));
        }
    }

    if (uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONUP || uMsg == WM_MBUTTONUP)
    { // Hand over to panel for handling
        __baseCls::ProcessSwndMessage(uMsg, wParam, lParam, lRet);
    }
    SetMsgHandled(TRUE);
    return 0;
}

void STreeView::RedrawItem(SItemPanel *pItem)
{
    pItem->InvalidateRect(NULL);
}

void STreeView::OnMouseLeave()
{
    __baseCls::OnMouseLeave();

    if (m_pHoverItem)
    {
        m_pHoverItem->DoFrameEvent(WM_MOUSELEAVE, 0, 0);
        m_pHoverItem = NULL;
        // SSLOGI() << "m_pHoverItem = " << m_pHoverItem;
    }
}

BOOL STreeView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    SItemPanel *pSelItem = GetItemPanel(m_hSelected);
    if (pSelItem)
    {
        CRect rcItem = pSelItem->GetItemRect();
        CPoint pt2 = pt - rcItem.TopLeft();
        if (pSelItem->DoFrameEvent(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt2.x, pt2.y)))
            return TRUE;
    }
    return __baseCls::OnMouseWheel(nFlags, zDelta, pt);
}

void STreeView::OnKillFocus(SWND wndFocus)
{
    __baseCls::OnKillFocus(wndFocus);
    SItemPanel *itemPanel = GetItemPanel(m_hSelected);
    if (itemPanel)
        itemPanel->GetFocusManager()->StoreFocusedView();
}

void STreeView::OnSetFocus(SWND wndOld)
{
    __baseCls::OnSetFocus(wndOld);
    SItemPanel *itemPanel = GetItemPanel(m_hSelected);
    if (itemPanel)
    {
        itemPanel->GetFocusManager()->RestoreFocusedView();
    }
}
BOOL STreeView::OnScroll(BOOL bVertical, UINT uCode, int nPos)
{
    int nOldPos = m_siVer.nPos;
    __baseCls::OnScroll(bVertical, uCode, nPos);
    int nNewPos = m_siVer.nPos;
    if (nOldPos != nNewPos)
    {
        UpdateVisibleItems();

        // Accelerate UI refresh during scrolling
        if (uCode == SB_THUMBTRACK)
            ScrollUpdate();

        return TRUE;
    }
    return FALSE;
}

int STreeView::GetScrollLineSize(BOOL bVertical)
{
    return m_tvItemLocator->GetScrollLineSize();
}

SItemPanel *STreeView::GetItemPanel(HSTREEITEM hItem)
{
    VISIBLEITEMSMAP::CPair *pNode = m_pVisibleMap->Lookup(hItem);
    if (!pNode)
        return NULL;
    return pNode->m_value.pItem;
}

int STreeView::GetVisibleAccSelIndex() const
{
    int n = 0;
    for (SPOSITION it = m_visible_items.GetHeadPosition(); it;)
    {
        const ItemInfo &itInfo = m_visible_items.GetNext(it);
        if (IsItemSelected((HSTREEITEM)itInfo.pItem->GetItemIndex()))
            return n + 1;
        n++;
    }
    return 0;
}

IItemPanel *STreeView::HitTest(const POINT *pt) const
{
    SASSERT(pt);
    if (!pt)
        return NULL;
    CPoint pt2(*pt);
    return HitTest(pt2);
}

SItemPanel *STreeView::HitTest(CPoint &pt) const
{
    SPOSITION pos = m_visible_items.GetHeadPosition();
    while (pos)
    {
        ItemInfo ii = m_visible_items.GetNext(pos);
        CRect rcItem = ii.pItem->GetItemRect();
        if (rcItem.PtInRect(pt))
        {
            pt -= rcItem.TopLeft();
            return ii.pItem;
        }
    }
    return NULL;
}

BOOL STreeView::OnItemClickUp(IEvtArgs *pEvt)
{
    EventItemPanelClickUp *pClickEvt = sobj_cast<EventItemPanelClickUp>(pEvt);
    if (!pClickEvt)
        return TRUE;
    if (!m_bMultiSel)
        return TRUE;
    UINT uKeyFlags = GetKeyState(VK_CONTROL) & 0x8000 ? MK_CONTROL : 0;
    uKeyFlags |= GetKeyState(VK_SHIFT) & 0x8000 ? MK_SHIFT : 0;
    if (uKeyFlags == 0)
    {
        SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pEvt->Sender());
        HSTREEITEM hItem = (HSTREEITEM)pItemPanel->GetItemIndex();
        if (GetSelItemCount() > 1)
        {
            // remove other selected items
            SetSel(hItem, TRUE);
        }
    }
    return TRUE;
}

BOOL STreeView::OnItemClick(IEvtArgs *pEvt)
{
    EventItemPanelClick *pClickEvt = sobj_cast<EventItemPanelClick>(pEvt);
    if (!pClickEvt)
        return TRUE;

    SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pEvt->Sender());
    HSTREEITEM hItem = (HSTREEITEM)pItemPanel->GetItemIndex();

    if (!m_bMultiSel)
    {
        // Single selection mode
        if (!IsItemSelected(hItem))
        {
            SetSel(hItem, TRUE);
        }
    }
    else
    {
        // Multi selection mode
        UINT uKeyFlags = GetKeyState(VK_CONTROL) & 0x8000 ? MK_CONTROL : 0;
        uKeyFlags |= GetKeyState(VK_SHIFT) & 0x8000 ? MK_SHIFT : 0;

        if (uKeyFlags & MK_CONTROL)
        {
            // Ctrl + click: toggle selection
            if (IsItemSelected(hItem))
            {
                RemoveSelItem(hItem);
            }
            else
            {
                AddSelItem(hItem);
            }
            m_hSelected = hItem;
            m_hSelAnchor = hItem;
        }
        else if (uKeyFlags & MK_SHIFT)
        {
            // Shift + click: select the anchored span [anchor..clicked]
            // (Explorer semantics: the anchor stays fixed, the set is
            // replaced, so the span can shrink as well as grow).
            SetSelRange(m_hSelAnchor, hItem);
            m_hSelected = hItem;
        }
        else
        {
            // Single click: clear all and select clicked item
            if (!IsItemSelected(hItem))
            {
                SetSel(hItem, TRUE);
            }
        }
    }

    return TRUE;
}

BOOL STreeView::OnItemDblClick(IEvtArgs *pEvt)
{
    SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pEvt->Sender());
    HSTREEITEM hItem = (HSTREEITEM)pItemPanel->GetItemIndex();
    if (m_adapter->HasChildren(hItem))
    {
        m_adapter->ExpandItem(hItem, TVC_TOGGLE);
    }
    return true;
}

UINT STreeView::OnGetDlgCode() const
{
    if (m_bWantTab)
        return SC_WANTALLKEYS;
    else
        return SC_WANTARROWS | SC_WANTSYSKEY;
}

BOOL STreeView::OnSetCursor(const CPoint &pt)
{
    BOOL bRet = FALSE;
    if (m_itemCapture)
    {
        CRect rcItem = m_itemCapture->GetItemRect();
        bRet = m_itemCapture->DoFrameEvent(WM_SETCURSOR, 0, MAKELPARAM(pt.x - rcItem.left, pt.y - rcItem.top)) != 0;
    }
    else if (m_pHoverItem)
    {
        CRect rcItem = m_pHoverItem->GetItemRect();
        bRet = m_pHoverItem->DoFrameEvent(WM_SETCURSOR, 0, MAKELPARAM(pt.x - rcItem.left, pt.y - rcItem.top)) != 0;
    }
    if (!bRet)
    {
        bRet = __baseCls::OnSetCursor(pt);
    }
    return bRet;
}

BOOL STreeView::UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
{
    if (!m_pHoverItem)
        return __baseCls::UpdateToolTip(pt, tipInfo);
    return m_pHoverItem->UpdateToolTip(pt, tipInfo);
}

HRESULT STreeView::OnAttrIndent(const SStringW &strValue, BOOL bLoading)
{
    if (!bLoading)
        return E_FAIL;
    m_indent.parseString(strValue);
    m_tvItemLocator->SetIndent(m_indent.toPixelSize(GetScale()));
    return S_OK;
}

void STreeView::OnColorize(COLORREF cr)
{
    __baseCls::OnColorize(cr);
    DispatchMessage2Items(UM_SETCOLORIZE, cr, 0);
}

void STreeView::OnScaleChanged(int nScale)
{
    __baseCls::OnScaleChanged(nScale);
    GetScaleSkin(m_pLineSkin, nScale);
    m_tvItemLocator->SetIndent(m_indent.toPixelSize(nScale));
    DispatchMessage2Items(UM_SETSCALE, nScale, 0);
    UpdateVisibleItems();
}

HRESULT STreeView::OnLanguageChanged()
{
    HRESULT hret = __baseCls::OnLanguageChanged();
    DispatchMessage2Items(UM_SETLANGUAGE, 0, 0);
    return hret;
}

void STreeView::DispatchMessage2Items(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SPOSITION pos = m_visible_items.GetHeadPosition();
    while (pos)
    {
        ItemInfo ii = m_visible_items.GetNext(pos);
        ii.pItem->SDispatchMessage(uMsg, wParam, lParam);
    }
    for (UINT i = 0; i < m_itemRecycle.GetCount(); i++)
    {
        SList<SItemPanel *> *pLstTypeItems = m_itemRecycle[i];
        SPOSITION pos = pLstTypeItems->GetHeadPosition();
        while (pos)
        {
            SItemPanel *pItem = pLstTypeItems->GetNext(pos);
            pItem->SDispatchMessage(uMsg, wParam, lParam);
        }
    }
}

void STreeView::OnRebuildFont()
{
    __baseCls::OnRebuildFont();
    DispatchMessage2Items(UM_UPDATEFONT, 0, 0);
    UpdateVisibleItems(); // Prevent list item size changes from not being updated after font size changes. Other views do not need this process.
}

ITvAdapter *STreeView::GetAdapter() const
{
    return m_adapter;
}

void STreeView::SetItemLocator(ITreeViewItemLocator *pItemLocator)
{
    m_tvItemLocator = pItemLocator;
}

ITreeViewItemLocator *STreeView::GetItemLocator() const
{
    return m_tvItemLocator;
}

HSTREEITEM STreeView::GetSel() const
{
    return m_hSelected;
}

void STreeView::SetMultiSel(BOOL bMultiSel)
{
    if (bMultiSel && !m_bMultiSel)
    {
        // Turning multi-selection on: the anchor's item migrates into the
        // map - silently, its selection state does not change - and the
        // anchor stays on as the keyboard cursor.
        if (m_hSelected != ITEM_NULL)
            m_mapSelItems[m_hSelected] = TRUE;
        m_hSelAnchor = m_hSelected;
    }
    else if (!bMultiSel && m_bMultiSel)
    {
        // Turning multi-selection off. The map is the multi-selection
        // record only; the anchor is the single-selection record.
        int nCount = m_mapSelItems.GetCount();
        if (nCount == 1)
        {
            // The sole selected item keeps its state: move the record from
            // the map to the single-selection anchor. No event, no repaint.
            HSTREEITEM hItem = ITEM_NULL;
            BOOL bVal;
            for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos;)
            {
                m_mapSelItems.GetNextAssoc(pos, hItem, bVal);
            }
            m_mapSelItems.RemoveAll();
            m_hSelected = hItem;
            m_hSelAnchor = hItem;
        }
        else if (nCount > 1)
        {
            // Several items selected: clear everything (ClearSelItems fires
            // the per-item events and repaints) and reset the cursor.
            ClearSelItems();
            m_hSelected = ITEM_NULL;
            m_hSelAnchor = ITEM_NULL;
        }
        // nCount == 0: the anchor (cursor) is kept unchanged.
    }
    m_bMultiSel = bMultiSel;
}

BOOL STreeView::GetMultiSel() const
{
    return m_bMultiSel;
}

void STreeView::SetSelRange(HSTREEITEM hAnchor, HSTREEITEM hCursor)
{
    if (!m_bMultiSel || !m_adapter || hCursor == ITEM_NULL)
        return;
    if (hAnchor == ITEM_NULL)
        hAnchor = hCursor;

    // Determine the visible-order span [hLo..hHi]: walk forward from the
    // anchor looking for the cursor; if the cursor is not below it, walk
    // forward from the cursor instead (the anchor is below).
    BOOL bAnchorFirst = FALSE;
    HSTREEITEM h = hAnchor;
    while (h != ITEM_NULL)
    {
        if (h == hCursor)
        {
            bAnchorFirst = TRUE;
            break;
        }
        h = m_adapter->GetNextVisibleItem(h);
    }
    HSTREEITEM hLo = bAnchorFirst ? hAnchor : hCursor;
    HSTREEITEM hHi = bAnchorFirst ? hCursor : hAnchor;

    // Collect the span once and mark it, so membership tests below are cheap.
    SMap<HSTREEITEM, BOOL> inSpan;
    SArray<HSTREEITEM> arrSpan;
    h = hLo;
    while (h != ITEM_NULL)
    {
        inSpan[h] = TRUE;
        arrSpan.Add(h);
        if (h == hHi)
            break;
        h = m_adapter->GetNextVisibleItem(h);
    }

    // Deselect everything outside the span (snapshot first: RemoveSelItem
    // mutates the map while firing events).
    SArray<HSTREEITEM> arrStale;
    for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos;)
    {
        HSTREEITEM hSel;
        BOOL bVal;
        m_mapSelItems.GetNextAssoc(pos, hSel, bVal);
        if (!inSpan.Lookup(hSel))
            arrStale.Add(hSel);
    }
    for (int i = 0; i < (int)arrStale.GetCount(); i++)
    {
        RemoveSelItem(arrStale[i]);
    }
    // Select everything inside the span that is not selected yet.
    for (int i = 0; i < (int)arrSpan.GetCount(); i++)
    {
        if (!IsItemSelected(arrSpan[i]))
            AddSelItem(arrSpan[i]);
    }
}

void STreeView::AddSelItem(HSTREEITEM hItem)
{
    if (hItem == ITEM_NULL)
        return;

    // Already in the multi-selection map: the state does not change and no
    // per-item event fires.
    const ItemSelectionMap::CPair *pPair = m_mapSelItems.Lookup(hItem);
    if (pPair && pPair->m_value)
        return;

    m_mapSelItems[hItem] = TRUE;

    // Update the visible item's state
    SItemPanel *pItem = GetItemPanel(hItem);
    if (pItem)
    {
        pItem->SetSelected(TRUE);
        RedrawItem(pItem);
    }

    EventTreeItemSelChanged evt(this);
    evt.hItem = hItem;
    evt.bSelected = TRUE;
    FireEvent(evt);
}

void STreeView::RemoveSelItem(HSTREEITEM hItem)
{
    if (hItem == ITEM_NULL)
        return;

    // Not in the multi-selection map: nothing to remove, no event.
    if (!m_mapSelItems.RemoveKey(hItem))
        return;

    // Update the visible item's state
    SItemPanel *pItem = GetItemPanel(hItem);
    if (pItem)
    {
        pItem->ModifyItemState(0, WndState_Check);
        RedrawItem(pItem);
    }

    EventTreeItemSelChanged evt(this);
    evt.hItem = hItem;
    evt.bSelected = FALSE;
    FireEvent(evt);
}

void STreeView::ClearSelItems()
{
    // Route through RemoveSelItem so every deselected item fires its
    // per-item selection-state event. Keys are collected first because
    // RemoveSelItem mutates the map while firing events.
    SArray<HSTREEITEM> arrSel;
    for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos;)
    {
        HSTREEITEM hItem;
        BOOL bVal;
        m_mapSelItems.GetNextAssoc(pos, hItem, bVal);
        arrSel.Add(hItem);
    }
    for (int i = 0; i < (int)arrSel.GetCount(); i++)
    {
        RemoveSelItem(arrSel[i]);
    }
}

BOOL STreeView::IsItemSelected(HSTREEITEM hItem) const
{
    if (hItem == ITEM_NULL)
        return FALSE;

    if (!m_bMultiSel)
    {
        // Single-selection mode: the anchor m_hSelected is the selection
        // record.
        return hItem == m_hSelected;
    }
    // Multi-selection mode: the map is the only selection record; the
    // anchor is just the keyboard cursor there.
    const ItemSelectionMap::CPair *pVal = m_mapSelItems.Lookup(hItem);
    return pVal && pVal->m_value;
}

int STreeView::GetSelItemCount() const
{
    if (m_bMultiSel)
        return m_mapSelItems.GetCount();
    // Single-selection mode: 1 while the anchor is set.
    return m_hSelected != ITEM_NULL ? 1 : 0;
}

int STreeView::GetSelItems(HSTREEITEM *items, int nMaxCount) const
{
    if (m_bMultiSel)
    {
        int i = 0;
        for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos && nMaxCount > 0;)
        {
            HSTREEITEM hItem;
            BOOL bVal;
            m_mapSelItems.GetNextAssoc(pos, hItem, bVal);
            items[i++] = hItem;
            nMaxCount--;
        }
        return i;
    }
    // Single-selection mode: report the anchor.
    if (m_hSelected != ITEM_NULL && nMaxCount > 0)
    {
        items[0] = m_hSelected;
        return 1;
    }
    return 0;
}

void STreeView::DrawLines(IRenderTarget *pRT, const CRect &rc, HSTREEITEM hItem)
{
    int nIndent = m_tvItemLocator->GetIndent();
    if (nIndent == 0 || !m_pLineSkin || !m_bHasLines)
        return;
    SList<HSTREEITEM> lstParent;
    HSTREEITEM hParent = m_adapter->GetParentItem(hItem);
    while (hParent && hParent != STVI_ROOT)
    {
        lstParent.AddHead(hParent);
        hParent = m_adapter->GetParentItem(hParent);
    }
    // draw parent flags.
    enum
    {
        plus,
        plus_join,
        plus_bottom,
        minus,
        minus_join,
        minus_bottom,
        line,
        line_join,
        line_bottom,
        line_root,
    }; // 10 line states
    CRect rcLine = rc;
    rcLine.right = rcLine.left + nIndent;
    SPOSITION pos = lstParent.GetHeadPosition();
    while (pos)
    {
        HSTREEITEM hParent = lstParent.GetNext(pos);
        HSTREEITEM hNextSibling = m_adapter->GetNextSiblingItem(hParent);
        if (hNextSibling)
        {
            m_pLineSkin->DrawByIndex(pRT, rcLine, line);
        }
        rcLine.OffsetRect(nIndent, 0);
    }
    BOOL hasNextSibling = m_adapter->GetNextSiblingItem(hItem) != 0;
    BOOL hasPervSibling = m_adapter->GetPrevSiblingItem(hItem) != 0;
    BOOL hasChild = m_adapter->HasChildren(hItem);
    bool hasParent = m_adapter->GetParentItem(hItem) != STVI_ROOT;
    int iLine = -1;
    if (hasChild)
    { // test if is collapsed
        if (!m_adapter->IsItemExpanded(hItem))
        {
            if (!hasParent && !hasPervSibling) // no parent
                iLine = plus;
            else if (hasNextSibling)
                iLine = plus_join;
            else
                iLine = plus_bottom;
        }
        else
        {
            if (!hasParent && !hasPervSibling) // no parent
                iLine = minus;
            else if (hasNextSibling)
                iLine = minus_join;
            else
                iLine = minus_bottom;
        }
    }
    else
    {
        if (hasNextSibling)
        {
            if (!hasParent && !hasPervSibling)
                iLine = line_root;
            else
                iLine = line_join;
        }
        else
            iLine = line_bottom;
    }
    m_pLineSkin->DrawByIndex(pRT, rcLine, iLine);
}

void STreeView::OnLButtonDown(UINT nFlags, CPoint pt)
{
    if (m_bHasLines)
    {
        CPoint pt2 = pt;
        int nIndent = m_tvItemLocator->GetIndent();
        pt.x += nIndent;
        SItemPanel *pHoverItem = HitTest(pt);
        if (pHoverItem)
        {
            CRect rcItem = pHoverItem->GetItemRect();
            CRect rcLine(CPoint(rcItem.left - nIndent, rcItem.top + (rcItem.Height() - nIndent) / 2), CSize(nIndent, nIndent));
            if (rcLine.PtInRect(pt2))
            { // switch toggle state
                HSTREEITEM hItem = (HSTREEITEM)pHoverItem->GetItemIndex();
                if (m_adapter->HasChildren(hItem))
                    m_adapter->ExpandItem(hItem, TVC_TOGGLE);
                return;
            }
        }
    }
    SetMsgHandled(FALSE);
}

BOOL STreeView::OnDragCancelCapture(int reason)
{
    if (m_itemCapture)
    {
        if (m_itemCapture->CancelCaptureMode(reason))
        {
            m_itemCapture = NULL;
            return TRUE;
        }
        return FALSE;
    }
    return TRUE;
}

void STreeView::OnDragClearItemCapture()
{
    m_itemCapture = NULL;
}

BOOL STreeView::IsRubberBandSelEnabled() const
{
    return m_bMultiSel && IsBandSelEnabled() && m_adapter != NULL && m_tvItemLocator != NULL;
}

void STreeView::OnRubberBandStart()
{
    m_hBandOldSel = m_hSelected;
    m_arrBandSnapshot.RemoveAll();
    for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos;)
    {
        HSTREEITEM hItem;
        BOOL bVal;
        m_mapSelItems.GetNextAssoc(pos, hItem, bVal);
        m_arrBandSnapshot.Add(hItem);
    }
}

void STreeView::OnRubberBandSelect(const CRect &rcBand, BOOL bAdd)
{
    if (!m_adapter || !m_tvItemLocator)
        return;

    if (!bAdd)
        ClearSelItems();

    // Only visible (materialized) items can be hit by the band.
    CRect rcClient = GetClientRect();
    HSTREEITEM hLast = ITEM_NULL;
    HSTREEITEM hItem = m_adapter->GetFirstVisibleItem();
    while (hItem != ITEM_NULL)
    {
        int nTop = m_tvItemLocator->Item2Position(hItem) - m_siVer.nPos + rcClient.top;
        int nHei = m_tvItemLocator->GetItemHeight(hItem);
        CRect rcItem(rcClient.left, nTop, rcClient.right, nTop + nHei);
        if (nTop > rcBand.bottom)
            break; // items below the band, positions keep increasing
        CRect rcInter;
        if (rcInter.IntersectRect(rcItem, rcBand))
        {
            AddSelItem(hItem);
            hLast = hItem;
        }
        else if (!bAdd && IsItemSelected(hItem))
        {
            RemoveSelItem(hItem);
        }
        hItem = m_adapter->GetNextVisibleItem(hItem);
    }
    if (hLast != ITEM_NULL)
        m_hSelected = hLast;
}

void STreeView::OnRubberBandEnd(const CRect &rcBand, BOOL bCancelled)
{
    if (bCancelled)
    {
        // ESC / external cancel: restore the selection taken at band start.
        // Every restored item fires its per-item event
        // (EventTreeItemSelChanged) via AddSelItem / RemoveSelItem; the
        // anchor-style SelChanged is a single-selection event and is not
        // fired for multi-selection bands.
        ClearSelItems();
        for (int i = 0; i < (int)m_arrBandSnapshot.GetCount(); i++)
        {
            AddSelItem(m_arrBandSnapshot[i]);
        }
        m_arrBandSnapshot.RemoveAll();
        // Restore the cursor unconditionally, matching the other five
        // controls: when the band started with no selection the cursor goes
        // back to "none" instead of staying at the last banded item.
        m_hSelected = m_hBandOldSel;
        m_hSelAnchor = m_hSelected;
        Invalidate();
    }
    else
    {
        // Normal end: the cursor stayed on the last banded item; the range
        // anchor follows it so the next Shift+arrow works from there.
        m_hSelAnchor = m_hSelected;
    }
}

SNSEND
