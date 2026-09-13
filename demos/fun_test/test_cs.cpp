/**
 * swinx critical section / SRW lock tests.
 * CRITICAL_SECTION is recursive on Win32 - swinx backs it with
 * std::recursive_mutex, so the same semantics are asserted here.
 */

#include <gtest/gtest.h>
#include <windows.h>
// ------------------------------------------------------------------------
// Critical section
// ------------------------------------------------------------------------

TEST(swinx_cs, enter_leave)
{
    CRITICAL_SECTION cs;
    InitializeCriticalSection(&cs);

    EnterCriticalSection(&cs);
    LeaveCriticalSection(&cs);

    // re-acquirable after leave
    ASSERT_TRUE(TryEnterCriticalSection(&cs));
    LeaveCriticalSection(&cs);

    DeleteCriticalSection(&cs);
}

TEST(swinx_cs, recursive_enter)
{
    CRITICAL_SECTION cs;
    InitializeCriticalSection(&cs);

    EnterCriticalSection(&cs);
    EnterCriticalSection(&cs);
    ASSERT_TRUE(TryEnterCriticalSection(&cs));
    LeaveCriticalSection(&cs);
    LeaveCriticalSection(&cs);
    LeaveCriticalSection(&cs);

    DeleteCriticalSection(&cs);
}

TEST(swinx_cs, try_enter_unheld)
{
    CRITICAL_SECTION cs;
    InitializeCriticalSection(&cs);

    EXPECT_TRUE(TryEnterCriticalSection(&cs));
    LeaveCriticalSection(&cs);

    DeleteCriticalSection(&cs);
}

// 线程间用事件握手定序，而不是用 Sleep 去"估算"时间点：
//   1. 主线程先 EnterCriticalSection，再创建 worker —— worker 的第一次探测必然发生在
//      "锁被另一个线程持有"时；
//   2. worker 探测完置位 probedWhileHeld 并阻塞在 releaseNow 上 —— 主线程收到后释放锁
//      再置位 releaseNow，worker 的第二次探测必然发生在"锁已空闲"时。
// 旧实现靠 Sleep(100)/Sleep(250) 与主线程 Sleep(200) 三个时间点去凑顺序，只留了 150ms 余量：
// 只要主线程的 Sleep 过冲（机器繁忙时很常见），或 worker 的 Sleep 被信号打断而提前返回
// （swinx 的 Sleep 曾经忽略 select 的 EINTR，本进程还装着 SIGCHLD 处理器），
// 第二次探测就会在锁仍被持有时发生 → tryAfterRelease 读到 0，用例随机失败。
struct CsWorkerCtx {
    CRITICAL_SECTION *cs;
    HANDLE probedWhileHeld;     // 自动复位事件：worker 已完成"锁被他人持有"的探测
    HANDLE releaseNow;          // 手动复位事件：主线程已释放锁，可以再探测
    volatile LONG tryWhileHeld; // -1 unknown, 0 FALSE, 1 TRUE
    volatile LONG tryAfterRelease;
};

static DWORD WINAPI cs_worker(LPVOID p)
{
    CsWorkerCtx *c = (CsWorkerCtx *)p;
    c->tryWhileHeld = TryEnterCriticalSection(c->cs) ? 1 : 0;
    SetEvent(c->probedWhileHeld);
    // 等主线程明确释放（而不是猜它 200ms 后已经释放）
    WaitForSingleObject(c->releaseNow, 5000);
    c->tryAfterRelease = TryEnterCriticalSection(c->cs) ? 1 : 0;
    if (c->tryAfterRelease)
        LeaveCriticalSection(c->cs);
    return 0;
}

TEST(swinx_cs, try_enter_cross_thread)
{
    CRITICAL_SECTION cs;
    InitializeCriticalSection(&cs);

    HANDLE probedWhileHeld = CreateEventA(NULL, FALSE, FALSE, NULL); // auto-reset
    HANDLE releaseNow = CreateEventA(NULL, TRUE, FALSE, NULL);       // manual-reset
    ASSERT_TRUE(probedWhileHeld != NULL);
    ASSERT_TRUE(releaseNow != NULL);

    // 先持锁再启动 worker：第一次探测不可能抢在主线程持锁之前
    EnterCriticalSection(&cs);

    CsWorkerCtx ctx = { &cs, probedWhileHeld, releaseNow, -1, -1 };
    HANDLE th = CreateThread(NULL, 0, cs_worker, &ctx, 0, NULL);
    ASSERT_TRUE(th != NULL);

    // worker 完成探测才会置位，此刻锁一定还被主线程持有
    EXPECT_EQ(WaitForSingleObject(probedWhileHeld, 5000), WAIT_OBJECT_0);
    EXPECT_EQ(ctx.tryWhileHeld, 0);   // held by main at probe time

    LeaveCriticalSection(&cs);
    SetEvent(releaseNow);             // 此后 worker 的探测必然看到空闲的锁

    EXPECT_EQ(WaitForSingleObject(th, 5000), WAIT_OBJECT_0);
    EXPECT_EQ(ctx.tryAfterRelease, 1); // free afterwards

    CloseHandle(th);
    CloseHandle(releaseNow);
    CloseHandle(probedWhileHeld);
    DeleteCriticalSection(&cs);
}


