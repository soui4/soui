/**
 * Unit tests for the kernel-side Win32 entry points swinx added for wke:
 * environment expansion, file attribute probing, thread accounting,
 * InterlockedExchange, VirtualProtect, LoadLibraryExW, the wait-handle API,
 * CryptGenRandom, GetAdaptersAddresses and the shared standard-cursor rule.
 *
 * Windows runs the same cases against the real API, so only the shared
 * contract is asserted. swinx-specific error codes and deliberate deviations
 * sit behind #ifndef _WIN32.
 */
#include <gtest/gtest.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <wincrypt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <vector>

namespace
{

class WritablePath
{
public:
    WritablePath(const char *utf8)
    {
        m_wide.reserve(512);
        for (const char *p = utf8; *p; p++)
            m_wide.push_back((wchar_t)(unsigned char)*p);
        m_wide.push_back(0);
    }
    const wchar_t *get() const { return &m_wide[0]; }
private:
    std::vector<wchar_t> m_wide;
};

// Build "<temp dir><leaf>" into out; returns FALSE when no temp dir is usable.
bool probeTempPath(char *out, size_t cb, const char *leaf)
{
    out[0] = 0;
    DWORD n = GetTempPathA((DWORD)cb, out);
    if (n == 0 || strlen(out) + strlen(leaf) + 1 > cb)
        return false;
    strcat(out, leaf);
    return true;
}

volatile LONG g_waitHits = 0;

void CALLBACK waitProbeCallback(PVOID context, BYTE fired)
{
    (void)context;
    (void)fired;
    InterlockedIncrement(&g_waitHits);
}

bool bytesDiffer(const BYTE *a, const BYTE *b, size_t n)
{
    for (size_t i = 0; i < n; i++)
        if (a[i] != b[i])
            return true;
    return false;
}

} // namespace

// ------------------------------------------------------------------------
// ExpandEnvironmentStringsW
// ------------------------------------------------------------------------

TEST(swinx_wke_sysapi, expand_environment_strings_substitutes_and_keeps_unknown)
{
    const wchar_t *name = L"SWINX_WKE_PROBE_VAR";
    ASSERT_TRUE(SetEnvironmentVariableW(name, L"probe-value") != FALSE);

    wchar_t out[64];
    memset(out, 0, sizeof(out));
    DWORD n = ExpandEnvironmentStringsW(L"%SWINX_WKE_PROBE_VAR%-tail", out, 64);
    EXPECT_GT(n, 0u);
    EXPECT_EQ((size_t)n, wcslen(out) + 1);
    EXPECT_TRUE(out[0] == L'p' && out[1] == L'r');

    // Unknown names are copied verbatim, % included.
    memset(out, 0, sizeof(out));
    n = ExpandEnvironmentStringsW(L"%SWINX_WKE_ABSENT_VAR%", out, 64);
    // Win32 returns the character count including the terminating NUL:
    // 22 characters of text + 1.
    EXPECT_EQ(n, 23u);
    EXPECT_TRUE(wcslen(out) + 1 == (size_t)n);

    // A buffer that is too small reports the required size instead of 0.
    memset(out, 0, sizeof(out));
    DWORD need = ExpandEnvironmentStringsW(L"%SWINX_WKE_PROBE_VAR%-tail", out, 2);
    EXPECT_GT(need, 2u);
    EXPECT_EQ(out[0], (wchar_t)0);

    SetEnvironmentVariableW(name, NULL);
}

// ------------------------------------------------------------------------
// GetFileAttributesExW
// ------------------------------------------------------------------------

TEST(swinx_wke_sysapi, file_attributes_ex_reports_a_regular_file)
{
    char path[MAX_PATH];
    if (!probeTempPath(path, sizeof(path), "swinx_wke_attr_probe.txt"))
    {
        SUCCEED() << "no usable temp path";
        return;
    }
    FILE *f = fopen(path, "wb");
    ASSERT_TRUE(f != NULL);
    fputs("0123456789", f);
    fclose(f);

    WIN32_FILE_ATTRIBUTE_DATA data;
    memset(&data, 0, sizeof(data));
    WritablePath wp(path);
    ASSERT_TRUE(GetFileAttributesExW(wp.get(), GetFileExInfoStandard, &data) != FALSE);
    EXPECT_FALSE(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    EXPECT_EQ(data.nFileSizeLow, 10u);
    EXPECT_EQ(data.nFileSizeHigh, 0u);


    remove(path);
}

TEST(swinx_wke_sysapi, file_attributes_ex_on_a_missing_file_fails)
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    memset(&data, 0, sizeof(data));
    WritablePath wp("./swinx-wke-absent-file-xyz.bin");
    EXPECT_FALSE(GetFileAttributesExW(wp.get(), GetFileExInfoStandard, &data));
}

