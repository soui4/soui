#include "stdafx.h"
#include <SAppCfg.h>
#include "MainDlg.h"
#include "SGameTheme.h"
#include "CnchessSkin.h"
#include "ChessBoard.h"
#include "ChessPiece.h"
#include "SRatingBar.h"
#include <helper/slog.h>

#if defined(__IOS__)
#include <ios_entry.h>
#endif

#define SYS_NAMED_RESOURCE _T("soui-sys-resource")
#ifdef __APPLE__
static const TCHAR * kPath_SysRes = _T("/soui-sys-resource");
#else
static const TCHAR * kPath_SysRes = _T("/../../../soui-sys-resource");
#endif
static const TCHAR *kPath_UiRes = _T("/uires");

using namespace SOUI;

static SStringT getResourceDir()
{
#ifdef __APPLE__
    // macOS 和 iOS 统一：资源安装在 .app/<res_name>/ 下（由 CMake add_macos_res_folder 处理）
    char szBundlePath[1024] = {0};
    GetAppleBundlePath(szBundlePath, sizeof(szBundlePath));
    return S_CA2T(szBundlePath);
#else
    SStringA file(__FILE__);
    file = file.Left(file.ReverseFind(PATH_SLASH));
    return S_CA2T(file);
#endif
}


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));

    int nRet = 0;
    SApplication app(hInstance);
    // Register external window class
    app.RegisterSkinClass<SSkinAni>();
    app.RegisterSkinClass<SSkinPiece>();
    app.RegisterSkinClass<SSkinBoard>();
    app.RegisterWindowClass<SGifPlayer>();
    app.RegisterWindowClass<CChessBoard>();
    app.RegisterWindowClass<CChessPiece>();
    app.RegisterWindowClass<SOUI::SRatingBar>();
    SStringT appDir = app.GetAppDir();

    SAppCfg cfg;
    SStringT resDir = getResourceDir();


    cfg.SetRender(Render_Skia)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetLog(TRUE);


    // Load system resources
#ifdef ENABLE_BUILD_RESOURCE
#if defined(BUILD_SYS_RES)
    cfg.SetSysResPeHandle(hInstance);
#else
    cfg.SetSysResPeFile(SYS_NAMED_RESOURCE);
#endif
#else
    cfg.SetSysResFile(resDir + kPath_SysRes);
#endif//ENABLE_BUILD_RESOURCE

#if !defined(_DEBUG) && defined(ENABLE_BUILD_RESOURCE)
    cfg.SetAppResPeHandle(hInstance);
#else
    cfg.SetAppResFile(resDir + kPath_UiRes);
#endif

    if (!cfg.DoConfig(&app))
    {
        return -1;
    }
    // 覆盖系统 MessageBox 模板: 使用与游戏主题一致的弹窗样式(深木色+金色)
    app.SetMessageBoxTemplateResId(_T("LAYOUT:XML_MSGBOX"));
    // Show main dialog
    {
        SLog::setLogLevel(LOG_LEVEL_DEBUG);
        MyProfile *pMyProfile = new MyProfile();

        SAutoRefPtr<SGameTheme> theme(new SGameTheme(),FALSE);
        CMainDlg dlgMain(theme);
        dlgMain.Create(GetActiveWindow());
        if(dlgMain.SendMessage(WM_INITDIALOG))
        {
            dlgMain.CenterWindow(dlgMain.m_hWnd);
            #ifdef __IOS__
            dlgMain.ShowWindow(SW_MAXIMIZE);
            #else
            dlgMain.ShowWindow(SW_SHOWNORMAL);
            #endif//__IOS__
            nRet = app.Run(dlgMain.m_hWnd);
        }
        delete pMyProfile;
    }

    OleUninitialize();
    return nRet;
}


#if defined(__IOS__)
int main(int argc, char **argv)
{
    return swinx_ios_entry(argc, argv, _tWinMain);
}
#elif !defined(_WIN32) || defined(__MINGW32__)
int main(int argc, char **argv)
{
    HINSTANCE hInst = GetModuleHandle(NULL);
    return _tWinMain(hInst, 0, NULL, SW_SHOWNORMAL);
}
#endif //_WIN32