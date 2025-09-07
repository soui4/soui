﻿// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include <SouiFactory.h>
#include <helper/SMenuWndHook.h>
#include <helper/SAutoBuf.h>
#include <controls.extend/SFadeFrame.h>
#include <controls.extend/SRadioBox2.h>
#include <controls.extend/SVscrollbar.h>
#include <controls.extend/SSkinNewScrollBar.h>
#include <controls.extend/SChromeTabCtrl.h>
#ifdef _WIN32
#include <controls.extend/SIECtrl.h>
#endif
#include <controls.extend/SChatEdit.h>
#include <controls.extend/SScrollText.h>
#include <controls.extend/SCalendar2.h>
#include <controls.extend/SHexEdit.h>
#include <controls.extend/SRoundImage.h>
#include <controls.extend/SStaticGdip.h>
#include <controls.extend/SRoundWnd.h>
#include <controls.extend/SImageKnob.h>
#include <controls.extend/SMcListViewEx/SHeaderCtrlEx.h>
#include <controls.extend/SMcListViewEx/SMCListViewEx.h>

#ifdef _WIN32
#include "uianimation/UiAnimationWnd.h"
#endif
#include "appledock/SDesktopDock.h"
#include "SMatrixWindow.h"
#ifdef _WIN32
#include "SmileyCreateHook.h"
#endif
#include "clock/sclock.h"
#include "FpsWnd.h"

//<--定一个tag="demo"的slog输出
#define kLogTag "demo"
//-->

#define NANOSVG_IMPLEMENTATION
#include <nanosvg.h>
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvgrast.h>

#include "MainDlg.h"


#define	RESTYPE_FILE  0 //从文件中加载资源，加载失败再从PE加载
#define	RESTYPE_PE 1 //从PE资源中加载UI资源
#define	RESTYPE_ZIP 2 //从zip包中加载资源
#define RESTYPE_7Z 3//从7zip包中加载资源

#ifdef _DEBUG
#define RES_TYPE RESTYPE_FILE      //从文件中加载资源，加载失败再从PE加载
#else
#define RES_TYPE RESTYPE_PE		//从PE资源中加载UI资源
#endif
//#define RES_TYPE RESTYPE_ZIP   //从zip包中加载资源
//#define RES_TYPE RESTYPE_7Z   //从7zip包中加载资源

#include <resprovider-7zip/zip7resprovider-param.h>
#include <resprovider-zip/zipresprovider-param.h>

#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")

#include <controls.extend/smiley/SSmileyCtrl.h>
#include "skin/SSkinLoader.h"
#include "trayicon/SShellTray.h"
#include "qrcode/SQrCtrl.h"

#include <interface/STaskLoop-i.h>
#include <helper/SFunctor.hpp>
#include <commgr2.h>

#include <string>
#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

#define INIT_R_DATA
#include "res/resource.h"

static SStringT getSourceDir() {
#ifdef __APPLE__
	char szBunblePath[1024];
	GetAppleBundlePath(szBunblePath, sizeof(szBunblePath));
	return S_CA2T(szBunblePath)+_T("/Contents/Resources");
#else
	SStringA file(__FILE__);
    file = file.Left(file.ReverseFind(PATH_SLASH));
	return S_CA2T(file);
#endif
}

