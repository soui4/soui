#include "stdafx.h"
#include <SAppCfg.h>
#include <SouiFactory.h>
#include <helper/SMenuWndHook.h>
#include <helper/SAutoBuf.h>
#include <controls.extend/SFadeFrame.h>
#include <controls.extend/SRadioBox2.h>
#include <controls.extend/SVscrollbar.h>
#include <controls.extend/SSkinNewScrollBar.h>
#include <controls.extend/SChromeTabCtrl.h>

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

#if defined(_WIN32) && !defined(__MINGW32__)
#include <controls.extend/SIECtrl.h>
#include "uianimation/UiAnimationWnd.h"
#include "SmileyCreateHook.h"
#endif

#include "appledock/SDesktopDock.h"
#include "SMatrixWindow.h"

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

static const TCHAR *kPath_SysRes = _T("/../../soui-sys-resource");
static const TCHAR *kPath_UiRes = _T("/uires");

#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")

#include <controls.extend/smiley/SSmileyCtrl.h>
#include "skin/SSkinLoader.h"
#include "trayicon/SShellTray.h"
#include "qrcode/SQrCtrl.h"


#define INIT_R_DATA
#include "res/resource.h"

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

class SApplication2 : public SApplication {
  public:
    SApplication2(HINSTANCE hInst)
        : SApplication(hInst)
    {
    }

