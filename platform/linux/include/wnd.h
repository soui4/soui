#ifndef _WND_H__
#define _WND_H__
#include <platform.h>


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

HDC WINAPI GetDC(HWND hWnd);
void WINAPI ReleaseDC(HWND hwnd, HDC hdc);

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
void GetWindowRect(HWND hWnd,RECT *rc);

BOOL SetNativeWndAlpha(HWND hWnd, BYTE byAlpha);


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

HRESULT DefWindowProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp);
#endif//_WND_H__