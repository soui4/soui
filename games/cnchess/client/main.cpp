#include "stdafx.h"
#include <SAppCfg.h>
#include "MainDlg.h"
#include "SGameTheme.h"

static const TCHAR * kPath_SysRes = _T("/../../../soui-sys-resource");
static const TCHAR *kPath_UiRes = _T("/uires");

using namespace SOUI;

static SStringT getSourceDir()
{
    SStringA file(__FILE__);
    file = file.Left(file.ReverseFind(PATH_SLASH));
    return S_CA2T(file);
}


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));

    int nRet = 0;
    SApplication app(hInstance);
    
    SStringT appDir = app.GetAppDir();

    SAppCfg cfg;
    SStringT srcDir = getSourceDir();


    cfg.SetRender(Render_Skia)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetLog(TRUE);
#ifdef _WIN32
    cfg.SetSysResPeFile(appDir + _T("/soui-sys-resource.dll"));
    cfg.SetAppResPeHandle(hInstance);
#else
    SStringT strAppRes = appDir+_T("/chess_uires.zip");
    if(GetFileAttributes(strAppRes) != INVALID_FILE_ATTRIBUTES)
        cfg.SetAppResZip(strAppRes, "");
    else
        cfg.SetAppResFile(srcDir + kPath_UiRes);

    SStringT strSysRes = appDir + _T("/soui-sys-resource.zip");
    if(GetFileAttributes(strSysRes) != INVALID_FILE_ATTRIBUTES)
        cfg.SetSysResZip(strSysRes, "");
    else
        cfg.SetSysResFile(srcDir + kPath_SysRes);
#endif
    if (!cfg.DoConfig(&app))
    {
        return -1;
    }
#ifndef _WIN32
    AddFontResource((srcDir + _T("/../../../simsun.ttc")).c_str());
#endif
    // Show main dialog
    {
        SAutoRefPtr<SGameTheme> theme(new SGameTheme(),FALSE);
        SStringT themeDir = appDir + _T("/def_theme");
        if(GetFileAttributes(themeDir) == INVALID_FILE_ATTRIBUTES)
        {
            themeDir = srcDir + _T("/def_theme");
        }
        theme->Load(themeDir);

        CMainDlg dlgMain(theme);
        dlgMain.Create(GetActiveWindow());
        if(dlgMain.SendMessage(WM_INITDIALOG))
        {
            dlgMain.CenterWindow(dlgMain.m_hWnd);
            dlgMain.ShowWindow(SW_SHOWNORMAL);
            nRet = app.Run(dlgMain.m_hWnd);
        }
    }

    OleUninitialize();
    return nRet;
}


#if !defined(_WIN32) || defined(__MINGW32__) 
int main(int argc, char **argv)
{
    HINSTANCE hInst = GetModuleHandle(NULL);
    return _tWinMain(hInst, 0, NULL, SW_SHOWNORMAL);
}
#endif //_WIN32