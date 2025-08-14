#include "stdafx.h"
#include "ClientDlg.h"

#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")

#define INIT_R_DATA
#include "res/resource.h"

static SStringT getSourceDir()
{
    SStringA file(__FILE__);
    file = file.Left(file.ReverseFind(PATH_SLASH));
    file = file.Left(file.ReverseFind(PATH_SLASH));
    return S_CA2T(file);
}

static const TCHAR *kPath_SysRes = _T("/../../soui-sys-resource");
static const TCHAR *kPath_UiRes = _T("/uires");

class SOUIEngine {
  private:
    SComMgr2 m_ComMgr;
    SApplication *m_theApp;
    SouiFactory m_souiFac;

  public:
    SOUIEngine()
        : m_theApp(NULL)
    {
    }

    BOOL Init(HINSTANCE hInstance)
    {
        SAutoRefPtr<IRenderFactory> pRenderFactory;
        BOOL bLoaded = TRUE;
        do
        {
            // 使用SKIA渲染界面
            bLoaded = m_ComMgr.CreateRender_Skia((IObjRef **)&pRenderFactory);
            SASSERT_MSGA(bLoaded, "load interface [render] failed!");
            if (!bLoaded)
                break;
            // 设置图像解码引擎。默认为GDIP。基本主流图片都能解码。系统自带，无需其它库
            SAutoRefPtr<IImgDecoderFactory> pImgDecoderFactory;
            bLoaded = m_ComMgr.CreateImgDecoder((IObjRef **)&pImgDecoderFactory);
            SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("imgdecoder"));
            if (!bLoaded)
                break;

            pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
            m_theApp = new SApplication(pRenderFactory, hInstance);
            m_theApp->InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id, sizeof(R.id) / sizeof(int));
        } while (false);
        return bLoaded;
    };
    BOOL LoadRes()
    {
        BOOL bLoaded = FALSE;
        do
        {
#ifdef _WIN32
            SAutoRefPtr<IResProvider> sysResProvider;
            HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
            if (!hModSysResource)
                break;
            sysResProvider.Attach(m_souiFac.CreateResProvider(RES_PE));
            sysResProvider->Init((WPARAM)hModSysResource, 0);
            m_theApp->LoadSystemNamedResource(sysResProvider);
            FreeLibrary(hModSysResource);

            SAutoRefPtr<IResProvider> pResProvider;
            pResProvider.Attach(m_souiFac.CreateResProvider(RES_FILE));
            SStringT appRes = getSourceDir() + kPath_UiRes;
            bLoaded = pResProvider->Init((LPARAM)appRes.c_str(), 0);
            if (!bLoaded)
            {
                pResProvider.Attach(m_souiFac.CreateResProvider(RES_PE));
                bLoaded = pResProvider->Init((WPARAM)m_theApp->GetModule(), 0);
            }
            SASSERT_FMT(bLoaded, _T("load app resource failed"));
            if (!bLoaded)
                break;
            m_theApp->AddResProvider(pResProvider);
#else
            SAutoRefPtr<IResProvider> sysResProvider;
            sysResProvider.Attach(m_souiFac.CreateResProvider(RES_FILE));
            SStringT sysRes = getSourceDir() + kPath_SysRes;
            if (!sysResProvider->Init((LPARAM)sysRes.c_str(), 0))
                break;
            m_theApp->LoadSystemNamedResource(sysResProvider);
            // load use resource
            SAutoRefPtr<IResProvider> pResProvider(m_souiFac.CreateResProvider(RES_FILE), FALSE);
            SStringT appRes = getSourceDir() + kPath_UiRes;
            bLoaded = pResProvider->Init((LPARAM)appRes.c_str(), 0);
            if (!bLoaded)
                break;
            m_theApp->AddResProvider(pResProvider);
#endif //_WIN32
        } while (false);
        SASSERT(bLoaded);
        return bLoaded;
    }

    // 注册用户自定义皮肤和控件
    void Regitercustom()
    {
    }

    ~SOUIEngine()
    {
        if (m_theApp)
        {
            delete m_theApp;
            m_theApp = NULL;
        }
    }

    SApplication *GetApp()
    {
        return m_theApp;
    }
};

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));
    int nRet = 0;
    {
        SOUIEngine souiEngine;
        if (souiEngine.Init(hInstance))
        {
            // 加载系统资源
            souiEngine.LoadRes();
            // 注册用户自定义皮肤/控件/布局/其它自定义
            souiEngine.Regitercustom();
            SHostWnd *dlgMain = new CClientDlg();
            dlgMain->Create(GetActiveWindow());
            dlgMain->SendMessage(WM_INITDIALOG);
            dlgMain->CenterWindow();
            dlgMain->ShowWindow(SW_SHOWNORMAL);
            nRet = souiEngine.GetApp()->Run(dlgMain->m_hWnd);
            delete dlgMain;
        }
        else
        {
            MessageBox(0, _T("无法正常初使化SOUI"), _T("错误"), MB_OK | MB_ICONERROR);
        }
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