#include <jni.h>
#include <android/asset_manager_jni.h>
#include <string>
#include <map>

#include <souistd.h>
#include <SAppCfg.h>
#include <logdef.h>
#include <soui4android.h>
#include "SouiRealWndHandler.h"
#include "MainDlg.h"
#include <helper/slog.h>

#define kLogTag "soui-demo-jni"

using namespace SNS;


void (SwinxLogCallback)(const char *pLogStr, int level){
    SLOG("swinx",level)<<pLogStr;
}

class SouiAndroidApp : public Soui4AndroidEntry{
    SApplication* m_souiApp = nullptr;
    std::map<long,SAutoRefPtr<SHostWnd> > m_screenHostMap;
public:
    SouiAndroidApp(){
        InitSoui4AndroidEntry(this);
    }
    HWND ScreenStartup(long screenId, LPCSTR pszLayout) override;
    void ScreenShutdown(long screenId) override;
    void UninitApp() override;
    BOOL InitApp(AAssetManager* assetMgr, LPCSTR pszAssetDir) override;
};

static SouiAndroidApp theApp;

BOOL SouiAndroidApp::InitApp(AAssetManager* assetMgr, LPCSTR pszAssetDir){
    m_souiApp = new SApplication((HINSTANCE) nullptr);

    SetSwinxLogCallback(SwinxLogCallback,0);
    SAppCfg cfg;
    cfg.SetRender(Render_Skia)
            .SetImgDecoder(ImgDecoder_Stb)
            .SetAppDir(S_CA2T(pszAssetDir, CP_UTF8))
            .SetSysResAndroidAsset(assetMgr,_T("soui_sys_res"))
            .SetAppResAndroidAsset(assetMgr,_T("uires"));
    BOOL ok = cfg.DoConfig(m_souiApp);
    if(!ok){
        delete m_souiApp;
        return FALSE;
    }
    SAutoRefPtr<IRealWndHandler> realWndHander(new CSouiRealWndHandler,FALSE);
    m_souiApp->SetRealWndHandler(realWndHander);
    return TRUE;
}

void SouiAndroidApp::UninitApp(){
    SASSERT(m_souiApp);
    m_souiApp->Release();
    m_souiApp = nullptr;
}

HWND SouiAndroidApp::ScreenStartup(long screenId, LPCSTR pszLayout) {
    SASSERT(m_souiApp);
    if (screenId == 0 || !m_souiApp) {
        SLOGE() << "SouiStartup: invalid args (screenId=" << screenId << ", m_souiApp=" << (void*)m_souiApp << ")";
        return 0;
    }
    //AndroidPlatformAPI& api = AndroidPlatformAPI::instance();
    LPCSTR layout = pszLayout;
    auto existing = m_screenHostMap.find(screenId);
    if (existing != m_screenHostMap.end()) {
        SLOGW()<<"SouiStartup: re-entering screenId="<<screenId<<", destroying previous SHostWnd first";
        existing->second->DestroyWindow();
        m_screenHostMap.erase(existing);
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
    //int wid = api.getScreenWidth();
    //int hei = api.getScreenHeight();
    pDlg->ShowWindow(SW_SHOW);

    HWND ret = pDlg->m_hWnd;
    m_screenHostMap.insert(std::make_pair(screenId, pDlg));
    pDlg->Release();

    SLOGI()<<"SouiStartup: OK screenId="<<screenId<<" layout="<<(layout ? layout : "(null)")<<" hMainWnd="<<(int)ret;
    return ret;
}

void SouiAndroidApp::ScreenShutdown(long screenId) {
    if (screenId == 0) return;
    auto it = m_screenHostMap.find(screenId);
    if (it == m_screenHostMap.end()) {
        SLOGW()<<"SouiShutdown: screenId="<<screenId<<" NOT found in m_screenHostMap "
               "(already destroyed? never started?)";
        return;
    }
    SLOGI()<<"SouiShutdown: screenId="<<screenId<<", calling SHostWnd::DestroyWindow()";
    it->second->DestroyWindow();
    m_screenHostMap.erase(it);
}