// ------------------------------------------------------------------------
// GetThreadTimes
// ------------------------------------------------------------------------

TEST(swinx_wke_sysapi, thread_times_accepts_only_four_output_pointers)
{
    FILETIME creation, exit_, kernel, user;
    ASSERT_TRUE(GetThreadTimes(GetCurrentThread(), &creation, &exit_, &kernel, &user) != FALSE);
    // A live thread has no exit time: both implementations leave it zeroed, so
    // this is the shared part of the contract.
    EXPECT_EQ(exit_.dwLowDateTime | exit_.dwHighDateTime, 0u) << "a live thread has no exit time";
#ifndef _WIN32
    // swinx keeps no thread accounting at all; Windows fills in the creation
    // time from the thread object.
    EXPECT_EQ(creation.dwLowDateTime | creation.dwHighDateTime, 0u);
#endif
}

// ------------------------------------------------------------------------
// InterlockedExchange
// ------------------------------------------------------------------------

TEST(swinx_wke_sysapi, interlocked_exchange_returns_the_previous_value)
{
    LONG target = 7;
    EXPECT_EQ(InterlockedExchange(&target, 42), 7);
    EXPECT_EQ(target, 42);
    EXPECT_EQ(InterlockedExchange(&target, -1), 42);
    EXPECT_EQ(target, -1);
}

// ------------------------------------------------------------------------
// VirtualProtect
// ------------------------------------------------------------------------

TEST(swinx_wke_sysapi, virtual_protect_round_trip_on_allocated_memory)
{
    const size_t cb = 4096;
    LPVOID mem = VirtualAlloc(NULL, cb, MEM_COMMIT, PAGE_READWRITE);
    ASSERT_TRUE(mem != NULL);
    memset(mem, 0x5A, cb);

    DWORD old = 0;
    ASSERT_TRUE(VirtualProtect(mem, cb, PAGE_READONLY, &old) != FALSE);
    EXPECT_NE(old, 0u);
    EXPECT_EQ(((BYTE *)mem)[0], (BYTE)0x5A);

    ASSERT_TRUE(VirtualProtect(mem, cb, PAGE_READWRITE, &old) != FALSE);
    ((BYTE *)mem)[0] = 0x11;
    EXPECT_EQ(((BYTE *)mem)[0], (BYTE)0x11);
    EXPECT_TRUE(VirtualFree(mem, 0, MEM_RELEASE) != FALSE);
}

#ifndef _WIN32
TEST(swinx_wke_sysapi, virtual_protect_rejects_degenerate_arguments_in_swinx)
{
    DWORD old = 0;
    // swinx: NULL address or zero length fail without touching the page table.
    EXPECT_FALSE(VirtualProtect(NULL, 4096, PAGE_READONLY, &old));
    BYTE b = 0;
    EXPECT_FALSE(VirtualProtect(&b, 0, PAGE_READONLY, &old));
}
#endif

// ------------------------------------------------------------------------
// LoadLibraryExW
// ------------------------------------------------------------------------

TEST(swinx_wke_sysapi, load_library_ex_resolves_the_same_module_twice)
{
    char exe[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, exe, sizeof(exe) - 1);
    if (n == 0)
    {
        SUCCEED() << "cannot locate the running image";
        return;
    }
    exe[n] = 0;
    WritablePath wp(exe);

    // Loading the image that is already resident must hand back the same
    // handle (dlopen / LoadLibraryEx both keep a reference count).
    HMODULE first = LoadLibraryExW(wp.get(), NULL, 0);
    if (!first)
    {
        SUCCEED() << "the running image is not dlopen-able on this platform";
        return;
    }
    HMODULE second = LoadLibraryExW(wp.get(), NULL, 0);
    EXPECT_TRUE(second == first);
}

// ------------------------------------------------------------------------
// RegisterWaitForSingleObject
// ------------------------------------------------------------------------

TEST(swinx_wke_sysapi, register_wait_rejects_null_arguments)
{
    HANDLE h = NULL;
    HANDLE ev = CreateEventA(NULL, FALSE, FALSE, NULL);
    ASSERT_TRUE(ev != NULL);

    EXPECT_FALSE(RegisterWaitForSingleObject(&h, NULL, waitProbeCallback, NULL, 10, 0));
    EXPECT_FALSE(RegisterWaitForSingleObject(&h, ev, NULL, NULL, 10, 0));
    EXPECT_TRUE(CloseHandle(ev) != FALSE);
}

