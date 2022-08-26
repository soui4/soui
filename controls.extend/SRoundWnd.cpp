#include "stdafx.h"
#include "SRoundWnd.h"

SNSBEGIN

SRoundWnd::SRoundWnd(void):m_cornerSize(0)
{
	m_bLayeredWindow = TRUE;
}

SRoundWnd::~SRoundWnd(void)
{
}

void SRoundWnd::OnCommitSurface(IRenderTarget *pRtDest, LPCRECT pRcDest, IRenderTarget *pRtSrc, LPCRECT pRcSrc, BYTE alpha)
{
	SAutoRefPtr<IRenderTarget> pRT; 
	CRect rc = GetClientRect();
	GETRENDERFACTORY->CreateRenderTarget(&pRT, rc.Width(), rc.Height());
	pRT->OffsetViewportOrg(-rc.left,-rc.top);
	pRT->ClearRect(&rc, 0);
	SAutoRefPtr<IBrushS> br;
	pRT->CreateSolidColorBrush(RGBA(0xBA, 0xB3, 0x99, 0xFF), &br);
	pRT->SelectObject(br,NULL);
	if(m_cornerSize==0){
		pRT->FillEllipse(&rc);
	}else{
		pRT->FillRoundRect(&rc,CPoint(m_cornerSize,m_cornerSize));
	}
	pRT->SetXfermode(kSrcIn_Mode,NULL);
	pRT->AlphaBlend(&rc,pRtSrc,&rc,255);
	pRtDest->AlphaBlend(pRcDest,pRT,&rc,alpha);
}

SNSEND
