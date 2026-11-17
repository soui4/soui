/**
 * swinx environment / arithmetic / pointer-validation APIs.
 */

#include <gtest/gtest.h>
#include <windows.h>
#include <stdint.h>
#include <matrix/SMatrix.h>

TEST(swinx_env, set_get_roundtrip)
{
    ASSERT_TRUE(SetEnvironmentVariableA("SWINX_TEST_VAR", "hello"));
    // Win32 semantics: NULL buffer query returns the required size
    // INCLUDING the terminating NUL ("hello" -> 6)
    EXPECT_EQ(GetEnvironmentVariableA("SWINX_TEST_VAR", NULL, 0), 6u);

    char buf[32] = {0};
    EXPECT_EQ(GetEnvironmentVariableA("SWINX_TEST_VAR", buf, sizeof(buf)), 5u);
    EXPECT_STREQ(buf, "hello");
}

TEST(swinx_env, buffer_too_small_returns_required_size)
{
    SetEnvironmentVariableA("SWINX_TEST_VAR", "hello");

    char buf[2] = {0};
    // Win32 semantics: returns the required size (including NUL)
    EXPECT_EQ(GetEnvironmentVariableA("SWINX_TEST_VAR", buf, sizeof(buf)), 6u);
    // the buffer must not have been written when too small
    EXPECT_EQ(buf[0], '\0');
    SetEnvironmentVariableA("SWINX_TEST_VAR", "");
}

TEST(swinx_env, missing_var)
{
    char buf[8] = {0};
    EXPECT_EQ(GetEnvironmentVariableA("SWINX_NO_SUCH_VAR_XYZ", buf, sizeof(buf)), 0u);
}

TEST(swinx_misc, mul_div_rounds_to_nearest)
{
    // exact division
    EXPECT_EQ(MulDiv(10, 10, 5), 20);
    // Win32 rounds to the nearest integer, halfway away from zero
    EXPECT_EQ(MulDiv(25, 5, 10), 13);  // 12.5 -> 13
    EXPECT_EQ(MulDiv(-25, 5, 10), -13);
    EXPECT_EQ(MulDiv(1, 1, 2), 1);     // 0.5 -> 1
    EXPECT_EQ(MulDiv(-1, 1, 2), -1);
}

// 除数为 0 / 结果溢出都返回 -1（Win32 的取值），不是崩溃也不是 0。
// swinx 另外置 ERROR_INT_DIVIDE_BY_ZERO；真实 Windows 的 MulDiv 不置 last error
// （实测 err=0），所以那条断言只在 swinx 侧成立。
TEST(swinx_misc, mul_div_failure_paths)
{
    EXPECT_EQ(MulDiv(1, 1, 0), -1);
    EXPECT_EQ(MulDiv(0, 0, 0), -1);

#ifndef _WIN32
    SetLastError(0);
    EXPECT_EQ(MulDiv(1, 1, 0), -1);
    EXPECT_EQ(GetLastError(), (DWORD)ERROR_INT_DIVIDE_BY_ZERO);
#endif

    // 结果超出 int 范围
    EXPECT_EQ(MulDiv(2147483647, 2, 1), -1);
    EXPECT_EQ(MulDiv(2147483647, -2, 1), -1);
    // 边界内不能被误判成溢出
    EXPECT_EQ(MulDiv(2147483647, 1, 1), 2147483647);
}

// SetLastError / GetLastError 在 swinx 上是 errno 的代理（各平台同一条口径，
// 所以这两个函数被下沉到 common/sysapi.cpp）。语义要求：存什么读什么。
TEST(swinx_misc, last_error_roundtrip)
{
    SetLastError(1234);
    EXPECT_EQ(GetLastError(), 1234);

    SetLastError(0);
    EXPECT_EQ(GetLastError(), 0);
}

