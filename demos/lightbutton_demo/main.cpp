#include "stdafx.h"
#include <SLightButton.h>
#include <SAppCfg.h>
#include "MainDlg.h"

static const TCHAR * kPath_SysRes = _T("/../../soui-sys-resource");
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
    SAppCfg cfg;
    SStringT srcDir = getSourceDir();
    cfg.SetRender(Render_Skia)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetAppDir(srcDir)
        .SetSysResFile(srcDir + kPath_SysRes)
        .SetAppResFile(srcDir + kPath_UiRes)
        .SetLog(TRUE);
    if (!cfg.DoConfig(&app))
    {
        return -1;
    }
    // Register external window class
    app.RegisterWindowClass<SLightButton>();
    // Show main dialog
    {
        CMainDlg dlgMain;
        dlgMain.Create(GetActiveWindow());
        dlgMain.SendMessage(WM_INITDIALOG);
        dlgMain.CenterWindow(dlgMain.m_hWnd);
        dlgMain.ShowWindow(SW_SHOWNORMAL);
        nRet = app.Run(dlgMain.m_hWnd);
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