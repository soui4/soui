﻿#include "souistd.h"
#include "control/STileView.h"

SNSBEGIN

class STileViewDataSetObserver : public TObjRefImpl<ILvDataSetObserver> {
  public:
    STileViewDataSetObserver(STileView *pView)
        : m_pOwner(pView)
    {
    }
    STDMETHOD_(void, onChanged)(THIS) OVERRIDE;

    STDMETHOD_(void, onInvalidated)(THIS) OVERRIDE;

    STDMETHOD_(void, OnItemChanged)(THIS_ int iItem) OVERRIDE;

  protected:
    STileView *m_pOwner;
};

//////////////////////////////////////////////////////////////////////////
void STileViewDataSetObserver::onChanged()
{
    m_pOwner->onDataSetChanged();
}

void STileViewDataSetObserver::onInvalidated()
{
    m_pOwner->onDataSetInvalidated();
}

void STileViewDataSetObserver::OnItemChanged(int iItem)
{
    m_pOwner->onItemDataChanged(iItem);
}

//////////////////////////////////////////////////////////////////////////
STileView::STileView()
    : m_iSelItem(-1)
    , m_iFirstVisible(-1)
    , m_pHoverItem(NULL)
    , m_itemCapture(NULL)
    , m_nMarginSize(0.0f, SLayoutSize::px)
    , m_bWantTab(FALSE)
    , m_bDatasetInvalidated(false)
    , m_bPendingUpdate(false)
    , m_iPendingUpdateItem(-2)
    , m_iPendingViewItem(-1)
    , SHostProxy(this)

{
    m_bFocusable = TRUE;
    m_observer.Attach(new STileViewDataSetObserver(this));
    m_dwUpdateInterval = 40;
    m_evtSet.addEvent(EVENTID(EventLVSelChanging));
    m_evtSet.addEvent(EVENTID(EventLVSelChanged));
}

STileView::~STileView()
{
    m_observer = NULL;
    m_tvItemLocator = NULL;
}

BOOL STileView::SetAdapter(ILvAdapter *adapter)
{
    if (!m_tvItemLocator)
    {
        SASSERT_MSGA(FALSE, "error: A item locator is in need before setting adapter!!!");
        return FALSE;
    }
    if (m_adapter == adapter)
    {
        SSLOGW() << "the new adapter is same to previous set adapter, same as notifyDatasetChanged";
        if (m_adapter)
        {
            onDataSetChanged();
        }
        return TRUE;
    }
    if (m_adapter)
    {
        m_adapter->unregisterDataSetObserver(m_observer);
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
        SPOSITION pos = m_lstItems.GetHeadPosition();
        while (pos)
        {
            ItemInfo ii = m_lstItems.GetNext(pos);
            ii.pItem->Destroy();
        }
        m_lstItems.RemoveAll();
        m_pHoverItem = NULL;
        m_itemCapture = NULL;
        m_iSelItem = -1;
        m_iFirstVisible = -1;
    }

    m_adapter = adapter;
    if (m_tvItemLocator)
    {
        m_tvItemLocator->SetTileViewWidth(GetClientRect().Width(), FALSE);
        m_tvItemLocator->SetAdapter(adapter);
    }
    if (m_adapter)
    {
        SXmlNode xmlNode = m_xmlTemplate.root().first_child();
        m_adapter->InitByTemplate(&xmlNode);
        m_adapter->registerDataSetObserver(m_observer);
        for (int i = 0; i < m_adapter->getViewTypeCount(); i++)
        {
            m_itemRecycle.Add(new SList<SItemPanel *>());
        }
        onDataSetChanged();
    }
    return TRUE;
}

