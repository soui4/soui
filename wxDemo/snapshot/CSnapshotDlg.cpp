#include "stdafx.h"
#include "CSnapshotDlg.h"

CSnapshotDlg::CSnapshotDlg()
	: SHostDialog(UIRES.LAYOUT.XML_DLG_SNAPSHOT)
{
}

CSnapshotDlg::~CSnapshotDlg(void)
{
}


BOOL CSnapshotDlg::OnInitDialog(HWND wnd, LPARAM lInitParam)
{
	//获取屏幕大小
	DEVMODE dm;
	ZeroMemory(&dm, sizeof(dm));
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

	int nxScreen, nyScreen;
#ifndef DEBUG
// 	nxScreen = 400;
// 	nyScreen = 400;

	nxScreen = dm.dmPelsWidth;
	nyScreen = dm.dmPelsHeight;
#else
	nxScreen = dm.dmPelsWidth;
	nyScreen = dm.dmPelsHeight;
#endif // !DEBUG

	::SetWindowPos(this->m_hWnd, HWND_TOPMOST, 0, 0, nxScreen, nyScreen, SWP_SHOWWINDOW);
	SetForegroundWindow(this->m_hWnd);

	CDC hScrDC, hMemDC;
	HBITMAP   hBitmap, hOldBitmap;
	hScrDC = CreateDC(L"DISPLAY", NULL, NULL, NULL);
	hMemDC = CreateCompatibleDC(hScrDC);
	hBitmap = CreateCompatibleBitmap(hScrDC, nxScreen, nyScreen);
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, nxScreen, nyScreen, hScrDC, 0, 0, SRCCOPY | CAPTUREBLT);
	hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

	SSnapshotCtrl* pSnapshot = FindChildByName2<SSnapshotCtrl>(L"snapshot");
	SASSERT(pSnapshot);
	pSnapshot->SetBmpResource(new CBitmap(hBitmap));

	return TRUE;
}

void CSnapshotDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_ESCAPE)
		EndDialog(IDCANCEL);
	else
		SetMsgHandled(FALSE);
}

void CSnapshotDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
// 	SSnapshotCtrl* pSnapshot = FindChildByName2<SSnapshotCtrl>(L"snapshot");
// 	SASSERT(pSnapshot);
// 	if (pSnapshot->CanRevokeOperate())
// 	{
// 		pSnapshot->RevokeOperate();
// 	}
// 	else
// 		EndDialog(IDCANCEL);
}