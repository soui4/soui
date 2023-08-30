#include "stdafx.h"
#include "FpsWnd.h"
#include <Mmsystem.h>
#pragma comment(lib,"Winmm.lib")

SNSBEGIN

FpsWnd::FpsWnd(void)
{
	timeBeginPeriod(1);
}

FpsWnd::~FpsWnd(void)
{
	timeEndPeriod(1);
}

void FpsWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__baseCls::OnShowWindow(bShow,nStatus);
	if(IsVisible(TRUE))
		GetContainer()->RegisterTimelineHandler(this);
	else
		GetContainer()->UnregisterTimelineHandler(this);
}

void FpsWnd::OnNextFrame (THIS)
{
	Invalidate();
}

void FpsWnd::OnPaint(IRenderTarget *pRT)
{
	SPainter painter;
	BeforePaint(pRT,painter);

	DWORD now = timeGetTime();
	m_lstRenderTs.push_back(now);
	for(;!m_lstRenderTs.empty();)
	{
		DWORD tsHead = m_lstRenderTs.front();
		if(now-tsHead<=1000)
			break;
		m_lstRenderTs.pop_front();
	}
	UINT fps = (UINT)m_lstRenderTs.size();
	SStringT str = SStringT().Format(_T("fps:%u"),fps);
	CRect rc = GetClientRect();
	pRT->DrawText(str,str.GetLength(),&rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);

	AfterPaint(pRT,painter);
}


SNSEND