void STileView::UpdateScrollBar()
{
    CRect rcClient = SWindow::GetClientRect();
    CSize size = rcClient.Size();
    CSize szView;
    szView.cx = rcClient.Width();
    szView.cy = m_tvItemLocator ? m_tvItemLocator->GetTotalHeight() : 0;

    //  关闭滚动条
    m_wBarVisible = SSB_NULL;

    if (size.cy < szView.cy)
    {
        //  需要纵向滚动条
        m_wBarVisible |= SSB_VERT;
        m_siVer.nMin = 0;
        m_siVer.nMax = szView.cy - 1;
        m_siVer.nPage = size.cy;
        m_siVer.nPos = smin(m_siVer.nPos, m_siVer.nMax - (int)m_siVer.nPage);
    }
    else
    {
        //  不需要纵向滚动条
        m_siVer.nPage = size.cy;
        m_siVer.nMin = 0;
        m_siVer.nMax = size.cy - 1;
        m_siVer.nPos = 0;
    }

    SetScrollPos(TRUE, m_siVer.nPos, FALSE);

    //  重新计算客户区及非客户区
    SSendMessage(WM_NCCALCSIZE);

    InvalidateRect(NULL);
}

void STileView::onDataSetChanged()
{
    if (!m_adapter)
    {
        return;
    }
    if (m_tvItemLocator)
    {
        m_tvItemLocator->OnDataSetChanged();
    }
    if (m_iSelItem != -1 && m_iSelItem >= m_adapter->getCount())
        m_iSelItem = -1;

    UpdateScrollBar();
    UpdateVisibleItems();
}

void STileView::onDataSetInvalidated()
{
    m_bDatasetInvalidated = TRUE;
    Invalidate();
}

void STileView::UpdateVisibleItem(int iItem)
{
    SAutoEnableHostPrivUiDef enableUiDef(this);
    SItemPanel *pItem = GetItemPanel(iItem);
    SASSERT(pItem);
    SXmlNode xmlNode = m_xmlTemplate.root().first_child();
    m_adapter->getView(iItem, pItem, &xmlNode);
    pItem->Invalidate();
}

void STileView::onItemDataChanged(int iItem)
{
    if (!m_adapter)
        return;
    if (!IsVisible(TRUE))
    {
        m_bPendingUpdate = true;
        m_iPendingUpdateItem = m_iPendingUpdateItem == -2 ? iItem : -1;
        return;
    }
    if (iItem < m_iFirstVisible)
        return;
    if (iItem >= m_iFirstVisible + (int)m_lstItems.GetCount())
        return;
    UpdateVisibleItem(iItem);
}

void STileView::OnPaint(IRenderTarget *pRT)
{
    if (m_bDatasetInvalidated)
    {
        UpdateVisibleItems();
        m_bDatasetInvalidated = FALSE;
    }
    SPainter duiDC;
    BeforePaint(pRT, duiDC);

    int iFirst = m_iFirstVisible;
    if (iFirst != -1)
    {
        CRect rcClient;
        GetClientRect(&rcClient);
        pRT->PushClipRect(&rcClient, RGN_AND);

        float fMat[9];
        pRT->GetTransform(fMat);
        SMatrix mtx(fMat);

        CRect rcClip;
        pRT->GetClipBox(&rcClip);
        SAutoRefPtr<IRegionS> rgnClip;
        pRT->GetClipRegion(&rgnClip);

        int nOffset = m_tvItemLocator->Item2Position(iFirst) - m_siVer.nPos;
        int nLastBottom = rcClient.top + m_tvItemLocator->GetMarginSize() + nOffset;

        SPOSITION pos = m_lstItems.GetHeadPosition();
        int i = 0;
        for (; pos; i++)
        {
            ItemInfo ii = m_lstItems.GetNext(pos);
            CRect rcItem = m_tvItemLocator->GetItemRect(iFirst + i);
            rcItem.OffsetRect(rcClient.left, 0);
            rcItem.MoveToY(nLastBottom);
            if (m_tvItemLocator->IsLastInRow(iFirst + i))
            {
                nLastBottom = rcItem.bottom + m_tvItemLocator->GetMarginSize();
            }
            if (SItemPanel::IsItemInClip(mtx, rcClip, rgnClip, rcItem))
            {
                ii.pItem->Draw(pRT, rcItem);
            }
        }

        pRT->PopClip();
    }
    AfterPaint(pRT, duiDC);
}

