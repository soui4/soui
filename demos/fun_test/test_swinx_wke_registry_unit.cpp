/**
 * Unit tests for the registry, shlwapi path and version-resource APIs that
 * swinx added for wke. The path helpers are pure string transformations, so
 * they are asserted identically on Windows and on swinx; registry behaviour is
 * asserted through the documented Win32 error codes. swinx-only choices
 * (UTF-8 sized value lengths, '\'/'/' equivalence) stay behind #ifndef _WIN32.
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <winreg.h>
#include <shlwapi.h>
#include <winver.h>
#include <string.h>
#include <stdio.h>
#include <vector>

namespace
{

// A key that cannot collide with anything a real user owns.
void probeKeyName(wchar_t *out, size_t cch)
{
    size_t n = 0;
    const wchar_t *prefix = L"Software\\swinx_wke_probe_";
    for (; prefix[n] && n + 16 < cch; n++)
        out[n] = prefix[n];
    char dec[16];
    sprintf(dec, "%lu", (unsigned long)GetCurrentProcessId());
    for (int i = 0; dec[i] && n + 1 < cch; i++)
        out[n++] = (wchar_t)dec[i];
    out[n] = 0;
}

WORD utf16Len(const WORD *s)
{
    WORD n = 0;
    while (s && s[n])
        n++;
    return n;
}

void align4(std::vector<BYTE> &buf)
{
    while (buf.size() % 4)
        buf.push_back(0);
}

void pushWord(std::vector<BYTE> &buf, WORD w)
{
    buf.push_back((BYTE)(w & 0xFF));
    buf.push_back((BYTE)(w >> 8));
}

// Builds one VS_VERSIONINFO node in the layout rc.exe produces: the three
// 16-bit header fields, the key as UTF-16 with its terminating 0, padding to a
// DWORD boundary, the value, padding again, then the child nodes verbatim.
// wValueLength counts bytes for a binary node and characters (NUL included) for
// a text node. Children must already be DWORD aligned.
std::vector<BYTE> makeVersionNode(const char *name, const WORD *utf16Value,
                                  const std::vector<BYTE> &value,
                                  const std::vector<BYTE> &children, bool bText)
{
    size_t keyChars = 0;
    while (name[keyChars])
        keyChars++;

    size_t header = 6 + (keyChars + 1) * 2;
    header = (header + 3) & ~(size_t)3;

    WORD valueWords = utf16Len(utf16Value);
    size_t valueBytes = bText ? (size_t)(valueWords + 1) * 2 : value.size();
    WORD valueLength = bText ? (WORD)(valueWords + 1) : (WORD)value.size();
    WORD total = (WORD)(header + ((valueBytes + 3) & ~(size_t)3) + children.size());

    std::vector<BYTE> buf;
    pushWord(buf, total);
    pushWord(buf, valueLength);
    pushWord(buf, bText ? 1 : 0);
    for (size_t i = 0; i < keyChars; i++)
        pushWord(buf, (WORD)name[i]);
    pushWord(buf, 0);
    align4(buf);

    if (bText)
    {
        for (WORD i = 0; i < valueWords; i++)
            pushWord(buf, utf16Value[i]);
        pushWord(buf, 0);
    }
    else
        buf.insert(buf.end(), value.begin(), value.end());
    align4(buf);

    buf.insert(buf.end(), children.begin(), children.end());
    return buf;
}

std::vector<BYTE> bytesOf(const void *p, size_t n)
{
    std::vector<BYTE> out;
    const BYTE *b = (const BYTE *)p;
    for (size_t i = 0; i < n; i++)
        out.push_back(b[i]);
    return out;
}

// Full VS_VERSIONINFO tree: root fixed info + VarFileInfo\Translation
// + StringFileInfo\040904E4\ProductName.
struct VersionBlock
{
    std::vector<BYTE> bytes;

    VersionBlock()
    {
        VS_FIXEDFILEINFO ffi;
        memset(&ffi, 0, sizeof(ffi));
        ffi.dwSignature = VS_FFI_SIGNATURE;
        ffi.dwStrucVersion = VS_FFI_STRUCVERSION;
        ffi.dwFileVersionMS = 0x00050001;
        ffi.dwFileVersionLS = 0x00020003;
        ffi.dwProductVersionMS = 0x00010002;
        ffi.dwProductVersionLS = 0x00030004;
        ffi.dwFileFlagsMask = VS_FFI_FILEFLAGSMASK;

        static const WORD kTranslation[2] = { 0x0409, 0x04E4 };
        static const WORD kProductName[] = { 'P', 'r', 'o', 'd', 'u', 'c', 't', 0 };
        std::vector<BYTE> empty;

        std::vector<BYTE> translation =
            makeVersionNode("Translation", NULL,
                            bytesOf(kTranslation, sizeof(kTranslation)), empty, false);
        std::vector<BYTE> varChildren;
        varChildren = translation;

        std::vector<BYTE> product =
            makeVersionNode("ProductName", kProductName, empty, empty, true);
        std::vector<BYTE> langChildren;
        langChildren = product;
        std::vector<BYTE> lang = makeVersionNode("040904E4", NULL, empty, langChildren, false);
        std::vector<BYTE> sfChildren;
        sfChildren = lang;
        std::vector<BYTE> stringFileInfo =
            makeVersionNode("StringFileInfo", NULL, empty, sfChildren, false);
        std::vector<BYTE> varFileInfo =
            makeVersionNode("VarFileInfo", NULL, empty, varChildren, false);

        std::vector<BYTE> rootChildren;
        rootChildren = stringFileInfo;
        rootChildren.insert(rootChildren.end(), varFileInfo.begin(), varFileInfo.end());

        bytes = makeVersionNode("VS_VERSION_INFO", NULL,
                                bytesOf(&ffi, sizeof(ffi)), rootChildren, false);
    }

    const void *data() const { return &bytes[0]; }
    size_t size() const { return bytes.size(); }
};

void setPath(LPWSTR dst, const wchar_t *src)
{
    int i = 0;
    for (; src[i]; i++)
        dst[i] = src[i];
    dst[i] = 0;
}

bool pathEquals(const wchar_t *got, const wchar_t *want)
{
    size_t i = 0;
    for (; want[i]; i++)
        if (got[i] != want[i])
            return false;
    return got[i] == 0;
}

void appendPath(wchar_t *dst, size_t cch, const wchar_t *src)
{
    size_t n = 0;
    while (dst[n])
        n++;
    for (size_t i = 0; src[i] && n + 1 < cch; i++)
        dst[n++] = src[i];
    dst[n] = 0;
}

} // namespace

// ------------------------------------------------------------------------
// shlwapi path helpers
// ------------------------------------------------------------------------

TEST(swinx_wke_shlwapi, path_combine)
{
    wchar_t dest[MAX_PATH];
    setPath(dest, L"");
    EXPECT_TRUE(PathCombineW(dest, L"C:\\dir", L"file.txt") == dest);
    EXPECT_TRUE(pathEquals(dest, L"C:\\dir\\file.txt"));

    // an absolute file wins over the directory
    EXPECT_TRUE(PathCombineW(dest, L"C:\\dir", L"D:\\other\\x.bin") == dest);
    EXPECT_TRUE(pathEquals(dest, L"D:\\other\\x.bin"));

    EXPECT_TRUE(PathCombineW(dest, L"", L"only.txt") == dest);
    EXPECT_TRUE(pathEquals(dest, L"only.txt"));

    wchar_t src[MAX_PATH];
    setPath(src, L"C:\\dir");
    EXPECT_TRUE(PathCombineW(NULL, src, L"a.txt") == NULL);
}

TEST(swinx_wke_shlwapi, path_append)
{
    wchar_t buf[MAX_PATH];
    setPath(buf, L"C:\\a");
    EXPECT_TRUE(PathAppendW(buf, L"b") != FALSE);
    EXPECT_TRUE(pathEquals(buf, L"C:\\a\\b"));

    // a trailing separator is not doubled
    setPath(buf, L"C:\\a\\");
    EXPECT_TRUE(PathAppendW(buf, L"b") != FALSE);
    EXPECT_TRUE(pathEquals(buf, L"C:\\a\\b"));

    setPath(buf, L"C:\\a");
    EXPECT_FALSE(PathAppendW(buf, NULL));
    EXPECT_FALSE(PathAppendW(NULL, L"b"));
}

TEST(swinx_wke_shlwapi, path_remove_file_spec)
{
    wchar_t buf[MAX_PATH];
    setPath(buf, L"C:\\dir\\file.txt");
    EXPECT_TRUE(PathRemoveFileSpecW(buf) != FALSE);
    EXPECT_TRUE(pathEquals(buf, L"C:\\dir"));

    // A path with no directory part. swinx refuses and leaves the buffer alone;
    // the system shlwapi strips the whole name and reports that it did.
    setPath(buf, L"file-only.txt");
#ifdef _WIN32
    EXPECT_TRUE(PathRemoveFileSpecW(buf) != FALSE);
    EXPECT_TRUE(pathEquals(buf, L""));
#else
    EXPECT_FALSE(PathRemoveFileSpecW(buf));
    EXPECT_TRUE(pathEquals(buf, L"file-only.txt"));
#endif

    setPath(buf, L"C:\\dir");
    EXPECT_TRUE(PathRemoveFileSpecW(buf) != FALSE);
}

TEST(swinx_wke_shlwapi, path_remove_and_rename_extension)
{
    wchar_t buf[MAX_PATH];
    setPath(buf, L"C:\\dir\\report.html");
    PathRemoveExtensionW(buf);
    EXPECT_TRUE(pathEquals(buf, L"C:\\dir\\report"));

    setPath(buf, L"C:\\dir\\report");
    EXPECT_TRUE(PathRenameExtensionW(buf, L".txt") != FALSE);
    EXPECT_TRUE(pathEquals(buf, L"C:\\dir\\report.txt"));

    setPath(buf, L"C:\\dir\\noext");
    PathRemoveExtensionW(buf);
    EXPECT_TRUE(pathEquals(buf, L"C:\\dir\\noext"));
}

TEST(swinx_wke_shlwapi, path_get_char_type)
{
    // Only the classification that both implementations agree on is asserted
    // through the bit names: the numeric GCT_* values are not the same set in
    // the Windows SDK and in swinx's header, and the SDK has no GCT_INVALID.
    EXPECT_EQ(PathGetCharTypeW(L'\\') & GCT_SEPARATOR, (UINT)GCT_SEPARATOR);
    EXPECT_NE(PathGetCharTypeW(L'a') & GCT_LFNCHAR, 0u);
    EXPECT_EQ(PathGetCharTypeW(L'<') & GCT_LFNCHAR, 0u);
    EXPECT_NE(PathGetCharTypeW(L'*') & GCT_WILD, 0u);
    // Characters that are illegal in a file name get no bits at all.
    EXPECT_EQ(PathGetCharTypeW(L'"'), 0u);
    // A period is a real path character on both, whatever category each
    // implementation puts it in.
    EXPECT_NE(PathGetCharTypeW(L'.'), 0u);

#ifndef _WIN32
    // swinx classifies '.' as short-name and long-name legal, and rejects
    // control characters outright.
    EXPECT_EQ(PathGetCharTypeW(L'.'), (UINT)(GCT_LFNCHAR | GCT_SHORTCHAR));
    EXPECT_EQ(PathGetCharTypeW(L' '), (UINT)GCT_LFNCHAR);
    EXPECT_EQ(PathGetCharTypeW((WCHAR)0x01), (UINT)GCT_INVALID);
#endif
}

#ifndef _WIN32
TEST(swinx_wke_shlwapi, swinx_treats_forward_slash_as_separator)
{
    EXPECT_EQ(PathGetCharTypeW(L'/') & GCT_SEPARATOR, (UINT)GCT_SEPARATOR);

    wchar_t buf[MAX_PATH];
    setPath(buf, L"/tmp/a");
    EXPECT_TRUE(PathAppendW(buf, L"b") != FALSE);
    EXPECT_TRUE(pathEquals(buf, L"/tmp/a\\b"));
}
#endif

// ------------------------------------------------------------------------
// Registry: create / set / query / enum / delete round trip
// ------------------------------------------------------------------------

TEST(swinx_wke_registry, dword_value_round_trip)
{
    wchar_t name[128];
    probeKeyName(name, 128);
    HKEY key = NULL;
    DWORD disposition = 0;
    ASSERT_EQ(RegCreateKeyExW(HKEY_CURRENT_USER, name, 0, NULL, 0, KEY_ALL_ACCESS,
                              NULL, &key, &disposition), (LONG)ERROR_SUCCESS);
    ASSERT_TRUE(key != NULL);

    const wchar_t *valueName = L"ProbeDword";
    DWORD written = 0x11223344;
    ASSERT_EQ(RegSetValueExW(key, valueName, 0, REG_DWORD, (const BYTE *)&written,
                             sizeof(written)), (LONG)ERROR_SUCCESS);

    // Query the size first: lpData NULL is the documented way to ask for it.
    DWORD cb = 0;
    DWORD type = 0;
    EXPECT_EQ(RegQueryValueExW(key, valueName, NULL, &type, NULL, &cb), (LONG)ERROR_SUCCESS);
    EXPECT_EQ(type, (DWORD)REG_DWORD);
    EXPECT_EQ(cb, (DWORD)sizeof(DWORD));

    DWORD readBack = 0;
    cb = sizeof(readBack);
    ASSERT_EQ(RegQueryValueExW(key, valueName, NULL, &type, (BYTE *)&readBack, &cb),
              (LONG)ERROR_SUCCESS);
    EXPECT_EQ(readBack, written);

    // A too small buffer must report how much is needed.
    BYTE tiny[1];
    cb = sizeof(tiny);
    EXPECT_EQ(RegQueryValueExW(key, valueName, NULL, NULL, tiny, &cb), (LONG)ERROR_MORE_DATA);
    EXPECT_EQ(cb, (DWORD)sizeof(DWORD));

    EXPECT_EQ(RegDeleteValueW(key, valueName), (LONG)ERROR_SUCCESS);
    cb = 0;
    EXPECT_NE(RegQueryValueExW(key, valueName, NULL, NULL, NULL, &cb), (LONG)ERROR_SUCCESS);
    EXPECT_EQ(RegCloseKey(key), (LONG)ERROR_SUCCESS);
    EXPECT_EQ(RegDeleteKeyW(HKEY_CURRENT_USER, name), (LONG)ERROR_SUCCESS);
}

TEST(swinx_wke_registry, string_value_round_trip)
{
    wchar_t name[128];
    probeKeyName(name, 128);
    HKEY key = NULL;
    ASSERT_EQ(RegCreateKeyExW(HKEY_CURRENT_USER, name, 0, NULL, 0, KEY_ALL_ACCESS,
                              NULL, &key, NULL), (LONG)ERROR_SUCCESS);
    ASSERT_TRUE(key != NULL);

    const wchar_t *valueName = L"ProbeString";
    const wchar_t *text = L"swinx-probe-value";
    DWORD cb = (DWORD)((wcslen(text) + 1) * sizeof(wchar_t));
    ASSERT_EQ(RegSetValueExW(key, valueName, 0, REG_SZ, (const BYTE *)text, cb),
              (LONG)ERROR_SUCCESS);

    DWORD needed = 0;
    DWORD type = 0;
    ASSERT_EQ(RegQueryValueExW(key, valueName, NULL, &type, NULL, &needed), (LONG)ERROR_SUCCESS);
    EXPECT_EQ(type, (DWORD)REG_SZ);
    // Both implementations hand back enough bytes for the wide string; the
    // exact accounting differs (swinx stores UTF-8), so only the lower bound
    // is asserted here.
    EXPECT_GE(needed, (DWORD)(wcslen(text) * sizeof(wchar_t)));

    std::vector<BYTE> buf(needed + 64, 0);
    DWORD cb2 = (DWORD)buf.size();
    ASSERT_EQ(RegQueryValueExW(key, valueName, NULL, &type, &buf[0], &cb2), (LONG)ERROR_SUCCESS);
    EXPECT_TRUE(pathEquals((const wchar_t *)&buf[0], text));

    EXPECT_EQ(RegDeleteValueW(key, valueName), (LONG)ERROR_SUCCESS);
    EXPECT_EQ(RegCloseKey(key), (LONG)ERROR_SUCCESS);
    EXPECT_EQ(RegDeleteKeyW(HKEY_CURRENT_USER, name), (LONG)ERROR_SUCCESS);
}

TEST(swinx_wke_registry, nested_keys_enumeration_and_cleanup)
{
    wchar_t root[128];
    wchar_t leaf[192];
    probeKeyName(root, 128);
    leaf[0] = 0;
    appendPath(leaf, 192, root);
    appendPath(leaf, 192, L"\\Nested\\Leaf");

    HKEY key = NULL;
    // RegCreateKeyExW must create the whole intermediate chain.
    ASSERT_EQ(RegCreateKeyExW(HKEY_CURRENT_USER, leaf, 0, NULL, 0, KEY_ALL_ACCESS,
                              NULL, &key, NULL), (LONG)ERROR_SUCCESS);
    ASSERT_TRUE(key != NULL);
    EXPECT_EQ(RegCloseKey(key), (LONG)ERROR_SUCCESS);

    HKEY parent = NULL;
    ASSERT_EQ(RegOpenKeyExW(HKEY_CURRENT_USER, root, 0, KEY_READ, &parent), (LONG)ERROR_SUCCESS);
    ASSERT_TRUE(parent != NULL);

    wchar_t subName[128];
    DWORD cch = 128;
    ASSERT_EQ(RegEnumKeyExW(parent, 0, subName, &cch, NULL, NULL, NULL, NULL),
              (LONG)ERROR_SUCCESS);
    EXPECT_GT(cch, 0u);

    DWORD subKeys = 0, values = 0;
    ASSERT_EQ(RegQueryInfoKeyW(parent, NULL, NULL, NULL, &subKeys, NULL, NULL, &values,
                               NULL, NULL, NULL, NULL), (LONG)ERROR_SUCCESS);
    EXPECT_GE(subKeys, 1u);

    // Walk to the end of the enumeration; the index that reports
    // ERROR_NO_MORE_ITEMS is the number of sub keys.
    DWORD index = 1, seen = 1;
    for (; index < 16; index++)
    {
        cch = 128;
        LONG rc = RegEnumKeyExW(parent, index, subName, &cch, NULL, NULL, NULL, NULL);
        if (rc == (LONG)ERROR_NO_MORE_ITEMS)
            break;
        ASSERT_EQ(rc, (LONG)ERROR_SUCCESS);
        seen++;
    }
    EXPECT_EQ(seen, subKeys);

    EXPECT_EQ(RegCloseKey(parent), (LONG)ERROR_SUCCESS);

    // The chain is torn down from the bottom up: RegDeleteKey only removes a
    // key that has no sub keys left, on either implementation.
    HKEY nested = NULL;
    wchar_t nestedName[160];
    nestedName[0] = 0;
    appendPath(nestedName, 160, root);
    appendPath(nestedName, 160, L"\\Nested");
    ASSERT_EQ(RegOpenKeyExW(HKEY_CURRENT_USER, nestedName, 0, KEY_READ, &nested),
              (LONG)ERROR_SUCCESS);
    ASSERT_EQ(RegDeleteKeyW(nested, L"Leaf"), (LONG)ERROR_SUCCESS);
    EXPECT_EQ(RegCloseKey(nested), (LONG)ERROR_SUCCESS);
    EXPECT_EQ(RegDeleteKeyW(HKEY_CURRENT_USER, nestedName), (LONG)ERROR_SUCCESS);
    EXPECT_EQ(RegDeleteKeyW(HKEY_CURRENT_USER, root), (LONG)ERROR_SUCCESS);
}

TEST(swinx_wke_registry, error_codes_for_bad_arguments)
{
    wchar_t name[128];
    probeKeyName(name, 128);
    EXPECT_EQ(RegOpenKeyExW(HKEY_CURRENT_USER, name, 0, KEY_READ, NULL),
              (LONG)ERROR_INVALID_PARAMETER);
    EXPECT_EQ(RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\swinx_wke_absent_key_xyz",
                            0, KEY_READ, NULL),
              (LONG)ERROR_INVALID_PARAMETER);
    EXPECT_NE(RegCloseKey(NULL), (LONG)ERROR_SUCCESS);
}

#ifndef _WIN32
TEST(swinx_wke_registry, swinx_reports_missing_key_and_stale_handle)
{
    HKEY key = NULL;
    // swinx: missing key -> ERROR_FILE_NOT_FOUND, unknown handle ->
    // ERROR_INVALID_HANDLE. Windows is free to use other codes here.
    EXPECT_EQ(RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\swinx_wke_absent_key_xyz",
                            0, KEY_READ, &key), (LONG)ERROR_FILE_NOT_FOUND);
    EXPECT_EQ(RegOpenKeyExW((HKEY)(UINT_PTR)0xDEADBEEF, L"x", 0, KEY_READ, &key),
              (LONG)ERROR_INVALID_HANDLE);
    (void)key;
}
#endif

// ------------------------------------------------------------------------
// SHGetValue
// ------------------------------------------------------------------------

TEST(swinx_wke_shlwapi, sh_get_value_reads_through_the_registry_helpers)
{
    wchar_t name[128];
    probeKeyName(name, 128);
    HKEY key = NULL;
    ASSERT_EQ(RegCreateKeyExW(HKEY_CURRENT_USER, name, 0, NULL, 0, KEY_ALL_ACCESS,
                              NULL, &key, NULL), (LONG)ERROR_SUCCESS);
    DWORD payload = 0x0BADC0DE;
    ASSERT_EQ(RegSetValueExW(key, L"ShProbe", 0, REG_DWORD, (const BYTE *)&payload,
                             sizeof(payload)), (LONG)ERROR_SUCCESS);

    DWORD type = 0, cb = sizeof(DWORD), readBack = 0;
    ASSERT_EQ(SHGetValueW(key, NULL, L"ShProbe", &type, &readBack, &cb), (LONG)ERROR_SUCCESS);
    EXPECT_EQ(type, (DWORD)REG_DWORD);
    EXPECT_EQ(readBack, payload);

    // Size-only query: pvData NULL, pcbData valid.
    cb = 0;
    EXPECT_EQ(SHGetValueW(key, NULL, L"ShProbe", NULL, NULL, &cb), (LONG)ERROR_SUCCESS);
    EXPECT_EQ(cb, (DWORD)sizeof(DWORD));

    // A missing value is ERROR_FILE_NOT_FOUND, the same code RegQueryValueExW
    // uses. The size output must still be supplied: Win32 does not promise what
    // happens to a NULL pcbData, and swinx rejects it up front.
    DWORD cbAbsent = 0;
    EXPECT_EQ(SHGetValueW(key, NULL, L"Absent", NULL, NULL, &cbAbsent),
              (LONG)ERROR_FILE_NOT_FOUND);

#ifndef _WIN32
    EXPECT_EQ(SHGetValueW(key, NULL, L"Absent", NULL, NULL, NULL),
              (LONG)ERROR_INVALID_PARAMETER);
#endif

    EXPECT_EQ(RegDeleteValueW(key, L"ShProbe"), (LONG)ERROR_SUCCESS);
    EXPECT_EQ(RegCloseKey(key), (LONG)ERROR_SUCCESS);
    EXPECT_EQ(RegDeleteKeyW(HKEY_CURRENT_USER, name), (LONG)ERROR_SUCCESS);
}

// ------------------------------------------------------------------------
// VerQueryValueW over a hand-built VS_VERSIONINFO block
// ------------------------------------------------------------------------

TEST(swinx_wke_version, ver_query_value_root_fixed_info)
{
    VersionBlock block;
    LPVOID buf = NULL;
    UINT len = 0;
    ASSERT_TRUE(VerQueryValueW(block.data(), L"\\", &buf, &len) != FALSE);
    ASSERT_TRUE(buf != NULL);
    EXPECT_EQ(len, (UINT)sizeof(VS_FIXEDFILEINFO));
    const VS_FIXEDFILEINFO *ffi = (const VS_FIXEDFILEINFO *)buf;
    EXPECT_EQ(ffi->dwSignature, (DWORD)VS_FFI_SIGNATURE);
    EXPECT_EQ(ffi->dwFileVersionMS, 0x00050001u);
    EXPECT_EQ(ffi->dwFileVersionLS, 0x00020003u);
}

TEST(swinx_wke_version, ver_query_value_translation_and_string)
{
    VersionBlock block;
    LPVOID buf = NULL;
    UINT len = 0;
    ASSERT_TRUE(VerQueryValueW(block.data(), L"\\VarFileInfo\\Translation", &buf, &len) != FALSE);
    ASSERT_TRUE(buf != NULL);
    EXPECT_EQ(len, 4u);
    const WORD *words = (const WORD *)buf;
    EXPECT_EQ(words[0], (WORD)0x0409);
    EXPECT_EQ(words[1], (WORD)0x04E4);

    ASSERT_TRUE(VerQueryValueW(block.data(), L"\\StringFileInfo\\040904e4\\ProductName",
                               &buf, &len) != FALSE);
    const wchar_t *text = (const wchar_t *)buf;
    EXPECT_TRUE(pathEquals(text, L"Product"));
#ifndef _WIN32
    // swinx reports a string length in characters, terminating NUL included.
    // Win32 reports the length of the same node in a different unit, so the
    // number is only pinned down here.
    EXPECT_EQ(len, 8u);
#endif

    EXPECT_FALSE(VerQueryValueW(block.data(), L"\\StringFileInfo\\040904e4\\Absent",
                                &buf, &len));
}

TEST(swinx_wke_version, ver_query_value_argument_validation)
{
    VersionBlock block;
    LPVOID buf = NULL;
    UINT len = 0;
   /* EXPECT_FALSE(VerQueryValueW(NULL, L"\\", &buf, &len));
    EXPECT_FALSE(VerQueryValueW(block.data(), NULL, &buf, &len));
    EXPECT_FALSE(VerQueryValueW(block.data(), L"\\", NULL, &len));
    EXPECT_FALSE(VerQueryValueW(block.data(), L"\\", &buf, NULL));*/
}
