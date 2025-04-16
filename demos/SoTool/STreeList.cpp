#include "stdafx.h"
#include "STreeList.h"

namespace SOUI
{
    SMCTreeCtrl::SMCTreeCtrl():m_nTreeWidth(100)
    {
    }

    SMCTreeCtrl::~SMCTreeCtrl()
    {

    }

    void SMCTreeCtrl::OnFinalRelease()
    {
        delete this;
    }

    void SMCTreeCtrl::OnNodeFree(LPTVITEM & pItemData)
    {
		if (m_pListener)
		{
			m_pListener->OnDeleteItem(this, pItemData->hItem, pItemData->lParam);
		}
		MCITEM *pMcItem = (MCITEM*)pItemData->lParam;
		delete pMcItem;
		delete pItemData;
    }

    void SMCTreeCtrl::OnInsertItem(LPTVITEM & pItemData)
    {
        MCITEM *pMcItem = new MCITEM;
        pMcItem->arrText.SetCount(m_arrColWidth.GetCount());
        pItemData->lParam = (LPARAM)pMcItem;
		if (m_pListener)
		{
			m_pListener->OnInsertItem(this, pItemData->hItem);
		}
    }

    int SMCTreeCtrl::CalcItemWidth(const LPTVITEM pItem)
    {
		return m_nTreeWidth;
    }

	void SMCTreeCtrl::UpdateContentWidth()
	{
		m_nContentWidth = m_nTreeWidth;
		for(UINT i=0;i<m_arrColWidth.GetCount();i++)
			m_nContentWidth += m_arrColWidth[i];
	}

    int SMCTreeCtrl::InsertColumn(int iCol,int nWid)
    {
        if(iCol < 0) iCol = m_arrColWidth.GetCount();
        m_arrColWidth.InsertAt(iCol,nWid);
        
        HSTREEITEM hItem = GetRootItem();
        while(hItem)
        {
            MCITEM *pMcItem = (MCITEM*)STreeCtrl::GetItemData(hItem);
            pMcItem->arrText.InsertAt(iCol,SStringT());
            hItem = GetNextItem(hItem);
        }
        UpdateContentWidth();
		UpdateScrollBar();
        Invalidate();
        return iCol;
    }

    BOOL SMCTreeCtrl::DeleteColumn(int iCol)
    {
        if(iCol<0 || iCol>=(int)m_arrColWidth.GetCount()) 
            return FALSE;
        HSTREEITEM hItem = GetRootItem();
        while(hItem)
        {
            MCITEM *pMcItem = (MCITEM*)STreeCtrl::GetItemData(hItem);
            pMcItem->arrText.RemoveAt(iCol);
            hItem = GetNextItem(hItem);
        }
        m_arrColWidth.RemoveAt(iCol);
        
		UpdateContentWidth();
		UpdateScrollBar();
		Invalidate();
        return TRUE;
    }


    BOOL SMCTreeCtrl::SetItemText(HSTREEITEM hItem,int iCol,const SStringT strText)
    {
        if(iCol<0 || iCol>=(int)m_arrColWidth.GetCount()) 
            return FALSE;
            
        MCITEM *pMcItem = (MCITEM*)STreeCtrl::GetItemData(hItem);
        SASSERT(pMcItem);
        pMcItem->arrText.SetAt(iCol,strText);
        return TRUE;
    }

    void SMCTreeCtrl::DrawItem(IRenderTarget *pRT,const CRect & rc, HSTREEITEM hItem)
    {
        CRect rcTreeItem = rc;
        rcTreeItem.right = rcTreeItem.left + m_nTreeWidth;
        DrawTreeItem(pRT,rcTreeItem,hItem);
        
        CRect rcListItem = rc;
        rcListItem.left = rcTreeItem.right;
        DrawListItem(pRT,rcListItem,hItem);
    }

