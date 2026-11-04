/*
 * cnchess_main.cpp -- 中国象棋客户端的 wasm 宿主入口
 *
 * 与桌面入口 main.cc 的差别只有"谁来驱动帧"和"谁来发网络事件"：
 *
 *   1) 帧驱动：浏览器主线程不能被 SApplication::Run() 的阻塞循环占住，改成
 *      emscripten_set_main_loop(CnchessPump, 0, 1)。每一帧做三件事，顺序有讲究：
 *        a. PeekMessage/TranslateMessage/DispatchMessage 抽干（带时间预算），
 *           SOUI 的定时器、动画、输入都走这条；
 *        b. IMessageLoop::ExecutePendingTask()——SApplication::Run() 本来在每条
 *           消息前后调它，绕开 Run() 就必须自己调，否则 ThemeDownloadManager 从
 *           解压流程 PostTask 回来的进度/结果永远不执行，主题不会落地；
 *        c. STaskLoopPump()——wasm 的 ITaskLoop 有两条后端：不带 -pthread 的产物里它是
 *           一张主线程队列，解压任务就在这一行里跑完；带 -pthread 的产物里 start() 已经
 *           建了一条 worker 线程，队列归它消费，这一行会跳过这些实例（本函数什么都不做，
 *           慢任务一帧都不占）。桌面版它一直是工作线程。
 *   2) 网络：ws 组件在 wasm 下用 Emscripten 的 WebSocket，回调由浏览器事件循环在
 *      主线程同步投递（见 components/ws/src/wasm/），不再需要收包线程。
 *   3) 弹窗：登录、确认、求和都用 SModalRoot（CMainDlg 里的 __EMSCRIPTEN__ 分支），
 *      没有 SHostDialog::DoModal 的嵌套消息循环。
 *
 * 统计量 / 输入注入只服务于无头烟测：node 里加载这个模块，驱动登录、看主题是否落地、
 * 棋盘是否插入、解压任务有没有按构建能力跑到 worker 线程上去（stat 19 与 6 是否相等）。
 */
#include <emscripten.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

/* 客户端自己的预编译头：后面 MainDlg.h/LobbyHandler.h 里的 SObject/SWindow/
 * SAutoRefPtr 都是不带命名空间写的，靠 stdafx.h 末尾的 using namespace SOUI 才能解析。 */
#include "stdafx.h"

#include <SApp.h>
#include <SPlaySound.h>

#include <commask.h>
/* SCOM_MASK 必须在 <SAppCfg.h> 之前定义：SAppCfg::DoConfig() 的 switch 无条件引用
 * 所有组件的 SCreateInstance，掩码全开时 wasm 构建图里没有的组件会变成真实链接依赖
 * （直接失败或把 libwebsockets / 7z 拉进来）。这里只要这五个。
 * taskloop 和 resprovider-zip 是客户端自己要的：解压主题包、任务队列（带 -pthread 时
 * 是 worker 线程，不带时是主线程泵）。 */
#define SCOM_MASK                                                                                  \
    (scom_mask_render_gdi | scom_mask_imgdecoder_stb | scom_mask_taskloop | scom_mask_ws |          \
     scom_mask_resprovider_zip)

#include <SAppCfg.h>
#include <TaskLoop/wasm/TaskLoopWasm.h>
#include <ws/src/wasm/WsClientWasm.h>
#include <gif/SSkinAni.h>
#include <gif/SGifPlayer.h>
#include <helper/slog.h>

#include "MainDlg.h"
#include "SGameTheme.h"
#include "CnchessSkin.h"
#include "ChessBoard.h"
#include "ChessPiece.h"
#include "SRatingBar.h"

#include "framebuffer.h"
#include "input.h"

/* MEMFS 挂载点，与 wasm/CMakeLists.txt 的 --preload-file 目标一一对应。
 * canvas2d 后端文本走浏览器内置字体，不挂字体目录；cairo 后端会额外把 fonts/
 * 预载进 /opt/soui/fonts（见 CNCHESS_WASM_CAIRO_FONTS 分支的 AddFontResourceA）。 */
