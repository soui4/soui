/*
 * probe_timer.cpp -- runtime probe for the swinx FreeRTOS host-emulation build.
 *
 * Scratch verification tool (not a repo target): links /tmp/build-frt/libswinx.a
 * and checks the two Win32 timer/pump contracts that the wasm fix exposed:
 *   T0  PeekMessage on an empty queue returns FALSE immediately (it does NOT wait)
 *   T1  SetTimer(NULL,0,elapse,TIMERPROC) + PeekMessage -> proc runs, no WM_TIMER dispatched
 *   T2  the same timer driven from a blocking GetMessage loop; proc posts WM_QUIT -> GetMessage returns FALSE
 *   T3  GetMessage with an empty queue sleeps until another thread posts (wall time >> CPU time)
 *
 * Timer behaviour is documented in swinx/doc/swinx-wasm-design.md section 4.3.
 */
#include <windows.h>
#include "SConnection.h"

#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

static volatile long g_calls = 0;
static volatile UINT_PTR g_lastId = 0;
static volatile long g_dispatchedTimer = 0;
static volatile long g_quitPosted = 0;
static uint64_t g_mainTid = 0;

static void CALLBACK plainProc(HWND h, UINT msg, UINT_PTR id, DWORD ts)
{
    (void)h;
    (void)ts;
    (void)msg;
    g_lastId = id;
    g_calls++;
}

static void CALLBACK quitProc(HWND h, UINT msg, UINT_PTR id, DWORD ts)
{
    plainProc(h, msg, id, ts);
    if (g_calls >= 2 && !g_quitPosted)
    {
        g_quitPosted = 1;
        // PostMessageA(NULL, WM_QUIT, ...) does not reach the queue on this
        // port: core wnd.cpp:1747 treats a NULL hWnd as an IPC post and drops
        // it, and PostQuitMessage routes through the freertos stub
        // PostThreadMessageA (sysbridge.cpp:1318), which ignores its arguments.
        // So the quit goes straight through the connection, the way a driver
        // would post it.
        SConnMgr::instance()->getConnection(g_mainTid)->postMsg(NULL, WM_QUIT, 0, 0);
    }
}

static int pumpOnce(MSG &m)
{
    if (PeekMessage(&m, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&m);
        DispatchMessage(&m);
        if (m.message == WM_TIMER)
            g_dispatchedTimer++;
        return 1;
    }
    return 0;
}

static void drainQueue(void)
{
    MSG m;
    for (int i = 0; i < 200; ++i)
    {
        if (!PeekMessage(&m, NULL, 0, 0, PM_REMOVE))
            break;
    }
}

static int fail(const char *name, const char *what)
{
    printf("FAIL %s: %s\n", name, what);
    return 1;
}

static int test_peek_does_not_wait(void)
{
    drainQueue();
    DWORD t0 = GetTickCount();
    MSG m;
    BOOL got = PeekMessage(&m, NULL, 0, 0, PM_REMOVE);
    DWORD wall = GetTickCount() - t0;
    printf("T0 peek-empty: got=%d wall=%lums\n", (int)got, (unsigned long)wall);
    if (got)
        return fail("T0", "PeekMessage returned a message on an empty queue");
    if (wall > 50)
        return fail("T0", "PeekMessage waited -- it must return immediately");
    return 0;
}

static int test_peek_fires_timer(void)
{
    drainQueue();
    g_calls = 0;
    g_lastId = 0;
    g_dispatchedTimer = 0;
    UINT_PTR id = SetTimer(NULL, 0, 50, plainProc);
    if (!id)
        return fail("T1", "SetTimer(NULL,0,50,TIMERPROC) returned 0");
    MSG m;
    DWORD t0 = GetTickCount();
    while (GetTickCount() - t0 < 400)
    {
        pumpOnce(m);
        Sleep(2);
    }
    KillTimer(NULL, id);
    printf("T1 peek-pump: calls=%ld id=%lu procId=%lu dispatchedWM_TIMER=%ld\n",
           (long)g_calls, (unsigned long)id, (unsigned long)g_lastId, (long)g_dispatchedTimer);
    if (g_calls < 3)
        return fail("T1", "TIMERPROC never ran inside PeekMessage");
    if (g_lastId != id)
        return fail("T1", "TIMERPROC got a wrong timer id");
    if (g_dispatchedTimer != 0)
        return fail("T1", "a callback timer leaked a dispatchable WM_TIMER");
    return 0;
}

static int test_get_msg_fires_timer(void)
{
    drainQueue();
    g_calls = 0;
    g_quitPosted = 0;
    g_dispatchedTimer = 0;
    UINT_PTR id = SetTimer(NULL, 0, 60, quitProc);
    if (!id)
        return fail("T2", "SetTimer returned 0");
    MSG m;
    DWORD t0 = GetTickCount();
    BOOL r = TRUE;
    int loops = 0;
    while ((r = GetMessage(&m, NULL, 0, 0)) != FALSE)
    {
        TranslateMessage(&m);
        DispatchMessage(&m);
        if (m.message == WM_TIMER)
            g_dispatchedTimer++;
        if (++loops > 200)
            break;
    }
    DWORD wall = GetTickCount() - t0;
    KillTimer(NULL, id);
    printf("T2 blocking-get: ret=%d calls=%ld wall=%lums loops=%d\n",
           (int)r, (long)g_calls, (unsigned long)wall, loops);
    if (r != FALSE)
        return fail("T2", "GetMessage did not return FALSE on WM_QUIT");
    if (g_calls < 2)
        return fail("T2", "TIMERPROC never ran inside the blocking GetMessage");
    if (wall < 100)
        return fail("T2", "GetMessage returned before two timer ticks elapsed");
    return 0;
}