class SApplication2 : public SApplication
{
public:
	SApplication2(IRenderFactory* pRendFactory, HINSTANCE hInst) :SApplication(pRendFactory, hInst) {}

protected:
	STDMETHOD_(IBitmapS*, LoadImage)(THIS_ LPCTSTR pszType, LPCTSTR pszResName)
	{
		int nBufSize = GetRawBufferSize(pszType, pszResName);
		SAutoBuf buf(nBufSize);
		char * pBuf = buf;
		BOOL bLoad = GetRawBuffer(pszType, pszResName, pBuf, nBufSize);
		if (bLoad && nBufSize > 6)
		{
			if (_tcscmp(pszType, _T("svg")) == 0)
			{
				const unsigned char bom16[2] = { 0xff,0xfe };
				const unsigned char bom8[3] = { 0xef,0xbb,0xbf };
				SStringA strBuf;
				if (memcmp(pBuf, bom16, 2) == 0)
				{
					strBuf = S_CW2A(SStringW((WCHAR*)(pBuf + 2), (nBufSize - 2) / 2), CP_UTF8);
				}
				else if (memcmp(pBuf, bom8, 3) == 0)
				{
					strBuf = SStringA(pBuf + 3, nBufSize - 3);
				}
				else
				{
					strBuf = S_CA2A(SStringA(pBuf, nBufSize), CP_ACP, CP_UTF8);
				}
				if (strBuf.Left(4) == "<svg")
				{
					NSVGimage* image = nsvgParse((char*)strBuf.c_str(), "px", 96.0f);
					IBitmapS* Ret = NULL;
					if (image)
					{
						int w = (int)image->width;
						int h = (int)image->height;

						NSVGrasterizer* rast = nsvgCreateRasterizer();

						unsigned char* img = (unsigned char*)malloc(w * h * 4);
						nsvgRasterize(rast, image, 0, 0, 1, img, w, h, w * 4);
						GETRENDERFACTORY->CreateBitmap(&Ret);
						Ret->Init(w, h, img);
						free(img);

						nsvgDeleteRasterizer(rast);
						nsvgDelete(image);

					}
					return Ret;
				}
			}
		}
		return SResLoadFromMemory::LoadImage(pBuf, nBufSize);
	}
};

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
	//必须要调用OleInitialize来初始化运行环境
	HRESULT hRes = OleInitialize(NULL);
	SASSERT(SUCCEEDED(hRes));
	int nRet = 0;

	//将程序的运行路径修改到demo所在的目录
	SStringT appDir = getSourceDir();
	#ifdef __linux__
		AddFontResource((appDir+_T("/../../simsun.ttc")).c_str());
	#elif defined(__APPLE__)
		AddFontResource((appDir+_T("/fonts/simsun.ttc")).c_str());
	#endif
	SouiFactory souiFac;
    SComMgr2 *pComMgr = new SComMgr2();
    {
		int nType =  MessageBox(GetActiveWindow(), _T("选择渲染类型：\n[yes]: Skia\n[no]:GDI\n[cancel]:Quit"), _T("select a render"), MB_ICONQUESTION | MB_YESNOCANCEL);
		if (nType == IDCANCEL)
		{
			nRet = -1;
			goto exit;
		}

		//定义一个唯一的SApplication对象，SApplication管理整个应用程序的资源
		SApplication* theApp = new SApplication2(NULL, hInstance);


		//定义一组类SOUI系统中使用的类COM组件
		//SAutoRefPtr是一个SOUI系统中使用的智能指针类
		SAutoRefPtr<IImgDecoderFactory> pImgDecoderFactory; //图片解码器，由imagedecoder-wid.dll模块提供
		SAutoRefPtr<IRenderFactory> pRenderFactory;         //UI渲染模块，由render-gdi.dll或者render-skia.dll提供
		SAutoRefPtr<ITranslatorMgr> trans;                  //多语言翻译模块，由translator.dll提供
		SAutoRefPtr<IScriptFactory> pScriptLua;              //lua脚本模块，由scriptmodule-lua.dll提供
		SAutoRefPtr<ILogMgr>  pLogMgr;                //log4z对象

		BOOL bLoaded = FALSE;
		//从各组件中显式创建上述组件对象
		if (nType == IDYES)
			bLoaded = pComMgr->CreateRender_Skia((IObjRef**)&pRenderFactory);
		else
			bLoaded = pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), nType == IDYES ? _T("render_skia") : _T("render_gdi"));
		bLoaded = pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("imgdecoder"));
		bLoaded = pComMgr->CreateTranslator((IObjRef**)&trans);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("translator"));

		if (pComMgr->CreateLog4z((IObjRef**)&pLogMgr) && pLogMgr)
		{
			//uncomment next line to disable log mgr to output debug string.
			pLogMgr->setLoggerName("demo");

			//uncomment next line to record info level log.
			pLogMgr->setLoggerLevel(LOG_LEVEL_INFO);
			pLogMgr->start();
		}

		//为渲染模块设置它需要引用的图片解码模块
		pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
		theApp->SetRenderFactory(pRenderFactory);

		theApp->SetLogManager(pLogMgr);
		//控件注册要放到AddResProvider前： 2016年3月8日

		//向SApplication系统中注册由外部扩展的控件及SkinObj类
		SWkeLoader wkeLoader;
		if (wkeLoader.Init(_T("wke.dll")))
		{
			theApp->RegisterWindowClass<SWkeWebkit>();//注册WKE浏览器
		}
		theApp->RegisterWindowClass<SMatrixWindow>();//
		theApp->RegisterWindowClass<S3dWindow>();//
		theApp->RegisterWindowClass<SFreeMoveWindow>();//
		theApp->RegisterWindowClass<SClock>();//
        theApp->RegisterWindowClass<SGifPlayer>();     // theApp中增加方法：RegisterWindowClass,替换RegisterWndFactory(TplSWindowFactory<SGifPlayer>())
        theApp->RegisterSkinClass<SSkinAni>();         // 注册SkinGif
        theApp->RegisterSkinClass<SSkinVScrollbar>();  // 注册纵向滚动条皮肤
		theApp->RegisterSkinClass<SSkinNewScrollbar>();//注册纵向滚动条皮肤
		theApp->RegisterSkinClass<SDemoSkin>();

		theApp->RegisterWindowClass<SMCListViewEx>();
		theApp->RegisterWindowClass<SHeaderCtrlEx>();
		theApp->RegisterWindowClass<SIPAddressCtrl>();//注册IP控件
		theApp->RegisterWindowClass<SPropertyGrid>();//注册属性表控件
		theApp->RegisterWindowClass<SChromeTabCtrl>();//注册ChromeTabCtrl
		theApp->RegisterWindowClass<SChatEdit>();//注册ChatEdit
		theApp->RegisterWindowClass<SScrollText>();//注册SScrollText
		theApp->RegisterWindowClass<SDesktopDock>();//注册SDesktopDock
		theApp->RegisterWindowClass<SImageMaskWnd>();//注册SImageMaskWnd
		theApp->RegisterWindowClass<SRatingBar>();//注册SRatingBar
		theApp->RegisterWindowClass<SInterpolatorView>();
		theApp->RegisterWindowClass<SPathView>();
		theApp->RegisterWindowClass<SQrCtrl>();
		theApp->RegisterWindowClass<SProgressRing>();
		theApp->RegisterWindowClass<SCheckBox2>();
		theApp->RegisterWindowClass<SAniWindow>();
		theApp->RegisterWindowClass<SGroupList>();
		theApp->RegisterWindowClass<SRoundImage>();
		theApp->RegisterWindowClass<SRoundWnd>();
		theApp->RegisterWindowClass<SHexEdit>();
        theApp->RegisterWindowClass<SImageKnob>();
		theApp->RegisterWindowClass<SFadeFrame>();//注册渐显隐动画控件
		theApp->RegisterWindowClass<SRadioBox2>();//注册RadioBox2
		theApp->RegisterWindowClass<SCalendar2>();//注册SCalendar2

		theApp->RegisterWindowClass<SShellTray>();
		theApp->RegisterWindowClass<FpsWnd>();
		
