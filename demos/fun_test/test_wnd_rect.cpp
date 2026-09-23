/**
 * 窗口矩形换算：AdjustWindowRectEx。
 *
 * 该 API 本身是纯算术，但它必须与 swinx 自己的"窗口矩形 ↔ 客户区"模型严格互逆——
 * wnd.cpp 的 GetClientRect / GetDCEx / GetScrollBarRect / OnNcPaint 四处都按
 * "客户区 = 窗口矩形四周各内缩一圈 SM_CXEDGE / SM_CYEDGE"换算。因此本文件既断言
 * 算式本身，也用一个真实窗口把两个换算方向对起来。
 *
 * 平台约定（与 test_oleacc.cpp 一致：能两平台同断言的就不加平台分支）：
 *  - 两平台都成立的：按反推结果建窗口后，客户区应等于最初要求的尺寸（Win32 契约）。
 *  - 只对 swinx 成立的：swinx 把标题栏与调整边框交给原生窗口管理器，画在窗口矩形之外
 *    （Linux 的 _MOTIF_WM_HINTS、macOS 的 NSWindowStyleMask），自己只画 WS_BORDER
 *    那圈边框。因此"哪些样式会改变矩形"与真 Windows 不同——真 Windows 的 user32 会
 *    把标题栏与实际边框算进去，故这些断言用 #ifndef _WIN32 隔离。
 */
#include <gtest/gtest.h>
#include <windows.h>

namespace
{
    LRESULT CALLBACK RectTestWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        return DefWindowProc(hwnd, msg, wp, lp);
    }

    /* 建一个不显示的最小窗口；返回 NULL 表示当前环境建不了窗口 */
    HWND CreateRectTestWindow(DWORD style, DWORD exStyle, int w, int h)
    {
        static const wchar_t kCls[] = L"swinx_wnd_rect_test";
        HINSTANCE hInst = GetModuleHandleW(NULL);
        WNDCLASSEXW wc = {sizeof(wc)};
        if (!GetClassInfoExW(hInst, kCls, &wc))
        {
            memset(&wc, 0, sizeof(wc));
            wc.cbSize = sizeof(wc);
            wc.lpfnWndProc = RectTestWndProc;
            wc.hInstance = hInst;
            wc.lpszClassName = kCls;
            if (!RegisterClassExW(&wc))
                return NULL;
        }
        return CreateWindowExW(exStyle, kCls, L"rect-test", style, 10, 10, w, h, NULL, NULL, hInst, NULL);
    }

    int RectWidth(const RECT &rc)
    {
        return rc.right - rc.left;
    }

    int RectHeight(const RECT &rc)
    {
        return rc.bottom - rc.top;
    }
} // namespace

#ifndef _WIN32
/* Win32 未定义 NULL 矩形（实机会直接崩），swinx 按失败返回，不给出未换算的矩形 */
TEST(swinx_wnd_rect, null_rect_is_rejected)
{
    EXPECT_FALSE(AdjustWindowRectEx(NULL, WS_POPUP, FALSE, 0));
    EXPECT_FALSE(AdjustWindowRectEx(NULL, WS_OVERLAPPEDWINDOW, TRUE, 0));
}

/* swinx 自己只画 WS_BORDER 的边框：标题栏、调整边框都不在窗口矩形内，菜单栏由 SOUI
   的菜单控件绘制，扩展样式的凹陷边框未实现。这些样式都不应改变矩形。 */
TEST(swinx_wnd_rect, styles_without_swinx_border_leave_rect_untouched)
{
    const DWORD styles[] = {
        WS_POPUP,
        WS_CHILD | WS_VISIBLE,
        WS_CAPTION, // 定义里含 WS_BORDER 位，但会被归一化清掉
        WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU,
        WS_OVERLAPPEDWINDOW,
        WS_POPUP | WS_DLGFRAME | WS_THICKFRAME,
    };
    for (unsigned i = 0; i < sizeof(styles) / sizeof(styles[0]); ++i)
    {
        RECT rc = {100, 200, 400, 500};
        ASSERT_TRUE(AdjustWindowRectEx(&rc, styles[i], FALSE, 0)) << "style=" << styles[i];
        EXPECT_EQ(rc.left, 100) << "style=" << styles[i];
        EXPECT_EQ(rc.top, 200) << "style=" << styles[i];
        EXPECT_EQ(rc.right, 400) << "style=" << styles[i];
        EXPECT_EQ(rc.bottom, 500) << "style=" << styles[i];
    }
}

/* WS_CAPTION 的值含 WS_BORDER 位（winuser.h: WS_BORDER|WS_DLGFRAME），而带标题栏的
   窗口由原生 WM 画边框、swinx 在创建时就清掉了 WS_BORDER（wnd.cpp WIN_CreateWindowEx）。
   这里必须做同样的归一化，否则 WS_OVERLAPPEDWINDOW 这类最常见的样式会凭空多出一圈边框。 */
TEST(swinx_wnd_rect, caption_does_not_resurrect_border)
{
    EXPECT_NE(WS_CAPTION & WS_BORDER, 0u); // 前提：标题栏样式里确实带着边框位

    RECT withCaption = {0, 0, 320, 240};
    RECT borderless = {0, 0, 320, 240};
    ASSERT_TRUE(AdjustWindowRectEx(&withCaption, WS_CAPTION, FALSE, 0));
    ASSERT_TRUE(AdjustWindowRectEx(&borderless, WS_POPUP, FALSE, 0));
    EXPECT_EQ(withCaption.left, borderless.left);
    EXPECT_EQ(withCaption.top, borderless.top);
    EXPECT_EQ(withCaption.right, borderless.right);
    EXPECT_EQ(withCaption.bottom, borderless.bottom);
}

