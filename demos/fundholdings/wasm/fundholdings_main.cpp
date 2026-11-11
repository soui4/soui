/*
 * fundholdings_main.cpp -- 基金持仓工具的 wasm 入口
 *
 * 桌面入口 main.cpp 用 app.Run() 阻塞跑消息循环，浏览器主线程不能被它占住，
 * 这里与 demos/wasm-demo/soui_main.cpp、games/cnchess/client/wasm/cnchess_main.cpp
 * 用同一套形态：
 *   1) emscripten_set_main_loop(FundPump, 0, 1) 每帧抽干消息队列（带时间预算）；
 *   2) IMessageLoop::ExecutePendingTask() —— 绕开 SApplication::Run() 后必须自己补，
 *      否则 SNotifyCenter 从后台线程 PostTask 回来的抓取结果不会执行；
 *   3) STaskLoopPump() —— 带 -pthread 的产物里任务跑在 worker 线程上，这一行只剩
 *      泵后端的队列；基金数据抓取因此不会占用 UI 帧。
 *
 * 网络：components/httpclient/wasm 用同步 XMLHttpRequest 实现 IHttpClient，而同步
 * XHR 只允许在 worker 里调用 —— 与本构建的 TaskLoop worker 后端正好配套。浏览器
 * 禁止脚本设置 Cookie/Referer/User-Agent、也读不到 Set-Cookie，所以请求默认经
 * demos/wasm-demo/serve-isolated.py 的同源转发端点 /__soui_http?url=... 转发
 * （页面里也可以用 window.souiHttpProxy 覆盖前缀）。
 */
#include <emscripten.h>
#include <stdio.h>
#include <string.h>

/* SCOM_MASK 必须在任何包含 commgr2.h 的头之前定义：SAppCfg::DoConfig() 和
 * SComMgr2 的工厂函数按它展开，掩码里多一位就会把 wasm 构建图里没有的组件变成
 * 真实链接依赖。这里只要渲染、图片解码、任务队列和浏览器 HTTP 客户端。 */
#define SCOM_MASK                                                                                  \
    (scom_mask_render_gdi | scom_mask_imgdecoder_stb | scom_mask_taskloop | scom_mask_httpclient)

#include "stdafx.h"

#include <SAppCfg.h>
#include <SGridCtrl.h>

#include "MainDlg.h"
#include "SFundWidgets.h"
#include "applog.h"

#include <TaskLoop/wasm/TaskLoopWasm.h>
#include <HttpClient.h>

#include "framebuffer.h"
#include "input.h"

// R 的实体只在本 TU 定义一次（与桌面 main.cpp 相同）：stdafx.h 里 res/resource.h
// 走 extern 声明分支，这里在 PCH 之后重新包含并打开 INIT_R_DATA。
#define INIT_R_DATA
#include "res/resource.h"

// MEMFS 挂载点，与 wasm/CMakeLists.txt 的 --preload-file 目标一一对应。
// wasm 版按 mobile/uires（竖屏 390x844）加载，皮肤/字体/滚动条皮肤都在 uires 里，
// 因此不加载系统资源，SAppCfg 遇到 ResType_Unknown 会跳过这一段。
static const TCHAR kAppResDir[] = _T("/opt/soui/uires");
static const TCHAR kAppDir[] = _T("/opt/soui");

// 一帧最多工作多久：要大于单帧重绘耗时，又远小于把标签页冻住的时间。
static const double kPumpBudgetMs = 100.0;

static HWND g_hWnd = NULL;
static CMainDlg *g_pMain = NULL;
static int g_fbW = 0;
static int g_fbH = 0;
static int g_cPumps = 0;
static int g_cPresents = 0;
static int g_cReady = 0;
static int g_cFatal = 0;
static int g_cPumpTruncated = 0;

static void host_fatal(const char *why)
{
    g_cFatal = 1;
    printf("[fundholdings-wasm] FATAL: %s\n", why);
    EM_ASM({
        if (typeof Module.souiFatal === 'function')
            Module.souiFatal(UTF8ToString($0));
    }, why);
}

static void host_present(const RECT *dirty)
{
    if (!dirty)
        return;
    ++g_cPresents;
    EM_ASM({
        if (typeof Module.swinxPresent === 'function')
            Module.swinxPresent($0, $1, $2, $3);
    }, dirty->left, dirty->top, dirty->right, dirty->bottom);
}

static SWindow *findByName(const char *pszName)
{
    if (!g_pMain || !g_hWnd || !pszName)
        return NULL;
    return g_pMain->FindChildByName(pszName);
}

