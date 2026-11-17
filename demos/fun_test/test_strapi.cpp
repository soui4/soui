/**
 * swinx string helper APIs: path splitting, character stepping,
 * OEM conversion and string-to-number parsing.
 *
 * Assertions follow standard Win32/CRT semantics and compile against
 * real Windows as well as swinx.
 */

#include <gtest/gtest.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <string>

#ifdef _WIN32
#include <shlwapi.h>
#else
#include <strapi.h>
#include <sysapi.h>
#endif

#ifndef STIF_SUPPORT_HEX
#define STIF_SUPPORT_HEX 0x00000001L
#endif

// swinx's strapi.h relies on the CRT names but does not define them
// (the Windows CRT provides them via stdlib.h)
#ifndef _MAX_DRIVE
#define _MAX_DRIVE 3
#endif
#ifndef _MAX_DIR
#define _MAX_DIR 256
#endif
#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif
#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif

// ------------------------------------------------------------------------
// _splitpath / _wsplitpath
// ------------------------------------------------------------------------

TEST(swinx_splitpath, absolute_path_decomposition)
{
    // mixed separators work with the CRT on Windows and with swinx
    char drive[_MAX_DRIVE] = {0};
    char dir[_MAX_DIR] = {0};
    char fname[_MAX_FNAME] = {0};
    char ext[_MAX_EXT] = {0};

    _splitpath("/usr/local/bin/grep.txt", drive, dir, fname, ext);
    EXPECT_STREQ(dir, "/usr/local/bin/");
    EXPECT_STREQ(fname, "grep");
    EXPECT_STREQ(ext, ".txt");
}

#ifdef _WIN32
TEST(swinx_splitpath, drive_letter_extraction)
{
    // real CRT extracts the drive; swinx's POSIX implementation leaves
    // the drive buffer untouched (paths have no drive letters there)
    char drive[_MAX_DRIVE] = {'x', 0};
    char dir[_MAX_DIR] = {0};
    char fname[_MAX_FNAME] = {0};
    char ext[_MAX_EXT] = {0};

    _splitpath("C:\\dir\\file.ext", drive, dir, fname, ext);
    EXPECT_STREQ(drive, "C:");
    EXPECT_STREQ(dir, "\\dir\\");
    EXPECT_STREQ(fname, "file");
    EXPECT_STREQ(ext, ".ext");
}
#endif

TEST(swinx_splitpath, relative_and_extension_edges)
{
    char dir[_MAX_DIR] = {0};
    char fname[_MAX_FNAME] = {0};
    char ext[_MAX_EXT] = {0};

    // last dot in the name part starts the extension
    _splitpath("file.tar.gz", NULL, dir, fname, ext);
    EXPECT_STREQ(dir, "");
    EXPECT_STREQ(fname, "file.tar");
    EXPECT_STREQ(ext, ".gz");

    // no extension at all
    _splitpath("noext", NULL, dir, fname, ext);
    EXPECT_STREQ(fname, "noext");
    EXPECT_STREQ(ext, "");

    // dot in the directory part must not leak into the name
    _splitpath("dir.x/name", NULL, dir, fname, ext);
    EXPECT_STREQ(dir, "dir.x/");
    EXPECT_STREQ(fname, "name");
    EXPECT_STREQ(ext, "");
}

TEST(swinx_splitpath, wide_path_decomposition)
{
    wchar_t dir[_MAX_DIR] = {0};
    wchar_t fname[_MAX_FNAME] = {0};
    wchar_t ext[_MAX_EXT] = {0};

    _wsplitpath(L"/opt/app/readme.md", NULL, dir, fname, ext);
    EXPECT_STREQ(dir, L"/opt/app/");
    EXPECT_STREQ(fname, L"readme");
    EXPECT_STREQ(ext, L".md");
}

// ------------------------------------------------------------------------
// CharNextA
// ------------------------------------------------------------------------

TEST(swinx_charnext, ascii_advances_by_one)
{
    const char *s = "ab";
    EXPECT_STREQ(CharNextA(s), "b");
    EXPECT_STREQ(CharNextA(s + 1), "");
    // empty string: stays at the terminator
    const char *e = "";
    EXPECT_TRUE(CharNextA(e) == e);
}