static void *quitAfterDelay(void *p)
{
    usleep((useconds_t)((unsigned long)(uintptr_t)p) * 1000u);
    SConnection *conn = SConnMgr::instance()->getConnection(g_mainTid);
    conn->postMsg(NULL, WM_USER + 77, 0, 0);
    conn->postMsg(NULL, WM_QUIT, 0, 0);
    return NULL;
}

static int run_until_quit(unsigned *pFirstMsg, DWORD *pWallMs, double *pCpuMs)
{
    struct timespec ts0;
    clock_gettime(CLOCK_MONOTONIC, &ts0);
    clock_t cpu0 = clock();
    DWORD t0 = GetTickCount();

    pthread_t th;
    pthread_create(&th, NULL, quitAfterDelay, (void *)(uintptr_t)400);

    MSG m;
    BOOL r = TRUE;
    unsigned first = 0;
    int loops = 0;
    while ((r = GetMessage(&m, NULL, 0, 0)) != FALSE)
    {
        if (!first)
            first = m.message;
        TranslateMessage(&m);
        DispatchMessage(&m);
        if (++loops > 50)
            break;
    }
    pthread_join(th, NULL);

    *pWallMs = GetTickCount() - t0;
    *pCpuMs = (double)(clock() - cpu0) * 1000.0 / (double)CLOCKS_PER_SEC;
    *pFirstMsg = first;
    (void)ts0;
    return r == FALSE ? 0 : 1;
}

static int test_get_msg_waits(void)
{
    drainQueue();
    unsigned first = 0;
    DWORD wall = 0;
    double cpu = 0;
    if (run_until_quit(&first, &wall, &cpu))
        return fail("T3", "GetMessage did not return FALSE on the posted quit");
    printf("T3 blocking-get-empty: first=%u wall=%lums cpu=%.1fms\n",
           first, (unsigned long)wall, cpu);
    if (first != WM_USER + 77)
        return fail("T3", "GetMessage skipped the first posted message");
    if (wall < 350)
        return fail("T3", "GetMessage returned before the other thread posted: it does not wait");
    if (cpu > 80.0)
        return fail("T3", "GetMessage burned CPU while waiting: it spins instead of sleeping");
    return 0;
}

// The anti-spin guard inside getMsg: a queue that holds only messages outside
// the filter window must not turn waitMsg()'s "queue non-empty -> return true"
// into a busy loop.
static void *quitFilteredLater(void *p)
{
    usleep((useconds_t)((unsigned long)(uintptr_t)p) * 1000u);
    SConnMgr::instance()->getConnection(g_mainTid)->postMsg(NULL, WM_QUIT, 0, 0);
    return NULL;
}

static int test_get_msg_waits_through_filter(void)
{
    drainQueue();
    SConnection *conn = SConnMgr::instance()->getConnection(g_mainTid);
    // a message for a window the pump is not interested in: it stays queued
    conn->postMsg((HWND)(uintptr_t)0x1234, WM_USER + 5, 0, 0);

    clock_t cpu0 = clock();
    DWORD t0 = GetTickCount();
    pthread_t th;
    pthread_create(&th, NULL, quitFilteredLater, (void *)(uintptr_t)400);
    MSG m;
    BOOL r = TRUE;
    while ((r = GetMessage(&m, (HWND)(uintptr_t)0x9999, 0, 0)) != FALSE)
    {
        TranslateMessage(&m);
        DispatchMessage(&m);
    }
    pthread_join(th, NULL);
    DWORD wall = GetTickCount() - t0;
    double cpu = (double)(clock() - cpu0) * 1000.0 / (double)CLOCKS_PER_SEC;
    printf("T4 filtered-queue: ret=%d wall=%lums cpu=%.1fms\n", (int)r,
           (unsigned long)wall, cpu);
    if (r != FALSE)
        return fail("T4", "GetMessage never returned the quit through the filter");
    if (wall < 350)
        return fail("T4", "GetMessage returned before the quit was posted");
    if (cpu > 80.0)
        return fail("T4", "GetMessage spun on a non-empty but filtered queue");
    return 0;
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    g_mainTid = (uint64_t)GetCurrentThreadId();
    printf("freertos emu probe, mainTid=%llu\n", (unsigned long long)g_mainTid);
    int rc = 0;
    rc |= test_peek_does_not_wait();
    rc |= test_peek_fires_timer();
    rc |= test_get_msg_fires_timer();
    rc |= test_get_msg_waits();
    rc |= test_get_msg_waits_through_filter();
    printf("%s\n", rc ? "PROBE FAILED" : "PROBE OK");
    return rc;
}
