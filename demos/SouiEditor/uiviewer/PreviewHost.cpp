// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PreviewHost.h"	
#include "../uieditor/Global.h"
#include "Adapter.h"
#include "helper/SplitString.h"
#define kLogTag "CPreviewHost"


static DWORD		g_dwLastMsgTime;

void (CALLBACK cb_HeartTimer)(HWND hwnd, UINT msg, UINT_PTR timerId, DWORD ts)
{
	if ((GetTickCount() - g_dwLastMsgTime) > 5000)
	{
		PostQuitMessage(2);
	}
}

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

LRESULT CPreviewHost::OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(GetKeyState(VK_MENU)&0x80)
	{
		SetMsgHandled(FALSE);
		return TRUE;
	}else
	{
		CPoint pt(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		if(uMsg == WM_LBUTTONDOWN)
		{//tell editor the clicked window id.
			m_pHover->SetVisible(FALSE);
			m_pSel->SetVisible(FALSE);
			SWND swnd = GetRoot()->SwndFromPoint(pt,true);
			m_pSel->SetVisible(TRUE);
			m_pHover->SetVisible(TRUE);
			if(swnd)
			{
				SWindow *p = SWindowMgr::GetWindow(swnd);
				CRect rcSel = p->GetWindowRect();
				m_pSel->Move(rcSel);
				SList<int> lstIndex;
				GetSwndIndex(p,lstIndex);
				if(m_bVirtualRoot)
				{
					lstIndex.RemoveHead();
				}
				if(m_hOwner)
				{
					COPYDATASTRUCT cds;
					cds.dwData = select_id;
					cds.cbData = sizeof(int)*lstIndex.GetCount();
					int *pData = new int[lstIndex.GetCount()];
					int i=0;
					SPOSITION pos = lstIndex.GetHeadPosition();
					while(pos)
					{
						pData[i++]=lstIndex.GetNext(pos);
					}
					cds.lpData = pData;
					::SendMessage(m_hOwner,WM_COPYDATA,(WPARAM)m_hWnd,(LPARAM)&cds);
					delete []pData;
				}
			}
		}else if(uMsg == WM_MOUSEMOVE)
		{//highlight the hovered window
			m_pHover->SetVisible(FALSE);
			SWND swnd = GetRoot()->SwndFromPoint(pt,true);
			m_pHover->SetVisible(TRUE);
			if(swnd)
			{
				SWindow *p = SWindowMgr::GetWindow(swnd);
				CRect rc = p->GetWindowRect();
				m_pHover->Move(rc);
			}else
			{
				m_pHover->Move2(0,0,0,0);
			}
		}

		return TRUE;
	}
}

void CPreviewHost::SelectCtrlByOrder(int *pOrder,int nLen)
{
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

		if (i == nLen - 1)
		{
			CRect rcSel = pChild->GetWindowRect();
			m_pSel->Move(rcSel);
			m_pSel->SetVisible(TRUE);
		}
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
	BOOL bLoaded = FALSE;
	if (!m_utf8Buffer.IsEmpty())
	{
		bLoaded = xmlDoc.load_buffer(m_utf8Buffer.c_str(), m_utf8Buffer.GetLength(), xml_parse_default, enc_utf8);
	}
	else
	{
		bLoaded = LOADXML(xmlDoc, m_strXmlLayout);
	}
	if (bLoaded)
	{
		m_iRootIndex = 0;
		SXmlNode xmlSoui = xmlDoc.root().child(L"SOUI");
		if (xmlSoui)
		{
			SXmlNode xmlSib = xmlSoui.child(L"root").previous_sibling();
			while (xmlSib)
			{
				m_iRootIndex++;
				xmlSib = xmlSib.previous_sibling();
			}

			m_bSOUIWnd = TRUE;
			xmlSoui.attribute(L"translucent").set_value(0);
			xmlSoui.attribute(L"wndType").set_value(L"normal");
			return InitFromXml(&xmlSoui);
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
				int inc_width = xmlInclude.attribute(L"width").as_int(600);
				int inc_height = xmlInclude.attribute(L"height").as_int(400);
				SXmlNode doc_root = xmlDoc2.root().child(L"soui").child(L"root");
				doc_root.attribute(L"width").set_value(inc_width);
				doc_root.attribute(L"height").set_value(inc_height);
				SXmlNode xmlRoot = xmlDoc2.root().first_child();
				InitFromXml(&xmlRoot);
				GetRoot()->CreateChildren(&xmlInclude);
				return TRUE;
			}
			else
			{
				m_bVirtualRoot = TRUE;
				SXmlNode xmlRoot2 = xmlDoc2.root().first_child().first_child();
				xmlRoot2.append_copy(xmlDoc.root().first_child());
				SXmlNode xmlRoot = xmlDoc2.root().first_child();
				return InitFromXml(&xmlRoot);
			}
		}
	}
	else
	{
		SLOGFMTI("Load layout [%s] Failed", S_CT2A(m_strXmlLayout).c_str());
		return FALSE;
	}
}