    void SMCTreeCtrl::DrawTreeItem(IRenderTarget *pRT, CRect & rc,HSTREEITEM hItem)
    {
        STreeCtrl::DrawItem(pRT,rc,hItem);
    }

    void SMCTreeCtrl::DrawListItem(IRenderTarget *pRT, CRect & rc,HSTREEITEM hItem)
    {
        MCITEM *pMcItem =(MCITEM*)STreeCtrl::GetItemData(hItem);
        CRect rcItem = rc;
        rcItem.right=rcItem.left;
        
        for(UINT i=0;i<m_arrColWidth.GetCount();i++)
        {
            rcItem.left = rcItem.right;
            rcItem.right += m_arrColWidth[i];
            pRT->DrawText(pMcItem->arrText[i],pMcItem->arrText[i].GetLength(),rcItem,DT_SINGLELINE|DT_LEFT|DT_VCENTER);
        }
    }

    BOOL SMCTreeCtrl::SetColWidth(int iCol,int nWid)
    {
        if(iCol<0 || iCol>=(int)m_arrColWidth.GetCount()) 
            return FALSE;
        m_arrColWidth.SetAt(iCol,nWid);
		UpdateContentWidth();
		UpdateScrollBar();
		Invalidate();

        return TRUE;
    }

    void SMCTreeCtrl::SetTreeWidth(int nWid)
    {
        m_nTreeWidth = nWid;
		UpdateContentWidth();
		UpdateScrollBar();
		Invalidate();
    }

    BOOL SMCTreeCtrl::SetItemData( HSTREEITEM hItem, LPARAM lParam )
    {
        MCITEM *pData = (MCITEM*)STreeCtrl::GetItemData(hItem);
        pData->lParam = lParam;
		return TRUE;
    }

    LPARAM SMCTreeCtrl::GetItemData( HSTREEITEM hItem ) const
    {
        MCITEM *pData = (MCITEM*)STreeCtrl::GetItemData(hItem);
        return pData->lParam;
    }

    HSTREEITEM SMCTreeCtrl::InsertItem( LPCTSTR pszText,int iImage,int iSelImage,LPARAM lParam,HSTREEITEM hParent/*=STVI_ROOT*/,HSTREEITEM hAfter /*= STVI_LAST*/,BOOL bEnsureVisible/*=FALSE*/ )
    {
        MCITEM *pData = new MCITEM;
        pData->arrText.SetCount(m_arrColWidth.GetCount());
        pData->lParam = lParam;
        HSTREEITEM hRet = STreeCtrl::InsertItem(pszText,iImage,iSelImage,(LPARAM)pData,hParent,hAfter);
		if(hRet && bEnsureVisible)
		{
			STreeCtrl::EnsureVisible(hRet);
		}
		return hRet;
    }

	CPoint SMCTreeCtrl::GetViewOrigin() const
	{
		CPoint pt;
		pt.x = m_siHoz.nPos;
		pt.y = m_siVer.nPos;
		return pt;
	}

	CSize SMCTreeCtrl::GetViewSize() const
	{
		CSize sz;
		sz.cx = m_siHoz.nMax;
		sz.cy = m_siVer.nMax;
		return sz;
	}

    //////////////////////////////////////////////////////////////////////////
    STreeList::STreeList(void)
    :m_nHeaderHeight(20)
    ,m_pHeader(NULL)
    ,m_pTreeCtrl(NULL)
    {
        m_bClipClient = TRUE;
    }

    STreeList::~STreeList(void)
    {
    }

