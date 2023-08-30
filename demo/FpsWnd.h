#pragma once
#include <list>
SNSBEGIN

class FpsWnd : public SWindow, ITimelineHandler
{
	DEF_SOBJECT(SWindow,L"fps")
public:
	FpsWnd(void);
	~FpsWnd(void);

protected:
	virtual STDMETHODIMP_(void) OnNextFrame(THIS);

protected:
	void OnShowWindow( BOOL bShow, UINT nStatus );
	void OnPaint(IRenderTarget *pRT);

	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)
		MSG_WM_SHOWWINDOW(OnShowWindow)
	SOUI_MSG_MAP_END()
	std::list<DWORD> m_lstRenderTs;	
};

SNSEND
