﻿#include "souistd.h"
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

//////////////////////////////////////////////////////////////////////////
STreeViewItemLocator::STreeViewItemLocator(int nIndent)
    : m_nLineHeight(50)
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
    { // 有子节点
        HSTREEITEM hChild = m_adapter->GetFirstChildItem(hItem);
        int nBranchHeight = 0;
        while (hChild != ITEM_NULL)
        {
            // 设置偏移
            _SetItemOffset(hChild, nBranchHeight);
            _InitBranch(hChild);
            nBranchHeight += _GetItemVisibleHeight(hChild);
            hChild = m_adapter->GetNextSiblingItem(hChild);
        }
        _SetBranchHeight(hItem, nBranchHeight);
        // 设置默认宽度
        _SetBranchWidth(hItem, m_szDef.cx + m_nIndent);
    }
    else
    { // 无子节点
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
    { // 从first开始查找
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
    { // 从last开始查找
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

    SASSERT(FALSE); // 不应该走到这里来
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
    // 注意更新各级父节点的偏移量
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
    { // 父节点的宽度不是由当前结点控制的
        if (nCurBranchWidth < nNewWidth + nIndent)
        { // 新宽度扩展了父节点的显示宽度
            _SetBranchWidth(hParent, nNewWidth + nIndent);
            if (IsItemExpanded(hParent))
                _UpdateBranchWidth(hParent, nCurBranchWidth, nNewWidth + nIndent);
        }
    }
    else
    { // 父节点的宽度正好是由hItem的显示宽度
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
    // 获得父节点开始位置
    HSTREEITEM hParent = m_adapter->GetParentItem(hItem);
    if (hParent != ITEM_NULL && hParent != ITEM_ROOT)
    {
        nRet = Item2Position(hParent);
        // 越过父节点
        nRet += GetItemHeight(hParent);
    }
    // 越过前面兄弟结点
    nRet += _GetItemOffset(hItem);

    return nRet;
}

int STreeViewItemLocator::GetScrollLineSize() const
{
    return m_nLineHeight;
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
    // 初始化列表项高度等数据
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

//////////////////////////////////////////////////////////////////////////
STreeView::STreeView()
    : m_itemCapture(NULL)
    , m_pHoverItem(NULL)
    , m_hSelected(ITEM_NULL)
    , m_pVisibleMap(new VISIBLEITEMSMAP)
    , m_bWantTab(FALSE)
    , m_pLineSkin(GETBUILTINSKIN(SKIN_SYS_TREE_LINES))
    , m_bHasLines(FALSE)
    , SHostProxy(this)
{
    m_bFocusable = TRUE;

    m_evtSet.addEvent(EVENTID(EventTVSelChanging));
    m_evtSet.addEvent(EVENTID(EventTVSelChanged));
    m_observer.Attach(new STreeViewDataSetObserver(this));
    m_tvItemLocator.Attach(new STreeViewItemLocator);
}

STreeView::~STreeView()
{
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
    m_adapter = adapter;
    if (m_adapter)
    {
        m_adapter->registerDataSetObserver(m_observer);
    }
    {
        // free all itemPanels in recycle
        for (size_t i = 0; i < m_itemRecycle.GetCount(); i++)
        {
            SList<SItemPanel *> *lstItemPanels = m_itemRecycle.GetAt(i);
            SPOSITION pos = lstItemPanels->GetHeadPosition();
            while (pos)
            {
                SItemPanel *pItemPanel = lstItemPanels->GetNext(pos);
                pItemPanel->Destroy();
            }
            delete lstItemPanels;
        }
        m_itemRecycle.RemoveAll();

        // free all visible itemPanels
        SPOSITION pos = m_visible_items.GetHeadPosition();
        while (pos)
        {
            ItemInfo ii = m_visible_items.GetNext(pos);
            ii.pItem->Destroy();
        }
        m_visible_items.RemoveAll();
        m_pVisibleMap->RemoveAll();
        m_pHoverItem = NULL;
        m_itemCapture = NULL;
        m_hSelected = 0;
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
    SXmlNode xmlTemplate = xmlNode.child(L"template");
    if (xmlTemplate)
    {
        m_xmlTemplate.Reset();
        m_xmlTemplate.root().append_copy(xmlTemplate);
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

    // destroy all itempanel
    SPOSITION pos = m_visible_items.GetHeadPosition();
    while (pos)
    {
        ItemInfo ii = m_visible_items.GetNext(pos);
        ii.pItem->Release();
    }
    m_visible_items.RemoveAll();
    m_pVisibleMap->RemoveAll();

    for (int i = 0; i < (int)m_itemRecycle.GetCount(); i++)
    {
        SList<SItemPanel *> *pLstTypeItems = m_itemRecycle[i];
        SPOSITION pos = pLstTypeItems->GetHeadPosition();
        while (pos)
        {
            SItemPanel *pItem = pLstTypeItems->GetNext(pos);
            pItem->Release();
        }
        delete pLstTypeItems;
    }
    m_itemRecycle.RemoveAll();

    __baseCls::OnDestroy();
}

void STreeView::EnsureVisible(HSTREEITEM hItem)
{
    // 保证hItem被正确展开
    HSTREEITEM hParent = m_adapter->GetParentItem(hItem);
    while (hParent != ITEM_ROOT)
    {
        m_adapter->ExpandItem(hParent, TVC_EXPAND);
        hParent = m_adapter->GetParentItem(hParent);
    }
    // 滚动视图
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

void STreeView::SetSel(HSTREEITEM hItem, BOOL bNotify /*=FALSE*/)
{
    if (!m_adapter)
        return;

    if (bNotify)
    {
        EventTVSelChanging evt(this);
        evt.bCancel = FALSE;
        evt.hOldSel = m_hSelected;
        evt.hNewSel = hItem;
        FireEvent(&evt);
        if (evt.bCancel)
        { // Cancel SetSel and restore selection state
            return;
        }
    }

    if (m_hSelected == hItem)
        return;

    SItemPanel *pItem = GetItemPanel(m_hSelected);
    if (pItem)
    {
        pItem->GetFocusManager()->ClearFocus();
        pItem->ModifyItemState(0, WndState_Check);
        RedrawItem(pItem);
    }
    m_hSelected = hItem;
    pItem = GetItemPanel(m_hSelected);
    if (pItem)
    {
        pItem->ModifyItemState(WndState_Check, 0);
        RedrawItem(pItem);
    }

    if (bNotify)
    {
        EventTVSelChanged evt(this);
        evt.hOldSel = m_hSelected;
        evt.hNewSel = hItem;
        FireEvent(evt);
    }
}

void STreeView::OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags)
{
    if (!m_adapter)
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
        OnScroll(TRUE, SB_PAGEUP, 0);
        break;
    case VK_NEXT:
        OnScroll(TRUE, SB_PAGEDOWN, 0);
        break;
    case VK_HOME:
        OnScroll(TRUE, SB_TOP, 0);
        break;
    case VK_END:
        OnScroll(TRUE, SB_BOTTOM, 0);
        break;
    case VK_LEFT:
        if (m_hSelected != ITEM_NULL)
        {
            if (m_adapter->HasChildren(m_hSelected) && m_adapter->IsItemExpanded(m_hSelected))
                m_adapter->ExpandItem(m_hSelected,
                                      TVC_COLLAPSE); // collapse the selected item
            else
                nNewSelItem = m_adapter->GetPrevVisibleItem(m_hSelected);
        }
        break;
    case VK_RIGHT:
        if (m_hSelected != ITEM_NULL)
        {
            if (m_adapter->HasChildren(m_hSelected) && !m_adapter->IsItemExpanded(m_hSelected))
                m_adapter->ExpandItem(m_hSelected,
                                      TVC_EXPAND); // collapse the selected item
            else
                nNewSelItem = m_adapter->GetNextVisibleItem(m_hSelected);
        }
        break;
    }

    if (nNewSelItem != ITEM_NULL)
    {
        EnsureVisible(nNewSelItem);
        SetSel(nNewSelItem, TRUE);
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
    SWindow::GetClientRect(&rcClient); // 不计算滚动条大小
    CSize size = rcClient.Size();
    //  关闭滚动条
    m_wBarVisible = SSB_NULL;

    if (size.cy < szView.cy || (size.cy < szView.cy + GetSbWidth() && size.cx < szView.cx))
    {
        //  需要纵向滚动条
        m_wBarVisible |= SSB_VERT;
        m_siVer.nMin = 0;
        m_siVer.nMax = szView.cy - 1;
        m_siVer.nPage = rcClient.Height();

        if (size.cx - GetSbWidth() < szView.cx)
        {
            // 如果是适应宽度，则说明需要更新项宽度，否则需要横向滚动条
            if (m_adapter->isViewWidthMatchParent())
            {
                UpdateVisibleItems();
            }
            else
            {
                //  需要横向滚动条
                m_wBarVisible |= SSB_HORZ;
                m_siVer.nPage = size.cy - GetSbWidth() > 0 ? size.cy - GetSbWidth() : 0; // 注意同时调整纵向滚动条page信息

                m_siHoz.nMin = 0;
                m_siHoz.nMax = szView.cx - 1;
                m_siHoz.nPage = (size.cx - GetSbWidth()) > 0 ? (size.cx - GetSbWidth()) : 0;
            }
        }
        else
        {
            //  不需要横向滚动条
            m_siHoz.nPage = size.cx;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = m_siHoz.nPage - 1;
            m_siHoz.nPos = 0;
        }
    }
    else
    {
        //  不需要纵向滚动条
        m_siVer.nPage = size.cy;
        m_siVer.nMin = 0;
        m_siVer.nMax = size.cy - 1;
        m_siVer.nPos = 0;

        if (size.cx < szView.cx)
        {
            //  需要横向滚动条
            m_wBarVisible |= SSB_HORZ;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = szView.cx - 1;
            m_siHoz.nPage = size.cx;
        }
        else
        {
            //  不需要横向滚动条
            m_siHoz.nPage = size.cx;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = m_siHoz.nPage - 1;
            m_siHoz.nPos = 0;
        }
    }

    //  根据需要调整原点位置
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

    //  重新计算客户区及非客户区
    SSendMessage(WM_NCCALCSIZE);

    InvalidateRect(NULL);
}

void STreeView::UpdateVisibleItems()
{
    if (!m_adapter)
        return;
    SAutoEnableHostPrivUiDef enableUiDef(this);
    HSTREEITEM hItem = m_tvItemLocator->Position2Item(m_siVer.nPos);
    if (hItem == ITEM_NULL)
    {
        // 如果没有可显示的，则移除所有item
        SPOSITION pos = m_visible_items.GetHeadPosition();
        while (pos)
        {
            ItemInfo ii = m_visible_items.GetNext(pos);

            if (ii.pItem == m_pHoverItem)
            {
                m_pHoverItem->DoFrameEvent(WM_MOUSELEAVE, 0, 0);
                m_pHoverItem = NULL;
            }

            ii.pItem->GetEventSet()->setMutedState(true);
            if ((HSTREEITEM)ii.pItem->GetItemIndex() == m_hSelected)
            {
                ii.pItem->ModifyItemState(0, WndState_Check);
                ii.pItem->GetFocusManager()->ClearFocus();
            }
            ii.pItem->SetVisible(FALSE); // 防止执行SItemPanel::OnTimeFrame()
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
            { // 创建一个新的列表项
                bNewItem = TRUE;
                ii.pItem = SItemPanel::Create(this, SXmlNode(), this);
                ii.pItem->GetEventSet()->subscribeEvent(EventItemPanelClick::EventID, Subscriber(&STreeView::OnItemClick, this));
            }
            else
            {
                ii.pItem = lstRecycle->RemoveHead();
            }
            ii.pItem->SetItemIndex(hItem);
        }
        m_pVisibleMap->SetAt(hItem, ii);
        ii.pItem->SetVisible(TRUE);

        if (hItem == m_hSelected)
            ii.pItem->ModifyItemState(WndState_Check, 0);
        else
            ii.pItem->ModifyItemState(0, WndState_Check);

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
        // 不使宽度
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
        }

        ii.pItem->GetEventSet()->setMutedState(true);
        if ((HSTREEITEM)ii.pItem->GetItemIndex() == m_hSelected)
        {
            ii.pItem->ModifyItemState(0, WndState_Check);
            ii.pItem->GetFocusManager()->ClearFocus();
            m_hSelected = 0;
        }
        ii.pItem->SetVisible(FALSE); // 防止执行SItemPanel::OnTimeFrame()
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
        UpdateVisibleItems(); // 根据新的滚动条状态重新记算显示列表项
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
            while (hItem)
            {
                if (hItem == hBranch)
                {
                    bInvalid = true;
                    break;
                }
                hItem = m_adapter->GetParentItem(hItem);
            }
            if (bInvalid)
            {
                SXmlNode xmlNode = m_xmlTemplate.root().first_child();
                m_adapter->getView(hBranch, ii.pItem, &xmlNode);
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
    }

    if (m_pHoverItem)
    {
        HSTREEITEM hHover = m_pHoverItem->GetItemIndex();
        if (hHover == hItem || m_adapter->IsDecendentItem(hItem, hHover))
        {
            m_pHoverItem->DoFrameEvent(WM_MOUSELEAVE, 0, 0);
            m_pHoverItem = NULL;
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

    if (m_itemCapture)
    {
        CRect rcItem = m_itemCapture->GetItemRect();
        pt.Offset(-rcItem.TopLeft());
        lRet = m_itemCapture->DoFrameEvent(uMsg, wParam, MAKELPARAM(pt.x, pt.y));
    }
    else
    {
        if (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN)
        { // 交给panel处理
            CPoint pt2(pt);
            SItemPanel *pPanel = HitTest(pt2);
            if (!pPanel && m_hSelected) // hit in none-item area,so make item to killfocus
            {
                SItemPanel *pSelItem = GetItemPanel(m_hSelected);
                if (pSelItem)
                    pSelItem->DoFrameEvent(WM_KILLFOCUS, 0, 0);
                // m_hSelected = NULL;
            }

            __baseCls::ProcessSwndMessage(uMsg, wParam, lParam, lRet);
        }

        SOsrPanel *pHover = HitTest(pt);
        if (pHover != m_pHoverItem)
        {
            SOsrPanel *oldHover = m_pHoverItem;
            m_pHoverItem = pHover;
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
    { // 交给panel处理
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

        // 加速滚动时UI的刷新
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

BOOL STreeView::OnItemClick(IEvtArgs *pEvt)
{
    SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pEvt->Sender());
    HSTREEITEM hItem = (HSTREEITEM)pItemPanel->GetItemIndex();
    if (hItem != m_hSelected)
    {
        SetSel(hItem, TRUE);
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
    UpdateVisibleItems(); // 防止因为字体大小变化后，列表项大小发生变化没有更新。其它view不需要这个过程。
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

SNSEND
