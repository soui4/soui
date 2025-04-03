#include "stdafx.h"
#include "SToolBar.h"
#include <helper/SMenu.h>

namespace SOUI
{
	SToolBar::SToolBar(void)
		:m_bShowText(TRUE)
		,m_nItemSize(-1)
		,m_bVert(FALSE)
		,m_iClickItem(-1)
		,m_iHoverItem(-1)
		,m_nVisibleItems(0)
		,m_nDropDownSize(15)
		,m_dwDropBtnState(0)
	{
		GetEventSet()->addEvent(EVENTID(EventToolBarCmd));
	}

	SToolBar::~SToolBar(void)
	{
	}


	BOOL SToolBar::CreateChildren(pugi::xml_node xmlNode)
	{
		pugi::xml_node xmlItem = xmlNode.child(L"data").first_child();
		while(xmlItem)
		{
			if(_wcsicmp(xmlItem.name(),L"item")==0)
			{//add item
				ToolBarItem item;
				item.iIcon = xmlItem.attribute(L"iconIndex").as_int(0);
				item.nId = xmlItem.attribute(L"id").as_int(0);
				item.lParam = xmlItem.attribute(L"data").as_uint(0);
				item.strText = S_CW2T(xmlItem.attribute(L"text").as_string());
				item.strTip = S_CW2T(xmlItem.attribute(L"tip").as_string());
				item.dwState = xmlItem.attribute(L"disable").as_bool(false)?WndState_Disable:WndState_Normal;
				if(xmlItem.attribute(L"checked").as_bool(false))
					item.dwState|=WndState_Check;
				m_arrItems.Add(item);
			}else if(_wcsicmp(xmlItem.name(),L"sep")==0)
			{
				ToolBarItem item;
				item.nId = -1;
				m_arrItems.Add(item);
			}
			xmlItem=xmlItem.next_sibling();
		}
		if(xmlNode.child(L"menuStyle"))
		{
			m_menuStyle.root().append_copy(xmlNode.child(L"menuStyle"));
		}
		return TRUE;
	}

	SIZE SToolBar::MeasureContent(int nParentWid,int nParentHei)
	{		
		CSize szItem;
		if(m_skinState) 
			szItem = m_skinState->GetSkinSize();
		int nSep = GetSepWid();
		int nSepCount=0;
		for(UINT i=0;i<m_arrItems.GetCount();i++)
			if(IsSeparator(&m_arrItems[i])) nSepCount++;
		int nItemCount = m_arrItems.GetCount() - nSepCount;

		CSize szContent;
		if(m_bVert)
		{
			szContent.cx = szItem.cx;
			szContent.cy = nItemCount*szItem.cy + nSep*nSepCount;
		}else
		{
			szContent.cx = nItemCount*szItem.cx + nSep*nSepCount;
			szContent.cy = szItem.cy;
		}
		return szContent;
	}

	CSize SToolBar::GetItemSize() const
	{
		CRect rc = GetClientRect();
		CSize szState;
		if(m_skinState) szState = m_skinState->GetSkinSize();
		if(m_bVert)
		{
			int nItemHei = m_nItemSize>0?m_nItemSize:szState.cy;
			return CSize(nItemHei,rc.Width());
		}else
		{
			int nItemWid = m_nItemSize>0?m_nItemSize:szState.cx;
			return CSize(rc.Height(),nItemWid);
		}
	}

	int SToolBar::GetSepWid() const
	{
		if(!m_skinSep)
			return 2;
		CSize szSep = m_skinSep->GetSkinSize();
		return m_bVert?szSep.cy:szSep.cx;
	}


	BOOL SToolBar::IsSeparator(const ToolBarItem * pItem) const
	{
		return pItem->nId<0;
	}

	CRect SToolBar::GetItemRect(int iItem) const
	{
		if(iItem<0 || iItem >= m_arrItems.GetCount())
			return CRect();
		CSize szItem = GetItemSize();
		int   nSep = GetSepWid();
		CRect rc = GetClientRect();
		if(m_bVert)
		{
			rc.bottom=rc.top;
		}else
		{
			rc.right = rc.left;
		}
		for(int i=0;i<=iItem;i++)
		{
			if(IsSeparator(&m_arrItems[i]))
			{
				if(m_bVert)
				{
					rc.top = rc.bottom;
					rc.bottom+=nSep;
				}else
				{
					rc.left = rc.right;
					rc.right+= nSep;
				}
			}else
			{
				if(m_bVert)
				{
					rc.top = rc.bottom;
					rc.bottom += szItem.cy;
				}else
				{
					rc.left = rc.right;
					rc.right += szItem.cx;
				}
			}
		}
		return rc;
	}