BOOL STileView::OnScroll(BOOL bVertical, UINT uCode, int nPos)
{
    int nOldPos = m_siVer.nPos;
    __baseCls::OnScroll(bVertical, uCode, nPos);
    int nNewPos = m_siVer.nPos;
    if (nOldPos != nNewPos)
    {
        UpdateVisibleItems();

        //加速滚动时UI的刷新
        if (uCode == SB_THUMBTRACK)
        {
            ScrollUpdate();
        }
        return TRUE;
    }
    return FALSE;
}

void STileView::UpdateVisibleItems()
{
    if (!m_adapter)
    {
        return;
    }
    SAutoEnableHostPrivUiDef enableUiDef(this);
    int iOldFirstVisible = m_iFirstVisible;
    int iOldLastVisible = m_iFirstVisible + (int)m_lstItems.GetCount();

    int iNewFirstVisible = m_tvItemLocator->Position2Item(m_siVer.nPos);
    int iNewLastVisible = iNewFirstVisible;

    int pos = m_tvItemLocator->Item2Position(iNewFirstVisible);
    int iHoverItem = m_pHoverItem ? (int)m_pHoverItem->GetItemIndex() : -1;

    ItemInfo *pItemInfos = new ItemInfo[m_lstItems.GetCount()];
    SPOSITION spos = m_lstItems.GetHeadPosition();
    int i = 0;
    while (spos)
    {
        pItemInfos[i++] = m_lstItems.GetNext(spos);
    }

    m_lstItems.RemoveAll();

    if (iNewFirstVisible != -1)
    {
        while (pos < m_siVer.nPos + (int)m_siVer.nPage && iNewLastVisible < m_adapter->getCount())
        {
            DWORD dwState = WndState_Normal;
            if (iHoverItem == iNewLastVisible)
                dwState |= WndState_Hover;
            if (m_iSelItem == iNewLastVisible)
                dwState |= WndState_Check;

            ItemInfo ii = { NULL, -1 };
            ii.nType = m_adapter->getItemViewType(iNewLastVisible, dwState);
            if (iNewLastVisible >= iOldFirstVisible && iNewLastVisible < iOldLastVisible)
            {
                // use the old visible item
                int iItem = iNewLastVisible - iOldFirstVisible;
                SASSERT(iItem >= 0 && iItem <= (iOldLastVisible - iOldFirstVisible));
                if (ii.nType == pItemInfos[iItem].nType)
                {
                    ii = pItemInfos[iItem];
                    pItemInfos[iItem].pItem = NULL; //标记该行已经被重用
                }
            }
            BOOL bNewItem = FALSE;
            if (!ii.pItem)
            {
                // create new visible item
                SList<SItemPanel *> *lstRecycle = m_itemRecycle.GetAt(ii.nType);
                if (lstRecycle->IsEmpty())
                {
                    //创建一个新的列表项
                    bNewItem = TRUE;
                    ii.pItem = SItemPanel::Create(this, SXmlNode(), this);
                    ii.pItem->GetEventSet()->subscribeEvent(EventItemPanelClick::EventID, Subscriber(&STileView::OnItemClick, this));
                }
                else
                {
                    ii.pItem = lstRecycle->RemoveHead();
                }
                ii.pItem->SetItemIndex(iNewLastVisible);
            }
            ii.pItem->SetVisible(TRUE);
            CRect rcItem = m_tvItemLocator->GetItemRect(iNewLastVisible);
            rcItem.MoveToXY(0, 0);
            ii.pItem->Move(rcItem);

            //设置状态，同时暂时禁止应用响应statechanged事件。
            ii.pItem->GetEventSet()->setMutedState(true);
            ii.pItem->ModifyItemState(dwState, 0);
            ii.pItem->GetEventSet()->setMutedState(false);
            if (dwState & WndState_Hover)
                m_pHoverItem = ii.pItem;

            SXmlNode xmlNode = m_xmlTemplate.root().first_child();
            ii.pItem->LockUpdate();
            m_adapter->getView(iNewLastVisible, ii.pItem, &xmlNode);
            ii.pItem->UnlockUpdate();
            if (bNewItem)
            {
                ii.pItem->SDispatchMessage(UM_SETSCALE, GetScale(), 0);
                ii.pItem->SDispatchMessage(UM_SETLANGUAGE, 0, 0);
                ii.pItem->DoColorize(GetColorizeColor());
            }

            ii.pItem->UpdateLayout();
            if (iNewLastVisible == m_iSelItem)
            {
                ii.pItem->ModifyItemState(WndState_Check, 0);
            }

            m_lstItems.AddTail(ii);

            if (m_tvItemLocator->IsLastInRow(iNewLastVisible))
            {
                pos += rcItem.Height() + m_tvItemLocator->GetMarginSize();
            }

            iNewLastVisible++;
        }
    }

    // move old visible items which were not reused to recycle
    for (int i = 0; i < (iOldLastVisible - iOldFirstVisible); i++)
    {
        ItemInfo ii = pItemInfos[i];
        if (!ii.pItem)
        {
            continue;
        }
        if (ii.pItem->GetState() & WndState_Hover)
        {
            ii.pItem->DoFrameEvent(WM_MOUSELEAVE, 0, 0);
            m_pHoverItem = NULL;
        }
        ii.pItem->GetEventSet()->setMutedState(true);
        if (ii.pItem->GetState() & WndState_Check)
        {
            ii.pItem->ModifyItemState(0, WndState_Check);
            ii.pItem->GetFocusManager()->ClearFocus();
        }
        ii.pItem->SetVisible(FALSE);
        ii.pItem->GetEventSet()->setMutedState(false);
        m_itemRecycle[ii.nType]->AddTail(ii.pItem);
    }
    delete[] pItemInfos;

    m_iFirstVisible = iNewFirstVisible;
    InvalidateRect(NULL);
}

