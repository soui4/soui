#include <jni.h>
#include <android/asset_manager_jni.h>
#include <string>
#include <map>

#include <souistd.h>
#include <SAppCfg.h>
#include <logdef.h>
#include "AndroidPlatformAPI.h"
#include "SouiRealWndHandler.h"
#include "SouiSurfaceProxy.h"
#include "MainDlg.h"
#include <helper/slog.h>

#define kLogTag "soui-demo-jni"

using namespace SNS;

static SApplication* s_souiApp = nullptr;

void (SwinxLogCallback)(const char *pLogStr, int level){
    SLOG("swinx",level)<<pLogStr;
}

extern SNS::SApplication * SouiInitApp(AAssetManager* assetMgr, LPCSTR pszAssetDir){
    s_souiApp = new SApplication((HINSTANCE) nullptr);

    SetSwinxLogCallback(SwinxLogCallback,0);
    SAppCfg cfg;
    cfg.SetRender(Render_Skia)
            .SetImgDecoder(ImgDecoder_Stb)
            .SetAppDir(S_CA2T(pszAssetDir, CP_UTF8))
            .SetSysResAndroidAsset(assetMgr,_T("soui_sys_res"))
            .SetAppResAndroidAsset(assetMgr,_T("uires"));
    BOOL ok = cfg.DoConfig(s_souiApp);
    if(!ok){
        delete s_souiApp;
        return nullptr;
    }
    SAutoRefPtr<IRealWndHandler> realWndHander(new CSouiRealWndHandler,FALSE);
    s_souiApp->SetRealWndHandler(realWndHander);
    return s_souiApp;
}

extern void SouiUninitApp(SNS::SApplication *pApp){
    SASSERT(s_souiApp == pApp);
    s_souiApp->Release();
    s_souiApp = nullptr;
}

static std::map<long,SAutoRefPtr<SHostWnd> > s_screenHostMap;

HWND SouiStartup(long screenId, LPCSTR pszLayout) {
    SASSERT(s_souiApp);
    if (screenId == 0 || !s_souiApp) {
        SLOGE()<<"SouiStartup: invalid args (screenId="<<screenId<<", s_souiApp="<<(void*)s_souiApp<<")";
        return 0;
    }
    AndroidPlatformAPI& api = AndroidPlatformAPI::instance();
    LPCSTR layout = pszLayout;
    auto existing = s_screenHostMap.find(screenId);
    if (existing != s_screenHostMap.end()) {
        SLOGW()<<"SouiStartup: re-entering screenId="<<screenId<<", destroying previous SHostWnd first";
        existing->second->DestroyWindow();
        s_screenHostMap.erase(existing);
    }

    SHostWnd* pDlg = nullptr;
    if (layout && !::strcmp(layout, "layout:dlg_main")) {
        pDlg = new CMainDlg();
    } else {
        pDlg = new SHostWnd(layout);
    }
    if (!pDlg->Create(NULL,0,0,50,50)) {
        SLOGE()<<"SouiStartup: SHostWnd.Create(NULL) failed screenId="<<screenId<<" layout="<<(layout ? layout : "(null)");
        delete pDlg;
        return 0;
    }
    pDlg->SendMessage(WM_INITDIALOG);
    int wid = api.getScreenWidth();
    int hei = api.getScreenHeight();
    pDlg->ShowWindow(SW_SHOW);

    HWND ret = pDlg->m_hWnd;
    s_screenHostMap.insert(std::make_pair(screenId, pDlg));
    pDlg->Release();

    SLOGI()<<"SouiStartup: OK screenId="<<screenId<<" layout="<<(layout ? layout : "(null)")<<" hMainWnd="<<(int)ret;
    return ret;
}

void SouiShutdown(long screenId) {
    if (screenId == 0) return;
    auto it = s_screenHostMap.find(screenId);
    if (it == s_screenHostMap.end()) {
        SLOGW()<<"SouiShutdown: screenId="<<screenId<<" NOT found in s_screenHostMap "
               "(already destroyed? never started?)";
        return;
    }
    SLOGI()<<"SouiShutdown: screenId="<<screenId<<", calling SHostWnd::DestroyWindow()";
    it->second->DestroyWindow();
    s_screenHostMap.erase(it);
}