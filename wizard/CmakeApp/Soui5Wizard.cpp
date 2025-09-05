// dui-demo.cpp : main source file
//

#include "MainDlg.h"
#include "stdafx.h"
#include <resprovider-zip/zipresprovider-param.h>
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")

#define INIT_R_DATA
#include "res/resource.h"

using namespace SNS;

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

class SOUIEngine {
private:
  SComMgr m_ComMgr;
  SApplication *m_theApp;
  SouiFactory m_souiFac;

public:
  SOUIEngine() : m_theApp(NULL) {}

  BOOL Init(HINSTANCE hInstance) {
    SAutoRefPtr<IRenderFactory> pRenderFactory;
    BOOL bLoaded = TRUE;
    do {
      //使用SKIA渲染界面
      bLoaded = m_ComMgr.CreateRender_Skia((IObjRef **)&pRenderFactory);
      SASSERT_FMT(bLoaded, _T("load interface [render] failed!"));
      if (!bLoaded)
        break;
      //设置图像解码引擎。默认为GDIP。基本主流图片都能解码。系统自带，无需其它库
      SAutoRefPtr<IImgDecoderFactory> pImgDecoderFactory;
      bLoaded = m_ComMgr.CreateImgDecoder((IObjRef **)&pImgDecoderFactory);
      SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("imgdecoder"));
      if (!bLoaded)
        break;

      pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
      m_theApp = new SApplication(pRenderFactory, hInstance);
    } while (false);
    return bLoaded;
  };
  //加载系统资源
  BOOL LoadSystemRes() {
    BOOL bLoaded = TRUE;
	#ifdef _WIN32
    HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
    if (hModSysResource) {
	  SAutoRefPtr<IResProvider> sysResProvider;
      sysResProvider.Attach(m_souiFac.CreateResProvider(RES_PE));
      sysResProvider->Init((WPARAM)hModSysResource, 0);
      m_theApp->LoadSystemNamedResource(sysResProvider);
      FreeLibrary(hModSysResource);
    } else {
      bLoaded = FALSE;
    }
	#else
	SAutoRefPtr<IResProvider> sysResProvider;
	bLoaded = m_ComMgr.CreateResProvider_ZIP((IObjRef**)&sysResProvider);
	SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));
	ZIPRES_PARAM param;
	ZipFile(&param,m_theApp->GetRenderFactory(), _T("data/soui-sys-resource.zip"), "");
	bLoaded = sysResProvider->Init((WPARAM)&param, 0);
    m_theApp->LoadSystemNamedResource(sysResProvider);
	#endif
    SASSERT(bLoaded);
    return bLoaded;
  }
  //加载用户资源
  BOOL LoadUserRes() {
    SAutoRefPtr<IResProvider> pResProvider;
    BOOL bLoaded = FALSE;
    do {
#if !defined(_WIN32) || defined(_DEBUG)
      //选择了仅在Release版本打包资源则在DEBUG下始终使用文件加载
      {
        pResProvider.Attach(m_souiFac.CreateResProvider(RES_FILE));
        bLoaded = pResProvider->Init((LPARAM) _T("data/uires"), 0);
        SASSERT(bLoaded);
        if (!bLoaded)
          break;
      }
#else
      {
        pResProvider.Attach(m_souiFac.CreateResProvider(RES_PE));
        bLoaded = pResProvider->Init((WPARAM)m_theApp->GetModule(), 0);
        SASSERT(bLoaded);
        if (!bLoaded)
          break;
      }
#endif
      //如果需要在代码中使用R::id::namedid这种方式来使用控件必须要这一行代码：2016年2月2日，R::id,R.name是由uiresbuilder
      //增加-h .\res\resource.h 这2个参数后生成的。
      m_theApp->InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id,
                               sizeof(R.id) / sizeof(int));
      m_theApp->AddResProvider(pResProvider);
    } while (false);
    return bLoaded;
  }
  //加载LUA支持
  BOOL LoadLUAModule() {
    BOOL bLoaded = FALSE;
    SAutoRefPtr<IScriptFactory> pScriptLuaFactory;
    bLoaded = m_ComMgr.CreateScrpit_Lua((IObjRef **)&pScriptLuaFactory);
    SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("scirpt_lua"));
    m_theApp->SetScriptFactory(pScriptLuaFactory);
    return bLoaded;
  }
  //加载多语言支持
  BOOL LoadMultiLangsModule() {
    BOOL bLoaded = FALSE;
    SAutoRefPtr<ITranslatorMgr> trans;
    bLoaded = m_ComMgr.CreateTranslator((IObjRef **)&trans);
    SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("translator"));
    if (trans) { //加载语言翻译包
      m_theApp->SetTranslator(trans);
      SXmlDoc xmlLang;
      if (m_theApp->LoadXmlDocment(xmlLang, _T("translator:lang_cn"))) {
        SAutoRefPtr<ITranslator> langCN;
        trans->CreateTranslator(&langCN);
        SXmlNode nodeLang = xmlLang.root().child(L"language");
        langCN->Load(&nodeLang, 1); // 1=LD_XML
        trans->InstallTranslator(langCN);
      }
    }
    return bLoaded;
  }
  //注册用户自定义皮肤和控件
  void Regitercustom() {}

  ~SOUIEngine() {
    if (m_theApp) {
      delete m_theApp;
      m_theApp = NULL;
    }
  }

  SApplication *GetApp() { return m_theApp; }
};

void SetDefaultDir() {
  SStringT strSrcDir = getSourceDir();
  SetCurrentDirectory(strSrcDir);
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
                     LPTSTR lpstrCmdLine, int /*nCmdShow*/) {
  HRESULT hRes = OleInitialize(NULL);
  SASSERT(SUCCEEDED(hRes));
  SetDefaultDir();
  #ifndef _WIN32
  AddFontResource("fonts/simsun.ttc");
  #endif//_WIN32
  int nRet = 0;
  {
    SOUIEngine souiEngine;
    if (souiEngine.Init(hInstance)) {
      //加载系统资源
      souiEngine.LoadSystemRes();
      //注册用户自定义皮肤/控件/布局/其它自定义
      souiEngine.Regitercustom();
      //加载用户资源
      souiEngine.LoadUserRes();
      CMainDlg dlgMain;
      dlgMain.Create(GetActiveWindow());
      dlgMain.SendMessage(WM_INITDIALOG);
      dlgMain.CenterWindow(dlgMain.m_hWnd);
      dlgMain.ShowWindow(SW_SHOWNORMAL);
      nRet = souiEngine.GetApp()->Run(dlgMain.m_hWnd);
    } else {
      MessageBox(NULL, _T("无法正常初使化SOUI"), _T("错误"),
                 MB_OK | MB_ICONERROR);
    }
  }
  OleUninitialize();
  return nRet;
}

#if !defined(_WIN32) || defined(__MINGW32__)
int main(int argc, char **argv) {
  HINSTANCE hInst = GetModuleHandle(NULL);
  return _tWinMain(hInst, 0, NULL, SW_SHOWNORMAL);
}
#endif //_WIN32