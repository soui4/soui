/**
 * swinx thread management tests (CreateThread / Suspend / Resume).
 */
#include <gtest/gtest.h>
#include <windows.h>

#ifdef _WIN32
typedef DWORD tid_t;
#endif

// Concurrency-safe counter (Interlocked so it also builds on compilers that
// lack <atomic>, e.g. VS2008).
static volatile LONG s_counter;

static DWORD WINAPI counter_thread(LPVOID)
{
    for (int i = 0; i < 1000; i++)
        InterlockedIncrement(&s_counter);
    return 1234;
}

TEST(swinx_thread, create_and_join)
{
    s_counter = 0;
    tid_t tid = 0;
    HANDLE th = CreateThread(NULL, 0, counter_thread, NULL, 0, &tid);
    ASSERT_TRUE(th != NULL);
    EXPECT_NE(tid, GetCurrentThreadId());

    EXPECT_EQ(WaitForSingleObject(th, 5000), WAIT_OBJECT_0);
    EXPECT_EQ(s_counter, 1000);
    CloseHandle(th);
}

TEST(swinx_thread, multiple_threads)
{
    s_counter = 0;
    enum { N = 4 };
    HANDLE ths[N] = {0};
    for (int i = 0; i < N; i++) {
        ths[i] = CreateThread(NULL, 0, counter_thread, NULL, 0, NULL);
        ASSERT_TRUE(ths[i] != NULL);
    }
    EXPECT_EQ(WaitForMultipleObjects(N, ths, TRUE, 10000), WAIT_OBJECT_0);
    EXPECT_EQ(s_counter, 1000 * N);
    for (int i = 0; i < N; i++)
        CloseHandle(ths[i]);
}

static DWORD WINAPI waiting_thread(LPVOID p)
{
    HANDLE evt = (HANDLE)p;
    WaitForSingleObject(evt, 5000);
    return 0;
}

TEST(swinx_thread, suspend_resume)
{
    HANDLE evt = CreateEventA(NULL, TRUE, FALSE, NULL);
    ASSERT_TRUE(evt != NULL);

    HANDLE th = CreateThread(NULL, 0, waiting_thread, evt, 0, NULL);
    ASSERT_TRUE(th != NULL);

    Sleep(50); // let the thread enter its wait
    DWORD prev = SuspendThread(th);
    EXPECT_EQ(prev, 0u);
    EXPECT_EQ(SuspendThread(th), 1u); // suspend count grows

    EXPECT_EQ(ResumeThread(th), 2u);
    EXPECT_EQ(ResumeThread(th), 1u);
    EXPECT_EQ(ResumeThread(th), 0u); // fully resumed

    SetEvent(evt);
    EXPECT_EQ(WaitForSingleObject(th, 3000), WAIT_OBJECT_0);

    CloseHandle(th);
    CloseHandle(evt);
}

TEST(swinx_thread, thread_id_uniqueness)
{
    s_counter = 0;
    tid_t t1 = 0, t2 = 0;
    HANDLE h1 = CreateThread(NULL, 0, counter_thread, NULL, 0, &t1);
    HANDLE h2 = CreateThread(NULL, 0, counter_thread, NULL, 0, &t2);
    ASSERT_TRUE(h1 != NULL && h2 != NULL);
    EXPECT_NE(t1, t2);
    EXPECT_NE(t1, (tid_t)GetCurrentThreadId());

    HANDLE ths[2] = { h1, h2 };
    EXPECT_EQ(WaitForMultipleObjects(2, ths, TRUE, 5000), WAIT_OBJECT_0);
    CloseHandle(h1);
    CloseHandle(h2);
}
