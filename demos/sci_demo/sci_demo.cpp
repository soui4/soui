// sci_demo.cpp : main source file for the headless Scintilla DUI test app.
//

#include "stdafx.h"
#include "MainDlg.h"
#include <SAppCfg.h>
#include "SScintillaView.h"

#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")

using namespace SNS;

static SStringT getResourceDir()
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
    SStringT srcDir = getResourceDir();
    cfg.SetRender(Render_Skia).SetImgDecoder(ImgDecoder_Stb).SetAppDir(srcDir).SetLog(TRUE);
#ifdef _WIN32
    cfg.SetSysResPeFile(SYS_NAMED_RESOURCE);
#else
    cfg.SetSysResFile(srcDir + _T("/../../soui-sys-resource"));
#endif
    cfg.SetAppResFile(srcDir + _T("/uires"));
    if (!cfg.DoConfig(&app))
    {
        return -1;
    }

    // Register the headless Scintilla DUI control so <scintilla/> can be
    // created from the XML layout (the widget lives in controls.extend).
    app.RegisterWindowClass<SScintillaView>();

    {
        CMainDlg dlgMain;
        nRet = dlgMain.DoModal();
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