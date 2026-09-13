// test_multimon.cpp — swinx 多显示器 API 回归测试
//
// 覆盖：EnumDisplayMonitors / GetMonitorInfo / MonitorFromPoint /
//       MonitorFromRect / MonitorFromWindow / EnumDisplayDevices。
// Windows 上直接对真实 Win32 API 跑（作为 swinx 语义实现的基准参照）；
// 非 Windows 上对 swinx 兼容层跑。无显示连接的环境（无 DISPLAY、无 GUI
// 会话）下相关用例自动跳过，不作为失败。
#include <windows.h>
#ifndef _WIN32
#include <multimon.h> // swinx 提供的 Win32 兼容头（Windows 上用系统 SDK）
#endif
#include <gtest/gtest.h>

// 兼容旧版 gtest（无 GTEST_SKIP）：跳过按成功处理并立即结束用例
#define MULTIMON_SKIP(reason)  \
    do                         \
    {                          \
        SUCCEED() << reason;   \
        return;                \
    } while (0)

namespace
{

struct EnumCount
{
    int n;
    int validRects;
};

static BOOL CALLBACK countMonProc(HMONITOR, HDC, LPRECT rc, LPARAM lp)
{
    EnumCount *p = (EnumCount *)lp;
    p->n++;
    if (rc && rc->right > rc->left && rc->bottom > rc->top)
        p->validRects++;
    return TRUE;
}

static bool hasDisplay()
{
    EnumCount ec = {0, 0};
    if (!EnumDisplayMonitors(NULL, NULL, countMonProc, (LPARAM)&ec))
        return false;
    return ec.n >= 1;
}

static HMONITOR getPrimary()
{
    POINT pt = {0, 0};
    return MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
}

} // namespace

// 枚举至少一台显示器，且回调矩形非空
TEST(swinx_multimon, enum_display_monitors)
{
    if (!hasDisplay())
        MULTIMON_SKIP("no display connection");
    EnumCount ec = {0, 0};
    ASSERT_TRUE(EnumDisplayMonitors(NULL, NULL, countMonProc, (LPARAM)&ec));
    EXPECT_GE(ec.n, 1);
    EXPECT_EQ(ec.validRects, ec.n);
}

// 裁剪矩形远离桌面时不枚举任何显示器
TEST(swinx_multimon, enum_display_monitors_clip)
{
    if (!hasDisplay())
        MULTIMON_SKIP("no display connection");
    RECT rcFar = {-100000, -100000, -99000, -99000};
    EnumCount ec = {0, 0};
    EXPECT_TRUE(EnumDisplayMonitors(NULL, &rcFar, countMonProc, (LPARAM)&ec));
    EXPECT_EQ(ec.n, 0);
}

// GetMonitorInfo：主显示器几何有效、带 PRIMARY 标志、工作区不超出显示器
TEST(swinx_multimon, get_monitor_info_primary)
{
    HMONITOR hMon = getPrimary();
    if (!hMon)
        MULTIMON_SKIP("no display connection");
    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    ASSERT_TRUE(GetMonitorInfoA(hMon, &mi));
    EXPECT_GT(mi.rcMonitor.right, mi.rcMonitor.left);
    EXPECT_GT(mi.rcMonitor.bottom, mi.rcMonitor.top);
    EXPECT_GE(mi.rcWork.right, mi.rcMonitor.left);
    EXPECT_LE(mi.rcWork.bottom, mi.rcMonitor.bottom);
    EXPECT_TRUE(mi.dwFlags & MONITORINFOF_PRIMARY);
    // cbSize 不合法时必须拒绝
    MONITORINFO miBad = mi;
    miBad.cbSize = 1;
    EXPECT_FALSE(GetMonitorInfoA(hMon, &miBad));
}

// MonitorFromPoint 标志语义：桌面内命中；桌面外按标志返回 NULL/最近/主屏
TEST(swinx_multimon, monitor_from_point_flags)
{
    if (!hasDisplay())
        MULTIMON_SKIP("no display connection");
    HMONITOR hPrimary = getPrimary();
    ASSERT_NE(hPrimary, (HMONITOR)NULL);
    // 原点在主屏内（主屏左上角即 (0,0)）
    POINT pt0 = {0, 0};
    EXPECT_EQ(MonitorFromPoint(pt0, MONITOR_DEFAULTTONULL), hPrimary);
    // 桌面外的点
    POINT ptFar = {-100000, -100000};
    EXPECT_EQ(MonitorFromPoint(ptFar, MONITOR_DEFAULTTONULL), (HMONITOR)NULL);
    EXPECT_NE(MonitorFromPoint(ptFar, MONITOR_DEFAULTTONEAREST), (HMONITOR)NULL);
    EXPECT_EQ(MonitorFromPoint(ptFar, MONITOR_DEFAULTTOPRIMARY), hPrimary);
}

// MonitorFromRect：主屏中心的矩形必命中；远处矩形 TONULL 返回 NULL
TEST(swinx_multimon, monitor_from_rect)
{
    HMONITOR hMon = getPrimary();
    if (!hMon)
        MULTIMON_SKIP("no display connection");
    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    ASSERT_TRUE(GetMonitorInfoA(hMon, &mi));
    int cx = (mi.rcMonitor.left + mi.rcMonitor.right) / 2;
    int cy = (mi.rcMonitor.top + mi.rcMonitor.bottom) / 2;
    RECT rcCenter = {cx - 10, cy - 10, cx + 10, cy + 10};
    EXPECT_NE(MonitorFromRect(&rcCenter, MONITOR_DEFAULTTONULL), (HMONITOR)NULL);
    RECT rcFar = {-100000, -100000, -99000, -99000};
    EXPECT_EQ(MonitorFromRect(&rcFar, MONITOR_DEFAULTTONULL), (HMONITOR)NULL);
    // 跨屏大矩形至少命中一台
    RECT rcHuge = {-50000, -50000, 50000, 50000};
    EXPECT_NE(MonitorFromRect(&rcHuge, MONITOR_DEFAULTTONULL), (HMONITOR)NULL);
}

// MonitorFromWindow：NULL 窗口按标志语义；有效显示器句柄信息可查
TEST(swinx_multimon, monitor_from_window_null_semantics)
{
    if (!hasDisplay())
        MULTIMON_SKIP("no display connection");
    HMONITOR hPrimary = getPrimary();
    ASSERT_NE(hPrimary, (HMONITOR)NULL);
    EXPECT_EQ(MonitorFromWindow(NULL, MONITOR_DEFAULTTONULL), (HMONITOR)NULL);
    EXPECT_EQ(MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY), hPrimary);
}

// EnumDisplayDevices：至少一台设备，名称非空
TEST(swinx_multimon, enum_display_devices)
{
    DISPLAY_DEVICEA dd = {};
    dd.cb = sizeof(dd);
    if (!EnumDisplayDevicesA(NULL, 0, &dd, 0))
        MULTIMON_SKIP("no display connection");
    EXPECT_NE(dd.DeviceName[0], '\0');
    EXPECT_TRUE(dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP);
    // 越界索引返回 FALSE
    DISPLAY_DEVICEA dd2 = {};
    dd2.cb = sizeof(dd2);
    EXPECT_FALSE(EnumDisplayDevicesA(NULL, 1000, &dd2, 0));
}

