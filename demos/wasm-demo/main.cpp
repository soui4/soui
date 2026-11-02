/*
 * main.cpp -- swinx WebAssembly 裸窗口演示（阶段 0：只依赖 swinx，不依赖 SOUI 上层）
 *
 * 这个文件做的事和 src/platform/wasm/ 三块拼图的对应关系：
 *
 *   1) 窗口与消息：RegisterClassA/CreateWindowExA 建一个 WS_POPUP 窗口，
 *      窗口矩形 = 帧缓冲矩形，因此 client 坐标 == 屏幕坐标 == canvas 像素坐标，
 *      JS 宿主不需要再做一次原点换算。消息泵只用 PeekMessage + TranslateMessage +
 *      DispatchMessage，这仍然能拿到 WM_TIMER：wasm/SConnection::peekMsg() 会先调
 *      updateMsgQueue()（见 src/platform/wasm/SConnection.cpp:485），与 GetMessage
 *      路径同源。
 *   2) 绘制：WM_PAINT 里 BeginPaint/EndPaint 包住一组 GDI 调用（FillRect、
 *      Rectangle、Ellipse、RoundRect、FrameRect、MoveToEx/LineTo），落到 cairo
 *      image surface；EndPaint -> ReleaseDC -> commitCanvas() 把脏矩形内的像素
 *      拷进帧缓冲，并把脏矩形交给 SwinxSetFbPresentCb() 注册的回调。
 *      commitCanvas() 用上屏范围 = DC 裁剪框 ∩ 窗口更新区，所以失效区域要刻意选小
 *      （指针点 24x24、动画条带 300x250、标题带一条），页面面板上的"节省比例"
 *      就是相对整帧拷贝算出来的。
 *   3) 输入：JS 宿主的 DOM 事件经 SwinxInjectMouse/SwinxInjectKey 投进目标窗口
 *      所在线程的队列。这两个入口收的是屏幕坐标，内部按 wndObj->rc 换算 client。
 *      注意第一条 WM_MOUSEMOVE 会被 swinx 的 hover 进入/离开状态机吞掉
 *      （src/wnd.cpp 的 WM_MOUSEMOVE 分支在 htCode 首次变成 HTCLIENT 时置
 *      bSkipMsg），这是 Win32 语义，不是丢消息。
 *
 * 关于文字：本演示不画任何文本。非 wasm 平台的字形走 fontconfig，而浏览器里
 * 既没有 /etc/fonts 也没有系统字体目录，TextOut 只会得到空白；真实字形需要
 * 阶段 2 用 --preload-file 预置 TTF。这里用形状 + JS 侧数据面板代替文字说明。
 *
 * 帧缓冲像素是 cairo CAIRO_FORMAT_ARGB32：内存字节序 B,G,R,A（预乘 alpha），
 * 小端读成 uint32 即 0xAARRGGBB。Canvas ImageData 要的是 R,G,B,A 直色，换算在
 * JS 侧做（见 index.html）。本演示所有像素 alpha=255，预乘与直色等价。
 */
#include <emscripten.h>
#include <math.h>
#include <string.h>

#include <windows.h>

#include "framebuffer.h"
#include "input.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const char kWndClass[] = "SwinxWasmDemo";

/* 帧缓冲尺寸在编译期烘进 libswinx.a（swinx/wasm.cmake 的 SWINX_FB_WIDTH/HEIGHT
 * 缓存变量），所以这里一律用运行期取值，不写死。 */
static int g_fbW = 0;
static int g_fbH = 0;

static HWND  g_hWnd = NULL;
static int   g_pumpCount = 0;
static int   g_paintCount = 0;
static int   g_timerCount = 0;
static int   g_mouseCount = 0;
static int   g_moveCount = 0;
static int   g_keyCount = 0;
static int   g_charCount = 0;
static int   g_band = 0;      /* 标题带配色序号，WM_KEYDOWN 循环切换 */
static POINT g_cursor = { 0, 0 };
static POINT g_box = { 0, 0 }; /* 方向键移动的小方块，演示两块脏矩形 */
static POINT g_marks[24];      /* 左键留下的标记环 */
static int   g_markCount = 0;

