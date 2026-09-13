/**
 * swinx kernel sync object tests (events / mutex / semaphore / interlocked).
 * These tests run against the swinx Win32-compat layer on non-Windows
 * platforms; on Windows they are skipped (the real API is exercised there).
 */
#include <gtest/gtest.h>
#include <windows.h>
#include <string>
#include <vector>
#include <helper/SSemaphore.h>

static std::string uniqueName(const char *prefix)
{
    char buf[128];
    sprintf(buf, "%s_%u", prefix, GetCurrentProcessId());
    return buf;
}

static DWORD WINAPI set_event_thread(LPVOID p)
{
    HANDLE evt = (HANDLE)p;
    Sleep(100);
    SetEvent(evt);
    return 0;
}

struct MutexCtx {
    HANDLE mutex;
    HANDLE acquired;
    HANDLE releaseNow; // 主线程置位后 worker 才释放互斥锁
};

static DWORD WINAPI hold_and_release_mutex(LPVOID p)
{
    MutexCtx *c = (MutexCtx *)p;
    // 一直持有，直到主线程用事件明确通知释放：这样"持有期间等待必然超时"就不依赖
    // 两侧 Sleep 的时间差（旧实现靠 worker 的 Sleep(300) 压过主线程 150ms 的探测窗口，
    // Sleep 被信号打断而提前返回时，主线程的探测就会拿到锁 → 用例随机失败）。
    if (WaitForSingleObject(c->mutex, 1000) == WAIT_OBJECT_0)
        SetEvent(c->acquired);
    WaitForSingleObject(c->releaseNow, 5000);
    ReleaseMutex(c->mutex);
    return 0;
}

// ------------------------------------------------------------------------
// Events
// ------------------------------------------------------------------------

TEST(swinx_sync, event_manual_reset)
{
    HANDLE h = CreateEventA(NULL, TRUE, FALSE, NULL);
    ASSERT_TRUE(h != NULL);

    EXPECT_EQ(WaitForSingleObject(h, 0), WAIT_TIMEOUT);

    EXPECT_TRUE(SetEvent(h));
    // manual-reset event stays signaled for multiple waits
    EXPECT_EQ(WaitForSingleObject(h, 0), WAIT_OBJECT_0);
    EXPECT_EQ(WaitForSingleObject(h, 0), WAIT_OBJECT_0);

    EXPECT_TRUE(ResetEvent(h));
    EXPECT_EQ(WaitForSingleObject(h, 0), WAIT_TIMEOUT);

    CloseHandle(h);
}

TEST(swinx_sync, event_auto_reset)
{
    HANDLE h = CreateEventA(NULL, FALSE, FALSE, NULL);
    ASSERT_TRUE(h != NULL);

    SetEvent(h);
    EXPECT_EQ(WaitForSingleObject(h, 0), WAIT_OBJECT_0);
    // auto-reset event is consumed by a single successful wait
    EXPECT_EQ(WaitForSingleObject(h, 0), WAIT_TIMEOUT);

    CloseHandle(h);
}

TEST(swinx_sync, event_initial_state)
{
    HANDLE h = CreateEventA(NULL, TRUE, TRUE, NULL);
    ASSERT_TRUE(h != NULL);
    EXPECT_EQ(WaitForSingleObject(h, 0), WAIT_OBJECT_0);
    CloseHandle(h);
}

TEST(swinx_sync, event_named_open)
{
    std::string name = uniqueName("swinx_evt_test");

    HANDLE h1 = CreateEventA(NULL, TRUE, FALSE, name.c_str());
    ASSERT_TRUE(h1 != NULL);

    HANDLE h2 = OpenEventA(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, name.c_str());
    ASSERT_TRUE(h2 != NULL);

    // signaling through the second handle wakes a waiter on the first
    SetEvent(h2);
    EXPECT_EQ(WaitForSingleObject(h1, 0), WAIT_OBJECT_0);

    CloseHandle(h2);
    CloseHandle(h1);
}

