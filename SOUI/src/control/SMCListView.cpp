﻿#include "souistd.h"
#include "control/SMCListView.h"
#include "helper/SListViewItemLocator.h"

#pragma warning(disable : 4267 4018)

#define ITEM_MARGIN 4
SNSBEGIN
class SMCListViewDataSetObserver : public TObjRefImpl<ILvDataSetObserver> {
  public:
    SMCListViewDataSetObserver(SMCListView *pView)
        : m_pOwner(pView)
    {
    }
    STDMETHOD_(void, onChanged)(THIS) OVERRIDE;

    STDMETHOD_(void, onInvalidated)(THIS) OVERRIDE;

    STDMETHOD_(void, OnItemChanged)(THIS_ int iItem) OVERRIDE;

  protected:
    SMCListView *m_pOwner;
};

//////////////////////////////////////////////////////////////////////////
void SMCListViewDataSetObserver::onChanged()
{
    m_pOwner->onDataSetChanged();
}

void SMCListViewDataSetObserver::onInvalidated()
{
    m_pOwner->onDataSetInvalidated();
}

void SMCListViewDataSetObserver::OnItemChanged(int iItem)
{
    m_pOwner->onItemDataChanged(iItem);
}

//////////////////////////////////////////////////////////////////////////
//  SMCListView

SMCListView::SMCListView()
    : m_pHeader(NULL)
    , m_iSelItem(-1)
    , m_iFirstVisible(-1)
    , m_pHoverItem(NULL)
    , m_itemCapture(NULL)
    , m_pSkinDivider(NULL)
    , m_bWantTab(FALSE)
    , m_bDatasetInvalidated(TRUE)
    , m_bPendingUpdate(false)
    , m_iPendingUpdateItem(-2)
    , m_iPendingViewItem(-1)
    , m_crGrid(CR_INVALID)
    , SHostProxy(this)
{
    m_bFocusable = TRUE;
    m_bClipClient = TRUE;
    m_dwUpdateInterval = 40;
    m_nHeaderHeight.fromString(L"25dp");
    m_observer.Attach(new SMCListViewDataSetObserver(this));

    m_evtSet.addEvent(EVENTID(EventLVSelChanging));
    m_evtSet.addEvent(EVENTID(EventLVSelChanged));
}

SMCListView::~SMCListView()
{
    m_observer = NULL;
    m_lvItemLocator = NULL;
}

BOOL SMCListView::SetAdapter(IMcAdapter *adapter)
{
    if (!m_lvItemLocator)
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
    if (m_lvItemLocator)
        m_lvItemLocator->SetAdapter(adapter);
    if (m_adapter)
    {
        SXmlNode xmlNode = m_xmlTemplate.root().first_child();
        m_adapter->InitByTemplate(&xmlNode);
        m_adapter->registerDataSetObserver(m_observer);
        for (int i = 0; i < m_adapter->getViewTypeCount(); i++)
        {
            m_itemRecycle.Add(new SList<SItemPanel *>());
        }
        _UpdateAdapterColumnsWidth();
        onDataSetChanged();
    }
    return TRUE;
}

int SMCListView::InsertColumn(int nIndex, LPCTSTR pszText, int nWidth, UINT fmt, LPARAM lParam, BOOL bDpiAware /*=TRUE*/, float fWeight /*=0.0f*/)
{
    SASSERT(m_pHeader);
    int nRet = m_pHeader->InsertItem(nIndex, pszText, nWidth, fmt, lParam, bDpiAware, fWeight);
    UpdateScrollBar();
    return nRet;
}

