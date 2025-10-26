#include "stdafx.h"
#include "DesignWnd.h"
#include "../XmlEditor.h"

SNSBEGIN
CDesignWnd::CDesignWnd(LPCTSTR pszLayoutId)
:SHostWnd(pszLayoutId)
,m_previewHost(NULL)
,m_pXmlEditor(NULL)
,m_nDelta(0)
{
}

CDesignWnd::~CDesignWnd(void)
{
}

BOOL CDesignWnd::Init(LPCTSTR pszLayoutId,HWND hWndParent)
{
	if(m_previewHost != NULL)
	{
		Close();
	}
	m_previewHost = new CPreviewHost(this,pszLayoutId,hWndParent);
	m_previewHost->CreateEx(m_hWnd,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,0,0,0,0,0,NULL);
	UpdateViewPos();
    return TRUE;
}

void CDesignWnd::UpdateLayoutBuf(SStringA strUtf8)
{
	if(m_previewHost)
		m_previewHost->UpdateLayoutBuf(strUtf8);
}

void CDesignWnd::SelectCtrlByOrder(const int *pOrder, int nLen)
{
	if(m_previewHost)
		m_previewHost->SelectCtrlByOrder(pOrder,nLen);
}

void CDesignWnd::Close()
{
	if(m_previewHost){
		m_previewHost->DestroyWindow();
		delete m_previewHost;
		m_previewHost = NULL;
	}
}

static const int KCanvas_Size = 4096;

void CDesignWnd::SetScrollMax(int hmax, int vmax) const
{
	SCROLLINFO si = { 0 };
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = -hmax / 2;
	si.nMax = hmax / 2;
	si.nPage = 100;
	si.nPos = 0;
	SetScrollInfo(m_hWnd, SB_HORZ, &si, TRUE);
	si.nPos = 0;
	si.nMin = -vmax / 2;
	si.nMax = vmax / 2;
	SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);
}

int CDesignWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int ret = SHostWnd::OnCreate(lpCreateStruct);
	m_pNcPainter->SetAttribute(L"system", L"1", TRUE);// enable system scrollbar
	SetScrollMax(KCanvas_Size, KCanvas_Size);
	return ret;
}

void CDesignWnd::OnHScroll(int nSBCode, int nPos, HWND hScrollBar)
{
	OnScroll(SB_HORZ,nSBCode,nPos,hScrollBar);
}

void CDesignWnd::OnVScroll(int nSBCode, int nPos, HWND hScrollBar)
{
	OnScroll(SB_VERT,nSBCode,nPos,hScrollBar);
}

void CDesignWnd::OnScroll(int nBar,int nSBCode, int nPos, HWND hScrollBar)
{
	SCROLLINFO si={sizeof(si),SIF_ALL,0};
	GetScrollInfo(m_hWnd,nBar,&si);
	int nNewPos = si.nPos;
	switch(nSBCode)
	{
	case SB_THUMBTRACK:
		nNewPos=nPos;
		break;
	case SB_THUMBPOSITION:
		nNewPos=nPos;
		break;
	case SB_LINEDOWN:
		nNewPos++;
		break;
	case SB_LINEUP:
		nNewPos--;
		break;
	case SB_PAGEUP:
		nNewPos-=si.nPage;
		break;
	case SB_PAGEDOWN:
		nNewPos+=si.nPage;
		break;
	}
	nNewPos = smin(nNewPos,(int)(si.nMax-si.nPage));
	nNewPos = smax(si.nMin,nNewPos);
	SetScrollPos(m_hWnd,nBar,nNewPos,FALSE);

	UpdateViewPos();
}

void CDesignWnd::OnSize(UINT nType, CSize size)
{
	SHostWnd::OnSize(nType,size);
	UpdateViewPos();
}

CPoint CDesignWnd::GetViewPos() const
{
	if(m_previewHost == NULL)
		return CPoint(0,0);
	CRect rcPreview=m_previewHost->GetWindowRect();
	CRect rcHost=GetClientRect();
	int x = GetScrollPos(m_hWnd,SB_HORZ);
	int y = GetScrollPos(m_hWnd,SB_VERT);

	CPoint ptCenter(-x,-y);
	CSize szHost = rcHost.Size()/2;
	CSize szHalf = rcPreview.Size()/2;
	ptCenter.x += szHost.cx-szHalf.cx;
	ptCenter.y += szHost.cy-szHalf.cy;
	
	return ptCenter;
}

void CDesignWnd::UpdateViewPos()
{
	if(m_previewHost == NULL)
		return;
	CPoint pt = GetViewPos();
	m_previewHost->SetWindowPos(0,pt.x,pt.y,0,0,SWP_NOZORDER|SWP_NOSIZE);
}

void CDesignWnd::OnResize()
{
	UpdateViewPos();
}

void CDesignWnd::OnRePos(const POINT *pt)
{
	SetWindowPos(0,pt->x,pt->y,0,0,SWP_NOSIZE|SWP_NOZORDER);
}

BOOL CDesignWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	m_nDelta += zDelta;
	int nDelta = m_nDelta / WHEEL_DELTA; // 1个滚动单位为120个delta值
	m_nDelta %= WHEEL_DELTA; // 一个滚动单位的delta值，下次滚动时，需要减去这个值，才能得到正确的滚动值。
	for(int i=0, cnt=abs(nDelta);i<cnt;i++){
		if(GetKeyState(VK_MENU)&0x8000)
		{
			OnScroll(SB_HORZ,zDelta>0?SB_LINEUP:SB_LINEDOWN,0,NULL);
		}else{
			OnScroll(SB_VERT,zDelta>0?SB_LINEUP:SB_LINEDOWN,0,NULL);
		}
	}
	return TRUE;
}
void CDesignWnd::OnSelectedCtrl(const int *pOrder, int nLen)
{
	m_pXmlEditor->OnSelectedCtrl(pOrder,nLen);
}

void CDesignWnd::OnUpdateSize(int nWidth, int nHeight)
{
	m_pXmlEditor->OnUpdateSize(nWidth,nHeight);
}

void CDesignWnd::OnUpdatePos(SStringW strPos)
{
	m_pXmlEditor->OnUpdatePos(strPos);
}

SNSEND