void STileView::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);

    CRect rcClient = SWindow::GetClientRect();
    m_tvItemLocator->SetTileViewWidth(rcClient.Width(), FALSE); //重设TileView宽度
    UpdateScrollBar();                                          //重设滚动条

    UpdateVisibleItems();
}

void STileView::OnDestroy()
{
    if (m_adapter)
    {
        m_adapter->unregisterDataSetObserver(m_observer);
    }

    // destroy all itempanel
    SPOSITION pos = m_lstItems.GetHeadPosition();
    while (pos)
    {
        ItemInfo ii = m_lstItems.GetNext(pos);
        ii.pItem->Release();
    }
    m_lstItems.RemoveAll();

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

//////////////////////////////////////////////////////////////////////////

BOOL STileView::IsItemRedrawDelay() const
{
    return TRUE;
}
BOOL STileView::IsTimelineEnabled() const
{
    return !m_sbHorz.IsThumbTracking() && !m_sbVert.IsThumbTracking();
}
CRect STileView::CalcItemDrawRect(int iItem) const
{
    //相对整个窗体的实际绘制位置
    int nOffset = m_tvItemLocator->Item2Position(iItem) - m_siVer.nPos;

    CRect rcClient = GetClientRect();
    //获取left/right
    CRect rcItem = m_tvItemLocator->GetItemRect(iItem);
    rcItem.OffsetRect(rcClient.TopLeft());
    //修正top/bottom
    rcItem.MoveToY(rcClient.top + m_tvItemLocator->GetMarginSize() + nOffset);
    return rcItem;
}

BOOL STileView::OnItemGetRect(const SOsrPanel *pItem, CRect &rcItem) const
{
    int iPosition = (int)pItem->GetItemIndex();
    if (iPosition < 0 || iPosition >= m_adapter->getCount())
        return FALSE;
    rcItem = CalcItemDrawRect(iPosition);
    return TRUE;
}

void STileView::OnItemSetCapture(SOsrPanel *pItem, BOOL bCapture)
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

void STileView::RedrawItem(SOsrPanel *pItem)
{
    pItem->InvalidateRect(NULL);
}

SItemPanel *STileView::HitTest(CPoint &pt) const
{
    SPOSITION pos = m_lstItems.GetHeadPosition();
    while (pos)
    {
        ItemInfo ii = m_lstItems.GetNext(pos);
        CRect rcItem = ii.pItem->GetItemRect();
        if (rcItem.PtInRect(pt))
        {
            pt -= rcItem.TopLeft();
            return ii.pItem;
        }
    }
    return NULL;
}

IItemPanel *STileView::HitTest(const POINT *pt) const
{
    SASSERT(pt);
    if (!pt)
        return NULL;
    CPoint pt2(*pt);
    return HitTest(pt2);
}

LRESULT STileView::OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SetMsgHandled(FALSE);
    if (!m_adapter)
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
        { //交给panel处理
            __baseCls::ProcessSwndMessage(uMsg, wParam, lParam, lRet);
        }

        SItemPanel *pHover = HitTest(pt);
        if (pHover != m_pHoverItem)
        {
            SOsrPanel *nOldHover = m_pHoverItem;
            m_pHoverItem = pHover;
            if (nOldHover)
            {
                nOldHover->DoFrameEvent(WM_MOUSELEAVE, 0, 0);
                RedrawItem(nOldHover);
            }
            if (m_pHoverItem)
            {
                m_pHoverItem->DoFrameEvent(WM_MOUSEHOVER, wParam, MAKELPARAM(pt.x, pt.y));
                RedrawItem(m_pHoverItem);
            }
        }
        if (m_pHoverItem)
        {
            m_pHoverItem->DoFrameEvent(uMsg, wParam, MAKELPARAM(pt.x, pt.y));
        }
    }

    if (uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONUP || uMsg == WM_MBUTTONUP)
    { //交给panel处理
        __baseCls::ProcessSwndMessage(uMsg, wParam, lParam, lRet);
    }
    SetMsgHandled(TRUE);

    return 0;
}