#ifdef _WIN32
		theApp->RegisterWindowClass<SIECtrl>();//注册IECtrl
		theApp->RegisterWindowClass<SStaticGdip>();
		if (SUCCEEDED(CUiAnimation::Init()))
		{
			theApp->RegisterWindowClass<SUiAnimationWnd>();//注册动画控件
		}
#endif

		//如果需要在代码中使用R::id::namedid这种方式来使用控件必须要这一行代码：2016年2月2日，R::id,R.name是由uiresbuilder 增加-h .\res\resource.h 这2个参数后生成的。
		theApp->InitXmlNamedID((const LPCWSTR*)&R.name, (const int*)&R.id, sizeof(R.id) / sizeof(int));

        theApp->SetAppDir(appDir.c_str());
        SetCurrentDirectory(appDir.c_str());

#ifdef _WIN32
		//加载系统资源
#if (defined(LIB_CORE) && defined(LIB_SOUI_COM))
		HMODULE hSysResource = hInstance;
#else
		HMODULE hSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
#endif
		if (hSysResource)
		{
			SAutoRefPtr<IResProvider> sysResProvider;
			sysResProvider.Attach(souiFac.CreateResProvider(RES_PE));
			sysResProvider->Init((WPARAM)hSysResource, 0);
			theApp->LoadSystemNamedResource(sysResProvider);

#if !(defined(LIB_CORE) && defined(LIB_SOUI_COM))
			FreeLibrary(hSysResource);
#endif
		}
#else
#ifdef __APPLE__
		static const TCHAR* kPath_SysRes = _T("/soui-sys-resource");
#else
		static const TCHAR* kPath_SysRes = _T("/../../soui-sys-resource");
#endif//__APPLE__
		static const TCHAR* kPath_DemoRes = _T("/uires");

        SAutoRefPtr<IResProvider> sysResProvider;
		sysResProvider.Attach(souiFac.CreateResProvider(RES_FILE));
		SStringT sysRes = appDir + kPath_SysRes;
		sysResProvider->Init((WPARAM)sysRes.c_str(), 0);
		theApp->LoadSystemNamedResource(sysResProvider);
#endif
		//定义一人个资源提供对象,SOUI系统中实现了3种资源加载方式，分别是从文件加载，从EXE的资源加载及从ZIP压缩包加载
		SAutoRefPtr<IResProvider>   pResProvider;

