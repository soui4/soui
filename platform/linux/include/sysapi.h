#ifndef _SYS_API_H
#define _SYS_API_H
#include <ctypes.h>
#include <unistd.h>

inline long InterlockedDecrement(long *v){
    return __atomic_fetch_sub(v,1,__ATOMIC_SEQ_CST);
}

inline long InterlockedIncrement(long *v){
    return __atomic_fetch_add(v,1,__ATOMIC_SEQ_CST);
}

void qsort_s(void * _Base, 
         size_t _NumOfElements,  size_t _SizeOfElements,
         int (__cdecl * _PtFuncCompare)(void *, const void *, const void *), void *_Context);


BOOL IIDFromString(LPCWSTR s, GUID *id);


void SetLastError(int e);

int GetLastError() ;

int MulDiv(int a,int b,int c);

tid_t GetCurrentThreadId();

int MultiByteToWideChar(int cp,int flags, const char *src, int len,wchar_t *dst,int dstLen);

int WideCharToMultiByte(int cp,int flags,const wchar_t* src,int len,char * dst,int dstLen,BOOL *p1,BOOL *p2);

#define STIF_DEFAULT        0x00000000L
#define STIF_SUPPORT_HEX    0x00000001L
BOOL StrToInt64ExW(const wchar_t *str, DWORD flags, LONGLONG *ret);
BOOL StrToIntExW(const wchar_t *str, DWORD flags, INT *ret);

void GetLocalTime(SYSTEMTIME *pSysTime);
void GetSystemTime(SYSTEMTIME * lpSystemTime);

time_t  _mkgmtime(struct tm * _Tm);
int  _localtime64_s (
        struct tm *ptm,
        const __time64_t *ptime
        );

void DestroyIcon(HICON hIcon);
int  GetWindowScale(HWND hWnd);

void PostThreadMessage(uint64_t tid, UINT msg,WPARAM wp,LPARAM lp);
BOOL WaitMessage();

BOOL PeekMessage(LPMSG pMsg, HWND  hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT  wRemoveMsg);

BOOL TranslateMessage(LPMSG pMsg);
BOOL DispatchMessage(LPMSG pMsg);

int GetSystemMetrics(int nIndex);

HCURSOR SetCursor(HCURSOR hCursor);
BOOL ShellExecute(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters,
    LPCSTR lpDirectory, INT nShowCmd);

SHORT
GetKeyState(
     int nVirtKey);

typedef LRESULT (CALLBACK* HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);

HHOOK
SetWindowsHookEx(
     int idHook,
     HOOKPROC lpfn,
     HINSTANCE hmod,
     DWORD dwThreadId);

BOOL
UnhookWindowsHookEx(
     HHOOK hhk);


LRESULT
CallNextHookEx(
     HHOOK hhk,
     int nCode,
     WPARAM wParam,
     LPARAM lParam);

UINT
MapVirtualKey(
     UINT uCode,
     UINT uMapType);

BOOL
CreateCaret(
     HWND hWnd,
     HBITMAP hBitmap,
     int nWidth,
     int nHeight);


BOOL
DestroyCaret(
    VOID);



BOOL RegisterDragDrop(HWND, IDropTarget *pDrapTarget);

BOOL CallMsgFilter(
     LPMSG lpMsg,
     int nCode);


__time64_t _mktime64(const struct tm *ptime);

__time64_t _time64(__time64_t * _Time);

HCURSOR
LoadCursor(
     HINSTANCE hInstance,
     LPCSTR lpCursorName);

BOOL DestroyCursor(HCURSOR hCursor);


BOOL IsFilePathValid(const char* path);

BOOL GetTextMetrics(HDC hdc,TEXTMETRIC *txtMetric);

BOOL HasFont(LPCTSTR fontName);

int IsBadReadPtr(const void* ptr, size_t size);
int IsBadWritePtr(const void* ptr, size_t size);

void ZeroMemory(void *p,size_t size);

uint32_t GetFileAttributes(const char *path);

FILE * _wfopen(const wchar_t *path,const wchar_t *mode);

long file_length(const char * path);

#define HEAP_NO_SERIALIZE               0x00000001      
#define HEAP_GROWABLE                   0x00000002      
#define HEAP_GENERATE_EXCEPTIONS        0x00000004      
#define HEAP_ZERO_MEMORY                0x00000008      
#define HEAP_REALLOC_IN_PLACE_ONLY      0x00000010      
#define HEAP_TAIL_CHECKING_ENABLED      0x00000020      
#define HEAP_FREE_CHECKING_ENABLED      0x00000040      
#define HEAP_DISABLE_COALESCE_ON_FREE   0x00000080      
#define HEAP_CREATE_ALIGN_16            0x00010000      
#define HEAP_CREATE_ENABLE_TRACING      0x00020000      
#define HEAP_CREATE_ENABLE_EXECUTE      0x00040000      
#define HEAP_MAXIMUM_TAG                0x0FFF              
#define HEAP_PSEUDO_TAG_FLAG            0x8000              
#define HEAP_TAG_SHIFT                  18                  
#define HEAP_CREATE_SEGMENT_HEAP        0x00000100      
#define HEAP_CREATE_HARDENED            0x00000200      

HANDLE WINAPI
HeapCreate(
    DWORD flOptions,
    size_t dwInitialSize,
    size_t dwMaximumSize
);

BOOL
WINAPI
HeapDestroy(
    HANDLE hHeap
);

LPVOID
WINAPI
HeapAlloc(
    HANDLE hHeap,
    DWORD dwFlags,
    size_t dwBytes
);

BOOL
WINAPI
HeapFree(
    HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem
);

BOOL
WINAPI
FlushInstructionCache(
    HANDLE hProcess,
    LPCVOID lpBaseAddress,
    size_t dwSize
);

HANDLE
WINAPI
GetCurrentProcess(
    VOID
);

DWORD WINAPI GetModuleFileName(
    HMODULE hModule,
    LPSTR lpFilename,
    DWORD nSize
);

BOOL WINAPI IsSupportMaxMove();
#endif