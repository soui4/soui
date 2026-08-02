#include <jni.h>
#include <android/asset_manager_jni.h>
#include <string>
#include <map>

#include <souistd.h>
#include <SAppCfg.h>
#include <logdef.h>
#include <soui4android.h>
using namespace SOUI;

#include "MainDlg.h"
#include "SGameTheme.h"
#include "CnchessSkin.h"
#include "ChessBoard.h"
#include "ChessPiece.h"
#include "myprofile.h"
#include <helper/slog.h>
#include <gif/SSkinAni.h>
#include <gif/SGifPlayer.h>

#define kLogTag "cnchess-jni"

class CnchessAndroidApp : public Soui4AndroidEntry {
    SApplication* m_souiApp = nullptr;
    std::map<long, SAutoRefPtr<SHostWnd>> m_screenHostMap;
    SAutoRefPtr<SGameTheme> m_theme;
    std::string m_assetDir;

public:
    CnchessAndroidApp() {
        InitSoui4AndroidEntry(this);
    }

    HWND ScreenStartup(long screenId, LPCSTR pszLayout) override;
    void ScreenShutdown(long screenId) override;
    void UninitApp() override;
    BOOL InitApp(AAssetManager* assetMgr, LPCSTR pszAssetDir) override;
};

static CnchessAndroidApp theApp;

BOOL CnchessAndroidApp::InitApp(AAssetManager* assetMgr, LPCSTR pszAssetDir) {
    m_assetDir = pszAssetDir;

    m_souiApp = new SApplication((HINSTANCE) nullptr);

    SAppCfg cfg;
    cfg.SetRender(Render_Skia)
            .SetImgDecoder(ImgDecoder_Stb)
            .SetAppDir(S_CA2T(pszAssetDir, CP_UTF8))
            .SetSysResAndroidAsset(assetMgr, _T("soui_sys_res"))
            .SetAppResAndroidAsset(assetMgr, _T("uires"));
    BOOL ok = cfg.DoConfig(m_souiApp);
    if (!ok) {
        delete m_souiApp;
        return FALSE;
    }

    // 注册自定义皮肤类
    m_souiApp->RegisterSkinClass<SSkinAni>();
    m_souiApp->RegisterSkinClass<SSkinPiece>();
    m_souiApp->RegisterSkinClass<SSkinBoard>();
    // 注册自定义窗口类
    m_souiApp->RegisterWindowClass<SGifPlayer>();
    m_souiApp->RegisterWindowClass<CChessBoard>();
    m_souiApp->RegisterWindowClass<CChessPiece>();

    // def_theme 和 avatar 已由 Java 侧 (CnchessApplication) 从 assets 复制到 filesDir
    SLOGI() << "CnchessAndroidApp::InitApp completed, assetDir=" << pszAssetDir;
    return TRUE;
}

void CnchessAndroidApp::UninitApp() {
    SASSERT(m_souiApp);
    m_souiApp->Release();
    m_souiApp = nullptr;
}

HWND CnchessAndroidApp::ScreenStartup(long screenId, LPCSTR pszLayout) {
    SASSERT(m_souiApp);
    if (screenId == 0 || !m_souiApp) {
        SLOGE() << "ScreenStartup: invalid args";
        return 0;
    }

    auto existing = m_screenHostMap.find(screenId);
    if (existing != m_screenHostMap.end()) {
        existing->second->DestroyWindow();
        m_screenHostMap.erase(existing);
    }

    // 加载主题（def_theme 已由 Java 侧复制到 filesDir）
    m_theme.Attach(new SGameTheme());
    SStringT themeDir = S_CA2T(m_assetDir.c_str(), CP_UTF8) + _T("/cnchess/def_theme");
    if (!m_theme->Load(themeDir)) {
        SLOGE() << "ScreenStartup: failed to load theme from " << themeDir.c_str();
        return 0;
    }

    // 加载头像（avatar 已由 Java 侧复制到 filesDir）
    MyProfile* pMyProfile = new MyProfile();
    SStringT avatarPath = S_CA2T(m_assetDir.c_str(), CP_UTF8) + _T("/cnchess/avatar/myshow.gif");
    FILE* f = _tfopen(avatarPath, _T("rb"));
    if (f) {
        fseek(f, 0, SEEK_END);
        int nSize = ftell(f);
        fseek(f, 0, SEEK_SET);
        BYTE* pData = new BYTE[nSize];
        fread(pData, 1, nSize, f);
        pMyProfile->SetAvatarData(pData, nSize);
        fclose(f);
        delete[] pData;
    }

    // 创建主窗口
    CMainDlg* pDlg = new CMainDlg(m_theme);
    if (!pDlg->Create(NULL, 0, 0, 50, 50)) {
        SLOGE() << "ScreenStartup: CMainDlg.Create failed";
        delete pDlg;
        return 0;
    }
    pDlg->SendMessage(WM_INITDIALOG);
    pDlg->ShowWindow(SW_SHOW);

    HWND ret = pDlg->m_hWnd;
    m_screenHostMap.insert(std::make_pair(screenId, pDlg));
    pDlg->Release();

    SLOGI() << "ScreenStartup: OK screenId=" << screenId;
    return ret;
}

void CnchessAndroidApp::ScreenShutdown(long screenId) {
    if (screenId == 0) return;
    auto it = m_screenHostMap.find(screenId);
    if (it == m_screenHostMap.end()) {
        SLOGW() << "ScreenShutdown: screenId=" << screenId << " NOT found";
        return;
    }
    SLOGI() << "ScreenShutdown: screenId=" << screenId;
    it->second->DestroyWindow();
    m_screenHostMap.erase(it);
    delete MyProfile::getSingletonPtr();
}
