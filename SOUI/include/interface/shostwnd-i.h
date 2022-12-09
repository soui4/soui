#pragma once
#include <interface/SNativeWnd-i.h>
#include <interface/SNcPainter-i.h>
#include <interface/smsgloop-i.h>

SNSBEGIN

typedef struct IWindow IWindow;
typedef struct IApplication IApplication;
typedef struct IHostPresenter IHostPresenter;
typedef struct IHostWnd IHostWnd;

typedef struct _EventHandlerInfo{
	FunCallback fun;
	void * ctx;
}EventHandlerInfo;

#undef INTERFACE
#define INTERFACE IHostWnd
DECLARE_INTERFACE_(IHostWnd, INativeWnd)
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
    //////////////////////////////////////////////////////////////////////////
    STDMETHOD_(HWND, CreateNative)
    (THIS_ LPCTSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, int nID, LPVOID lpParam) PURE;

    STDMETHOD_(HWND, GetHwnd)(THIS) PURE;

    STDMETHOD_(BOOL, SubclassWindow)(THIS_ HWND hWnd) PURE;

    STDMETHOD_(HWND, UnsubclassWindow)(THIS_ BOOL bForce /*= FALSE*/) PURE;

    STDMETHOD_(const MSG *, GetCurrentMessage)(CTHIS) SCONST PURE;

    STDMETHOD_(int, GetDlgCtrlID)(CTHIS) SCONST PURE;

    STDMETHOD_(DWORD, GetStyle)(CTHIS) SCONST PURE;

    STDMETHOD_(DWORD, GetExStyle)(CTHIS) SCONST PURE;

    STDMETHOD_(LONG_PTR, GetWindowLongPtr)(CTHIS_ int nIndex) SCONST PURE;

    STDMETHOD_(LONG_PTR, SetWindowLongPtr)(THIS_ int nIndex, LONG_PTR dwNewLong) PURE;

    STDMETHOD_(HWND, GetParent)(THIS) PURE;

    STDMETHOD_(HWND, SetParent)(THIS_ HWND hWndNewParent) PURE;

    STDMETHOD_(BOOL, IsWindowEnabled)(CTHIS) SCONST PURE;

    STDMETHOD_(BOOL, ModifyStyle)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags /*=0*/) PURE;

    STDMETHOD_(BOOL, ModifyStyleEx)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags /*=0*/) PURE;

    STDMETHOD_(BOOL, SetWindowPos)
    (THIS_ HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags) PURE;

    STDMETHOD_(BOOL, CenterWindow)(THIS_ HWND hWndCenter /*= NULL*/) PURE;

    STDMETHOD_(BOOL, DestroyWindow)(THIS) PURE;

    STDMETHOD_(BOOL, IsWindow)(THIS) PURE;

    STDMETHOD_(BOOL, Invalidate)(THIS_ BOOL bErase /*= TRUE*/) PURE;

    STDMETHOD_(BOOL, InvalidateRect)(THIS_ LPCRECT lpRect, BOOL bErase /* = TRUE*/) PURE;

    STDMETHOD_(BOOL, GetWindowRect)(CTHIS_ LPRECT lpRect) SCONST PURE;

    STDMETHOD_(BOOL, GetClientRect)(CTHIS_ LPRECT lpRect) SCONST PURE;

    STDMETHOD_(BOOL, ClientToScreen)(CTHIS_ LPPOINT lpPoint) SCONST PURE;

    STDMETHOD_(BOOL, ClientToScreen2)(CTHIS_ LPRECT lpRect) SCONST PURE;

    STDMETHOD_(BOOL, ScreenToClient)(CTHIS_ LPPOINT lpPoint) SCONST PURE;

    STDMETHOD_(BOOL, ScreenToClient2)(CTHIS_ LPRECT lpRect) SCONST PURE;

    STDMETHOD_(int, MapWindowPoints)(CTHIS_ HWND hWndTo, LPPOINT lpPoint, UINT nCount) SCONST PURE;

    STDMETHOD_(int, MapWindowRect)(CTHIS_ HWND hWndTo, LPRECT lpRect) SCONST PURE;

    STDMETHOD_(UINT_PTR, SetTimer)
    (THIS_ UINT_PTR nIDEvent, UINT nElapse, void(CALLBACK * lpfnTimer)(HWND, UINT, UINT_PTR, DWORD) /*= NULL*/) PURE;

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
    (THIS_ UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) PURE;

    STDMETHOD_(BOOL, PostMessage)
    (THIS_ UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) PURE;

    STDMETHOD_(BOOL, SendNotifyMessage)
    (THIS_ UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) PURE;

    STDMETHOD_(BOOL, SetWindowText)(THIS_ LPCTSTR lpszString) PURE;

    STDMETHOD_(int, GetWindowText)(CTHIS_ LPTSTR lpszStringBuf, int nMaxCount) SCONST PURE;

    STDMETHOD_(BOOL, IsIconic)(CTHIS) SCONST PURE;

    STDMETHOD_(BOOL, IsZoomed)(CTHIS) SCONST PURE;

    STDMETHOD_(BOOL, IsWindowVisible)(CTHIS) SCONST PURE;

    STDMETHOD_(BOOL, MoveWindow)
    (THIS_ int x, int y, int nWidth, int nHeight, BOOL bRepaint /*= TRUE*/) PURE;

    STDMETHOD_(BOOL, MoveWindow2)(THIS_ LPCRECT lpRect, BOOL bRepaint /*= TRUE*/) PURE;

    STDMETHOD_(BOOL, ShowWindow)(THIS_ int nCmdShow) PURE;

    STDMETHOD_(int, SetWindowRgn)(THIS_ HRGN hRgn, BOOL bRedraw /*=TRUE*/) PURE;

    STDMETHOD_(BOOL, SetLayeredWindowAttributes)
    (THIS_ COLORREF crKey, BYTE bAlpha, DWORD dwFlags) PURE;

    STDMETHOD_(BOOL, UpdateLayeredWindow)
    (THIS_ HDC hdcDst, POINT * pptDst, SIZE * psize, HDC hdcSrc, POINT * pptSrc, COLORREF crKey, BLENDFUNCTION * pblend, DWORD dwFlags) PURE;

    STDMETHOD_(void, SetMsgHandler)(THIS_ FunMsgHandler fun, void *ctx) PURE;
	STDMETHOD_(MsgHandlerInfo *,GetMsgHandler)(THIS) PURE;

    //////////////////////////////////////////////////////////////////////////
    STDMETHOD_(HWND, CreateEx)
    (THIS_ HWND hWndParent, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight) PURE;
	STDMETHOD_(HWND, Create)
	(THIS_ HWND hWndParent, int x DEF_VAL(0), int y DEF_VAL(0), int nWidth DEF_VAL(0), int nHeight DEF_VAL(0)) PURE;
    STDMETHOD_(void, SetLayoutId)(THIS_ LPCTSTR pszLayoutId) PURE;
    STDMETHOD_(IWindow *, GetIRoot)(THIS) PURE;
	STDMETHOD_(BOOL,IsTranslucent)(CTHIS) SCONST PURE;
	STDMETHOD_(IHostPresenter*,GetPresenter)(THIS) PURE;
	STDMETHOD_(void,SetPresenter)(THIS_ IHostPresenter* pPresenter) PURE;
	STDMETHOD_(IMessageLoop*,GetMsgLoop)(THIS) PURE;

    /**
     * @brief 根据ID查找子窗口
     * @param nId int--子窗口ID
     * @return IWindow*--匹配窗口
     * @remark 采用广度优先算法搜索匹配子窗口
     */
    STDMETHOD_(IWindow *, FindIChildByID)(THIS_ int nId) PURE;

    /**
     * @brief 根据Name查找子窗口
     * @param pszName LPCWSTR--子窗口Name
     * @return IWindow*--匹配窗口
     * @remark 采用广度优先算法搜索匹配子窗口
     */
    STDMETHOD_(IWindow *, FindIChildByName)(THIS_ LPCWSTR pszName) PURE;

	STDMETHOD_(IWindow *, FindIChildByNameA)(THIS_ LPCSTR pszName) PURE;

    STDMETHOD_(INcPainter *, GetNcPainter)(THIS) PURE;

    STDMETHOD_(void, SetEventHandler)(THIS_ FunCallback fun, void *ctx) PURE;
	
	STDMETHOD_(EventHandlerInfo*,GetEventHandler)(THIS) PURE;

	STDMETHOD_(BOOL, AnimateHostWindow)(THIS_ DWORD dwTime, DWORD dwFlags) PURE;
	STDMETHOD_(void,EnableDragDrop)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IHostDialog