#ifndef _WIN32
// ------------------------------------------------------------------------
// SRW lock
// ------------------------------------------------------------------------

TEST(swinx_srw, exclusive_acquire_release)
{
    SRWLOCK lock;
    InitializeSRWLock(&lock);

    AcquireSRWLockExclusive(&lock);
    ReleaseSRWLockExclusive(&lock);
    // re-acquirable after release
    AcquireSRWLockExclusive(&lock);
    ReleaseSRWLockExclusive(&lock);

    UninitializeSRWLock(&lock);
}

TEST(swinx_srw, shared_acquire_release)
{
    SRWLOCK lock;
    InitializeSRWLock(&lock);

    AcquireSRWLockShared(&lock);
    ReleaseSRWLockShared(&lock);
    AcquireSRWLockShared(&lock);
    ReleaseSRWLockShared(&lock);

    UninitializeSRWLock(&lock);
}

struct SrwSharedCtx {
    SRWLOCK *lock;
    HANDLE acquired;   // manual-reset event
    volatile LONG ok;
};

static DWORD WINAPI srw_shared_worker(LPVOID p)
{
    SrwSharedCtx *c = (SrwSharedCtx *)p;
    // shared locks allow concurrent readers: this must succeed
    // even while the main thread holds the shared lock
    AcquireSRWLockShared(c->lock);
    c->ok = 1;
    SetEvent(c->acquired);
    Sleep(50);
    ReleaseSRWLockShared(c->lock);
    return 0;
}

TEST(swinx_srw, shared_concurrent_readers)
{
    SRWLOCK lock;
    InitializeSRWLock(&lock);

    HANDLE evt = CreateEventA(NULL, TRUE, FALSE, NULL);
    ASSERT_TRUE(evt != NULL);

    SrwSharedCtx ctx = { &lock, evt, 0 };
    AcquireSRWLockShared(&lock);

    HANDLE th = CreateThread(NULL, 0, srw_shared_worker, &ctx, 0, NULL);
    ASSERT_TRUE(th != NULL);
    EXPECT_EQ(WaitForSingleObject(evt, 3000), WAIT_OBJECT_0);
    EXPECT_EQ(ctx.ok, 1);

    ReleaseSRWLockShared(&lock);
    EXPECT_EQ(WaitForSingleObject(th, 3000), WAIT_OBJECT_0);

    CloseHandle(th);
    CloseHandle(evt);
    UninitializeSRWLock(&lock);
}



// ------------------------------------------------------------------------
// SRWLock try-acquire variants
// ------------------------------------------------------------------------

TEST(swinx_srw, try_acquire_variants)
{
    SRWLOCK lock;
    InitializeSRWLock(&lock);

    // exclusive: second acquire attempt fails while held
    EXPECT_TRUE(TryAcquireSRWLockExclusive(&lock));
    EXPECT_FALSE(TryAcquireSRWLockExclusive(&lock));
    EXPECT_FALSE(TryAcquireSRWLockShared(&lock));
    ReleaseSRWLockExclusive(&lock);

    // shared: multiple readers coexist, exclusive is blocked
    EXPECT_TRUE(TryAcquireSRWLockShared(&lock));
    EXPECT_TRUE(TryAcquireSRWLockShared(&lock));
    EXPECT_FALSE(TryAcquireSRWLockExclusive(&lock));
    ReleaseSRWLockShared(&lock);
    ReleaseSRWLockShared(&lock);

    // fully released again
    EXPECT_TRUE(TryAcquireSRWLockExclusive(&lock));
    ReleaseSRWLockExclusive(&lock);

    UninitializeSRWLock(&lock);
}

#endif//_WIN32