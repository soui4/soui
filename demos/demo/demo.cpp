#include "stdafx.h"
#include <SAppCfg.h>
#include <SouiFactory.h>
#include <helper/SMenuWndHook.h>
#include <helper/SAutoBuf.h>
#include <SCtrlsRegister.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "uianimation/UiAnimationWnd.h"
#include "SmileyCreateHook.h"
#elif defined(__IOS__)
#include <ios_entry.h>
#endif

#include "appledock/SDesktopDock.h"
#include "SMatrixWindow.h"
//#include <SScintillaView.h>
#include <SEdit2.h>
#include "clock/sclock.h"
#include "FpsWnd.h"
//#include <vld.h>
//<-- defines a slog output with tag="demo"
#define kLogTag "demo"
//-->


#include "MainDlg.h"

#define	RESTYPE_FILE  0 // load resources from file, fall back to PE resources on failure
#define	RESTYPE_PE 1 // load UI resources from PE resources
#define	RESTYPE_ZIP 2 // load resources from a zip package
#define RESTYPE_7Z 3// load resources from a 7zip package

#ifdef _DEBUG
#define RES_TYPE RESTYPE_FILE      // load from file, fall back to PE resources on failure
#else
#define RES_TYPE RESTYPE_PE		// load UI resources from PE resources
#endif

#define SYS_NAMED_RESOURCE _T("soui-sys-resource")
#ifdef __APPLE__
    static const TCHAR *kPath_SysRes = _T("/soui-sys-resource");
#else
    static const TCHAR *kPath_SysRes = _T("/../../soui-sys-resource");
#endif //__APPLE__

#include "skin/SSkinLoader.h"
#include "trayicon/SShellTray.h"
#include "qrcode/SQrCtrl.h"


#define INIT_R_DATA
#include "res/resource.h"

// ============================================================================
// NativeCall handler example: registered via
// IScriptFactory::RegisterNativeCallHandler.
// Lua-side NativeCall("cppSum", 1, 2, ...) arrives here; the variadic
// arguments come as a VARIANT array plus a count, unboxed by index
// (nil->VT_EMPTY, boolean->VT_BOOL, integer->VT_I8, number->VT_R8,
// string->VT_LPSTR). The boxes live on the bridge stack; all argument
// data is valid only during the call - do not free or retain it.
// ============================================================================
static int DemoNativeCallHandler(void *ctx, const char *name, const VARIANT *args, int argc)
{
    (void)ctx;
    SLOGW2("demo") << "NativeCall handler: name=" << name << " argc=" << argc;
    if (strcmp(name, "cppSum") == 0)
    {
        int64_t sum = 0;
        for (int i = 0; i < argc; i++)
        {
            switch (args[i].vt)
            {
            case VT_I8:  sum += args[i].llVal; break;
            case VT_I4:  sum += args[i].lVal;  break;
            case VT_R8:  sum += (int64_t)args[i].dblVal; break;
            default: break;
            }
        }
        return (int)sum;
    }
    return -1;
}

void demo_SWinxLogCallback(const char *pLogStr, int level){
    SLOG("swinx",level)<<pLogStr;
}
static SStringT getSourceDir()
{
#ifdef __APPLE__
    // macOS and iOS unified: resources are installed under .app/<res_name>/ (handled by CMake add_macos_res_folder)
    char szBundlePath[1024] = {0};
    GetAppleBundlePath(szBundlePath, sizeof(szBundlePath));
    return S_CA2T(szBundlePath);
#else//__APPLE__
    TCHAR szModule[MAX_PATH] = {0};
    GetModuleFileName(NULL, szModule, MAX_PATH);
    SStringT str = szModule;
    int slash = str.ReverseFind(PATH_SLASH);
    if (slash >= 0) str = str.Left(slash + 1);
    return str + _T("demo_res");
#endif//__APPLE__
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
    // OleInitialize must be called to initialize the runtime environment
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));
    int nRet = 0;

    // Change the working directory to the directory containing demo
    SStringT appDir = getSourceDir();
    SetCurrentDirectory(appDir);
#ifdef __linux__
    AddFontResource((appDir + _T("/../../simsun.ttc")).c_str());
#elif defined(__APPLE__)
    AddFontResource((appDir + _T("/fonts/simsun.ttc")).c_str());
#endif
    //int nType = IDYES;
    int nType = MessageBox(GetActiveWindow(), _T("Select render type://n[yes]: Skia\n[no]:GDI\n[cancel]:Quit"), _T("select a render"), MB_ICONQUESTION | MB_YESNOCANCEL);
    if (nType == IDCANCEL)
    {
        return 0;
    }
    #ifndef _WIN32
    SetSwinxLogCallback(demo_SWinxLogCallback,0);
    #endif//_WIN32
    SApplication app(hInstance);
    SAppCfg cfg;
    cfg.SetRender(nType == IDYES ? Render_Skia : Render_Gdi)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetAppDir(appDir)
        .SetLog(TRUE, 2, "demo")
        .EnableMultiLang(_T("translator:lang_cn"),TRUE);
#if (defined(DLL_CORE) || (defined(LIB_CORE) && defined(LIB_SOUI_COM)))
    cfg.EnableScript(TRUE);
#endif // DLL_CORE

