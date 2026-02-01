#include "stdafx.h"
#include "MainDlg.h"
#include "SouiRealWndHandler.h"
#include "CmdLine.h"
#include <helper/SAppDir.h>
#include <SAppCfg.h>
#include <SCtrlsRegister.h>
#include "AttrStorage.h"
#include "designer/SizingFrame.h"
#include "SysdataMgr.h"
#include <helper/slog.h>
#define kLogTag "SouiEditor"

//定义唯一的一个R,UIRES对象,ROBJ_IN_CPP是resource.h中定义的宏。
#define INIT_R_DATA
#include "res/resource.h"

#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#ifdef __APPLE__
static const TCHAR *kPath_AppRes = _T("/uires");
static const TCHAR *kPath_SysRes = _T("/data/soui-sys-resource.zip");
#else
static const TCHAR *kPath_AppRes = _T("/uieditor_uires");
static const TCHAR *kPath_SysRes = _T("/../../soui-sys-resource.zip");
#endif

static SStringT getSourceDir()
{
#ifdef __APPLE__
    char szBunblePath[1024];
    GetAppleBundlePath(szBunblePath, sizeof(szBunblePath));
    return S_CA2T(szBunblePath) + _T("/Contents/Resources");
#else
    SStringA file(__FILE__);
    file = file.Left(file.ReverseFind(PATH_SLASH));
	if(*__FILE__=='.'){
		char absPath[MAX_PATH]={0};
		GetFullPathNameA(file,MAX_PATH,absPath,NULL);
		file = absPath;
	}
    return S_CA2T(file);
#endif
}

class CUiEditorApp : public SApplication
{
public:
    CUiEditorApp(HINSTANCE hInst):SApplication(hInst)
    {
    }

    ~CUiEditorApp()
    {
    }

    public:
    IWindow *CreateWindowByName(LPCWSTR pszWndClass) const override{
        IWindow *pRet = SApplication::CreateWindowByName(pszWndClass);
        if(pRet)
            return pRet;
        SStringW strParent = CSysDataMgr::getSingleton().GetUserWidgetParent(pszWndClass);
        if(!strParent.IsEmpty())
        {
            return SApplication::CreateWindowByName(strParent);
        }
        return NULL;
    }

    ISkinObj *CreateSkinByName(LPCWSTR pszSkinClass) const override{
        ISkinObj *pRet = SApplication::CreateSkinByName(pszSkinClass);
        if(pRet)
            return pRet;
        SStringW strParent = CSysDataMgr::getSingleton().GetUserSkinParent(pszSkinClass);
        if(!strParent.IsEmpty())
        {
            return SApplication::CreateSkinByName(strParent);
        }
        return NULL;
    }
};

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));
    int nRet = 0;

    CUiEditorApp theApp(hInstance);

    SStringT appDir = theApp.GetAppDir();
    SetCurrentDirectory(appDir);
    SStringT srcDir = getSourceDir();
    SAppCfg cfg;
    cfg.SetRender(Render_Skia)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetAppDir(appDir)
        .SetLog(TRUE, LOG_LEVEL_INFO, "uiedtior");
#ifdef __linux__
    AddFontResource((srcDir + _T("/../../simsun.ttc")).c_str());
#elif defined(__APPLE__)
    AddFontResource((srcDir + _T("/fonts/simsun.ttc")).c_str());
#endif    
#ifdef _WIN32
    cfg.SetSysResPeFile(SYS_NAMED_RESOURCE);
    cfg.SetAppResPeHandle(hInstance);
#elif defined(__APPLE__)
    cfg.SetSysResZip(srcDir + kPath_SysRes);
    cfg.SetAppResFile(srcDir + kPath_AppRes);
#else
    cfg.SetSysResZip(appDir + kPath_SysRes);
    cfg.SetAppResFile(appDir + kPath_AppRes);
#endif

    if (!cfg.DoConfig(&theApp))
    {
        return -1;
    }

    CSysDataMgr * pSysDataMgr = new CSysDataMgr;
#ifdef __APPLE__
	SStringT strCfgDir = srcDir + _T("/Config");
#else
	SStringT strCfgDir = appDir + _T("/Config");
#endif
	SApplication::getSingleton().SetFilePrefix(strCfgDir);
	pSysDataMgr->LoadSysData(strCfgDir);


    theApp.InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id, sizeof(R.id) / sizeof(int));
    
    SAutoRefPtr<IAttrStorageFactory> attrStorageFactory(new SAttrStorageFactory(), FALSE);
    theApp.SetAttrStorageFactory(attrStorageFactory);

    Scintilla_RegisterClasses(hInstance);

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
    delete pSysDataMgr;
    Scintilla_ReleaseResources();

    OleUninitialize();
    return nRet;
}

#if !defined(_WIN32) || defined(__MINGW32__) 
int main(int argc,char **argv){
	return _tWinMain(GetModuleHandle(NULL),0,GetCommandLine(),SW_SHOWDEFAULT);
}
#endif//_WIN32