	int SToolBar::HitTest(CPoint pt) const
	{
		CSize szItem = GetItemSize();
		int   nSep = GetSepWid();
		CRect rc = GetClientRect();
		if(m_bVert)
		{
			rc.bottom=rc.top;
		}else
		{
			rc.right = rc.left;
		}
		for(int i=0;i<m_nVisibleItems;i++)
		{
			if(IsSeparator(&m_arrItems[i]))
			{
				if(m_bVert)
				{
					rc.top = rc.bottom;
					rc.bottom+=nSep;
				}else
				{
					rc.left = rc.right;
					rc.right+= nSep;
				}
			}else
			{
				if(m_bVert)
				{
					rc.top = rc.bottom;
					rc.bottom += szItem.cy;
				}else
				{
					rc.left = rc.right;
					rc.right += szItem.cx;
				}
			}
			if(rc.PtInRect(pt))
			{
				if(IsSeparator(&m_arrItems[i]))
					return -1;
				else
					return i;
			}
		}
		return -1;
	}


	void SToolBar::DrawItem(IRenderTarget *pRT,const CRect &rcItem,const ToolBarItem * pItem)
	{
		pRT->PushClipRect(rcItem,RGN_AND);
		if(IsSeparator(pItem))
		{
			if(m_skinSep)
				m_skinSep->DrawByIndex(pRT,rcItem,0);
		}else
		{
			if(m_skinState) 
				m_skinState->DrawByState(pRT,rcItem,pItem->dwState);

			CSize szIcon;
			if(pItem->icon)
				szIcon = pItem->icon->Size();
			else if(m_skinIcons)
				szIcon = m_skinIcons->GetSkinSize();
			if(m_bShowText)
			{
				int iState = SState2Index::GetDefIndex(pItem->dwState,true);
				COLORREF crTxt = GetStyle().GetTextColor(iState);
				COLORREF crOld = pRT->GetTextColor();
				if(crTxt != CR_INVALID)
					pRT->SetTextColor(crTxt);

				CSize szTxt ;
				pRT->MeasureText(pItem->strText,pItem->strText.GetLength(),&szTxt);
				int nInter = (rcItem.Height()-szIcon.cy-szTxt.cy)/3;
				CRect rcIcon(rcItem.CenterPoint(),szIcon);
				rcIcon.OffsetRect(-szIcon.cx/2,0);
				rcIcon.MoveToY(rcItem.top + nInter);
				if(pItem->icon)
					pRT->DrawBitmapEx(rcIcon,pItem->icon,CRect(CPoint(),szIcon),EM_STRETCH,0xff);
				else if(m_skinIcons && pItem->iIcon>=0)
					m_skinIcons->DrawByIndex(pRT,rcIcon,pItem->iIcon);

				CRect rcTxt(rcItem.CenterPoint(),szTxt);
				rcTxt.OffsetRect(-szTxt.cx/2,0);
				rcTxt.MoveToY(rcIcon.bottom+nInter);
				pRT->TextOut(rcTxt.left,rcTxt.top,pItem->strText.c_str(),pItem->strText.GetLength());
				pRT->SetTextColor(crOld);
			}else
			{
				CRect rcIcon(rcItem.CenterPoint(),szIcon);
				rcIcon.OffsetRect(-szIcon.cx/2,-szIcon.cy/2);
				if(pItem->icon)
					pRT->DrawBitmapEx(rcIcon,pItem->icon,CRect(CPoint(),szIcon),EM_STRETCH,0xff);
				else if(m_skinIcons && pItem->iIcon>=0)
					m_skinIcons->DrawByIndex(pRT,rcIcon,pItem->iIcon);
			}
		}
		pRT->PopClip();
	}

	void SToolBar::OnPaint(IRenderTarget *pRT)
	{
		SPainter painter;
		BeforePaint(pRT,painter);
		CSize szItem = GetItemSize();
		CRect rcItem(GetClientRect().TopLeft(),szItem);
		int nSep = GetSepWid();
		if(m_bVert)
			rcItem.bottom=rcItem.top;
		else
			rcItem.right=rcItem.left;
		for(UINT i=0;i<m_nVisibleItems;i++)
		{
			ToolBarItem & item = m_arrItems[i];
			if(m_bVert)
			{
				rcItem.top = rcItem.bottom;
				rcItem.bottom += item.nId<0?nSep:szItem.cy;
			}else
			{
				rcItem.left = rcItem.right;
				rcItem.right += item.nId<0?nSep:szItem.cx;;
			}

			DrawItem(pRT,rcItem,&item);
		}
		if(HasDropDownButton())
		{
			CRect rc = GetDropdownButtonRect();
			DrawDropButton(pRT,rc,m_dwDropBtnState);
		}
		AfterPaint(pRT,painter);
	}