TEST(swinx_sync, event_cross_thread)
{
    HANDLE h = CreateEventA(NULL, TRUE, FALSE, NULL);
    ASSERT_TRUE(h != NULL);

    // pass the event handle itself (not a pointer to it) - the thread
    // function treats LPVOID as HANDLE
    HANDLE th = CreateThread(NULL, 0, set_event_thread, h, 0, NULL);
    ASSERT_TRUE(th != NULL);

    DWORD ts0 = GetTickCount();
    EXPECT_EQ(WaitForSingleObject(h, 3000), WAIT_OBJECT_0);
    EXPECT_GE(GetTickCount() - ts0, 50u);

    EXPECT_EQ(WaitForSingleObject(th, 3000), WAIT_OBJECT_0);
    CloseHandle(th);
    CloseHandle(h);
}

TEST(swinx_sync, event_wait_timeout)
{
    HANDLE h = CreateEventA(NULL, TRUE, FALSE, NULL);
    ASSERT_TRUE(h != NULL);
    DWORD ts0 = GetTickCount();
    EXPECT_EQ(WaitForSingleObject(h, 120), WAIT_TIMEOUT);
    EXPECT_GE(GetTickCount() - ts0, 100u);
    CloseHandle(h);
}

// ------------------------------------------------------------------------
// WaitForMultipleObjects
// ------------------------------------------------------------------------

TEST(swinx_sync, wait_multiple_all)
{
    HANDLE evts[2] = {
        CreateEventA(NULL, TRUE, FALSE, NULL),
        CreateEventA(NULL, TRUE, FALSE, NULL),
    };
    ASSERT_TRUE(evts[0] != NULL && evts[1] != NULL);

    // wait-all times out until both are signaled
    SetEvent(evts[0]);
    EXPECT_EQ(WaitForMultipleObjects(2, evts, TRUE, 100), WAIT_TIMEOUT);
    SetEvent(evts[1]);
    EXPECT_EQ(WaitForMultipleObjects(2, evts, TRUE, 100), WAIT_OBJECT_0);

    CloseHandle(evts[0]);
    CloseHandle(evts[1]);
}

TEST(swinx_sync, wait_multiple_any)
{
    HANDLE evts[2] = {
        CreateEventA(NULL, TRUE, FALSE, NULL),
        CreateEventA(NULL, TRUE, FALSE, NULL),
    };
    ASSERT_TRUE(evts[0] != NULL && evts[1] != NULL);

    SetEvent(evts[1]);
    // returns the index of the first signaled object
    EXPECT_EQ(WaitForMultipleObjects(2, evts, FALSE, 100), WAIT_OBJECT_0 + 1);

    CloseHandle(evts[0]);
    CloseHandle(evts[1]);
}

// ------------------------------------------------------------------------
// Mutex
// ------------------------------------------------------------------------

TEST(swinx_sync, mutex_ownership)
{
    HANDLE m = CreateMutexA(NULL, FALSE, NULL);
    ASSERT_TRUE(m != NULL);

    EXPECT_EQ(WaitForSingleObject(m, 100), WAIT_OBJECT_0);
    EXPECT_TRUE(ReleaseMutex(m));
    // released: re-acquirable
    EXPECT_EQ(WaitForSingleObject(m, 100), WAIT_OBJECT_0);
    EXPECT_TRUE(ReleaseMutex(m));

    // note: swinx mutexes are pipe-based and not owner-recursive like Win32;
    // a second wait by the owning thread would block, so it is not tested here

    CloseHandle(m);
}

TEST(swinx_sync, mutex_cross_thread)
{
    HANDLE m = CreateMutexA(NULL, FALSE, NULL);
    ASSERT_TRUE(m != NULL);

    HANDLE acquired = CreateEventA(NULL, TRUE, FALSE, NULL);
    ASSERT_TRUE(acquired != NULL);
    HANDLE releaseNow = CreateEventA(NULL, TRUE, FALSE, NULL);
    ASSERT_TRUE(releaseNow != NULL);

    MutexCtx ctx = { m, acquired, releaseNow };

    HANDLE th = CreateThread(NULL, 0, hold_and_release_mutex, &ctx, 0, NULL);
    ASSERT_TRUE(th != NULL);

    // wait until the worker holds the mutex
    EXPECT_EQ(WaitForSingleObject(acquired, 3000), WAIT_OBJECT_0);

    // the mutex is held by the worker thread (it is blocked on releaseNow,
    // so it cannot release behind our back): wait times out
    EXPECT_EQ(WaitForSingleObject(m, 150), WAIT_TIMEOUT);

    // after the worker releases, we can acquire it
    SetEvent(releaseNow);
    EXPECT_EQ(WaitForSingleObject(m, 3000), WAIT_OBJECT_0);
    ReleaseMutex(m);

    EXPECT_EQ(WaitForSingleObject(th, 3000), WAIT_OBJECT_0);
    CloseHandle(th);
    CloseHandle(releaseNow);
    CloseHandle(acquired);
    CloseHandle(m);
}