  protected:
    STDMETHOD_(IBitmapS *, LoadImage)(THIS_ LPCTSTR pszType, LPCTSTR pszResName)
    {
        int nBufSize = GetRawBufferSize(pszType, pszResName);
        SAutoBuf buf(nBufSize);
        char *pBuf = buf;
        BOOL bLoad = GetRawBuffer(pszType, pszResName, pBuf, nBufSize);
        if (bLoad && nBufSize > 6)
        {
            if (_tcscmp(pszType, _T("svg")) == 0)
            {
                const unsigned char bom16[2] = { 0xff, 0xfe };
                const unsigned char bom8[3] = { 0xef, 0xbb, 0xbf };
                SStringA strBuf;
                if (memcmp(pBuf, bom16, 2) == 0)
                {
                    strBuf = S_CW2A(SStringW((WCHAR *)(pBuf + 2), (nBufSize - 2) / 2), CP_UTF8);
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
                    NSVGimage *image = nsvgParse((char *)strBuf.c_str(), "px", 96.0f);
                    IBitmapS *Ret = NULL;
                    if (image)
                    {
                        int w = (int)image->width;
                        int h = (int)image->height;

                        NSVGrasterizer *rast = nsvgCreateRasterizer();

                        unsigned char *img = (unsigned char *)malloc(w * h * 4);
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
    // 必须要调用OleInitialize来初始化运行环境
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));
    int nRet = 0;

    // 将程序的运行路径修改到demo所在的目录
    SStringT appDir = getSourceDir();
    SetCurrentDirectory(appDir);
#ifdef __linux__
    AddFontResource((appDir + _T("/../../simsun.ttc")).c_str());
#elif defined(__APPLE__)
    AddFontResource((appDir + _T("/fonts/simsun.ttc")).c_str());
#endif
    int nType = MessageBox(GetActiveWindow(), _T("选择渲染类型：\n[yes]: Skia\n[no]:GDI\n[cancel]:Quit"), _T("select a render"), MB_ICONQUESTION | MB_YESNOCANCEL);
    if (nType == IDCANCEL)
    {
        return 0;
    }
    SApplication2 app(hInstance);
    SAppCfg cfg;
    cfg.SetRender(nType == IDYES ? Render_Skia : Render_Gdi)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetAppDir(appDir)
        .SetLog(TRUE, 2, "demo")
        .EnableMultiLang(_T("translator:lang_cn"),TRUE);
#if (defined(DLL_CORE) || (defined(LIB_CORE) && defined(LIB_SOUI_COM)))
    cfg.EnableScript(TRUE);
#endif // DLL_CORE

// 加载系统资源
#ifdef _WIN32
#if (defined(LIB_CORE) && defined(LIB_SOUI_COM))
    cfg.SetSysResPeHandle(hInstance);
#else
    cfg.SetSysResPeFile(SYS_NAMED_RESOURCE);
#endif
#else
#ifdef __APPLE__
    static const TCHAR *kPath_SysRes = _T("/soui-sys-resource");
#else
    static const TCHAR *kPath_SysRes = _T("/../../soui-sys-resource");
#endif //__APPLE__
    cfg.SetSysResFile(appDir + kPath_SysRes);
#endif
#if defined(_WIN32) && (RES_TYPE == RESTYPE_PE) // 从EXE资源加载
    cfg.SetAppResPeHandle(hInstance);
#elif (RES_TYPE == RESTYPE_ZIP) // 从ZIP包加载
    cfg.SetAppResZipFile(appDir + _T("/uires.zip"), "souizip");
#elif (RES_TYPE == RESTYPE_7Z)  // 从7z包加载
    cfg.SetAppRes7ZipFile(appDir + _T("/uires.zip"), "souizip");
#else // #if (RES_TYPE == RESTYPE_FILE)//从文件加载
    cfg.SetAppResFile(appDir + _T("/uires"));
#endif

    // 向SApplication系统中注册由外部扩展的控件及SkinObj类
    app.RegisterSkinClass<SSkinAni>();          // 注册SkinGif
    app.RegisterSkinClass<SSkinVScrollbar>();   // 注册纵向滚动条皮肤
    app.RegisterSkinClass<SSkinNewScrollbar>(); // 注册纵向滚动条皮肤
    app.RegisterSkinClass<SDemoSkin>();

    SWkeLoader wkeLoader;
    if (wkeLoader.Init(_T("wke.dll")))
    {
        app.RegisterWindowClass<SWkeWebkit>(); // 注册WKE浏览器
    }
    app.RegisterWindowClass<SMatrixWindow>();   //
    app.RegisterWindowClass<S3dWindow>();       //
    app.RegisterWindowClass<SFreeMoveWindow>(); //
    app.RegisterWindowClass<SClock>();          //
    app.RegisterWindowClass<SGifPlayer>();      // theApp中增加方法：RegisterWindowClass,替换RegisterWndFactory(TplSWindowFactory<SGifPlayer>())

    app.RegisterWindowClass<SMCListViewEx>();
    app.RegisterWindowClass<SHeaderCtrlEx>();
    app.RegisterWindowClass<SIPAddressCtrl>(); // 注册IP控件
    app.RegisterWindowClass<SPropertyGrid>();  // 注册属性表控件
    app.RegisterWindowClass<SChromeTabCtrl>(); // 注册ChromeTabCtrl
    app.RegisterWindowClass<SChatEdit>();      // 注册ChatEdit
    app.RegisterWindowClass<SScrollText>();    // 注册SScrollText
    app.RegisterWindowClass<SDesktopDock>();   // 注册SDesktopDock
    app.RegisterWindowClass<SImageMaskWnd>();  // 注册SImageMaskWnd
    app.RegisterWindowClass<SRatingBar>();     // 注册SRatingBar
    app.RegisterWindowClass<SInterpolatorView>();
    app.RegisterWindowClass<SPathView>();
    app.RegisterWindowClass<SQrCtrl>();
    app.RegisterWindowClass<SProgressRing>();
    app.RegisterWindowClass<SCheckBox2>();
    app.RegisterWindowClass<SAniWindow>();
    app.RegisterWindowClass<SGroupList>();
    app.RegisterWindowClass<SRoundImage>();
    app.RegisterWindowClass<SRoundWnd>();
    app.RegisterWindowClass<SHexEdit>();
    app.RegisterWindowClass<SImageKnob>();
    app.RegisterWindowClass<SFadeFrame>(); // 注册渐显隐动画控件
    app.RegisterWindowClass<SRadioBox2>(); // 注册RadioBox2
    app.RegisterWindowClass<SCalendar2>(); // 注册SCalendar2

    app.RegisterWindowClass<SShellTray>();
    app.RegisterWindowClass<FpsWnd>();

#if defined(_WIN32) && !defined(__MINGW32__)
    app.RegisterWindowClass<SIECtrl>(); // 注册IECtrl
    app.RegisterWindowClass<SStaticGdip>();
    if (SUCCEEDED(CUiAnimation::Init()))
    {
        app.RegisterWindowClass<SUiAnimationWnd>(); // 注册动画控件
    }
#endif

    if (!cfg.DoConfig(&app))
    {
        return -1;
    }
    // 如果需要在代码中使用R::id::namedid这种方式来使用控件必须要这一行代码：2016年2月2日，R::id,R.name是由uiresbuilder 增加-h .\res\resource.h 这2个参数后生成的。
    app.InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id, sizeof(R.id) / sizeof(int));
    SSkinLoader *SkinLoader = new SSkinLoader(&app);
    SkinLoader->LoadDefSkin();

#ifdef _WIN32
    // 采用hook绘制菜单的边框
        SMenuWndHook::InstallHook(hInstance, L"_skin.sys.menu.border");
#endif

    // 演示R.color.xxx,R.string.xxx在代码中的使用。
    COLORREF crRed = GETCOLOR(R.color.red);
    SStringW strTitle = GETSTRING(R.string.title);

    app.EnableNotifyCenter(TRUE);
    {
#if defined(_WIN32) && !defined(__MINGW32__)
        SmileyCreateHook smileyHook;
#endif
        // 设置提示窗口布局
        STipWnd::SetLayout(_T("layout:dlg_tip"));
        CMainDlg dlgMain;
        dlgMain.Create(GetActiveWindow(), 0, 0, 888, 650);
        dlgMain.GetNative()->SendMessage(WM_INITDIALOG);
        dlgMain.CenterWindow();
        dlgMain.ShowWindow(SW_SHOWNORMAL);

        nRet = app.Run(dlgMain.m_hWnd);
    }

    // 应用程序退出
    delete SkinLoader;

#ifdef _WIN32
        // 卸载菜单边框绘制hook
        SMenuWndHook::UnInstallHook();
#endif
#if defined(_WIN32) && !defined(__MINGW32__)
        CUiAnimation::Free();
#endif
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