static const TCHAR kSysResDir[] = _T("/opt/soui/soui-sys-resource");
static const TCHAR kAppResDir[] = _T("/opt/soui/uires");
static const TCHAR kAppDir[] = _T("/opt/soui");

/* 一帧最多工作多久。与 soui-wasm 同口径：要大于单帧重绘耗时，又远小于把标签页
 * 冻住的时间。超预算时剩下的消息留给下一帧（requestAnimationFrame 会再来）。 */
static const double kPumpBudgetMs = 100.0;

static HWND g_hWnd = NULL;
static CMainDlg *g_pMain = NULL;
static SAutoRefPtr<SGameTheme> g_pTheme;
static MyProfile *g_pProfile = NULL;

static int g_fbW = 0;
static int g_fbH = 0;
static int g_cPumps = 0;
static int g_cPresents = 0;
static int g_cReady = 0;
static int g_cFontOk = 0;
static int g_cPumpTruncated = 0;

static void host_fatal(const char *why)
{
    printf("[cnchess-wasm] FATAL: %s\n", why);
    /* 页面侧钩子名与 soui-wasm 的宿主（soui-host.js）统一成 souiFatal；
     * cnchessFatal 是旧外壳用的名字，留一份兜底，免得缓存里换到旧壳页时
     * 启动失败完全没有提示。 */
    EM_ASM({
        if (typeof Module.souiFatal === 'function')
            Module.souiFatal(UTF8ToString($0));
        else if (typeof Module.cnchessFatal === 'function')
            Module.cnchessFatal(UTF8ToString($0));
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

static SWindow *findChild(const char *pszName)
{
    if (!g_pMain || !g_hWnd)
        return NULL;
    return g_pMain->FindChildByName(pszName);
}

/* 控件文字按字节读（wasm 构建 TCHAR 就是 char，内容本来就是 UTF-8）。 */
static int windowTextBytes(SWindow *pWnd, char *pBuf, int nMax)
{
    if (!pWnd || !pBuf || nMax <= 0)
        return 0;
    TCHAR szBuf[256];
    int n = pWnd->GetWindowText(szBuf, 256, TRUE);
    if (n < 0)
        n = 0;
    if (n > 256)
        n = 256;
    if (n > nMax)
        n = nMax;
    memcpy(pBuf, szBuf, n * sizeof(TCHAR));
    return n * (int)sizeof(TCHAR);
}

extern "C"
{
    /* 统计量。下标是烟测与页面外壳共享的契约：
     *   0  泵次数              1  上屏次数            2  宿主窗口建好
     *   3  字体注册结果        4  主题已加载          5  chessboard 子节点数
     *   6  TaskLoop 累计执行   7  TaskLoop 排队数    8  TaskLoop 存活实例数
     *   9  登录弹窗可见        10 根窗口宽           11 根窗口高
     *   12 上一帧是否超预算    13 GetTickCount()     14 主题进度弹窗可见
     *   15 登录弹窗存在        16 根窗口直接子节点数 17 主对话框已初始化
     *   18 worker 后端的实例数  19 worker 线程执行的任务数  20 这份产物带没带 -pthread
     *   21 ws 投递给监听器的帧数 22 其中发生在非主线程的帧数
     *
     * 4/5 是端到端的主证据：4 说明服务器下发的主题包走完了 ws 收包 + 落盘 + 解压
     * （TaskLoop）+ 加载；5 说明 InitGame 真的把背景/棋盘/90 个落子位插进了 chessboard
     * （game_container 里只有一个 chessboard，那是布局自带的结构，不能拿来做证据）。
     * 6/7/8 证明 TaskLoop 后端在这条链上被用到，并且队列确实被宿主泵清空了。
     * 18/19/20 是 worker 后端的取证三口：20 说这份产物链接时带没带 -pthread（构建能力），
     * 18 说当前有几条队列真由 worker 线程驱动（运行时，页面没 crossOriginIsolated 时
     * SharedArrayBuffer 拿不到，退回泵后端就是 0），19 说累计有几条任务是在 worker 上
     * 跑掉的。19 与 6 相等才是"解压整条离开了 UI 线程"，光看 6>0 两条后端都给得出。
     * 21/22 是同一口径用在收包链上：ws 组件自己开了一条投递队列（与桌面版每个连接
     * 一条工作线程同形态），21 说有几帧真的交到了 IConnListener 手里，22 说其中几帧
     * 不在主线程上。浏览器里 socket 对象建在主线程（工具链限制），所以"收包搬后台"能
     * 证明的边界就是回调之后那一段 —— 22 与 21 相等即是这段全搬走了。 */
    int CnchessStat(int index)
    {
        switch (index)
        {
        case 0: return g_cPumps;
        case 1: return g_cPresents;
        case 2: return g_cReady;
        case 3: return g_cFontOk;
        case 4: return (g_pMain && g_pMain->IsThemeLoaded()) ? 1 : 0;
        case 5:
        {
            SWindow *pBoard = findChild("chessboard");
            return pBoard ? (int)pBoard->GetChildrenCount() : -1;
        }
        case 6: return (int)STaskLoopRunCount();
        case 7: return STaskLoopPendingCount();
        case 8: return STaskLoopLiveCount();
        case 9:
        {
            SWindow *pBtn = findChild("btn_login");
            return (pBtn && pBtn->IsVisible(TRUE)) ? 1 : 0;
        }
        case 10:
        case 11:
        {
            RECT rc;
            if (!g_pMain || !g_hWnd)
                return 0;
            SWindow *pRoot = g_pMain->GetRoot();
            if (!pRoot)
                return 0;
            pRoot->GetWindowRect(&rc);
            return (index == 10) ? (rc.right - rc.left) : (rc.bottom - rc.top);
        }
        case 12: return g_cPumpTruncated;
        case 13: return (int)GetTickCount();
        case 14:
        {
            SWindow *pStatus = findChild("txt_status");
            return (pStatus && pStatus->IsVisible(TRUE)) ? 1 : 0;
        }
        case 15: return findChild("btn_login") ? 1 : 0;
        case 16:
        {
            if (!g_pMain || !g_hWnd)
                return 0;
            SWindow *pRoot = g_pMain->GetRoot();
            return pRoot ? (int)pRoot->GetChildrenCount() : 0;
        }
        case 17: return g_pMain ? 1 : 0;
        case 18: return STaskLoopWorkerCount();
        case 19: return (int)STaskLoopWorkerRunCount();
        case 21: return (int)WS_WasmDeliveredFrames();
        case 22: return (int)WS_WasmDeliveredFramesOffMain();
        /* 构建能力，不是运行时状态：产物没带 -pthread 时这里是 0，烟测据此决定该期待
         * 哪条后端。运行时有没有真起线程看 18 —— 页面缺 crossOriginIsolated 时 20 是 1
         * 而 18 是 0，这条差值就是"能用但没并行"那个坑的取证入口。 */
#ifdef __EMSCRIPTEN_PTHREADS__
        case 20: return 1;
#else
        case 20: return 0;
#endif
        default: return 0;
        }
    }

    void CnchessPump(void)
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

        /* SApplication::Run() 里每条消息前后都会跑这一步，这里绕开了 Run()，
         * 必须自己补：ThemeDownloadManager 把解压进度/结果 PostTask 回 UI 线程，
         * 不执行的话主题永远停在"下载完成但没加载"。 */
        SApplication *pApp = SApplication::getSingletonPtr();
        if (pApp)
        {
            IMessageLoop *pLoop = pApp->GetMsgLoop();
            if (pLoop)
                pLoop->ExecutePendingTask();
        }

        /* 泵后端：解压这类任务就在这一行里跑完。worker 后端（带 -pthread 的产物）里
         * 这些实例被跳过，这一行只剩泵后端那些没建线程的队列，预算照给不影响正确性。
         * 预算至少留 5ms，避免消息已经把 100ms 吃光时把 0 传进去（0 在 API 里是"不限预算"）。 */
        double left = tDeadline - emscripten_get_now();
        if (left < 5.0)
            left = 5.0;
        STaskLoopPump(left);
    }

    void CnchessMouse(UINT message, int x, int y, WPARAM fwKeys)
    {
        if (g_hWnd)
            SwinxInjectMouse(g_hWnd, message, x, y, fwKeys);
    }

    /* 滚轮也是鼠标消息，但 wParam 的布局不同：低字仍是 MK_ 键位，高字是带符号的
     * 滚动量（Win32 以 WHEEL_DELTA=120 为一格）。SwinxInjectMouse 把这个值原样交给
     * 窗口，所以打包放在宿主这一层，不新增 swinx 接口。zDelta 由页面换算成 120 的
     * 整数倍——浏览器给的是像素/行/页三种单位，页面侧还要把不足一格的增量攒起来，
     * 否则触控板每次几像素的 deltaY 取整后恒为 0，STileView::OnMouseWheel 永远滚不动。
     *
     * 调用方（cnchess.html）必须先补一条同坐标的 WM_MOUSEMOVE：SHostWnd::OnMouseWheel
     * 不读滚轮消息自带的 lParam，而是用最近一条鼠标消息缓存的坐标做命中测试。 */
    void CnchessWheel(int x, int y, int zDelta, UINT fwKeys)
    {
        if (g_hWnd)
            SwinxInjectMouse(g_hWnd, WM_MOUSEWHEEL, x, y, MAKEWPARAM((WORD)fwKeys, (WORD)zDelta));
    }

    void CnchessKey(UINT message, UINT vk, UINT scanCode)
    {
        if (g_hWnd)
            SwinxInjectKey(g_hWnd, message, vk, scanCode);
    }

    /* 输入法提交的成品文本（UTF-8）：登录框里输中文昵称走这条路，不经按键映射。 */
    void CnchessText(const char *pszUtf8)
    {
        if (g_hWnd)
            SwinxInjectText(g_hWnd, pszUtf8);
    }

    /* 当前插入符位置（宿主窗口坐标 == 帧缓冲坐标：窗口建在 (0,0)）。SOUI 的 SCaret
     * 每次移动光标都调 ::SetCaretPos，值存在 wasm SConnection 里。页面用它把输入法
     * 代理框钉在正在编辑的那行字旁边——浏览器的候选词窗跟着焦点元素走，代理框钉在
     * 画布外，候选窗就飘到屏幕外面，用户看不见自己在往哪儿输入。
     * 返回 0 表示还没有窗口建过插入符（焦点不在编辑类控件上时就是这样）。 */
    int CnchessCaretPos(int *pXY)
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

    /* 按音效名走一遍真实链路：主题 sounds.xml 查相对路径 -> 文件前缀拼成绝对路径 ->
     * SPlaySound(SND_ASYNC|SND_NOSTOP|SND_FILENAME)。CChessGame::PlayEffectSound 就是
     * 这三步，只是它要等到落子才触发；无头烟测需要在不进入棋局的情况下取证"主题包里
     * 的 wav 能不能被 swinx 读出来并交给页面"，所以把同一段逻辑单独开一个口。
     * 返回值：0=主题或主对话框没就绪，1=这个名字在 sounds.xml 里没有，
     * 2=PlaySound 返回 TRUE（已交给页面钩子），3=返回 FALSE（页面没装 swinxAudioPlay
     * 钩子，或文件不在 wasm 文件系统里）。3 与 2 的区别正是这道口有没有意义的地方。 */
    int CnchessPlayEffectSound(const char *pszNameUtf8)
    {
        if (!g_pMain || !g_pTheme || !pszNameUtf8)
            return 0;
        SStringW strRel = g_pTheme->GetEffectSoundFile(S_CA2W(pszNameUtf8, CP_UTF8));
        if (strRel.IsEmpty())
            return 1;
        TCHAR szPrefix[MAX_PATH] = {0};
        SApplication::getSingletonPtr()->GetFilePrefix(szPrefix, MAX_PATH);
        SStringT strFull = SStringT(szPrefix) + S_CW2T(strRel, CP_UTF8);
        return SPlaySound(strFull, NULL, SND_ASYNC | SND_NOSTOP | SND_FILENAME) ? 2 : 3;
    }

    /* 按控件名点一下（按下 + 抬起）。模态视图被 InsertChild 进宿主根窗口，所以
     * 名字查找能穿过模态层直接找到 btn_login / btn_avatar_N / btn_ok。 */
    int CnchessClick(const char *pszName)
    {
        SWindow *pWnd = findChild(pszName);
        if (!pWnd)
            return 0;
        RECT rc;
        pWnd->GetWindowRect(&rc);
        int x = (rc.left + rc.right) / 2;
        int y = (rc.top + rc.bottom) / 2;
        /* 先泵到命中测试真的落到目标控件上再注入。原因：SModalView 在入场动画期间会
         * 短路 SwndFromPoint（SOUI/src/core/SModalViewSession.cpp 里 m_isAnimating 那条
         * 分支），整棵子树都收不到鼠标消息。动画靠 kPulseTimer 的 WM_TIMER 推进，而
         * WM_TIMER 只按**真实时间**到期：实测 node 里空泵 32 帧只花 1ms，一帧动画都没推进
         * （命中窗口仍是 modal 视图）；放开到 1.5s 墙上时间后约 140ms 命中就落到目标控件、
         * 插入符随之出现。浏览器里真人点击天然落在动画结束之后，只有注入式烟测会撞上这两帧。
         * 到上限仍没落点就照原样注入，让失败留在烟测断言里而不是这里静默吞掉。 */
        SWindow *pRoot = g_pMain ? g_pMain->GetRoot() : NULL;
        const double tDeadline = emscripten_get_now() + 1500.0;
        for (int i = 0; pRoot && i < 100000; ++i)
        {
            CPoint ptClick(x, y);
            if (pRoot->SwndFromPoint(ptClick, TRUE) == pWnd->GetSwnd())
                break;
            CnchessPump();
            if (emscripten_get_now() >= tDeadline)
                break;
        }
        CnchessMouse(WM_MOUSEMOVE, x, y, 0);
        CnchessMouse(WM_LBUTTONDOWN, x, y, MK_LBUTTON);
        CnchessMouse(WM_LBUTTONUP, x, y, 0);
        return 1;
    }

    /* 直接写控件文字。烟测用它填服务器地址/用户名：它们要验证的是"点登录之后
     * ws + 主题链路"，不是键盘映射，逐键注入只会把失败原因搅在一起。 */
    int CnchessSetText(const char *pszName, const char *pszUtf8)
    {
        SWindow *pWnd = findChild(pszName);
        if (!pWnd)
            return 0;
        pWnd->SetWindowText(S_CA2T(pszUtf8, CP_UTF8));
        return 1;
    }

    int CnchessChildText(const char *pszName, char *pBuf, int nMax)
    {
        return windowTextBytes(findChild(pszName), pBuf, nMax);
    }

    int CnchessChildVisible(const char *pszName)
    {
        SWindow *pWnd = findChild(pszName);
        if (!pWnd)
            return 0;
        return pWnd->IsVisible(TRUE) ? 1 : 2;
    }

    int CnchessChildCount(const char *pszName)
    {
        SWindow *pWnd = findChild(pszName);
        return pWnd ? (int)pWnd->GetChildrenCount() : -1;
    }

    /* 控件矩形（宿主窗口坐标），0 = 控件不存在。 */
    int CnchessChildRect(const char *pszName, int *pRect)
    {
        SWindow *pWnd = findChild(pszName);
        if (!pWnd)
            return 0;
        RECT rc;
        pWnd->GetWindowRect(&rc);
        if (pRect)
        {
            pRect[0] = rc.left;
            pRect[1] = rc.top;
            pRect[2] = rc.right;
            pRect[3] = rc.bottom;
        }
        return 1;
    }
}

int main()
{
    g_fbW = SwinxFbWidth();
    g_fbH = SwinxFbHeight();

    SwinxFbClear(0xFF2A1A0Eu);
    SwinxSetFbPresentCb(host_present);

#ifdef CNCHESS_WASM_CAIRO_FONTS
    /* cairo 后端：字体只来自 AddFontResource 注册的 app font（见 swinx 的
     * FontFallback.cpp —— wasm 下系统没有字体目录，fontconfig 只认 app font）。
     * 构建已把字体目录预载进 MEMFS 的 /opt/soui/fonts，这里枚举并逐个注册；
     * 注册失败（目录为空 / 文件损坏）则 g_cFontOk 仍置 1，但中文会回退成豆腐块，
     * 由下面的打印暴露"注册了几个"。 */
    {
        const char *fontDir = "/opt/soui/fonts";
        int nFont = 0;
        DIR *d = opendir(fontDir);
        if (d)
        {
            struct dirent *e;
            while ((e = readdir(d)) != NULL)
            {
                const char *name = e->d_name;
                size_t len = strlen(name);
                int isFont = (len > 4 && (!stricmp(name + len - 4, ".ttf") ||
                                          !stricmp(name + len - 4, ".ttc") ||
                                          !stricmp(name + len - 4, ".otf")));
                if (!isFont)
                    continue;
                char path[512];
                snprintf(path, sizeof(path), "%s/%s", fontDir, name);
                if (AddFontResourceA(path) > 0)
                    nFont++;
            }
            closedir(d);
        }
        g_cFontOk = (nFont > 0) ? 1 : 0;
        printf("[cnchess-wasm] font=cairo(bundled), %d file(s) registered from %s\n", nFont, fontDir);
    }
#else
    /* canvas2d 后端用浏览器内置字体渲染文本（见 readme.md 的"字体"一节），
     * 不再从 MEMFS 预载 TTF，也不调用 AddFontResource。stat 3 固定报 1，
     * 表示"文本字形由浏览器提供"，与 wasm-demo 的 route C 一致。 */
    g_cFontOk = 1;
    printf("[cnchess-wasm] font=canvas2d(browser)\n");
#endif

    /* 和演示一样：SApplication 是进程级单例，而 simulate_infinite_loop=1 会让
     * main() 以异常方式退出并展开栈，放栈上会在主循环刚开始时被析构。 */
    SApplication *app = new SApplication(GetModuleHandle(NULL));
    app->RegisterSkinClass<SSkinAni>();
    app->RegisterSkinClass<SSkinPiece>();
    app->RegisterSkinClass<SSkinBoard>();
    app->RegisterWindowClass<SGifPlayer>();
    app->RegisterWindowClass<CChessBoard>();
    app->RegisterWindowClass<CChessPiece>();
    app->RegisterWindowClass<SRatingBar>();

    SAppCfg cfg;
    /* 主题包由服务器下发（PC 版），所以 OSId 保持 Windows / pc_theme.zip 那一套，
     * 不走移动端的 mobile_theme。渲染器用 GDI：wasm 构建里没有 Skia。 */
    cfg.SetRender(Render_Gdi)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetSysResFile(kSysResDir)
        .SetAppResFile(kAppResDir)
        .SetAppDir(kAppDir)
        .SetUidefId(_T("uidef:xml_init"));
    if (!cfg.DoConfig(app))
    {
        host_fatal("SAppCfg::DoConfig() 失败：渲染器/图片解码器/资源目录三者之一没起来");
        return 1;
    }
    app->SetMessageBoxTemplateResId(_T("LAYOUT:XML_MSGBOX"));

    /* MyProfile 是 SSingleton：构造函数把自己登记成单例，桌面版在 _tWinMain 里
     * new 出来、退出前 delete。wasm 没有退出路径（主循环不返回），这里同样建一个
     * 并有意不释放——它的生命周期就是一个页面。少了它 getSingletonPtr() 是 NULL，
     * 登录成功的第一个回调就会踩空。 */
    g_pProfile = new MyProfile();

    g_pTheme.Attach(new SGameTheme());
    g_pMain = new CMainDlg(g_pTheme);

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
    /* Create 只建窗口；WM_INITDIALOG 才会跑 CMainDlg::OnInitDialog，建登录模态视图
     * 并发起连接。桌面版由 Create 内部同步发这条消息，这里显式发一次，时序更明确。 */
    g_pMain->SendMessage(WM_INITDIALOG);
    g_pMain->ShowWindow(SW_SHOWNORMAL);
    g_cReady = 1;

    printf("[cnchess-wasm] ready %dx%d\n", g_fbW, g_fbH);
    emscripten_set_main_loop(CnchessPump, 0, 1);
    return 0;
}