BOOL SMCListView::CreateChildren(SXmlNode xmlNode)
{
    //  listctrl的子控件只能是一个header控件
    SXmlNode xmlTemplate = xmlNode.child(L"template");
    xmlTemplate.set_userdata(1);
    SXmlNode xmlHeader = xmlNode.child(L"headerStyle");
    xmlHeader.set_userdata(1);
    m_pHeader = sobj_cast<SHeaderCtrl>(CreateChildByName(xmlHeader.attribute(L"wndclass").as_string(SHeaderCtrl::GetClassName())));
    if (!m_pHeader)
        return FALSE;
    InsertChild(m_pHeader);
    m_pHeader->InitFromXml(&xmlHeader);

    if (!__baseCls::CreateChildren(xmlNode))
        return FALSE;
    if (xmlTemplate)
    {
        m_xmlTemplate.root().append_copy(xmlTemplate);
        SLayoutSize nItemHei = GETLAYOUTSIZE(xmlTemplate.attribute(L"itemHeight").value());
        if (nItemHei.fSize > 0.0f)
        { //指定了itemHeight属性时创建一个固定行高的定位器
            IListViewItemLocator *pItemLocator = new SListViewItemLocatorFix(nItemHei, m_nDividerSize);
            SetItemLocator(pItemLocator);
            pItemLocator->Release();
        }
        else
        { //创建一个行高可变的行定位器，从defHeight属性中获取默认行高
            IListViewItemLocator *pItemLocator = new SListViewItemLocatorFlex(GETLAYOUTSIZE(xmlTemplate.attribute(L"defHeight").as_string(L"30dp")), m_nDividerSize);
            SetItemLocator(pItemLocator);
            pItemLocator->Release();
        }
    }

    m_pHeader->GetEventSet()->subscribeEvent(EventHeaderItemChanging::EventID, Subscriber(&SMCListView::OnHeaderSizeChanging, this));
    m_pHeader->GetEventSet()->subscribeEvent(EventHeaderItemSwap::EventID, Subscriber(&SMCListView::OnHeaderSwap, this));
    m_pHeader->GetEventSet()->subscribeEvent(EventHeaderClick::EventID, Subscriber(&SMCListView::OnHeaderClick, this));

    return TRUE;
}

CRect SMCListView::GetListRect()
{
    CRect rcList;

    GetClientRect(&rcList);
    rcList.top += GetHeaderHeight();

    return rcList;
}

//  更新滚动条
void SMCListView::UpdateScrollBar()
{
    CSize szView;
    szView.cx = m_pHeader->GetTotalWidth(false);
    int nMinWid = m_pHeader->GetTotalWidth(true);
    szView.cy = m_lvItemLocator->GetTotalHeight();

    CRect rcClient;
    SWindow::GetClientRect(&rcClient); //不计算滚动条大小
    rcClient.top += GetHeaderHeight();
    if (rcClient.bottom < rcClient.top)
        rcClient.bottom = rcClient.top;
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

        int horzSize = size.cx - GetSbWidth();
        if (horzSize < nMinWid)
        {
            // 小于表头的最小宽度, 需要横向滚动条
            m_wBarVisible |= SSB_HORZ;
            m_siVer.nPage = size.cy - GetSbWidth() > 0 ? size.cy - GetSbWidth() : 0; //注意同时调整纵向滚动条page信息

            m_siHoz.nMin = 0;
            m_siHoz.nMax = szView.cx - 1;
            m_siHoz.nPage = (size.cx - GetSbWidth()) > 0 ? (size.cx - GetSbWidth()) : 0;
        }
        else
        {
            if (horzSize < szView.cx || m_pHeader->IsAutoResize())
            { //大于最小宽度，小于现在宽度，则调整表头的宽度。
                CRect rcHead = m_pHeader->GetWindowRect();
                rcHead.right = rcHead.left + horzSize;
                m_pHeader->Move(rcHead);
                szView.cx = horzSize;
            }
            //  不需要横向滚动条
            m_siHoz.nPage = szView.cx;
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

        if (size.cx < nMinWid)
        {
            //小于表头的最小宽度,  需要横向滚动条
            m_wBarVisible |= SSB_HORZ;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = szView.cx - 1;
            m_siHoz.nPage = size.cx;
        }
        else
        {
            if (size.cx < szView.cx || m_pHeader->IsAutoResize())
            { //大于最小宽度，小于现在宽度，则调整表头的宽度。
                CRect rcHead = m_pHeader->GetWindowRect();
                rcHead.right = rcHead.left + size.cx;
                m_pHeader->Move(rcHead);
                szView.cx = size.cx;
            }
            //  不需要横向滚动条
            m_siHoz.nPage = szView.cx;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = m_siHoz.nPage - 1;
            m_siHoz.nPos = 0;
        }
    }

    //  根据需要调整原点位置
    if (HasScrollBar(FALSE) && m_siHoz.nPos + m_siHoz.nPage > szView.cx)
    {
        m_siHoz.nPos = szView.cx - m_siHoz.nPage;
    }

    if (HasScrollBar(TRUE) && m_siVer.nPos + m_siVer.nPage > szView.cy)
    {
        m_siVer.nPos = szView.cy - m_siVer.nPage;
    }

    SetScrollPos(TRUE, m_siVer.nPos, TRUE);
    SetScrollPos(FALSE, m_siHoz.nPos, TRUE);

    //  重新计算客户区及非客户区
    SSendMessage(WM_NCCALCSIZE);

    Invalidate();
}