LRESULT STileView::OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet = 0;
    SItemPanel *pItem = GetItemPanel(m_iSelItem);
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

void STileView::OnMouseLeave()
{
    __baseCls::OnMouseLeave();

    if (m_pHoverItem)
    {
        m_pHoverItem->DoFrameEvent(WM_MOUSELEAVE, 0, 0);
        m_pHoverItem = NULL;
    }
}

void STileView::OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags)
{
    if (!m_adapter)
    {
        SetMsgHandled(FALSE);
        return;
    }

    if (m_iSelItem != -1 && m_bWantTab)
    {
        SItemPanel *pItem = GetItemPanel(m_iSelItem);
        if (pItem)
        {
            pItem->DoFrameEvent(WM_KEYDOWN, nChar, MAKELONG(nFlags, nRepCnt));
            if (pItem->IsMsgHandled())
            {
                return;
            }
        }
    }

    int nNewSelItem = -1;
    SWindow *pOwner = GetOwner();
    if (pOwner && (nChar == VK_ESCAPE || nChar == VK_RETURN))
    {
        pOwner->SSendMessage(WM_KEYDOWN, nChar, MAKELONG(nFlags, nRepCnt));
        return;
    }

    if (nChar == VK_RIGHT && m_iSelItem < m_adapter->getCount() - 1)
    {
        nNewSelItem = m_iSelItem + 1;
    }
    else if (nChar == VK_LEFT && m_iSelItem > 0)
    {
        nNewSelItem = m_iSelItem - 1;
    }
    else if (nChar == VK_UP && m_iSelItem > 0)
    {
        nNewSelItem = m_tvItemLocator->GetUpItem(m_iSelItem);
    }
    else if (nChar == VK_DOWN && m_iSelItem < m_adapter->getCount() - 1)
    {
        nNewSelItem = m_tvItemLocator->GetDownItem(m_iSelItem);
    }
    else
    {
        switch (nChar)
        {
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
        }
        if (nChar == VK_PRIOR || nChar == VK_HOME)
        {
            if (!m_lstItems.IsEmpty())
            {
                nNewSelItem = (int)(m_lstItems.GetHead().pItem->GetItemIndex());
            }
        }
        else if (nChar == VK_NEXT || nChar == VK_END)
        {
            if (!m_lstItems.IsEmpty())
            {
                nNewSelItem = (int)(m_lstItems.GetTail().pItem->GetItemIndex());
            }
        }
    }

    if (nNewSelItem != -1)
    {
        EnsureVisible(nNewSelItem);
        SetSel(nNewSelItem, TRUE);
    }
    else
    {
        SetMsgHandled(FALSE);
    }
}

