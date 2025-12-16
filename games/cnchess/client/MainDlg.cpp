#include "stdafx.h"
#include "MainDlg.h"

CMainDlg::CMainDlg()
    : SHostWnd(_T("layout:dlg_main"))
    , m_pChessGame(NULL)
{
}

CMainDlg::~CMainDlg()
{
    if (m_pChessGame)
    {
        delete m_pChessGame;
        m_pChessGame = NULL;
    }
}

BOOL CMainDlg::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
    // 获取游戏容器窗口
    SWindow* pGameContainer = FindChildByName(L"game_container");
    if (pGameContainer)
    {
        m_pChessGame = new ChessGame(this);
        m_pChessGame->Init(pGameContainer);
    }
    
    return TRUE;
}

void CMainDlg::OnClose()
{
    CSimpleWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
    SendFrameNofityState(WN_MAXIMIZE);
}

void CMainDlg::OnRestore()
{
    SendFrameNofityState(WN_RESTORE);
}

void CMainDlg::OnMinimize()
{
    SendFrameNofityState(WN_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
    SendFrameNofitySize(nType, size);
    SetMsgHandled(FALSE);
}

void CMainDlg::OnScaleChanged(int nScale)
{
    SDpiHandler<CMainDlg>::OnScaleChanged(nScale);
}