TEST(swinx_sync, mutex_named_open)
{
    std::string name = uniqueName("swinx_mtx_test");

    HANDLE m1 = CreateMutexA(NULL, FALSE, name.c_str());
    ASSERT_TRUE(m1 != NULL);

    HANDLE m2 = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, name.c_str());
    ASSERT_TRUE(m2 != NULL);

    // acquire through one handle, release through the other (same thread)
    EXPECT_EQ(WaitForSingleObject(m1, 100), WAIT_OBJECT_0);
    EXPECT_TRUE(ReleaseMutex(m2));
    EXPECT_EQ(WaitForSingleObject(m2, 100), WAIT_OBJECT_0);
    ReleaseMutex(m1);

    CloseHandle(m2);
    CloseHandle(m1);
}

// ------------------------------------------------------------------------
// Semaphore
// ------------------------------------------------------------------------

TEST(swinx_sync, semaphore_counting)
{
    HANDLE s = CreateSemaphoreA(NULL, 0, 2, NULL);
    ASSERT_TRUE(s != NULL);

    EXPECT_EQ(WaitForSingleObject(s, 0), WAIT_TIMEOUT);

    LONG prev = -1;
    EXPECT_TRUE(ReleaseSemaphore(s, 2, &prev));
    EXPECT_EQ(prev, 0);

    EXPECT_EQ(WaitForSingleObject(s, 0), WAIT_OBJECT_0);
    EXPECT_EQ(WaitForSingleObject(s, 0), WAIT_OBJECT_0);
    EXPECT_EQ(WaitForSingleObject(s, 0), WAIT_TIMEOUT);

    CloseHandle(s);
}

TEST(swinx_sync, semaphore_initial_count)
{
    HANDLE s = CreateSemaphoreA(NULL, 2, 2, NULL);
    ASSERT_TRUE(s != NULL);
    EXPECT_EQ(WaitForSingleObject(s, 0), WAIT_OBJECT_0);
    EXPECT_EQ(WaitForSingleObject(s, 0), WAIT_OBJECT_0);
    EXPECT_EQ(WaitForSingleObject(s, 0), WAIT_TIMEOUT);
    CloseHandle(s);
}

TEST(swinx_sync, semaphore_named_open)
{
    std::string name = uniqueName("swinx_sem_test");

    HANDLE s1 = CreateSemaphoreA(NULL, 0, 2, name.c_str());
    ASSERT_TRUE(s1 != NULL);

    HANDLE s2 = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, name.c_str());
    ASSERT_TRUE(s2 != NULL);

    // signaling through the opened handle is visible on the original
    LONG prev = -1;
    EXPECT_TRUE(ReleaseSemaphore(s2, 1, &prev));
    EXPECT_EQ(prev, 0);
    EXPECT_EQ(WaitForSingleObject(s1, 0), WAIT_OBJECT_0);
    EXPECT_EQ(WaitForSingleObject(s1, 0), WAIT_TIMEOUT);

    CloseHandle(s2);
    CloseHandle(s1);
}

// ------------------------------------------------------------------------
// Interlocked operations
// ------------------------------------------------------------------------

TEST(swinx_sync, interlocked)
{
    LONG v = 41;
    EXPECT_EQ(InterlockedIncrement(&v), 42);
    EXPECT_EQ(v, 42);

    LONGLONG v64 = 0x7ffffffffffffffeLL;
    EXPECT_EQ(InterlockedIncrement64(&v64), 0x7fffffffffffffffLL);
    EXPECT_EQ(v64, 0x7fffffffffffffffLL);
}

// 迁自 test.cpp（原 soui.semaphore）：SOUI 封装的 SSemaphore 助手类基本可用性
TEST(swinx_sync, soui_semaphore_notify)
{
    SOUI::SSemaphore semaphore;
    semaphore.notify();
}
