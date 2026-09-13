/**
 * swinx flock regression tests: the global handle table / shared memory lock
 * was switched from POSIX named semaphores (sem_open/sem_wait/sem_post) to
 * fcntl record locks on a dedicated lock file.
 *
 * Why: a named semaphore keeps its kernel state after every user exits. If
 * the last holder DIES while holding the lock (count drained by sem_wait,
 * never restored), the semaphore stays exhausted and the NEXT process blocks
 * forever inside the static GLobalHandleTable constructor - before main -
 * i.e. the program cannot start at all. fcntl record locks are released by
 * the kernel when the owning process exits, cleanly or by a crash, so a
 * crashed holder can never wedge a later start.
 *
 * These tests pin that behavior:
 *   1. crash_holder_releases_flock            - raw kernel level: exclusion
 *      while the holder is alive + automatic release when it is SIGKILLed,
 *      then the swinx lock class acquires again without blocking.
 *   2. named_objects_usable_after_abrupt_exit - end-to-end: after a process
 *      dies abruptly with named objects still registered, the global handle
 *      table stays usable from a fresh process.
 *   3. startup_survives_crashed_lock_holder   - THE original failure
 *      scenario: a holder grabs the very lock the GLobalHandleTable
 *      constructor takes at startup and is SIGKILLed; a brand-new process
 *      (execve re-runs the static constructor) must start and pass.
 *
 * Differential power: if the implementation ever regresses to named
 * semaphores (or any lock that leaks on crash), the acquires below block
 * forever. Every blocking step therefore runs under a SIGALRM watchdog that
 * reports through the result pipe, so a leak fails the test with a distinct
 * code instead of wedging the run. Child results travel through pipes (not
 * waitpid status) because swinx's SIGCHLD drain thread reaps waitpid(-1)
 * and would otherwise race us for the exit codes. Lock holders also exit as
 * soon as the test's quit pipe closes, so even a failing assertion cannot
 * leave a lock wedged for later tests.
 *
 * POSIX-only (fork/SIGKILL); on Windows real Win32 APIs are exercised and
 * have no such failure mode, so the whole file is compiled out there.
 */

#ifndef _WIN32

#include <gtest/gtest.h>
#include <windows.h> // swinx API surface (CreateEventA/CreateProcessA/...)
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <unistd.h>

#include <string>

// The class under test - the exact template instantiation the global handle
// table uses (kSharedNumber = 5, see sharedmem.h / SharedMemory).
#include "../../swinx/src/sharedmem.h"

namespace
{

// Watchdog result byte: "the acquisition blocked longer than the alarm
// allowed" => the lock leaked => regression to named semaphores.
const unsigned char kWatchdogResult = 200;

// Result pipe fd of the current verifier child; the SIGALRM handler reports
// kWatchdogResult through it (write is async-signal-safe).
volatile sig_atomic_t g_result_fd = -1;

void alarm_exit_handler(int)
{
    if (g_result_fd >= 0)
    {
        char c = (char)kWatchdogResult;
        ssize_t n = write((int)g_result_fd, &c, 1);
        (void)n;
    }
    _exit(kWatchdogResult);
}

// Best-effort reap. swinx's SIGCHLD drain thread does waitpid(-1, WNOHANG)
// and may already have reaped our forked child; tolerate ECHILD.
void reap_child(pid_t pid)
{
    while (waitpid(pid, NULL, 0) == -1 && errno == EINTR)
    {
    }
}

// Derive the lock-file path exactly like TNamedSemRwLock::init does
// (keep in sync with sharedmem.h: <lockDir>/soui_flock_<sanitized>.lock).
// Used only to assert exclusion/release at the raw fcntl level.
std::string flock_path(const char *name)
{
#if defined(__ANDROID__) || defined(__OHOS__)
    const char *dir = "/data/local/tmp/";
#else
    const char *dir = "/tmp/";
#endif
    std::string s = name ? name : "default";
    for (char &c : s)
    {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || c == '_' || c == '-'))
        {
            c = '_';
        }
    }
    return std::string(dir) + "soui_flock_" + s + ".lock";
}

