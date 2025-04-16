#include "stdafx.h"
#include "STabCtrlEx.h"

namespace SOUI
{
	//////////////////////////////////////////////////////////////////////////
	// STabCtrlEx

	STabCtrlEx::STabCtrlEx() : m_nCurrentPage(0)
		, m_pSkinTab(GETBUILTINSKIN(SKIN_SYS_TAB_PAGE))
		, m_pSkinIcon(NULL)
		, m_pSkinTabInter(NULL)
		, m_pSkinFrame(NULL)
		, m_nTabInterSize(0)
		, m_nTabPos(0)
		, m_nHoverTabItem(-1)
		, m_nAnimateSteps(0)
		, m_ptText(-1,-1)
		, m_txtDir(Text_Horz)
	{
		m_szTab.cx = m_szTab.cy = -1;
		m_bFocusable = TRUE;
		m_evtSet.addEvent(EVENTID(EventTabSelChanging));
		m_evtSet.addEvent(EVENTID(EventTabSelChanged));
		m_evtSet.addEvent(EVENTID(EventTabItemHover));
		m_evtSet.addEvent(EVENTID(EventTabItemLeave));
	}

	void STabCtrlEx::OnPaint(IRenderTarget *pRT)
	{
		SPainter painter;
		BeforePaint(pRT, painter);

		CRect rcItem, rcItemPrev;
		CRect rcSplit;
		DWORD dwState;
		CRect rcTitle = GetTitleRect();


		pRT->PushClipRect(&rcTitle, RGN_AND);

		for (size_t i = 0; i < GetItemCount(); i++)
		{
			dwState = WndState_Normal;
			if (i == m_nCurrentPage) dwState = WndState_PushDown;
			else if (i == m_nHoverTabItem) dwState = WndState_Hover;

			GetItemRect(i, rcItem);
			if (rcItem.IsRectEmpty()) continue;

			//画分隔线
			if (i > 0 && m_pSkinTabInter)
			{
				rcSplit = rcItem;
				rcSplit.top = rcItemPrev.bottom;
				rcSplit.bottom = rcSplit.top + m_nTabInterSize;
				m_pSkinTabInter->DrawByIndex(pRT, rcSplit, 0);
			}

			DrawItem(pRT, rcItem, i, dwState);
			rcItemPrev = rcItem;
		}
		pRT->PopClip();

		if (m_pSkinFrame)
		{
			CRect rcPage = GetChildrenLayoutRect();
			m_pSkinFrame->DrawByState(pRT, rcPage, WndState_Normal);
		}

		if (IsFocused() && IsFocusable() && m_bDrawFocusRect)
		{
			CRect rc;
			GetItemRect(m_nCurrentPage, rc);
			rc.DeflateRect(2, 2);
			DrawDefFocusRect(pRT, &rc);
		}
		AfterPaint(pRT, painter);
	}

	CRect STabCtrlEx::GetChildrenLayoutRect()
	{
		CRect rcRet;
		GetClientRect(rcRet);
		rcRet.left += m_szTab.cx;
		return rcRet;
	}

	void STabCtrlEx::OnLButtonDown(UINT nFlags, CPoint point)
	{
		SWindow::OnLButtonDown(nFlags, point);
		int iClickItem = HitTest(point);
		if (iClickItem != m_nCurrentPage)
		{
			SetCurSel(iClickItem);
		}
	}

	BOOL STabCtrlEx::RemoveItem(int nIndex, int nSelPage/*=0*/)
	{
		STabPageEx * pTab = GetItem(nIndex);
		OnItemRemoved(pTab);

		m_pItemPanel->DestroyChild(pTab);
		m_lstPages.RemoveAt(nIndex);

		if (m_nCurrentPage == nIndex)
		{
			if (nSelPage < 0) nSelPage = 0;
			if (nSelPage >= (int)GetItemCount()) nSelPage = GetItemCount() - 1;
			m_nCurrentPage = -1;
			SetCurSel(nSelPage);
		}
		else
		{
			if (m_nCurrentPage > nIndex) m_nCurrentPage--;
			CRect rcTitle = GetTitleRect();
			InvalidateRect(rcTitle);
		}
		return TRUE;
	}

	void STabCtrlEx::RemoveAllItems(void)
	{
		for (int i = GetItemCount() - 1; i >= 0; i--)
		{
			STabPageEx * pTab = GetItem(i);
			OnItemRemoved(pTab);
			m_pItemPanel->DestroyChild(pTab);
			m_lstPages.RemoveAt(i);
		}
		m_nCurrentPage = -1;
		Invalidate();
	}