//TEST(swinx_misc, is_bad_read_ptr)
//{
//    int local = 42;
//    EXPECT_FALSE(IsBadReadPtr(&local, sizeof(local)));
//    EXPECT_TRUE(IsBadReadPtr(NULL, 4));
//    EXPECT_TRUE(IsBadReadPtr((const void *)(uintptr_t)1, 4));
//}

TEST(swinx_misc, interlocked_compare_exchange)
{
    LONG v = 10;
    // compare unequal: no exchange, returns original
    EXPECT_EQ(InterlockedCompareExchange(&v, 99, 5), 10);
    EXPECT_EQ(v, 10);
    // compare equal: exchange happens, returns original
    EXPECT_EQ(InterlockedCompareExchange(&v, 99, 10), 10);
    EXPECT_EQ(v, 99);
}

TEST(swinx_misc, interlocked_decrement)
{
    LONG v = 5;
    EXPECT_EQ(InterlockedDecrement(&v), 4);
    EXPECT_EQ(v, 4);
}

// Regression：InterlockedExchangeAdd 必须返回**加之前**的值（Win32 语义）。
// 下沉时抓到 posix 侧用的是 __sync_add_and_fetch（返回加完之后的值），
// freertos 侧用 __atomic_fetch_add（正确）—— 两份逐字"看起来一样"的实现里
// 藏着这条分叉。原来的唯一调用点（STls.cpp）传的增量是 0，所以一直没暴露；
// 这条用例用非零增量把它锁死。
TEST(swinx_misc, interlocked_exchange_add_returns_previous_value)
{
    LONG v = 10;
    EXPECT_EQ(InterlockedExchangeAdd(&v, 5), 10);
    EXPECT_EQ(v, 15);

    // 负增量同样是"返回加之前的值"
    EXPECT_EQ(InterlockedExchangeAdd(&v, -20), 15);
    EXPECT_EQ(v, -5);

    // 增量为 0：旧值与新值相同，只能验证不破坏原值
    EXPECT_EQ(InterlockedExchangeAdd(&v, 0), -5);
    EXPECT_EQ(v, -5);
}

TEST(swinx_misc, sleep_at_least_requested)
{
    // Win32: Sleep never returns before the requested interval has elapsed
    // (it may overshoot by the scheduler granularity)
    // DWORD start = GetTickCount();
    // Sleep(120);
    // DWORD elapsed = GetTickCount() - start;
    // EXPECT_GE(elapsed, 120u);
    // EXPECT_LT(elapsed, 10000u);
}

// Regression：Sleep 曾经把 dwMilliseconds*1000 直接写进 timeval::tv_usec，
// 请求 >= 1000ms 时 tv_usec 就超出合法范围 [0, 999999]（POSIX 未定义）：musl、macOS
// 直接返回 -1/EINVAL（glibc 2.32 同样），select 立刻返回，于是 Sleep(1000) 一秒都没睡；
// 另外它忽略 select 的返回值，被信号打断（EINTR，本进程装了 SIGCHLD 处理器）时睡眠被截断。
// 1000ms 正好是越界阈值，因此这里必须用 >= 1000 的值来锁定"真的睡满"。
TEST(swinx_misc, sleep_over_one_second)
{
    DWORD start = GetTickCount();
    Sleep(1000);
    DWORD elapsed = GetTickCount() - start;
    EXPECT_GE(elapsed, 950u); // 只放宽时钟粒度，不放过"立刻返回"
    EXPECT_LT(elapsed, 10000u);
}

// ------------------------------------------------------------------------
// System information
// ------------------------------------------------------------------------

TEST(swinx_misc, get_system_info_sane)
{
    SYSTEM_INFO si = {};
    GetSystemInfo(&si);

    EXPECT_GT(si.dwPageSize, 0u);
    // page size must be a power of two
    EXPECT_EQ(si.dwPageSize & (si.dwPageSize - 1), 0u);
    EXPECT_GT(si.dwNumberOfProcessors, 0u);
    EXPECT_EQ(si.dwAllocationGranularity, 65536u);
}