//更新表头位置
void SMCListView::UpdateHeaderCtrl()
{
    CRect rcClient;
    GetClientRect(&rcClient);
    CRect rcHeader(rcClient);
    rcHeader.bottom = rcHeader.top + GetHeaderHeight();
    rcHeader.left -= m_siHoz.nPos;
    if (m_pHeader)
        m_pHeader->Move(rcHeader);
}

void SMCListView::DeleteColumn(int iCol)
{
    if (m_pHeader->DeleteItem(iCol))
    {
        UpdateScrollBar();
    }
}

int SMCListView::GetColumnCount() const
{
    if (!m_pHeader)
        return 0;

    return m_pHeader->GetItemCount();
}

void SMCListView::UpdateChildrenPosition()
{
    __baseCls::UpdateChildrenPosition();
    UpdateHeaderCtrl();
}

int SMCListView::GetHeaderHeight() const
{
    return m_nHeaderHeight.toPixelSize(GetScale());
}

BOOL SMCListView::OnHeaderClick(IEvtArgs *pEvt)
{
    EventHeaderClick *pEvt2 = sobj_cast<EventHeaderClick>(pEvt);
    SASSERT(pEvt2);
    SHDITEM hi = { SHDI_ORDER | SHDI_FORMAT, 0 };
    UINT *pFmts = new UINT[m_pHeader->GetItemCount()];
    int *pOrders = new int[m_pHeader->GetItemCount()];
    int iCol = -1;
    for (int i = 0; i < m_pHeader->GetItemCount(); i++)
    {
        m_pHeader->GetItem(i, &hi);
        pFmts[hi.iOrder] = hi.fmt;
        pOrders[hi.iOrder] = i;
        if (i == pEvt2->iItem)
            iCol = hi.iOrder;
    }
    if (m_adapter && m_adapter->OnSort(iCol, pFmts, m_pHeader->GetItemCount()))
    {
        //更新表头的排序状态
        for (int i = 0; i < m_pHeader->GetItemCount(); i++)
        {
            m_pHeader->SetItemSort(pOrders[i], pFmts[i]);
        }
        onDataSetChanged();
    }
    delete[] pOrders;
    delete[] pFmts;
    return TRUE;
}

BOOL SMCListView::OnHeaderSizeChanging(IEvtArgs *pEvt)
{
    UpdateScrollBar();
    UpdateHeaderCtrl();
    if (!m_lvItemLocator->IsFixHeight())
    {
        _UpdateAdapterColumnsWidth();
        UpdateVisibleItems();
    }
    else
    {
        SPOSITION pos = m_lstItems.GetHeadPosition();
        while (pos)
        {
            ItemInfo ii = m_lstItems.GetNext(pos);
            CRect rcItem = ii.pItem->GetWindowRect();
            rcItem.right = m_pHeader->GetTotalWidth();
            ii.pItem->Move(rcItem);
            CRect rcSubItem(rcItem);
            rcSubItem.right = rcSubItem.left = 0;
            for (int i = 0; i < m_pHeader->GetItemCount(); i++)
            {
                SHDITEM hi = { SHDI_ORDER, 0 };
                m_pHeader->GetItem(i, &hi);
                rcSubItem.left = rcSubItem.right;
                rcSubItem.right += m_pHeader->GetItemWidth(i);
                SStringW strColName;
                m_adapter->GetColumnName(hi.iOrder, &strColName);
                SWindow *pCol = ii.pItem->FindChildByName(strColName);
                if (pCol)
                {
                    pCol->Move(rcSubItem);
                }
            }
            SASSERT(rcSubItem.right == m_pHeader->GetTotalWidth());
        }

        InvalidateRect(GetListRect());
    }
    return TRUE;
}

BOOL SMCListView::OnHeaderSwap(IEvtArgs *pEvt)
{
    OnHeaderSizeChanging(NULL);
    return TRUE;
}

void SMCListView::onDataSetChanged()
{
    if (!m_adapter)
        return;
    if (!IsVisible(TRUE))
    {
        m_bPendingUpdate = true;
        m_iPendingUpdateItem = -1;
        return;
    }

    //更新列显示状态
    m_pHeader->GetEventSet()->setMutedState(true);
    for (size_t i = 0; i < m_pHeader->GetItemCount(); i++)
    {
        int iCol = m_pHeader->GetOriItemIndex(i);
        m_pHeader->SetItemVisible(i, m_adapter->IsColumnVisible(iCol));
    }
    m_pHeader->GetEventSet()->setMutedState(false);

    if (m_lvItemLocator)
        m_lvItemLocator->OnDataSetChanged();
    if (m_iSelItem >= m_adapter->getCount())
        m_iSelItem = -1;

    UpdateScrollBar();
    UpdateVisibleItems();
}

