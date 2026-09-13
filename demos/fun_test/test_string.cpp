/**
 * swinx string / code conversion tests.
 * Includes a regression test for the GetStringTypeExW/A char-type-3
 * classification (the `!=C1 || !=C2 || !=C3` always-true bug).
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <string>

// ------------------------------------------------------------------------
// Wide <-> multi-byte conversion
// ------------------------------------------------------------------------

TEST(swinx_string, widechar_multibyte_utf8_roundtrip)
{
    const wchar_t *src = L"中文abc123";
    char szUtf8[128] = {0};
    int n = WideCharToMultiByte(CP_UTF8, 0, src, -1, szUtf8, sizeof(szUtf8), NULL, NULL);
    ASSERT_GT(n, 0);

    wchar_t szWide[128] = {0};
    MultiByteToWideChar(CP_UTF8, 0, szUtf8, -1, szWide, sizeof(szWide) / sizeof(wchar_t));
    EXPECT_TRUE(wcscmp(src, szWide) == 0);
}

TEST(swinx_string, widechar_multibyte_cp936_roundtrip)
{
    const wchar_t *src = L"中文字符串test";
    char sz936[100] = {0};
    int n = WideCharToMultiByte(936, 0, src, -1, sz936, sizeof(sz936), NULL, NULL);
    ASSERT_GT(n, 0);

    wchar_t szWide[100] = {0};
    MultiByteToWideChar(936, 0, sz936, -1, szWide, 100);
    EXPECT_TRUE(wcscmp(src, szWide) == 0);
}

TEST(swinx_string, widechar_to_multibyte_length)
{
    const wchar_t *src = L"hello";
    // size query with cbMultiByte == 0 returns required size including '\0'
    int need = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);
    EXPECT_EQ(need, 6);
}

// ------------------------------------------------------------------------
// GetStringTypeEx
// ------------------------------------------------------------------------

TEST(swinx_string, get_string_type_w_ctype1)
{
    const wchar_t *src = L"aB1 ";
    WORD types[4] = {0};
    EXPECT_TRUE(GetStringTypeExW(0, CT_CTYPE1, src, 4, types));

    EXPECT_TRUE(types[0] & C1_LOWER);
    EXPECT_FALSE(types[0] & C1_UPPER);

    EXPECT_TRUE(types[1] & C1_UPPER);
    EXPECT_FALSE(types[1] & C1_LOWER);

    EXPECT_TRUE(types[2] & C1_DIGIT);
    EXPECT_FALSE(types[2] & C1_ALPHA);

    EXPECT_TRUE(types[3] & C1_SPACE);
}

TEST(swinx_string, get_string_type_w_cjk)
{
    const wchar_t *src = L"中";
    WORD types[1] = {0};
    EXPECT_TRUE(GetStringTypeExW(0, CT_CTYPE1, src, 1, types));
    EXPECT_TRUE(types[0] & C1_DEFINED);
    // a CJK char is not an ASCII digit or space
    EXPECT_FALSE(types[0] & C1_DIGIT);
    EXPECT_FALSE(types[0] & C1_SPACE);
}

TEST(swinx_string, get_string_type_a)
{
    const char *src = "aB1";
    WORD types[3] = {0};
    EXPECT_TRUE(GetStringTypeExA(0, CT_CTYPE1, src, 3, types));
    EXPECT_TRUE(types[0] & C1_LOWER);
    EXPECT_TRUE(types[1] & C1_UPPER);
    EXPECT_TRUE(types[2] & C1_DIGIT);
}

// ------------------------------------------------------------------------
// CharLower / CharUpper
// ------------------------------------------------------------------------

TEST(swinx_string, char_lower)
{
    char buf[] = "AbC123";
    LPSTR ret = CharLowerA(buf);
    EXPECT_EQ(ret, buf);
    EXPECT_STREQ(buf, "abc123");
}

TEST(swinx_string, lstrlen)
{
    EXPECT_EQ(lstrlenA("abc"), 3);
    EXPECT_EQ(lstrlenW(L"abc"), 3);
}
