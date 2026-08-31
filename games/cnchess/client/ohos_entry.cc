//
// ohos_entry.cc
//
// cnchess 鸿蒙（OHOS）平台入口实现，对应 Android 端 android_entry.cc。
//
// 实现 Soui4OhosEntry 的四个虚函数：
//   - InitApp:       创建 SApplication，注册自定义皮肤/窗口类，配置资源（Skia + Stb）
//   - UninitApp:     释放 SApplication
//   - ScreenStartup: 创建 CMainDlg 主窗口
//   - ScreenShutdown:销毁主窗口
//
// 资源加载：通过 OHOS rawfile（NativeResourceManager）直读 uires / soui_sys_res，
//   与 ohos-demo 的 SouiOhosApp::InitApp 方式一致。
//
// 同时注册 N-API 模块 "cnchess"，ArkTS 侧 import 'libcnchess.so' 触发加载，
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
#include "MainDlg.h"
#include "SGameTheme.h"
#include "CnchessSkin.h"
#include "ChessBoard.h"
#include "ChessPiece.h"
#include "myprofile.h"
#include <gif/SSkinAni.h>
#include <gif/SGifPlayer.h>

#define kLogTag "cnchess-ohos"

class CnchessOhosApp : public Soui4OhosEntry {
    SApplication *m_souiApp = nullptr;
    std::map<long, SAutoRefPtr<SHostWnd>> m_screenHostMap;
    SAutoRefPtr<SGameTheme> m_theme;
    std::string m_appDir;

public:
    CnchessOhosApp() {
        InitSoui4OhosEntry(this);
    }

    BOOL InitApp(SouiOhosAssetManager assetMgr, LPCSTR pszAssetDir) override;
    void UninitApp() override;
    HWND ScreenStartup(long screenId, LPCSTR pszLayout) override;
    void ScreenShutdown(long screenId) override;
};

static CnchessOhosApp theApp;

BOOL CnchessOhosApp::InitApp(SouiOhosAssetManager assetMgr, LPCSTR pszAssetDir) {
    SLOGI() << "CnchessOhosApp::InitApp assetDir=" << (pszAssetDir ? pszAssetDir : "(null)");

    m_appDir = pszAssetDir ? pszAssetDir : "";

    m_souiApp = new SApplication((HINSTANCE) nullptr);
    // 注册自定义皮肤类
    m_souiApp->RegisterSkinClass<SSkinAni>();
    m_souiApp->RegisterSkinClass<SSkinPiece>();
    m_souiApp->RegisterSkinClass<SSkinBoard>();
    // 注册自定义窗口类
    m_souiApp->RegisterWindowClass<SGifPlayer>();
    m_souiApp->RegisterWindowClass<CChessBoard>();
    m_souiApp->RegisterWindowClass<CChessPiece>();

    SAppCfg cfg;
    cfg.SetRender(Render_Skia)
            .SetImgDecoder(ImgDecoder_Stb)
            .SetAppDir(S_CA2T(m_appDir.c_str(), CP_UTF8));

    // 系统/应用资源：从 HAP rawfile 直读
    NativeResourceManager *resMgr = reinterpret_cast<NativeResourceManager *>(assetMgr);
    if (!resMgr) {
        SLOGE() << "CnchessOhosApp::InitApp: assetMgr is null";
        delete m_souiApp;
        m_souiApp = nullptr;
        return FALSE;
    }
    cfg.SetSysResOhosRawFile(resMgr, _T("soui_sys_res"));
    cfg.SetAppResOhosRawFile(resMgr, _T("uires"));

    BOOL ok = cfg.DoConfig(m_souiApp);
    if (!ok) {
        SLOGE() << "CnchessOhosApp::InitApp DoConfig failed";
        delete m_souiApp;
        m_souiApp = nullptr;
        return FALSE;
    }

    SLOGI() << "CnchessOhosApp::InitApp completed";
    return TRUE;
}

void CnchessOhosApp::UninitApp() {
    SLOGI() << "CnchessOhosApp::UninitApp";
    if (m_souiApp) {
        m_souiApp->Release();
        m_souiApp = nullptr;
    }
}

HWND CnchessOhosApp::ScreenStartup(long screenId, LPCSTR pszLayout) {
    SLOGI() << "CnchessOhosApp::ScreenStartup screenId=" << screenId
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

    // 主题在连接服务器成功后由 ThemeDownloadManager 下载加载
    m_theme.Attach(new SGameTheme());
    // 加载头像
    MyProfile *pMyProfile = new MyProfile();
    // 创建主窗口
    CMainDlg *pDlg = new CMainDlg(m_theme);
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

    SLOGI() << "CnchessOhosApp::ScreenStartup OK screenId=" << screenId;
    return ret;
}

void CnchessOhosApp::ScreenShutdown(long screenId) {
    SLOGI() << "CnchessOhosApp::ScreenShutdown screenId=" << screenId;
    if (screenId == 0) return;
    auto it = m_screenHostMap.find(screenId);
    if (it == m_screenHostMap.end()) {
        SLOGW() << "ScreenShutdown: screenId=" << screenId << " NOT found";
        return;
    }
    it->second->DestroyWindow();
    m_screenHostMap.erase(it);
    delete MyProfile::getSingletonPtr();
}

/* ============================================================
 * N-API 模块注册（cnchess）
 *
 * ArkTS 侧 import 'libcnchess.so' 触发本模块加载，
 * 加载时 __attribute__((constructor)) 执行 napi_module_register，
 * 同时 theApp 静态构造函数调用 InitSoui4OhosEntry 注册业务入口。
 * ============================================================ */

#include "napi/native_api.h"

static napi_value InitCnchess(napi_env env, napi_callback_info info) {
    (void)info;
    // 触发 CnchessOhosApp 静态构造（若尚未执行），确保 Soui4OhosEntry 已注册。
    // theApp 的构造函数在库加载时已执行，这里仅返回 true。
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

EXTERN_C_START
static napi_value CnchessModuleInit(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"init", nullptr, InitCnchess, nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module g_cnchessModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = CnchessModuleInit,
    .nm_modname = "cnchess",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterCnchessModule(void) {
    napi_module_register(&g_cnchessModule);
}
