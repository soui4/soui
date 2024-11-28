#include <config.h>
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
#if defined(_DEBUG) && !defined(NO_DEBUG_SUFFIX)
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#endif

#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#include <windows.h>

#include <CommCtrl.h>
#include <Shlwapi.h>
#include <OleCtl.h>
#include <tchar.h>
#include <stdio.h>
#include <helper/SAssertFmt.h>
#include <utilities-def.h>
#include <utilities.h>


#include <core/SDefine.h>

#include <interface/SFactory-i.h>
#include <interface/SRender-i.h>
#include <interface/SWindow-i.h>
#include <interface/SAdapter-i.h>
#include <interface/SCtrl-i.h>

#include <event/SEvents.h>
#define INIT_R_DATA
#include "res/resource.h"

#include "ComLoader.h"
#include "TestLvAdapter.h"

ISouiFactory * s_souiFac =NULL;



typedef struct _TestCtrl{
	IWindow * base;
	wchar_t   szText[100];
}TestCtrl,*PTestCtrl;

PTestCtrl GetTestCtrl(IWindow *pWnd){
	return (PTestCtrl)pWnd->lpVtbl->GetUserData(pWnd);
}

void TestCtrl_OnPaint(PTestCtrl pCtrl,IRenderTarget *pRT)
{
	RECT rc;
	pCtrl->base->lpVtbl->GetClientRect(pCtrl->base,&rc);
#ifdef _UNICODE
	pRT->lpVtbl->DrawText(pRT,pCtrl->szText,-1,&rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
#else
	{
		char szBuf[200];
		WideCharToMultiByte(CP_ACP,0,pCtrl->szText,-1,szBuf,200,NULL,NULL);
		pRT->lpVtbl->DrawText(pRT,szBuf,-1,&rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
	}
#endif
}

BOOL TestCtrl_SwndProc(IWindow *pWnd,UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult){
	PTestCtrl pCtrl = GetTestCtrl(pWnd);
	if(uMsg == WM_PAINT){
		IRenderTarget *pRT = (IRenderTarget *)wParam;
		TestCtrl_OnPaint(pCtrl,pRT);
		lResult = 0;
		return TRUE;
	}else if(uMsg == WM_NCDESTROY)
	{
		free(pCtrl);
		return TRUE;
	}else if(uMsg == UM_GETDESIREDSIZE){
		SIZE *pRet = (SIZE*)lResult;
		pRet->cx = 200,pRet->cy = 20;
		return TRUE;
	}
	return FALSE;
} 

HRESULT TestCtrl_OnAttr(IObject *pObj, const IStringW * attrName,const IStringW* attrValue,BOOL bLoading){
	if(attrName->lpVtbl->CompareNoCase(attrName,L"text") == 0)
	{
		IWindow *pWnd = (IWindow*)pObj;
		PTestCtrl pCtrl = GetTestCtrl(pWnd);
		wcscpy(pCtrl->szText,attrValue->lpVtbl->c_str(attrValue));
		return S_OK;
	}
	return E_INVALIDARG;
}

IWindow * TestCtrl_New(const IApplication *pApp){
	PTestCtrl pCtrl = (PTestCtrl)malloc(sizeof(TestCtrl));
	DWORD dwOldPort=0;
	pCtrl->base = (IWindow*)pApp->lpVtbl->CreateObject(pApp,L"window",Window);
	pCtrl->base->lpVtbl->SetUserData(pCtrl->base,(ULONG_PTR)pCtrl);
	pCtrl->base->lpVtbl->SetSwndProc(pCtrl->base,TestCtrl_SwndProc);
	pCtrl->base->lpVtbl->SetAttrHandler(pCtrl->base,TestCtrl_OnAttr);
	wcscpy(pCtrl->szText,L"test ctrl");
	return pCtrl->base;
}

//debug时方便调试设置当前目录以便从文件加载资源
void SetDefaultDir()
{
	TCHAR szCurrentDir[MAX_PATH] = { 0 };
	LPTSTR lpInsertPos;
	GetModuleFileName(NULL, szCurrentDir, sizeof(szCurrentDir));

	lpInsertPos = _tcsrchr(szCurrentDir, _T('\\'));
#ifdef _DEBUG
	_tcscpy(szCurrentDir, _T("d:\\work\\soui4lib\\demo2"));
#else
	_tcscpy(lpInsertPos + 1, _T("\0"));
#endif
	SetCurrentDirectory(szCurrentDir);
}
BOOL OnBtnFlash(IEvtArgs *evt,void *Ctx)
{
	IHostWnd *pHostWnd = (IHostWnd*)Ctx;
	INativeWnd* pNativeWnd = pHostWnd->lpVtbl->GetNative(pHostWnd);
	HWND hwnd = pNativeWnd->lpVtbl->GetHwnd(pNativeWnd);
	FlashWindow(hwnd,TRUE);
	return TRUE;
}

BOOL OnBtnMenu(IEvtArgs *evt,void *Ctx){
	IMenu *pMenu = NULL;
	IWindow *pSender = (IWindow*)evt->lpVtbl->Sender(evt);
	IHostWnd *pHostWnd = (IHostWnd*)Ctx;
	INativeWnd* pNativeWnd = pHostWnd->lpVtbl->GetNative(pHostWnd);
	HWND hwnd = pNativeWnd->lpVtbl->GetHwnd(pNativeWnd);
	RECT rcBtn;
	int nCmd = 0;
	pSender->lpVtbl->GetWindowRect(pSender,&rcBtn);
	pNativeWnd->lpVtbl->ClientToScreen2(pNativeWnd,&rcBtn);

	pMenu = s_souiFac->lpVtbl->CreateMenu(s_souiFac,NULL);
	pMenu->lpVtbl->LoadMenu(pMenu,UIRES.SMENU.menu_test);
	nCmd = pMenu->lpVtbl->TrackPopupMenu(pMenu,TPM_RETURNCMD,rcBtn.left,rcBtn.bottom,hwnd,NULL,100);
	pMenu->lpVtbl->Release(pMenu);
	return TRUE;
}

BOOL OnBtnMenuEx(IEvtArgs *evt,void *Ctx){
	IMenuEx *pMenu = NULL;
	IWindow *pSender = (IWindow*)evt->lpVtbl->Sender(evt);
	IHostWnd *pHostWnd = (IHostWnd*)Ctx;
	INativeWnd* pNativeWnd = pHostWnd->lpVtbl->GetNative(pHostWnd);
	HWND hwnd = pNativeWnd->lpVtbl->GetHwnd(pNativeWnd);
	RECT rcBtn;
	int nCmd = 0;
	pSender->lpVtbl->GetWindowRect(pSender,&rcBtn);
	pNativeWnd->lpVtbl->ClientToScreen2(pNativeWnd,&rcBtn);

	pMenu = s_souiFac->lpVtbl->CreateMenuEx(s_souiFac);
	pMenu->lpVtbl->LoadMenu(pMenu,UIRES.SMENU.menuex_test);
	nCmd = pMenu->lpVtbl->TrackPopupMenu(pMenu,TPM_RETURNCMD,rcBtn.left,rcBtn.bottom,hwnd,0);
	pMenu->lpVtbl->Release(pMenu);
	return TRUE;
}


BOOL OnBtnEndDialog(IEvtArgs *evt,void *Ctx)
{
	IHostDialog *hostWnd = (IHostDialog *)Ctx;
	hostWnd->lpVtbl->EndDialog(hostWnd,IDOK);
	return TRUE;
}

BOOL OnHostMsg(const LPMSG pMsg,LRESULT *pRes,void *Ctx)
{
	if(pMsg->message != WM_INITDIALOG)
	{
		return FALSE;
	}
	{
		IEvtSlot *pSlot;
		IHostDialog *hostWnd = (IHostDialog *)Ctx;
		IWindow* root = hostWnd->lpVtbl->GetIRoot(hostWnd);
		IWindow* btnClose= root->lpVtbl->FindIChildByID(root,R.id.btn_close);
		pSlot = s_souiFac->lpVtbl->CreateFuncSlot(s_souiFac,OnBtnEndDialog,hostWnd);
		btnClose->lpVtbl->SubscribeEvent(btnClose,EVT_CMD,pSlot);
		pSlot->lpVtbl->Release(pSlot);
	}

	return TRUE;
}

BOOL OnBtnDialog(IEvtArgs *evt,void *Ctx)
{
	IHostDialog* hostDialog = s_souiFac->lpVtbl->CreateHostDialog(s_souiFac,UIRES.LAYOUT.DLG_TEST);
	INativeWnd* pNativeWnd = hostDialog->lpVtbl->GetNative(hostDialog);

	pNativeWnd->lpVtbl->SetMsgHandler(pNativeWnd,OnHostMsg,hostDialog);
	hostDialog->lpVtbl->DoModal(hostDialog,NULL);

	hostDialog->lpVtbl->Release(hostDialog);
	return TRUE;
}

BOOL OnBtnAdd(IEvtArgs *evt,void *Ctx){
	IHostWnd *pHostWnd = (IHostWnd*)Ctx;
	IWindow *pRoot = pHostWnd->lpVtbl->GetIRoot(pHostWnd);
	IWindow *pLvWnd=pRoot->lpVtbl->FindIChildByID(pRoot,R.id.lv_test);
	IListView *pLv=NULL;
	ILvAdapter *pAdapter =NULL;
	SIZE sz;
	pRoot->lpVtbl->GetDesiredSize(pRoot,&sz,-1,-1);
	pLvWnd->lpVtbl->QueryInterface(pLvWnd,&IID_IListView,(IObjRef**)&pLv);
	pAdapter= pLv->lpVtbl->GetAdapter(pLv);
	TestLvAdapter_Add(pAdapter);
	return TRUE;
}

BOOL OnBtnClean(IEvtArgs *evt,void *Ctx){
	IHostWnd *pHostWnd = (IHostWnd*)Ctx;
	IWindow *pRoot = pHostWnd->lpVtbl->GetIRoot(pHostWnd);
	IWindow *pLvWnd=pRoot->lpVtbl->FindIChildByID(pRoot,R.id.lv_test);
	IListView *pLv=NULL;
	ILvAdapter *pAdapter =NULL;
	pLvWnd->lpVtbl->QueryInterface(pLvWnd,&IID_IListView,(IObjRef**)&pLv);
	pAdapter= pLv->lpVtbl->GetAdapter(pLv);
	TestLvAdapter_Clean(pAdapter);
	return TRUE;
}

BOOL OnSliderPos(IEvtArgs *evt, void *Ctx)
{
	IHostWnd *pHostWnd = (IHostWnd*)Ctx;
	StEventSliderPos * data = (StEventSliderPos *)evt->lpVtbl->Data(evt);
	int nPos = data->nPos;
	IWindow *pTxt = pHostWnd->lpVtbl->FindIChildByID(pHostWnd,R.id.txt_pos);
	TCHAR szPos[50];
	_stprintf(szPos,_T("%d"),nPos);
	pTxt->lpVtbl->SetWindowText(pTxt,szPos);
	return TRUE;
}

BOOL LoadRenderFac(IRenderFactory ** ppRenderFac){
	BOOL bLoaded = TRUE;
	IImgDecoderFactory *pImgDecoderFactory;
	do{
		//使用GDI渲染界面
		bLoaded = LoadComObj(Render_Skia,(IObjRef * *)ppRenderFac);
		SASSERT(bLoaded);
		if(!bLoaded) break;
		//设置图像解码引擎。默认为GDIP。基本主流图片都能解码。系统自带，无需其它库
		bLoaded = LoadComObj(Decoder_Gdip,(IObjRef * *)&pImgDecoderFactory);
		SASSERT(bLoaded);
		if(!bLoaded) break;

		(*ppRenderFac)->lpVtbl->SetImgDecoderFactory(*ppRenderFac,pImgDecoderFactory);
		pImgDecoderFactory->lpVtbl->Release(pImgDecoderFactory);
	}while(FALSE);
	return bLoaded;
}

//加载系统资源
BOOL LoadSystemRes(IApplication *pApp,HINSTANCE hInstance, ISouiFactory *pSourFac)
{
#if (defined(LIB_CORE) && defined(LIB_SOUI_COM))
	HMODULE hModSysResource = hInstance;
#else
	HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
#endif
	if (hModSysResource)
	{
		IResProvider * sysResProvider = pSourFac->lpVtbl->CreateResProvider(pSourFac,RES_PE);
		sysResProvider->lpVtbl->Init(sysResProvider,(WPARAM)hModSysResource, 0);
		pApp->lpVtbl->LoadSystemNamedResource(pApp,sysResProvider);
		sysResProvider->lpVtbl->Release(sysResProvider);
		FreeLibrary(hModSysResource);
		return TRUE;
	}
	else
	{
		SASSERT(FALSE);
		return FALSE;
	}
}


//加载用户资源
BOOL LoadUserRes(IApplication *pApp,ISouiFactory *pSourFac)
{
	IResProvider*   pResProvider=NULL;
	BOOL bLoaded = FALSE;
	do{
#ifdef _DEBUG		
		//选择了仅在Release版本打包资源则在DEBUG下始终使用文件加载
		{
			pResProvider = pSourFac->lpVtbl->CreateResProvider(pSourFac,RES_FILE );
			bLoaded = pResProvider->lpVtbl->Init(pResProvider,(LPARAM)_T("uires"), 0);
			SASSERT(bLoaded);
			if(!bLoaded) break;
		}
#else
		{
			pResProvider = pSourFac->lpVtbl->CreateResProvider(pSourFac,RES_PE);
			bLoaded = pResProvider->lpVtbl->Init(pResProvider,(WPARAM)pApp->lpVtbl->GetModule(pApp), 0);
			SASSERT(bLoaded);
			if(!bLoaded) break;
		}
#endif
		pApp->lpVtbl->InitXmlNamedID(pApp,(const LPCWSTR*)&R.name, (const int*)&R.id,sizeof(R.id)/sizeof(int));
		{
			IResProviderMgr *pResMgr = pApp->lpVtbl->GetResProviderMgr(pApp);
			pResMgr->lpVtbl->AddResProvider(pResMgr,pResProvider,_T("uidef:xml_init"));
			pResProvider->lpVtbl->Release(pResProvider);
		}
	}while(FALSE);
	return bLoaded;
}

void SConnect(ISouiFactory *pFac,IHostWnd *pHostWnd,int wndId,FunCallback fun,int nEvtID){
	IEvtSlot *pSlot=NULL;
	IWindow* wnd = pHostWnd->lpVtbl->FindIChildByID(pHostWnd,wndId);
	pSlot = pFac->lpVtbl->CreateFuncSlot(pFac,fun,pHostWnd);
	wnd->lpVtbl->SubscribeEvent(wnd,nEvtID,pSlot);
	pSlot->lpVtbl->Release(pSlot);
}

IObject * DemoC_CreateObject(const IApplication *pApp,LPCWSTR pszName,SObjectType nType){
	if(wcscmp(pszName, L"testCtrl")==0){
		return (IObject*)TestCtrl_New(pApp);
	}
	return NULL;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpstrCmdLine, int nCmdShow)
{
	int nRet = 0;
	HRESULT hRes = OleInitialize(NULL);
	SASSERT(SUCCEEDED(hRes));
	{
		IApplication * theApp = NULL;
		IRenderFactory * pRenderFac = NULL;
		DWORD dwOldProt = 0;
		ISouiFactory * souiFac = CreateSouiFactory();

		s_souiFac = souiFac;

		SetDefaultDir();

		if(!LoadRenderFac(&pRenderFac))
			return -1;
		theApp=souiFac->lpVtbl->CreateApp(souiFac,pRenderFac,hInstance,_T("test"),FALSE);
		theApp->lpVtbl->SetCreateObjectCallback(theApp,DemoC_CreateObject);

		if(!LoadSystemRes(theApp, hInstance,souiFac))
			return -2;
		if(!LoadUserRes(theApp,souiFac))
			return -3;

		{
			//使用接口方式创建HostWnd。
			IHostWnd * hostWnd = souiFac->lpVtbl->CreateHostWnd(souiFac,UIRES.LAYOUT.XML_MAINWND);
			INativeWnd* nativeWnd = hostWnd->lpVtbl->GetNative(hostWnd);
			hostWnd->lpVtbl->Create(hostWnd,NULL,0,0,0,0);
			{
				IListView *pLv =NULL;
				IWindow* root = hostWnd->lpVtbl->GetIRoot(hostWnd);
				IWindow *pLvTest = root->lpVtbl->FindIChildByName(root,L"lv_test");
				if(S_OK==pLvTest->lpVtbl->QueryInterface(pLvTest,&IID_IListView,(IObjRef**)&pLv))
				{
					ILvAdapter *pAdapter=NULL;
					TestLvAdapter_Init(&pAdapter);
					pLv->lpVtbl->SetAdapter(pLv,pAdapter);
					pAdapter->lpVtbl->Release(pAdapter);

					pLv->lpVtbl->Release(pLv);
				}
			}
			nativeWnd->lpVtbl->ShowWindow(nativeWnd,SW_SHOWNORMAL);

			//使用事件订阅方式响应界面控件事件。
			{
				SConnect(souiFac,hostWnd,R.id.btn_flash,OnBtnFlash,EVT_CMD);
				SConnect(souiFac,hostWnd,R.id.btn_menu,OnBtnMenu,EVT_CMD);
				SConnect(souiFac,hostWnd,R.id.btn_menuex,OnBtnMenuEx,EVT_CMD);
				SConnect(souiFac,hostWnd,R.id.btn_dialog,OnBtnDialog,EVT_CMD);
				SConnect(souiFac,hostWnd,R.id.btn_add,OnBtnAdd,EVT_CMD);
				SConnect(souiFac,hostWnd,R.id.btn_clean,OnBtnClean,EVT_CMD);
				SConnect(souiFac,hostWnd,R.id.tst_slider,OnSliderPos,EVT_SLIDER_POS);
			}

			nRet = theApp->lpVtbl->Run(theApp, nativeWnd->lpVtbl->GetHwnd(nativeWnd));

			hostWnd->lpVtbl->Release(hostWnd);
		}
		theApp->lpVtbl->Release(theApp);
		souiFac->lpVtbl->Release(souiFac);
		
		s_souiFac = NULL;
	}
	OleUninitialize();
	return nRet;
}
