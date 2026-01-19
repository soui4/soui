// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"    
#include "LoginDlg.h"
#include <junqiProtocol.h>
#include <helper/SMenuEx.h>
#include <helper/slog.h>
#include <mmsystem.h>
#define kLogTag "MainDlg"
#ifdef _WIN32
#include "win32_audio.h"
#endif
CMainDlg::CMainDlg(SGameTheme* pTheme) 
: SHostWnd(_T("LAYOUT:XML_MAINWND"))
, m_pTheme(pTheme)
, m_bMute(FALSE)
, m_webSocketClient(GetMsgLoop())
{
    m_pGame = new CJunqiGame(this,pTheme);
    m_pLobbyHandler = new LobbyHandler();
    m_webSocketClient.SetMessageHandler(this);
}

CMainDlg::~CMainDlg()
{
    delete m_pGame;
    delete m_pLobbyHandler;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
    CLoginDlg dlgLogin;
    if (dlgLogin.DoModal() != IDOK) {
        OnClose();
        return FALSE;
    }
    MyProfile* myProfile = MyProfile::getSingletonPtr();
    myProfile->SetSex(dlgLogin.m_cSex);
    myProfile->SetName(dlgLogin.m_strName);

    SStringT strTitle = SStringT().Format(_T("用户:%s"), myProfile->GetName().c_str());
    FindChildByName(L"txt_title")->SetWindowText(strTitle);
    SetWindowText(strTitle);
    m_pLobbyHandler->Init(FindChildByName(L"room_container"), &m_webSocketClient);
    m_pGame->Init(FindChildByName(L"game_container"), &m_webSocketClient);

    SStringA svr = S_CT2A(dlgLogin.m_strSvr);
    BOOL bRet = m_webSocketClient.ConnectToServer(svr, "");
    SLOGI()<<"connect to server ret:"<<bRet;
    return TRUE;
}

//TODO:消息映射
void CMainDlg::OnClose()
{
    SNativeWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
    SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}

void CMainDlg::OnRestore()
{
    SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}

void CMainDlg::OnMinimize()
{
    SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
    SetMsgHandled(FALSE);   
    SWindow *pBtnMax = FindChildByName(L"btn_max");
    SWindow *pBtnRestore = FindChildByName(L"btn_restore");
    if(!pBtnMax || !pBtnRestore) return;
    
    if (nType == SIZE_MAXIMIZED)
    {
        pBtnRestore->SetVisible(TRUE);
        pBtnMax->SetVisible(FALSE);
    }
    else if (nType == SIZE_RESTORED)
    {
        pBtnRestore->SetVisible(FALSE);
        pBtnMax->SetVisible(TRUE);
    }
}
void CMainDlg::OnScaleChanged(int nScale)
{
}

BOOL CMainDlg::OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE>> data)
{
    BOOL bRet = FALSE;
    const BYTE *pData = data?data->data():NULL;
    int nSize = data?data->size():0;
    switch(dwType)
    {
    case GMT_CONNECT:
        {
            if (nSize == sizeof(WORD))
            {
                WORD wCode = *((WORD*)pData);
                if (wCode == 0)
                {
                    // 连接成功
                    OnConnected();
                }
                else
                {
                    // 连接失败
                    OnDisconnected();
                }
            }
        }
        break;
    case GMT_SOCKCLOSE:
        {
            // 连接断开
            OnDisconnected();
        }
        break;
    }
    bRet = m_pLobbyHandler->OnMessage(dwType, data);
    if(bRet) return TRUE;
    bRet = m_pGame->OnMessage(dwType, data);
    return bRet;
}

void CMainDlg::OnConnected()
{
    SLOGI()<<"Connected to server";
    m_pLobbyHandler->OnConnected();
    m_pGame->OnConnected();
}

void CMainDlg::OnDisconnected()
{
    SLOGI()<<"Disconnected from server";
    m_pLobbyHandler->OnDisconnected();
    m_pGame->OnDisconnected();
}

void CMainDlg::OnBtnMute()
{
    FindChildByName(L"btn_mute")->SetVisible(FALSE);
    FindChildByName(L"btn_unmute")->SetVisible(TRUE);
    m_bMute = FALSE;
}

void CMainDlg::OnBtnUnmute()
{
    FindChildByName(L"btn_mute")->SetVisible(TRUE);
    FindChildByName(L"btn_unmute")->SetVisible(FALSE);
    m_bMute = TRUE;
}

void CMainDlg::PlayWave(LPCTSTR pszSound)
{
    if(m_bMute) return;
    SLOGI() << "PlayWave:" << pszSound;
#ifdef _WIN32
    Win32PlaySound(pszSound, NULL, SND_ASYNC | SND_NOSTOP | SND_FILENAME);
#else
    // Use the default PlaySound implementation on other platforms
    ::PlaySound(pszSound, NULL, SND_ASYNC | SND_NOSTOP | SND_FILENAME);
#endif
}