void SMCListView::onDataSetInvalidated()
{
    m_bDatasetInvalidated = TRUE;
    Invalidate();
}

void SMCListView::onItemDataChanged(int iItem)
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
    if (m_lvItemLocator->IsFixHeight())
        UpdateVisibleItem(iItem);
    else
        UpdateVisibleItems();
}

void SMCListView::OnPaint(IRenderTarget *pRT)
{
    if (m_bDatasetInvalidated)
    {
        UpdateVisibleItems();
        m_bDatasetInvalidated = FALSE;
    }
    SPainter duiDC;
    BeforePaint(pRT, duiDC);

    float fMat[9];
    pRT->GetTransform(fMat);
    SMatrix mtx(fMat);

    int iFirst = m_iFirstVisible;
    if (iFirst != -1)
    {
        CRect rcClient;
        GetClientRect(&rcClient);
        rcClient.top += GetHeaderHeight();

        pRT->PushClipRect(&rcClient, RGN_AND);

        CRect rcClip, rcInter;
        SAutoRefPtr<IRegionS> rgnClip;
        pRT->GetClipBox(&rcClip);
        pRT->GetClipRegion(&rgnClip);

        SPOSITION pos = m_lstItems.GetHeadPosition();
        int i = 0;
        IRenderObj *oldPen = NULL;
        if (m_crGrid != CR_INVALID)
        {
            SAutoRefPtr<IPenS> pen;
            pRT->CreatePen(PS_SOLID, m_crGrid, 1, &pen);
            pRT->SelectObject(pen, &oldPen);
        }

        for (; pos; i++)
        {
            ItemInfo ii = m_lstItems.GetNext(pos);
            CRect rcItem = _OnItemGetRect(iFirst + i);
            if (SItemPanel::IsItemInClip(mtx, rcClip, rgnClip, rcItem))
                ii.pItem->Draw(pRT, rcItem);
            if (m_crGrid != CR_INVALID)
            {
                BOOL bAntiAlias = pRT->SetAntiAlias(FALSE);
                if (i == 0)
                {
                    POINT pts[2] = { { rcItem.left, rcItem.top }, { rcItem.right, rcItem.top } };
                    pRT->DrawLines(pts, 2);
                }
                POINT pts[2] = { { rcItem.left, rcItem.bottom - 1 }, { rcItem.right, rcItem.bottom - 1 } };
                pRT->DrawLines(pts, 2);
                pRT->SetAntiAlias(bAntiAlias);
            }
            rcItem.top = rcItem.bottom;
            rcItem.bottom += m_lvItemLocator->GetDividerSize();
            if (m_pSkinDivider && !rcItem.IsRectEmpty() && rgnClip->RectInRegion(&rcItem))
            { //绘制分隔线
                m_pSkinDivider->DrawByIndex(pRT, rcItem, 0);
            }
        }
        if (m_crGrid != CR_INVALID)
        {
            // draw vertical grid.
            BOOL bAntiAlias = pRT->SetAntiAlias(FALSE);

            CRect rcTop = _OnItemGetRect(iFirst);
            CRect rcBottom = _OnItemGetRect(iFirst + m_lstItems.GetCount() - 1);
            POINT pts[2] = { { rcTop.left, rcTop.top }, { rcTop.left, rcBottom.bottom } };
            pRT->DrawLines(pts, 2);
            pts[0].x--, pts[1].x--;
            for (int i = 0; i < m_pHeader->GetItemCount(); i++)
            {
                if (!m_pHeader->IsItemVisible(i))
                    continue;
                int wid = m_pHeader->GetItemWidth(i);
                pts[0].x += wid;
                pts[1].x += wid;
                pRT->DrawLines(pts, 2);
            }
            pRT->SetAntiAlias(bAntiAlias);

            pRT->SelectObject(oldPen, NULL);
        }
        pRT->PopClip();
    }
    AfterPaint(pRT, duiDC);
}

