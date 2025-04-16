// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include "MainDlg.h"
#include "SCaptureButton.h"
#include "STabCtrlEx.h"
#include <SouiFactory.h>
#include <commgr2.h>
#pragma comment(lib,"shlwapi.lib")

//从PE文件加载，注意从文件加载路径位置
#define RES_TYPE 1

#define INIT_R_DATA
#include "res/resource.h"

#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")

static SStringT getSourceDir()
{
    SStringA file(__FILE__);
    file = file.Left(file.ReverseFind(PATH_SLASH));
    return S_CA2T(file);
}

static const TCHAR *kPath_SysRes = _T("/../../soui-sys-resource");
static const TCHAR *kPath_UiRes = _T("/uires");


BOOL LoadRes(SouiFactory &souiFac, SApplication *theApp)
{
    BOOL bLoaded = FALSE;
    do
    {
#ifdef _WIN32
        SAutoRefPtr<IResProvider> sysResProvider;
        HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
        if (!hModSysResource)
            break;
        sysResProvider.Attach(souiFac.CreateResProvider(RES_PE));
        sysResProvider->Init((WPARAM)hModSysResource, 0);
        theApp->LoadSystemNamedResource(sysResProvider);
        FreeLibrary(hModSysResource);

        SAutoRefPtr<IResProvider> pResProvider;
        pResProvider.Attach(souiFac.CreateResProvider(RES_FILE));
        SStringT appRes = getSourceDir() + kPath_UiRes;
        bLoaded = pResProvider->Init((LPARAM)appRes.c_str(), 0);
        if (!bLoaded)
        {
            pResProvider.Attach(souiFac.CreateResProvider(RES_PE));
            bLoaded = pResProvider->Init((WPARAM)theApp->GetModule(), 0);
        }
        SASSERT_FMT(bLoaded, _T("load app resource failed"));
        if (!bLoaded)
            break;
        theApp->AddResProvider(pResProvider);
#else
        SAutoRefPtr<IResProvider> sysResProvider;
        sysResProvider.Attach(souiFac.CreateResProvider(RES_FILE));
        SStringT sysRes = getSourceDir() + kPath_SysRes;
        if (!sysResProvider->Init((LPARAM)sysRes.c_str(), 0))
            break;
        theApp->LoadSystemNamedResource(sysResProvider);
        // load use resource
        SAutoRefPtr<IResProvider> pResProvider(souiFac.CreateResProvider(RES_FILE), FALSE);
        SStringT appRes = getSourceDir() + kPath_UiRes;
        bLoaded = pResProvider->Init((LPARAM)appRes.c_str(), 0);
        if (!bLoaded)
            break;
        theApp->AddResProvider(pResProvider);
#endif //_WIN32
    } while (false);
    SASSERT(bLoaded);
    return bLoaded;
}


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));
	SouiFactory souiFac;
    int nRet = 0;
    
    SComMgr2 *pComMgr = new SComMgr2();

    {
        BOOL bLoaded=FALSE;
        SAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
        SAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
        bLoaded = pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
        SASSERT_FMT(bLoaded,_T("load interface [render] failed!"));
        bLoaded=pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
        SASSERT_FMT(bLoaded,_T("load interface [%s] failed!"),_T("imgdecoder"));

        pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
        SApplication *theApp = new SApplication(pRenderFactory, hInstance);
        
        theApp->RegisterWindowClass<SEdit2>();
        theApp->RegisterWindowClass<SImgCanvas>();
        theApp->RegisterWindowClass<SFolderTreeList>();
		theApp->RegisterWindowClass<SCaptureButton>();
		theApp->RegisterWindowClass<STabCtrlEx>();

        if(LoadRes(souiFac, theApp))
        {
            CMainDlg dlgMain;
            dlgMain.Create(GetActiveWindow());
            dlgMain.SendMessage(WM_INITDIALOG);
            dlgMain.CenterWindow(dlgMain.m_hWnd);
            dlgMain.ShowWindow(SW_SHOWNORMAL);
            nRet = theApp->Run(dlgMain.m_hWnd);
        }

        delete theApp;
    }
    
    delete pComMgr;
    
    OleUninitialize();
    return nRet;
}


#ifndef _WIN32
int main(int argc, char **argv)
{
    HINSTANCE hInst = GetModuleHandle(NULL);
    return _tWinMain(hInst, 0, NULL, SW_SHOWNORMAL);
}
#endif //_WIN32