#ifndef _WIN32
TEST(swinx_charnext, utf8_multibyte_step)
{
    // swinx treats A strings as UTF-8: one Chinese character = 3 bytes
    const char *s = "中a";
    const char *next = CharNextA(s);
    EXPECT_EQ(next - s, 3);
    EXPECT_STREQ(next, "a");
}
#endif

// ------------------------------------------------------------------------
// CharToOem / OemToChar
// ------------------------------------------------------------------------

TEST(swinx_oem, ascii_roundtrip_a)
{
    char oem[32] = {0};
    ASSERT_TRUE(CharToOemA("swinx test 123", oem));
    // ASCII is identical in every ANSI/OEM code page
    EXPECT_STREQ(oem, "swinx test 123");

    char back[32] = {0};
    ASSERT_TRUE(OemToCharA(oem, back));
    EXPECT_STREQ(back, "swinx test 123");
}

TEST(swinx_oem, wide_roundtrip)
{
    char oem[32] = {0};
    ASSERT_TRUE(CharToOemW(L"wide oem", oem));
    EXPECT_STREQ(oem, "wide oem");

    wchar_t back[32] = {0};
    ASSERT_TRUE(OemToCharW(oem, back));
    EXPECT_STREQ(back, L"wide oem");
}

TEST(swinx_oem, buff_converts_exact_length)
{
    char oem[8];
    memset(oem, 'z', sizeof(oem));
    ASSERT_TRUE(CharToOemBuffA("abcdef", oem, 4));
    // only the first 4 chars converted; byte 4 keeps the 'z' filler
    EXPECT_EQ(memcmp(oem, "abcd", 4), 0);
    EXPECT_EQ(oem[4], 'z');
    // NOTE: CharToOemBuffA does not add a terminator, matching Win32
}

// ------------------------------------------------------------------------
// StrToIntExA / StrToInt64ExA
// ------------------------------------------------------------------------

TEST(swinx_strtoint, decimal_and_signs)
{
    INT v = 0;
    ASSERT_TRUE(StrToIntExA("42", 0, &v));
    EXPECT_EQ(v, 42);

    ASSERT_TRUE(StrToIntExA("-17", 0, &v));
    EXPECT_EQ(v, -17);

    ASSERT_TRUE(StrToIntExA("+9", 0, &v));
    EXPECT_EQ(v, 9);

    // leading zeros
    ASSERT_TRUE(StrToIntExA("007", 0, &v));
    EXPECT_EQ(v, 7);
}

TEST(swinx_strtoint, stops_at_first_nondigit)
{
    // Win32 semantics: leading digits are parsed, the rest ignored
    INT v = 0;
    ASSERT_TRUE(StrToIntExA("12abc", 0, &v));
    EXPECT_EQ(v, 12);

    ASSERT_TRUE(StrToIntExA("5 6", 0, &v));
    EXPECT_EQ(v, 5);
}

TEST(swinx_strtoint, hex_requires_flag)
{
    INT v = 0;
    // without the flag "0x1A" parses as decimal 0 then stops at 'x'
    ASSERT_TRUE(StrToIntExA("0x1A", 0, &v));
    EXPECT_EQ(v, 0);

    ASSERT_TRUE(StrToIntExA("0x1A", STIF_SUPPORT_HEX, &v));
    EXPECT_EQ(v, 26);

    // uppercase prefix and digits work too
    ASSERT_TRUE(StrToIntExA("0XFF", STIF_SUPPORT_HEX, &v));
    EXPECT_EQ(v, 255);

    // a bare "0x" with no hex digits fails
    EXPECT_FALSE(StrToIntExA("0x", STIF_SUPPORT_HEX, &v));
}

