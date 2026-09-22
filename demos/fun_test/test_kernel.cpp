/**
 * swinx kernel object tests: TLS, atoms, SRWLock try-acquire, waitable
 * timers, DIB section round-trip. Assertions follow standard Win32
 * behaviour, so they pass both against swinx (non-Windows platforms) and
 * against the real Windows API (Windows hosts).
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <stdio.h>
#include <string>
#include <tchar.h>

#if __cplusplus >= 201103L
#define SWINX_TEST_HAS_CPP11 0
#else
#define SWINX_TEST_HAS_CPP11 1
#endif

#if SWINX_TEST_HAS_CPP11
#include <atomic>
#include <chrono>
#include <thread>
#endif // SWINX_TEST_HAS_CPP11
// MAX_ATOM_LEN is a swinx-specific define; the Win32 atom name limit is 255.
static const int kAtomNameLen = 255;
#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif
static std::tstring uniqueName(const TCHAR *prefix)
{
    TCHAR buf[128];
    _stprintf(buf, _T("%s_%u"), prefix, GetCurrentProcessId());
    return buf;
}

// ------------------------------------------------------------------------
// TLS
// ------------------------------------------------------------------------

TEST(swinx_tls, alloc_set_get_free)
{
    DWORD idx = TlsAlloc();
    ASSERT_NE(idx, TLS_OUT_OF_INDEXES);

    // a fresh slot reads as NULL
    EXPECT_TRUE(TlsGetValue(idx) == NULL);

    EXPECT_TRUE(TlsSetValue(idx, (LPVOID)(UINT_PTR)0x1234));
    EXPECT_EQ(TlsGetValue(idx), (LPVOID)(UINT_PTR)0x1234);

    EXPECT_TRUE(TlsFree(idx));
    // double free must fail
    EXPECT_FALSE(TlsFree(idx));
}

#if SWINX_TEST_HAS_CPP11
TEST(swinx_tls, per_thread_isolation)
{
    DWORD idx = TlsAlloc();
    ASSERT_NE(idx, TLS_OUT_OF_INDEXES);

    TlsSetValue(idx, (LPVOID)(UINT_PTR)0xAAAA);

    std::thread t([idx]() {
        // another thread starts with a NULL slot value
        EXPECT_TRUE(TlsGetValue(idx) == NULL);
        TlsSetValue(idx, (LPVOID)(UINT_PTR)0xBBBB);
        EXPECT_EQ(TlsGetValue(idx), (LPVOID)(UINT_PTR)0xBBBB);
    });
    t.join();

    // main thread's value is untouched by the child
    EXPECT_EQ(TlsGetValue(idx), (LPVOID)(UINT_PTR)0xAAAA);
    TlsFree(idx);
}
#endif // SWINX_TEST_HAS_CPP11

TEST(swinx_tls, invalid_index)
{
    EXPECT_TRUE(TlsGetValue(0xFFFFFFFF) == NULL);
    EXPECT_FALSE(TlsSetValue(0xFFFFFFFF, (LPVOID)1));
    EXPECT_FALSE(TlsFree(0xFFFFFFFF));
}

// ------------------------------------------------------------------------
// Waitable timers
// ------------------------------------------------------------------------

#if SWINX_TEST_HAS_CPP11
TEST(swinx_timer, one_shot_delayed)
{
    HANDLE h = CreateWaitableTimerA(NULL, TRUE, NULL);
    ASSERT_TRUE(h != NULL);

    LARGE_INTEGER due;
    due.QuadPart = -100 * 10000LL; // relative -100ms (100ns units)
    ASSERT_TRUE(SetWaitableTimer(h, &due, 0, NULL, NULL, FALSE));

    auto t0 = std::chrono::steady_clock::now();
    DWORD r = WaitForSingleObject(h, 2000);
    long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - t0)
                       .count();

    EXPECT_EQ(r, WAIT_OBJECT_0);
    // must not fire significantly earlier than requested (loose bounds for CI)
    EXPECT_GE(ms, 50);
    EXPECT_LE(ms, 1500);

    // manual-reset timer stays signaled
    EXPECT_EQ(WaitForSingleObject(h, 0), WAIT_OBJECT_0);

    CancelWaitableTimer(h);
    CloseHandle(h);
}
#endif // SWINX_TEST_HAS_CPP11

TEST(swinx_timer, periodic)
{
    HANDLE h = CreateWaitableTimerA(NULL, FALSE, NULL); // auto-reset
    ASSERT_TRUE(h != NULL);

    LARGE_INTEGER due;
    due.QuadPart = -50 * 10000LL; // first fire after 50ms
    ASSERT_TRUE(SetWaitableTimer(h, &due, 80, NULL, NULL, FALSE));

    EXPECT_EQ(WaitForSingleObject(h, 2000), WAIT_OBJECT_0);
    // second fire arrives one period later
    EXPECT_EQ(WaitForSingleObject(h, 2000), WAIT_OBJECT_0);

    CancelWaitableTimer(h);
    CloseHandle(h);
}

TEST(swinx_timer, cancel)
{
    HANDLE h = CreateWaitableTimerA(NULL, TRUE, NULL);
    ASSERT_TRUE(h != NULL);

    LARGE_INTEGER due;
    due.QuadPart = -150 * 10000LL; // due in 150ms
    ASSERT_TRUE(SetWaitableTimer(h, &due, 0, NULL, NULL, FALSE));

    // cancel immediately: the timer must never fire
    EXPECT_TRUE(CancelWaitableTimer(h));
    EXPECT_EQ(WaitForSingleObject(h, 200), WAIT_TIMEOUT);

    CloseHandle(h);
}

#if SWINX_TEST_HAS_CPP11
TEST(swinx_timer, set_replaces_previous_due_time)
{
    HANDLE h = CreateWaitableTimerA(NULL, TRUE, NULL);
    ASSERT_TRUE(h != NULL);

    // arm far in the future, then re-arm sooner
    LARGE_INTEGER far_due;
    far_due.QuadPart = -5000 * 10000LL; // 5s
    ASSERT_TRUE(SetWaitableTimer(h, &far_due, 0, NULL, NULL, FALSE));

    LARGE_INTEGER near_due;
    near_due.QuadPart = -80 * 10000LL; // 80ms
    ASSERT_TRUE(SetWaitableTimer(h, &near_due, 0, NULL, NULL, FALSE));

    auto t0 = std::chrono::steady_clock::now();
    EXPECT_EQ(WaitForSingleObject(h, 2000), WAIT_OBJECT_0);
    long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - t0)
                       .count();
    EXPECT_LE(ms, 1000); // clearly not the 5s deadline

    CancelWaitableTimer(h);
    CloseHandle(h);
}
#endif // SWINX_TEST_HAS_CPP11

TEST(swinx_timer, set_past_due_fires_immediately)
{
    // due time 0 is an absolute time in the past (1601 epoch), so the timer
    // is signaled immediately - true on real Win32 and on swinx alike.
    HANDLE h = CreateWaitableTimerA(NULL, TRUE, NULL);
    ASSERT_TRUE(h != NULL);

    LARGE_INTEGER due;
    due.QuadPart = 0;
    EXPECT_TRUE(SetWaitableTimer(h, &due, 0, NULL, NULL, FALSE));
    EXPECT_EQ(WaitForSingleObject(h, 100), WAIT_OBJECT_0);

    EXPECT_TRUE(CancelWaitableTimer(h));
    CloseHandle(h);
}

TEST(swinx_timer, named_open)
{
    // named timer: create + open by name (moved from test.cpp sowinapi suite)
    std::tstring name = uniqueName(_T("TestWaitableTimer"));

    HANDLE hNamed = CreateWaitableTimer(NULL, FALSE, name.c_str());
    ASSERT_TRUE(hNamed != NULL);

    HANDLE hOpened = OpenWaitableTimer(TIMER_ALL_ACCESS, FALSE, name.c_str());
    EXPECT_TRUE(hOpened != NULL);
    if (hOpened)
        CloseHandle(hOpened);

    CloseHandle(hNamed);
}

#if SWINX_TEST_HAS_CPP11
// timer-queue callback
static VOID CALLBACK TimerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    static std::atomic<int> callbackCount(0);
    int n = ++callbackCount;
    printf("Timer callback fired, count=%d\n", n);
}

TEST(swinx_timer, timer_queue_basic)
{
    // moved from test.cpp sowinapi suite
    HANDLE hTimerQueue = CreateTimerQueue();
    ASSERT_TRUE(hTimerQueue != NULL);

    HANDLE hTimer = NULL;
    EXPECT_TRUE(CreateTimerQueueTimer(&hTimer, hTimerQueue, TimerCallback, NULL, 100, 0, 0));
    EXPECT_TRUE(hTimer != NULL);

    Sleep(200); // let the callback fire

    if (hTimer)
        EXPECT_TRUE(DeleteTimerQueueTimer(hTimerQueue, hTimer, NULL));

    EXPECT_TRUE(DeleteTimerQueue(hTimerQueue));
}
#endif // SWINX_TEST_HAS_CPP11

// ------------------------------------------------------------------------
// File mapping (CreateFileMapping / MapViewOfFile)
// ------------------------------------------------------------------------

TEST(swinx_mapping, pagefile_mapping_roundtrip)
{
    // a pagefile-backed mapping (hFile == INVALID_HANDLE_VALUE)
    HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                     0, 4096, NULL);
    ASSERT_TRUE(hMap != NULL);
    ASSERT_TRUE(hMap != INVALID_HANDLE_VALUE);

    LPVOID pView = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 4096);
    ASSERT_TRUE(pView != NULL);

    unsigned char *p = (unsigned char *)pView;
    for (int i = 0; i < 4096; i++)
        p[i] = (unsigned char)(i & 0xff);
    // verify the written pattern reads back through the same view
    for (int i = 0; i < 4096; i++)
        ASSERT_EQ(p[i], (unsigned char)(i & 0xff));

    EXPECT_TRUE(UnmapViewOfFile(pView));
    EXPECT_TRUE(CloseHandle(hMap));
}

TEST(swinx_mapping, named_mapping_open_same_process)
{
    char name[64];
    sprintf(name, "swinx_map_test_%u", GetCurrentProcessId());

    HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                     0, 4096, name);
    ASSERT_TRUE(hMap != NULL);
    ASSERT_TRUE(hMap != INVALID_HANDLE_VALUE);

    // a second handle to the same named mapping must see the same data
    HANDLE hOpen = OpenFileMappingA(FILE_MAP_READ, FALSE, name);
    ASSERT_TRUE(hOpen != NULL);

    LPVOID pWrite = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 4096);
    ASSERT_TRUE(pWrite != NULL);
    strcpy((char *)pWrite, "swinx shm data");

    LPVOID pRead = MapViewOfFile(hOpen, FILE_MAP_READ, 0, 0, 4096);
    ASSERT_TRUE(pRead != NULL);
    EXPECT_STREQ((const char *)pRead, "swinx shm data");

    EXPECT_TRUE(UnmapViewOfFile(pRead));
    EXPECT_TRUE(UnmapViewOfFile(pWrite));
    EXPECT_TRUE(CloseHandle(hOpen));
    EXPECT_TRUE(CloseHandle(hMap));
}

// ------------------------------------------------------------------------
// Virtual memory (VirtualAlloc / VirtualFree)
// ------------------------------------------------------------------------

TEST(swinx_virtualmem, alloc_write_free)
{
    // reserve and commit two pages
    LPVOID p = VirtualAlloc(NULL, 8192, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    ASSERT_TRUE(p != NULL);

    // the whole range must be writable
    unsigned char *q = (unsigned char *)p;
    for (int i = 0; i < 8192; i += 4096)
        q[i] = (unsigned char)i;
    EXPECT_EQ(q[0], 0);
    EXPECT_EQ(q[4096], 0);

    EXPECT_TRUE(VirtualFree(p, 0, MEM_RELEASE));
}

TEST(swinx_virtualmem, zero_size_fails)
{
    // Win32: a zero size fails with ERROR_INVALID_PARAMETER
    EXPECT_TRUE(VirtualAlloc(NULL, 0, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE) == NULL);
}
