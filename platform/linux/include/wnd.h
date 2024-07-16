#ifndef _WND_H__
#define _WND_H__

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam) ((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam) ((int)(short)HIWORD(lParam))
#endif

/***********************************************************************
 *		CreateWindowExW (USER32.@)
 */
HWND WINAPI CreateWindowEx( DWORD exStyle, LPCSTR className,
                                 LPCSTR windowName, DWORD style, INT x,
                                 INT y, INT width, INT height,
                                 HWND parent, HMENU menu,
                                 HINSTANCE instance, LPVOID data );


BOOL WINAPI DestroyWindow(HWND hWnd);

BOOL WINAPI IsWindow(HWND hwnd);

BOOL
WINAPI PostMessage(
     HWND hWnd,
     UINT Msg,
     WPARAM wParam,
     LPARAM lParam);

LRESULT WINAPI CallWindowProc(WNDPROC proc, HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

LRESULT WINAPI SendMessage(
     HWND hWnd,
     UINT Msg,
     WPARAM wParam,
     LPARAM lParam);

int
WINAPI MessageBox(
     HWND hWnd,
     LPCTSTR lpText,
     LPCTSTR lpCaption,
     UINT uType);

BOOL
WINAPI SetForegroundWindow(
 HWND hWnd);

HWND WINAPI GetForegroundWindow();

BOOL
WINAPI SetWindowPos(
     HWND hWnd,
     HWND hWndInsertAfter,
     int X,
     int Y,
     int cx,
     int cy,
     UINT uFlags);

LONG GetWindowLong(HWND hWnd,int nIndex);

LONG SetWindowLong(HWND hWnd, int nIndex,LONG data);

LONG_PTR GetWindowLongPtr(HWND hWnd,int nIndex);

LONG_PTR SetWindowLongPtr(HWND hWnd, int nIndex,LONG_PTR data);


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

BOOL EnableWindow(HWND hWnd,BOOL bEnable);

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
     HDC hdc;
}PAINTSTRUCT;

HDC BeginPaint(HWND hWnd, PAINTSTRUCT *ps);
BOOL EndPaint(HWND hWnd, const PAINTSTRUCT *ps);

BOOL UpdateWindow(HWND hWnd);

BOOL GetClientRect(HWND hWnd,RECT *lpRect);
BOOL GetWindowRect(HWND hWnd,RECT *lpRect);
BOOL InvalidateRect(
  HWND       hWnd,
  const RECT *lpRect,
  BOOL       bErase
);
BOOL SetNativeWndAlpha(HWND hWnd, BYTE byAlpha);


BOOL ClientToScreen(HWND hWnd,LPPOINT ppt);
BOOL ScreenToClient(HWND hWnd,LPPOINT ppt);

HMONITOR
MonitorFromWindow(
    HWND hwnd,
    DWORD dwFlags);

BOOL
GetMonitorInfo(
 HMONITOR hMonitor,
 LPMONITORINFO lpmi);

HRESULT DefWindowProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp);

BOOL ShowWindow(HWND hWnd, int nCmdShow);

BOOL MoveWindow(HWND hWnd,int x, int y, int nWidth, int nHeight, BOOL bRepaint);

BOOL IsWindowVisible(HWND hWnd);

BOOL IsZoomed(HWND hWnd);
BOOL IsIconic(HWND hWnd);
int GetWindowText(HWND hWnd, LPTSTR lpszStringBuf, int nMaxCount);

int GetWindowTextLength(HWND hWnd);
BOOL SetWindowText(HWND hWnd , LPCTSTR lpszString);
HDC GetDC(HWND hWnd);
int ReleaseDC(HWND hWnd,HDC hdc);

int MapWindowPoints(HWND hWndFrom,HWND hWndTo, LPPOINT lpPoint, UINT nCount);

#endif//_WND_H__