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

// 硬编码期望值的解码用例: 比上面的往返更能暴露"双向同错"的回归, 也是 Android/OHOS
// 上的真实故障点 —— 这两个平台的 iconv 只实现 Unicode 系编码(UTF-8/16/32、
// US-ASCII、wchar_t), iconv_open("WINDOWS-936") 必然失败, CP936 曾整段无法转换。
// swinx 现在用内置码表(swinx/src/cptable.h)兜底, 见 sysapi.cpp 的
// builtinToUnicode / builtinToMb。
TEST(swinx_string, cp936_bytes_decode_to_expected_wide_chars)
{
    // "中文字符串" 的 GBK 编码: d6d0 cec4 d7d6 b7fb b4ae
    const char szGbk[] = "\xd6\xd0\xce\xc4\xd7\xd6\xb7\xfb\xb4\xae";
    wchar_t szWide[16] = {0};
    int n = MultiByteToWideChar(936, 0, szGbk, 10, szWide, 16);
    ASSERT_EQ(n, 5);
    EXPECT_EQ(szWide[0], 0x4e2d); // 中
    EXPECT_EQ(szWide[1], 0x6587); // 文
    EXPECT_EQ(szWide[2], 0x5b57); // 字
    EXPECT_EQ(szWide[3], 0x7b26); // 符
    EXPECT_EQ(szWide[4], 0x4e32); // 串
}

TEST(swinx_string, cp936_wide_to_multibyte_bytes_match_expected)
{
    const wchar_t *src = L"中文";
    char szGbk[16] = {0};
    int n = WideCharToMultiByte(936, 0, src, 2, szGbk, sizeof(szGbk), NULL, NULL);
    ASSERT_EQ(n, 4);
    EXPECT_EQ((unsigned char)szGbk[0], 0xd6);
    EXPECT_EQ((unsigned char)szGbk[1], 0xd0);
    EXPECT_EQ((unsigned char)szGbk[2], 0xce);
    EXPECT_EQ((unsigned char)szGbk[3], 0xc4);
}

// Windows-125x 是单字节代码页, bionic/musl 同样不认识它们, 一并覆盖。
TEST(swinx_string, cp1252_bytes_decode_to_expected_wide_chars)
{
    const char szAnsi[] = "\x80\x93\x94\xe9";
    wchar_t szWide[8] = {0};
    int n = MultiByteToWideChar(1252, 0, szAnsi, 4, szWide, 8);
    ASSERT_EQ(n, 4);
    EXPECT_EQ(szWide[0], 0x20ac); // 欧元符号
    EXPECT_EQ(szWide[1], 0x201c); // 左双引号
    EXPECT_EQ(szWide[2], 0x201d); // 右双引号
    EXPECT_EQ(szWide[3], 0x00e9); // e with acute
}

TEST(swinx_string, cp936_invalid_sequence_fails_with_err_invalid_chars)
{
    const char szBad[] = "\xff"; // 0xff 在 CP936 里既不是首字节也不是合法单字节
    wchar_t szWide[8] = {0};
    SetLastError(0);
    EXPECT_EQ(MultiByteToWideChar(936, MB_ERR_INVALID_CHARS, szBad, 1, szWide, 8), 0);
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