void STileView::EnsureVisible(int iItem)
{
    if (iItem < 0 || iItem >= m_adapter->getCount())
    {
        return;
    }
    if (!IsVisible(TRUE))
    {
        m_iPendingViewItem = iItem;
        return;
    }

    CRect rcItem = m_tvItemLocator->GetItemRect(iItem);
    if (rcItem.top < m_siVer.nPos)
    {
        // scroll up
        OnScroll(TRUE, SB_THUMBPOSITION, rcItem.top);
    }
    if (rcItem.bottom > m_siVer.nPos + (int)m_siVer.nPage)
    {
        // scroll down
        OnScroll(TRUE, SB_THUMBPOSITION, rcItem.bottom - m_siVer.nPage);
    }
}

BOOL STileView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    SItemPanel *pSelItem = GetItemPanel(m_iSelItem);
    if (pSelItem)
    {
        CRect rcItem = pSelItem->GetItemRect();
        CPoint pt2 = pt - rcItem.TopLeft();
        if (pSelItem->DoFrameEvent(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt2.x, pt2.y)))
        {
            return TRUE;
        }
    }
    return __baseCls::OnMouseWheel(nFlags, zDelta, pt);
}

int STileView::GetScrollLineSize(BOOL bVertical)
{
    return m_tvItemLocator->GetScrollLineSize();
}

SItemPanel *STileView::GetItemPanel(int iItem)
{
    if (!m_adapter || iItem < 0 || iItem >= m_adapter->getCount())
    {
        return NULL;
    }
    SPOSITION pos = m_lstItems.GetHeadPosition();
    while (pos)
    {
        ItemInfo ii = m_lstItems.GetNext(pos);
        if ((int)ii.pItem->GetItemIndex() == iItem)
        {
            return ii.pItem;
        }
    }
    return NULL;
}

BOOL STileView::CreateChildren(SXmlNode xmlNode)
{
    SXmlNode xmlTemplate = xmlNode.child(L"template");
    if (xmlTemplate)
    {
        m_xmlTemplate.Reset();
        m_xmlTemplate.root().append_copy(xmlTemplate);
        {
            //创建一个定位器
            STileViewItemLocator *pItemLocator = new STileViewItemLocator(this, xmlTemplate.attribute(L"itemHeight").as_string(L"10dp"), xmlTemplate.attribute(L"itemWidth").as_string(L"10dp"), m_nMarginSize);
            SetItemLocator(pItemLocator);
            pItemLocator->Release();
        }
    }
    return TRUE;
}

void STileView::SetItemLocator(ITileViewItemLocator *pItemLocator)
{
    m_tvItemLocator = pItemLocator;
    if (m_tvItemLocator)
    {
        m_tvItemLocator->SetAdapter(GetAdapter());
        m_tvItemLocator->SetScale(GetScale());
    }
    onDataSetChanged();
}

BOOL STileView::UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
{
    if (!m_pHoverItem)
    {
        return __baseCls::UpdateToolTip(pt, tipInfo);
    }
    return m_pHoverItem->UpdateToolTip(pt, tipInfo);
}

void STileView::SetSel(int iItem, BOOL bNotify)
{
    if (!m_adapter)
    {
        return;
    }

    if (iItem >= m_adapter->getCount())
    {
        return;
    }

    if (iItem < 0)
    {
        iItem = -1;
    }

    int nOldSel = m_iSelItem;
    int nNewSel = iItem;

    m_iSelItem = nNewSel;
    if (bNotify)
    {
        EventLVSelChanging evt(this);
        evt.bCancel = FALSE;
        evt.iOldSel = nOldSel;
        evt.iNewSel = nNewSel;
        FireEvent(evt);
        if (evt.bCancel)
        {
            // Cancel SetSel and restore selection state
            m_iSelItem = nOldSel;
            return;
        }
    }

    if (nOldSel == nNewSel)
    {
        return;
    }

    m_iSelItem = nOldSel;
    SItemPanel *pItem = GetItemPanel(nOldSel);
    if (pItem)
    {
        pItem->GetFocusManager()->ClearFocus();
        pItem->ModifyItemState(0, WndState_Check);
        RedrawItem(pItem);
    }
    m_iSelItem = nNewSel;
    pItem = GetItemPanel(nNewSel);
    if (pItem)
    {
        pItem->ModifyItemState(WndState_Check, 0);
        RedrawItem(pItem);
    }

    if (bNotify)
    {
        EventLVSelChanged evt(this);
        evt.iOldSel = nOldSel;
        evt.iNewSel = nNewSel;
        FireEvent(evt);
    }
}

