// android_entry.cc
//
// 基金持仓助手的 Android 入口实现，对应桌面版 main.cpp。
//
// 职责与桌面版保持一致，只把「窗口宿主」换成 Android 的 SouiScreen：
//   - InitApp:        创建 SApplication，注册扩展控件，配置 Skia/Stb 与 assets 资源
//   - ScreenStartup:  创建 CMainDlg，加载 layout:dlg_main（mobile/uires）
//   - ScreenShutdown: 先 Shutdown() 收尾后台线程，再销毁窗口
//   - UninitApp:      释放 SApplication
//
// 业务逻辑（MainDlg.cpp / fund_*.cpp / SFundWidgets.cpp）与桌面版完全共用，
// 布局由 mobile/uires 提供，这里不出现任何按平台编写的 UI 分支。

#include <jni.h>
#include <android/asset_manager_jni.h>
#include <string>
#include <map>

#include "stdafx.h"
#include <SAppCfg.h>
#include <logdef.h>
#include <soui4android.h>
using namespace SOUI;

#include <SGridCtrl.h>
#include "MainDlg.h"
#include "SFundWidgets.h"
#include "applog.h"

#define kLogTag "fundholdings-jni"

// R 的实体只在应用内定义一次（桌面版在 main.cpp 里定义）。
#define INIT_R_DATA
#include "res/resource.h"

class FundHoldingsAndroidApp : public Soui4AndroidEntry {
    SApplication *m_souiApp = NULL;
    std::map<long, SAutoRefPtr<SHostWnd> > m_screenHostMap;
    std::string m_assetDir;

public:
    FundHoldingsAndroidApp() {
        InitSoui4AndroidEntry(this);
    }

    HWND ScreenStartup(long screenId, LPCSTR pszLayout) OVERRIDE;
    void ScreenShutdown(long screenId) OVERRIDE;
    void UninitApp() OVERRIDE;
    BOOL InitApp(AAssetManager *assetMgr, LPCSTR pszAssetDir) OVERRIDE;
};

static FundHoldingsAndroidApp theApp;

BOOL FundHoldingsAndroidApp::InitApp(AAssetManager *assetMgr, LPCSTR pszAssetDir) {
    m_assetDir = pszAssetDir ? pszAssetDir : "";

    // XML 里不带单位的尺寸按 dp 解析：与桌面版 main.cpp 保持一致，
    // 手机竖屏下字体、间距、圆角才能使用同一套设计值。
    SLayoutSize::defUnit = SOUI::dp;

    m_souiApp = new SApplication((HINSTANCE)NULL);

    // 扩展控件（XML 标签名由 DEF_SOBJECT 决定）
    m_souiApp->RegisterWindowClass<SGridCtrl>();
    m_souiApp->RegisterWindowClass<SFundKpiCard>();
    m_souiApp->RegisterWindowClass<SFundSpark>();
    m_souiApp->RegisterWindowClass<SFundContrib>();
    m_souiApp->RegisterWindowClass<SFundTabs>();
    m_souiApp->RegisterWindowClass<SFundWatchTabs>();
    m_souiApp->RegisterWindowClass<SFundIndexBar>();
    m_souiApp->RegisterWindowClass<SFundAbout>();
    m_souiApp->RegisterWindowClass<SFundScroll>(); // <fundscroll> 移动版竖向滚动容器

    SAppCfg cfg;
    cfg.SetRender(Render_Skia)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetAppDir(S_CA2T(m_assetDir.c_str(), CP_UTF8))
        .SetLog(TRUE, LOG_LEVEL_INFO, "fundholdings")
        .SetSysResAndroidAsset(assetMgr, _T("soui_sys_res"))
        .SetAppResAndroidAsset(assetMgr, _T("uires"));

    if (!cfg.DoConfig(m_souiApp)) {
        delete m_souiApp;
        m_souiApp = NULL;
        return FALSE;
    }

    // 命名 ID 与桌面版完全一致（R.ID.edit_code / R.ID.grid_holdings ...），
    // 这样业务代码无需区分平台。
    m_souiApp->InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id,
                              sizeof(R.id) / sizeof(int));
    m_souiApp->EnableNotifyCenter(TRUE);

    // 请求日志面板依赖 CLogSink，安装后日志会同时进入界面与 log4z
    CLogSink::Instance().Install();

    SLOGI() << "FundHoldingsAndroidApp::InitApp completed, assetDir=" << m_assetDir.c_str();
    return TRUE;
}

void FundHoldingsAndroidApp::UninitApp() {
    CLogSink::Instance().Uninstall();
    if (m_souiApp) {
        m_souiApp->Release();
        m_souiApp = NULL;
    }
}

HWND FundHoldingsAndroidApp::ScreenStartup(long screenId, LPCSTR pszLayout) {
    SASSERT(m_souiApp);
    if (screenId == 0 || !m_souiApp) {
        SLOGE() << "ScreenStartup: invalid args";
        return 0;
    }

    std::map<long, SAutoRefPtr<SHostWnd> >::iterator existing = m_screenHostMap.find(screenId);
    if (existing != m_screenHostMap.end()) {
        existing->second->DestroyWindow();
        m_screenHostMap.erase(existing);
    }

    CMainDlg *pDlg = new CMainDlg();
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

    SLOGI() << "ScreenStartup OK: screenId=" << screenId << " layout="
            << (pszLayout ? pszLayout : "(null)");
    return ret;
}

void FundHoldingsAndroidApp::ScreenShutdown(long screenId) {
    if (screenId == 0) return;
    std::map<long, SAutoRefPtr<SHostWnd> >::iterator it = m_screenHostMap.find(screenId);
    if (it == m_screenHostMap.end()) {
        SLOGW() << "ScreenShutdown: screenId=" << screenId << " NOT found";
        return;
    }

    // 先停后台任务与定时器，再销毁窗口：与桌面版 app.Run 退出后调用 Shutdown 的顺序一致
    CMainDlg *pDlg = static_cast<CMainDlg *>(it->second.operator->());
    pDlg->Shutdown();
    it->second->DestroyWindow();
    m_screenHostMap.erase(it);
}
