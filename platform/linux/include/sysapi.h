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

uint32_t GetCurrentThreadId();

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
void GetWindowRect(HWND hWnd,RECT *rc);
int  GetWindowScale(HWND hWnd);

void PostThreadMessage(int tid, UINT msg,WPARAM wp,LPARAM lp);
BOOL WaitMessage();

BOOL IsWindow(HWND hWnd);
void DestroyWindow(HWND hwnd);

HDC GetDC(HWND hWnd);
void ReleaseDC(HWND hwnd,HDC hdc);

int GetSystemMetrics(int nIndex);

void SetCursor(HCURSOR hCursor);
BOOL ShellExecute(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters,
    LPCSTR lpDirectory, INT nShowCmd);

BOOL ClientToScreen(HWND hWnd,LPPOINT ppt);
BOOL ScreenToClient(HWND hWnd,LPPOINT ppt);

BOOL GetClipBox(HDC hdc, RECT *pRc);

HMONITOR
MonitorFromWindow(
    HWND hwnd,
    DWORD dwFlags);

BOOL
GetMonitorInfo(
 HMONITOR hMonitor,
 LPMONITORINFO lpmi);

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

BOOL
PostMessage(
     HWND hWnd,
     UINT Msg,
     WPARAM wParam,
     LPARAM lParam);

LRESULT SendMessage(
     HWND hWnd,
     UINT Msg,
     WPARAM wParam,
     LPARAM lParam);

int
MessageBox(
     HWND hWnd,
     LPCTSTR lpText,
     LPCTSTR lpCaption,
     UINT uType);

BOOL
SetForegroundWindow(
 HWND hWnd);

HWND GetForegroundWindow();

 BOOL
ShowWindow(
     HWND hWnd,
     int nCmdShow);

BOOL
SetWindowPos(
     HWND hWnd,
     HWND hWndInsertAfter,
     int X,
     int Y,
     int cx,
     int cy,
     UINT uFlags);

LONG
GetWindowLong(
     HWND hWnd,
     int nIndex);

LONG SetWindowLong(HWND hWnd, int nIndex,LONG data);

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


BOOL
HideCaret(
     HWND hWnd);

BOOL
ShowCaret(
     HWND hWnd);

BOOL
SetCaretPos(
     int X,
     int Y);

BOOL
GetCaretPos(
    LPPOINT lpPoint);


HWND GetActiveWindow();

HWND GetDesktopWindow();

BOOL IsWindowEnabled(HWND hWnd);

void EnableWindow(HWND hWnd,BOOL bEnable);

HWND SetActiveWindow(HWND hWnd);

HWND GetParent(HWND hwnd);
HWND SetParent(HWND hWnd, HWND hParent);

BOOL GetCursorPos(LPPOINT ppt);

HWND WindowFromPoint(POINT pt);

DWORD GetTickCount();

typedef VOID (CALLBACK* TIMERPROC)(HWND, UINT, UINT_PTR, DWORD);

UINT_PTR
SetTimer(
     HWND hWnd,
     UINT_PTR nIDEvent,
     UINT uElapse,
     TIMERPROC lpTimerFunc);

BOOL
KillTimer(
     HWND hWnd,
     UINT_PTR uIDEvent);

HWND GetFocus();

HWND SetFocus(HWND hWnd);

typedef struct _PAINTSTRUCT{

}PAINTSTRUCT;

HDC BeginPaint(HWND hWnd, PAINTSTRUCT *ps);
void EndPaint(HWND hWnd, PAINTSTRUCT *ps);

BOOL UpdateWindow(HWND hWnd);

BOOL GetClientRect(HWND hWnd,RECT *pRc);

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

#endif