// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include "MainDlg.h"

#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")

#define INIT_R_DATA
#include "res/resource.h"
#ifdef _WIN32
#include "snapshot/SSnapshotCtrl.h"
#endif//_WIN32
#include <string>

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif
static std::tstring getSourceDir() {
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);
	//if run on remote linux machine, path should like /home/user/.vs/pathname/xxx/out/build/Linux-GCC-Debug/bin
	//convert this path to /home/user/.vs/pathname/xxx/src
	TCHAR* pdotvs = _tcsstr(szPath, _T(".vs"));
	if (pdotvs) {
		//run on remote linux machine
		TCHAR* out = _tcsstr(szPath, _T("out"));
		SASSERT(out);
		_tcscpy(out, _T("src"));
		return szPath;
	}
	else {
#ifdef _WIN32
		TCHAR* build = _tcsstr(szPath, _T("\\out"));
		if (!build)
		{
			GetCurrentDirectory(MAX_PATH, szPath);
			return szPath;
		}
#else
		TCHAR* build = _tcsstr(szPath, _T("/build/bin"));
		SASSERT(build);
#endif
		build[0] = 0;
		return szPath;
	}
}


static const TCHAR* kPath_SysRes = _T("/soui-sys-resource");
static const TCHAR* kPath_WxDemoRes = _T("/wxDemo/uires");

class SOUIEngine
{
private:
	SComMgr m_ComMgr;
	SApplication *m_theApp;
	SouiFactory m_souiFac;
public:
	SOUIEngine():m_theApp(NULL){}
		
	BOOL Init(HINSTANCE hInstance){
		SAutoRefPtr<IRenderFactory> pRenderFactory;
		BOOL bLoaded = TRUE;
		do{
			//使用SKIA渲染界面
#ifdef _WIN32
			bLoaded = m_ComMgr.CreateRender_Skia((IObjRef * *)& pRenderFactory);
#else
			bLoaded = m_ComMgr.CreateRender_GDI((IObjRef**)&pRenderFactory);
#endif//_WIN32
			SASSERT_MSGA(bLoaded, "load interface [render] failed!");
			if(!bLoaded) break;
			//设置图像解码引擎。默认为GDIP。基本主流图片都能解码。系统自带，无需其它库
			SAutoRefPtr<IImgDecoderFactory> pImgDecoderFactory;
			bLoaded = m_ComMgr.CreateImgDecoder((IObjRef * *)& pImgDecoderFactory);
			SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("imgdecoder"));
			if(!bLoaded) break;

			pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
			m_theApp = new SApplication(pRenderFactory, hInstance);	
		}while(false);
		return bLoaded;
	};
	BOOL LoadRes()
	{
		BOOL bLoaded = FALSE;
		do{
#ifdef _WIN32
			SAutoRefPtr<IResProvider> sysResProvider;
			HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
			if (!hModSysResource)
				break;
			sysResProvider.Attach(m_souiFac.CreateResProvider(RES_PE));
			sysResProvider->Init((WPARAM)hModSysResource, 0);
			m_theApp->LoadSystemNamedResource(sysResProvider);
			FreeLibrary(hModSysResource);

			SAutoRefPtr<IResProvider>   pResProvider;
			pResProvider.Attach(m_souiFac.CreateResProvider(RES_FILE));
			bLoaded = pResProvider->Init((LPARAM)_T("uires"), 0);
			if (!bLoaded) {
				pResProvider.Attach(m_souiFac.CreateResProvider(RES_PE));
				bLoaded = pResProvider->Init((WPARAM)m_theApp->GetModule(), 0);
			}
			SASSERT_FMT(bLoaded, _T("load app resource failed"));
			if (!bLoaded) break;
			m_theApp->AddResProvider(pResProvider);
#else
			SAutoRefPtr<IResProvider> sysResProvider;
			sysResProvider.Attach(m_souiFac.CreateResProvider(RES_FILE));
			std::string sysRes = getSourceDir() + kPath_SysRes;
			if (!sysResProvider->Init((LPARAM)sysRes.c_str(), 0))
				break;
			m_theApp->LoadSystemNamedResource(sysResProvider);
			//load use resource
			SAutoRefPtr<IResProvider> pResProvider(m_souiFac.CreateResProvider(RES_FILE), FALSE);
			std::string appRes = getSourceDir() + kPath_WxDemoRes;
			bLoaded = pResProvider->Init((LPARAM)appRes.c_str(), 0);
			if (!bLoaded) break;
			m_theApp->AddResProvider(pResProvider);
#endif//_WIN32
		}while(false);
		SASSERT(bLoaded);
		return bLoaded;
	}

	//加载多语言支持
	BOOL LoadMultiLangsModule()
	{
		BOOL bLoaded = FALSE;
		SAutoRefPtr<ITranslatorMgr> trans;
		bLoaded = m_ComMgr.CreateTranslator((IObjRef * *)& trans);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("translator"));
		if (trans)
		{//加载语言翻译包
			m_theApp->SetTranslator(trans);
			SXmlDoc xmlDoc;
			if (m_theApp->LoadXmlDocment(xmlDoc,  _T("translator:lang_cn")))
			{
				SAutoRefPtr<ITranslator> langCN;
				trans->CreateTranslator(&langCN);
				SXmlNode xmlLang = xmlDoc.root().child(L"language");
				langCN->Load(&xmlLang, 1);//1=LD_XML
				trans->InstallTranslator(langCN);
			}
		}
		return bLoaded;
	}
	//注册用户自定义皮肤和控件
	void Regitercustom()
	{
		#ifdef _WIN32
		m_theApp->RegisterWindowClass<SShellTray>();

		m_theApp->RegisterWindowClass<SSnapshotCtrl>();//
		#endif//_WIN32
	}

	~SOUIEngine()
	{
		if (m_theApp)
		{
			delete m_theApp;
			m_theApp = NULL;
		}
	}

	SApplication* GetApp()
	{
		return m_theApp;
	}
};

