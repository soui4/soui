#include "stdafx.h"
#include "MainDlg.h"    
#include "LoginDlg.h"
#include <helper/SMenuEx.h>
#include <helper/SFunctor.hpp>
#include <helper/slog.h>
#include <mmsystem.h>
#define kLogTag "MainDlg"

CMainDlg::CMainDlg(SGameTheme* pTheme) 
: SHostWnd(_T("LAYOUT:XML_MAINWND"))
, m_pTheme(pTheme)
, m_bMute(FALSE)
{
    m_pChessGame = new CChessGame(this, pTheme);
    m_pLobbyHandler = new LobbyHandler();
    m_webSocketClient.SetMessageHandler(this);
}

CMainDlg::~CMainDlg()
{
    delete m_pChessGame;
    delete m_pLobbyHandler;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
    CLoginDlg dlgLogin;
    if (dlgLogin.DoModal() != IDOK) {
        OnClose();
        return FALSE;
    }
    
    SStringT strTitle = SStringT().Format(_T("中国象棋"));
    FindChildByName(L"txt_title")->SetWindowText(strTitle);
    SetWindowText(strTitle);
    m_pLobbyHandler->Init(FindChildByName(L"room_container"), &m_webSocketClient);
    m_pChessGame->Init(FindChildByName(L"game_container"), &m_webSocketClient);

    SStringA svr = S_CT2A(dlgLogin.m_strSvr);
    BOOL bRet = m_webSocketClient.ConnectToServer(svr, "");
    SLOGI()<<"connect to server ret:"<<bRet;
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