	void SToolBar::OnLButtonDown(UINT nFlags,CPoint pt)
	{
		__baseCls::OnLButtonDown(nFlags,pt);
		int iItem = HitTest(pt);
		if(iItem!=-1)
		{
			if(m_arrItems[iItem].dwState & (WndState_Disable|WndState_Check))
				return;
			m_iClickItem = iItem;
			m_arrItems[iItem].dwState |= WndState_PushDown;
			CRect rcItem = GetItemRect(iItem);
			InvalidateRect(rcItem);
		}else
		{
			CRect rc = GetDropdownButtonRect();
			if(rc.PtInRect(pt))
			{
				m_dwDropBtnState |= WndState_PushDown;
				InvalidateRect(rc);
			}
		}
	}

	void SToolBar::OnLButtonUp(UINT nFlags,CPoint pt)
	{
		__baseCls::OnLButtonUp(nFlags,pt);
		if(m_iClickItem!=-1)
		{
			m_arrItems[m_iClickItem].dwState &= ~WndState_PushDown;
			CRect rc = GetItemRect(m_iClickItem);
			InvalidateRect(rc);

			int iHover = HitTest(pt);
			if(iHover == m_iClickItem)
			{
				ToolBarItem & item = m_arrItems[m_iClickItem];
				EventToolBarCmd evt(this);
				evt.iItem = m_iClickItem;
				evt.nCmdId = item.nId;
				evt.strText = item.strText;
				evt.lParam = item.lParam;
				FireEvent(evt);
			}
			m_iClickItem = -1;
			OnMouseMove(nFlags,pt);
		}else if(m_dwDropBtnState & WndState_PushDown)
		{
			CRect rc = GetDropdownButtonRect();
			m_dwDropBtnState &= ~WndState_PushDown;
			InvalidateRect(rc);
			if(rc.PtInRect(pt))
			{
				m_dwDropBtnState |= WndState_Hover;
				ShowDropDownMenu(rc);
			}
		}
	}

	void SToolBar::OnMouseMove(UINT nFlags,CPoint pt)
	{
		__baseCls::OnMouseMove(nFlags,pt);
		if(m_iClickItem == -1)
		{
			int iItem = HitTest(pt);
			if(iItem != m_iHoverItem)
			{
				if(m_iHoverItem!=-1)
				{
					m_arrItems[m_iHoverItem].dwState &= ~WndState_Hover;
					CRect rc = GetItemRect(m_iHoverItem);
					InvalidateRect(rc);
					m_iHoverItem = -1;
				}
				if(iItem != -1)
				{
					if(!(m_arrItems[iItem].dwState & (WndState_Disable |WndState_Check)))
					{
						m_arrItems[iItem].dwState |= WndState_Hover;
						CRect rc = GetItemRect(iItem);
						InvalidateRect(rc);
						m_iHoverItem = iItem;
					}
				}
			}
			if(iItem == -1)
			{
				CRect rcDropButton = GetDropdownButtonRect();
				if(rcDropButton.PtInRect(pt))
				{
					m_dwDropBtnState |= WndState_Hover;
					InvalidateRect(rcDropButton);
				}else if(m_dwDropBtnState & WndState_Hover)
				{
					m_dwDropBtnState &=~WndState_Hover;
					InvalidateRect(rcDropButton);
				}
			}
		}
	}

	void SToolBar::OnSize(UINT nType, CSize size)
	{
		__baseCls::OnSize(nType,size);
		UpdateVisibleItemCount();
	}

	void SToolBar::OnMouseLeave()
	{
		if(m_iClickItem!=-1)
			return;
		if(m_iHoverItem!=-1)
		{
			m_arrItems[m_iHoverItem].dwState &= ~WndState_Hover;
			InvalidateRect(GetItemRect(m_iHoverItem));
			m_iHoverItem = -1;
		}
		if(m_dwDropBtnState & WndState_Hover)
		{
			m_dwDropBtnState = 0;
			InvalidateRect(GetDropdownButtonRect());
		}
	}

	BOOL SToolBar::OnUpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
	{
		int iItem = HitTest(pt);
		if(iItem==-1)
			return FALSE;
		tipInfo.dwCookie=iItem;
		tipInfo.rcTarget = GetItemRect(iItem);
		tipInfo.strTip = m_arrItems[iItem].strTip;
		tipInfo.swnd = m_swnd;
		return TRUE;
	}

	void SToolBar::InsertItem(const ToolBarItem & item,int nPos/*=-1*/)
	{
		if(nPos<0 || nPos>=m_arrItems.GetCount())
		{
			nPos = m_arrItems.Add(item);
		}else 
		{
			m_arrItems.InsertAt(nPos,item);
		}
		m_arrItems[nPos].dwState = 0;
		if(m_nVisibleItems==m_arrItems.GetCount()-1)
			UpdateVisibleItemCount();
		Invalidate();
	}

	void SToolBar::SetIconsSkin(SAutoRefPtr<ISkinObj> skinIcons)
	{
		m_skinIcons = skinIcons;
		Invalidate();
	}