BOOL SMCListView::OnScroll(BOOL bVertical, UINT uCode, int nPos)
{
    int nOldPos = bVertical ? m_siVer.nPos : m_siHoz.nPos;
    __baseCls::OnScroll(bVertical, uCode, nPos);
    int nNewPos = bVertical ? m_siVer.nPos : m_siHoz.nPos;
    if (nOldPos != nNewPos)
    {
        if (bVertical)
            UpdateVisibleItems();
        else
            UpdateHeaderCtrl();
        //加速滚动时UI的刷新
        if (uCode == SB_THUMBTRACK)
            ScrollUpdate();

        return TRUE;
    }
    return FALSE;
}

void SMCListView::UpdateVisibleItems()
{
    if (!m_adapter)
        return;
    SAutoEnableHostPrivUiDef enableUiDef(this);
    int iOldFirstVisible = m_iFirstVisible;
    int iOldLastVisible = m_iFirstVisible + m_lstItems.GetCount();
    int nOldTotalHeight = m_lvItemLocator->GetTotalHeight();

    int iNewFirstVisible = m_lvItemLocator->Position2Item(m_siVer.nPos);
    int iNewLastVisible = iNewFirstVisible;
    int pos = m_lvItemLocator->Item2Position(iNewFirstVisible);
    int iHoverItem = m_pHoverItem ? (int)m_pHoverItem->GetItemIndex() : -1;
    m_pHoverItem = NULL;

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
            {                                                   // use the old visible item
                int iItem = iNewLastVisible - iOldFirstVisible; //(iNewLastVisible-iNewFirstVisible) +
                                                                //(iNewFirstVisible-iOldFirstVisible);
                SASSERT(iItem >= 0 && iItem <= (iOldLastVisible - iOldFirstVisible));
                if (pItemInfos[iItem].nType == ii.nType)
                { //类型相同才能重用
                    ii = pItemInfos[iItem];
                    pItemInfos[iItem].pItem = NULL; //标记该行已经被重用
                }
            }
            BOOL bNewItem = FALSE;
            if (!ii.pItem)
            { // create new visible item
                SList<SItemPanel *> *lstRecycle = m_itemRecycle.GetAt(ii.nType);
                if (lstRecycle->IsEmpty())
                { //创建一个新的列表项
                    bNewItem = TRUE;
                    ii.pItem = SItemPanel::Create(this, SXmlNode(), this);
                    ii.pItem->GetEventSet()->subscribeEvent(EventItemPanelClick::EventID, Subscriber(&SMCListView::OnItemClick, this));
                }
                else
                {
                    ii.pItem = lstRecycle->RemoveHead();
                }
                ii.pItem->SetItemIndex(iNewLastVisible);
            }
            ii.pItem->SetVisible(TRUE);
            CRect rcItem(0, 0, m_pHeader->GetTotalWidth(), 100000);
            if (m_lvItemLocator->IsFixHeight())
            {
                rcItem.bottom = m_lvItemLocator->GetItemHeight(iNewLastVisible);
                ii.pItem->Move(rcItem);
            }

            //设置状态，同时暂时禁止应用响应statechanged事件。
            ii.pItem->GetEventSet()->setMutedState(true);
            ii.pItem->ModifyItemState(dwState, 0);
            ii.pItem->GetEventSet()->setMutedState(false);
            if (dwState & WndState_Hover)
                m_pHoverItem = ii.pItem;

            //应用可以根据ii.pItem的状态来决定如何初始化列表数据
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

            if (!m_lvItemLocator->IsFixHeight())
            { //计算出列表行高度
                SIZE szView;
                m_adapter->getViewDesiredSize(&szView, iNewLastVisible, ii.pItem, rcItem.Width(), rcItem.Height());
                m_lvItemLocator->SetItemHeight(iNewLastVisible, szView.cy);
                rcItem.bottom = szView.cy;
                ii.pItem->Move(rcItem);
            }
            ii.pItem->UpdateLayout();

            //调整网格大小
            CRect rcSubItem(rcItem);
            rcSubItem.right = rcSubItem.left;
            for (int i = 0; i < m_pHeader->GetItemCount(); i++)
            {
                SHDITEM hditem = { SHDI_ORDER | SHDI_WIDTH, 0 };
                m_pHeader->GetItem(i, &hditem);
                SStringW strColName;
                m_adapter->GetColumnName(hditem.iOrder, &strColName);
                SWindow *pColWnd = ii.pItem->FindChildByName(strColName);
                SASSERT(pColWnd);
                if (m_pHeader->IsItemVisible(i))
                {
                    pColWnd->SetVisible(true);
                    rcSubItem.left = rcSubItem.right;
                    rcSubItem.right += hditem.cx;
                    pColWnd->Move(rcSubItem);
                }
                else
                {
                    pColWnd->SetVisible(false);
                }
            }

            m_lstItems.AddTail(ii);
            pos += rcItem.bottom + m_lvItemLocator->GetDividerSize();

            iNewLastVisible++;
        }
    }

    // move old visible items which were not reused to recycle
    for (int i = 0; i < (iOldLastVisible - iOldFirstVisible); i++)
    {
        ItemInfo ii = pItemInfos[i];
        if (!ii.pItem)
            continue;

        if (ii.pItem->GetState() & WndState_Hover)
        {
            ii.pItem->DoFrameEvent(WM_MOUSELEAVE, 0, 0);
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

    if (!m_lvItemLocator->IsFixHeight() && m_lvItemLocator->GetTotalHeight() != nOldTotalHeight)
    { // update scroll range
        UpdateScrollBar();
        UpdateHeaderCtrl();
        UpdateVisibleItems(); //根据新的滚动条状态重新记录显示列表项
    }
    else
    {
        InvalidateRect(NULL);
    }
}

void SMCListView::UpdateVisibleItem(int iItem)
{
    SAutoEnableHostPrivUiDef enableUiDef(this);
    SASSERT(m_lvItemLocator->IsFixHeight());
    SItemPanel *pItem = GetItemPanel(iItem);
    SASSERT(pItem);
    SXmlNode xmlNode = m_xmlTemplate.root().first_child();
    m_adapter->getView(iItem, pItem, &xmlNode);
    pItem->Invalidate();
}

void SMCListView::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);
    UpdateScrollBar();
    UpdateHeaderCtrl();
    _UpdateAdapterColumnsWidth();
    UpdateVisibleItems();
}

