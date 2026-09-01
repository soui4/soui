// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include <core/SModalViewSession.h>
#if !defined(__MOBILE__)
#include "LoginDlg.h"
#endif
#include <helper/SMenuEx.h>
#include <helper/SFunctor.hpp>
#include <helper/slog.h>
#include <mmsystem.h>
#include <shlobj.h>
#define kLogTag "MainDlg"

#ifdef _WIN32
#include "win32_audio.h"
#endif
CMainDlg::CMainDlg(SGameTheme* pTheme)
: SHostWnd(_T("LAYOUT:XML_MAINWND"))
, m_pTheme(pTheme)
, m_bMute(FALSE)
, m_bThemeLoaded(false)
, m_bGameInited(false)
, m_bConnected(false)
, m_bLobbyInited(false)
, m_pThemeProgressModal(NULL)
, m_themeProgressSession(0)
, m_modalRoot(NULL)
{
    m_pGame = new CChessGame(this,pTheme);
    m_pLobbyHandler = new LobbyHandler();
    m_webSocketClient.SetMessageHandler(this);
    m_themeDownloader.SetListener(this);
}

CMainDlg::~CMainDlg()
{
    delete m_pGame;
    delete m_pLobbyHandler;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
    m_modalRoot = FindChildByName("modal_root");
    #if defined (__IOS__)
    FindChildByName("pane_ios_header")->SetVisible(TRUE,TRUE);
    #endif
    // 初始化主题下载缓存目录
    #if defined(__MOBILE__)
        TCHAR szCachePath[MAX_PATH]={0};
        SHGetSpecialFolderPath(0,szCachePath,CSIDL_PERSONAL,TRUE);
        m_strThemeCacheDir = szCachePath;
    #else
        m_strThemeCacheDir = SApplication::getSingleton().GetAppDir();
    #endif
    m_strThemeCacheDir += _T("/theme_cache");
    m_themeDownloader.Init(m_strThemeCacheDir);

    // 主题不在此加载，等待连接服务器成功后由 ThemeDownloadManager 下载加载

#if defined(__MOBILE__)
    // 移动平台：使用 SModalRoot/SModalView 替代 SHostDialog::DoModal()
    // Android/iOS 平台不支持独立消息循环，必须使用模态视图方式
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

    ModalViewSessionID session_id = BeginModalViewSession(pModal,m_modalRoot);

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
            SStringT strSvr = pEdtSvr->GetWindowText();
            SStringT strName = pEdtName->GetWindowText();
            int iSel = pComboSex->GetCurSel();
            char cSex = pComboSex->GetItemData(iSel);
            OnLoginSuccess(strSvr, strName, cSex);
        } else {
            OnClose();
        }
        return TRUE;
    });
    return TRUE;
#else
    CLoginDlg dlgLogin;
    if (dlgLogin.DoModal() != IDOK) {
        OnClose();
        return FALSE;
    }
    MyProfile* myProfile = MyProfile::getSingletonPtr();
    myProfile->SetSex(dlgLogin.m_cSex);
    myProfile->SetName(dlgLogin.m_strName);
    // 设置用户选择的头像：优先外部图片，否则内置ID
    if (!dlgLogin.m_strAvatarPath.IsEmpty())
    {
        myProfile->SetAvatarPath(dlgLogin.m_strAvatarPath);
    }
    else
    {
        int nId = (dlgLogin.m_nAvatarId >= 1 && dlgLogin.m_nAvatarId <= 5) ? dlgLogin.m_nAvatarId : 1;
        myProfile->SetAvatarId(nId);
    }

    SStringT strTitle = SStringT().Format(_T("用户:%s"), myProfile->GetName().c_str());
    FindChildByName(L"txt_title")->SetWindowText(strTitle);
    SetWindowText(strTitle);

    // 初始化大厅（不依赖主题）
    m_pLobbyHandler->Init(FindChildByName(L"room_container"), &m_webSocketClient);
    m_bLobbyInited = true;

    // 游戏初始化延迟到主题加载完成后（OnThemeReady）

    SStringA svr = S_CT2A(dlgLogin.m_strSvr);
    BOOL bRet = m_webSocketClient.ConnectToServer(svr, "");
    SLOGI()<<"connect to server ret:"<<bRet;
    return TRUE;
#endif
}

#if defined(__MOBILE__)
void CMainDlg::OnLoginSuccess(SStringT strSvr, SStringT strName, char cSex)
{
    MyProfile* myProfile = MyProfile::getSingletonPtr();
    myProfile->SetSex(cSex);
    myProfile->SetName(strName);
    // 移动端默认根据性别匹配默认内置头像
    int nId = (cSex == SEX_FEMALE) ? BuiltinAvatar::FEMALE : BuiltinAvatar::MALE;
    myProfile->SetAvatarId(nId);

    // 初始化大厅（不依赖主题）
    m_pLobbyHandler->Init(FindChildByName(L"room_container"), &m_webSocketClient);
    m_bLobbyInited = true;

    // 游戏初始化延迟到主题加载完成后（OnThemeReady）

    SStringA svr = S_CT2A(strSvr);
    BOOL bRet = m_webSocketClient.ConnectToServer(svr, "");
    SLOGI()<<"connect to server ret:"<<bRet;
}
#endif

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
    m_bConnected = true;

    // 显示主题下载进度弹窗
    ShowThemeProgress();
    UpdateThemeProgress(0, _T("正在检查主题版本..."));

    // 请求主题（发送本地MD5，服务器决定是否需要下载）
    m_themeDownloader.RequestTheme(&m_webSocketClient);

    // 大厅不依赖主题，可以立即处理连接
    m_pLobbyHandler->OnConnected();

    // 游戏依赖主题，仅在已初始化时通知连接
    if (m_bGameInited)
    {
        m_pGame->OnConnected();
    }
}

