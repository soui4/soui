/**
 * swinx hook manager tests (SetWindowsHookEx / UnhookWindowsHookEx / CallHook).
 * The concurrency test is a regression test for the hook object lifetime fix:
 * call_hook now holds a reference for the duration of the callback, so a hook
 * unhooked (and previously deleted) from another thread while its callback is
 * in flight no longer causes a use-after-free.
 */
#include <gtest/gtest.h>
#include <windows.h>

#ifndef _WIN32
#include <atomic>
#include <thread>

static std::atomic<int> s_hookCalls;

static LRESULT CALLBACK test_hook_proc(int code, WPARAM wParam, LPARAM lParam)
{
    s_hookCalls++;
    // non-zero return tells the dispatcher the message was handled (skipped)
    return 1;
}

static LRESULT CALLBACK zero_hook_proc(int code, WPARAM wParam, LPARAM lParam)
{
    s_hookCalls++;
    return 0;
}

TEST(swinx_hook, set_unhook_basic)
{
    HHOOK h = SetWindowsHookExA(WH_GETMESSAGE, test_hook_proc, 0, GetCurrentThreadId());
    ASSERT_TRUE(h != NULL);

    EXPECT_TRUE(UnhookWindowsHookEx(h));
    // double unhook fails
    EXPECT_FALSE(UnhookWindowsHookEx(h));

    // invalid handles fail cleanly
    EXPECT_FALSE(UnhookWindowsHookEx(NULL));
}

TEST(swinx_hook, call_hook_dispatches)
{
    s_hookCalls = 0;
    HHOOK h = SetWindowsHookExA(WH_GETMESSAGE, test_hook_proc, 0, GetCurrentThreadId());
    ASSERT_TRUE(h != NULL);

    // same-thread hook is invoked synchronously
    EXPECT_TRUE(CallHook(WH_GETMESSAGE, HC_ACTION, 0, 0));
    EXPECT_EQ(s_hookCalls.load(), 1);

    // after unhooking, CallHook finds no hook and returns FALSE
    EXPECT_TRUE(UnhookWindowsHookEx(h));
    EXPECT_FALSE(CallHook(WH_GETMESSAGE, HC_ACTION, 0, 0));
    EXPECT_EQ(s_hookCalls.load(), 1);
}

TEST(swinx_hook, call_hook_no_hook_installed)
{
    s_hookCalls = 0;
    EXPECT_FALSE(CallHook(WH_KEYBOARD, HC_ACTION, 0, 0));
    EXPECT_EQ(s_hookCalls.load(), 0);
}

TEST(swinx_hook, call_next_hook_without_handle)
{
    // CallNextHookEx(NULL, ...) ends the chain safely
    EXPECT_EQ(CallNextHookEx(NULL, HC_ACTION, 0, 0), 0);
}

TEST(swinx_hook, per_type_isolation)
{
    s_hookCalls = 0;
    HHOOK hKeyboard = SetWindowsHookExA(WH_KEYBOARD, test_hook_proc, 0, GetCurrentThreadId());
    HHOOK hMouse = SetWindowsHookExA(WH_MOUSE, zero_hook_proc, 0, GetCurrentThreadId());
    ASSERT_TRUE(hKeyboard != NULL && hMouse != NULL);

    // a hook only receives calls for its own type
    EXPECT_TRUE(CallHook(WH_KEYBOARD, HC_ACTION, 0, 0));
    EXPECT_EQ(s_hookCalls.load(), 1);

    EXPECT_EQ(CallHook(WH_MOUSE, HC_ACTION, 0, 0), 0); // zero_hook_proc returns 0
    EXPECT_EQ(s_hookCalls.load(), 2);

    EXPECT_FALSE(CallHook(WH_CALLWNDPROC, HC_ACTION, 0, 0));
    EXPECT_EQ(s_hookCalls.load(), 2);

    UnhookWindowsHookEx(hKeyboard);
    UnhookWindowsHookEx(hMouse);
}

TEST(swinx_hook, newest_hook_called_first)
{
    s_hookCalls = 0;
    HHOOK hOld = SetWindowsHookExA(WH_GETMESSAGE, zero_hook_proc, 0, GetCurrentThreadId());
    HHOOK hNew = SetWindowsHookExA(WH_GETMESSAGE, test_hook_proc, 0, GetCurrentThreadId());
    ASSERT_TRUE(hOld != NULL && hNew != NULL);

    // the most recently installed hook is called first; its non-zero return
    // marks the message as handled
    EXPECT_TRUE(CallHook(WH_GETMESSAGE, HC_ACTION, 0, 0));
    EXPECT_EQ(s_hookCalls.load(), 1);

    UnhookWindowsHookEx(hNew);
    // after removing the newest, the older hook is reached
    EXPECT_EQ(CallHook(WH_GETMESSAGE, HC_ACTION, 0, 0), 0);
    EXPECT_EQ(s_hookCalls.load(), 2);

    UnhookWindowsHookEx(hOld);
}

// Regression test for the hook lifetime fix: unhooking from another thread
// while CallHook is dispatching must not crash. Before the fix the hook
// object was deleted under the exclusive lock while the dispatcher still
// dereferenced it outside the lock (use-after-free).
TEST(swinx_hook, concurrent_unhook_vs_call)
{
    for (int round = 0; round < 200; round++) {
        s_hookCalls = 0;
        HHOOK h = SetWindowsHookExA(WH_GETMESSAGE, test_hook_proc, 0, GetCurrentThreadId());
        ASSERT_TRUE(h != NULL);

        std::thread unhooker([h]() {
            UnhookWindowsHookEx(h);
        });

        // may run before, during, or after the concurrent unhook - all outcomes
        // are legal, the point is that neither thread touches freed memory
        CallHook(WH_GETMESSAGE, HC_ACTION, 0, 0);

        unhooker.join();

        // unhook again if the background thread lost the race; FALSE is fine
        UnhookWindowsHookEx(h);
    }
}

// Hammer both directions: repeated set/unhook on one thread while the main
// thread keeps calling hooks on the shared chain.
TEST(swinx_hook, churn_while_calling)
{
    s_hookCalls = 0;
    std::thread churner([]() {
        for (int i = 0; i < 500; i++) {
            HHOOK h = SetWindowsHookExA(WH_MOUSE, zero_hook_proc, 0, GetCurrentThreadId());
            if (h)
                UnhookWindowsHookEx(h);
        }
    });

    for (int i = 0; i < 500; i++) {
        CallHook(WH_MOUSE, HC_ACTION, 0, 0);
    }

    churner.join();
}

#endif//_WIN32