/**
 * demo_native.cpp
 *
 * SOUI for OHOS 业务入口实现，对应 Android 端 demo_native.cpp。
 *
 * 实现 Soui4OhosEntry 的四个虚函数：
 *   - InitApp:       创建 SApplication，配置资源（Skia 渲染 + Stb 图像解码）
 *   - UninitApp:     释放 SApplication
 *   - ScreenStartup: 创建 SHostWnd 主窗口（CMainDlg 或通用 SHostWnd）
 *   - ScreenShutdown:销毁主窗口
 *
 * 资源加载说明：
 *   OHOS 的 rawfile 资源管理器（SouiOhosAssetManager，实际指向 NativeResourceManager*）
 *   已通过 SAppCfg::SetSysResOhosRawFile / SetAppResOhosRawFile 接入，
 *   demo 直接从 HAP rawfile 直读 uires / soui_sys_res，无需先复制到 filesDir。
 */

#include <string>
#include <map>

#include <souistd.h>
#include <SAppCfg.h>
#include <logdef.h>
#include <soui4ohos.h>
#include <rawfile/raw_file_manager.h>
#include <helper/slog.h>

#include "SouiRealWndHandler.h"
#include "MainDlg.h"

#define kLogTag "soui-ohos-demo"

SNSBEGIN

void SwinxLogCallback(const char *pLogStr, int level) {
    SLOG("swinx", level) << pLogStr;
}

class SouiOhosApp : public Soui4OhosEntry {
    SApplication *m_souiApp = nullptr;
    std::map<long, SAutoRefPtr<SHostWnd>> m_screenHostMap;
    std::string m_appDir;

public:
    SouiOhosApp() {
        InitSoui4OhosEntry(this);
    }

    BOOL InitApp(SouiOhosAssetManager assetMgr, LPCSTR pszAssetDir) override;
    void UninitApp() override;
    HWND ScreenStartup(long screenId, LPCSTR pszLayout) override;
    void ScreenShutdown(long screenId) override;
};

static SouiOhosApp theApp;

BOOL SouiOhosApp::InitApp(SouiOhosAssetManager assetMgr, LPCSTR pszAssetDir) {
    SLOGI() << "SouiOhosApp::InitApp assetDir=" << (pszAssetDir ? pszAssetDir : "(null)");

    m_appDir = pszAssetDir ? pszAssetDir : "";

    m_souiApp = new SApplication((HINSTANCE) nullptr);

    SetSwinxLogCallback(SwinxLogCallback, 0);

    SAppCfg cfg;
    cfg.SetRender(Render_Skia)
            .SetImgDecoder(ImgDecoder_Stb)
            .SetAppDir(S_CA2T(m_appDir.c_str(), CP_UTF8));

    // 系统/应用资源：从 HAP rawfile 直读（NativeResourceManager 由平台层
    // OH_ResourceManager_InitNativeResourceManager 创建并塞入 SouiOhosAssetManager）
    NativeResourceManager *resMgr = reinterpret_cast<NativeResourceManager*>(assetMgr);
    if (!resMgr) {
        SLOGE() << "SouiOhosApp::InitApp: assetMgr is null";
        delete m_souiApp;
        m_souiApp = nullptr;
        return FALSE;
    }
    cfg.SetSysResOhosRawFile(resMgr, _T("soui_sys_res"));
    cfg.SetAppResOhosRawFile(resMgr, _T("uires"));

    BOOL ok = cfg.DoConfig(m_souiApp);
    if (!ok) {
        SLOGE() << "SouiOhosApp::InitApp DoConfig failed";
        delete m_souiApp;
        m_souiApp = nullptr;
        return FALSE;
    }

    SAutoRefPtr<IRealWndHandler> realWndHandler(new CSouiRealWndHandler, FALSE);
    m_souiApp->SetRealWndHandler(realWndHandler);

    SLOGI() << "SouiOhosApp::InitApp OK";
    return TRUE;
}

void SouiOhosApp::UninitApp() {
    SLOGI() << "SouiOhosApp::UninitApp";
    if (m_souiApp) {
        m_souiApp->Release();
        m_souiApp = nullptr;
    }
}

HWND SouiOhosApp::ScreenStartup(long screenId, LPCSTR pszLayout) {
    SLOGI() << "SouiOhosApp::ScreenStartup screenId=" << screenId
            << " layout=" << (pszLayout ? pszLayout : "(null)");

    if (screenId == 0 || !m_souiApp) {
        SLOGE() << "SouiOhosApp::ScreenStartup: invalid args";
        return 0;
    }

    auto existing = m_screenHostMap.find(screenId);
    if (existing != m_screenHostMap.end()) {
        SLOGW() << "SouiOhosApp::ScreenStartup: re-entering screenId=" << screenId;
        existing->second->DestroyWindow();
        m_screenHostMap.erase(existing);
    }

    SHostWnd *pDlg = nullptr;
    if (pszLayout && ::strcmp(pszLayout, "layout:dlg_main") == 0) {
        pDlg = new CMainDlg();
    } else {
        pDlg = new SHostWnd(S_CA2T(pszLayout ? pszLayout : "", CP_UTF8));
    }

    if (!pDlg->Create(NULL, 0, 0, 50, 50)) {
        SLOGE() << "SouiOhosApp::ScreenStartup: SHostWnd.Create failed";
        delete pDlg;
        return 0;
    }

    pDlg->SendMessage(WM_INITDIALOG);
    pDlg->ShowWindow(SW_SHOW);

    HWND ret = pDlg->m_hWnd;
    m_screenHostMap.insert(std::make_pair(screenId, pDlg));
    pDlg->Release();

    SLOGI() << "hjx, SouiOhosApp::ScreenStartup OK screenId=" << screenId << " hMainWnd=" << (void*)ret;
    return ret;
}

void SouiOhosApp::ScreenShutdown(long screenId) {
    SLOGI() << "SouiOhosApp::ScreenShutdown screenId=" << screenId;
    if (screenId == 0) return;
    auto it = m_screenHostMap.find(screenId);
    if (it == m_screenHostMap.end()) {
        SLOGW() << "SouiOhosApp::ScreenShutdown: screenId=" << screenId << " not found";
        return;
    }
    it->second->DestroyWindow();
    m_screenHostMap.erase(it);
}

SNSEND

/* ============================================================
 * N-API 模块注册（souidemo）
 *
 * ArkTS 侧 import 'libsouidemo.so' 触发本模块加载，
 * 加载时 __attribute__((constructor)) 执行 napi_module_register，
 * 同时 theApp 静态构造函数调用 InitSoui4OhosEntry 注册业务入口。
 * ============================================================ */

#include "napi/native_api.h"

static napi_value InitDemo(napi_env env, napi_callback_info info) {
    (void)info;
    // 触发 SouiOhosApp 静态构造（若尚未执行），确保 Soui4OhosEntry 已注册。
    // theApp 的构造函数在库加载时已执行，这里仅返回 true。
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

EXTERN_C_START
static napi_value DemoModuleInit(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"init", nullptr, InitDemo, nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module g_demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = DemoModuleInit,
    .nm_modname = "souidemo",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterDemoModule(void) {
    napi_module_register(&g_demoModule);
}