	void STabCtrlEx::OnMouseMove(UINT nFlags, CPoint point)
	{
		CRect rcItem;
		int nOldHover = m_nHoverTabItem;
		m_nHoverTabItem = -1;
		int nTabCount = GetItemCount();
		for (int i = 0; i < nTabCount; i++)
		{
			GetItemRect(i, rcItem);

			if (rcItem.PtInRect(point))
			{
				m_nHoverTabItem = i;
				break;
			}
		}
		if (m_nHoverTabItem != nOldHover)
		{
			if (nOldHover != -1)
			{
				if (nOldHover != m_nCurrentPage)
				{
					GetItemRect(nOldHover, rcItem);
					InvalidateRect(rcItem);
				}
				EventTabItemLeave evt(this);
				evt.iLeave = nOldHover;
				FireEvent(evt);
			}
			if (m_nHoverTabItem != -1)
			{
				if (m_nHoverTabItem != m_nCurrentPage)
				{
					GetItemRect(m_nHoverTabItem, rcItem);
					InvalidateRect(rcItem);
				}
				EventTabItemHover evt(this);
				evt.iHover = m_nHoverTabItem;
				FireEvent(evt);
			}
		}
	}

	void STabCtrlEx::OnDestroy()
	{
		m_lstPages.RemoveAll();
	}

	SWindow * STabCtrlEx::GetPage(int iPage)
	{
		if (iPage < 0 || iPage >= (int)GetItemCount()) return NULL;
		return m_lstPages[iPage];
	}

	int STabCtrlEx::GetPageIndex(LPCTSTR pszName, BOOL bTitle)
	{
		if (bTitle)
		{
			for (UINT i = 0; i < m_lstPages.GetCount(); i++)
			{
				if (_tcscmp(m_lstPages[i]->GetTitle(), pszName) == 0)
					return i;
			}
		}
		else
		{
			SStringW strName = S_CT2W(pszName);
			for (UINT i = 0; i < m_lstPages.GetCount(); i++)
			{
				if (m_lstPages[i]->GetName() == strName)
					return i;
			}
		}
		return -1;
	}

	SWindow * STabCtrlEx::GetPage(LPCTSTR pszName, BOOL bTitle/*=TRUE*/)
	{
		int iPage = GetPageIndex(pszName, bTitle);
		if (iPage == -1) return NULL;
		return m_lstPages[iPage];
	}

	BOOL STabCtrlEx::SetCurSel(int nIndex, bool bScrollView)
	{
		if (nIndex < 0 || nIndex>(int)GetItemCount() - 1 || (m_nCurrentPage == nIndex)) return FALSE;
		int nOldPage = m_nCurrentPage;

		EventTabSelChanging evt(this);
		evt.uOldSel = nOldPage;
		evt.uNewSel = nIndex;
		FireEvent(evt);
		if (evt.bCancel)
			return FALSE;
		STabPageEx *pTab = GetItem(nIndex);
		CRect rcItem;
		GetItemRect(m_nCurrentPage, rcItem);
		InvalidateRect(rcItem);
		m_nCurrentPage = nIndex;
		GetItemRect(m_nCurrentPage, rcItem);
		InvalidateRect(rcItem);
		if (bScrollView)
		{
			int iPosY = 0;
			for (int i = 0; i < m_nCurrentPage; i++)
			{
				iPosY += m_lstPages[i]->GetHeight();
			}
			m_pItemPanel->GetEventSet()->setMutedState(true);
			m_pItemPanel->SetViewOrigin(CPoint(0, iPosY));
			m_pItemPanel->GetEventSet()->setMutedState(false);
		}

		EventTabSelChanged evt2(this);
		evt2.uNewSel = nIndex;
		evt2.uOldSel = nOldPage;

		FireEvent(evt2);
		return TRUE;
	}

	BOOL STabCtrlEx::SetCurSel(LPCTSTR pszName, BOOL bTitle/*=TRUE */)
	{
		int iPage = GetPageIndex(pszName, bTitle);
		if (iPage == -1) return FALSE;
		return SetCurSel(iPage);
	}

	BOOL STabCtrlEx::SetItemTitle(int nIndex, LPCTSTR lpszTitle)
	{
		STabPageEx* pTab = GetItem(nIndex);
		if (pTab)
		{
			pTab->SetTitle(lpszTitle);

			CRect rcTitle = GetTitleRect();
			InvalidateRect(rcTitle);
			return TRUE;
		}

		return FALSE;
	}

