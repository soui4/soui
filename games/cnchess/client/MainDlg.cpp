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
#include "utils.h"
#include <valueAnimator/SPropertyAnimator.h>
#define kLogTag "MainDlg"

const int CMainDlg::ANI_TIP = 0x50;

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
, m_pThemeProgressModal(NULL)
, m_themeProgressSession(0)
, m_bThemeBusy(false)
, m_modalRoot(NULL)
, m_pTipContainer(NULL)
, m_nSelAvatarId(1)
, m_tsLastTip(0)
{
    m_webSocketClient.SetMessageHandler(this);
    m_themeDownloader.SetListener(this);
    m_pGame = new CChessGame(this,pTheme);
    m_pLobbyHandler = new LobbyHandler(this);
    m_pEndgameHandler = new EndgameHandler(this, pTheme);
    m_pLobbyHandler->SetWebSocket(&m_webSocketClient);
    m_pEndgameHandler->SetWebSocket(&m_webSocketClient);
}

CMainDlg::~CMainDlg()
{
    delete m_pGame;
    delete m_pLobbyHandler;
    delete m_pEndgameHandler;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
    m_modalRoot = FindChildByName("modal_root");
    m_pTipContainer = FindChildByName("tip_container");
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
    // 解压在工作线程中进行，把UI线程的消息循环交给它，以便进度/结果回调切回UI线程
    m_themeDownloader.SetMsgLoop(GetMsgLoop());
    m_pLobbyHandler->Init(FindChildByName(L"room_container"));
    m_pEndgameHandler->Init(FindChildByName(L"endgame_container"));

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
                int nAvatarId = node.attribute(L"avatar_id").as_int(1);
                if (nAvatarId < 1 || nAvatarId >= BuiltinAvatar::COUNT) nAvatarId = 1;
                m_nSelAvatarId = nAvatarId;
            }
        }
    }

    // 头像选择：仅提供内置头像(CHAIR 为空座位占位, 不提供), 选中项高亮
    auto UpdateAvatarSel = [pModal](int nId){
        for (int i = 1; i < BuiltinAvatar::COUNT; ++i)
        {
            SStringT strName;
            strName.Format(_T("btn_avatar_%d"), i);
            SWindow *pBtn = pModal->FindChildByName(strName);
            if (pBtn) pBtn->SetAlpha(i == nId ? 255 : 160);
        }
    };
    UpdateAvatarSel(m_nSelAvatarId);
    for (int i = 1; i < BuiltinAvatar::COUNT; ++i)
    {
        SStringT strName;
        strName.Format(_T("btn_avatar_%d"), i);
        SWindow *pBtn = pModal->FindChildByName(strName);
        if (pBtn) pBtn->SubscribeEvent(EventCmd::EventID, [this, i, UpdateAvatarSel](IEvtArgs *e){
            m_nSelAvatarId = i;
            UpdateAvatarSel(i);
            return TRUE;
        });
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
            node.attribute2(L"avatar_id").set_value(m_nSelAvatarId);
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
            OnLoginSuccess(strSvr, strName, cSex, m_nSelAvatarId);
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

    // 游戏初始化延迟到主题加载完成后（OnThemeReady）

    SStringA svr = S_CT2A(dlgLogin.m_strSvr);
    BOOL bRet = m_webSocketClient.ConnectToServer(svr, "");
    SLOGI()<<"connect to server ret:"<<bRet;
    return TRUE;
#endif
}

#if defined(__MOBILE__)
void CMainDlg::OnLoginSuccess(SStringT strSvr, SStringT strName, char cSex, int nAvatarId)
{
    MyProfile* myProfile = MyProfile::getSingletonPtr();
    myProfile->SetSex(cSex);
    myProfile->SetName(strName);
    // 使用登录弹窗选择的内置头像; 非法值回退为按性别匹配默认头像
    if (nAvatarId < 1 || nAvatarId >= BuiltinAvatar::COUNT)
        nAvatarId = (cSex == SEX_FEMALE) ? BuiltinAvatar::FEMALE : BuiltinAvatar::MALE;
    myProfile->SetAvatarId(nAvatarId);

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

    // 显示主题资源进度弹窗：从版本检查开始，直到主题解压完成并应用成功才关闭
    m_bThemeBusy = true;
    ShowThemeProgress();
    UpdateThemeProgress(0, _T("正在检查主题版本..."), _T("正在更新主题资源..."));

    // 请求主题（发送本地MD5，服务器决定是否需要下载）
    m_themeDownloader.RequestTheme(&m_webSocketClient);

    // 大厅不依赖主题，可以立即处理连接
    m_pLobbyHandler->OnConnected();
    m_pEndgameHandler->OnConnected();

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
    // 残局大厅与普通大厅共用房间/桌卡信息, 二者都要接收
    if (dwType == GMT_ROOM_INFO || dwType == GMT_TABLE_INFO)
        m_pEndgameHandler->OnMessage(dwType, data);
    if(bRet) return TRUE;
    bRet = m_pEndgameHandler->OnMessage(dwType, data);
    if(bRet) return TRUE;
    bRet = m_pGame->OnMessage(dwType, data);
    return bRet;
}

