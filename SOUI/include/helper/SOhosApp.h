#ifndef __SOHOSAPP__H__
#define __SOHOSAPP__H__

#include <SApp.h>
#include <SAppCfg.h>
#include <core/SHostWnd.h>

#if defined(__OHOS__)
#include <node_api.h>
#endif

SNSBEGIN

struct SOUI_EXP SOhosAppCallbacks {
    const char *appName;
    const char *appResourceDir;
    const char *appResourceZipName;
    const char *sysResourceDir;
    const char *logName;
    void (*registerClasses)(SApplication &app);
    void *(*onAppConfigured)(SApplication &app, const char *rawRoot, const char *appDir);
    void (*onAppCleanup)(SApplication &app, void *context);
    SHostWnd *(*createMainWindow)(int width, int height);
    void (*destroyMainWindow)(SHostWnd *host);
};

#if defined(__OHOS__)
SOUI_EXP napi_value SOhosNapiInit(napi_env env, napi_value exports, const SOhosAppCallbacks *callbacks);
#endif

SNSEND

#endif // __SOHOSAPP__H__
