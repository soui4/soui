// mclistview_demo.cpp : main source file
//

#include "stdafx.h"
#include "StudentSmsDlg.h"
#include <SAppCfg.h>

#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")

using namespace SNS;

#define INIT_R_DATA
#include "res/resource.h"

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
    cfg.SetSysResZip(srcDir+_T("/../../soui-sys-resource.zip"));
#endif //_WIN32
    cfg.SetAppResFile(srcDir+_T("/uires"));
     if (!cfg.DoConfig(&app))
    {
        return -1;
    }
#ifndef _WIN32
    AddFontResource(srcDir+_T("/../../simsun.ttc"));
#endif //_WIN32

    // 如果需要在代码中使用R::id::namedid这种方式来使用控件必须要这一行代码：2016年2月2日，R::id,R.name是由uiresbuilder
    // 增加-h .\res\resource.h 这2个参数后生成的。
    app.InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id, sizeof(R.id) / sizeof(int));
    //register extended ctrls
    {
        // show main window
        CStudentSmsDlg dlgMain;
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