// The per-platform name of the global handle table backing shared memory.
// Keep in sync with kGlobalShareMemName in sysobjs.cpp.
const char *global_table_shm_name()
{
#if defined(__APPLE__)
    return "/share_soui_A95AB24431E7";
#else
    return "/share_mem_soui-2BACFFE6-9ED7-4AC8-B699-A95AB24431E7";
#endif
}

// fork a child that takes the swinx exclusive lock for `name` through
// swinx::TNamedSemRwLock and holds it until the parent SIGKILLs it (the
// normal, crash-like path) or the quit pipe closes (assertion-failure path:
// the holder releases instead of wedging the rest of the suite).
// The child writes one 'L' byte into ready[1] once the lock is held; an
// early child death shows up as EOF on the parent's read instead of a hang.
// Never returns in the child.
pid_t fork_lock_holder(const char *name, const int ready[2], const int quitp[2])
{
    pid_t pid = fork();
    if (pid != 0)
        return pid;

    // child
    close(ready[0]);
    close(quitp[1]);
    swinx::TNamedSemRwLock<5> lock;
    if (!lock.init(name))
        _exit(2);
    lock.lockExclusive();
    if (write(ready[1], "L", 1) != 1)
        _exit(3);
    char c = 0;
    ssize_t n = read(quitp[0], &c, 1); // until SIGKILL or parent quit
    (void)c;
    (void)n;
    _exit(4);
}

} // namespace

// ------------------------------------------------------------------------
// 1) Kernel-level contract of the fix, on the real swinx lock class:
//    a live holder excludes other processes, and a crashed (SIGKILLed)
//    holder's lock is released by the kernel so the next acquirer - via
//    swinx::TNamedSemRwLock itself - gets through without blocking.
// ------------------------------------------------------------------------
TEST(swinx_flock, crash_holder_releases_flock)
{
    const char *kName = "/soui_flock_regression_crash_holder";

    int ready[2], quitp[2];
    ASSERT_EQ(pipe(ready), 0);
    ASSERT_EQ(pipe(quitp), 0);

    pid_t holder = fork_lock_holder(kName, ready, quitp);
    ASSERT_GT(holder, 0);
    close(ready[1]);
    close(quitp[0]); // parent never writes 'Q': SIGKILL is the normal path

    char buf = 0;
    ASSERT_EQ(read(ready[0], &buf, 1), 1)
        << "holder failed to take the lock (EOF = holder died early)";
    close(ready[0]);
    ASSERT_EQ(buf, 'L');

    // (a) exclusion: while the holder is alive, a plain nonblocking fcntl
    //     write-lock on the same lock file must be refused.
    std::string path = flock_path(kName);
    int fd = open(path.c_str(), O_RDWR, 0666);
    ASSERT_GE(fd, 0) << "lock file not created by TNamedSemRwLock: " << path;
    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    int rc = fcntl(fd, F_SETLK, &fl);
    if (rc == 0)
    {
        // would break later steps and mean the lock excludes nothing
        fl.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &fl);
    }
    ASSERT_EQ(rc, -1) << "holder's lock does not exclude other processes";
    ASSERT_TRUE(errno == EACCES || errno == EAGAIN) << "errno=" << errno;

    // (b) crash: SIGKILL the holder, exactly like an abnormal termination.
    ASSERT_EQ(kill(holder, SIGKILL), 0);
    reap_child(holder);
    close(quitp[1]);

    // (c) release: the same nonblocking write-lock must now succeed without
    //     any manual cleanup - the kernel dropped the dead holder's lock.
    ASSERT_EQ(fcntl(fd, F_SETLK, &fl), 0)
        << "crashed holder's lock was not released by the kernel";
    fl.l_type = F_UNLCK;
    ASSERT_EQ(fcntl(fd, F_SETLK, &fl), 0);
    close(fd);

    // (d) fresh acquire through the swinx class, in a new process, under a
    //     watchdog so a leaked lock fails the test instead of hanging it.
    int res[2];
    ASSERT_EQ(pipe(res), 0);
    pid_t verifier = fork();
    ASSERT_GE(verifier, 0);
    if (verifier == 0)
    {
        close(res[0]);
        g_result_fd = res[1];
        signal(SIGALRM, alarm_exit_handler);
        alarm(10);
        int code = 0;
        swinx::TNamedSemRwLock<5> lock;
        if (!lock.init(kName))
            code = 5;
        else
        {
            lock.lockExclusive(); // must NOT block: the holder died
            lock.unlockExclusive();
        }
        char c = (char)code;
        ssize_t n = write(res[1], &c, 1);
        (void)n;
        _exit(code);
    }
    close(res[1]);
    char rbuf = 0;
    ASSERT_EQ(read(res[0], &rbuf, 1), 1) << "verifier died before reporting";
    close(res[0]);
    reap_child(verifier);
    EXPECT_EQ((unsigned char)rbuf, 0) << "verifier step code " << (int)rbuf;
    if ((unsigned char)rbuf == kWatchdogResult)
        FAIL() << "acquire blocked after holder crash: lock leaked "
                  "(regression to named semaphores?)";
}