void CMainDlg::SwitchToTab(int nIndex)
{
    STabCtrl *pTab = FindChildByName2<STabCtrl>(L"main_tabctrl");
    if (pTab)
        pTab->SetCurSel(nIndex);
}

void CMainDlg::SwitchToGame()
{
    STabCtrl *pTab = FindChildByName2<STabCtrl>(L"main_tabctrl");
    if (!pTab)
        return;
    // 桌面/移动端页签顺序不同, 按游戏页窗口名查找所在页签
    int nIndex = pTab->GetPageIndex(_T("game_container"), FALSE);
    if (nIndex >= 0)
        pTab->SetCurSel(nIndex);
}

void CMainDlg::OnThemeReady(const SStringT& strThemeDir, bool bUpdated)
{
    SLOGI() << "OnThemeReady: dir=" << strThemeDir.c_str() << " updated=" << bUpdated;

    // 资源已解压完成，进度视图继续显示到主题真正可用：先把进度置满并切到"应用主题"阶段
    UpdateThemeProgress(100, _T("解压完成，正在加载主题..."), _T("正在应用主题资源..."));

    // 重新加载主题
    bool bThemeOK = false;
    if (m_pTheme)
    {
        if (m_pTheme->Load(strThemeDir))
        {
            m_bThemeLoaded = true;
            bThemeOK = true;
            SLOGI() << "Theme loaded successfully from " << strThemeDir.c_str();
        }
        else
        {
            SLOGE() << "Failed to load theme from " << strThemeDir.c_str();
        }
    }
    m_pEndgameHandler->OnThemdReady();
    InitGame();
    m_bThemeBusy = false;
    HideThemeProgress();

    if (!bThemeOK)
    {
        SLOGE() << "Theme resource is extracted but failed to load: " << strThemeDir.c_str();
    }
}

void CMainDlg::OnThemeProgress(int nPercent)
{
    SLOGI() << "Theme download progress: " << nPercent << "%";
    if (!m_bThemeBusy)
        return; // 资源准备阶段已结束(成功或失败)，丢弃迟到的进度回调
    // 弹窗必须覆盖整个"主题资源准备"过程，任何一次进度都保证它处于显示状态
    ShowThemeProgress();
    SStringT strStatus;
    strStatus.Format(_T("正在下载... %d%%"), nPercent);
    UpdateThemeProgress(nPercent, strStatus, _T("正在更新主题资源..."));
}

void CMainDlg::OnThemeExtractProgress(int nPercent)
{
    SLOGI() << "Theme extract progress: " << nPercent << "%";
    if (!m_bThemeBusy)
        return; // 资源准备阶段已结束(成功或失败)，丢弃迟到的进度回调
    // 与下载进度共用同一个模态视图：解压在工作线程中进行，这里确保弹窗已显示，
    // 并且一直保持到解压完成、主题可用(OnThemeReady)之后才关闭
    ShowThemeProgress();
    SStringT strStatus;
    strStatus.Format(_T("正在解压... %d%%"), nPercent);
    UpdateThemeProgress(nPercent, strStatus, _T("正在解压主题资源..."));
}

void CMainDlg::OnThemeError(const SStringA& strErr)
{
    SLOGE() << "Theme download error: " << strErr;
    UpdateThemeProgress(0, _T("下载失败，尝试使用缓存..."), _T("主题资源更新失败"));
    // 如果游戏尚未初始化且主题未加载，使用本地备用主题（如果有）
    if (!m_bGameInited && !m_bThemeLoaded)
    {
        // 尝试从缓存目录加载
        SStringT strCacheThemeDir = m_themeDownloader.GetThemeDir();
        if (m_pTheme && m_pTheme->Load(strCacheThemeDir))
        {
            m_bThemeLoaded = true;
        }
        else
        {
            SLOGE() << "No fallback theme available";
        }
    }
    // 资源准备已终止（失败）：先清busy挡掉迟到的进度回调，再关闭弹窗
    m_bThemeBusy = false;
    HideThemeProgress();
}

