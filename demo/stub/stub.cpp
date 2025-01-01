#ifndef _WIN32
#include <windows.h>
#include <mmsystem.h>

#include <time.h>

DWORD timeGetTime(void)
{
    DWORD uptime = 0;
    struct timespec on;
    if(clock_gettime(CLOCK_MONOTONIC, &on) == 0)
        uptime = on.tv_sec*1000 + on.tv_nsec/1000000;
    return uptime;
}
MMRESULT timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc)
{
    return 0;
}
MMRESULT timeBeginPeriod(UINT uPeriod)
{
    return 0;
}
MMRESULT timeEndPeriod(UINT uPeriod)
{
    return 0;
}
#include <sys/stat.h>
#include <sys/types.h>
BOOL CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    mode_t mode = 0755;
    return mkdir(lpPathName, mode);
}

BOOL CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    char szPath[MAX_PATH];
    if (0 == WideCharToMultiByte(CP_UTF8, 0, lpPathName, -1, szPath, MAX_PATH, nullptr, nullptr))
        return FALSE;
    return CreateDirectoryA(szPath, lpSecurityAttributes);
}

#include <commdlg.h>
BOOL  GetOpenFileNameA(LPOPENFILENAMEA)
{
    return TRUE;
}
BOOL  GetOpenFileNameW(LPOPENFILENAMEW)
{
    return TRUE;
}
BOOL  GetSaveFileNameA(LPOPENFILENAMEA)
{
    return TRUE;
}
BOOL  GetSaveFileNameW(LPOPENFILENAMEW)
{
    return TRUE;
}
BOOL FlashWindow(HWND hWnd, BOOL bInvert)
{
    return TRUE;
}
BOOL ChooseColorA(LPCHOOSECOLORA)
{
    return TRUE;
}
BOOL ChooseColorW(LPCHOOSECOLORW)
{
    return TRUE;
}


long BroadcastSystemMessageA(DWORD flags, LPDWORD lpInfo, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return 0;
}
long BroadcastSystemMessageW(DWORD flags, LPDWORD lpInfo, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return 0;
}

#include <process.h>
uintptr_t _beginthreadex(
    void*                    _Security,
    unsigned                 _StackSize,
    _beginthreadex_proc_type _StartAddress,
    void*                    _ArgList,
    unsigned                 _InitFlag,
    unsigned*                _ThrdAddr
    )
{
    return 0;
}

BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode)
{
    return TRUE;
}

#include <shlobj.h>
STDAPI SHCreateStdEnumFmtEtc(_In_ UINT cfmt, _In_reads_(cfmt) const FORMATETC afmt[], _Outptr_ IEnumFORMATETC **ppenumFormatEtc)
{
    return 0;
}

#endif