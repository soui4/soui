#include "stdafx.h"
#include <SAppCfg.h>
#include "MainDlg.h"
#include "SGameTheme.h"
#include "CnchessSkin.h"
#include "ChessBoard.h"
#include "ChessPiece.h"
#include <helper/slog.h>

#if defined(__IOS__)
#include <ios_entry.h>
#endif

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
    SStringT appDir = app.GetAppDir();

    SAppCfg cfg;
    SStringT resDir = getResourceDir();


    cfg.SetRender(Render_Skia)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetLog(TRUE);
#ifdef _WIN32
    cfg.SetSysResPeFile(appDir + _T("/soui-sys-resource.dll"));
    cfg.SetAppResPeHandle(hInstance);
    // SStringT strAppRes = resDir + kPath_UiRes;
    // cfg.SetAppResFile(strAppRes);
#else
    SStringT strAppRes = appDir+_T("/chess_uires.zip");
    if(GetFileAttributes(strAppRes) != INVALID_FILE_ATTRIBUTES)
        cfg.SetAppResZip(strAppRes, "");
    else
        cfg.SetAppResFile(resDir + kPath_UiRes);

    SStringT strSysRes = appDir + _T("/soui-sys-resource.zip");
    if(GetFileAttributes(strSysRes) != INVALID_FILE_ATTRIBUTES)
        cfg.SetSysResZip(strSysRes, "");
    else
        cfg.SetSysResFile(resDir + kPath_SysRes);
#endif
    if (!cfg.DoConfig(&app))
    {
        return -1;
    }
#ifndef _WIN32
    // 加载宋体字体
    #ifdef __APPLE__
    // Apple 平台：字体应位于 bundle 根目录
    SStringT strFont = resDir + _T("/simsun.ttc");
    #else
    // Linux 平台：字体位于源码树上三级目录
    SStringT strFont = resDir + _T("/../../../simsun.ttc");
    #endif
    if(GetFileAttributes(strFont) != INVALID_FILE_ATTRIBUTES)
        AddFontResource(strFont.c_str());
#endif
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