void SMCListView::OnDestroy()
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

BOOL SMCListView::IsItemRedrawDelay() const
{
    return TRUE;
}

BOOL SMCListView::IsTimelineEnabled() const
{
    return !m_sbHorz.IsThumbTracking() && !m_sbVert.IsThumbTracking();
}

BOOL SMCListView::OnItemGetRect(const SOsrPanel *pItem, CRect &rcItem) const
{
    int iPosition = (int)pItem->GetItemIndex();
    if (iPosition < 0 || iPosition >= m_adapter->getCount())
        return FALSE;
    rcItem = _OnItemGetRect(iPosition);
    return TRUE;
}

CRect SMCListView::_OnItemGetRect(int iPosition) const
{
    int nOffset = m_lvItemLocator->Item2Position(iPosition) - m_siVer.nPos;
    CRect rcItem = GetClientRect();
    rcItem.top += GetHeaderHeight() + nOffset;
    rcItem.bottom = rcItem.top + m_lvItemLocator->GetItemHeight(iPosition);
    rcItem.left -= m_siHoz.nPos;
    rcItem.right = rcItem.left + m_pHeader->GetTotalWidth();
    return rcItem;
}

void SMCListView::OnItemSetCapture(SOsrPanel *pItem, BOOL bCapture)
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

void SMCListView::RedrawItem(SOsrPanel *pItem)
{
    pItem->InvalidateRect(NULL);
}

SItemPanel *SMCListView::HitTest(CPoint &pt) const
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

IItemPanel *SMCListView::HitTest(const POINT *pt) const
{
    SASSERT(pt);
    if (!pt)
        return NULL;
    CPoint pt2(*pt);
    return HitTest(pt2);
}

LRESULT SMCListView::OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
        else if (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN)
        {
            // 点击空白区域取消选中
            SetSel(-1, TRUE);
        }
    }

    if (uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONUP || uMsg == WM_MBUTTONUP)
    { //交给panel处理
        __baseCls::ProcessSwndMessage(uMsg, wParam, lParam, lRet);
    }
    SetMsgHandled(TRUE);
    return 0;
}

LRESULT SMCListView::OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

void SMCListView::OnMouseLeave()
{
    __baseCls::OnMouseLeave();
    if (m_pHoverItem)
    {
        m_pHoverItem->DoFrameEvent(WM_MOUSELEAVE, 0, 0);
        m_pHoverItem = NULL;
    }
}