// ------------------------------------------------------------------------
// 2) End-to-end health: a process that dies abruptly (no CloseHandle, no
//    destructors - _exit skips all cleanup) must leave the global handle
//    table usable for fresh processes: its named objects stay openable and
//    new named objects can still be created.
// ------------------------------------------------------------------------
TEST(swinx_flock, named_objects_usable_after_abrupt_exit)
{
    char evt1[64], mut1[64], evt2[64];
    unsigned pid = (unsigned)GetCurrentProcessId();
    snprintf(evt1, sizeof(evt1), "swinx_flock_evt_%u", pid);
    snprintf(mut1, sizeof(mut1), "swinx_flock_mut_%u", pid);
    snprintf(evt2, sizeof(evt2), "swinx_flock_evt2_%u", pid);

    // crasher: register named objects, then die abruptly
    int ready[2];
    ASSERT_EQ(pipe(ready), 0);
    pid_t crasher = fork();
    ASSERT_GE(crasher, 0);
    if (crasher == 0)
    {
        close(ready[0]);
        HANDLE e = CreateEventA(NULL, TRUE, FALSE, evt1);
        if (!e)
            _exit(2);
        HANDLE m = CreateMutexA(NULL, FALSE, mut1);
        if (!m)
            _exit(3);
        char c = 'D';
        ssize_t n = write(ready[1], &c, 1);
        (void)n;
        _exit(7); // abrupt: no CloseHandle, no static destructors
    }
    close(ready[1]);
    char buf = 0;
    ASSERT_EQ(read(ready[0], &buf, 1), 1)
        << "crasher failed to create named objects";
    close(ready[0]);
    ASSERT_EQ(buf, 'D');
    reap_child(crasher);

    // verifier: a fresh process must still use the table, under a watchdog
    int res[2];
    ASSERT_EQ(pipe(res), 0);
    pid_t verifier = fork();
    ASSERT_GE(verifier, 0);
    if (verifier == 0)
    {
        close(res[0]);
        g_result_fd = res[1];
        signal(SIGALRM, alarm_exit_handler);
        alarm(15);
        int code = 0;
        HANDLE e = OpenEventA(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, evt1);
        if (!e)
            code = 4;
        else
        {
            HANDLE m = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, mut1);
            if (!m)
                code = 5;
            else
            {
                HANDLE e2 = CreateEventA(NULL, TRUE, TRUE, evt2);
                if (!e2)
                    code = 6;
                else if (SetEvent(e2) != TRUE)
                    code = 8;
                else if (WaitForSingleObject(e2, 2000) != WAIT_OBJECT_0)
                    code = 9;
                if (e2)
                    CloseHandle(e2);
                CloseHandle(m);
            }
            CloseHandle(e);
        }
        char c = (char)code;
        ssize_t n = write(res[1], &c, 1);
        (void)n;
        _exit(code);
    }
    close(res[1]);
    char rbuf = 0;
    ASSERT_EQ(read(res[0], &rbuf, 1), 1) << "verifier died before reporting";
    close(res[0]);
    reap_child(verifier);
    EXPECT_EQ((unsigned char)rbuf, 0) << "verifier step code " << (int)rbuf;
    if ((unsigned char)rbuf == kWatchdogResult)
        FAIL() << "global handle table wedged after an abrupt process exit";
}