	BOOL STabCtrlEx::OnScrollviewOrginChanger(EventArgs *e)
	{
		EventScrollViewOriginChanged *ev = static_cast<EventScrollViewOriginChanged*>(e);
		//视图滚动方向
		BOOL bUp = ev->ptNewOrigin.y - ev->ptOldOrigin.y > 0 ? TRUE : FALSE;
		int iNextViewPos = 0;
		//向上滚动
		if (bUp) {
			//pos += 16;//总要让人家显示一点吧
			//CRect viewSize = m_pItemPanel->GetClientRect();
			int icurViewHei = 0;//viewSize.Height();
			for (int i = 0; i < m_lstPages.GetCount(); i++)
			{
				iNextViewPos += m_lstPages[i]->GetHeight();
				if ((icurViewHei < ev->ptNewOrigin.y) && (ev->ptNewOrigin.y < iNextViewPos))
				{
					SetCurSel(i > 0 ? i : 0, false);
					break;
				}
			}
		}
		else
		{
			for (int i = 0; i < m_lstPages.GetCount(); i++)
			{
				iNextViewPos += m_lstPages[i]->GetHeight();
				if (iNextViewPos > ev->ptNewOrigin.y)
				{
					SetCurSel(i > 0 ? i : 0, false);
					break;
				}
			}
		}
		return true;
	}

	BOOL STabCtrlEx::CreateChildren(IXmlNode* pNode)
	{
		SXmlNode xmlNode(pNode);
		m_pItemPanel = (SScrollView *)SApplication::getSingleton().CreateWindowByName(SScrollView::GetClassName());
		SASSERT(m_pItemPanel);
		InsertChild(m_pItemPanel);
		SXmlNode socrollviewStyle = xmlNode.child(L"socrollviewStyle");
		if (socrollviewStyle)
		{
			m_pItemPanel->InitFromXml(&socrollviewStyle);
		}
		else m_pItemPanel->SSendMessage(WM_CREATE);//如果没有设置socrollviewStyle则必须手动发送一次CREATE消息，否则滚动条无法正常初使化
		//自适应内容
		m_pItemPanel->SetAttribute(L"viewSize", L"-1,-1");
		m_pItemPanel->SetAttribute(L"layout", L"vbox");

		m_pItemPanel->GetEventSet()->subscribeEvent(EVT_SCROLLVIEW_ORIGINCHANGED, Subscriber(&STabCtrlEx::OnScrollviewOrginChanger, this));
		xmlNode = xmlNode.child(L"pages");
		if (xmlNode)
		{
			for (SXmlNode xmlChild = xmlNode.first_child(); xmlChild; xmlChild = xmlChild.next_sibling())
			{
				InsertItem(xmlChild, -1, TRUE);
			}
		}
		if (m_nCurrentPage == -1 || m_nCurrentPage >= (int)m_lstPages.GetCount())
		{
			m_nCurrentPage = 0;
		}
		if (m_lstPages.GetCount() == 0)
		{
			m_nCurrentPage = -1;
		}
		return TRUE;
	}

	STabPageEx * STabCtrlEx::CreatePageFromXml(SXmlNode xmlPage)
	{
		if (wcscmp(xmlPage.name(), STabPageEx::GetClassName()) != 0) return NULL;
		return new STabPageEx();//(STabPageEx *)SApplication::getSingleton().CreateWindowByName(STabPageEx::GetClassName());
	}

	int STabCtrlEx::InsertItem(LPCWSTR lpContent, int iInsert/*=-1*/)
	{
		pugi::xml_document xmlDoc;
		if (!xmlDoc.load_buffer(lpContent, wcslen(lpContent) * sizeof(wchar_t), pugi::parse_default, pugi::encoding_utf16)) return -1;
		return InsertItem(xmlDoc.first_child(), iInsert);
	}

	int STabCtrlEx::InsertItem(SXmlNode xmlNode, int iInsert/*=-1*/, BOOL bLoading/*=FALSE*/)
	{
		STabPageEx *pChild = CreatePageFromXml(xmlNode);
		if (!pChild) return -1;

		m_pItemPanel->InsertChild(pChild);
		pChild->InitFromXml(&xmlNode);
		pChild->SetVisible(TRUE);
		if (iInsert == -1)
			iInsert = m_lstPages.GetCount();
		m_lstPages.InsertAt(iInsert, pChild);
		if (!bLoading)
		{
			UpdateChildrenPosition();
			if (m_nCurrentPage >= iInsert)
				m_nCurrentPage++;
			InvalidateRect(GetTitleRect());
			if (m_nCurrentPage == -1)
				SetCurSel(iInsert);
		}//*/	
		return iInsert;
	}

