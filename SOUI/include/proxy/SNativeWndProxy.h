#ifndef __SNATIVEWNDPROXY__H__
#define __SNATIVEWNDPROXY__H__

SNSBEGIN

template <class T>
class TNativeWndProxy
	: public T
	, public SNativeWnd {
public:
	STDMETHOD_(long, AddRef)(THIS)
	{
		return SNativeWnd::AddRef();
	}
	STDMETHOD_(long, Release)(THIS)
	{
		return SNativeWnd::Release();
	}
	STDMETHOD_(void, OnFinalRelease)(THIS)
	{
		SNativeWnd::OnFinalRelease();
	}
	STDMETHOD_(int, GetID)(THIS) SCONST
	{
		return SNativeWnd::GetID();
	}
	STDMETHOD_(void, SetID)(THIS_ int nID)
	{
		return SNativeWnd::SetID(nID);
	}

	STDMETHOD_(HWND, CreateNative)
		(THIS_ LPCTSTR lpWindowName,
		DWORD dwStyle,
		DWORD dwExStyle,
		int x,
		int y,
		int nWidth,
		int nHeight,
		HWND hWndParent,
		int nID,
		LPVOID lpParam) OVERRIDE
	{
		return SNativeWnd::CreateNative(lpWindowName, dwStyle, dwExStyle, x, y, nWidth, nHeight,
			hWndParent,nID, lpParam);
	}

	STDMETHOD_(HWND, GetHwnd)(THIS) OVERRIDE
	{
		return SNativeWnd::GetHwnd();
	}

	STDMETHOD_(BOOL, SubclassWindow)(THIS_ HWND hWnd) OVERRIDE
	{
		return SNativeWnd::SubclassWindow(hWnd);
	}
	STDMETHOD_(HWND, UnsubclassWindow)(THIS_ BOOL bForce /*= FALSE*/) OVERRIDE
	{
		return SNativeWnd::UnsubclassWindow(bForce);
	}
	STDMETHOD_(const MSG *, GetCurrentMessage)(THIS) SCONST OVERRIDE
	{
		return SNativeWnd::GetCurrentMessage();
	}
	STDMETHOD_(int, GetDlgCtrlID)(THIS) SCONST OVERRIDE
	{
		return SNativeWnd::GetDlgCtrlID();
	}
	STDMETHOD_(DWORD, GetStyle)(THIS) SCONST OVERRIDE
	{
		return SNativeWnd::GetStyle();
	}
	STDMETHOD_(DWORD, GetExStyle)(THIS) SCONST OVERRIDE
	{
		return SNativeWnd::GetExStyle();
	}
	STDMETHOD_(LONG_PTR, GetWindowLongPtr)(THIS_ int nIndex) SCONST OVERRIDE
	{
		return SNativeWnd::GetWindowLongPtr(nIndex);
	}
	STDMETHOD_(LONG_PTR, SetWindowLongPtr)(THIS_ int nIndex, LONG_PTR dwNewLong) OVERRIDE
	{
		return SNativeWnd::SetWindowLongPtr(nIndex, dwNewLong);
	}
	STDMETHOD_(HWND, GetParent)(THIS) OVERRIDE
	{
		return SNativeWnd::GetParent();
	}
	STDMETHOD_(HWND, SetParent)(THIS_ HWND hWndNewParent) OVERRIDE
	{
		return SNativeWnd::SetParent(hWndNewParent);
	}
	STDMETHOD_(BOOL, IsWindowEnabled)(THIS) SCONST OVERRIDE
	{
		return SNativeWnd::IsWindowEnabled();
	}
	STDMETHOD_(BOOL, ModifyStyle)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0) OVERRIDE
	{
		return SNativeWnd::ModifyStyle(dwRemove, dwAdd, nFlags);
	}
	STDMETHOD_(BOOL, ModifyStyleEx)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0) OVERRIDE
	{
		return SNativeWnd::ModifyStyleEx(dwRemove, dwAdd, nFlags);
	}
	STDMETHOD_(BOOL, SetWindowPos)
		(THIS_ HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags) OVERRIDE
	{
		return SNativeWnd::SetWindowPos(hWndInsertAfter, x, y, cx, cy, nFlags);
	}
	STDMETHOD_(BOOL, CenterWindow)(THIS_ HWND hWndCenter = NULL) OVERRIDE
	{
		return SNativeWnd::CenterWindow(hWndCenter);
	}
	STDMETHOD_(BOOL, IsWindow)(CTHIS) SCONST OVERRIDE
	{
		return SNativeWnd::IsWindow();
	}
	STDMETHOD_(BOOL, Invalidate)(THIS_ BOOL bErase = TRUE) OVERRIDE
	{
		return SNativeWnd::Invalidate(bErase);
	}
	STDMETHOD_(BOOL, InvalidateRect)(THIS_ LPCRECT lpRect, BOOL bErase = TRUE) OVERRIDE
	{
		return SNativeWnd::InvalidateRect(lpRect, bErase);
	}
	STDMETHOD_(BOOL, GetWindowRect)(THIS_ LPRECT lpRect) SCONST OVERRIDE
	{
		return SNativeWnd::GetWindowRect(lpRect);
	}
	STDMETHOD_(BOOL, GetClientRect)(THIS_ LPRECT lpRect) SCONST OVERRIDE
	{
		return SNativeWnd::GetClientRect(lpRect);
	}
	STDMETHOD_(BOOL, ClientToScreen)(THIS_ LPPOINT lpPoint) SCONST OVERRIDE
	{
		return SNativeWnd::ClientToScreen(lpPoint);
	}
	STDMETHOD_(BOOL, ClientToScreen2)(THIS_ LPRECT lpRect) SCONST OVERRIDE
	{
		return SNativeWnd::ClientToScreen2(lpRect);
	}
	STDMETHOD_(BOOL, ScreenToClient)(THIS_ LPPOINT lpPoint) SCONST OVERRIDE
	{
		return SNativeWnd::ScreenToClient(lpPoint);
	}
	STDMETHOD_(BOOL, ScreenToClient2)(THIS_ LPRECT lpRect) SCONST OVERRIDE
	{
		return SNativeWnd::ScreenToClient2(lpRect);
	}
	STDMETHOD_(int, MapWindowPoints)
		(THIS_ HWND hWndTo, LPPOINT lpPoint, UINT nCount) SCONST OVERRIDE
	{
		return SNativeWnd::MapWindowPoints(hWndTo, lpPoint, nCount);
	}
	STDMETHOD_(int, MapWindowRect)(THIS_ HWND hWndTo, LPRECT lpRect) SCONST OVERRIDE
	{
		return SNativeWnd::MapWindowRect(hWndTo, lpRect);
	}
	STDMETHOD_(UINT_PTR, SetTimer)
		(THIS_ UINT_PTR nIDEvent,
		UINT nElapse,
		void(CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD) = NULL) OVERRIDE
	{
		return SNativeWnd::SetTimer(nIDEvent, nElapse, lpfnTimer);
	}
	STDMETHOD_(BOOL, KillTimer)(THIS_ UINT_PTR nIDEvent) OVERRIDE
	{
		return SNativeWnd::KillTimer(nIDEvent);
	}
	STDMETHOD_(HDC, GetDC)(THIS) OVERRIDE
	{
		return SNativeWnd::GetDC();
	}
	STDMETHOD_(HDC, GetWindowDC)(THIS) OVERRIDE
	{
		return SNativeWnd::GetWindowDC();
	}
	STDMETHOD_(int, ReleaseDC)(THIS_ HDC hDC) OVERRIDE
	{
		return SNativeWnd::ReleaseDC(hDC);
	}
    STDMETHOD_(BOOL, CreateCaret)(THIS_ HBITMAP hBitmap, int nWidth, int nHeight) OVERRIDE
	{
        return SNativeWnd::CreateCaret(hBitmap, nWidth, nHeight);
	}
	STDMETHOD_(BOOL, HideCaret)(THIS) OVERRIDE
	{
		return SNativeWnd::HideCaret();
	}
	STDMETHOD_(BOOL, ShowCaret)(THIS) OVERRIDE
	{
		return SNativeWnd::ShowCaret();
	}
	STDMETHOD_(HWND, SetCapture)(THIS) OVERRIDE
	{
		return SNativeWnd::SetCapture();
	}

	STDMETHOD_(HWND, GetCapture)(THIS) OVERRIDE
	{
		return SNativeWnd::GetCapture();
	}

	STDMETHOD_(BOOL, ReleaseCapture)(THIS) OVERRIDE
	{
		return SNativeWnd::ReleaseCapture();
	}

	STDMETHOD_(HWND, SetFocus)(THIS) OVERRIDE
	{
		return SNativeWnd::SetFocus();
	}
	STDMETHOD_(LRESULT, SendMessage)
		(THIS_ UINT message, WPARAM wParam = 0, LPARAM lParam = 0) OVERRIDE
	{
		return SNativeWnd::SendMessage(message, wParam, lParam);
	}
	STDMETHOD_(BOOL, PostMessage)(THIS_ UINT message, WPARAM wParam = 0, LPARAM lParam = 0) OVERRIDE
	{
		return SNativeWnd::PostMessage(message, wParam, lParam);
	}
	STDMETHOD_(BOOL, SendNotifyMessage)
		(THIS_ UINT message, WPARAM wParam = 0, LPARAM lParam = 0) OVERRIDE
	{
		return SNativeWnd::SendNotifyMessage(message, wParam, lParam);
	}
	STDMETHOD_(BOOL, SetWindowText)(THIS_ LPCTSTR lpszString) OVERRIDE
	{
		return SNativeWnd::SetWindowText(lpszString);
	}
	STDMETHOD_(int, GetWindowText)(THIS_ LPTSTR lpszStringBuf, int nMaxCount) SCONST OVERRIDE
	{
		return SNativeWnd::GetWindowText(lpszStringBuf, nMaxCount);
	}
	STDMETHOD_(BOOL, IsIconic)(THIS) SCONST OVERRIDE
	{
		return SNativeWnd::IsIconic();
	}
	STDMETHOD_(BOOL, IsZoomed)(THIS) SCONST OVERRIDE
	{
		return SNativeWnd::IsZoomed();
	}
	STDMETHOD_(BOOL, IsWindowVisible)(THIS) SCONST OVERRIDE
	{
		return SNativeWnd::IsWindowVisible();
	}
	STDMETHOD_(BOOL, MoveWindow)
		(THIS_ int x, int y, int nWidth, int nHeight, BOOL bRepaint = TRUE) OVERRIDE
	{
		return SNativeWnd::MoveWindow(x, y, nWidth, nHeight, bRepaint);
	}
	STDMETHOD_(BOOL, MoveWindow2)(THIS_ LPCRECT lpRect, BOOL bRepaint = TRUE) OVERRIDE
	{
		return SNativeWnd::MoveWindow2(lpRect, bRepaint);
	}
	STDMETHOD_(BOOL, ShowWindow)(THIS_ int nCmdShow) OVERRIDE
	{
		return SNativeWnd::ShowWindow(nCmdShow);
	}
	STDMETHOD_(int, SetWindowRgn)(THIS_ HRGN hRgn, BOOL bRedraw = TRUE) OVERRIDE
	{
		return SNativeWnd::SetWindowRgn(hRgn, bRedraw);
	}
	STDMETHOD_(BOOL, SetLayeredWindowAttributes)
		(THIS_ COLORREF crKey, BYTE byAlpha, DWORD dwFlags) OVERRIDE
	{
		return SNativeWnd::SetLayeredWindowAttributes(crKey, byAlpha, dwFlags);
	}
	STDMETHOD_(BOOL, UpdateLayeredWindow)
		(THIS_ HDC hdcDst,
		POINT *pptDst,
		SIZE *psize,
		HDC hdcSrc,
		POINT *pptSrc,
		COLORREF crKey,
		BLENDFUNCTION *pblend,
		DWORD dwFlags)
	{
		return SNativeWnd::UpdateLayeredWindow(hdcDst, pptDst, psize, hdcSrc, pptSrc, crKey, pblend,
			dwFlags);
	}

	STDMETHOD_(void, SetMsgHandler)(THIS_ FunMsgHandler fun, void *ctx) OVERRIDE
	{
		return SNativeWnd::SetMsgHandler(fun, ctx);
	}

	STDMETHOD_(MsgHandlerInfo *, GetMsgHandler)(THIS) OVERRIDE{
		return SNativeWnd::GetMsgHandler();
	}

};

SNSEND
#endif // __SNATIVEWNDPROXY__H__