// ------------------------------------------------------------------------
// 3) THE original failure scenario: a process died while holding the lock
//    that the static GLobalHandleTable constructor takes BEFORE main, so
//    the next process could never start. Here a holder grabs exactly that
//    lock (same shared-memory name => same lock file) and is SIGKILLed;
//    a brand-new process (CreateProcessA => fork+execve re-runs the static
//    constructor) must start and pass.
// ------------------------------------------------------------------------

// runs standalone as a fast smoke test, and as the exec'd probe below
TEST(swinx_flock, startup_probe)
{
    char name[64];
    snprintf(name, sizeof(name), "swinx_flock_probe_evt_%u",
        (unsigned)GetCurrentProcessId());
    HANDLE e = CreateEventA(NULL, TRUE, FALSE, name);
    ASSERT_TRUE(e != NULL);
    EXPECT_EQ(WaitForSingleObject(e, 0), WAIT_TIMEOUT); // created unsignaled
    EXPECT_TRUE(SetEvent(e));
    EXPECT_EQ(WaitForSingleObject(e, 2000), WAIT_OBJECT_0);
    CloseHandle(e);
}

namespace
{

// spawn a fresh copy of this binary running only startup_probe; wait for its
// exit code with a deadline. Returns false (and kills the child) on timeout,
// which is how a startup wedge manifests.
bool spawn_probe_and_wait(DWORD timeout_ms, DWORD *code_out)
{
    char exe[MAX_PATH] = { 0 };
    if (GetModuleFileNameA(NULL, exe, MAX_PATH) == 0)
        return false;
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "--gtest_filter=swinx_flock.startup_probe");

    STARTUPINFOA si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof(pi));
    if (!CreateProcessA(exe, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        return false;

    // swinx: GetExitCodeProcess returns TRUE only after the drain thread
    // reaped the child, so polling it doubles as a liveness check
    bool ok = false;
    for (DWORD t = 0; t < timeout_ms; t += 20)
    {
        if (GetExitCodeProcess(pi.hProcess, code_out))
        {
            ok = true;
            break;
        }
        Sleep(20);
    }
    if (!ok)
    {
        kill((pid_t)pi.dwProcessId, SIGKILL); // un-wedge the suite
        for (int t = 0; t < 100; t++)
        {
            if (GetExitCodeProcess(pi.hProcess, code_out))
                break;
            Sleep(20);
        }
    }
    CloseHandle(pi.hProcess);
    return ok;
}

} // namespace

TEST(swinx_flock, startup_survives_crashed_lock_holder)
{
    // holder: take the GLOBAL table's lock through the swinx lock class
    // (same shared-memory name => same lock file the constructor uses)
    int ready[2], quitp[2];
    ASSERT_EQ(pipe(ready), 0);
    ASSERT_EQ(pipe(quitp), 0);

    pid_t holder = fork_lock_holder(global_table_shm_name(), ready, quitp);
    ASSERT_GT(holder, 0);
    close(ready[1]);
    close(quitp[0]);

    char buf = 0;
    ASSERT_EQ(read(ready[0], &buf, 1), 1)
        << "holder failed to take the global lock (EOF = holder died early)";
    close(ready[0]);
    ASSERT_EQ(buf, 'L');

    // crash while holding: the old named-semaphore implementation left the
    // semaphore drained here, wedging every later process before main.
    ASSERT_EQ(kill(holder, SIGKILL), 0);
    reap_child(holder);
    close(quitp[1]);

    // a brand-new process must START (its static constructor re-takes this
    // lock) and pass the probe
    DWORD code = 0;
    EXPECT_TRUE(spawn_probe_and_wait(30000, &code))
        << "probe never exited: startup wedged after a crashed lock holder";
    EXPECT_EQ(code, 0u) << "probe failed in a fresh process after holder crash";
}

#endif //_WIN32