void SMCListView::OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags)
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
                return;
        }
    }

    int nNewSelItem = -1;
    SWindow *pOwner = GetOwner();
    if (pOwner && (nChar == VK_ESCAPE || nChar == VK_RETURN))
    {
        pOwner->SSendMessage(WM_KEYDOWN, nChar, MAKELONG(nFlags, nRepCnt));
        return;
    }

    if (nChar == VK_DOWN && m_iSelItem < m_adapter->getCount() - 1)
        nNewSelItem = m_iSelItem + 1;
    else if (nChar == VK_UP && m_iSelItem > 0)
        nNewSelItem = m_iSelItem - 1;
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
                nNewSelItem = (int)m_lstItems.GetHead().pItem->GetItemIndex();
            }
        }
        else if (nChar == VK_NEXT || nChar == VK_END)
        {
            if (!m_lstItems.IsEmpty())
            {
                nNewSelItem = (int)m_lstItems.GetTail().pItem->GetItemIndex();
            }
        }
    }

    if (nNewSelItem != -1)
    {
        EnsureVisible(nNewSelItem);
        // 取消选择通知
        SetSel(nNewSelItem, TRUE);
    }
    else
    {
        SetMsgHandled(FALSE);
    }
}

void SMCListView::EnsureVisible(int iItem)
{
    if (iItem < 0 || iItem >= m_adapter->getCount())
        return;
    if (!IsVisible(TRUE))
    {
        m_iPendingViewItem = iItem;
        return;
    }

    int iFirstVisible = m_iFirstVisible;
    int iLastVisible = m_iFirstVisible + m_lstItems.GetCount();

    if (iItem >= iFirstVisible && iItem < iLastVisible)
    {
        if (iItem == iFirstVisible)
        {
            int pos = m_lvItemLocator->Item2Position(iItem);
            OnScroll(TRUE, SB_THUMBPOSITION, pos);
        }
        else if (iItem == iLastVisible - 1)
        {
            if (iItem == m_adapter->getCount() - 1)
                OnScroll(TRUE, SB_BOTTOM, 0);
            else
            {
                int pos = m_lvItemLocator->Item2Position(iItem + 1) - m_siVer.nPage;
                OnScroll(TRUE, SB_THUMBPOSITION, pos);
            }
        }

        return;
    }

    if (iItem < iFirstVisible)
    { // scroll up
        int pos = m_lvItemLocator->Item2Position(iItem);
        OnScroll(TRUE, SB_THUMBPOSITION, pos);
    }
    else // if(iItem >= iLastVisible)
    {    // scroll down
        if (iItem == m_adapter->getCount() - 1)
        {
            OnScroll(TRUE, SB_BOTTOM, 0);
        }
        else
        {
            int pos = m_lvItemLocator->Item2Position(iItem + 1) - m_siVer.nPage;
            OnScroll(TRUE, SB_THUMBPOSITION, pos);
        }
    }
    if (!m_lvItemLocator->IsFixHeight())
    {
        int pos = m_lvItemLocator->Item2Position(iItem);
        OnScroll(TRUE, SB_THUMBPOSITION, pos);
    }
}

BOOL SMCListView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    SItemPanel *pSelItem = GetItemPanel(m_iSelItem);
    if (pSelItem)
    {
        CRect rcItem = pSelItem->GetItemRect();
        CPoint pt2 = pt - rcItem.TopLeft();
        if (pSelItem->DoFrameEvent(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt2.x, pt2.y)))
            return TRUE;
    }
    return __baseCls::OnMouseWheel(nFlags, zDelta, pt);
}

int SMCListView::GetScrollLineSize(BOOL bVertical)
{
    return m_lvItemLocator->GetScrollLineSize();
}

SItemPanel *SMCListView::GetItemPanel(int iItem)
{
    if (!m_adapter || iItem < 0 || iItem >= m_adapter->getCount())
        return NULL;
    SPOSITION pos = m_lstItems.GetHeadPosition();
    while (pos)
    {
        ItemInfo ii = m_lstItems.GetNext(pos);
        if ((int)ii.pItem->GetItemIndex() == iItem)
            return ii.pItem;
    }
    return NULL;
}

void SMCListView::SetItemLocator(IListViewItemLocator *pItemLocator)
{
    m_lvItemLocator = pItemLocator;
    if (m_lvItemLocator)
    {
        m_lvItemLocator->SetAdapter(GetAdapter());
        m_lvItemLocator->SetScale(GetScale());
    }
    onDataSetChanged();
}

BOOL SMCListView::UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
{
    if (!m_pHoverItem)
        return __baseCls::UpdateToolTip(pt, tipInfo);
    return m_pHoverItem->UpdateToolTip(pt, tipInfo);
}

void SMCListView::SetSel(int iItem, BOOL bNotify)
{
    if (!m_adapter)
        return;

    if (iItem >= m_adapter->getCount())
        return;

    if (iItem < 0)
        iItem = -1;

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
        { // Cancel SetSel and restore selection state
            m_iSelItem = nOldSel;
            return;
        }
    }

    if (nOldSel == nNewSel)
        return;

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

