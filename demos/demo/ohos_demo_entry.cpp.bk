#include "stdafx.h"
#include <helper/SOhosApp.h>
#include <SCtrlsRegister.h>

#include "MainDlg.h"
#include "SMatrixWindow.h"
#include "appledock/SDesktopDock.h"
#include "clock/sclock.h"
#include "FpsWnd.h"
#include "qrcode/SQrCtrl.h"
#include "skin/SSkinLoader.h"
#include "trayicon/SShellTray.h"

#include <node_api.h>

#define INIT_R_DATA
#include "res/resource.h"

namespace {

void registerDemoClasses(SOUI::SApplication &app)
{
    SOUI::SCtrlsRegister::RegisterCtrls(&app);
    app.RegisterSkinClass<SDemoSkin>();

    app.RegisterWindowClass<SMatrixWindow>();
    app.RegisterWindowClass<S3dWindow>();
    app.RegisterWindowClass<SFreeMoveWindow>();
    app.RegisterWindowClass<SClock>();
    app.RegisterWindowClass<SDesktopDock>();
    app.RegisterWindowClass<SInterpolatorView>();
    app.RegisterWindowClass<SPathView>();
    app.RegisterWindowClass<SQrCtrl>();
    app.RegisterWindowClass<SProgressRing>();
    app.RegisterWindowClass<SCheckBox2>();
    app.RegisterWindowClass<SAniWindow>();
    app.RegisterWindowClass<SGroupList>();
    app.RegisterWindowClass<SShellTray>();
    app.RegisterWindowClass<FpsWnd>();
}

void *onDemoConfigured(SOUI::SApplication &app, const char *, const char *)
{
    app.InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id, sizeof(R.id) / sizeof(int));
    SSkinLoader *skinLoader = new SSkinLoader(&app);
    skinLoader->LoadDefSkin();
    SOUI::STipWnd::SetLayout(_T("layout:dlg_tip"));
    return skinLoader;
}

void onDemoCleanup(SOUI::SApplication &, void *context)
{
    delete static_cast<SSkinLoader *>(context);
}

SOUI::SHostWnd *createDemoMainWindow(int width, int height)
{
    CMainDlg *dlg = new CMainDlg;
    if (!dlg->Create(GetActiveWindow(), 0, 0, width, height))
    {
        delete dlg;
        return nullptr;
    }
    return dlg;
}

void destroyDemoMainWindow(SOUI::SHostWnd *host)
{
    delete host;
}

const SOUI::SOhosAppCallbacks kDemoCallbacks = {
    "souidemo",
    "demo",
    "uires.zip",
    "soui-sys-resource",
    "ohos_demo",
    registerDemoClasses,
    onDemoConfigured,
    onDemoCleanup,
    createDemoMainWindow,
    destroyDemoMainWindow,
};

napi_value init(napi_env env, napi_value exports)
{
    return SOUI::SOhosNapiInit(env, exports, &kDemoCallbacks);
}

extern "C" {
static napi_module g_souiDemoModule = {
    NAPI_MODULE_VERSION,
    0,
    __FILE__,
    init,
    "souidemo",
    nullptr,
    { 0 },
};

static napi_module g_libsouiDemoModule = {
    NAPI_MODULE_VERSION,
    0,
    __FILE__,
    init,
    "libsouidemo",
    nullptr,
    { 0 },
};

__attribute__((constructor)) static void registerSouiDemoModules()
{
    napi_module_register(&g_souiDemoModule);
    napi_module_register(&g_libsouiDemoModule);
}
}

} // namespace