#if defined(_WIN32) && (RES_TYPE==RESTYPE_PE)//从EXE资源加载
		pResProvider.Attach(souiFac.CreateResProvider(RES_PE));
		pResProvider->Init((WPARAM)hInstance, 0);
#elif (RES_TYPE==RESTYPE_ZIP)//从ZIP包加载
		bLoaded = pComMgr->CreateResProvider_ZIP((IObjRef**)&pResProvider);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));

		ZIPRES_PARAM param;
		ZipFile(&param,pRenderFactory, _T("uires.zip"), "souizip");
		bLoaded = pResProvider->Init((WPARAM)&param, 0);
		SASSERT(bLoaded);
#elif (RES_TYPE==RESTYPE_7Z)//从ZIP包加载
		bLoaded = pComMgr->CreateResProvider_7ZIP((IObjRef**)&pResProvider);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));

		ZIP7RES_PARAM param;
		Zip7File(&param,pRenderFactory, _T("uires.zip"), "souizip");
		bLoaded = pResProvider->Init((WPARAM)&param, 0);
		SASSERT(bLoaded);
#else //#if (RES_TYPE == RESTYPE_FILE)//从文件加载
		pResProvider.Attach(souiFac.CreateResProvider(RES_FILE));
		if (!pResProvider->Init((LPARAM)_T("uires"), 0))
		{
			pResProvider.Attach(souiFac.CreateResProvider(RES_PE));
			if (!pResProvider->Init((WPARAM)hInstance, 0))
			{
				SASSERT(0);
				delete theApp;
				nRet = 1;
				goto exit;
			}
		}
#endif//RES_TYPE

		pRenderFactory = NULL;
		//将创建的IResProvider交给SApplication对象
		theApp->AddResProvider(pResProvider);

		SSkinLoader* SkinLoader = new SSkinLoader(theApp);
		SkinLoader->LoadDefSkin();
		if (trans)
		{//加载语言翻译包
			theApp->SetTranslator(trans);
			SXmlDoc xmlLang;
			if (theApp->LoadXmlDocment(xmlLang, _T("translator:lang_cn")))
			{
				SAutoRefPtr<ITranslator> langCN;
				trans->CreateTranslator(&langCN);
				SXmlNode xmlNode = xmlLang.root().child(L"language");
				langCN->Load(&xmlNode, 1);//1=LD_XML
				trans->InstallTranslator(langCN);
				SStringW strFont;
				langCN->getFontInfo(&strFont);
				if (!strFont.IsEmpty())
				{//从翻译文件中获取并设置程序的字体信息
					theApp->SetDefaultFontInfo(strFont);
				}
			}
		}
#if (defined(DLL_CORE) || (defined(LIB_CORE) && defined(LIB_SOUI_COM)))
		//加载LUA脚本模块，注意，脚本模块只有在SOUI内核是以DLL方式编译时才能使用。
		bLoaded = pComMgr->CreateScrpit_Lua((IObjRef**)&pScriptLua);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("scirpt_lua"));
		theApp->SetScriptFactory(pScriptLua);
#endif//DLL_CORE


#ifdef _WIN32
		//采用hook绘制菜单的边框
		SMenuWndHook::InstallHook(hInstance, L"_skin.sys.menu.border");
#endif

		//演示R.color.xxx,R.string.xxx在代码中的使用。
		COLORREF crRed = GETCOLOR(R.color.red);
		SStringW strTitle = GETSTRING(R.string.title);

		theApp->EnableNotifyCenter(TRUE);
		{
#ifdef _WIN32
			SmileyCreateHook  smileyHook;
#endif
			//设置提示窗口布局
			STipWnd::SetLayout(_T("layout:dlg_tip"));

			//创建并显示使用SOUI布局应用程序窗口,为了保存窗口对象的析构先于其它对象，把它们缩进一层。
			CMainDlg dlgMain;
			dlgMain.Create(GetActiveWindow(), 0, 0, 888, 650);

			dlgMain.GetNative()->SendMessage(WM_INITDIALOG);
			dlgMain.CenterWindow();
			dlgMain.ShowWindow(SW_SHOWNORMAL);

			nRet = theApp->Run(dlgMain.m_hWnd);
		}

		//应用程序退出
		delete SkinLoader;
		delete theApp;
		if (pLogMgr)
		{
			pLogMgr->stop();
		}

#ifdef _WIN32
		//卸载菜单边框绘制hook
		SMenuWndHook::UnInstallHook();
		CUiAnimation::Free();
#endif
	}
exit:
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

