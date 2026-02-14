// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PreviewHost.h"	
#include "Adapter.h"
#include "helper/SplitString.h"
#include <layout/SouiLayout.h>
#include <layout/SLinearLayout.h>
#include <layout/SGridLayout.h>
#include <layout/SAnchorLayout.h>

#define kLogTag "CPreviewHost"


CPreviewHost::CPreviewHost(IListener *pListener,LPCTSTR pszLayoutId, HWND hEditor/* = NULL*/)
: SHostWnd(pszLayoutId)
,m_pHover(NULL)
,m_pSel(NULL)
,m_bVirtualRoot(FALSE)
,m_bSOUIWnd(FALSE)
,m_iRootIndex(0)
,m_hOwner(hEditor)
,m_pListener(pListener)
{
}

CPreviewHost::~CPreviewHost()
{
}

void CPreviewHost::OnFinalMessage(HWND hWnd)
{
	SHostWnd::OnFinalMessage(hWnd);
	if(m_pListener){
		delete this;
    }
    else
    {
        PostQuitMessage(0);
    }
}

LRESULT CPreviewHost::OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(!m_pListener){
		SetMsgHandled(FALSE);
		return 0;
	}
	SASSERT(m_pSel);
	if(GetKeyState(VK_MENU)&0x80 || m_pSel->GetCapture() == m_pSel->GetSwnd())
	{
		SetMsgHandled(FALSE);
		return TRUE;
	}else
	{
		CPoint pt(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		if(uMsg == WM_LBUTTONDOWN)
		{//tell editor the clicked window id.
			m_pHover->SetVisible(FALSE);
			SWND swnd = GetRoot()->SwndFromPoint(pt,true);
			m_pHover->SetVisible(TRUE);
			if(swnd && swnd!=m_pSel->GetSwnd())
			{
				SWindow *p = SWindowMgr::GetWindow(swnd);
				CRect rcSel = p->GetWindowRect();
				m_pSel->Move(rcSel);
                m_pSel->SetTarget(swnd);
				SList<int> lstIndex;
				GetSwndIndex(p,lstIndex);
				if(m_bVirtualRoot)
				{
					lstIndex.RemoveHead();
				}
				int *pOrder = new int[lstIndex.GetCount()];
				SPOSITION pos = lstIndex.GetHeadPosition();
				for(int i=0;i<lstIndex.GetCount();i++)
				{
					pOrder[i] = lstIndex.GetNext(pos);
				}
				m_pListener->OnSelectedCtrl(pOrder,lstIndex.GetCount());
				delete []pOrder;
			}else{
				SetMsgHandled(FALSE);
				return TRUE;
			}
		}else if(uMsg == WM_MOUSEMOVE)
		{//highlight the hovered window
			m_pHover->SetVisible(FALSE);
			SWND swnd = GetRoot()->SwndFromPoint(pt,true);
			m_pHover->SetVisible(TRUE);
			if(swnd == m_pSel->GetSwnd())
			{
				CRect rc = m_pSel->GetWindowRect();
				m_pHover->Move(rc);
				SetMsgHandled(FALSE);
			}else{
				if(swnd)
				{
					SWindow *p = SWindowMgr::GetWindow(swnd);
					CRect rc = p->GetWindowRect();
					m_pHover->Move(rc);
				}else
				{
					m_pHover->Move2(0,0,0,0);
				}
				SetCursor(::LoadCursor(NULL,IDC_ARROW));
			}
		}

		return TRUE;
	}
}

void CPreviewHost::SelectCtrlByOrder(const int *pOrder,int nLen)
{
    //刷新前重新布局，会自动检查布局脏标志
    GetRoot()->UpdateLayout();
	if(!pOrder)
	{
		m_pSel->SetVisible(FALSE,TRUE);
		return;
	}
	SWindow *pRoot = this->GetRoot();
	SWindow *pChild = pRoot;
	int i = 0;
	if (m_bSOUIWnd)
	{
		i = 2;
	}
	else if (!m_bVirtualRoot)
	{
		i = 1;
	}
	for (; i<nLen; i++)
	{
		int child_index = pOrder[i];
		pChild = pChild->GetChild(child_index+1);
		if (!pChild)
			break;
	}
    if (pChild && pChild->IsVisible(TRUE))
    {
        CRect rcSel = pChild->GetWindowRect();
        m_pSel->SetTarget(pChild->GetSwnd());
        m_pSel->Move(rcSel);
        m_pSel->SetVisible(TRUE,TRUE);
    }
    else
    {
        m_pSel->SetVisible(FALSE,TRUE);
    }
}