/* COLORREF（swinx 里是 0xAABBGGRR）转帧缓冲的 0xAARRGGBB。
 * 只有直接写帧缓冲时才需要，GDI 调用一律用 RGB()。 */
static uint32_t fb_pixel(COLORREF c)
{
    unsigned int r = GetRValue(c);
    unsigned int g = GetGValue(c);
    unsigned int b = GetBValue(c);
    unsigned int a = GetAValue(c);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

static const COLORREF kBandColors[] = {
    RGB(0x2E, 0x86, 0xC7),
    RGB(0xD3, 0x54, 0x3A),
    RGB(0x3F, 0xA9, 0x67),
    RGB(0x8E, 0x44, 0xAD),
};

/* 把 rect 夹到 client 区域内再失效；rect 为 NULL 表示整窗。 */
static void invalidate_clipped(HWND hWnd, const RECT *rect)
{
    RECT client;
    GetClientRect(hWnd, &client);
    if (!rect)
    {
        InvalidateRect(hWnd, NULL, TRUE);
        return;
    }
    RECT r = *rect;
    if (r.left < client.left) r.left = client.left;
    if (r.top < client.top) r.top = client.top;
    if (r.right > client.right) r.right = client.right;
    if (r.bottom > client.bottom) r.bottom = client.bottom;
    if (r.left < r.right && r.top < r.bottom)
        InvalidateRect(hWnd, &r, TRUE);
}

/* 以 pt 为中心、半径 rad 的方形失效区，用来把脏矩形控制在很小的一片。 */
static void invalidate_around(HWND hWnd, POINT pt, int rad)
{
    RECT r = { pt.x - rad, pt.y - rad, pt.x + rad, pt.y + rad };
    invalidate_clipped(hWnd, &r);
}

/* --- 绘制 ---------------------------------------------------------------- */

static void draw_border(HDC hdc)
{
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(0xE6, 0xEC, 0xF5));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, 1, 1, g_fbW - 1, g_fbH - 1);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBr);
    DeleteObject(pen);
}

static void draw_band(HDC hdc)
{
    RECT band = { 2, 2, g_fbW - 2, 58 };
    HBRUSH br = CreateSolidBrush(kBandColors[g_band % 4]);
    FillRect(hdc, &band, br);
    DeleteObject(br);

    /* 带内一组同心椭圆：验证弧线抗锯齿与描边路径 */
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(0xFF, 0xFF, 0xFF));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    int cx = 40, cy = 30;
    for (int rad = 6; rad <= 24; rad += 6)
        Ellipse(hdc, cx - rad, cy - rad, cx + rad, cy + rad);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBr);
    DeleteObject(pen);
}

static void draw_meter(HDC hdc)
{
    /* 进度槽用 FrameRect（只描边），内部按 g_timerCount 的比例填充 */
    RECT slot = { 24, 74, 324, 102 };
    HBRUSH frame = CreateSolidBrush(RGB(0x8A, 0x9B, 0xB0));
    FrameRect(hdc, &slot, frame);
    DeleteObject(frame);

    int percent = (g_timerCount % 101);
    RECT fill = slot;
    fill.left += 2;
    fill.top += 2;
    fill.right -= 2;
    fill.bottom -= 2;
    fill.right = fill.left + (fill.right - fill.left) * percent / 100;
    if (fill.right > fill.left)
    {
        HBRUSH br = CreateSolidBrush(RGB(0x5F, 0xD1, 0x8A));
        FillRect(hdc, &fill, br);
        DeleteObject(br);
    }
}

static void draw_spinner(HDC hdc)
{
    int cx = 174, cy = 236, rad = 88;
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(0xE6, 0xEC, 0xF5));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Ellipse(hdc, cx - rad, cy - rad, cx + rad, cy + rad);

    double phase = (g_timerCount % 60) * (2.0 * M_PI / 60.0);
    for (int spoke = 0; spoke < 12; ++spoke)
    {
        double a = phase + spoke * (2.0 * M_PI / 12.0);
        int inner = spoke % 3 == 0 ? 24 : 52;
        MoveToEx(hdc, cx + (int)(inner * cos(a)), cy + (int)(inner * sin(a)), NULL);
        LineTo(hdc, cx + (int)((rad - 4) * cos(a)), cy + (int)((rad - 4) * sin(a)));
    }

    /* 指针：一条从圆心出发的线，随 WM_TIMER 转动 */
    MoveToEx(hdc, cx, cy, NULL);
    LineTo(hdc, cx + (int)((rad - 12) * cos(phase)), cy + (int)((rad - 12) * sin(phase)));

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBr);
    DeleteObject(pen);
}