	CRect STabCtrlEx::GetTitleRect()
	{
		CRect rcTitle;
		GetClientRect(rcTitle);
		rcTitle.right = rcTitle.left + m_szTab.cx;
		return rcTitle;
	}

	BOOL STabCtrlEx::GetItemRect(int nIndex, CRect &rcItem)
	{
		if (nIndex < 0 || nIndex >= (int)GetItemCount())
			return FALSE;

		CRect rcTitle = GetTitleRect();

		rcItem = CRect(rcTitle.TopLeft(), m_szTab);
		rcItem.OffsetRect(0, m_nTabPos + nIndex * (rcItem.Height() + m_nTabInterSize));
		rcItem.IntersectRect(rcItem, rcTitle);
		return TRUE;
	}

	STabPageEx* STabCtrlEx::GetItem(int nIndex)
	{
		if (nIndex < 0 || nIndex >= (int)GetItemCount()) return NULL;
		return m_lstPages[nIndex];
	}

	void STabCtrlEx::DrawItem(IRenderTarget *pRT, const CRect &rcItem, int iItem, DWORD dwState)
	{
		if (rcItem.IsRectEmpty()) return;
		
		if (m_pSkinTab)
			m_pSkinTab->DrawByState(pRT, rcItem, dwState);
		//int iState = IIF_STATE3(dwState, WndState_Normal, WndState_Hover, WndState_PushDown);
		//根据状态从style中获得字体，颜色
		IFontPtr font = m_style.GetTextFont(0);
		COLORREF crTxt = m_style.GetTextColor(0);
		CAutoRefPtr<IFont> oldFont;
		if (font) pRT->SelectObject(font, (IRenderObj**)&oldFont);
		COLORREF crOld = 0;
		if (crTxt != CR_INVALID) crOld = pRT->SetTextColor(crTxt);

		CRect rcIcon(m_ptIcon + rcItem.TopLeft(), CSize(0, 0));
		if (m_pSkinIcon)
		{
			rcIcon.right = rcIcon.left + m_pSkinIcon->GetSkinSize().cx;
			rcIcon.bottom = rcIcon.top + m_pSkinIcon->GetSkinSize().cy;
			int iIcon = GetItem(iItem)->GetIconIndex();
			if (iIcon == -1) iIcon = iItem;
			m_pSkinIcon->DrawByIndex(pRT, rcIcon, iIcon);
		}
		LPCTSTR strTitle= GetItem(iItem)->GetTitle();
		if (m_ptText.x != -1 && m_ptText.y != -1)
		{//从指定位置开始绘制文字
			if (m_txtDir == Text_Horz)
				pRT->TextOut(rcItem.left + m_ptText.x, rcItem.top + m_ptText.y, strTitle, -1);
			else
				TextOutV(pRT, rcItem.left + m_ptText.x, rcItem.top + m_ptText.y, strTitle);
		}
		else
		{
			CRect rcText = rcItem;
			UINT alignStyle = m_style.GetTextAlign();
			UINT align = alignStyle;
			if (m_ptText.x == -1 && m_ptText.y != -1)
			{//指定了Y偏移，X居中
				rcText.top += m_ptText.y;
				align = alignStyle&(DT_CENTER | DT_RIGHT | DT_SINGLELINE | DT_END_ELLIPSIS);
			}
			else if (m_ptText.x != -1 && m_ptText.y == -1)
			{//指定了X偏移，Y居中
				rcText.left += m_ptText.x;
				align = alignStyle&(DT_VCENTER | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS);
			}

			if (m_txtDir == Text_Horz)
				pRT->DrawText(strTitle, -1, &rcText, align);
			else
				DrawTextV(pRT, rcText, strTitle);
		}

		//恢复字体，颜色
		if (font) pRT->SelectObject(oldFont);
		if (crTxt != CR_INVALID) pRT->SetTextColor(crOld);
	}

	BOOL STabCtrlEx::OnUpdateToolTip(CPoint pt, SwndToolTipInfo & tipInfo)
	{
		int iItem = HitTest(pt);
		if (iItem == -1) return FALSE;
		if (GetItem(iItem)->GetToolTipText().IsEmpty()) return FALSE;
		tipInfo.swnd = m_swnd;
		tipInfo.dwCookie = iItem;
		GetItemRect(iItem, tipInfo.rcTarget);
		tipInfo.strTip = GetItem(iItem)->GetToolTipText();
		return TRUE;
	}