#ifdef _WIN32
#define PATH_SLASH '\\'
#else
#define PATH_SLASH '/'
#endif//_WIN32
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
	HRESULT hRes = OleInitialize(NULL);
	SASSERT(SUCCEEDED(hRes));
	int nRet = 0;
	{
		SOUIEngine souiEngine;
		if (souiEngine.Init(hInstance))
		{
			//获取AppPath
			TCHAR szFilePath[MAX_PATH + 1];
			GetModuleFileName(NULL, szFilePath, MAX_PATH);
			(_tcsrchr(szFilePath, _T(PATH_SLASH)))[1] = 0;
			SStringT sstrExePath = szFilePath;

			CGlobalUnits::instance()->m_sstrAppPath = sstrExePath;

			sstrExePath += SStringT(_T("emojis"))+ _T(PATH_SLASH);
			if (GetFileAttributes(sstrExePath) != INVALID_FILE_ATTRIBUTES)
			{
				CGlobalUnits::instance()->SetEmojiPath(sstrExePath);
			}
			else
			{
#ifdef _WIN32
				CGlobalUnits::instance()->SetEmojiPath(_T("D:\\work\\soui4.git\\wxDemo\\emojis\\"));
#else
				std::string strDir = getSourceDir();
				strDir += "/wxDemo/emojis/";
				CGlobalUnits::instance()->SetEmojiPath(strDir.c_str());
#endif//_WIN32
			}
			CGlobalUnits::instance()->OperateEmojis();

			//处理拼音搜索
            CGlobalUnits::instance()->OperatePinyinMap();

			//加载系统资源
			souiEngine.LoadRes();
			//注册用户自定义皮肤/控件/布局/其它自定义
			souiEngine.Regitercustom();
			//加载多语言翻译模块。
			souiEngine.LoadMultiLangsModule();
			CMainDlg dlgMain;
			dlgMain.Create(GetActiveWindow());
			dlgMain.SendMessage(WM_INITDIALOG);
			dlgMain.CenterWindow(dlgMain.m_hWnd);
			dlgMain.ShowWindow(SW_SHOWNORMAL);
			nRet = souiEngine.GetApp()->Run(dlgMain.m_hWnd);
		}
		else
		{
			MessageBox(0, _T("无法正常初使化SOUI"), _T("错误"), MB_OK | MB_ICONERROR);
		}
	}
	OleUninitialize();
	return nRet;
}

#ifndef _WIN32
int main(int argc, char ** argv){
	HINSTANCE hInst = GetModuleHandle(NULL);
	return _tWinMain(hInst,0,NULL,SW_SHOWNORMAL);
}
#endif//_WIN32