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

TEST(swinx_strtoint, rejects_non_numeric)
{
    INT v = 123;
    EXPECT_FALSE(StrToIntExA("abc", 0, &v));
    EXPECT_FALSE(StrToIntExA("", 0, &v));
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
// code page queries
// ------------------------------------------------------------------------

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