UINT SMCListView::OnGetDlgCode() const
{
    if (m_bWantTab)
        return SC_WANTALLKEYS;
    else
        return SC_WANTARROWS | SC_WANTSYSKEY;
}

void SMCListView::OnKillFocus(SWND wndFocus)
{
    __baseCls::OnKillFocus(wndFocus);

    if (m_iSelItem == -1)
        return;

    SItemPanel *pSelPanel = GetItemPanel(m_iSelItem);
    if (pSelPanel)
        pSelPanel->GetFocusManager()->StoreFocusedView();
}

void SMCListView::OnSetFocus(SWND wndOld)
{
    __baseCls::OnSetFocus(wndOld);
    if (m_iSelItem == -1)
        return;

    SItemPanel *pSelPanel = GetItemPanel(m_iSelItem);
    if (pSelPanel)
        pSelPanel->GetFocusManager()->RestoreFocusedView();
}

BOOL SMCListView::OnSetCursor(const CPoint &pt)
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

BOOL SMCListView::OnItemClick(IEvtArgs *pEvt)
{
    SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pEvt->Sender());
    int iItem = (int)pItemPanel->GetItemIndex();
    if (iItem != m_iSelItem)
    {
        SetSel(iItem, TRUE);
    }
    return TRUE;
}

void SMCListView::OnColorize(COLORREF cr)
{
    __baseCls::OnColorize(cr);
    DispatchMessage2Items(UM_SETCOLORIZE, cr, 0);
    SPOSITION pos = m_lstItems.GetHeadPosition();
    while (pos)
    {
        ItemInfo ii = m_lstItems.GetNext(pos);
        ii.pItem->DoColorize(cr);
    }
}

void SMCListView::OnScaleChanged(int nScale)
{
    __baseCls::OnScaleChanged(nScale);
    if (m_lvItemLocator)
        m_lvItemLocator->SetScale(nScale);
    DispatchMessage2Items(UM_SETSCALE, nScale, 0);
    UpdateVisibleItems();
}

HRESULT SMCListView::OnLanguageChanged()
{
    HRESULT hret = __baseCls::OnLanguageChanged();
    DispatchMessage2Items(UM_SETLANGUAGE, 0, 0);
    return hret;
}

void SMCListView::DispatchMessage2Items(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

void SMCListView::OnShowWindow(BOOL bShow, UINT nStatus)
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

SHeaderCtrl *SMCListView::GetHeaderCtrl() const
{
    return m_pHeader;
}

void SMCListView::OnRebuildFont()
{
    __baseCls::OnRebuildFont();
    DispatchMessage2Items(UM_UPDATEFONT, 0, 0);
}

int SMCListView::GetSel() const
{
    return m_iSelItem;
}

IListViewItemLocator *SMCListView::GetItemLocator() const
{
    return m_lvItemLocator;
}

IMcAdapter *SMCListView::GetAdapter() const
{
    return m_adapter;
}

IHeaderCtrl *SMCListView::GetIHeaderCtrl() const
{
    return GetHeaderCtrl();
}

void SMCListView::GetDesiredSize(THIS_ SIZE *psz, int nParentWid, int nParentHei)
{
    __baseCls::GetDesiredSize(psz, nParentWid, nParentHei);
    ILayoutParam *pLayoutParam = GetLayoutParam();
    if (pLayoutParam->IsWrapContent(Vert) && m_lvItemLocator && m_lvItemLocator->IsFixHeight())
    {
        CRect rcPadding = GetStyle().GetPadding();
        psz->cy = m_lvItemLocator->GetTotalHeight() + rcPadding.top + rcPadding.bottom;
        if (nParentHei > 0 && psz->cy > nParentHei)
            psz->cy = nParentHei;
    }
}

void SMCListView::_UpdateAdapterColumnsWidth() const
{
    if (!m_lvItemLocator->IsFixHeight() && m_adapter)
    {
        int *nWids = new int[m_pHeader->GetItemCount()];
        for (int i = 0; i < m_pHeader->GetItemCount(); i++)
        {
            SHDITEM hi = { SHDI_ORDER, 0 };
            m_pHeader->GetItem(i, &hi);
            nWids[hi.iOrder] = m_pHeader->GetItemWidth(i);
        }
        m_adapter->SetColumnsWidth(nWids, m_pHeader->GetItemCount());
        delete[] nWids;
    }
}

SNSEND
