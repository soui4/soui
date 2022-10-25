#pragma once
#include <interface/obj-ref-i.h>
#include <interface/SEvtArgs-i.h>
SNSBEGIN

typedef BOOL (*FunMsgHandler)(const LPMSG pMsg, LRESULT *pRes, void *ctx);

typedef struct _MsgHandlerInfo{
	FunMsgHandler fun;
	void * ctx;
}MsgHandlerInfo;

#undef INTERFACE
#define INTERFACE INativeWnd
DECLARE_INTERFACE_(INativeWnd, IObjRef)
{
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief 创建HWND
     * @param lpWindowName --窗口标题
     * @param dwStyle --窗口Style
     * @param dwExStyle --窗口ExStyle
     * @param x --窗口X
     * @param y --窗口Y
     * @param nWidth --窗口宽度
     * @param nHeight --窗口调试
     * @param hWndParent --父窗口
     * @param nID -- 窗口ID
     * @param lpParam --窗口自定义参数
     * @return HWND
     */
    STDMETHOD_(HWND, CreateNative)
    (THIS_ LPCTSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, int nID DEF_VAL(0), LPVOID lpParam DEF_VAL(0)) PURE;

    STDMETHOD_(HWND, GetHwnd)(THIS) PURE;

    STDMETHOD_(BOOL, SubclassWindow)(THIS_ HWND hWnd) PURE;

    STDMETHOD_(HWND, UnsubclassWindow)(THIS_ BOOL bForce DEF_VAL(FALSE)) PURE;

    STDMETHOD_(const MSG *, GetCurrentMessage)(CTHIS) SCONST PURE;

    STDMETHOD_(int, GetDlgCtrlID)(CTHIS) SCONST PURE;

    STDMETHOD_(DWORD, GetStyle)(CTHIS) SCONST PURE;

    STDMETHOD_(DWORD, GetExStyle)(CTHIS) SCONST PURE;

    STDMETHOD_(LONG_PTR, GetWindowLongPtr)(CTHIS_ int nIndex) SCONST PURE;

    STDMETHOD_(LONG_PTR, SetWindowLongPtr)(THIS_ int nIndex, LONG_PTR dwNewLong) PURE;

    STDMETHOD_(HWND, GetParent)(THIS) PURE;

    STDMETHOD_(HWND, SetParent)(THIS_ HWND hWndNewParent) PURE;

    STDMETHOD_(BOOL, IsWindowEnabled)(CTHIS) SCONST PURE;

    STDMETHOD_(BOOL, ModifyStyle)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags DEF_VAL(0)) PURE;

    STDMETHOD_(BOOL, ModifyStyleEx)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags DEF_VAL(0)) PURE;

    STDMETHOD_(BOOL, SetWindowPos)
    (THIS_ HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags) PURE;

    STDMETHOD_(BOOL, CenterWindow)(THIS_ HWND hWndCenter DEF_VAL(NULL)) PURE;

    STDMETHOD_(BOOL, DestroyWindow)(THIS) PURE;

    STDMETHOD_(BOOL, IsWindow)(THIS) PURE;

    STDMETHOD_(BOOL, Invalidate)(THIS_ BOOL bErase DEF_VAL(TRUE)) PURE;

    STDMETHOD_(BOOL, InvalidateRect)(THIS_ LPCRECT lpRect, BOOL bErase DEF_VAL(TRUE)) PURE;

    STDMETHOD_(BOOL, GetWindowRect)(CTHIS_ LPRECT lpRect) SCONST PURE;

    STDMETHOD_(BOOL, GetClientRect)(CTHIS_ LPRECT lpRect) SCONST PURE;

    STDMETHOD_(BOOL, ClientToScreen)(CTHIS_ LPPOINT lpPoint) SCONST PURE;

    STDMETHOD_(BOOL, ClientToScreen2)(CTHIS_ LPRECT lpRect) SCONST PURE;

    STDMETHOD_(BOOL, ScreenToClient)(CTHIS_ LPPOINT lpPoint) SCONST PURE;

    STDMETHOD_(BOOL, ScreenToClient2)(CTHIS_ LPRECT lpRect) SCONST PURE;

    STDMETHOD_(int, MapWindowPoints)(CTHIS_ HWND hWndTo, LPPOINT lpPoint, UINT nCount) SCONST PURE;

    STDMETHOD_(int, MapWindowRect)(CTHIS_ HWND hWndTo, LPRECT lpRect) SCONST PURE;

    STDMETHOD_(UINT_PTR, SetTimer)
    (THIS_ UINT_PTR nIDEvent, UINT nElapse, void(CALLBACK * lpfnTimer)(HWND, UINT, UINT_PTR, DWORD) DEF_VAL(NULL)) PURE;

    STDMETHOD_(BOOL, KillTimer)(THIS_ UINT_PTR nIDEvent) PURE;

    STDMETHOD_(HDC, GetDC)(THIS) PURE;

    STDMETHOD_(HDC, GetWindowDC)(THIS) PURE;

    STDMETHOD_(int, ReleaseDC)(THIS_ HDC hDC) PURE;

    STDMETHOD_(BOOL, CreateCaret)(THIS_ HBITMAP hBitmap) PURE;

    STDMETHOD_(BOOL, HideCaret)(THIS) PURE;

    STDMETHOD_(BOOL, ShowCaret)(THIS) PURE;

    STDMETHOD_(HWND, GetCapture)(THIS) PURE;

    STDMETHOD_(HWND, SetCapture)(THIS) PURE;

    STDMETHOD_(BOOL, ReleaseCapture)(THIS) PURE;

    STDMETHOD_(HWND, SetFocus)(THIS) PURE;

    STDMETHOD_(LRESULT, SendMessage)
    (THIS_ UINT message, WPARAM wParam DEF_VAL(0), LPARAM lParam DEF_VAL(0)) PURE;

    STDMETHOD_(BOOL, PostMessage)
    (THIS_ UINT message, WPARAM wParam DEF_VAL(0), LPARAM lParam DEF_VAL(0)) PURE;

    STDMETHOD_(BOOL, SendNotifyMessage)
    (THIS_ UINT message, WPARAM wParam DEF_VAL(0), LPARAM lParam DEF_VAL(0)) PURE;

    STDMETHOD_(BOOL, SetWindowText)(THIS_ LPCTSTR lpszString) PURE;

    STDMETHOD_(int, GetWindowText)(CTHIS_ LPTSTR lpszStringBuf, int nMaxCount) SCONST PURE;

    STDMETHOD_(BOOL, IsIconic)(CTHIS) SCONST PURE;

    STDMETHOD_(BOOL, IsZoomed)(CTHIS) SCONST PURE;

    STDMETHOD_(BOOL, IsWindowVisible)(CTHIS) SCONST PURE;

    STDMETHOD_(BOOL, MoveWindow)
    (THIS_ int x, int y, int nWidth, int nHeight, BOOL bRepaint DEF_VAL(TRUE)) PURE;

    STDMETHOD_(BOOL, MoveWindow2)(THIS_ LPCRECT lpRect, BOOL bRepaint DEF_VAL(TRUE)) PURE;

    STDMETHOD_(BOOL, ShowWindow)(THIS_ int nCmdShow) PURE;

    STDMETHOD_(int, SetWindowRgn)(THIS_ HRGN hRgn, BOOL bRedraw DEF_VAL(TRUE)) PURE;

    STDMETHOD_(BOOL, SetLayeredWindowAttributes)
    (THIS_ COLORREF crKey, BYTE bAlpha, DWORD dwFlags) PURE;

    STDMETHOD_(BOOL, UpdateLayeredWindow)
    (THIS_ HDC hdcDst, POINT * pptDst, SIZE * psize, HDC hdcSrc, POINT * pptSrc, COLORREF crKey, BLENDFUNCTION * pblend, DWORD dwFlags) PURE;

    STDMETHOD_(void, SetMsgHandler)(THIS_ FunMsgHandler fun, void *ctx) PURE;

	STDMETHOD_(MsgHandlerInfo *,GetMsgHandler)(THIS) PURE;
};

SNSEND