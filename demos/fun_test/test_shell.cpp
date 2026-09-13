/**
 * swinx shell/path API tests: shlwapi Path* helpers, shellapi path utils,
 * CoCreateGuid and SAFEARRAY. Assertions follow standard Win32 semantics
 * and compile against real Windows as well as swinx. Path literals use
 * each platform's native form: drive/backslash paths under _WIN32,
 * POSIX slash paths under swinx/Linux.
 *
 * Regression: PathIsDirectoryA used to AND GetFileAttributesA's result
 * with FILE_ATTRIBUTE_DIRECTORY without rejecting INVALID_FILE_ATTRIBUTES
 * (0xFFFFFFFF), so non-existent paths read as directories.
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <shlwapi.h>
#include <oleauto.h>
#include <stdio.h>
#include <string.h>
#include <string>

static std::string shellTempPath(const char *name)
{
#ifdef _WIN32
    char tmp[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    char buf[320];
    sprintf(buf, "%sswinx_shell_test_%u_%s", tmp, GetCurrentProcessId(), name);
#else
    char buf[256];
    sprintf(buf, "/tmp/swinx_shell_test_%u_%s", GetCurrentProcessId(), name);
#endif
    return buf;
}

// platform-native separator for building child paths under the temp dir
#ifdef _WIN32
static const char kPathSep = '\\';
#else
static const char kPathSep = '/';
#endif

// ------------------------------------------------------------------------
// shlwapi: path existence / type
// ------------------------------------------------------------------------

TEST(swinx_path, file_exists_and_is_directory)
{
    std::string dir = shellTempPath("pdir");
    ASSERT_TRUE(CreateDirectoryA(dir.c_str(), NULL));
    std::string file = dir + kPathSep + "f.txt";
    HANDLE h = CreateFileA(file.c_str(), GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);
    CloseHandle(h);

    EXPECT_TRUE(PathFileExistsA(dir.c_str()));
    EXPECT_TRUE(PathFileExistsA(file.c_str()));
    std::string missing = dir + kPathSep + "no_such_file";
    EXPECT_FALSE(PathFileExistsA(missing.c_str()));

    EXPECT_TRUE(PathIsDirectoryA(dir.c_str()));
    EXPECT_FALSE(PathIsDirectoryA(file.c_str()));
    // regression: 0xFFFFFFFF & FILE_ATTRIBUTE_DIRECTORY != 0 made every
    // missing path read as a directory
    EXPECT_FALSE(PathIsDirectoryA(missing.c_str()));

    DeleteFileA(file.c_str());
    RemoveDirectoryA(dir.c_str());
}

// ------------------------------------------------------------------------
// shlwapi: relative/absolute classification
// ------------------------------------------------------------------------

TEST(swinx_path, is_relative)
{
    // relative names and extension-only names behave the same everywhere
    EXPECT_TRUE(PathIsRelativeA("plain.txt"));

#ifdef _WIN32
    // Windows path rules: drive-qualified and drive-relative forms
    EXPECT_TRUE(PathIsRelativeA("foo\\bar.txt"));
    EXPECT_FALSE(PathIsRelativeA("\\abs"));      // drive-relative, not relative
    EXPECT_FALSE(PathIsRelativeA("C:\\abs"));    // drive-qualified
    EXPECT_TRUE(PathIsRelativeA("/abs/path"));   // real Win32 treats it drive-relative
#else
    // POSIX path rules: a leading '/' is absolute
    EXPECT_TRUE(PathIsRelativeA("foo/bar.txt"));
    EXPECT_FALSE(PathIsRelativeA("/abs"));
    EXPECT_FALSE(PathIsRelativeA("/abs/path"));
#endif
}

// ------------------------------------------------------------------------
// shlwapi: wildcard matching
// ------------------------------------------------------------------------

TEST(swinx_path, match_spec)
{
    EXPECT_TRUE(PathMatchSpecA("file.txt", "*.txt"));
    EXPECT_FALSE(PathMatchSpecA("file.doc", "*.txt"));
    EXPECT_TRUE(PathMatchSpecA("archive.tar.gz", "*.gz"));
    EXPECT_FALSE(PathMatchSpecA("readme.md", "*.gz"));
}

TEST(swinx_path, match_spec_multiple)
{
    // semicolon-delimited specs: real shlwapi accepts them by default,
    // swinx requires the explicit PMSF_MULTIPLE flag
#ifdef _WIN32
    EXPECT_TRUE(PathMatchSpecA("a.png", "*.png;*.jpg"));
    EXPECT_TRUE(PathMatchSpecA("b.jpg", "*.png;*.jpg"));
    EXPECT_FALSE(PathMatchSpecA("c.bmp", "*.png;*.jpg"));
#else
    EXPECT_TRUE(PathMatchSpecExA("a.png", "*.png;*.jpg", PMSF_MULTIPLE));
    EXPECT_TRUE(PathMatchSpecExA("b.jpg", "*.png;*.jpg", PMSF_MULTIPLE));
    EXPECT_FALSE(PathMatchSpecExA("c.bmp", "*.png;*.jpg", PMSF_MULTIPLE));
#endif
}

// ------------------------------------------------------------------------
// shlwapi: common prefix
// ------------------------------------------------------------------------

TEST(swinx_path, common_prefix)
{
    char buf[MAX_PATH] = {0};
    int n;
#ifdef _WIN32
    n = PathCommonPrefixA("C:\\dir\\one.txt", "C:\\dir\\two.txt", buf);
    // common prefix "C:\dir" is 6 characters
    EXPECT_EQ(n, 6);
    EXPECT_STREQ(buf, "C:\\dir");

    // no shared component -> 0 and empty buffer
    memset(buf, 'x', sizeof(buf));
    n = PathCommonPrefixA("C:\\dir\\a", "D:\\dir\\b", buf);
    EXPECT_EQ(n, 0);
    EXPECT_STREQ(buf, "");

    // drive root "C:\" counts as a complete component (3 chars, incl. the sep)
    memset(buf, 'x', sizeof(buf));
    n = PathCommonPrefixA("C:\\foo", "C:\\", buf);
    EXPECT_EQ(n, 3);
    EXPECT_STREQ(buf, "C:\\");

    // a partial component ("foo" of "foobar") is NOT a common component;
    // the result falls back to the drive root boundary
    memset(buf, 'x', sizeof(buf));
    n = PathCommonPrefixA("C:\\foobar", "C:\\foo", buf);
    EXPECT_EQ(n, 3);
    EXPECT_STREQ(buf, "C:\\");

    // comparison is case-insensitive, buffer keeps file1's spelling
    memset(buf, 'x', sizeof(buf));
    n = PathCommonPrefixA("C:\\FOO", "C:\\foo", buf);
    EXPECT_EQ(n, 6);
    EXPECT_STREQ(buf, "C:\\FOO");
#else
    n = PathCommonPrefixA("/dir/one.txt", "/dir/two.txt", buf);
    // common prefix "/dir" is 4 characters
    EXPECT_EQ(n, 4);
    EXPECT_STREQ(buf, "/dir");

    // no shared component -> 0 and empty buffer
    memset(buf, 'x', sizeof(buf));
    n = PathCommonPrefixA("/dir/a", "/other/b", buf);
    EXPECT_EQ(n, 0);
    EXPECT_STREQ(buf, "");

    // the POSIX root "/" ends a comparison and counts as the whole prefix
    memset(buf, 'x', sizeof(buf));
    n = PathCommonPrefixA("/foo", "/", buf);
    EXPECT_EQ(n, 1);
    EXPECT_STREQ(buf, "/");

    // a partial component ("foo" of "foobar") is NOT a common component.
    // unlike the drive root "C:\", the POSIX root "/" is not reported as a
    // component boundary, so there is nothing to fall back to -> 0
    memset(buf, 'x', sizeof(buf));
    n = PathCommonPrefixA("/foobar", "/foo", buf);
    EXPECT_EQ(n, 0);
    EXPECT_STREQ(buf, "");

    // comparison is case-insensitive, buffer keeps file1's spelling
    memset(buf, 'x', sizeof(buf));
    n = PathCommonPrefixA("/FOO", "/foo", buf);
    EXPECT_EQ(n, 4);
    EXPECT_STREQ(buf, "/FOO");
#endif
}

// ------------------------------------------------------------------------
// shellapi: file name extraction / quoting / canonicalization
// ------------------------------------------------------------------------

TEST(swinx_path, find_file_name)
{
#ifdef _WIN32
    EXPECT_STREQ(PathFindFileNameA("C:\\a\\b\\c.txt"), "c.txt");
#else
    EXPECT_STREQ(PathFindFileNameA("/a/b/c.txt"), "c.txt");
#endif
    EXPECT_STREQ(PathFindFileNameA("plain.txt"), "plain.txt");
    EXPECT_TRUE(PathFindFileNameA(NULL) == NULL);
}

TEST(swinx_path, quote_spaces)
{
    char p[MAX_PATH];

    strcpy(p, "my file.txt");
    PathQuoteSpacesA(p);
    EXPECT_STREQ(p, "\"my file.txt\"");

    // no spaces -> untouched (both platforms)
    strcpy(p, "nospace.txt");
    PathQuoteSpacesA(p);
    EXPECT_STREQ(p, "nospace.txt");

    // input already quoted: real shlwapi adds another pair of quotes
    // unconditionally, swinx avoids double-quoting
    strcpy(p, "\"already quoted.txt\"");
    PathQuoteSpacesA(p);
#ifdef _WIN32
    EXPECT_STREQ(p, "\"\"already quoted.txt\"\"");
#else
    EXPECT_STREQ(p, "\"already quoted.txt\"");
#endif
}

TEST(swinx_path, canonicalize_removes_dotdot)
{
    // only assert dot-dot elimination: the exact separator in the result
    // differs between real Windows ('\\') and swinx ('/')
    char buf[MAX_PATH] = {0};
#ifdef _WIN32
    ASSERT_TRUE(PathCanonicalizeA(buf, "a\\b\\..\\c.txt"));
#else
    ASSERT_TRUE(PathCanonicalizeA(buf, "a/b/../c.txt"));
#endif
    EXPECT_TRUE(strstr(buf, "..") == NULL);
    EXPECT_TRUE(buf[0] != '\0');

#ifdef _WIN32
    ASSERT_TRUE(PathCanonicalizeA(buf, "a\\b\\c\\d.txt"));
#else
    ASSERT_TRUE(PathCanonicalizeA(buf, "a/b/c/d.txt"));
#endif
    EXPECT_TRUE(strstr(buf, "..") == NULL);
}

// ------------------------------------------------------------------------
// shellapi: GetFullPathName
// ------------------------------------------------------------------------

TEST(swinx_path, get_full_path_name_absolute)
{
    std::string abs = shellTempPath("gfp.bin");
    HANDLE h = CreateFileA(abs.c_str(), GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, 0, 0);
    ASSERT_TRUE(h != INVALID_HANDLE_VALUE);
    CloseHandle(h);

    char buf[MAX_PATH] = {0};
    LPSTR filePart = NULL;
    DWORD n = GetFullPathNameA(abs.c_str(), MAX_PATH, buf, &filePart);
    EXPECT_GT(n, 0u);
    ASSERT_TRUE(filePart != NULL);
    // the file part is the base name, the full path must end with it
    std::string base = abs.substr(abs.find_last_of("/\\") + 1);
    EXPECT_STREQ(filePart, base.c_str());
    EXPECT_EQ(strlen(buf) - strlen(filePart), abs.size() - base.size());
    EXPECT_STREQ(buf + strlen(buf) - strlen(filePart), filePart);

    DeleteFileA(abs.c_str());
}

TEST(swinx_path, get_full_path_name_relative)
{
    char cwd[MAX_PATH] = {0};
    ASSERT_GT(GetCurrentDirectoryA(MAX_PATH, cwd), 0u);

    char buf[MAX_PATH] = {0};
    LPSTR filePart = NULL;
    DWORD n = GetFullPathNameA("somefile.dat", MAX_PATH, buf, &filePart);
    EXPECT_GT(n, 0u);
    ASSERT_TRUE(filePart != NULL);
    EXPECT_STREQ(filePart, "somefile.dat");
    // relative names resolve under the current directory
    EXPECT_EQ(strncmp(buf, cwd, strlen(cwd)), 0);
}

// ------------------------------------------------------------------------
// COM: GUID generation
// ------------------------------------------------------------------------

TEST(swinx_com, co_create_guid_uniqueness)
{
    GUID g1 = {}, g2 = {};
    EXPECT_EQ(CoCreateGuid(&g1), S_OK);
    EXPECT_EQ(CoCreateGuid(&g2), S_OK);

    GUID zero = {};
    EXPECT_NE(memcmp(&g1, &zero, sizeof(GUID)), 0);
    EXPECT_NE(memcmp(&g2, &zero, sizeof(GUID)), 0);
    EXPECT_NE(memcmp(&g1, &g2, sizeof(GUID)), 0);
}

// ------------------------------------------------------------------------
// SAFEARRAY
// ------------------------------------------------------------------------

TEST(swinx_safearray, create_access_roundtrip)
{
    SAFEARRAYBOUND bound = {10, 0};
    SAFEARRAY *psa = SafeArrayCreate(VT_I4, 1, &bound);
    ASSERT_TRUE(psa != NULL);

    EXPECT_EQ(SafeArrayGetDim(psa), 1u);
    EXPECT_EQ(SafeArrayGetElemsize(psa), sizeof(LONG));

    LONG lo = -1, hi = -1;
    EXPECT_EQ(SafeArrayGetLBound(psa, 1, &lo), S_OK);
    EXPECT_EQ(SafeArrayGetUBound(psa, 1, &hi), S_OK);
    EXPECT_EQ(lo, 0);
    EXPECT_EQ(hi, 9);

    LONG *data = NULL;
    EXPECT_EQ(SafeArrayAccessData(psa, (void **)&data), S_OK);
    ASSERT_TRUE(data != NULL);
    for (LONG i = 0; i < 10; i++)
        data[i] = i * 7;
    EXPECT_EQ(SafeArrayUnaccessData(psa), S_OK);

    for (LONG i = 0; i < 10; i++)
    {
        LONG v = 0;
        EXPECT_EQ(SafeArrayGetElement(psa, &i, &v), S_OK);
        EXPECT_EQ(v, i * 7);
    }

    // put/get through the element API
    LONG idx = 3, v2 = 111;
    EXPECT_EQ(SafeArrayPutElement(psa, &idx, &v2), S_OK);
    LONG v3 = 0;
    EXPECT_EQ(SafeArrayGetElement(psa, &idx, &v3), S_OK);
    EXPECT_EQ(v3, 111);

    EXPECT_EQ(SafeArrayDestroy(psa), S_OK);
}

TEST(swinx_safearray, copy_independent_storage)
{
    SAFEARRAYBOUND bound = {4, 0};
    SAFEARRAY *src = SafeArrayCreate(VT_I4, 1, &bound);
    ASSERT_TRUE(src != NULL);

    LONG idx = 1, val = 42;
    EXPECT_EQ(SafeArrayPutElement(src, &idx, &val), S_OK);

    SAFEARRAY *dst = NULL;
    EXPECT_EQ(SafeArrayCopy(src, &dst), S_OK);
    ASSERT_TRUE(dst != NULL);

    val = 999;
    EXPECT_EQ(SafeArrayPutElement(src, &idx, &val), S_OK);

    LONG read = 0;
    idx = 1;
    EXPECT_EQ(SafeArrayGetElement(dst, &idx, &read), S_OK);
    EXPECT_EQ(read, 42); // the copy must keep the old value

    EXPECT_EQ(SafeArrayDestroy(src), S_OK);
    EXPECT_EQ(SafeArrayDestroy(dst), S_OK);
}

TEST(swinx_safearray, destroy_null_fails)
{
    // real oleaut32 returns S_OK for NULL; swinx may return E_POINTER.
    // Either way it must not crash.
    HRESULT hr = SafeArrayDestroy(NULL);
    EXPECT_TRUE(hr == S_OK || hr == E_POINTER);
}

// ------------------------------------------------------------------------
// PathQuoteSpaces / PathUnquoteSpaces round trip
// ------------------------------------------------------------------------

TEST(swinx_path, unquote_spaces)
{
    char buf[MAX_PATH];

    // paired quotes are stripped (verified against real shlwapi)
    strcpy(buf, "\"my file.txt\"");
    PathUnquoteSpacesA(buf);
    EXPECT_STREQ(buf, "my file.txt");

    // unquoted path is left alone
    strcpy(buf, "noquote.txt");
    PathUnquoteSpacesA(buf);
    EXPECT_STREQ(buf, "noquote.txt");

    // no closing quote -> untouched (real shlwapi does not strip the
    // leading quote of an unclosed quoted string)
    strcpy(buf, "\"unclosed");
    PathUnquoteSpacesA(buf);
    EXPECT_STREQ(buf, "\"unclosed");

    strcpy(buf, "\"pair\"");
    PathUnquoteSpacesA(buf);
    EXPECT_STREQ(buf, "pair");

    // quote/unquote round trip
    strcpy(buf, "my file.txt");
    PathQuoteSpacesA(buf);
    EXPECT_STREQ(buf, "\"my file.txt\"");
    PathUnquoteSpacesA(buf);
    EXPECT_STREQ(buf, "my file.txt");

    // NULL must not crash
    PathUnquoteSpacesA(NULL);
    SUCCEED();
}

TEST(swinx_path, is_prefix)
{
#ifdef _WIN32
    // all five cases verified against real shlwapi on Windows
    EXPECT_TRUE(PathIsPrefixA("C:\\foo", "C:\\foo\\bar"));
    EXPECT_TRUE(PathIsPrefixA("C:\\foo", "C:\\foo"));     // equal paths count
    EXPECT_TRUE(PathIsPrefixA("C:\\", "C:\\foo"));
    // boundary check: without a separator right after the prefix it is
    // not a prefix, and a prefix ending in a separator never matches
    EXPECT_FALSE(PathIsPrefixA("C:\\foo", "C:\\foobar"));
    EXPECT_FALSE(PathIsPrefixA("C:\\foo\\", "C:\\foo\\bar"));

    EXPECT_FALSE(PathIsPrefixA(NULL, "C:\\foo"));
    EXPECT_FALSE(PathIsPrefixA("C:\\foo", NULL));
#else
    EXPECT_TRUE(PathIsPrefixA("/foo", "/foo/bar"));
    EXPECT_TRUE(PathIsPrefixA("/foo", "/foo"));           // equal paths count
    EXPECT_TRUE(PathIsPrefixA("/", "/foo"));
    // boundary check: without a separator right after the prefix it is
    // not a prefix, and a prefix ending in a separator never matches
    EXPECT_FALSE(PathIsPrefixA("/foo", "/foobar"));
    EXPECT_FALSE(PathIsPrefixA("/foo/", "/foo/bar"));

    EXPECT_FALSE(PathIsPrefixA(NULL, "/foo"));
    EXPECT_FALSE(PathIsPrefixA("/foo", NULL));
#endif
}

// ------------------------------------------------------------------------
// PathFindExtension
// ------------------------------------------------------------------------

TEST(swinx_path, find_extension)
{
    // a '.' resets at separators AND spaces (verified against real shlwapi)
    EXPECT_STREQ(PathFindExtensionA("file.txt"), ".txt");
    EXPECT_STREQ(PathFindExtensionA("a.b.c"), ".c");
    EXPECT_STREQ(PathFindExtensionA("noext"), "");
    EXPECT_STREQ(PathFindExtensionA("my file.txt"), ".txt");  // dot after the space
    EXPECT_STREQ(PathFindExtensionA("a.b c"), "");            // dot before the space

#ifdef _WIN32
    EXPECT_STREQ(PathFindExtensionA("dir.x\\file"), "");      // dot lives in the dir part
#else
    EXPECT_STREQ(PathFindExtensionA("dir.x/file"), "");       // dot lives in the dir part
#endif
}