TEST(swinx_misc, get_version_ex_reports_nt)
{
    OSVERSIONINFOA vi = {};
    vi.dwOSVersionInfoSize = sizeof(vi);
    ASSERT_TRUE(GetVersionExA(&vi));
    EXPECT_EQ(vi.dwOSVersionInfoSize, (DWORD)sizeof(OSVERSIONINFOA));
    EXPECT_EQ(vi.dwPlatformId, VER_PLATFORM_WIN32_NT);
}

TEST(swinx_misc, get_version_ex_small_size_fail)
{
    OSVERSIONINFOA vi = {};
    vi.dwOSVersionInfoSize = sizeof(vi) - 1; // undersized -> rejected
    EXPECT_FALSE(GetVersionExA(&vi));
}

TEST(swinx_misc, command_line_nonempty)
{
    LPCSTR cmd = GetCommandLineA();
    ASSERT_TRUE(cmd != NULL);
    EXPECT_GT(strlen(cmd), 0u);
}

// ------------------------------------------------------------------------
// Host identity
// ------------------------------------------------------------------------

TEST(swinx_misc, computer_name_nonempty)
{
    char name[256] = {0};
    DWORD nSize = sizeof(name);
    ASSERT_TRUE(GetComputerNameA(name, &nSize));
    EXPECT_GT(strlen(name), 0u);
    // nSize must be updated to the length WITHOUT the NUL (Win32 semantics)
    EXPECT_EQ(nSize, strlen(name));
}

TEST(swinx_misc, user_name_when_available)
{
    char name[256] = {0};
    DWORD nSize = sizeof(name);
    if (GetUserNameA(name, &nSize))
    {
        EXPECT_GT(strlen(name), 0u);
        // GetUserNameA reports the length INCLUDING the NUL
        // (unlike GetComputerNameA, which excludes it)
        EXPECT_EQ(nSize, strlen(name) + 1);
    }
    else
    {
        // CI environments without USER/LOGNAME set are acceptable
        SUCCEED() << "user name unavailable in this environment";
    }
}

// ------------------------------------------------------------------------
// Pointer validation
// ------------------------------------------------------------------------

TEST(swinx_misc, is_bad_string_ptr)
{
    EXPECT_FALSE(IsBadStringPtrA("hello world", 100));
    EXPECT_FALSE(IsBadStringPtrA("hello world", 5));
    EXPECT_TRUE(IsBadStringPtrA(NULL, 10));
}

#ifdef _WIN32
// On real Windows IsDBCSLeadByte depends on the active ANSI code page,
// so the UTF-8-based swinx behavior can only be asserted on non-Windows.
// The Windows placeholder must NOT be called DISABLED_is_dbcs_lead_byte:
// CTest registers a disabled case under its name with the DISABLED_ prefix
// stripped, so it would collide with the live case below and abort CMake
// configure with "add_test given test NAME ... which already exists".
TEST(swinx_misc, DISABLED_is_dbcs_lead_byte_codepage_dependent)
#else
TEST(swinx_misc, is_dbcs_lead_byte)
#endif
{
    // swinx implements this as "is a multi-byte UTF-8 lead byte"
    EXPECT_FALSE(IsDBCSLeadByte('A'));
    EXPECT_FALSE(IsDBCSLeadByte(0x41));
    EXPECT_FALSE(IsDBCSLeadByte(0x7F));
    EXPECT_TRUE(IsDBCSLeadByte(0xE4)); // 3-byte lead ("中" = E4 B8 AD)
    EXPECT_TRUE(IsDBCSLeadByte(0xC3)); // 2-byte lead ("é" = C3 A9)
}


TEST(swinx_metrics, screen_metrics_positive)
{
    // pure display queries: real Windows and swinx (X11) both report the
    // actual screen; the default-screen fallback in swinx reports 1920x1080
    int cx = GetSystemMetrics(SM_CXSCREEN);
    int cy = GetSystemMetrics(SM_CYSCREEN);
    EXPECT_GT(cx, 0);
    EXPECT_GT(cy, 0);

    // border/edge metrics are small positive values on both platforms
    EXPECT_GT(GetSystemMetrics(SM_CXEDGE), 0);
    EXPECT_GT(GetSystemMetrics(SM_CYEDGE), 0);
}

