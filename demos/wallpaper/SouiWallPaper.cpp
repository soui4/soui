
#include "stdafx.h"
#include "SouiFactory.h"
#include "ui/MainDlg.h"

#define INIT_R_DATA
#include "res/resource.h"

#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
static const TCHAR *kPath_SysRes = _T("/../../soui-sys-resource");
static const TCHAR *kPath_DemoRes = _T("/uires");


SStringT getSourceDir() {
	SOUI::SStringA file(__FILE__);
	file = file.Left(file.ReverseFind(PATH_SLASH));
	return SOUI::S_CA2T(file);
}


BOOL LoadRes(SApplication *theApp, SouiFactory &souiFac)
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
        SStringT appRes = getSourceDir() + kPath_DemoRes;
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
        SStringT appRes = getSourceDir() + kPath_DemoRes;
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

	int nRet = 0;

	SComMgr *pComMgr = new SComMgr();
	{
		BOOL bLoaded = FALSE;
		SouiFactory souiFac;
		SAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
		SAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
		bLoaded = pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
		SASSERT_FMT(bLoaded, _T("load interface [render] failed!"));
		bLoaded = pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("imgdecoder"));

		pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
		SApplication *theApp = new SApplication(pRenderFactory, hInstance);
        if (!LoadRes(theApp, souiFac))
        {
            return FALSE;
        }

        theApp->InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id, sizeof(R.id) / sizeof(int));
		theApp->EnableNotifyCenter(TRUE);
		new CHttpDownloader();

		// BLOCK: Run application
		{
			CMainDlg dlgMain;
			dlgMain.Create(GetActiveWindow(),WS_POPUP,0ul);
			dlgMain.SendMessage(WM_INITDIALOG);
			dlgMain.CenterWindow(dlgMain.m_hWnd);
			dlgMain.ShowWindow(SW_SHOWNORMAL);
			nRet = theApp->Run(dlgMain.m_hWnd);
		}
		delete CHttpDownloader::getSingletonPtr();

		delete theApp;
	}

	delete pComMgr;

	OleUninitialize();
	return nRet;
}

#ifndef _WIN32
int main(int argc, char ** argv){
	HINSTANCE hInst = GetModuleHandle(NULL);
	return _tWinMain(hInst,0,NULL,SW_SHOWNORMAL);
}
#endif//_WIN32