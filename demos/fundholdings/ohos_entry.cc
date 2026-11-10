//
// ohos_entry.cc
//
// 基金持仓助手的鸿蒙（OHOS）平台入口实现，对应 Android 端 android_entry.cc。
//
// 实现 Soui4OhosEntry 的四个虚函数：
//   - InitApp:       创建 SApplication，注册扩展控件，配置资源（Skia + Stb）
//   - UninitApp:     释放 SApplication
//   - ScreenStartup: 创建 CMainDlg 主窗口（layout:dlg_main，来自 mobile/uires）
//   - ScreenShutdown:先 Shutdown() 收尾后台线程，再销毁主窗口
//
// 业务逻辑与桌面版共用（MainDlg.cpp / fund_*.cpp / SFundWidgets.cpp），
// 这里不出现任何按平台编写的 UI 分支。
//
// 同时注册 N-API 模块 "fundholdings"，ArkTS 侧 import 'libfundholdings.so' 触发加载，
// 加载时静态构造 theApp → InitSoui4OhosEntry(this) 注册业务入口。
//

#include <string>
#include <map>

#include <souistd.h>
#include <SAppCfg.h>
#include <logdef.h>
#include <soui4ohos.h>
#include <rawfile/raw_file_manager.h>
#include <helper/slog.h>
using namespace SNS;

#include <SGridCtrl.h>
#include "MainDlg.h"
#include "SFundWidgets.h"
#include "applog.h"

#define kLogTag "fundholdings-ohos"

// R 的实体只在应用内定义一次（与桌面版 main.cpp、Android 入口一致）。
#define INIT_R_DATA
#include "res/resource.h"

class FundHoldingsOhosApp : public Soui4OhosEntry {
    SApplication *m_souiApp = nullptr;
    std::map<long, SAutoRefPtr<SHostWnd>> m_screenHostMap;
    std::string m_appDir;

public:
    FundHoldingsOhosApp() {
        InitSoui4OhosEntry(this);
    }

    BOOL InitApp(SouiOhosAssetManager assetMgr, LPCSTR pszAssetDir) override;
    void UninitApp() override;
    HWND ScreenStartup(long screenId, LPCSTR pszLayout) override;
    void ScreenShutdown(long screenId) override;
};

static FundHoldingsOhosApp theApp;

BOOL FundHoldingsOhosApp::InitApp(SouiOhosAssetManager assetMgr, LPCSTR pszAssetDir) {
    SLOGI() << "FundHoldingsOhosApp::InitApp assetDir=" << (pszAssetDir ? pszAssetDir : "(null)");

    m_appDir = pszAssetDir ? pszAssetDir : "";

    // XML 里不带单位的尺寸按 dp 解析，与桌面版 main.cpp 保持一致
    SLayoutSize::defUnit = SOUI::dp;

    m_souiApp = new SApplication((HINSTANCE) nullptr);

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
            .SetAppDir(S_CA2T(m_appDir.c_str(), CP_UTF8))
            .SetLog(TRUE, LOG_LEVEL_INFO, "fundholdings");

    // 系统/应用资源：从 HAP rawfile 直读
    NativeResourceManager *resMgr = reinterpret_cast<NativeResourceManager *>(assetMgr);
    if (!resMgr) {
        SLOGE() << "FundHoldingsOhosApp::InitApp: assetMgr is null";
        delete m_souiApp;
        m_souiApp = nullptr;
        return FALSE;
    }
    cfg.SetSysResOhosRawFile(resMgr, _T("soui_sys_res"));
    cfg.SetAppResOhosRawFile(resMgr, _T("uires"));

    BOOL ok = cfg.DoConfig(m_souiApp);
    if (!ok) {
        SLOGE() << "FundHoldingsOhosApp::InitApp DoConfig failed";
        delete m_souiApp;
        m_souiApp = nullptr;
        return FALSE;
    }

    // 命名 ID 与桌面版完全一致，业务代码无需区分平台
    m_souiApp->InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id,
                              sizeof(R.id) / sizeof(int));
    m_souiApp->EnableNotifyCenter(TRUE);

    // 请求日志面板依赖 CLogSink
    CLogSink::Instance().Install();

    SLOGI() << "FundHoldingsOhosApp::InitApp completed";
    return TRUE;
}

void FundHoldingsOhosApp::UninitApp() {
    SLOGI() << "FundHoldingsOhosApp::UninitApp";
    CLogSink::Instance().Uninstall();
    if (m_souiApp) {
        m_souiApp->Release();
        m_souiApp = nullptr;
    }
}

HWND FundHoldingsOhosApp::ScreenStartup(long screenId, LPCSTR pszLayout) {
    SLOGI() << "FundHoldingsOhosApp::ScreenStartup screenId=" << screenId
            << " layout=" << (pszLayout ? pszLayout : "(null)");

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

    SLOGI() << "FundHoldingsOhosApp::ScreenStartup OK screenId=" << screenId;
    return ret;
}

void FundHoldingsOhosApp::ScreenShutdown(long screenId) {
    SLOGI() << "FundHoldingsOhosApp::ScreenShutdown screenId=" << screenId;
    if (screenId == 0) return;
    auto it = m_screenHostMap.find(screenId);
    if (it == m_screenHostMap.end()) {
        SLOGW() << "ScreenShutdown: screenId=" << screenId << " NOT found";
        return;
    }

    // 先停后台任务与定时器，再销毁窗口
    CMainDlg *pDlg = static_cast<CMainDlg *>(it->second.operator->());
    pDlg->Shutdown();
    it->second->DestroyWindow();
    m_screenHostMap.erase(it);
}

/* ============================================================
 * N-API 模块注册（fundholdings）
 * ============================================================ */

#include "napi/native_api.h"

static napi_value InitFundHoldings(napi_env env, napi_callback_info info) {
    (void)info;
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

EXTERN_C_START
static napi_value FundHoldingsModuleInit(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"init", nullptr, InitFundHoldings, nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module g_fundHoldingsModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = FundHoldingsModuleInit,
    .nm_modname = "fundholdings",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterFundHoldingsModule(void) {
    napi_module_register(&g_fundHoldingsModule);
}