extern "C" void FundPump(void)
{
    MSG msg;
    ++g_cPumps;
    g_cPumpTruncated = 0;

    const double tDeadline = emscripten_get_now() + kPumpBudgetMs;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            break;
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
        if (emscripten_get_now() >= tDeadline)
        {
            g_cPumpTruncated = 1;
            break;
        }
    }

    SApplication *pApp = SApplication::getSingletonPtr();
    if (pApp)
    {
        IMessageLoop *pLoop = pApp->GetMsgLoop();
        if (pLoop)
            pLoop->ExecutePendingTask();
    }

    double left = tDeadline - emscripten_get_now();
    if (left < 5.0)
        left = 5.0;
    STaskLoopPump(left);
}

/* ---------------- 页面注入 / 观测口（无头烟测用） ---------------- */
extern "C"
{
void FundMouse(UINT message, int x, int y, WPARAM fwKeys)
{
    if (g_hWnd)
        SwinxInjectMouse(g_hWnd, message, x, y, fwKeys);
}

void FundWheel(int x, int y, int zDelta, UINT fwKeys)
{
    if (g_hWnd)
        SwinxInjectMouse(g_hWnd, WM_MOUSEWHEEL, x, y, MAKEWPARAM((WORD)fwKeys, (WORD)zDelta));
}

void FundKey(UINT message, UINT vk, UINT scanCode)
{
    if (g_hWnd)
        SwinxInjectKey(g_hWnd, message, vk, scanCode);
}

void FundText(const char *pszUtf8)
{
    if (g_hWnd)
        SwinxInjectText(g_hWnd, pszUtf8);
}

int FundCaretPos(int *pXY)
{
    POINT pt;
    if (!::GetCaretPos(&pt))
        return 0;
    if (pXY)
    {
        pXY[0] = pt.x;
        pXY[1] = pt.y;
    }
    return 1;
}

/* 读控件文字（UTF-8 字节）。页面用它在烟测里断言状态栏/指数条确实更新了。 */
int FundTextOf(const char *pszName, char *pBuf, int nMax)
{
    if (!pBuf || nMax <= 0)
        return 0;
    pBuf[0] = '\0';
    SWindow *pWnd = findByName(pszName);
    if (!pWnd)
        return 0;
    SStringT str = pWnd->GetWindowText();
    int n = str.GetLength();
    if (n > nMax - 1)
        n = nMax - 1;
    if (n > 0)
        memcpy(pBuf, str.c_str(), (size_t)n);
    pBuf[n] = '\0';
    return n;
}

/* 用真实鼠标消息点一个控件（坐标取控件的宿主坐标中心）。烟测用它点「添加示例基金」。 */
int FundClick(const char *pszName)
{
    SWindow *pWnd = findByName(pszName);
    if (!pWnd || !g_hWnd)
        return 0;
    RECT rc;
    pWnd->GetWindowRect(&rc);
    int x = (rc.left + rc.right) / 2;
    int y = (rc.top + rc.bottom) / 2;
    SwinxInjectMouse(g_hWnd, WM_MOUSEMOVE, x, y, 0);
    SwinxInjectMouse(g_hWnd, WM_LBUTTONDOWN, x, y, MK_LBUTTON);
    SwinxInjectMouse(g_hWnd, WM_LBUTTONUP, x, y, 0);
    return 1;
}

int FundStat(int index)
{
    switch (index)
    {
        case 0: return g_cReady;
        case 1: return g_cPumps;
        case 2: return g_cPresents;
        case 3: return g_fbW;
        case 4: return g_fbH;
        case 5: return g_cFatal;
        case 6: return g_hWnd ? 1 : 0;
        case 7:
        {
            SWindow *p = findByName("watch_tabs");
            return p ? p->GetChildrenCount() : 0;
        }
        case 8:
        {
            SGridCtrl *p = g_pMain ? g_pMain->FindChildByName2<SGridCtrl>("grid_holdings") : NULL;
            return p ? p->GetRowCount() : 0;
        }
        case 9:
        {
            STabCtrl *p = g_pMain ? g_pMain->FindChildByName2<STabCtrl>("fund_swipe") : NULL;
            return p ? p->GetItemCount() : 0;
        }
        case 10: return g_cPumpTruncated;
        default: return 0;
    }
}
}

