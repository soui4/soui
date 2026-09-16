/**
 * swinx module/process APIs: module file name, LoadLibrary/GetProcAddress/
 * FreeLibrary (dlopen based), process & thread identity.
 */

#include <gtest/gtest.h>
#include <windows.h>
#include <math.h>
#include <sdef.h>

// VS2008 has no C99 snprintf; _snprintf is its near-equivalent.
#ifdef _MSC_VER
#define MODULE_SNPRINTF _snprintf
#else
#define MODULE_SNPRINTF snprintf
#endif

TEST(swinx_module, get_module_file_name_null)
{
    char buf[MAX_PATH] = {0};
    DWORD len = GetModuleFileNameA(NULL, buf, MAX_PATH);
    EXPECT_GT(len, 0u);
    EXPECT_LT(len, (DWORD)MAX_PATH);
    // the reported executable must exist (attribute bits vary by platform:
    // files are typically ARCHIVE(0x20) on Windows, NORMAL(0x80) under swinx)
    EXPECT_NE(GetFileAttributesA(buf), INVALID_FILE_ATTRIBUTES);
    EXPECT_FALSE(GetFileAttributesA(buf) & FILE_ATTRIBUTE_DIRECTORY);
}

TEST(swinx_module, process_id_sanity)
{
    EXPECT_NE(GetCurrentProcessId(), 0u);
}

struct ThreadCtx
{
    tid_t tid;
};

static DWORD WINAPI captureThreadId(LPVOID p)
{
    ((ThreadCtx *)p)->tid = GetCurrentThreadId();
    return 0;
}

TEST(swinx_module, thread_id_differs_cross_thread)
{
    tid_t main_tid = GetCurrentThreadId();
    EXPECT_NE(main_tid, (tid_t)0);

    ThreadCtx ctx;
    ctx.tid = 0;

    HANDLE th = CreateThread(NULL, 0, captureThreadId, &ctx, 0, NULL);
    ASSERT_TRUE(th != NULL);
    EXPECT_EQ(WaitForSingleObject(th, 3000), WAIT_OBJECT_0);

    EXPECT_NE(ctx.tid, (tid_t)0);
    EXPECT_NE(ctx.tid, main_tid);

    CloseHandle(th);
}

TEST(swinx_module, load_library_get_proc_address)
{
#ifdef _WIN32
    HMODULE h = LoadLibraryA("msvcrt.dll");
#elif defined(__APPLE__)
    HMODULE h = LoadLibraryA("libm.dylib"); // libm is part of libSystem on macOS
#else
    HMODULE h = LoadLibraryA("libm.so.6");
#endif
    ASSERT_TRUE(h != NULL);

    typedef double (*sqrt_fn)(double);
    sqrt_fn fn = (sqrt_fn)GetProcAddress(h, "sqrt");
    ASSERT_TRUE(fn != NULL);
    EXPECT_NEAR(fn(4.0), 2.0, 1e-12);

    EXPECT_TRUE(GetProcAddress(h, "no_such_symbol_xyz_123") == NULL);

    EXPECT_TRUE(FreeLibrary(h));
}

TEST(swinx_module, load_library_nonexistent_fails)
{
    EXPECT_TRUE(LoadLibraryA("no_such_library_xyz_12345.so") == NULL);
}

TEST(swinx_module, load_library_null_fails)
{
    EXPECT_TRUE(LoadLibraryA(NULL) == NULL);
}

TEST(swinx_module, get_module_handle_null)
{
    // GetModuleHandleA(NULL) returns the base module of the current process
    HMODULE h = GetModuleHandleA(NULL);
    EXPECT_TRUE(h != NULL);
}

// ------------------------------------------------------------------------
// Process creation semantics:
//  - POSIX (swinx): lpCommandLine holds the ARGUMENTS ONLY; the program is
//    given by lpApplicationName (/bin/sh via execve, no shell quoting layer).
//    pi.hProcess is a manual-reset event signaled when the child has
//    STARTED; GetExitCodeProcess returns TRUE only after the SIGCHLD drain
//    thread has reaped the child.
//  - Windows (real Win32): lpCommandLine is the full command line, so
//    cmd.exe /c is used. pi.hProcess signals when the child EXITS;
//    GetExitCodeProcess returns TRUE immediately, with STILL_ACTIVE(259)
//    until the process object is signaled.
// ------------------------------------------------------------------------

// spawn a child that exits with the given code
static BOOL spawn_exit_process(int code, PROCESS_INFORMATION *pi)
{
    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    char cmd[64];
#ifdef _WIN32
    MODULE_SNPRINTF(cmd, sizeof(cmd), "cmd.exe /c exit %d", code);
    return CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, pi);
#else
    MODULE_SNPRINTF(cmd, sizeof(cmd), "-c \"exit %d\"", code); // parsed into "-c" and "exit N"
    return CreateProcessA("/bin/sh", cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, pi);
#endif
}

