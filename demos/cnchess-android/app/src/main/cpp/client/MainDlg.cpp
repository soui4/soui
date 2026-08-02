// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"    
#include <core/SModalViewSession.h>
#include <helper/SMenuEx.h>
#include <helper/SFunctor.hpp>
#include <helper/slog.h>
#define kLogTag "MainDlg"
#include <mmsystem.h>
#ifdef _WIN32
#include "win32_audio.h"
#endif
CMainDlg::CMainDlg(SGameTheme* pTheme) 
: SHostWnd(_T("LAYOUT:XML_MAINWND"))
, m_pTheme(pTheme)
, m_bMute(FALSE)
{
    m_pGame = new CChessGame(this,pTheme);
    m_pLobbyHandler = new LobbyHandler();
    m_webSocketClient.SetMessageHandler(this);
}

CMainDlg::~CMainDlg()
{
    delete m_pGame;
    delete m_pLobbyHandler;
}

void CMainDlg::OnLoginSuccess(SStringT strSvr, SStringT strName, char cSex)
{
    MyProfile* myProfile = MyProfile::getSingletonPtr();
    myProfile->SetSex(cSex);
    myProfile->SetName(strName);

    m_pLobbyHandler->Init(FindChildByName(L"room_container"), &m_webSocketClient);
    m_pGame->Init(FindChildByName(L"game_container"), &m_webSocketClient);

    SStringA svr = S_CT2A(strSvr);
    BOOL bRet = m_webSocketClient.ConnectToServer(svr, "");
    SLOGI()<<"connect to server ret:"<<bRet;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
    // 使用 SModalRoot/SModalView 替代 SHostDialog::DoModal()
    // Android 平台不支持独立消息循环，必须使用模态视图方式
    SModalRoot *pModal = (SModalRoot*)SApplication::getSingleton().CreateWindowByName(SModalRoot::GetClassName());
    pModal->InitFromResId("layout:dlg_login_modal");
    
    // 预先读取配置填充表单
    SWindow *pEdtSvr = pModal->FindChildByName2<SEdit>(L"edt_svr");
    SWindow *pEdtName = pModal->FindChildByName2<SEdit>(L"edt_name");
    SComboBox *pComboSex = pModal->FindChildByName2<SComboBox>(L"cbx_sex");
    
    SStringT strCfg = SApplication::getSingleton().GetAppDir() + _T("/cnchess_cfg.xml");
    if(GetFileAttributes(strCfg) != INVALID_FILE_ATTRIBUTES)
    {
        SXmlDoc doc;
        if(doc.load_file(strCfg))
        {
            SXmlNode node = doc.root().child(L"config");
            if(node)
            {
                pEdtSvr->SetWindowText(S_CW2T(node.attribute(L"svr").as_string()));
                pEdtName->SetWindowText(S_CW2T(node.attribute(L"name").as_string()));
                pComboSex->SetCurSel(node.attribute(L"sex").as_int(0));
            }
        }
    }

    ModalViewSessionID session_id = BeginModalViewSession(pModal);
    
    // 登录按钮事件
    pModal->FindChildByName(L"btn_login")->SubscribeEvent(EventCmd::EventID, [=](IEvtArgs *e){
        SStringT strSvr = pEdtSvr->GetWindowText();
        SStringT strName = pEdtName->GetWindowText();
        int iSel = pComboSex->GetCurSel();
        char cSex = pComboSex->GetItemData(iSel);
        
        // 保存配置
        SXmlDoc doc;
        SXmlNode node = doc.root().child2(L"config");
        if (node)
        {
            node.attribute2(L"svr").set_value(S_CT2W(strSvr));
            node.attribute2(L"name").set_value(S_CT2W(strName));
            node.attribute2(L"sex").set_value(cSex);
        }
        SStringT strCfg = SApplication::getSingleton().GetAppDir() + _T("/cnchess_cfg.xml");
        doc.save_file(strCfg);
        
        EndModalViewSession(session_id, IDOK);
        return TRUE;
    });

    // 模态视图退出事件
    pModal->SubscribeEvent(EventExitModalView::EventID, [=](IEvtArgs *e){
        EventExitModalView *e2 = sobj_cast<EventExitModalView>(e);
        if(e2->exitCode == IDOK) {
            // 登录成功，继续初始化
            SStringT strSvr = pEdtSvr->GetWindowText();
            SStringT strName = pEdtName->GetWindowText();
            int iSel = pComboSex->GetCurSel();
            char cSex = pComboSex->GetItemData(iSel);
            OnLoginSuccess(strSvr, strName, cSex);
        } else {
            // 登录取消，关闭窗口
            OnClose();
        }
        return TRUE;
    });
    
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
	STaskHelper::post(GetMsgLoop(), this, &CMainDlg::_OnMessage, dwType, data);
    return TRUE;
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

BOOL CMainDlg::_OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data)
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
#ifdef _WIN32
    Win32PlaySound(pszSound, NULL, SND_ASYNC | SND_NOSTOP | SND_FILENAME);
#else
    // Use the default PlaySound implementation on other platforms
    ::PlaySound(pszSound, NULL, SND_ASYNC | SND_NOSTOP | SND_FILENAME);
#endif
}

