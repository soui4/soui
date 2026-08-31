// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include <helper/SDpiHelper.hpp>
#include "SGameTheme.h"
#include "WebSocketClient.h"
#include "LobbyHandler.h"
#include "ChessGame.h"
#include "ThemeDownloadManager.h"
#include "myprofile.h"

class CMainDlg : public SHostWnd
               , public SDpiHandler<CMainDlg>
               , public WebSocketClient::IListener
               , public ThemeDownloadManager::IListener
{
public:
    CMainDlg(SGameTheme* pTheme);
    ~CMainDlg();

    void OnClose();
    void OnMaximize();
    void OnRestore();
    void OnMinimize();
    void OnSize(UINT nType, CSize size);
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

    void PlayWave(LPCTSTR pszSound);
    // 主题是否已加载
    bool IsThemeLoaded() const { return m_bThemeLoaded; }
    SWindow *getModalRoot() const{return m_modalRoot;}
#if defined(__MOBILE__)
    // 登录成功回调（模态视图方式，移动平台不支持独立消息循环）
    void OnLoginSuccess(SStringT strSvr, SStringT strName, char cSex);
#endif

  protected:
    void OnScaleChanged(int nScale) override;
  protected:
    virtual BOOL OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data) override;

    void OnConnected();
    void OnDisconnected();
    BOOL _OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data);

    // ThemeDownloadManager::IListener
    virtual void OnThemeReady(const SStringT& strThemeDir, bool bUpdated) override;
    virtual void OnThemeProgress(int nPercent) override;
    virtual void OnThemeError(const SStringA& strErr) override;

    // 初始化游戏（需要主题已加载）
    void InitGameAndLobby();

    // 主题下载进度弹窗
    void ShowThemeProgress();
    void HideThemeProgress();
    void UpdateThemeProgress(int nPercent, const SStringT& strStatus);
  protected:
    void OnBtnMute();
    void OnBtnUnmute();
    //soui消息
    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_close", OnClose)
        EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
        EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
        EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
        EVENT_NAME_COMMAND(L"btn_mute", OnBtnMute)
        EVENT_NAME_COMMAND(L"btn_unmute", OnBtnUnmute)
        CHAIN_EVENT_MAP_MEMBER(*m_pGame)
        CHAIN_EVENT_MAP_MEMBER(*m_pLobbyHandler)
    EVENT_MAP_END2(SHostWnd)

    //HostWnd真实窗口消息处理
    BEGIN_MSG_MAP_EX(CMainDlg)
        CHAIN_MSG_MAP(SDpiHandler<CMainDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        CHAIN_MSG_MAP_MEMBER(*m_pGame)
        CHAIN_MSG_MAP(SHostWnd)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

private:
    CChessGame* m_pGame;  // 游戏核心逻辑
    LobbyHandler* m_pLobbyHandler;
        // 网络通信
    WebSocketClient m_webSocketClient;
    SAutoRefPtr<SGameTheme> m_pTheme;
    BOOL m_bMute;

    // 主题下载
    ThemeDownloadManager m_themeDownloader;
    SStringT m_strThemeCacheDir;
    bool m_bThemeLoaded;    // 主题是否已加载
    bool m_bGameInited;     // 游戏是否已初始化
    bool m_bConnected;      // WebSocket是否已连接
    bool m_bLobbyInited;    // 大厅是否已初始化

    // 主题下载进度弹窗
    SModalRoot* m_pThemeProgressModal;
    ModalViewSessionID m_themeProgressSession;
    SWindow * m_modalRoot;
};