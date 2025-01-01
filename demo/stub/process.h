typedef unsigned (__stdcall* _beginthreadex_proc_type)(void*);

uintptr_t _beginthreadex(
    void*                    _Security,
    unsigned                 _StackSize,
    _beginthreadex_proc_type _StartAddress,
    void*                    _ArgList,
    unsigned                 _InitFlag,
    unsigned*                _ThrdAddr
    );

BOOL WINAPI TerminateThread(HANDLE hThread, DWORD dwExitCode);