    BOOL STreeList::CreateChildren(SXmlNode xmlNode)
    {
        SXmlNode xmlHeader = xmlNode.child(L"headerstyle");
        SXmlNode xmlTreectrl = xmlNode.child(L"treectrlstyle");
        if(!xmlHeader || !xmlTreectrl) 
            return FALSE;
        m_pHeader = CreateHeader();
        InsertChild(m_pHeader);
        m_pHeader->InitFromXml(&xmlHeader);
        m_pHeader->GetEventSet()->subscribeEvent(EventHeaderItemChanging::EventID, Subscriber(&STreeList::OnHeaderSizeChanging,this));
        m_pHeader->GetEventSet()->subscribeEvent(EventHeaderItemSwap::EventID, Subscriber(&STreeList::OnHeaderSwap,this));

        m_pTreeCtrl = CreateMcTreeCtrl();
        InsertChild(m_pTreeCtrl);
        m_pTreeCtrl->InitFromXml(&xmlTreectrl);
        m_pTreeCtrl->GetEventSet()->subscribeEvent(EventScroll::EventID,Subscriber(&STreeList::OnScrollEvent,this));

        m_pHeader->InsertItem(0,m_strTreeLabel.GetText(),m_pTreeCtrl->m_nTreeWidth,HDF_CENTER,0);
        for(UINT i=1;i<m_pHeader->GetItemCount();i++)
        {
            int nWid = m_pHeader->GetItemWidth(i);
            m_pTreeCtrl->InsertColumn(i-1,nWid);
        }
        return TRUE;
    }

    BOOL STreeList::OnRelayout(const CRect &rcWnd)
    {
		if (!__baseCls::OnRelayout(rcWnd))
			return FALSE;
        SASSERT(m_pHeader && m_pTreeCtrl);
        CRect rcTreectrl = rcWnd;
        rcTreectrl.top = rcWnd.top + m_nHeaderHeight;
        m_pTreeCtrl->Move(rcTreectrl);
        
        CPoint ptOrg = m_pTreeCtrl->GetViewOrigin();
        CRect rcHeader = rcWnd;
        rcHeader.left -= ptOrg.x;
        rcHeader.right =rcHeader.left + smax(rcWnd.Width(),m_pTreeCtrl->GetViewSize().cx);
        rcHeader.bottom = rcWnd.top + m_nHeaderHeight;
        m_pHeader->Move(rcHeader);
		return TRUE;
    }

    BOOL STreeList::OnHeaderClick(EventArgs *pEvt)
    {
        return TRUE;
    }

    BOOL STreeList::OnHeaderSizeChanging(EventArgs *pEvt)
    {
        EventHeaderItemChanging *pEvt2=sobj_cast<EventHeaderItemChanging>(pEvt);
        m_pTreeCtrl->GetEventSet()->setMutedState(true);
        if(pEvt2->iItem == 0)
            m_pTreeCtrl->SetTreeWidth(pEvt2->nWidth);
        else
            m_pTreeCtrl->SetColWidth(pEvt2->iItem-1,pEvt2->nWidth);
        m_pTreeCtrl->GetEventSet()->setMutedState(false);
        return true;
    }

    BOOL STreeList::OnHeaderSwap(EventArgs *pEvt)
    {
        return true;
    }

    BOOL STreeList::OnScrollEvent( EventArgs *pEvt )
    {
        EventScroll *pEvt2 = sobj_cast<EventScroll>(pEvt);
        SASSERT(pEvt2);

        if(!pEvt2->bVertical)
        {
            CRect rcClient=GetClientRect();
            CSize szView = m_pTreeCtrl->GetViewSize();

            CRect rcHeader = rcClient;
            rcHeader.bottom=rcHeader.top + m_nHeaderHeight;
            rcHeader.right =rcHeader.left + smax(szView.cx,rcClient.Width());
            CPoint ptOrg = m_pTreeCtrl->GetViewOrigin();
            rcHeader.OffsetRect(-ptOrg.x,0);
            m_pHeader->Move(rcHeader);
        }

        return FALSE;
    }

    SHeaderCtrl * STreeList::CreateHeader()
    {
        return (SHeaderCtrl*)SApplication::getSingleton().CreateWindowByName(SHeaderCtrl::GetClassName());
    }

    SMCTreeCtrl * STreeList::CreateMcTreeCtrl()
    {
        return new SMCTreeCtrl;
    }
}