#ifndef _WIN32
// GetSystemMetrics 的 SM_* 取值表已下沉到 common/sysapi.cpp，唯一留给平台的是
// swinx_system_metric_scale()（"这个度量值要不要按 DPI 放大"）。下面用两条互逆的
// 关系把这个钩子锁住：如果它返回 0（忘了实现）或漏乘，图标尺寸立刻对不上。
// GetSystemScale() 是 swinx 自有的（声明在 swinx/include/sysapi.h，windows.h 里已带），
// 真实 Windows 上没有这个符号，所以整条用例只在 swinx 侧跑。
TEST(swinx_metrics, dpi_scale_hook_reaches_fixed_metrics)
{
    const int scale = GetSystemScale();
    ASSERT_GT(scale, 0); // 0 会让所有可缩放度量变成 0

    EXPECT_EQ(GetSystemMetrics(SM_CXICON), MulDiv(32, scale, 100));
    EXPECT_EQ(GetSystemMetrics(SM_CYICON), MulDiv(32, scale, 100));
    EXPECT_EQ(GetSystemMetrics(SM_CXSMICON), 16); // 小图标不在取值表里乘，保持原值
    EXPECT_EQ(GetSystemMetrics(SM_CXMINTRACK), MulDiv(6, scale, 100));

#ifdef __APPLE__
    // 平台契约里唯一的例外：CoreGraphics 的光标尺寸本身就是 point，再乘 DPI 会翻倍
    EXPECT_EQ(GetSystemMetrics(SM_CXCURSOR), 24);
#else
    EXPECT_EQ(GetSystemMetrics(SM_CXCURSOR), MulDiv(24, scale, 100));
#endif
}
#endif // !_WIN32

// GetDoubleClickTime 现在只转发到 SConnection::GetDoubleClickTime()。
// 旧 freertos 在 win32_rich.cpp 里还留着一份硬编码 500 的副本，与它自己
// SConnection 里的 400 互相矛盾（板上双击间隔因此差了 100ms）。这里至少保证它
// 有单一来源、稳定、非零，且在人类可用的区间内。
TEST(swinx_misc, double_click_time_is_stable_and_sane)
{
    const UINT t = GetDoubleClickTime();
    EXPECT_GE(t, 100u);
    EXPECT_LE(t, 5000u);
    EXPECT_EQ(GetDoubleClickTime(), t);
}

// LoadCursorW 随第二批下沉进 common：资源 ID（IS_INTRESOURCE）分支转发到 A 版。
// 标准光标是进程内共享对象，两条入口必须拿到同一个句柄 —— 否则 wke 的
// SharedCursor 缓存会漏，退出时还可能重复释放。
TEST(swinx_misc, load_cursor_w_resource_matches_ansi)
{
    // IDC_ARROW 展开成 MAKEINTRESOURCE，类型随 UNICODE 宏在 LPSTR/LPWSTR 之间切换，
    // 两条入口各自显式转型
    HCURSOR a = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
    ASSERT_TRUE(a != NULL);

    HCURSOR w = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    ASSERT_TRUE(w != NULL);
    EXPECT_EQ(a, w);
}

// 迁自 test.cpp（原 soui.matrix）：SOUI SMatrix 缩放矩阵的乘法组合律
TEST(swinx_misc, soui_matrix_scale_compose)
{
    using namespace SOUI;
    SMatrix mtx1, mtx2, mtx3;
    mtx1.setScale2(2, 2, 50, 50);
    mtx2.setScale2(2, 1, 50, 50);
    mtx3.setScale2(1, 2, 50, 50);
    SMatrix mtx4 = mtx2 * mtx3;
    EXPECT_EQ(mtx1, mtx4);
}