UINT STileView::OnGetDlgCode() const
{
    if (m_bWantTab)
    {
        return SC_WANTALLKEYS;
    }
    else
    {
        return SC_WANTARROWS | SC_WANTSYSKEY;
    }
}

void STileView::OnKillFocus(SWND wndFocus)
{
    __baseCls::OnKillFocus(wndFocus);

    if (m_iSelItem == -1)
    {
        return;
    }

    SItemPanel *pSelPanel = GetItemPanel(m_iSelItem);
    if (pSelPanel)
    {
        pSelPanel->GetFocusManager()->StoreFocusedView();
    }
}

void STileView::OnSetFocus(SWND wndOld)
{
    __baseCls::OnSetFocus(wndOld);
    if (m_iSelItem == -1)
    {
        return;
    }

    SItemPanel *pSelPanel = GetItemPanel(m_iSelItem);
    if (pSelPanel)
    {
        pSelPanel->GetFocusManager()->RestoreFocusedView();
    }
}

LRESULT STileView::OnSetScale(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int nScale = (int)wParam;
    m_tvItemLocator->SetScale(nScale);
    __baseCls::OnSetScale(uMsg, wParam, lParam);
    return LRESULT();
}

BOOL STileView::OnSetCursor(const CPoint &pt)
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

BOOL STileView::OnItemClick(IEvtArgs *pEvt)
{
    SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pEvt->Sender());
    int iItem = (int)pItemPanel->GetItemIndex();
    if (iItem != m_iSelItem)
    {
        SetSel(iItem, TRUE);
    }
    return true;
}

void STileView::OnColorize(COLORREF cr)
{
    __baseCls::OnColorize(cr);
    DispatchMessage2Items(UM_SETCOLORIZE, cr, 0);
}

void STileView::OnScaleChanged(int nScale)
{
    __baseCls::OnScaleChanged(nScale);
    if (m_tvItemLocator)
        m_tvItemLocator->SetScale(nScale);
    DispatchMessage2Items(UM_SETSCALE, nScale, 0);
    UpdateVisibleItems();
}

HRESULT STileView::OnLanguageChanged()
{
    HRESULT hret = __baseCls::OnLanguageChanged();
    DispatchMessage2Items(UM_SETLANGUAGE, 0, 0);
    return hret;
}

void STileView::DispatchMessage2Items(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SPOSITION pos = m_lstItems.GetHeadPosition();
    while (pos)
    {
        ItemInfo ii = m_lstItems.GetNext(pos);
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

void STileView::OnShowWindow(BOOL bShow, UINT nStatus)
{
    __baseCls::OnShowWindow(bShow, nStatus);
    if (IsVisible(TRUE))
    {
        if (m_bPendingUpdate)
        {
            if (m_iPendingUpdateItem == -1)
                onDataSetChanged();
            else
                onItemDataChanged(m_iPendingUpdateItem);
            m_bPendingUpdate = false;
            m_iPendingUpdateItem = -2;
        }
        if (m_iPendingViewItem != -1)
        {
            EnsureVisible(m_iPendingViewItem);
            m_iPendingViewItem = -1;
        }
    }
}

void STileView::OnRebuildFont()
{
    __baseCls::OnRebuildFont();
    DispatchMessage2Items(UM_UPDATEFONT, 0, 0);
}

ILvAdapter *STileView::GetAdapter() const
{
    return m_adapter;
}

ITileViewItemLocator *STileView::GetItemLocator() const
{
    return m_tvItemLocator;
}

int STileView::GetSel() const
{
    return m_iSelItem;
}

SNSEND