static void draw_shapes(HDC hdc)
{
    /* 右半边：圆角矩形 + 实心椭圆 + 空心矩形，三种笔刷组合 */
    HPEN pen = CreatePen(PS_SOLID, 3, RGB(0xF7, 0xC7, 0x4B));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH br = CreateSolidBrush(RGB(0x3A, 0x4A, 0x63));
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);

    RoundRect(hdc, 372, 74, 604, 190, 28, 28);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBr);

    HPEN pen2 = CreatePen(PS_SOLID, 2, RGB(0x9A, 0xC8, 0xFF));
    oldPen = (HPEN)SelectObject(hdc, pen2);
    HBRUSH br2 = CreateSolidBrush(RGB(0xC0, 0x39, 0x6E));
    oldBr = (HBRUSH)SelectObject(hdc, br2);
    Ellipse(hdc, 392, 214, 520, 320);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBr);

    HPEN pen3 = CreatePen(PS_SOLID, 1, RGB(0xE6, 0xEC, 0xF5));
    oldPen = (HPEN)SelectObject(hdc, pen3);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, 552, 214, 720, 320);
    /* 折线：GDI 的当前点在 LineTo 后前移，这里连成一个开放多边形 */
    MoveToEx(hdc, 562, 306, NULL);
    LineTo(hdc, 602, 236);
    LineTo(hdc, 642, 296);
    LineTo(hdc, 682, 226);
    LineTo(hdc, 712, 300);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBr);
    DeleteObject(pen);
    DeleteObject(pen2);
    DeleteObject(pen3);
    DeleteObject(br);
    DeleteObject(br2);
}

static void draw_marks(HDC hdc)
{
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(0xFF, 0xFF, 0xFF));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    for (int i = 0; i < g_markCount; ++i)
    {
        int x = g_marks[i].x, y = g_marks[i].y;
        MoveToEx(hdc, x - 9, y, NULL);
        LineTo(hdc, x + 10, y);
        MoveToEx(hdc, x, y - 9, NULL);
        LineTo(hdc, x, y + 10);
        Ellipse(hdc, x - 8, y - 8, x + 8, y + 8);
    }
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBr);
    DeleteObject(pen);
}

static void draw_box(HDC hdc)
{
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(0x11, 0x11, 0x11));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH br = CreateSolidBrush(RGB(0xF0, 0xE8, 0xD8));
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
    RoundRect(hdc, g_box.x, g_box.y, g_box.x + 36, g_box.y + 36, 10, 10);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBr);
    DeleteObject(pen);
    DeleteObject(br);
}

static void draw_cursor(HDC hdc)
{
    /* 用画笔颜色本身画一小段十字，避免为指针再申请刷子 */
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(0x10, 0x10, 0x10));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    int x = g_cursor.x, y = g_cursor.y;
    MoveToEx(hdc, x - 10, y, NULL);
    LineTo(hdc, x + 10, y);
    MoveToEx(hdc, x, y - 10, NULL);
    LineTo(hdc, x, y + 10);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

static void draw_scene(HDC hdc)
{
    RECT client;
    GetClientRect(g_hWnd, &client);
    HBRUSH bg = CreateSolidBrush(RGB(0x14, 0x18, 0x22));
    FillRect(hdc, &client, bg);
    DeleteObject(bg);

    draw_border(hdc);
    draw_band(hdc);
    draw_meter(hdc);
    draw_spinner(hdc);
    draw_shapes(hdc);
    draw_box(hdc);
    draw_marks(hdc);
    draw_cursor(hdc);
}

/* --- 窗口过程 ------------------------------------------------------------ */

