#include "stdafx.h"
#include "MainDlg.h"
#include "SImageBtnEx.h"
#include "SouiRealWndHandler.h"
#include "SImgCanvas.h"
#include "CmdLine.h"
#include <helper/SAppDir.h>
#include <interface/SMessageBox-i.h>
#include <zipresprovider-param.h>
#include <SAppCfg.h>
#include "SCtrlsRegister.h"
#include "AttrStorage.h"
#include "designer/SizingFrame.h"

#pragma comment(lib, "shlwapi.lib")


//定义唯一的一个R,UIRES对象,ROBJ_IN_CPP是resource.h中定义的宏。
#define INIT_R_DATA
#include "res/resource.h"

#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
static const TCHAR *kPath_SysRes = _T("/soui-sys-resource.zip");
static const TCHAR *kPath_AppRes = _T("/uieditor_uires");

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));
    int nRet = 0;

    SApplication theApp(hInstance);

    SStringT appDir = theApp.GetAppDir();
    SetCurrentDirectory(appDir);

    SAppCfg cfg;
    cfg.SetRender(Render_Skia)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetAppDir(appDir)
        .SetLog(TRUE, LOG_LEVEL_INFO, "uiedtior");
    
#ifdef _WIN32
    cfg.SetSysResPeFile(SYS_NAMED_RESOURCE);
    cfg.SetAppResPeHandle(hInstance);
#else
    cfg.SetSysResZip(appDir + kPath_SysRes);
    cfg.SetAppResFile(appDir + kPath_AppRes);
#endif

    if (!cfg.DoConfig(&theApp))
    {
        return -1;
    }

    theApp.InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id, sizeof(R.id) / sizeof(int));
    
    SAutoRefPtr<IAttrStorageFactory> attrStorageFactory(new SAttrStorageFactory(), FALSE);
    theApp.SetAttrStorageFactory(attrStorageFactory);

    Scintilla_RegisterClasses(hInstance);

    theApp.RegisterWindowClass<SImageBtnEx>();
    theApp.RegisterWindowClass<SImgCanvas>();
    theApp.RegisterWindowClass<SSizingFrame>();
    // 注册扩展控件
    SCtrlsRegister::RegisterCtrls(&theApp);

    //读取自定义消息框布局
    theApp.SetMessageBoxTemplateResId(_T("LAYOUT:xml_messagebox"));
    //设置真窗口处理接口
    CSouiRealWndHandler * pRealWndHandler = new CSouiRealWndHandler();
    theApp.SetRealWndHandler(pRealWndHandler);
    pRealWndHandler->Release();
    {
        CCmdLine cmdLine(GetCommandLine());
        CMainDlg dlgMain;
        if (cmdLine.GetParamCount() > 1)
            dlgMain.m_cmdWorkspaceFile = cmdLine.GetParam(1);
        dlgMain.Create(GetActiveWindow());
        dlgMain.SendMessage(WM_INITDIALOG);
        dlgMain.CenterWindow(dlgMain.m_hWnd);
        dlgMain.ShowWindow(SW_MAXIMIZE);
        nRet = theApp.Run(dlgMain.m_hWnd);
    }

    Scintilla_ReleaseResources();

    OleUninitialize();
    return nRet;
}

#if !defined(_WIN32) || defined(__MINGW32__) 
int main(int argc,char **argv){
	return _tWinMain(GetModuleHandle(NULL),0,GetCommandLine(),SW_SHOWDEFAULT);
}
#endif//_WIN32