void CMainDlg::OnDisconnected()
{
    SLOGI()<<"Disconnected from server";
    m_bConnected = false;
    m_pLobbyHandler->OnDisconnected();
    if (m_bGameInited)
    {
        m_pGame->OnDisconnected();
    }
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
    case GMT_THEME_ACK:
    case GMT_THEME_DATA:
        {
            // 主题下载消息由下载管理器处理
            m_themeDownloader.HandleMessage(dwType, pData, nSize);
            return TRUE;
        }
        break;
    }
    bRet = m_pLobbyHandler->OnMessage(dwType, data);
    if(bRet) return TRUE;
    bRet = m_pGame->OnMessage(dwType, data);
    return bRet;
}

void CMainDlg::OnThemeReady(const SStringT& strThemeDir, bool bUpdated)
{
    SLOGI() << "OnThemeReady: dir=" << strThemeDir.c_str() << " updated=" << bUpdated;

    // 更新进度弹窗
    UpdateThemeProgress(100, _T("主题加载完成"));

    // 重新加载主题
    if (m_pTheme)
    {
        if (m_pTheme->Load(strThemeDir))
        {
            m_bThemeLoaded = true;
            SLOGI() << "Theme loaded successfully from " << strThemeDir.c_str();
        }
        else
        {
            SLOGE() << "Failed to load theme from " << strThemeDir.c_str();
            HideThemeProgress();
            return;
        }
    }

    // 如果游戏尚未初始化，现在初始化
    if (!m_bGameInited)
    {
        InitGameAndLobby();
    }

    // 隐藏进度弹窗
    HideThemeProgress();
}

void CMainDlg::OnThemeProgress(int nPercent)
{
    SLOGI() << "Theme download progress: " << nPercent << "%";
    SStringT strStatus;
    strStatus.Format(_T("正在下载... %d%%"), nPercent);
    UpdateThemeProgress(nPercent, strStatus);
}

void CMainDlg::OnThemeError(const SStringA& strErr)
{
    SLOGE() << "Theme download error: " << strErr;
    UpdateThemeProgress(0, _T("下载失败，尝试使用缓存..."));
    // 如果游戏尚未初始化且主题未加载，使用本地备用主题（如果有）
    if (!m_bGameInited && !m_bThemeLoaded)
    {
        // 尝试从缓存目录加载
        SStringT strCacheThemeDir = m_themeDownloader.GetThemeDir();
        if (m_pTheme && m_pTheme->Load(strCacheThemeDir))
        {
            m_bThemeLoaded = true;
            InitGameAndLobby();
        }
        else
        {
            SLOGE() << "No fallback theme available";
        }
    }
    HideThemeProgress();
}

void CMainDlg::InitGameAndLobby()
{
    if (m_bGameInited) return;
    if (!m_bThemeLoaded) return;

    SLOGI() << "InitGameAndLobby: initializing game";
    m_pGame->Init(FindChildByName(L"game_container"), &m_webSocketClient);
    m_bGameInited = true;

    // 如果已连接，通知游戏
    if (m_bConnected)
    {
        m_pGame->OnConnected();
    }
}

void CMainDlg::ShowThemeProgress()
{
    if (m_pThemeProgressModal) return; // 已显示

    m_pThemeProgressModal = (SModalRoot*)SApplication::getSingleton().CreateWindowByName(SModalRoot::GetClassName());
    if (!m_pThemeProgressModal) return;

    m_pThemeProgressModal->InitFromResId(_T("layout:dlg_theme_progress"));
    m_themeProgressSession = BeginModalViewSession(m_pThemeProgressModal,m_modalRoot);
}

void CMainDlg::HideThemeProgress()
{
    if (!m_pThemeProgressModal) return;

    EndModalViewSession(m_themeProgressSession, IDOK);
    m_pThemeProgressModal = NULL;
    m_themeProgressSession = 0;
}

void CMainDlg::UpdateThemeProgress(int nPercent, const SStringT& strStatus)
{
    if (!m_pThemeProgressModal) return;

    SProgress* pProgress = m_pThemeProgressModal->FindChildByName2<SProgress>(L"progress_bar");
    if (pProgress) pProgress->SetValue(nPercent);

    SWindow* pStatus = m_pThemeProgressModal->FindChildByName(L"txt_status");
    if (pStatus) pStatus->SetWindowText(strStatus);
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