static LRESULT CALLBACK DemoWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        /* 50ms：一帧浏览器刷新约 16ms，泵每帧最多取空一次队列，因此 WM_TIMER
         * 的间隔是"至少 50ms"而不是精确 50ms，这与 Win32 的语义一致。 */
        SetTimer(hWnd, 1, 50, NULL);
        return 0;

    case WM_SIZE:
        /* WS_POPUP + 无 NC 区：窗口矩形即画布矩形，无需额外处理 */
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        draw_scene(hdc);
        EndPaint(hWnd, &ps); /* -> ReleaseDC -> commitCanvas -> 上屏回调 */
        ++g_paintCount;
        return 0;
    }

    case WM_TIMER:
        if (wParam == 1)
        {
            ++g_timerCount;
            /* 只让动画带失效：进度条 (24,74)-(324,102) 与转盘 (86,148)-(262,324)
             * 合成一条脏矩形，右半边的静态图形不参与重绘。 */
            RECT anim = { 24, 74, 324, 324 };
            invalidate_clipped(hWnd, &anim);
        }
        return 0;

    case WM_MOUSEMOVE:
    {
        POINT prev = g_cursor;
        g_cursor.x = GET_X_LPARAM(lParam);
        g_cursor.y = GET_Y_LPARAM(lParam);
        ++g_moveCount;
        invalidate_around(hWnd, prev, 12);
        invalidate_around(hWnd, g_cursor, 12);
        return 0;
    }

    case WM_LBUTTONDOWN:
        ++g_mouseCount;
        g_cursor.x = GET_X_LPARAM(lParam);
        g_cursor.y = GET_Y_LPARAM(lParam);
        if (g_markCount < (int)(sizeof(g_marks) / sizeof(g_marks[0])))
        {
            g_marks[g_markCount].x = g_cursor.x;
            g_marks[g_markCount].y = g_cursor.y;
            ++g_markCount;
        }
        else
        {
            /* 环形复用：整体重绘一次，代价是一次全窗脏矩形 */
            memmove(g_marks, g_marks + 1, sizeof(g_marks[0]) * (g_markCount - 1));
            g_marks[g_markCount - 1].x = g_cursor.x;
            g_marks[g_markCount - 1].y = g_cursor.y;
            invalidate_clipped(hWnd, NULL);
            return 0;
        }
        invalidate_around(hWnd, g_cursor, 12);
        return 0;

    case WM_RBUTTONDOWN:
    {
        ++g_mouseCount;
        g_band = (g_band + 1) % 4;
        RECT band = { 2, 2, g_fbW - 2, 58 };
        invalidate_clipped(hWnd, &band);
        return 0;
    }

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        ++g_keyCount;
        switch (wParam)
        {
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
        {
            POINT prev = g_box;
            int dx = (wParam == VK_RIGHT) - (wParam == VK_LEFT);
            int dy = (wParam == VK_DOWN) - (wParam == VK_UP);
            g_box.x += dx * 16;
            g_box.y += dy * 16;
            invalidate_around(hWnd, prev, 30);
            invalidate_around(hWnd, g_box, 30);
            break;
        }
        default:
        {
            g_band = (g_band + 1) % 4;
            RECT band = { 2, 2, g_fbW - 2, 58 };
            invalidate_clipped(hWnd, &band);
            break;
        }
        }
        return 0;

    case WM_CHAR:
        ++g_charCount; /* 由核心 TranslateMessage() 从 ASCII 按键生成 */
        return 0;

    case WM_DESTROY:
        KillTimer(hWnd, 1);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hWnd, message, wParam, lParam);
}

/* --- 上屏回调：commitCanvas 之后被调用，脏矩形是屏幕坐标 ---------------- */

static void demo_present(const RECT *dirty)
{
    if (!dirty)
        return;
    /* 直接把脏矩形交给 JS 宿主；JS 侧负责 ARGB32->RGBA 换算与 putImageData。
     * 用 EM_ASM 而不是返回值轮询，是为了让"一次 EndPaint = 一次上屏"这个因果
     * 关系在页面上可见。 */
    EM_ASM({
        if (typeof Module.swinxPresent === 'function')
            Module.swinxPresent($0, $1, $2, $3);
    }, dirty->left, dirty->top, dirty->right, dirty->bottom);
}