void CPreviewHost::GetSwndIndex(SWindow *pWnd,SList<int> &lstIndex)
{
	SWindow *pParent = pWnd->GetParent();
	if(pParent)
	{
		int idx = 0;
		SWindow *pPrev = pWnd->GetWindow(GSW_PREVSIBLING);
		while(pPrev)
		{
			idx++;
			pPrev = pPrev->GetWindow(GSW_PREVSIBLING);
		}
		lstIndex.AddHead(idx);
		GetSwndIndex(pParent,lstIndex);
	}else
	{
		lstIndex.AddHead(m_iRootIndex);
	}
}

BOOL CPreviewHost::OnLoadLayoutFromResourceID(SXmlDoc& xmlDoc) {
    BOOL bXmlParseSuccess = FALSE;
	if (!m_utf8Buffer.IsEmpty())
	{
		bXmlParseSuccess = xmlDoc.load_buffer(m_utf8Buffer.c_str(), m_utf8Buffer.GetLength(), xml_parse_default, enc_utf8);
	}
	else
	{
		bXmlParseSuccess = LOADXML(xmlDoc, m_strXmlLayout);
	}
	if (bXmlParseSuccess)
	{
		m_iRootIndex = 0;
		SXmlNode xmlSoui = xmlDoc.root().child(L"SOUI");
		if (xmlSoui)
		{
			//find root node index.
			SXmlNode xmlSib = xmlSoui.child(L"root").previous_sibling();
			while (xmlSib)
			{
				m_iRootIndex++;
				xmlSib = xmlSib.previous_sibling();
			}

			m_bSOUIWnd = TRUE;
			xmlSoui.attribute(L"translucent").set_value(0);
			xmlSoui.attribute(L"wndType").set_value(L"normal");
		}
		else
		{//include element.
			const WCHAR ksz_include_host[] = L"<soui  margin=\"2,2,2,2\" resizable=\"1\">"\
				L"<root width=\"800\" height=\"600\" colorBkgnd=\"#888888\"/>"\
				L"</soui>";
			SXmlDoc xmlDoc2;
			xmlDoc2.load_buffer(ksz_include_host, sizeof(ksz_include_host), xml_parse_default, enc_wchar);
			SXmlNode xmlInclude = xmlDoc.root().child(L"include");
			if (xmlInclude)
			{
				SXmlNode xmlRoot = xmlDoc2.root().first_child();
				int inc_width = xmlInclude.attribute(L"width").as_int(600);
				int inc_height = xmlInclude.attribute(L"height").as_int(400);
                SXmlNode doc_root = xmlRoot.child(L"root");
				doc_root.attribute(L"width").set_value(inc_width);
				doc_root.attribute(L"height").set_value(inc_height);
                for (SXmlNode node = xmlInclude.first_child(); node; node = node.next_sibling())
                {
                    doc_root.append_copy(node);
                }
                xmlDoc.Reset();
                xmlDoc.root().append_copy(xmlDoc2.root().first_child());				
			}
			else
			{
				m_bVirtualRoot = TRUE;
				SXmlNode xmlRoot2 = xmlDoc2.root().first_child().first_child();
				xmlRoot2.append_copy(xmlDoc.root().first_child());
                xmlDoc.Reset();
                xmlDoc.root().append_copy(xmlDoc2.root().first_child());	
			}
		}
	}
	else
	{
		SLOGFMTI("Load layout [%s] Failed", S_CT2A(m_strXmlLayout).c_str());
	}
    xmlDoc.GetParseResult(&m_GetXmlParseResult);
	return bXmlParseSuccess;
}
int CPreviewHost::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int nRet = SHostWnd::OnCreate(lpCreateStruct);
	if(nRet==0 && m_pListener)
	{
		const WCHAR *pszXml=L"<sizingframe name=\"_preview_sel_frame\" float=\"1\"/>"
			L"<window float=\"1\" name=\"_preview_hover_frame\" msgTransparent=\"1\" margin=\"1,1,1,1\" colorBorder=\"#0000ff\"/>";
		GetRoot()->CreateChildrenFromXml(pszXml);
		m_pSel = FindChildByName2<SSizingFrame>("_preview_sel_frame");
		m_pHover = FindChildByName("_preview_hover_frame");
	}
	return nRet;
}