	void SToolBar::UpdateVisibleItemCount()
	{
		CSize szItem = GetItemSize();
		int   nSep = GetSepWid();
		CRect rcClient = GetClientRect();
		CRect rc = rcClient;
		if(m_bVert)
		{
			rc.bottom=rc.top;
		}else
		{
			rc.right = rc.left;
		}
		m_nVisibleItems = m_arrItems.GetCount();
		for(int i=0;i<m_arrItems.GetCount();i++)
		{
			if(IsSeparator(&m_arrItems[i]))
			{
				if(m_bVert)
				{
					rc.top = rc.bottom;
					rc.bottom+=nSep;
				}else
				{
					rc.left = rc.right;
					rc.right+= nSep;
				}
			}else
			{
				if(m_bVert)
				{
					rc.top = rc.bottom;
					rc.bottom += szItem.cy;
				}else
				{
					rc.left = rc.right;
					rc.right += szItem.cx;
				}
			}
			if(m_bVert)
			{
				if(rc.bottom>rcClient.bottom)
				{
					m_nVisibleItems = i;
					break;
				}
			}else
			{
				if(rc.right>rcClient.right)
				{
					m_nVisibleItems = i;
					break;
				}
			}

		}
		if(m_nVisibleItems<m_arrItems.GetCount())
		{//show dropdown button
			if(m_bVert)
			{
				if(rcClient.bottom-rc.top<m_nDropDownSize)
					m_nVisibleItems--;
			}else
			{
				if(rcClient.right-rc.left<m_nDropDownSize)
					m_nVisibleItems--;
			}
			if(m_nVisibleItems<0) m_nVisibleItems=0;
		}
	}

	BOOL SToolBar::HasDropDownButton() const
	{
		return m_nVisibleItems<m_arrItems.GetCount();
	}

	CRect SToolBar::GetDropdownButtonRect() const
	{
		CRect rc;
		if(HasDropDownButton())
		{
			CRect rcItem = GetItemRect(m_nVisibleItems-1);
			rc=rcItem;
			if(m_bVert)
			{
				rc.top = rc.bottom;
				rc.bottom += m_nDropDownSize;
			}else
			{
				rc.left = rc.right;
				rc.right += m_nDropDownSize;
			}
		}
		return rc;
	}

	void SToolBar::DrawDropButton(IRenderTarget *pRT,const CRect &rc,DWORD dwState)
	{
		if(m_skinDropArrow)
		{
			m_skinDropArrow->DrawByState(pRT,rc,dwState);
		}
	}

	void SToolBar::ShowDropDownMenu(const CRect &rc)
	{
		SMenu menu;
		menu.SetIconSkin(m_skinIcons);
		SXmlNode xmlNode = m_menuStyle.root().child(L"menuStyle");
		menu.LoadMenu2(&xmlNode);
 		for(int i=m_nVisibleItems;i<m_arrItems.GetCount();i++)
 		{
			UINT uFlag = IsSeparator(&m_arrItems[i])?MF_SEPARATOR:MF_STRING;
			if(m_arrItems[i].dwState & WndState_Check)
				uFlag |= MF_CHECKED;
 			menu.AppendMenu(uFlag,i,m_arrItems[i].strText,m_arrItems[i].iIcon);
 		}
		CRect rcHost(rc);
		GetContainer()->FrameToHost(rcHost);
		CPoint pt = rc.BottomRight();
		::ClientToScreen(GetContainer()->GetHostHwnd(),&pt);

		UINT uCmd = menu.TrackPopupMenu(TPM_BOTTOMALIGN|TPM_RIGHTALIGN|TPM_RETURNCMD,pt.x,pt.y,NULL);
		if(uCmd)
		{
			ToolBarItem & item = m_arrItems[uCmd];
			EventToolBarCmd evt(this);
			evt.iItem = m_iClickItem;
			evt.nCmdId = item.nId;
			evt.strText = item.strText;
			evt.lParam = item.lParam;
			FireEvent(evt);
		}
	}

	BOOL SToolBar::SetItemCheck(int nItemId,BOOL bCheck)
	{
		int iItem = GetItemByID(nItemId);
		if(iItem==-1)
			return FALSE;
		if(bCheck)
			m_arrItems[iItem].dwState|=WndState_Check;
		else
			m_arrItems[iItem].dwState &=~WndState_Check;
		if(iItem < m_nVisibleItems)
		{
			CRect rcItem = GetItemRect(iItem);
			InvalidateRect(rcItem);
		}
		return TRUE;
	}

	int SToolBar::GetItemByID(int nID) const
	{
		int iRet = -1;
		for(UINT i=0;i<m_arrItems.GetCount();i++)
		{
			if(m_arrItems[i].nId == nID)
			{
				iRet = i;
				break;
			}
		}
		return iRet;
	}

}