/* --- JS 宿主可调用的入口（在 build.sh 里用 -sEXPORTED_FUNCTIONS 导出） --- */

extern "C"
{
    /* 统计量：JS 侧按下标轮询，写进页面面板。0..7 的含义见 index.html。 */
    int SwinxDemoStat(int index)
    {
        switch (index)
        {
        case 0: return g_pumpCount;
        case 1: return g_paintCount;
        case 2: return g_timerCount;
        case 3: return g_mouseCount;
        case 4: return g_keyCount;
        case 5: return g_charCount;
        case 6: return g_moveCount;
        case 7: return (int)GetTickCount();
        default: return 0;
        }
    }

    /* 消息泵：由 emscripten_set_main_loop 以 requestAnimationFrame 频率驱动。 */
    void SwinxDemoPump(void)
    {
        MSG msg;
        ++g_pumpCount;
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    /* 指针事件：(x, y) 是屏幕坐标，即 canvas 像素坐标——窗口在 (0,0) 且尺寸
     * 等于帧缓冲，所以 JS 侧不需要再加偏移。message 用 Win32 的消息号。 */
    void SwinxDemoMouse(UINT message, int x, int y, WPARAM fwKeys)
    {
        SwinxInjectMouse(g_hWnd, message, x, y, fwKeys);
    }

    /* 键盘事件：vk 由 JS 侧从 DOM KeyboardEvent 映射成 Win32 虚拟键码。 */
    void SwinxDemoKey(UINT message, UINT vk, UINT scanCode)
    {
        SwinxInjectKey(g_hWnd, message, vk, scanCode);
    }

    /* 全窗重绘：宿主在 canvas 尺寸变化、页面从后台恢复或自己搞脏了画面时调用。
     * 这是"重新 attach 一块显示"的最小动作——帧缓冲里的像素仍在，直接整帧 blit
     * 一次也能恢复，但走 InvalidateRect 才符合 Win32 的重绘习惯。 */
    void SwinxDemoRepaint(void)
    {
        if (g_hWnd)
            InvalidateRect(g_hWnd, NULL, TRUE);
    }
}

int main()
{
    g_fbW = SwinxFbWidth();
    g_fbH = SwinxFbHeight();

    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = 0; /* CS_OWNDC 等样式在本端口无人解析，留 0 以免被当成已支持 */
    wc.lpfnWndProc = DemoWndProc;
    wc.hInstance = (HINSTANCE)0x1000; /* 本端口不校验实例句柄，见 readme 的已知边界 */
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    /* 类背景刷由 clsmgr 拥有并在类注销时 DeleteObject，所以不能传 GetStockObject
     * 的共享刷子；传 COLORREF 索引也合法（IS_INTRESOURCE 分支会转成系统色刷）。 */
    wc.hbrBackground = CreateSolidBrush(RGB(0x14, 0x18, 0x22));
    wc.lpszClassName = kWndClass;
    if (!RegisterClassA(&wc))
        return 1;

    /* 先把背景刷成一色，再挂上屏回调，最后建窗：顺序反过来的话 CreateWindow 里
     * 那次同步绘制会因为没有回调而丢帧。 */
    SwinxFbClear(fb_pixel(RGB(0x14, 0x18, 0x22)));
    SwinxSetFbPresentCb(demo_present);

    g_box.x = g_fbW - 60;
    g_box.y = g_fbH - 60;
    g_cursor.x = g_fbW / 2;
    g_cursor.y = g_fbH / 2;

    g_hWnd = CreateWindowExA(0, kWndClass, "swinx wasm demo", WS_POPUP | WS_VISIBLE,
                             0, 0, g_fbW, g_fbH, NULL, NULL, wc.hInstance, NULL);
    if (!g_hWnd)
        return 2;

    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd); /* 同步走一次 WM_PAINT，首帧不依赖 rAF */

    /* fps=0 表示跟随 requestAnimationFrame；simulate_infinite_loop=1 让 main()
     * 以"主循环已接管"的方式退出，JS 侧的 Promise 仍会 resolve。 */
    emscripten_set_main_loop(SwinxDemoPump, 0, 1);
    return 0;
}