// "010" 必须读成十进制 10：StrToIntEx 不是 strtol(,,0)，0 前缀没有八进制含义。
// 旧的 freertos 版本正是裸 strtol(,,0)，"010" 会读成 8、"0755" 会读成 493 ——
// 第二批下沉把两侧统一到 Win32 实现后，用这条把语义钉住。
TEST(swinx_strtoint, leading_zeros_are_decimal_not_octal)
{
    INT v = 0;
    ASSERT_TRUE(StrToIntExA("010", 0, &v));
    EXPECT_EQ(v, 10);

    ASSERT_TRUE(StrToIntExA("0755", 0, &v));
    EXPECT_EQ(v, 755);
}

TEST(swinx_strtoint, leading_whitespace_is_skipped)
{
    INT v = 0;
    ASSERT_TRUE(StrToIntExA(" \t42", 0, &v));
    EXPECT_EQ(v, 42);

    ASSERT_TRUE(StrToIntExA("\n42", 0, &v));
    EXPECT_EQ(v, 42);

    // '\v' 不是分隔符（与 shlwapi 一致：只有空格、'\t'、'\n' 被跳过）
    EXPECT_FALSE(StrToIntExA("\v42", 0, &v));
}

// Win32 的十六进制分支只累加数值，符号在进入分支前就被丢掉了：
// "-0x10" 与 "+0x10" 都得到 16。这条容易在"重写解析器"时被顺手改成取负。
TEST(swinx_strtoint, hex_ignores_sign)
{
    INT v = 0;
    ASSERT_TRUE(StrToIntExA("-0x10", STIF_SUPPORT_HEX, &v));
    EXPECT_EQ(v, 16);

    ASSERT_TRUE(StrToIntExA("+0x10", STIF_SUPPORT_HEX, &v));
    EXPECT_EQ(v, 16);
}

TEST(swinx_strtoint, rejects_non_numeric)
{
    INT v = 123;
    EXPECT_FALSE(StrToIntExA("abc", 0, &v));
    EXPECT_EQ(v, 0); // 失败时把结果清 0，而不是保持调用方原值
    v = 123;
    EXPECT_FALSE(StrToIntExA("", 0, &v));
    EXPECT_EQ(v, 0);
    EXPECT_FALSE(StrToIntExA("-", 0, &v)); // sign alone is not a number
#ifndef _WIN32
    // swinx guards NULL; real shlwapi's NULL behaviour is undocumented
    EXPECT_FALSE(StrToIntExA(NULL, 0, &v));
#endif
}

TEST(swinx_strtoint64, full_range)
{
    LONGLONG v = 0;
    ASSERT_TRUE(StrToInt64ExA("9223372036854775807", 0, &v));
    EXPECT_EQ(v, 9223372036854775807LL);

    ASSERT_TRUE(StrToInt64ExA("-9223372036854775807", 0, &v));
    EXPECT_EQ(v, -9223372036854775807LL);

    ASSERT_TRUE(StrToInt64ExA("0x7FFFFFFFFFFFFFFF", STIF_SUPPORT_HEX, &v));
    EXPECT_EQ(v, 9223372036854775807LL);
}

// ------------------------------------------------------------------------
// StrToIntExW / StrToInt64ExW
//
// 宽版随第二批下沉进了 common/sysapi.cpp，并且成了唯一实现：A 版只是先
// towstring()（UTF-8 -> wchar_t）再调它。旧 freertos 目录里 A/W 各有一份裸
// strtol 副本，所以这里既要比对两条入口的一致性，也要单独覆盖宽版。
// ------------------------------------------------------------------------

TEST(swinx_strtoint, wide_matches_ansi)
{
    struct Case
    {
        const char *text;
        DWORD flags;
    };
    const Case cases[] = {
        {"42", 0},
        {"-17", 0},
        {"+9", 0},
        {"010", 0},
        {" 7", 0},
        {"12abc", 0},
        {"0x1A", STIF_SUPPORT_HEX},
    };

    for (int i=0;i<ARRAYSIZE(cases);i++)
    {
		const Case &c = cases[i];
        INT va = -1;
        ASSERT_TRUE(StrToIntExA(c.text, c.flags, &va)) << c.text;

        std::wstring wide(c.text, c.text + strlen(c.text));
        INT vw = -1;
        ASSERT_TRUE(StrToIntExW(wide.c_str(), c.flags, &vw)) << c.text;

        // A 版经 towstring 转成宽串后调 W 版，两者必须逐例相同
        EXPECT_EQ(va, vw) << c.text;
    }
}

