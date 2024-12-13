#pragma once

#include "apppublic.h"

#include "CBitmap.h"
#include "CGdiDc.h"

class SSnapshotCtrl : public SWindow
{
	DEF_SOBJECT(SWindow, L"SnapshotCtrl")
public:
	SSnapshotCtrl(void);
	~SSnapshotCtrl(void);

protected:
	void OnPaint(IRenderTarget* pRT);
	void OnMouseMove(UINT nFlags, SOUI::CPoint point);
	void OnLButtonDown(UINT nFlags, SOUI::CPoint point);
	void OnLButtonUp(UINT nFlags, SOUI::CPoint point);
	void OnLButtonDblClk(UINT nFlags, SOUI::CPoint point);
	void OnRButtonUp(UINT nFlags, CPoint point);

	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
		MSG_WM_RBUTTONUP(OnRButtonUp)
	SOUI_MSG_MAP_END()

public:
	void SetBmpResource(CBitmap* pBmp);

private:
	CBitmap* m_pBitmap;

	SOUI::CPoint	m_ptDown;				//鼠标按下的位置
	SOUI::CRect		m_rcCaptureArea;		//截图区域

	EcPosType		m_emPosType;
	bool	m_bCapture;
};