// Load system resources
#ifdef ENABLE_BUILD_RESOURCE
#if (defined(LIB_CORE) && defined(LIB_SOUI_COM))
    cfg.SetSysResPeHandle(hInstance);
#else
    cfg.SetSysResPeFile(SYS_NAMED_RESOURCE);
#endif
#else
    cfg.SetSysResFile(appDir + kPath_SysRes);
#endif//ENABLE_BUILD_RESOURCE

#if (RES_TYPE == RESTYPE_PE) && defined(ENABLE_BUILD_RESOURCE)
    cfg.SetAppResPeHandle(hInstance);
#elif (RES_TYPE == RESTYPE_ZIP) // load from a ZIP package
    cfg.SetAppResZipFile(appDir + _T("/uires.zip"), "souizip");
#elif (RES_TYPE == RESTYPE_7Z)  // load from a 7z package
    cfg.SetAppRes7ZipFile(appDir + _T("/uires.zip"), "souizip");
#else // #if (RES_TYPE == RESTYPE_FILE)// load from file
    cfg.SetAppResFile(appDir + _T("/uires"));
#endif

    // Register externally extended controls and SkinObj classes into SApplication
    SWkeLoader wkeLoader;
    wkeLoader.Init(_T("wke.dll"));

    SCtrlsRegister::RegisterCtrls(&app);
    app.RegisterSkinClass<SDemoSkin>();

    app.RegisterWindowClass<SMatrixWindow>();   //
    app.RegisterWindowClass<S3dWindow>();       //
    app.RegisterWindowClass<SFreeMoveWindow>(); //
    app.RegisterWindowClass<SClock>();          //
    app.RegisterWindowClass<SDesktopDock>(); // register SDesktopDock
    //app.RegisterWindowClass<SScintillaView>(); // register the windowless Scintilla edit control

    app.RegisterWindowClass<SInterpolatorView>();
    app.RegisterWindowClass<SPathView>();
    app.RegisterWindowClass<SQrCtrl>();
    app.RegisterWindowClass<SProgressRing>();
    app.RegisterWindowClass<SCheckBox2>();
    app.RegisterWindowClass<SAniWindow>();
    app.RegisterWindowClass<SGroupList>();

    app.RegisterWindowClass<SShellTray>();
    app.RegisterWindowClass<FpsWnd>();
    app.RegisterWindowClass<SEdit2>();
#if defined(_WIN32) && !defined(__MINGW32__)
    if (SUCCEEDED(CUiAnimation::Init()))
    {
        app.RegisterWindowClass<SUiAnimationWnd>(); // register the animation control
    }
#endif

    if (!cfg.DoConfig(&app))
    {
        return -1;
    }
    // This line is required to use controls via R::id::namedid in code. Since Feb 2, 2016, R::id/R::name are generated by uiresbuilder with the added -h .\res\resource.h arguments.
    app.InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id, sizeof(R.id) / sizeof(int));
    SSkinLoader *SkinLoader = new SSkinLoader(&app);
    SkinLoader->LoadDefSkin();

#ifdef _WIN32
    // Draw the menu border via a hook
        SMenuWndHook::InstallHook(hInstance, L"_skin.sys.menu.border");
#endif

    // Demonstrates using R.color.xxx / R.string.xxx in code.
    COLORREF crRed = GETCOLOR(R.color.red);
    SStringW strTitle = GETSTRING(R.string.title);
    COLORREF crTxtTheme = GETCOLOR(SNamedColor::THEME_COLOR_TXT_NORMAL);
    app.EnableNotifyCenter(TRUE);
    {
#if defined(_WIN32) && !defined(__MINGW32__)
        SmileyCreateHook smileyHook;
#endif
        // Set the tooltip window layout
        STipWnd::SetLayout(_T("layout:dlg_tip"));
        // Register the script NativeCall handler: the handler registry is
        // component-level shared; once registered on any SIScriptFactory
        // instance, script modules created afterwards can call NativeCall.
        IScriptFactory *pScriptFactory = app.GetScriptFactory();
        if (pScriptFactory)
        {
            pScriptFactory->RegisterNativeCallHandler(DemoNativeCallHandler, NULL);
            SLOGW2("demo") << "NativeCall handler registered";
        }
        CMainDlg dlgMain;
        dlgMain.Create(GetActiveWindow(), 0, 0, 888, 650);
        dlgMain.GetNative()->SendMessage(WM_INITDIALOG);
        dlgMain.CenterWindow();
        dlgMain.ShowWindow(SW_SHOWNORMAL);

        nRet = app.Run(dlgMain.m_hWnd);
    }

    // Application exit
    delete SkinLoader;

#ifdef _WIN32
        // Uninstall the menu border drawing hook
        SMenuWndHook::UnInstallHook();
#endif
#if defined(_WIN32) && !defined(__MINGW32__)
        CUiAnimation::Free();
#endif
    OleUninitialize();
    return nRet;
}

#if defined(__IOS__)
int main(int argc, char **argv)
{
    return swinx_ios_entry(argc,argv,_tWinMain);
}
#elif !defined(_WIN32) || defined(__MINGW32__) 
int main(int argc, char ** argv){
	HINSTANCE hInst = GetModuleHandle(NULL);
	return _tWinMain(hInst,0,NULL,SW_SHOWNORMAL);
}
#endif