// 页面深链：fundholdings.html?fund=161725 直接把该基金填进输入框并触发查询。
// 浏览器里没有命令行参数，页面分享/刷新后又容易丢掉刚加的示例，留一个 URL 入口
// 既能做调试，也方便把某一支基金发给人看。
static void ApplyDeepLink()
{
    char *pszCode = emscripten_run_script_string(
        "(function(){var m=/[?&]fund=(\\d{6})/.exec(location.search);return m?m[1]:'';})()");
    if (!pszCode || strlen(pszCode) != 6)
        return;
    SWindow *pEdit = g_pMain ? g_pMain->FindChildByName("edit_code") : NULL;
    SWindow *pBtn = g_pMain ? g_pMain->FindChildByName("btn_query") : NULL;
    if (!pEdit || !pBtn)
        return;
    pEdit->SetWindowTextU8(pszCode);
    pBtn->FireCommand();
}

int main()
{
    g_fbW = SwinxFbWidth();
    g_fbH = SwinxFbHeight();

    SwinxFbClear(0xFF0E1520u);
    SwinxSetFbPresentCb(host_present);

    // 页面可在启动前设置 window.souiHttpProxy 覆盖同源代理前缀（默认为
    // "/__soui_http?url="）。必须在 CMainDlg 建任务线程之前生效。
    {
        char *pszProxy = emscripten_run_script_string(
            "(typeof window!=='undefined'&&window.souiHttpProxy)?String(window.souiHttpProxy):''");
        if (pszProxy && pszProxy[0])
            HttpClientWasmSetProxyPrefix(pszProxy);
    }

    // XML 里不带单位的尺寸默认按 px 解析；与桌面版一致改成 dp，DPR 档位变化时
    // size/padding/margin 跟着缩放。canvas 没有 DPI 缩放时 dp 与 px 等价。
    SLayoutSize::defUnit = SOUI::dp;

    // SApplication 是进程级单例；simulate_infinite_loop=1 会让 main() 以异常方式
    // 退出并展开栈，放栈上会在主循环刚开始时被析构。
    SApplication *app = new SApplication(GetModuleHandle(NULL));

    SAppCfg cfg;
    cfg.SetRender(Render_Gdi)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetAppDir(kAppDir)
        .SetAppResFile(kAppResDir)
        .SetLog(FALSE); // wasm 构建没有 log4z，界面日志仍由 CLogSink 的内存队列提供
    if (!cfg.DoConfig(app))
    {
        host_fatal("SAppCfg::DoConfig() 失败：渲染器/图片解码器/资源目录三者之一没起来");
        return 1;
    }
    app->InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id, sizeof(R.id) / sizeof(int));
    app->EnableNotifyCenter(TRUE);

    // 界面「请求日志」的数据来源。SetLog(FALSE) 没有转交给 log4z 的回调，Install
    // 保存下来的 m_prev 为 NULL，日志只进内存队列，不碰文件系统。
    CLogSink::Instance().Install();

    // 与桌面 main.cpp 的注册列表保持一致；XML 里用到的 <gridctrl> 与自定义控件
    // 缺一个，Create() 就会因为找不到窗口类失败。
    app->RegisterWindowClass<SGridCtrl>();
    app->RegisterWindowClass<SFundKpiCard>();
    app->RegisterWindowClass<SFundSpark>();
    app->RegisterWindowClass<SFundContrib>();
    app->RegisterWindowClass<SFundTabs>();
    app->RegisterWindowClass<SFundWatchTabs>();
    app->RegisterWindowClass<SFundIndexBar>();
    app->RegisterWindowClass<SFundAbout>();
    app->RegisterWindowClass<SFundScroll>();

    g_pMain = new CMainDlg();
    g_hWnd = g_pMain->Create(NULL, 0, 0, g_fbW, g_fbH);
    if (!g_hWnd)
    {
        char msg[192];
        snprintf(msg, sizeof(msg),
                 "CMainDlg::Create() 失败 (err=%lu)：布局 XML 或控件类注册有问题",
                 (unsigned long)GetLastError());
        host_fatal(msg);
        return 2;
    }

    // Create 只建窗口；WM_INITDIALOG 才会跑 CMainDlg::OnInitDialog（建 Http/TaskLoop、
    // 载入自选、发起第一次后台抓取）。桌面版由 Create 内部同步发，这里显式发一次。
    g_pMain->SendMessage(WM_INITDIALOG);
    g_pMain->ShowWindow(SW_SHOWNORMAL);
    g_cReady = 1;
    ApplyDeepLink();

    printf("[fundholdings-wasm] ready %dx%d\n", g_fbW, g_fbH);
    emscripten_set_main_loop(FundPump, 0, 1);
    return 0;
}
