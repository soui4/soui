#pragma once
#include "PreviewHost.h"

class CPreviewContainer : public SNativeWnd,IListener
{
	CPreviewHost	m_previewHost;
	HBITMAP			m_hBgBmp;
public:
	CPreviewContainer(LPCTSTR pszLayoutId, HWND hEditor = 0);
	~CPreviewContainer(void);

protected:
	virtual void OnEditorExit() override;
	virtual void OnResize() override;
	virtual void OnRePos(const POINT *pt);
protected:
	CPoint GetViewPos() const;
	void UpdateViewPos();
protected:
	void SetScrollMax(int hmax, int vmax) const;
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnScroll(int nBar,int nSBCode, int nPos, HWND hScrollBar);
	void OnHScroll(int nSBCode, int nPos, HWND hScrollBar);
	void OnVScroll(int nSBCode, int nPos, HWND hScrollBar);
	void OnClose();
	void OnPaint(HDC hdc);
	void OnSize(UINT nType, CSize size);
	BOOL OnEraseBkgnd(HDC hdc);
	BEGIN_MSG_MAP_EX(CPreviewContainer)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_HSCROLL(OnHScroll)
		MSG_WM_VSCROLL(OnVScroll)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		CHAIN_MSG_MAP(SNativeWnd)
	END_MSG_MAP()


};