BOOL CPreviewHost::OnCopyData(HWND wnd, PCOPYDATASTRUCT pCopyDataStruct)
{
	SLOGI()<<"OnCopyData, dwData="<<pCopyDataStruct->dwData<<" cbData="<<pCopyDataStruct->cbData;
	if(pCopyDataStruct->dwData == update_buf_id)
	{
		m_utf8Buffer = SStringA((const char*)pCopyDataStruct->lpData,pCopyDataStruct->cbData);
		Reload();
	}else if(pCopyDataStruct->dwData == update_file_id)
	{
		Reload();
	}
	else if (pCopyDataStruct->dwData == exitviewer_id)
	{
		m_pListener->OnEditorExit();
	}
	else if (pCopyDataStruct->dwData == selctrl_id)
	{
		int * pOrder = (int*)pCopyDataStruct->lpData;
		int nLen = pCopyDataStruct->cbData/sizeof(int);
		SelectCtrlByOrder(pOrder,nLen);
	}else if(pCopyDataStruct->dwData == setviewerpos_id)
	{
		POINT * pt = (POINT*)pCopyDataStruct->lpData;
		m_pListener->OnRePos(pt);
	}
	return TRUE;
}

int CPreviewHost::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int nRet = SHostWnd::OnCreate(lpCreateStruct);
	if(nRet==0)
	{
		const WCHAR *pszXml=L"<window name=\"_preview_sel_frame\" float=\"1\" msgTransparent=\"1\" margin=\"2,2,2,2\" colorBorder=\"#ff0000\"/>"
			L"<window float=\"1\" name=\"_preview_hover_frame\" msgTransparent=\"1\" margin=\"1,1,1,1\" colorBorder=\"#0000ff\"/>";
		GetRoot()->CreateChildrenFromXml(pszXml);
		m_pSel = FindChildByName("_preview_sel_frame");
		m_pHover = FindChildByName("_preview_hover_frame");
	}

#ifdef _WIN32
	if(!m_hOwner)
	{
		m_hOwner = FindWindowEx(NULL,NULL,ksz_editor_cls,ksz_editor_wnd);
	}
	if(!m_hOwner)
	{
		m_hOwner = FindWindowEx(NULL,NULL,ksz_uipreview_cls,ksz_uipreview_wnd);
	}
#endif//_WIN32
	if (m_hOwner)
	{
		SLOGI()<<"post kmsg_viewer_create to owner, owner="<<m_hOwner<<" m_hWnd="<<m_hWnd;
		g_dwLastMsgTime = GetTickCount();
		::PostMessage(m_hOwner, kmsg_viewer_create, (WPARAM)m_hWnd, 0);
		::SetTimer(0,0,1000,cb_HeartTimer);
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
		{//todo:

		}
	}
}

void CPreviewHost::OnSize(UINT nType, CSize size)
{
	SHostWnd::OnSize(nType,size);
}

void CPreviewHost::OnExitSizeMove()
{
	m_pListener->OnResize();
}

LRESULT CPreviewHost::OnHeart(UINT uMsg, WPARAM wp, LPARAM lp)
{
	g_dwLastMsgTime = GetTickCount();
	return 0;
}

void CPreviewHost::OnDestroy()
{
	if(m_hOwner)
	{
		::PostMessage(m_hOwner,kmsg_viewer_destroy,0,0);
	}
	SetMsgHandled(FALSE);
}

void CPreviewHost::Reload()
{
	CRect rcWnd = GetWindowRect();
	OnDestroy();
	CREATESTRUCT cs;
	cs.cx = 0;
	cs.cy = 0;
	OnCreate(&cs);

	OnSize(0,rcWnd.Size());
	//todo: keep tab visible pages same as before.

}

LRESULT CPreviewHost::OnNcMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(wParam != HTCAPTION)
		SetMsgHandled(FALSE);
	return 0;
}