DECLARE_INTERFACE_(IHostDialog, IHostWnd)
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
    //////////////////////////////////////////////////////////////////////////
    STDMETHOD_(HWND, CreateNative)
    (THIS_ LPCTSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, int nID, LPVOID lpParam) PURE;

    STDMETHOD_(HWND, GetHwnd)(THIS) PURE;

    STDMETHOD_(BOOL, SubclassWindow)(THIS_ HWND hWnd) PURE;

    STDMETHOD_(HWND, UnsubclassWindow)(THIS_ BOOL bForce /*= FALSE*/) PURE;

    STDMETHOD_(const MSG *, GetCurrentMessage)(CTHIS) SCONST PURE;

    STDMETHOD_(int, GetDlgCtrlID)(CTHIS) SCONST PURE;

    STDMETHOD_(DWORD, GetStyle)(CTHIS) SCONST PURE;

    STDMETHOD_(DWORD, GetExStyle)(CTHIS) SCONST PURE;

    STDMETHOD_(LONG_PTR, GetWindowLongPtr)(CTHIS_ int nIndex) SCONST PURE;

    STDMETHOD_(LONG_PTR, SetWindowLongPtr)(THIS_ int nIndex, LONG_PTR dwNewLong) PURE;

    STDMETHOD_(HWND, GetParent)(THIS) PURE;

    STDMETHOD_(HWND, SetParent)(THIS_ HWND hWndNewParent) PURE;

    STDMETHOD_(BOOL, IsWindowEnabled)(CTHIS) SCONST PURE;

    STDMETHOD_(BOOL, ModifyStyle)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags /*=0*/) PURE;

    STDMETHOD_(BOOL, ModifyStyleEx)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags /*=0*/) PURE;

    STDMETHOD_(BOOL, SetWindowPos)
    (THIS_ HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags) PURE;

    STDMETHOD_(BOOL, CenterWindow)(THIS_ HWND hWndCenter /*= NULL*/) PURE;

    STDMETHOD_(BOOL, DestroyWindow)(THIS) PURE;

    STDMETHOD_(BOOL, IsWindow)(THIS) PURE;

    STDMETHOD_(BOOL, Invalidate)(THIS_ BOOL bErase /*= TRUE*/) PURE;

    STDMETHOD_(BOOL, InvalidateRect)(THIS_ LPCRECT lpRect, BOOL bErase /* = TRUE*/) PURE;

    STDMETHOD_(BOOL, GetWindowRect)(CTHIS_ LPRECT lpRect) SCONST PURE;

    STDMETHOD_(BOOL, GetClientRect)(CTHIS_ LPRECT lpRect) SCONST PURE;

    STDMETHOD_(BOOL, ClientToScreen)(CTHIS_ LPPOINT lpPoint) SCONST PURE;

    STDMETHOD_(BOOL, ClientToScreen2)(CTHIS_ LPRECT lpRect) SCONST PURE;

    STDMETHOD_(BOOL, ScreenToClient)(CTHIS_ LPPOINT lpPoint) SCONST PURE;

    STDMETHOD_(BOOL, ScreenToClient2)(CTHIS_ LPRECT lpRect) SCONST PURE;

    STDMETHOD_(int, MapWindowPoints)(CTHIS_ HWND hWndTo, LPPOINT lpPoint, UINT nCount) SCONST PURE;

    STDMETHOD_(int, MapWindowRect)(CTHIS_ HWND hWndTo, LPRECT lpRect) SCONST PURE;

    STDMETHOD_(UINT_PTR, SetTimer)
    (THIS_ UINT_PTR nIDEvent, UINT nElapse, void(CALLBACK * lpfnTimer)(HWND, UINT, UINT_PTR, DWORD) /*= NULL*/) PURE;

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
    (THIS_ UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) PURE;

    STDMETHOD_(BOOL, PostMessage)
    (THIS_ UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) PURE;

    STDMETHOD_(BOOL, SendNotifyMessage)
    (THIS_ UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) PURE;

    STDMETHOD_(BOOL, SetWindowText)(THIS_ LPCTSTR lpszString) PURE;

    STDMETHOD_(int, GetWindowText)(CTHIS_ LPTSTR lpszStringBuf, int nMaxCount) SCONST PURE;

    STDMETHOD_(BOOL, IsIconic)(CTHIS) SCONST PURE;

    STDMETHOD_(BOOL, IsZoomed)(CTHIS) SCONST PURE;

    STDMETHOD_(BOOL, IsWindowVisible)(CTHIS) SCONST PURE;

    STDMETHOD_(BOOL, MoveWindow)
    (THIS_ int x, int y, int nWidth, int nHeight, BOOL bRepaint /*= TRUE*/) PURE;

    STDMETHOD_(BOOL, MoveWindow2)(THIS_ LPCRECT lpRect, BOOL bRepaint /*= TRUE*/) PURE;

    STDMETHOD_(BOOL, ShowWindow)(THIS_ int nCmdShow) PURE;

    STDMETHOD_(int, SetWindowRgn)(THIS_ HRGN hRgn, BOOL bRedraw /*=TRUE*/) PURE;

    STDMETHOD_(BOOL, SetLayeredWindowAttributes)
    (THIS_ COLORREF crKey, BYTE bAlpha, DWORD dwFlags) PURE;

    STDMETHOD_(BOOL, UpdateLayeredWindow)
    (THIS_ HDC hdcDst, POINT * pptDst, SIZE * psize, HDC hdcSrc, POINT * pptSrc, COLORREF crKey, BLENDFUNCTION * pblend, DWORD dwFlags) PURE;

    STDMETHOD_(void, SetMsgHandler)(THIS_ FunMsgHandler fun, void *ctx) PURE;
	STDMETHOD_(MsgHandlerInfo *,GetMsgHandler)(THIS) PURE;

    //////////////////////////////////////////////////////////////////////////
    STDMETHOD_(HWND, CreateEx)
    (THIS_ HWND hWndParent, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight) PURE;
    STDMETHOD_(HWND, Create)
    (THIS_ HWND hWndParent, int x DEF_VAL(0), int y DEF_VAL(0), int nWidth DEF_VAL(0), int nHeight DEF_VAL(0)) PURE;
    STDMETHOD_(void, SetLayoutId)(THIS_ LPCTSTR pszLayoutId) PURE;
    STDMETHOD_(IWindow *, GetIRoot)(THIS) PURE;
	STDMETHOD_(BOOL,IsTranslucent)(CTHIS) SCONST PURE;
	STDMETHOD_(IHostPresenter*,GetPresenter)(THIS) PURE;
	STDMETHOD_(void,SetPresenter)(THIS_ IHostPresenter* pPresenter) PURE;
	STDMETHOD_(IMessageLoop*,GetMsgLoop)(THIS) PURE;

	/**
     * @brief 根据ID查找子窗口
     * @param nId int--子窗口ID
     * @return IWindow*--匹配窗口
     * @remark 采用广度优先算法搜索匹配子窗口
     */
    STDMETHOD_(IWindow *, FindIChildByID)(THIS_ int nId) PURE;

    /**
     * @brief 根据Name查找子窗口
     * @param pszName LPCWSTR--子窗口Name
     * @return IWindow*--匹配窗口
     * @remark 采用广度优先算法搜索匹配子窗口
     */
    STDMETHOD_(IWindow *, FindIChildByName)(THIS_ LPCWSTR pszName) PURE;

	STDMETHOD_(IWindow *, FindIChildByNameA)(THIS_ LPCSTR pszName) PURE;

    STDMETHOD_(INcPainter *, GetNcPainter)(THIS) PURE;
    STDMETHOD_(void, SetEventHandler)(THIS_ FunCallback fun, void *ctx) PURE;
	STDMETHOD_(EventHandlerInfo*,GetEventHandler)(THIS) PURE;

	STDMETHOD_(BOOL, AnimateHostWindow)(THIS_ DWORD dwTime, DWORD dwFlags) PURE;
	STDMETHOD_(void,EnableDragDrop)(THIS) PURE;

    //////////////////////////////////////////////////////////////////////////
    STDMETHOD_(INT_PTR, DoModal)(THIS_ HWND hParent /*=NULL*/) PURE;
    STDMETHOD_(void, EndDialog)(THIS_ INT_PTR nResult) PURE;
};

SNSEND