void CPreviewHost::OnSwndInitFinish(IEvtArgs *e)
{
	SWindow *pRet = sobj_cast<SWindow>(e->Sender());
	if(pRet)
	{
		if (pRet->IsClass(SMCListView::GetClassName()))
		{
			CBaseMcAdapterFix *mcAdapter = new CBaseMcAdapterFix();
			((SMCListView*)pRet)->SetAdapter(mcAdapter);
			mcAdapter->Release();
		}
		if (pRet->IsClass(SListView::GetClassName()))
		{
			CBaseAdapterFix *listAdapter = new CBaseAdapterFix();
			((SListView*)pRet)->SetAdapter(listAdapter);
			listAdapter->Release();
		}
		if (pRet->IsClass(STileView::GetClassName()))
		{
			CBaseAdapterFix *listAdapter = new CBaseAdapterFix();
			((STileView*)pRet)->SetAdapter(listAdapter);
			listAdapter->Release();
		}
		if(pRet->IsClass(STreeView::GetClassName()))
		{
			SEmptyTreeAdapter *treeAdapter = new SEmptyTreeAdapter();
			((STreeView*)pRet)->SetAdapter(treeAdapter);
			treeAdapter->Release();
		}
		if(pRet->IsClass(SComboView::GetClassName()))
		{
			SComboView *pComboView = sobj_cast<SComboView>(pRet);
			SListView *pListView = pComboView->GetListView();
			CBaseAdapterFix *listAdapter = new CBaseAdapterFix();
			pListView->SetAdapter(listAdapter);
			listAdapter->Release();
		}
	}
}