void CMainDlg::InitGame()
{
    if (m_bGameInited) return;
    SLOGI() << "InitGame: initializing game";
    // 初始化大厅（不依赖主题）
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

    SModalRoot* pModal = (SModalRoot*)SApplication::getSingleton().CreateWindowByName(SModalRoot::GetClassName());
    if (!pModal) return;

    pModal->InitFromResId(_T("layout:dlg_theme_progress"));

    // 该弹窗只能由本类主动关闭：XML 中已设 quitOnClick="0" 与 quitOnEsc="0"，
    // 所以点击背景与按下ESC都不会结束模态会话，进度视图不会被用户中途关掉。
    ModalViewSessionID session = BeginModalViewSession(pModal, m_modalRoot);
    if (session == 0)
    {
        SLOGE() << "ShowThemeProgress: failed to begin modal view session";
        pModal->Release();
        return;
    }
    m_pThemeProgressModal = pModal;
    m_themeProgressSession = session;
}

void CMainDlg::HideThemeProgress()
{
    if (!m_pThemeProgressModal) return;

    EndModalViewSession(m_themeProgressSession, IDOK);
    m_pThemeProgressModal = NULL;
    m_themeProgressSession = 0;
}

void CMainDlg::UpdateThemeProgress(int nPercent, const SStringT& strStatus, LPCTSTR pszTitle)
{
    if (!m_pThemeProgressModal) return;

    SProgress* pProgress = m_pThemeProgressModal->FindChildByName2<SProgress>(L"progress_bar");
    if (pProgress) pProgress->SetValue(nPercent);

    SWindow* pStatus = m_pThemeProgressModal->FindChildByName(L"txt_status");
    if (pStatus) pStatus->SetWindowText(strStatus);

    if (pszTitle)
    {
        SWindow* pTitle = m_pThemeProgressModal->FindChildByName(L"txt_title");
        if (pTitle) pTitle->SetWindowText(pszTitle);
    }
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

void CMainDlg::onAnimationEnd(IValueAnimator *pAnimator)
{
    IPropertyAnimator *pPropAnimator = sobj_cast<IPropertyAnimator>(pAnimator);
    if (pPropAnimator && pPropAnimator->GetID() == ANI_TIP)
    {
        IWindow *pTip = pPropAnimator->GetTarget();
        if (pTip)
            pTip->Destroy();
    }
}

void CMainDlg::PlayTip(const SStringT &strTip)
{
    if (!m_pTipContainer || !m_pTheme) return;
    SXmlNode xmlTip = m_pTheme->GetTemplate(L"tip");
    SASSERT(xmlTip);
    SStringW strWndClass = xmlTip.attribute(L"wndclass").as_string(L"text");
    IWindow *pTip = SApplication::getSingletonPtr()->CreateWindowByName(strWndClass);
    pTip->InitFromXml(&xmlTip);
    // 让提示容器覆盖顶层内容区域（悬浮容器不参与 vbox 布局，这里显式设置尺寸）
    SWindow *pParent = m_pTipContainer->GetParent();
    if (pParent)
    {
        CRect rcContent;
        pParent->GetChildrenLayoutRect(&rcContent);
        m_pTipContainer->Move(rcContent);
    }
    m_pTipContainer->InsertIChild(pTip);
    pTip->SetWindowText(strTip);
    AnchorPos toPos;
    toPos.type = APT_Center_Top;
    toPos.x = SLayoutSize(-10, dp);
    toPos.y = SLayoutSize(-10, dp);
    toPos.fOffsetX = -0.5f;
    toPos.fOffsetY = -0.5f;

    ILayoutParam* pParam = (ILayoutParam*)pTip->GetLayoutParam();
    SAnchorLayoutParamStruct* pParamStruct = (SAnchorLayoutParamStruct*)pParam->GetRawData();
    AnchorPos fromPos = pParamStruct->pos;
    DWORD now = GetTickCount();
    if ((now - m_tsLastTip) < 1000) {
        fromPos.y.fSize += 50;
    }
    m_tsLastTip = now;
    SAutoRefPtr<IValueAnimator> pAnim = Util::MoveAndHideSprite(pTip, fromPos, toPos, 5000);
    pAnim->SetID(ANI_TIP);
    pAnim->addListener(this);//destroy the tip when animation end.
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