// poll until the child's exit code is retrievable
static BOOL wait_exit_code(HANDLE hProcess, DWORD *code)
{
    for (int t = 0; t < 150; t++)
    {
        if (GetExitCodeProcess(hProcess, code))
        {
#ifdef _WIN32
            if (*code != STILL_ACTIVE)
                return TRUE; // process object signaled: final exit code
#else
            return TRUE; // swinx: TRUE only after the child has been reaped
#endif
        }
        Sleep(20);
    }
    return FALSE;
}

TEST(swinx_process, create_process_exit_code)
{
    PROCESS_INFORMATION pi = {};
    ASSERT_TRUE(spawn_exit_process(42, &pi));
    EXPECT_NE(pi.dwProcessId, 0u);
    EXPECT_TRUE(pi.hProcess != NULL && pi.hProcess != INVALID_HANDLE_VALUE);

    // POSIX: signaled when the child has STARTED (swinx start-event)
    // Windows: signaled when the child has EXITED (real process handle)
    EXPECT_EQ(WaitForSingleObject(pi.hProcess, 5000), WAIT_OBJECT_0);

    // the handle resolves back to the child pid
    EXPECT_EQ(GetProcessId(pi.hProcess), pi.dwProcessId);

    DWORD code = 0;
    EXPECT_TRUE(wait_exit_code(pi.hProcess, &code));
    EXPECT_EQ(code, 42u);

    CloseHandle(pi.hProcess);
}

TEST(swinx_process, create_process_exit_zero)
{
    PROCESS_INFORMATION pi = {};

    ASSERT_TRUE(spawn_exit_process(0, &pi));

    DWORD code = (DWORD)-1;
    EXPECT_TRUE(wait_exit_code(pi.hProcess, &code));
    EXPECT_EQ(code, 0u);

    CloseHandle(pi.hProcess);
}

// POSIX: SIGCHLD is a coalescing (non-realtime) signal, so several children
// exiting close together must still ALL be reaped and registered by the
// drain thread (self-pipe design), each with its own exit code intact.
// Windows: verifies exit-code isolation across 6 concurrent children.
TEST(swinx_process, create_process_multiple_children_all_reaped)
{
    enum { kChildren = 6 };
    PROCESS_INFORMATION pis[kChildren] = {};

    for (int i = 0; i < kChildren; i++)
        ASSERT_TRUE(spawn_exit_process(i + 1, &pis[i])) << "child " << i;

    for (int i = 0; i < kChildren; i++)
    {
        DWORD code = (DWORD)-1;
        ASSERT_TRUE(wait_exit_code(pis[i].hProcess, &code))
            << "child " << i << " exit code never retrieved";
        EXPECT_EQ(code, (DWORD)(i + 1)) << "child " << i << " exit code mismatch";
        CloseHandle(pis[i].hProcess);
    }
}

TEST(swinx_process, get_exit_code_invalid_handle)
{
    DWORD code = 0;
    EXPECT_FALSE(GetExitCodeProcess(NULL, &code));
#ifdef _WIN32
    // real Win32: INVALID_HANDLE_VALUE is the pseudo current-process handle
    // (probed: returns TRUE with STILL_ACTIVE while this process is alive)
    EXPECT_TRUE(GetExitCodeProcess(INVALID_HANDLE_VALUE, &code));
    EXPECT_EQ(code, STILL_ACTIVE);
#else
    // swinx: not a registered process handle -> FALSE
    EXPECT_FALSE(GetExitCodeProcess(INVALID_HANDLE_VALUE, &code));
#endif
}

TEST(swinx_process, get_process_id_special_handles)
{
    // NULL is not a process handle -> 0 (Win32 semantics)
    EXPECT_EQ(GetProcessId(NULL), 0u);
    // swinx convention: INVALID_HANDLE_VALUE refers to the current process
    EXPECT_EQ(GetProcessId(INVALID_HANDLE_VALUE), GetCurrentProcessId());
}


// ---------------- SOUI 组件（SComMgr2 创建） ----------------
// 迁自 test.cpp（原 soui.taskloop）

#include <souistd.h>
#include <commgr2.h>
#include <helper/SFunctor.hpp>

using namespace SOUI;

class TaskHost {
public:
    TaskHost(int id_) : id(id_) {}
    void task0() {
        SSLOGI() << "task0 done,id=" << id;
    }
    void task1(int p1) {
        SSLOGI() << "task1 done,id=" << id << " p1=" << p1;
    }
    void task2(int p1, std::string &p2) {
        SSLOGI() << "task2 done,id=" << id << " p1=" << p1 << " p2=" << p2.c_str();
    }
private:
    int id;
};

TEST(soui_component, taskloop_post_and_stop)
{
    SComMgr2 comMgr;
    SAutoRefPtr<ITaskLoop> taskLoop;
    comMgr.CreateTaskLoop((IObjRef **)&taskLoop);
    ASSERT_TRUE(taskLoop);
    TaskHost host(100);
    taskLoop->start("test_loop", Normal);
    STaskHelper::post(taskLoop, &host, &TaskHost::task0, false);
    STaskHelper::post(taskLoop, &host, &TaskHost::task1, 200, false);
    // 等最后一个任务执行完再退出
    STaskHelper::post(taskLoop, &host, &TaskHost::task2, 300, std::string("work task"), true);
    taskLoop->stop();
}
