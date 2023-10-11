#include "stdafx.h"
#include "AppUI.h"
#include "MainDlg.h"
#pragma comment(lib, "shlwapi.lib")

//从PE文件加载，注意从文件加载路径位置
#define RES_TYPE 0
//#define SYSRES_TYPE 0
// #define RES_TYPE 0   //PE
// #define RES_TYPE 1   //ZIP
// #define RES_TYPE 2   //7z
// #define RES_TYPE 2   //文件
//去掉多项支持，以免代码显得混乱
#if (RES_TYPE==1)
#include "resprovider-zip\zipresprovider-param.h"
#else 
#if (RES_TYPE==2)
#include "resprovider-7zip\zip7resprovider-param.h"
#endif
#endif
#ifdef _DEBUG
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#endif

[!if CHECKBOX_USE_LUA]
#ifdef _DEBUG
#pragma comment(lib,"lua-54d")
#pragma comment(lib,"scriptmodule-luad")
#else
#pragma comment(lib,"lua-54")
#pragma comment(lib,"scriptmodule-lua")
#endif
[!endif]

#define INIT_R_DATA
#include "res/resource.h"

HMODULE UI_Engine::m_hInstance = NULL;
HWND UI_Engine::m_hParent = NULL;
//////////////////////////////////////////////////////////////////////////
UI_Engine g_UIEngine;

//////////////////////////////////////////////////////////////////////////
UI_Engine::UI_Engine()
{
	
}

UI_Engine::~UI_Engine()
{
	
}

DWORD WINAPI UI_Engine::RunUI(LPVOID lpParam)
{
	HRESULT hRes = OleInitialize(NULL);
	SASSERT(SUCCEEDED(hRes));

	int nRet = 0;

	SComMgr *pComMgr = new SComMgr;
	SouiFactory souiFac;
	TCHAR szCurrentDir[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szCurrentDir, sizeof(szCurrentDir));

	LPTSTR lpInsertPos = _tcsrchr(szCurrentDir, _T('\\'));
#ifdef _DEBUG
	_tcscpy(lpInsertPos + 1, _T("..\\[!output PROJECT_NAME]"));
#else
	_tcscpy(lpInsertPos + 1, _T("\0"));
#endif
	SetCurrentDirectory(szCurrentDir);
	{
		BOOL bLoaded = FALSE;
		CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
		CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
[!if RADIO_RANDER_GDI]
		bLoaded = pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
[!else]
		bLoaded = pComMgr->CreateRender_Skia((IObjRef**)&pRenderFactory);
[!endif]
		SASSERT_FMT(bLoaded, _T("load interface [render] failed!"));
		bLoaded = pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("imgdecoder"));

		pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
		SApplication *theApp = new SApplication(pRenderFactory, m_hInstance);

#if (RES_TYPE != 0)
		//选择了仅在Release版本打包资源则系统资源在DEBUG下始终使用DLL加载
		{
			HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
			if (hModSysResource)
			{
				CAutoRefPtr<IResProvider> sysResProvider;
				sysResProvider.Attach(souiFac.CreateResProvider(RES_PE));
				sysResProvider->Init((WPARAM)hModSysResource, 0);
				theApp->LoadSystemNamedResource(sysResProvider);
				FreeLibrary(hModSysResource);
			}
			else
			{
				SASSERT(0);
	}
}
#else
		//钩选了复制系统资源选项
		{
			CAutoRefPtr<IResProvider> pSysResProvider;
			pSysResProvider.Attach(souiFac.CreateResProvider(RES_PE));
			bLoaded = pSysResProvider->Init((WPARAM)m_hInstance, 0);
			SASSERT(bLoaded);
			theApp->LoadSystemNamedResource(pSysResProvider);
		}
#endif

		CAutoRefPtr<IResProvider>   pResProvider;

#if (RES_TYPE != 0)		
		//选择了仅在Release版本打包资源则在DEBUG下始终使用文件加载
		{
			pResProvider.Attach(souiFac.CreateResProvider(RES_FILE));
			bLoaded = pResProvider->Init((LPARAM)_T("uires"), 0);
			SASSERT(bLoaded);
		}
#else
		{
[!if ResLoaderType == 0]
			pResProvider.Attach(souiFac.CreateResProvider(RES_PE));
			bLoaded = pResProvider->Init((WPARAM)m_hInstance, 0);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 1]
			bLoaded = pComMgr->CreateResProvider_ZIP((IObjRef**)&pResProvider);
			SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));
			ZIPRES_PARAM param;
			param.ZipFile(pRenderFactory, _T("uires.zip"), "[!output ZIP_PSW]");
			bLoaded = pResProvider->Init((WPARAM)&param, 0);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 2]
			bLoaded = pComMgr->CreateResProvider_7ZIP((IObjRef**)&pResProvider);
			SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));
			ZIP7RES_PARAM param;
			param.ZipFile(pRenderFactory, _T("uires.7z"), "[!output ZIP_PSW]");
			bLoaded = pResProvider->Init((WPARAM)&param, 0);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 3]
			pResProvider.Attach(souiFac.CreateResProvider(RES_FILE));
			bLoaded = pResProvider->Init((LPARAM)_T("uires"), 0);
			SASSERT(bLoaded);
[!endif]
		}
#endif

			//如果需要在代码中使用R::id::namedid这种方式来使用控件必须要这一行代码：2016年2月2日，R::id,R.name是由uiresbuilder 增加-h .\res\resource.h 这2个参数后生成的。
			theApp->InitXmlNamedID((const LPCWSTR*)&R.name,(const int*)&R.id,sizeof(R.id)/sizeof(int));
			theApp->AddResProvider(pResProvider);
		
[!if CHECKBOX_USE_LUA]
		//加载LUA脚本模块。
#if (defined(DLL_CORE) || defined(LIB_ALL)) && !defined(_WIN64)
		//加载LUA脚本模块，注意，脚本模块只有在SOUI内核是以DLL方式编译时才能使用。
		CAutoRefPtr<SOUI::IScriptFactory> pScriptLuaFactory;
		bLoaded = pComMgr->CreateScrpit_Lua((IObjRef**)&pScriptLuaFactory);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("scirpt_lua"));
		theApp->SetScriptFactory(pScriptLuaFactory);
#endif//DLL_CORE
[!endif]

[!if CHECKBOX_TRANSLATOR_SUPPORT]
		//加载多语言翻译模块。
		CAutoRefPtr<ITranslatorMgr> trans;
		bLoaded = pComMgr->CreateTranslator((IObjRef**)&trans);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("translator"));
		if (trans)
		{//加载语言翻译包
			theApp->SetTranslator(trans);
			SXmlDoc xmlLang;
			if (theApp->LoadXmlDocment(xmlLang, _T("lang_cn"), _T("translator")))
			{
				CAutoRefPtr<ITranslator> langCN;
				trans->CreateTranslator(&langCN);
				langCN->Load(&xmlLang.root().child(L"language"), 1);//1=LD_XML
				trans->InstallTranslator(langCN);
			}
		}
[!endif]

		// BLOCK: Run application
		{
			CMainDlg dlgMain;
			dlgMain.Create(m_hParent);
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

int UI_Engine::Init_UI(HMODULE hInstance, HWND parent)
{
	m_hInstance = hInstance;
	m_hParent = parent;

	DWORD dwThreadId, dwThrdParam = 1;
	HANDLE hThread = CreateThread(NULL, 0, RunUI, &dwThrdParam, 0, &dwThreadId);
	CloseHandle(hThread);

	return 1;
}
