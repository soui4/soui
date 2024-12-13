#pragma once
#include "CBitmap.h"
#include "CGdiDc.h"

#include "SSnapshotCtrl.h"
class CSnapshotDlg : public SHostDialog
{
public:
	~CSnapshotDlg(void);
	CSnapshotDlg(void);

protected:
	BOOL OnInitDialog(HWND wnd, LPARAM lInitParam);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnRButtonUp(UINT nFlags, CPoint point);

	BEGIN_MSG_MAP_EX(CSnapshotDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_RBUTTONUP(OnRButtonUp)
		CHAIN_MSG_MAP(SHostDialog)
		END_MSG_MAP()
};

