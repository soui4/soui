/**
 * GetClassInfoA/W 的回归用例：swinx 让它们复用 GetClassInfoEx 的查询结果，
 * 再把公共字段回填到不含 cbSize / hIconSm 的 WNDCLASS。
 * Windows 上跑的是系统 API，因此只断言两边都成立的公共契约。
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <string.h>

namespace
{

LRESULT CALLBACK classInfoTestProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    (void)hWnd;
    (void)msg;
    (void)wParam;
    (void)lParam;
    return 0;
}

}

TEST(swinx_class, get_class_info_a_reports_registered_class)
{
    const char *kName = "swinx_class_info_test_a";

    WNDCLASSA cls;
    memset(&cls, 0, sizeof(cls));
    cls.style = CS_HREDRAW | CS_VREDRAW;
    cls.lpfnWndProc = classInfoTestProc;
    cls.cbClsExtra = 4;
    cls.cbWndExtra = 8;
    cls.hInstance = GetModuleHandleA(NULL);
    cls.lpszClassName = kName;

    UnregisterClassA(kName, cls.hInstance);
    ASSERT_TRUE(RegisterClassA(&cls) != 0);

    WNDCLASSA info;
    memset(&info, 0, sizeof(info));
    EXPECT_TRUE(GetClassInfoA(cls.hInstance, kName, &info));
    EXPECT_EQ(cls.style, info.style);
    EXPECT_TRUE(info.lpfnWndProc == classInfoTestProc);
    EXPECT_EQ(cls.cbClsExtra, info.cbClsExtra);
    EXPECT_EQ(cls.cbWndExtra, info.cbWndExtra);
    EXPECT_TRUE(info.hInstance == cls.hInstance);
    EXPECT_TRUE(info.lpszClassName != NULL);
    if (info.lpszClassName)
        EXPECT_STREQ(kName, info.lpszClassName);

    EXPECT_TRUE(UnregisterClassA(kName, cls.hInstance));
}

TEST(swinx_class, get_class_info_w_reports_registered_class)
{
    const wchar_t *kName = L"swinx_class_info_test_w";

    WNDCLASSW cls;
    memset(&cls, 0, sizeof(cls));
    cls.style = CS_HREDRAW | CS_VREDRAW;
    cls.lpfnWndProc = classInfoTestProc;
    cls.cbClsExtra = 4;
    cls.cbWndExtra = 8;
    cls.hInstance = GetModuleHandleW(NULL);
    cls.lpszClassName = kName;

    UnregisterClassW(kName, cls.hInstance);
    ASSERT_TRUE(RegisterClassW(&cls) != 0);

    WNDCLASSW info;
    memset(&info, 0, sizeof(info));
    EXPECT_TRUE(GetClassInfoW(cls.hInstance, kName, &info));
    EXPECT_EQ(cls.style, info.style);
    EXPECT_TRUE(info.lpfnWndProc == classInfoTestProc);
    EXPECT_EQ(cls.cbClsExtra, info.cbClsExtra);
    EXPECT_EQ(cls.cbWndExtra, info.cbWndExtra);
    EXPECT_TRUE(info.hInstance == cls.hInstance);
    EXPECT_TRUE(info.lpszClassName != NULL);
    if (info.lpszClassName)
        EXPECT_EQ(0, wcscmp(kName, info.lpszClassName));

    EXPECT_TRUE(UnregisterClassW(kName, cls.hInstance));
}
