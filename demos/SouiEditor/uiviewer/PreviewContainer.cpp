#include "stdafx.h"
#include "PreviewContainer.h"
#include "resource.h"
#include "bmp.hpp"

static HBITMAP LoadBitmapFromMemory(const BYTE* data, DWORD dataSize)
{
	HDC hDC = GetDC(0);
	const BITMAPFILEHEADER *pBmpFileHeader=(const BITMAPFILEHEADER *)data; 
	if (pBmpFileHeader->bfType != ((WORD) ('M'<<8)|'B')) 
	{
		return NULL; 
	} 
	if (pBmpFileHeader->bfSize > dataSize) 
	{ 
		return NULL; 
	} 
	const LPBITMAPINFO lpBitmap=(const LPBITMAPINFO)(pBmpFileHeader+1); 
	const BYTE * lpBits=data+pBmpFileHeader->bfOffBits;
	HBITMAP hBitmap= CreateDIBitmap(hDC,&lpBitmap->bmiHeader,CBM_INIT,lpBits,lpBitmap,DIB_RGB_COLORS);
	ReleaseDC(0,hDC);
	return hBitmap;
}


CPreviewContainer::CPreviewContainer(LPCTSTR pszLayoutId, HWND hEditor)
:m_previewHost(this,pszLayoutId,hEditor)
{
	m_hBgBmp = LoadBitmapFromMemory(bg_bmp,ARRAYSIZE(bg_bmp));
}

CPreviewContainer::~CPreviewContainer(void)
{
	DeleteObject(m_hBgBmp);
}

static const int KCanvas_Size = 4096;

void CPreviewContainer::SetScrollMax(int hmax, int vmax) const
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

int CPreviewContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetScrollMax(KCanvas_Size, KCanvas_Size);

	m_previewHost.CreateEx(m_hWnd,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,0,0,0,0,0,NULL);
	CRect rcPreview = m_previewHost.GetWindowRect();

	CRect rcHost;
	GetClientRect(&rcHost);
	
	if(rcHost.Width()<rcPreview.Width())
	{
		SetScrollPos(m_hWnd,SB_HORZ,(rcHost.Width()-rcPreview.Width())/2,FALSE);
	}
	if(rcHost.Height()<rcPreview.Height())
	{
		SetScrollPos(m_hWnd,SB_VERT,(rcHost.Height()-rcPreview.Height())/2,FALSE);
	}
	
	return 0;
}

void CPreviewContainer::OnHScroll(int nSBCode, int nPos, HWND hScrollBar)
{
	OnScroll(SB_HORZ,nSBCode,nPos,hScrollBar);
}

void CPreviewContainer::OnVScroll(int nSBCode, int nPos, HWND hScrollBar)
{
	OnScroll(SB_VERT,nSBCode,nPos,hScrollBar);
}

void CPreviewContainer::OnScroll(int nBar,int nSBCode, int nPos, HWND hScrollBar)
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

void CPreviewContainer::OnClose()
{
	DestroyWindow();
	PostQuitMessage(0);
}

void CPreviewContainer::OnEditorExit()
{
	PostMessage(WM_CLOSE);
	//OnClose();
}

void CPreviewContainer::OnPaint(HDC hdc)
{
	PAINTSTRUCT ps;
	hdc = BeginPaint(m_hWnd,&ps);
	CRect rc;
	GetClientRect(rc);

	HBRUSH hBr = CreatePatternBrush(m_hBgBmp);
	FillRect(hdc,&rc,hBr);
	DeleteObject(hBr);

	EndPaint(m_hWnd,&ps);
}

void CPreviewContainer::OnSize(UINT nType, CSize size)
{
	UpdateViewPos();
}

CPoint CPreviewContainer::GetViewPos() const
{
	CRect rcPreview=m_previewHost.GetWindowRect();
	CRect rcHost;
	GetClientRect(&rcHost);
	int x = GetScrollPos(m_hWnd,SB_HORZ);
	int y = GetScrollPos(m_hWnd,SB_VERT);

	CPoint ptCenter(-x,-y);
	CSize szHost = rcHost.Size()/2;
	CSize szHalf = rcPreview.Size()/2;
	ptCenter.x += szHost.cx-szHalf.cx;
	ptCenter.y += szHost.cy-szHalf.cy;
	
	return ptCenter;
}

void CPreviewContainer::UpdateViewPos()
{
	CPoint pt = GetViewPos();
	m_previewHost.SetWindowPos(0,pt.x,pt.y,0,0,SWP_NOZORDER|SWP_NOSIZE);
}

void CPreviewContainer::OnResize()
{
	UpdateViewPos();
}

void CPreviewContainer::OnRePos(const POINT *pt)
{
	SetWindowPos(0,pt->x,pt->y,0,0,SWP_NOSIZE|SWP_NOZORDER);
}

BOOL CPreviewContainer::OnEraseBkgnd(HDC hdc)
{
	return TRUE;
}
