#ifndef __SHOSTWNDPROXY__H__
#define __SHOSTWNDPROXY__H__

SNSBEGIN


template <class T>
class THostWndProxy
	: public T
	, public SHostWnd {
public:
	THostWndProxy(LPCWSTR pszResId)
		: SHostWnd(pszResId)
	{
	}
	THostWndProxy(LPCSTR pszResId)
		: SHostWnd(pszResId)
	{
	}

public:
	STDMETHOD_(long, AddRef)(THIS)
	{
		return SHostWnd::AddRef();
	}
	STDMETHOD_(long, Release)(THIS)
	{
		return SHostWnd::Release();
	}
	STDMETHOD_(void, OnFinalRelease)(THIS)
	{
		SHostWnd::OnFinalRelease();
	}
	STDMETHOD_(int, GetID)(THIS) SCONST
	{
		return SHostWnd::GetID();
	}
	STDMETHOD_(void, SetID)(THIS_ int nID)
	{
		return SHostWnd::SetID(nID);
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
		return SHostWnd::CreateNative(lpWindowName, dwStyle, dwExStyle, x, y, nWidth, nHeight,
			hWndParent, nID, lpParam);
	}

	STDMETHOD_(HWND, GetHwnd)(THIS) OVERRIDE
	{
		return SHostWnd::GetHwnd();
	}

	STDMETHOD_(BOOL, SubclassWindow)(THIS_ HWND hWnd) OVERRIDE
	{
		return SHostWnd::SubclassWindow(hWnd);
	}
	STDMETHOD_(HWND, UnsubclassWindow)(THIS_ BOOL bForce = FALSE) OVERRIDE
	{
		return SHostWnd::UnsubclassWindow(bForce);
	}
	STDMETHOD_(const MSG *, GetCurrentMessage)(THIS) SCONST OVERRIDE
	{
		return SHostWnd::GetCurrentMessage();
	}
	STDMETHOD_(int, GetDlgCtrlID)(THIS) SCONST OVERRIDE
	{
		return SHostWnd::GetDlgCtrlID();
	}
	STDMETHOD_(DWORD, GetStyle)(THIS) SCONST OVERRIDE
	{
		return SHostWnd::GetStyle();
	}
	STDMETHOD_(DWORD, GetExStyle)(THIS) SCONST OVERRIDE
	{
		return SHostWnd::GetExStyle();
	}
	STDMETHOD_(LONG_PTR, GetWindowLongPtr)(THIS_ int nIndex) SCONST OVERRIDE
	{
		return SHostWnd::GetWindowLongPtr(nIndex);
	}
	STDMETHOD_(LONG_PTR, SetWindowLongPtr)(THIS_ int nIndex, LONG_PTR dwNewLong) OVERRIDE
	{
		return SHostWnd::SetWindowLongPtr(nIndex, dwNewLong);
	}
	STDMETHOD_(HWND, GetParent)(THIS) OVERRIDE
	{
		return SHostWnd::GetParent();
	}
	STDMETHOD_(HWND, SetParent)(THIS_ HWND hWndNewParent) OVERRIDE
	{
		return SHostWnd::SetParent(hWndNewParent);
	}
	STDMETHOD_(BOOL, IsWindowEnabled)(THIS) SCONST OVERRIDE
	{
		return SHostWnd::IsWindowEnabled();
	}
	STDMETHOD_(BOOL, ModifyStyle)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0) OVERRIDE
	{
		return SHostWnd::ModifyStyle(dwRemove, dwAdd, nFlags);
	}
	STDMETHOD_(BOOL, ModifyStyleEx)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0) OVERRIDE
	{
		return SHostWnd::ModifyStyleEx(dwRemove, dwAdd, nFlags);
	}
	STDMETHOD_(BOOL, SetWindowPos)
		(THIS_ HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags) OVERRIDE
	{
		return SHostWnd::SetWindowPos(hWndInsertAfter, x, y, cx, cy, nFlags);
	}
	STDMETHOD_(BOOL, CenterWindow)(THIS_ HWND hWndCenter = NULL) OVERRIDE
	{
		return SHostWnd::CenterWindow(hWndCenter);
	}
	STDMETHOD_(BOOL, DestroyWindow)(THIS) OVERRIDE
	{
		return SHostWnd::DestroyWindow();
	}
	STDMETHOD_(BOOL, IsWindow)(CTHIS) SCONST OVERRIDE
	{
		return SHostWnd::IsWindow();
	}
	STDMETHOD_(BOOL, Invalidate)(THIS_ BOOL bErase = TRUE) OVERRIDE
	{
		return SHostWnd::Invalidate(bErase);
	}
	STDMETHOD_(BOOL, InvalidateRect)(THIS_ LPCRECT lpRect, BOOL bErase = TRUE) OVERRIDE
	{
		return SHostWnd::InvalidateRect(lpRect, bErase);
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
		return SHostWnd::ClientToScreen(lpPoint);
	}
	STDMETHOD_(BOOL, ClientToScreen2)(THIS_ LPRECT lpRect) SCONST OVERRIDE
	{
		return SHostWnd::ClientToScreen2(lpRect);
	}
	STDMETHOD_(BOOL, ScreenToClient)(THIS_ LPPOINT lpPoint) SCONST OVERRIDE
	{
		return SHostWnd::ScreenToClient(lpPoint);
	}
	STDMETHOD_(BOOL, ScreenToClient2)(THIS_ LPRECT lpRect) SCONST OVERRIDE
	{
		return SHostWnd::ScreenToClient2(lpRect);
	}
	STDMETHOD_(int, MapWindowPoints)
		(THIS_ HWND hWndTo, LPPOINT lpPoint, UINT nCount) SCONST OVERRIDE
	{
		return SHostWnd::MapWindowPoints(hWndTo, lpPoint, nCount);
	}
	STDMETHOD_(int, MapWindowRect)(THIS_ HWND hWndTo, LPRECT lpRect) SCONST OVERRIDE
	{
		return SHostWnd::MapWindowRect(hWndTo, lpRect);
	}
	STDMETHOD_(UINT_PTR, SetTimer)
		(THIS_ UINT_PTR nIDEvent,
		UINT nElapse,
		void(CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD) = NULL) OVERRIDE
	{
		return SHostWnd::SetTimer(nIDEvent, nElapse, lpfnTimer);
	}
	STDMETHOD_(BOOL, KillTimer)(THIS_ UINT_PTR nIDEvent) OVERRIDE
	{
		return SHostWnd::KillTimer(nIDEvent);
	}
	STDMETHOD_(HDC, GetDC)(THIS) OVERRIDE
	{
		return SHostWnd::GetDC();
	}
	STDMETHOD_(HDC, GetWindowDC)(THIS) OVERRIDE
	{
		return SHostWnd::GetWindowDC();
	}
	STDMETHOD_(int, ReleaseDC)(THIS_ HDC hDC) OVERRIDE
	{
		return SHostWnd::ReleaseDC(hDC);
	}
    STDMETHOD_(BOOL, CreateCaret)(THIS_ HBITMAP hBitmap, int nWidth, int nHeight) OVERRIDE
	{
		return SHostWnd::CreateCaret(hBitmap,nWidth,nHeight);
	}
	STDMETHOD_(BOOL, HideCaret)(THIS) OVERRIDE
	{
		return SHostWnd::HideCaret();
	}
	STDMETHOD_(BOOL, ShowCaret)(THIS) OVERRIDE
	{
		return SHostWnd::ShowCaret();
	}
	STDMETHOD_(HWND, SetCapture)(THIS) OVERRIDE
	{
		return SHostWnd::SetCapture();
	}

	STDMETHOD_(HWND, GetCapture)(THIS) OVERRIDE
	{
		return SHostWnd::GetCapture();
	}

	STDMETHOD_(BOOL, ReleaseCapture)(THIS) OVERRIDE
	{
		return SHostWnd::ReleaseCapture();
	}

	STDMETHOD_(HWND, SetFocus)(THIS) OVERRIDE
	{
		return SHostWnd::SetFocus();
	}
	STDMETHOD_(LRESULT, SendMessage)
		(THIS_ UINT message, WPARAM wParam = 0, LPARAM lParam = 0) OVERRIDE
	{
		return SHostWnd::SendMessage(message, wParam, lParam);
	}
	STDMETHOD_(BOOL, PostMessage)(THIS_ UINT message, WPARAM wParam = 0, LPARAM lParam = 0) OVERRIDE
	{
		return SHostWnd::PostMessage(message, wParam, lParam);
	}
	STDMETHOD_(BOOL, SendNotifyMessage)
		(THIS_ UINT message, WPARAM wParam = 0, LPARAM lParam = 0) OVERRIDE
	{
		return SHostWnd::SendNotifyMessage(message, wParam, lParam);
	}
	STDMETHOD_(BOOL, SetWindowText)(THIS_ LPCTSTR lpszString) OVERRIDE
	{
		return SHostWnd::SetWindowText(lpszString);
	}
	STDMETHOD_(int, GetWindowText)(THIS_ LPTSTR lpszStringBuf, int nMaxCount) SCONST OVERRIDE
	{
		return SHostWnd::GetWindowText(lpszStringBuf, nMaxCount);
	}
	STDMETHOD_(BOOL, IsIconic)(THIS) SCONST OVERRIDE
	{
		return SHostWnd::IsIconic();
	}
	STDMETHOD_(BOOL, IsZoomed)(THIS) SCONST OVERRIDE
	{
		return SHostWnd::IsZoomed();
	}
	STDMETHOD_(BOOL, IsWindowVisible)(THIS) SCONST OVERRIDE
	{
		return SHostWnd::IsWindowVisible();
	}
	STDMETHOD_(BOOL, MoveWindow)
		(THIS_ int x, int y, int nWidth, int nHeight, BOOL bRepaint = TRUE) OVERRIDE
	{
		return SHostWnd::MoveWindow(x, y, nWidth, nHeight, bRepaint);
	}
	STDMETHOD_(BOOL, MoveWindow2)(THIS_ LPCRECT lpRect, BOOL bRepaint = TRUE) OVERRIDE
	{
		return SHostWnd::MoveWindow2(lpRect, bRepaint);
	}
	STDMETHOD_(BOOL, ShowWindow)(THIS_ int nCmdShow) OVERRIDE
	{
		return SHostWnd::ShowWindow(nCmdShow);
	}
	STDMETHOD_(int, SetWindowRgn)(THIS_ HRGN hRgn, BOOL bRedraw = TRUE) OVERRIDE
	{
		return SHostWnd::SetWindowRgn(hRgn, bRedraw);
	}
	STDMETHOD_(BOOL, SetLayeredWindowAttributes)
		(THIS_ COLORREF crKey, BYTE byAlpha, DWORD dwFlags) OVERRIDE
	{
		return SHostWnd::SetLayeredWindowAttributes(crKey, byAlpha, dwFlags);
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
		return SHostWnd::UpdateLayeredWindow(hdcDst, pptDst, psize, hdcSrc, pptSrc, crKey, pblend,
			dwFlags);
	}

	STDMETHOD_(void, SetMsgHandler)(THIS_ FunMsgHandler fun, void *ctx) OVERRIDE
	{
		return SHostWnd::SetMsgHandler(fun, ctx);
	}
	STDMETHOD_(MsgHandlerInfo *,GetMsgHandler)(THIS) OVERRIDE{
		return SHostWnd::GetMsgHandler();
	}

	STDMETHOD_(HWND, CreateEx)
    (THIS_ HWND hWndParent, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, IXmlNode *xmlInit DEF_VAL(NULL))
	{
        return SHostWnd::CreateEx(hWndParent, dwStyle, dwExStyle, x, y, nWidth, nHeight, xmlInit);
	}
	STDMETHOD_(HWND, Create)
		(THIS_ HWND hWndParent, int x = 0, int y = 0, int nWidth = 0, int nHeight = 0)
	{
		return SHostWnd::Create(hWndParent, x, y, nWidth, nHeight);
	}

	STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode *pNode) OVERRIDE{
		return SHostWnd::InitFromXml(pNode);
	}

	STDMETHOD_(void, SetLayoutId)(THIS_ LPCTSTR pszLayoutId) OVERRIDE
	{
		SHostWnd::SetLayoutId(pszLayoutId);
	}

	STDMETHOD_(IWindow *, GetIRoot)(THIS) OVERRIDE
	{
		return SHostWnd::GetIRoot();
	}

	STDMETHOD_(BOOL,IsTranslucent)(CTHIS) SCONST OVERRIDE{
		return SHostWnd::IsTranslucent();
	}
	STDMETHOD_(IHostPresenter*,GetPresenter)(THIS) OVERRIDE{
		return SHostWnd::GetPresenter();
	}

	STDMETHOD_(void,SetPresenter)(THIS_ IHostPresenter* pPresenter) OVERRIDE{
		return SHostWnd::SetPresenter(pPresenter);
	}

	STDMETHOD_(IMessageLoop*,GetMsgLoop)(THIS) OVERRIDE{
		return SHostWnd::GetMsgLoop();
	}

	STDMETHOD_(IWindow *, FindIChildByID)(THIS_ int nId) OVERRIDE{
		return SHostWnd::FindIChildByID(nId);
	}

	STDMETHOD_(IWindow *, FindIChildByName)(THIS_ LPCWSTR pszName) OVERRIDE{
		return SHostWnd::FindIChildByName(pszName);
	}

	STDMETHOD_(IWindow *, FindIChildByNameA)(THIS_ LPCSTR pszName) OVERRIDE{
		return SHostWnd::FindIChildByNameA(pszName);
	}

	STDMETHOD_(INcPainter*,GetNcPainter)(THIS) OVERRIDE
	{
		return SHostWnd::GetNcPainter();
	}

	STDMETHOD_(void, SetEventHandler)(THIS_ FunCallback fun, void *ctx) OVERRIDE{
		return SHostWnd::SetEventHandler(fun,ctx);
	}

	STDMETHOD_(EventHandlerInfo*, GetEventHandler)(THIS) OVERRIDE{
		return SHostWnd::GetEventHandler();
	}

	STDMETHOD_(BOOL, AnimateHostWindow)(THIS_ DWORD dwTime, DWORD dwFlags) OVERRIDE{
		return SHostWnd::AnimateHostWindow(dwTime,dwFlags);
	}

	STDMETHOD_(void,EnableDragDrop)(THIS) OVERRIDE{
		return SHostWnd::EnableDragDrop();
	}
	STDMETHOD_(void,ShowHostWnd)(THIS_ int uShowCmd,BOOL bWaitAniDone) OVERRIDE{
		return SHostWnd::ShowHostWnd(uShowCmd,bWaitAniDone);
	}
	STDMETHOD_(void, EnablePrivateUiDef)(THIS_ BOOL bEnable) OVERRIDE{
		return SHostWnd::EnablePrivateUiDef(bEnable);
	}
	STDMETHOD_(void, SetScale)(THIS_ int nScale,LPCRECT pDestRect){
		return SHostWnd::SetScale(nScale,pDestRect);
	}

};

SNSEND
#endif // __SHOSTWNDPROXY__H__