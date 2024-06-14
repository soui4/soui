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

void OffsetRect(RECT *prc,int xOffset,int yOffset);

BOOL IsRectEmpty(const RECT * prc);

void qsort_s(void * _Base, 
         size_t _NumOfElements,  size_t _SizeOfElements,
         int (__cdecl * _PtFuncCompare)(void *, const void *, const void *), void *_Context);


BOOL IIDFromString(LPCWSTR s, GUID *id);


void SetLastError(int e);

int GetLastError() ;

int MulDiv(int a,int b,int c);

uint64_t GetCurrentThreadId();

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

void SetCursor(HCURSOR hCursor);
BOOL ShellExecute(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters,
    LPCSTR lpDirectory, INT nShowCmd);

void DeleteObject(HGDIOBJ hObj);

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

BOOL GetModuleFileName(HMODULE hModule,LPTSTR pszPath,int length);

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

#endif