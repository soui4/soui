// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include "MainDlg.h"
#include "SCheckGroup.h"
#include <SAppCfg.h>
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")

#define INIT_R_DATA
#include "res/resource.h"

using namespace SNS;

static SStringT getResourceDir()
{
#ifdef __APPLE__
    char szBunblePath[1024];
    GetAppleBundlePath(szBunblePath, sizeof(szBunblePath));
    return S_CA2T(szBunblePath) + _T("/Contents/Resources/");
#else
    SStringA strFile = __FILE__;
    strFile = strFile.Left(strFile.ReverseFind(PATH_SLASH));
    return S_CA2T(strFile);
#endif
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
    cfg.SetSysResZip(srcDir+_T("data/soui-sys-resource.zip"));
#endif //_WIN32
#if !defined(_WIN32) || defined(_DEBUG)
    cfg.SetAppResFile(srcDir+_T("/uires"));
#else
    cfg.SetAppResPeHandle(hInstance);
#endif
    // regisger external widget and skinobj

    if (!cfg.DoConfig(&app))
    {
        return -1;
    }
#ifndef _WIN32
    AddFontResource(srcDir+"fonts/simsun.ttc");
#endif //_WIN32
    app.RegisterWindowClass<SCheckGroup>();
    // 如果需要在代码中使用R::id::namedid这种方式来使用控件必须要这一行代码：2016年2月2日，R::id,R.name是由uiresbuilder
    // 增加-h .\res\resource.h 这2个参数后生成的。
    app.InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id, sizeof(R.id) / sizeof(int));

    {
        // show main window
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