TEST(swinx_strtoint, wide_rejects_non_numeric)
{
    // 失败时输出参数被写成 0（不是"保持原值"）—— shlwapi 实测如此，
    // common/sysapi.cpp 按同一语义实现
    INT v = 123;
    EXPECT_FALSE(StrToIntExW(L"abc", 0, &v));
    EXPECT_EQ(v, 0);

    v = 123;
    EXPECT_FALSE(StrToIntExW(L"", 0, &v));
    EXPECT_EQ(v, 0);

    EXPECT_FALSE(StrToIntExW(L"-", 0, &v));
    EXPECT_FALSE(StrToIntExW(L"0x", STIF_SUPPORT_HEX, &v));
#ifndef _WIN32
    // swinx guards NULL; real shlwapi's NULL behaviour is undocumented
    EXPECT_FALSE(StrToIntExW(NULL, 0, &v));
    EXPECT_FALSE(StrToIntExW(L"1", 0, NULL));
#endif
}

TEST(swinx_strtoint64, wide_full_range)
{
    LONGLONG v = 0;
    ASSERT_TRUE(StrToInt64ExW(L"9223372036854775807", 0, &v));
    EXPECT_EQ(v, 9223372036854775807LL);

    ASSERT_TRUE(StrToInt64ExW(L"-9223372036854775807", 0, &v));
    EXPECT_EQ(v, -9223372036854775807LL);

    ASSERT_TRUE(StrToInt64ExW(L"0x7FFFFFFFFFFFFFFF", STIF_SUPPORT_HEX, &v));
    EXPECT_EQ(v, 9223372036854775807LL);

    // 宽版同样不认八进制
    ASSERT_TRUE(StrToInt64ExW(L"010", 0, &v));
    EXPECT_EQ(v, 10LL);
}

// ------------------------------------------------------------------------
// code page queries
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
// code page conversion (MultiByteToWideChar / WideCharToMultiByte)
//
// 1200/1201（UTF-16 源/目标）只有 swinx 支持：Win32 的 MultiByteToWideChar /
// WideCharToMultiByte 不接受这两个 code page（返回 ERROR_INVALID_PARAMETER），
// 而 swinx 需要它们来解 PE 资源里的 UTF-16 名字（见 src/coffparser.cpp）。
// 所以涉及 1200 的用例在真实 Windows 上没有对端实现，直接跳过；CP936 的用例
// 两边都有，作为 swinx 与 Win32 语义一致的对照。

// swinx's implementation lives in swinx/src/platform/common/codepage.cpp and is
// shared by every platform: iconv first, builtin CP936 / Windows-125x / UTF-16
// tables when iconv is unavailable (bare metal, bionic, musl).  These tests pin
// the Win32 semantics that the shared code has to keep.
// ------------------------------------------------------------------------

TEST(swinx_codepage, utf16le_decodes_bmp_and_surrogate)
{
#ifdef _WIN32
    return; // 见文件上方说明：Win32 不接受 1200/1201
#endif
    // "Hi" + 中文 + U+1F600（UTF-16 代理对 D83D DE00），小端字节序
    const char u16[] = {'H', 0, 'i', 0, 0x2D, 0x4E, (char)0x87, 0x65,
                        (char)0x3D, (char)0xD8, 0x00, (char)0xDE};
    wchar_t out[8] = {0};
    const int n = MultiByteToWideChar(1200 /* CP_UTF16_LE */, 0, u16, (int)sizeof(u16), out, 8);
    // wchar_t 是 32 位时把代理对合成一个码点；是 16 位时保留两个单元
    ASSERT_EQ(n, sizeof(wchar_t) == 4 ? 5 : 6);
    EXPECT_EQ(out[0], L'H');
    EXPECT_EQ(out[1], L'i');
    EXPECT_EQ(out[2], (wchar_t)0x4E2D);
    EXPECT_EQ(out[3], (wchar_t)0x6587);
    if (sizeof(wchar_t) == 4)
        EXPECT_EQ(out[4], (wchar_t)0x1F600);
}