TEST(swinx_wke_sysapi, register_wait_fires_the_callback_once)
{
    HANDLE h = NULL;
    HANDLE ev = CreateEventA(NULL, TRUE, FALSE, NULL);
    ASSERT_TRUE(ev != NULL);
    InterlockedExchange(&g_waitHits, 0);

    ASSERT_TRUE(RegisterWaitForSingleObject(&h, ev, waitProbeCallback, NULL, 500,
                                            WT_EXECUTEONLYONCE) != FALSE);
    ASSERT_TRUE(h != NULL);
    SetEvent(ev);

    // Give the wait thread a chance to run the callback, then unregister.
    Sleep(400);
    EXPECT_GE(InterlockedExchangeAdd(&g_waitHits, 0), 1);
    EXPECT_TRUE(UnregisterWait(h) != FALSE);
    EXPECT_TRUE(CloseHandle(ev) != FALSE);
}

// ------------------------------------------------------------------------
// CryptAcquireContextW / CryptGenRandom / CryptReleaseContext
// ------------------------------------------------------------------------

TEST(swinx_wke_crypt, verify_context_probe_and_random_bytes)
{
    HCRYPTPROV prov = 0;
    ASSERT_TRUE(CryptAcquireContextW(&prov, NULL, NULL, PROV_RSA_FULL,
                                     CRYPT_VERIFYCONTEXT) != FALSE);
    ASSERT_TRUE(prov != 0);

    BYTE a[16], b[16];
    memset(a, 0, sizeof(a));
    memset(b, 0, sizeof(b));
    ASSERT_TRUE(CryptGenRandom(prov, sizeof(a), a) != FALSE);
    ASSERT_TRUE(CryptGenRandom(prov, sizeof(b), b) != FALSE);
    EXPECT_TRUE(bytesDiffer(a, b, sizeof(a)));

    // Zero length is a no-op that must not touch the buffer. The Win32 CSP
    // refuses the call instead of treating it as a no-op.
#ifdef _WIN32
    EXPECT_FALSE(CryptGenRandom(prov, 0, NULL));
#else
    EXPECT_TRUE(CryptGenRandom(prov, 0, NULL) != FALSE);
#endif

    EXPECT_TRUE(CryptReleaseContext(prov, 0) != FALSE);
}

#ifndef _WIN32
TEST(swinx_wke_crypt, swinx_error_codes_for_bad_handles_and_containers)
{
    HCRYPTPROV prov = 0;
    // swinx has no key containers: asking for one without CRYPT_VERIFYCONTEXT
    // must fail instead of silently ignoring the name.
    EXPECT_FALSE(CryptAcquireContextW(&prov, L"swinx-probe-container", NULL,
                                      PROV_RSA_FULL, 0));
    EXPECT_EQ(GetLastError(), (DWORD)NTE_BAD_KEYSET);

    EXPECT_FALSE(CryptAcquireContextW(&prov, NULL, NULL, 0x7F, CRYPT_VERIFYCONTEXT));
    EXPECT_EQ(GetLastError(), (DWORD)NTE_BAD_PROV_TYPE);

    BYTE buf[4];
    EXPECT_FALSE(CryptGenRandom((HCRYPTPROV)(UINT_PTR)0xBADC0DE, sizeof(buf), buf));
    EXPECT_FALSE(CryptGenRandom(prov, 4, NULL));
    EXPECT_FALSE(CryptReleaseContext((HCRYPTPROV)(UINT_PTR)0xBADC0DE, 0));
    (void)prov;
}
#endif

// ------------------------------------------------------------------------
// GetAdaptersAddresses
// ------------------------------------------------------------------------

TEST(swinx_wke_iphlpapi, get_adapters_addresses_probe_then_read)
{
    ULONG needed = 0;
    ULONG rc = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, NULL, &needed);
    ASSERT_TRUE(rc == ERROR_BUFFER_OVERFLOW || rc == ERROR_SUCCESS || rc == ERROR_NO_DATA)
        << "rc=" << (unsigned long)rc;
    if (rc == ERROR_NO_DATA)
    {
        SUCCEED() << "no network interface on this host";
        return;
    }
    ASSERT_GT(needed, 0u);

    std::vector<BYTE> buf(needed + 64, 0);
    IP_ADAPTER_ADDRESSES *adapters = (IP_ADAPTER_ADDRESSES *)&buf[0];
    ULONG size = (ULONG)buf.size();
    rc = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, adapters, &size);
    if (rc == ERROR_BUFFER_OVERFLOW)
    {
        size = needed;
        buf.resize(needed);
        adapters = (IP_ADAPTER_ADDRESSES *)&buf[0];
        rc = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, adapters, &size);
    }
    ASSERT_EQ(rc, (DWORD)ERROR_SUCCESS);
    ASSERT_GE(size, (ULONG)sizeof(IP_ADAPTER_ADDRESSES));
    ASSERT_TRUE(adapters != NULL);

    int count = 0;
    for (IP_ADAPTER_ADDRESSES *p = adapters; p != NULL; p = p->Next)
    {
        count++;
        EXPECT_TRUE(p->AdapterName != NULL);
        EXPECT_GT(p->IfIndex + p->Ipv6IfIndex, 0u);
        if (count > 32)
            break;
    }
    EXPECT_GE(count, 1);

    EXPECT_EQ(GetAdaptersAddresses(AF_UNSPEC, 0, NULL, adapters, NULL),
              (DWORD)ERROR_INVALID_PARAMETER);
}