void CPreviewHost::OnFrameMoved(IEvtArgs *e)
{
	EventFrameMoved *e2 = sobj_cast<EventFrameMoved>(e);

	SWND hTarget = e2->hTarget;
	CRect rcNew = e2->rcNew;
	CRect rcOld = e2->rcOld;

    SWindow *pTarget = SWindowMgr::GetWindow(hTarget);
    SASSERT(pTarget);
    SWindow *pParent = pTarget->GetParent();
	if(!pParent)
		return;
    CRect rcParent;
	pParent->GetChildrenLayoutRect(&rcParent);
	if(rcNew.left < rcParent.left)
		rcNew.left = rcParent.left;
	if(rcNew.right > rcParent.right)
		rcNew.right = rcParent.right;
	if(rcNew.top < rcParent.top)
		rcNew.top = rcParent.top;
	if(rcNew.bottom > rcParent.bottom)
		rcNew.bottom = rcParent.bottom;
	int nWidth = rcNew.Width();
	int nHeight = rcNew.Height();

    ILayout *pLayout = pParent->GetLayout();
	ILayoutParam *pLayoutParam = pTarget->GetLayoutParam();
	if(pLayout->IsClass(SAnchorLayout::GetClassName())){
        if (nWidth != rcOld.Width())
        {
			SLayoutSize width(nWidth, px);
			pLayoutParam->SetSpecifiedSize(Horz, &width);
		}    
        else if (pLayoutParam->IsWrapContent(Horz))
            nWidth = SIZE_WRAP_CONTENT;
        else if (pLayoutParam->IsMatchParent(Horz))
            nWidth = SIZE_MATCH_PARENT;
        if (nHeight != rcOld.Height()){
			SLayoutSize height(nHeight, px);
			pLayoutParam->SetSpecifiedSize(Vert, &height);
		}
		else if (pLayoutParam->IsWrapContent(Vert))
			nHeight = SIZE_WRAP_CONTENT;
		else if (pLayoutParam->IsMatchParent(Vert))
            nHeight = SIZE_MATCH_PARENT;
		m_pListener->OnUpdateSize(nWidth,nHeight);
		SAnchorLayout *pAnchorLayout = (SAnchorLayout *)pLayout;
		{
			SAnchorLayoutParam *pAnchorParam = (SAnchorLayoutParam *)pLayoutParam;
			SAnchorLayoutParamStruct *pRawData = (SAnchorLayoutParamStruct *)pAnchorParam->GetRawData();
			AnchorPos &anchorPos = pRawData->pos;

			//将移动后的布局真实坐标转换成锚点坐标的配置。
			CPoint ptAnchor;
			switch(anchorPos.type){
				case APT_Left_Top:
				{
					ptAnchor.x = rcParent.left;
					ptAnchor.y = rcParent.top;
				}
				break;
				case APT_Center_Top:
				{
					ptAnchor.x = rcParent.CenterPoint().x;
					ptAnchor.y = rcParent.top;
				}
				break;
				case APT_Right_Top:
				{
					ptAnchor.x = rcParent.right;
					ptAnchor.y = rcParent.top;
				}
				break;
				case APT_Left_Center:
				{
					ptAnchor.x = rcParent.left;
					ptAnchor.y = rcParent.CenterPoint().y;
				}
				break;
				case APT_Center_Center:
				{
					ptAnchor = rcParent.CenterPoint();
				}	
				break;
				case APT_Right_Center:
				{
					ptAnchor.x = rcParent.right;
					ptAnchor.y = rcParent.CenterPoint().y;
				}
				break;
				case APT_Left_Bottom:
				{
					ptAnchor.x = rcParent.left;
					ptAnchor.y = rcParent.bottom;
				}
				break;
				case APT_Center_Bottom:
				{
					ptAnchor.x = rcParent.CenterPoint().x;
					ptAnchor.y = rcParent.bottom;
				}
				break;
				case APT_Right_Bottom:
				{
					ptAnchor.x = rcParent.right;
					ptAnchor.y = rcParent.bottom;
				}
				break;
				default:
				break;
			}
			//suppose config position is pos, then the real position is pos + ptAnchor + szOffset.
			//so config position = real position - ptAnchor - szOffset.
			CPoint pos = rcNew.TopLeft();
			float fOffsetX = anchorPos.fOffsetX;
			float fOffsetY = anchorPos.fOffsetY;
			CPoint szOffset(nWidth*fOffsetX, nHeight*fOffsetY);
			CPoint ptConfig = pos - ptAnchor - szOffset;
			m_pListener->OnUpdatePos(SStringW().Format(L"%d,%d,%d",ptConfig.x,ptConfig.y,anchorPos.type),FALSE);
		}
	}
    else if (pLayout->IsClass(SLinearLayout::GetClassName()))
    {
        if (nWidth != rcOld.Width())
        {
			SLayoutSize width(nWidth, px);
			pLayoutParam->SetSpecifiedSize(Horz, &width);
		}    
        else if (pLayoutParam->IsWrapContent(Horz))
            nWidth = SIZE_WRAP_CONTENT;
        else if (pLayoutParam->IsMatchParent(Horz))
            nWidth = SIZE_MATCH_PARENT;
        if (nHeight != rcOld.Height()){
			SLayoutSize height(nHeight, px);
			pLayoutParam->SetSpecifiedSize(Vert, &height);
		}
		else if (pLayoutParam->IsWrapContent(Vert))
			nHeight = SIZE_WRAP_CONTENT;
		else if (pLayoutParam->IsMatchParent(Vert))
            nHeight = SIZE_MATCH_PARENT;
		m_pListener->OnUpdateSize(nWidth,nHeight);
    }
    else if (pLayout->IsClass(SGridLayout::GetClassName()))
    {
        if (nWidth != rcOld.Width())
        {
			SLayoutSize width(nWidth, px);
			pLayoutParam->SetSpecifiedSize(Horz, &width);
		}   
        else if (pLayoutParam->IsWrapContent(Horz))
            nWidth = SIZE_WRAP_CONTENT;
        else if (pLayoutParam->IsMatchParent(Horz))
            nWidth = SIZE_MATCH_PARENT;
        if (nHeight != rcOld.Height()){
			SLayoutSize height(nHeight, px);
			pLayoutParam->SetSpecifiedSize(Vert, &height);
		}
        else if (pLayoutParam->IsWrapContent(Vert))
            nHeight = SIZE_WRAP_CONTENT;
        else if (pLayoutParam->IsMatchParent(Vert))
            nHeight = SIZE_MATCH_PARENT;
		m_pListener->OnUpdateSize(nWidth,nHeight);
    }
    else if (pLayout->IsClass(SouiLayout::GetClassName()))
    {
		SStringW strPos;
		SouiLayoutParam *pSouiLayoutParam = (SouiLayoutParam *)pLayoutParam;
		SouiLayoutParamStruct *pRawData = (SouiLayoutParamStruct *)pSouiLayoutParam->GetRawData();
		if(pRawData->nCount == 0){
			strPos.Format(L"%d,%d,@%d,@%d", 
				rcNew.left-rcParent.left, 
				rcNew.top-rcParent.top,
				rcNew.Width(),
				rcNew.Height());
		}
		else if(pRawData->nCount >= 2){
			//update left pos desc
			if(pRawData->posLeft.pit == PIT_CENTER){
				//support center mode
                int left = rcNew.left - rcParent.CenterPoint().x;
                if (pRawData->fOffsetX != 0.0f)
                {
                    left -= nWidth * pRawData->fOffsetX;
                }
				pRawData->posLeft.nPos = left;
				strPos += SStringW().Format(L"|%d", left);
			}
			else if(pRawData->posLeft.pit == PIT_PERCENT){
				//support percent mode
                int left = rcNew.left - rcParent.left;
                if (pRawData->fOffsetX != 0.0f)
                {
                    left -= nWidth * pRawData->fOffsetX;
                }
				left = left * 100 / rcParent.Width();
				pRawData->posLeft.nPos = left;
				strPos += SStringW().Format(L"%%%d", left);
            }else
            {
                pRawData->posLeft.pit = PIT_NORMAL;
                if (pRawData->posLeft.bMinus)
                {
					int pos = rcParent.right - rcNew.left;
					if (pRawData->fOffsetX != 0.0f)
                    {
                        pos += nWidth * pRawData->fOffsetX;
                    }
                    pRawData->posLeft.nPos = pos;
                    strPos += SStringW().Format(L"-%d", pos);
                }
                else
                {
					int pos = rcNew.left - rcParent.left;
					if (pRawData->fOffsetX != 0.0f)
                    {
                        pos -= nWidth * pRawData->fOffsetX;
                    }
                    pRawData->posLeft.nPos = pos;
					strPos += SStringW().Format(L"%d", pos);
                }
            }
			//update top pos desc
			if(pRawData->posTop.pit == PIT_CENTER)
			{
                int top = rcNew.top - rcParent.CenterPoint().y;
                if (pRawData->fOffsetY != 0.0f)
                {
                    top -= nHeight * pRawData->fOffsetY;
                }
				pRawData->posTop.nPos = top;
				strPos += SStringW().Format(L",|%d", top);
			}
			else if(pRawData->posTop.pit == PIT_PERCENT){
				//support percent mode
                int top = rcNew.top - rcParent.top;
				if (pRawData->fOffsetY != 0.0f)
                {
                    top -= nHeight * pRawData->fOffsetY;
                }
				top = top * 100 / rcParent.Height();
				pRawData->posTop.nPos = top;
				strPos += SStringW().Format(L",%%%d", top);
			}else{
				pRawData->posTop.pit = PIT_NORMAL;
				if(pRawData->posTop.bMinus){
					int pos = rcParent.bottom - rcNew.top;
					if (pRawData->fOffsetY != 0.0f)
                    {
                        pos += nHeight * pRawData->fOffsetY;
                    }
					pRawData->posTop.nPos = pos;
					strPos += SStringW().Format(L",-%d", pos);
				}else{
					int pos = rcNew.top - rcParent.top;
					if (pRawData->fOffsetY != 0.0f)
                    {
                        pos -= nHeight * pRawData->fOffsetY;
                    }
					pRawData->posTop.nPos = pos;
					strPos += SStringW().Format(L",%d", pos);
				}
			}
            
			if(pRawData->nCount == 2)
			{
				if(pLayoutParam->IsSpecifiedSize(Horz))
				{
					strPos += SStringW().Format(L",@%d",nWidth);
				}	
				if(pLayoutParam->IsSpecifiedSize(Vert))
				{
					strPos += SStringW().Format(L",@%d",nHeight);
				}
			}
		}
		if(pRawData->nCount == 4)
		{//update right and bottom
			if(pRawData->posRight.pit == PIT_SIZE)
			{
				//size mode
				pRawData->posRight.nPos = nWidth;
				strPos += SStringW().Format(L",@%d",nWidth);
			}else if(pRawData->posRight.pit == PIT_CENTER)
			{
				//center mode
				int pos = rcNew.right - rcParent.CenterPoint().x;
				if (pRawData->fOffsetX != 0.0f)
                {
                    pos -= nWidth * pRawData->fOffsetX;
                }
				pRawData->posRight.nPos = pos;
				strPos += SStringW().Format(L",|%d", pos);
			}else if(pRawData->posRight.pit == PIT_PERCENT)
			{
				//percent mode
				int pos = rcNew.right - rcParent.left;
				if (pRawData->fOffsetX != 0.0f)
                {
                    pos -= nWidth * pRawData->fOffsetX;
                }
				pos = pos * 100 / rcParent.Width();
				pRawData->posRight.nPos = pos;
				strPos += SStringW().Format(L",%%%d", pos);
			}else
			{
				//normal mode
				pRawData->posRight.pit = PIT_NORMAL;
				if(pRawData->posRight.bMinus){
					int pos = rcParent.right - rcNew.right;
					if (pRawData->fOffsetX != 0.0f)
                    {
                        pos += nWidth * pRawData->fOffsetX;
                    }
					pRawData->posRight.nPos = pos;
                    strPos += SStringW().Format(L",-%d", pos);
				}else{
					int pos = rcNew.right - rcParent.left;
					if (pRawData->fOffsetX != 0.0f)
                    {
                        pos -= nWidth * pRawData->fOffsetX;
                    }
					pRawData->posRight.nPos = pos;
                    strPos += SStringW().Format(L",%d", pos);
				}
			}
			if(pRawData->posBottom.pit == PIT_SIZE)
			{
				pRawData->posBottom.nPos = nHeight;
				strPos += SStringW().Format(L",@%d",nHeight);
			}
			else if(pRawData->posBottom.pit == PIT_CENTER){
				//center mode
				int pos = rcNew.bottom - rcParent.CenterPoint().y;
				if (pRawData->fOffsetY != 0.0f)
                {
                    pos -= nHeight * pRawData->fOffsetY;
                }
				pRawData->posBottom.nPos = pos;
				strPos += SStringW().Format(L",|%d", pos);
			}
			else if(pRawData->posBottom.pit == PIT_PERCENT){
				//percent mode
				int pos = rcNew.bottom - rcParent.top;
				if (pRawData->fOffsetY != 0.0f)
                {
                    pos -= nHeight * pRawData->fOffsetY;
                }
				pos = pos * 100 / rcParent.Height();
				pRawData->posBottom.nPos = pos;
				strPos += SStringW().Format(L",%%%d", pos);
			}else{
				pRawData->posBottom.pit = PIT_NORMAL;
				if(pRawData->posBottom.bMinus){
					int pos = rcParent.bottom - rcNew.bottom;
					if (pRawData->fOffsetY != 0.0f)
                    {
                        pos += nHeight * pRawData->fOffsetY;
                    }
					pRawData->posBottom.nPos = pos;
                    strPos += SStringW().Format(L",-%d", pos);
				}else{
					int pos = rcNew.bottom - rcParent.top;
					if (pRawData->fOffsetY != 0.0f)
					{
						pos -= nHeight * pRawData->fOffsetY;
					}
					pRawData->posBottom.nPos = pos;
					strPos += SStringW().Format(L",%d",pos);
				}
			}
		}
		pTarget->SetAttribute(L"pos",strPos,FALSE);
		m_pListener->OnUpdatePos(strPos,TRUE);
    }
    else
    {
        SLOGW()<<"unknown layout type";
    }
    pParent->RequestRelayout();
	
	GetRoot()->UpdateLayout();
	m_pSel->Move(pTarget->GetWindowRect());
}

void CPreviewHost::OnSize(UINT nType, CSize size)
{
	SHostWnd::OnSize(nType,size);
	if(m_pListener) 
		m_pListener->OnResize();
}

void CPreviewHost::OnExitSizeMove()
{
	if(m_pListener) 
		m_pListener->OnResize();
}
void CPreviewHost::Reload()
{
	OnDestroy();
	CREATESTRUCT cs;
	cs.cx = 0;
	cs.cy = 0;
	OnCreate(&cs);
	CRect rcWnd = GetWindowRect();
	OnSize(0,rcWnd.Size());
	//todo: keep tab visible pages same as before.

}

LRESULT CPreviewHost::OnNcMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(!m_pListener || wParam != HTCAPTION)
		SetMsgHandled(FALSE);
	return 0;
}

void CPreviewHost::UpdateLayoutBuf(SStringA strUtf8)
{
	m_utf8Buffer = strUtf8;
	Reload();
}
