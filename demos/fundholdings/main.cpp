#include "stdafx.h"
#include <SAppCfg.h>    // header-only 实现：只能由本 TU 包含一次
#include <SGridCtrl.h>  // RegisterWindowClass<SGridCtrl>
#include "MainDlg.h"
#include "SFundWidgets.h"
#include "applog.h"

// R 的实体只在 main.cpp 定义一次：stdafx.h（以及其它 TU）里 res/resource.h
// 走的是 extern 声明分支；此处在 PCH 之后重新包含并打开 INIT_R_DATA。
// res/resource.h 的 #ifdef INIT_R_DATA 块位于 include guard 之外，故仍会展开。
#define INIT_R_DATA
#include "res/resource.h"

#if defined(__IOS__)
#include <ios_entry.h>
#endif

#define kLogTag "fundhold"

// exe 所在目录：cmake 的 add_app_res_folder 会把 uires 拷到这里
// （与 demos/uieditor 的做法一致），因此打包后的程序不再依赖源码树路径。
static SStringT getAppDir() {
    TCHAR szPath[MAX_PATH] = {0};
    ::GetModuleFileName(NULL, szPath, MAX_PATH);
    SStringT s(szPath);
    int p = s.ReverseFind(PATH_SLASH);
    if (p > 0) return s.Left(p);
    return SStringT(_T("."));
}

// 资源根目录：macOS 下资源装在 .app/ 根目录（add_macos_res_folder 的布局），
// exe 本身在 .app/Contents/MacOS/，不能直接拿 exe 目录当资源目录。
static SStringT getResDir() {
#ifdef __APPLE__
    char szBundlePath[1024] = {0};
    GetAppleBundlePath(szBundlePath, sizeof(szBundlePath));
    if (szBundlePath[0]) return S_CA2T(szBundlePath);
#endif
    return getAppDir();
}

static SStringT getSysResDir() {
#ifdef __APPLE__
    return getResDir();
#endif
    SStringA strFile(__FILE__);
    return S_CA2T(strFile.Left(strFile.ReverseFind(PATH_SLASH)) + "/../..");
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/) {
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));

    // XML 里不带单位的尺寸默认按 px 解析（SLayoutSize::defUnit 的出厂值），
    // 那样界面的物理像素固定，配合 applyDpiScale 会出现"字体放大了、间距没放大"的错位。
    // 改成 dp 之后 size/padding/margin/圆角半径等全部随 scale 一起缩放；
    // 100% DPI 下 dp 与 px 数值等价，不改变原有观感。
    SLayoutSize::defUnit = SOUI::dp;

    int nRet = 0;
    SApplication app(hInstance);
    SAppCfg cfg;
    const SStringT resDir = getResDir();
    const SStringT sysResDir = getSysResDir();
    cfg.SetRender(Render_Skia)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetAppDir(resDir)
        .SetLog(TRUE, LOG_LEVEL_INFO, "fundholdings");  // log4z 负责落盘，界面日志来自 CLogSink

    // ENABLE_BUILD_RESOURCE 由 CMake 的 target_compile_resources 在确实用
    // windres 编译了 .rc 后定义（Windows 恒为真；Linux/macOS 取决于是否装了
    // MinGW）。有内嵌资源就直接从 PE 段读，否则退回磁盘上的 uires 目录。
#ifdef ENABLE_BUILD_RESOURCE
    cfg.SetAppResPeHandle(hInstance);
#else
    cfg.SetAppResFile(resDir + _T("/uires"));
#endif

#ifdef ENABLE_BUILD_RESOURCE
    cfg.SetSysResPeHandle(hInstance);
#else
    cfg.SetSysResFile(sysResDir + _T("/soui-sys-resource-dark"));
#endif

    if (!cfg.DoConfig(&app)) {
        return -1;
    }
    app.InitXmlNamedID((const LPCWSTR*)&R.name, (const int*)&R.id, sizeof(R.id) / sizeof(int));
    app.EnableNotifyCenter(TRUE);

    // DoConfig 里 SetLog(TRUE) 会把 SLog 回调接到 log4z（SouiLog_Callback）。
    // 在这之后挂上 CLogSink：日志先入内存队列供界面显示，再转交给 log4z 落盘，
    // 两边行为都保留，界面侧完全不用再碰日志文件。
    CLogSink::Instance().Install();

    // 注册外部窗口类（XML 里的标签名由 DEF_SOBJECT 决定）
    app.RegisterWindowClass<SGridCtrl>();
    app.RegisterWindowClass<SFundKpiCard>();   // <fundkpi>
    app.RegisterWindowClass<SFundSpark>();     // <fundspark>
    app.RegisterWindowClass<SFundContrib>();   // <fundcontrib>
    app.RegisterWindowClass<SFundTabs>();      // <fundtabs>
    app.RegisterWindowClass<SFundWatchTabs>(); // <fundwatchtabs>
    app.RegisterWindowClass<SFundIndexBar>(); // <fundindexbar> 大盘三大指数速览
    app.RegisterWindowClass<SFundAbout>();    // <fundabout> SOUI5 介绍页
    app.RegisterWindowClass<SFundScroll>();   // <fundscroll> 移动版竖向滚动容器

    {
        CMainDlg dlgMain;
        dlgMain.Create(GetActiveWindow());
        dlgMain.SendMessage(WM_INITDIALOG);
        dlgMain.CenterWindow(dlgMain.m_hWnd);
#ifdef __IOS__
        // 手机只有一个全屏窗口，直接最大化；
        // 布局本身由 mobile/uires 的 dlg_main.xml 决定，这里不做 UI 逻辑分支。
        dlgMain.ShowWindow(SW_MAXIMIZE);
#else
        dlgMain.ShowWindow(SW_SHOWNORMAL);
#endif
        nRet = app.Run(dlgMain.m_hWnd);

        // 窗口已经销毁，此时再等后台线程收尾，用户看不到任何卡顿。
        // （如果在 ~CMainDlg 里做，关窗口就要等正在跑的 HTTP 返回，看上去像死机。）
        dlgMain.Shutdown();
    }

    CLogSink::Instance().Uninstall();
    OleUninitialize();
    return nRet;
}

#if defined(__IOS__)
int main(int argc, char **argv) {
    return swinx_ios_entry(argc, argv, _tWinMain);
}
#elif !defined(_WIN32) || defined(__MINGW32__)
int main(int argc, char** argv) {
    HINSTANCE hInst = GetModuleHandle(NULL);
    return _tWinMain(hInst, 0, NULL, SW_SHOWNORMAL);
}
#endif  //_WIN32