	void STabCtrlEx::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if (nChar == VK_LEFT || nChar == VK_UP)
		{
			if (!SetCurSel(m_nCurrentPage - 1))
				SetCurSel(GetItemCount() - 1);
		}
		else if (nChar == VK_RIGHT || nChar == VK_DOWN)
		{
			if (!SetCurSel(m_nCurrentPage + 1))
				SetCurSel(0);
		}
		else if (nChar == VK_HOME)
		{
			SetCurSel(0);
		}
		else if (nChar == VK_END)
		{
			SetCurSel(GetItemCount() - 1);
		}
	}

	int STabCtrlEx::HitTest(CPoint pt)
	{
		int nTabCount = GetItemCount();
		for (int i = 0; i < nTabCount; i++)
		{
			CRect rcItem;
			GetItemRect(i, rcItem);
			if (rcItem.PtInRect(pt))
			{
				return i;
			}
		}
		return -1;
	}

	void STabCtrlEx::OnInitFinished(IXmlNode* xmlNode)
	{
		if (m_pSkinTab)
		{
			SIZE sz = m_pSkinTab->GetSkinSize();
			if (m_szTab.cx == -1) m_szTab.cx = sz.cx;
			if (m_szTab.cy == -1) m_szTab.cy = sz.cy;
		}
	}

	void STabCtrlEx::UpdateChildrenPosition()
	{
		CRect rcPage = GetChildrenLayoutRect();
		m_pItemPanel->Move(rcPage);
		CSize itemsize;
		STabPageEx *pLastPage = NULL;
		for (int i = 0; i < m_lstPages.GetCount(); i++)
		{
			pLastPage = m_lstPages[i];
			m_lstPages[i]->GetDesiredSize(&itemsize,rcPage.Width(),rcPage.Height());
			m_lstPages[i]->SetHeight(itemsize.cy);
		}
		//最后一页内容太少
		if (pLastPage&&pLastPage->GetHeight() < rcPage.Height())
		{
			pLastPage->SetHeight(rcPage.Height());
		}
		m_pItemPanel->UpdateLayout();
	}

	void STabCtrlEx::BeforePaint(IRenderTarget *pRT, SPainter &painter)
	{
		IFontPtr pFont = m_style.GetTextFont(0);
		if (pFont)
			pRT->SelectObject(pFont, (IRenderObj**)&painter.oldFont);

		COLORREF crTxt = m_style.GetTextColor(0);
		if (crTxt != CR_INVALID)
			painter.oldTextColor = pRT->SetTextColor(crTxt);

	}

	void STabCtrlEx::TextOutV(IRenderTarget *pRT, int x, int y, const SStringT & strText)
	{
		SStringT strTmp = strText;
		LPTSTR p = strTmp.GetBuffer(0);
		while (*p)
		{
			LPTSTR p2 = SStringT::_tchar_traits::CharNext(p);
			SIZE szWord;
			pRT->MeasureText(p, p2 - p, &szWord);
			pRT->TextOut(x, y, p, p2 - p);
			p = p2;
			y += szWord.cy;
		}
		strTmp.ReleaseBuffer();
	}

	SIZE STabCtrlEx::MeasureTextV(IRenderTarget *pRT, const SStringT & strText)
	{
		SIZE szRet = { 0,0 };
		SStringT strTmp = strText;
		LPTSTR p = strTmp.GetBuffer(0);
		while (*p)
		{
			LPTSTR p2 = SStringT::_tchar_traits::CharNext(p);
			SIZE szWord;
			pRT->MeasureText(p, p2 - p, &szWord);
			szRet.cx = smax(szRet.cx, szWord.cx);
			szRet.cy += szWord.cy;
			p = p2;
		}
		strTmp.ReleaseBuffer();

		return szRet;
	}

	void STabCtrlEx::DrawTextV(IRenderTarget *pRT, CRect rcText, const SStringT & strText)
	{
		SIZE szText = MeasureTextV(pRT, strText);
		TextOutV(pRT,
			rcText.left + (rcText.Width() - szText.cx) / 2,
			rcText.top + (rcText.Height() - szText.cy) / 2,
			strText
		);
	}

	void STabCtrlEx::OnColorize(COLORREF cr)
	{
		__baseCls::OnColorize(cr);
		if (m_pSkinIcon) m_pSkinIcon->OnColorize(cr);
		if (m_pSkinTab) m_pSkinTab->OnColorize(cr);
		if (m_pSkinTabInter) m_pSkinTabInter->OnColorize(cr);
		if (m_pSkinFrame) m_pSkinFrame->OnColorize(cr);
	}	

}//namespace SOUI