// ------------------------------------------------------------------------
// EnumDisplaySettingsW
// ------------------------------------------------------------------------

TEST(swinx_wke_multimon, enum_display_settings_argument_validation)
{

    DEVMODEW dm;
    memset(&dm, 0, sizeof(dm));
    dm.dmSize = (WORD)sizeof(dm);
    EXPECT_TRUE(EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &dm));
#ifndef _WIN32
    // swinx keeps no mode table: anything but the two sentinels ends the list.
    EXPECT_FALSE(EnumDisplaySettingsW(NULL, 9999, &dm));
#else
    (void)dm;
#endif
}

TEST(swinx_wke_multimon, enum_display_settings_current_mode)
{
    DEVMODEW dm;
    memset(&dm, 0, sizeof(dm));
    dm.dmSize = (WORD)sizeof(dm);
    if (!EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &dm))
    {
        SUCCEED() << "no display attached";
        return;
    }
    EXPECT_GT(dm.dmPelsWidth, 0u);
    EXPECT_GT(dm.dmPelsHeight, 0u);
    // Both must report a size that fits the buffer the caller handed over, but
    // GDI only fills in the display portion (188 on Win10) while swinx reports
    // the full DEVMODEW.
    EXPECT_GT(dm.dmSize, 0u);
    EXPECT_LE(dm.dmSize, (WORD)sizeof(DEVMODEW));
#ifndef _WIN32
    EXPECT_EQ(dm.dmSize, (WORD)sizeof(DEVMODEW));
#endif
}

// ------------------------------------------------------------------------
// LoadCursor 的"共享标准光标"契约
//
// wke 的 SharedCursor 会把 LoadCursor() 返回的句柄无条件交给 DestroyIcon /
// DestroyCursor。Windows 让这类预定义光标的销毁失败，swinx 由 CursorMgr 查自己的
// 缓存做同样的守卫；而 ~CursorMgr 要靠同一份缓存在退出时释放这些对象，所以守卫
// 只能命中缓存里的标准光标，不能变成"一概不释放"。
// ------------------------------------------------------------------------

TEST(swinx_wke_cursor, standard_cursor_survives_unconditional_destroy)
{
    HCURSOR first = LoadCursor(NULL, IDC_ARROW);
    ASSERT_TRUE(first != NULL);

    // Win32 documents DestroyCursor as failing for a predefined cursor, but
    // Win10's GDI returns TRUE and simply keeps the shared object alive, so the
    // refusal itself is only asserted where swinx owns the cache.
#ifndef _WIN32
    EXPECT_FALSE(DestroyCursor(first));
#endif
    // Asking again must hand back the very same (still shared) object.
    EXPECT_EQ(LoadCursor(NULL, IDC_ARROW), first);

#ifndef _WIN32
    // swinx routes DestroyIcon through the same guard; the return value of the
    // real DestroyIcon for a shared handle is left unasserted.
    EXPECT_FALSE(DestroyIcon((HICON)first));
    ICONINFO info;
    memset(&info, 0, sizeof(info));
    ASSERT_TRUE(GetIconInfo((HICON)first, &info)); // the refused destroy freed nothing
    if (info.hbmColor)
        DeleteObject(info.hbmColor);
    if (info.hbmMask)
        DeleteObject(info.hbmMask);
#endif
}

TEST(swinx_wke_cursor, caller_owned_icon_is_destroyable)
{
    // The counter-case: an object the caller created must still be freed, so
    // the guard above is scoped to the standard-cursor cache.
    HBITMAP mask = CreateBitmap(8, 8, 1, 1, NULL);
    HBITMAP color = CreateBitmap(8, 8, 1, 32, NULL);
    ASSERT_TRUE(mask != NULL && color != NULL);

    ICONINFO info;
    memset(&info, 0, sizeof(info));
    info.fIcon = TRUE;
    info.hbmMask = mask;
    info.hbmColor = color;
    HICON icon = CreateIconIndirect(&info);
    ASSERT_TRUE(icon != NULL);
    EXPECT_TRUE(icon != (HICON)LoadCursor(NULL, IDC_ARROW));

    EXPECT_TRUE(DestroyIcon(icon) != FALSE);
    DeleteObject(mask);
    DeleteObject(color);
}