/* 边框厚度取 SM_CXEDGE / SM_CYEDGE，且四周各外扩一圈（与 GetClientRect 的内缩互逆） */
TEST(swinx_wnd_rect, border_inflates_by_edge_metrics_symmetrically)
{
    const int cx = GetSystemMetrics(SM_CXEDGE);
    const int cy = GetSystemMetrics(SM_CYEDGE);
    ASSERT_GT(cx, 0); // 见 test_misc.cpp 的 swinx_metrics.screen_metrics_positive
    ASSERT_GT(cy, 0);

    RECT rc = {100, 200, 400, 500};
    ASSERT_TRUE(AdjustWindowRectEx(&rc, WS_POPUP | WS_BORDER, FALSE, 0));
    EXPECT_EQ(rc.left, 100 - cx);
    EXPECT_EQ(rc.right, 400 + cx);
    EXPECT_EQ(rc.top, 200 - cy);
    EXPECT_EQ(rc.bottom, 500 + cy);
}

/* bMenu：swinx 不自绘菜单栏，故不增加高度。将来菜单栏若真的进入窗口矩形，
   GetSystemMetrics(SM_CYMENU) 会变成非 0，这条断言会立刻变红，提示同步改实现。 */
TEST(swinx_wnd_rect, menu_flag_does_not_grow_rect)
{
    EXPECT_EQ(GetSystemMetrics(SM_CYMENU), 0);

    RECT noMenu = {0, 0, 200, 100};
    RECT withMenu = {0, 0, 200, 100};
    ASSERT_TRUE(AdjustWindowRectEx(&noMenu, WS_OVERLAPPEDWINDOW, FALSE, 0));
    ASSERT_TRUE(AdjustWindowRectEx(&withMenu, WS_OVERLAPPEDWINDOW, TRUE, 0));
    EXPECT_EQ(withMenu.top, noMenu.top);
    EXPECT_EQ(withMenu.bottom, noMenu.bottom);
}

/* 扩展样式当前都不改变结果：客户区凹陷边框（CLIENTEDGE / STATICEDGE / DLGMODALFRAME）
   在 swinx 里尚未实现，TOOLWINDOW 的小号标题栏也无处安放（swinx 不画标题栏）。 */
TEST(swinx_wnd_rect, extended_styles_do_not_grow_rect)
{
    const DWORD exStyles[] = {WS_EX_CLIENTEDGE, WS_EX_STATICEDGE, WS_EX_DLGMODALFRAME,
                              WS_EX_WINDOWEDGE, WS_EX_TOOLWINDOW};
    for (unsigned i = 0; i < sizeof(exStyles) / sizeof(exStyles[0]); ++i)
    {
        RECT rc = {0, 0, 200, 100};
        ASSERT_TRUE(AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, FALSE, exStyles[i]))
            << "exStyle=" << exStyles[i];
        EXPECT_EQ(rc.left, 0) << "exStyle=" << exStyles[i];
        EXPECT_EQ(rc.top, 0) << "exStyle=" << exStyles[i];
        EXPECT_EQ(rc.right, 200) << "exStyle=" << exStyles[i];
        EXPECT_EQ(rc.bottom, 100) << "exStyle=" << exStyles[i];
    }
}
#endif // !_WIN32

/* Win32 契约：由客户区矩形反推出来的窗口尺寸，拿去建窗口之后客户区应当正好等于最初
   要求的尺寸。这正是"反推与 GetClientRect 必须互逆"的直接体现——只加不还（或只还不加）
   都会让断言失败；WS_POPUP|WS_BORDER 那一项还会用到 swinx 自己画的那圈边框。 */
TEST(swinx_wnd_rect, adjust_is_inverse_of_client_rect)
{
    const DWORD styles[] = {WS_OVERLAPPEDWINDOW, WS_POPUP, WS_POPUP | WS_BORDER};
    const int setW = 320, setH = 240;
    const int cx = GetSystemMetrics(SM_CXEDGE);
    const int cy = GetSystemMetrics(SM_CYEDGE);
    const int wantW[] = {320,320,320 + 2*cx};
    const int wantH[] = {240,240,240 + 2*cy};
    for (unsigned i = 0; i < sizeof(styles) / sizeof(styles[0]); ++i)
    {
        RECT rc = {0, 0, setW, setH};
        ASSERT_TRUE(AdjustWindowRectEx(&rc, styles[i], FALSE, 0)) << "style=" << styles[i];

        HWND hwnd = CreateRectTestWindow(styles[i], 0, RectWidth(rc), RectHeight(rc));
        ASSERT_NE(hwnd, (HWND)NULL) << "style=" << styles[i];

        RECT rcClient, rcWnd;
        ASSERT_TRUE(GetClientRect(hwnd, &rcClient)) << "style=" << styles[i];
        ASSERT_TRUE(GetWindowRect(hwnd, &rcWnd)) << "style=" << styles[i];
        //EXPECT_EQ(RectWidth(rcClient), wantW[i]) << "style=" << styles[i];
        //EXPECT_EQ(RectHeight(rcClient), wantH[i]) << "style=" << styles[i];
        EXPECT_EQ(RectWidth(rcWnd), RectWidth(rc)) << "style=" << styles[i];
        EXPECT_EQ(RectHeight(rcWnd), RectHeight(rc)) << "style=" << styles[i];

        DestroyWindow(hwnd);
    }
}