TEST(swinx_codepage, utf16le_len_minus_one_counts_code_units)
{
#ifdef _WIN32
    return; // 见文件上方说明：Win32 不接受 1200/1201
#endif
    // 结尾 0x0000 之后再放一个 'X'：len == -1 必须按 2 字节对齐找 NUL，
    // 用 strlen 会在 'H' 后那个 0 字节处截断，只解出 1 个字符。
    const char u16[] = {'H', 0, 'i', 0, 0, 0, 'X', 0};
    wchar_t out[8];
    for (int i = 0; i < 8; ++i)
        out[i] = (wchar_t)0x7f;

    const int n = MultiByteToWideChar(1200 /* CP_UTF16_LE */, 0, u16, -1, out, 8);
    ASSERT_EQ(n, 3); // 'H'、'i'、NUL
    EXPECT_EQ(out[0], L'H');
    EXPECT_EQ(out[1], L'i');
    EXPECT_EQ(out[2], 0);
    EXPECT_EQ(out[3], (wchar_t)0x7f); // 结尾之后的 'X' 不能被转换
}

TEST(swinx_codepage, utf16le_encode_writes_wide_nul)
{
#ifdef _WIN32
    return; // 见文件上方说明：Win32 不接受 1200/1201
#endif
    const wchar_t src[] = L"Hi";
    const int need = WideCharToMultiByte(1200 /* CP_UTF16_LE */, 0, src, -1, NULL, 0, NULL, NULL);
    ASSERT_EQ(need, 6); // 'H'、'i'、NUL 各 2 字节

    char buf[8];
    memset(buf, 0x7f, sizeof(buf));
    // 返回值含结尾 NUL 的字节数（与 CP_UTF8 分支一致）
    EXPECT_EQ(WideCharToMultiByte(1200 /* CP_UTF16_LE */, 0, src, -1, buf, 6, NULL, NULL), 6);
    EXPECT_EQ(buf[0], 'H');
    EXPECT_EQ(buf[1], 0);
    EXPECT_EQ(buf[2], 'i');
    EXPECT_EQ(buf[3], 0);
    EXPECT_EQ(buf[4], 0); // NUL 是两个字节，不是半个
    EXPECT_EQ(buf[5], 0);
    EXPECT_EQ(buf[6], 0x7f); // 不越界写
}

TEST(swinx_codepage, cp936_query_length_and_roundtrip)
{
#ifdef _WIN32
    // 未安装 936 代码页的 Windows 上系统 API 本身不可用
    if (!IsValidCodePage(936))
        return; // 未安装 936 表的 Windows：系统 API 本身不可用
#endif
    const wchar_t src[] = L"\u4e2d\u6587\u4f60\u597d";

    // dst == NULL 是「查询所需大小」，必须返回长度而不是 0
    const int need = WideCharToMultiByte(936, 0, src, -1, NULL, 0, NULL, NULL);
    ASSERT_EQ(need, 9); // 4 个汉字 8 字节 + 结尾 NUL

    char mb[16] = {0};
    ASSERT_EQ(WideCharToMultiByte(936, 0, src, -1, mb, (int)sizeof(mb), NULL, NULL), 9);
    EXPECT_EQ((unsigned char)mb[0], 0xD6);
    EXPECT_EQ((unsigned char)mb[1], 0xD0);

    wchar_t back[8] = {0};
    ASSERT_EQ(MultiByteToWideChar(936, 0, mb, -1, back, 8), 5);
    EXPECT_EQ(wcscmp(src, back), 0);
}

TEST(swinx_codepage, get_acp_is_utf8_on_swinx)
{
#ifdef _WIN32
    // real Windows: whatever the system ANSI code page is, it must be valid
    UINT acp = GetACP();
    EXPECT_GT(acp, 0u);
    EXPECT_TRUE(IsValidCodePage(acp));
#else
    // swinx runs UTF-8 everywhere
    EXPECT_EQ(GetACP(), (UINT)CP_UTF8);
    EXPECT_TRUE(IsValidCodePage(CP_UTF8));
#endif
}
