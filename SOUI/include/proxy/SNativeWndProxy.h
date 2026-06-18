#ifndef __NativeWndImlPROXY__H__
#define __NativeWndImlPROXY__H__

SNSBEGIN

template <class T,class NativeWndIml = SNativeWnd>
class TNativeWndProxy
	: public T
	, public NativeWndIml {
public:
	TNativeWndProxy() {

	}
	template<typename P>
	TNativeWndProxy(P p1):NativeWndIml(p1){
	}

	STDMETHOD_(long, AddRef)(THIS)
	{
		return NativeWndIml::AddRef();
	}
	STDMETHOD_(long, Release)(THIS)
	{
		return NativeWndIml::Release();
	}
	STDMETHOD_(void, OnFinalRelease)(THIS)
	{
		NativeWndIml::OnFinalRelease();
	}
	
	STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode * pXmlNode) OVERRIDE {
		return NativeWndIml::InitFromXml(pXmlNode);
	}

	/**
	 * @brief Retrieves the object's name.
	 * @return Object's name as a wide string.
	 */
	STDMETHOD_(LPCWSTR, GetName)(THIS) SCONST OVERRIDE {
		return NativeWndIml::GetName();
	}

	/**
	 * @brief Retrieves the object's name in ANSI format.
	 * @return Object's name as an ANSI string.
	 */
	STDMETHOD_(LPCSTR, GetNameA)(THIS) SCONST OVERRIDE {
		return NativeWndIml::GetNameA();
	}

	/**
	 * @brief Sets the object's name.
	 * @param pszName Name to set.
	 */
	STDMETHOD_(void, SetName)(THIS_ LPCWSTR pszName) OVERRIDE {
		NativeWndIml::SetName(pszName);
	}

	/**
	 * @brief Retrieves the object's ID.
	 * @return Object's ID.
	 */
	STDMETHOD_(int, GetID)(THIS) SCONST OVERRIDE {
		return NativeWndIml::GetID();
	}

	/**
	 * @brief Sets the object's ID.
	 * @param nID ID to set.
	 */
	STDMETHOD_(void, SetID)(THIS_ int nID) OVERRIDE {
		NativeWndIml::SetID(nID);
	}

	/**
	 * @brief Handles attribute processing after setting an attribute.
	 * @param strAttribName Attribute name.
	 * @param strValue Attribute value.
	 * @param bLoading TRUE if the object is being loaded, FALSE otherwise.
	 * @param hr Result of the attribute setting.
	 * @return Result of the attribute processing.
	 */
	STDMETHOD_(HRESULT, AfterAttribute)(THIS_ LPCWSTR strAttribName, LPCWSTR strValue, BOOL bLoading, HRESULT hr) OVERRIDE {
		return NativeWndIml::AfterAttribute(strAttribName, strValue, bLoading, hr);
	}

	/**
	 * @brief Sets an attribute using ANSI strings.
	 * @param pszAttr Attribute name.
	 * @param pszValue Attribute value.
	 * @param bLoading TRUE if the object is being loaded, FALSE otherwise.
	 * @return Result of setting the attribute.
	 */
	STDMETHOD_(HRESULT, SetAttributeA)(THIS_ LPCSTR pszAttr, LPCSTR pszValue, BOOL bLoading) OVERRIDE {
		return NativeWndIml::SetAttributeA(pszAttr,pszValue, bLoading);
	}

	/**
	 * @brief Sets an attribute using wide strings.
	 * @param pszAttr Attribute name.
	 * @param pszValue Attribute value.
	 * @param bLoading TRUE if the object is being loaded, FALSE otherwise.
	 * @return Result of setting the attribute.
	 */
	STDMETHOD_(HRESULT, SetAttribute)(THIS_ LPCWSTR pszAttr, LPCWSTR pszValue, BOOL bLoading) OVERRIDE {
		return NativeWndIml::SetAttribute(pszAttr, pszValue, bLoading);
	}

	/**
	 * @brief Sets an attribute using IStringW objects.
	 * @param strAttr Attribute name.
	 * @param strValue Attribute value.
	 * @param bLoading TRUE if the object is being loaded, FALSE otherwise.
	 * @return Result of setting the attribute.
	 */
	STDMETHOD_(HRESULT, ISetAttribute)(THIS_ const IStringW* strAttr, const IStringW* strValue, BOOL bLoading) OVERRIDE {
		return NativeWndIml::ISetAttribute(strAttr, strValue, bLoading);
	}

	/**
	 * @brief Retrieves the object's class name.
	 * @return Class name as a wide string.
	 */
	/*STDMETHOD_(LPCWSTR, GetObjectClass)(THIS_) SCONST OVERRIDE {
		return NativeWndIml::GetOjbectClass();
	}*/

	/**
	 * @brief Retrieves the object's type.
	 * @return Object type identifier.
	 */
	STDMETHOD_(int, GetObjectType)(THIS) SCONST OVERRIDE {
		return NativeWndIml::GetObjectType();
	}

	/**
	 * @brief Checks if the object is of a specific class.
	 * @param lpszName Class name to check.
	 * @return TRUE if the object is of the specified class, FALSE otherwise.
	 */
	STDMETHOD_(BOOL, IsClass)(THIS_ LPCWSTR lpszName) SCONST OVERRIDE {
		return NativeWndIml::IsClass(lpszName);
	}

	/**
	 * @brief Retrieves the class name list of the object.
	 *
	 * @param ppClassName Pointer to a buffer that receives the class name list.
	 * @param nMaxCount Maximum number of strings to copy to the buffer.
	 * @return int The number of strings copied to the buffer.
	 */
	STDMETHOD_(int, GetClassNameList)(CTHIS_ wchar_t ppClassNameList[][50], int nMaxCount) SCONST {
		return NativeWndIml::GetClassNameList(ppClassNameList, nMaxCount);
	}

	/**
	 * @brief Retrieves an attribute value.
	 * @param strAttr Attribute name.
	 * @param pValue Pointer to store the attribute value.
	 * @return TRUE if the attribute is retrieved successfully, FALSE otherwise.
	 */
	STDMETHOD_(BOOL, GetAttribute)(THIS_ LPCWSTR strAttr, IStringW* pValue) SCONST OVERRIDE {
		return NativeWndIml::GetAttribute(strAttr, pValue);
	}

	/**
	 * @brief Handles initialization completion.
	 * @param xmlNode XML node containing the object's attributes.
	 */
	STDMETHOD_(void, OnInitFinished)(THIS_ IXmlNode* xmlNode) OVERRIDE {
		return NativeWndIml::OnInitFinished(xmlNode);
	}

	/**
	 * @brief Sets the attribute handler.
	 * @param attrHandler Function pointer to the attribute handler.
	 */
	STDMETHOD_(void, SetAttrHandler)(THIS_ FunAttrHandler attrHandler) OVERRIDE {
		return NativeWndIml::SetAttrHandler(attrHandler);
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
		return NativeWndIml::CreateNative(lpWindowName, dwStyle, dwExStyle, x, y, nWidth, nHeight,
			hWndParent,nID, lpParam);
	}

	STDMETHOD_(HWND, GetHwnd)(THIS) OVERRIDE
	{
		return NativeWndIml::GetHwnd();
	}

	STDMETHOD_(BOOL, SubclassWindow)(THIS_ HWND hWnd) OVERRIDE
	{
		return NativeWndIml::SubclassWindow(hWnd);
	}
	STDMETHOD_(HWND, UnsubclassWindow)(THIS_ BOOL bForce /*= FALSE*/) OVERRIDE
	{
		return NativeWndIml::UnsubclassWindow(bForce);
	}
	STDMETHOD_(const MSG *, GetCurrentMessage)(THIS) SCONST OVERRIDE
	{
		return NativeWndIml::GetCurrentMessage();
	}
	STDMETHOD_(int, GetDlgCtrlID)(THIS) SCONST OVERRIDE
	{
		return NativeWndIml::GetDlgCtrlID();
	}
	STDMETHOD_(DWORD, GetStyle)(THIS) SCONST OVERRIDE
	{
		return NativeWndIml::GetStyle();
	}
	STDMETHOD_(DWORD, GetExStyle)(THIS) SCONST OVERRIDE
	{
		return NativeWndIml::GetExStyle();
	}
	STDMETHOD_(LONG_PTR, GetWindowLongPtr)(THIS_ int nIndex) SCONST OVERRIDE
	{
		return NativeWndIml::GetWindowLongPtr(nIndex);
	}
	STDMETHOD_(LONG_PTR, SetWindowLongPtr)(THIS_ int nIndex, LONG_PTR dwNewLong) OVERRIDE
	{
		return NativeWndIml::SetWindowLongPtr(nIndex, dwNewLong);
	}
	STDMETHOD_(HWND, GetParent)(THIS) OVERRIDE
	{
		return NativeWndIml::GetParent();
	}
	STDMETHOD_(HWND, SetParent)(THIS_ HWND hWndNewParent) OVERRIDE
	{
		return NativeWndIml::SetParent(hWndNewParent);
	}
	STDMETHOD_(BOOL, IsWindowEnabled)(THIS) SCONST OVERRIDE
	{
		return NativeWndIml::IsWindowEnabled();
	}
	STDMETHOD_(BOOL, ModifyStyle)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0) OVERRIDE
	{
		return NativeWndIml::ModifyStyle(dwRemove, dwAdd, nFlags);
	}
	STDMETHOD_(BOOL, ModifyStyleEx)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0) OVERRIDE
	{
		return NativeWndIml::ModifyStyleEx(dwRemove, dwAdd, nFlags);
	}
	STDMETHOD_(BOOL, SetWindowPos)
		(THIS_ HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags) OVERRIDE
	{
		return NativeWndIml::SetWindowPos(hWndInsertAfter, x, y, cx, cy, nFlags);
	}
	STDMETHOD_(BOOL, CenterWindow)(THIS_ HWND hWndCenter DEF_VAL(0)) OVERRIDE
	{
		return NativeWndIml::CenterWindow(hWndCenter);
	}
	STDMETHOD_(BOOL, IsWindow)(CTHIS) SCONST OVERRIDE
	{
		return NativeWndIml::IsWindow();
	}
	STDMETHOD_(BOOL, Invalidate)(THIS_ BOOL bErase = TRUE) OVERRIDE
	{
		return NativeWndIml::Invalidate(bErase);
	}
	STDMETHOD_(BOOL, InvalidateRect)(THIS_ LPCRECT lpRect, BOOL bErase = TRUE) OVERRIDE
	{
		return NativeWndIml::InvalidateRect(lpRect, bErase);
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
		return NativeWndIml::ClientToScreen(lpPoint);
	}
	STDMETHOD_(BOOL, ClientToScreen2)(THIS_ LPRECT lpRect) SCONST OVERRIDE
	{
		return NativeWndIml::ClientToScreen2(lpRect);
	}
	STDMETHOD_(BOOL, ScreenToClient)(THIS_ LPPOINT lpPoint) SCONST OVERRIDE
	{
		return NativeWndIml::ScreenToClient(lpPoint);
	}
	STDMETHOD_(BOOL, ScreenToClient2)(THIS_ LPRECT lpRect) SCONST OVERRIDE
	{
		return NativeWndIml::ScreenToClient2(lpRect);
	}
	STDMETHOD_(int, MapWindowPoints)
		(THIS_ HWND hWndTo, LPPOINT lpPoint, UINT nCount) SCONST OVERRIDE
	{
		return NativeWndIml::MapWindowPoints(hWndTo, lpPoint, nCount);
	}
	STDMETHOD_(int, MapWindowRect)(THIS_ HWND hWndTo, LPRECT lpRect) SCONST OVERRIDE
	{
		return NativeWndIml::MapWindowRect(hWndTo, lpRect);
	}
	STDMETHOD_(UINT_PTR, SetTimer)
		(THIS_ UINT_PTR nIDEvent,
		UINT nElapse,
		void(CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD) = NULL) OVERRIDE
	{
		return NativeWndIml::SetTimer(nIDEvent, nElapse, lpfnTimer);
	}
	STDMETHOD_(BOOL, KillTimer)(THIS_ UINT_PTR nIDEvent) OVERRIDE
	{
		return NativeWndIml::KillTimer(nIDEvent);
	}
	STDMETHOD_(HDC, GetDC)(THIS) OVERRIDE
	{
		return NativeWndIml::GetDC();
	}
	STDMETHOD_(HDC, GetWindowDC)(THIS) OVERRIDE
	{
		return NativeWndIml::GetWindowDC();
	}
	STDMETHOD_(int, ReleaseDC)(THIS_ HDC hDC) OVERRIDE
	{
		return NativeWndIml::ReleaseDC(hDC);
	}
    STDMETHOD_(BOOL, CreateCaret)(THIS_ HBITMAP hBitmap, int nWidth, int nHeight) OVERRIDE
	{
        return NativeWndIml::CreateCaret(hBitmap, nWidth, nHeight);
	}
	STDMETHOD_(BOOL, HideCaret)(THIS) OVERRIDE
	{
		return NativeWndIml::HideCaret();
	}
	STDMETHOD_(BOOL, ShowCaret)(THIS) OVERRIDE
	{
		return NativeWndIml::ShowCaret();
	}
	STDMETHOD_(HWND, SetCapture)(THIS) OVERRIDE
	{
		return NativeWndIml::SetCapture();
	}

	STDMETHOD_(HWND, GetCapture)(THIS) OVERRIDE
	{
		return NativeWndIml::GetCapture();
	}

	STDMETHOD_(BOOL, ReleaseCapture)(THIS) OVERRIDE
	{
		return NativeWndIml::ReleaseCapture();
	}

	STDMETHOD_(HWND, SetFocus)(THIS) OVERRIDE
	{
		return NativeWndIml::SetFocus();
	}
	STDMETHOD_(LRESULT, SendMessage)
		(THIS_ UINT message, WPARAM wParam = 0, LPARAM lParam = 0) OVERRIDE
	{
		return NativeWndIml::SendMessage(message, wParam, lParam);
	}
	STDMETHOD_(BOOL, PostMessage)(THIS_ UINT message, WPARAM wParam = 0, LPARAM lParam = 0) OVERRIDE
	{
		return NativeWndIml::PostMessage(message, wParam, lParam);
	}
	STDMETHOD_(BOOL, SendNotifyMessage)
		(THIS_ UINT message, WPARAM wParam = 0, LPARAM lParam = 0) OVERRIDE
	{
		return NativeWndIml::SendNotifyMessage(message, wParam, lParam);
	}
	STDMETHOD_(BOOL, SetWindowText)(THIS_ LPCTSTR lpszString) OVERRIDE
	{
		return NativeWndIml::SetWindowText(lpszString);
	}
	STDMETHOD_(int, GetWindowText)(THIS_ LPTSTR lpszStringBuf, int nMaxCount) SCONST OVERRIDE
	{
		return NativeWndIml::GetWindowText(lpszStringBuf, nMaxCount);
	}
	STDMETHOD_(BOOL, IsIconic)(THIS) SCONST OVERRIDE
	{
		return NativeWndIml::IsIconic();
	}
	STDMETHOD_(BOOL, IsZoomed)(THIS) SCONST OVERRIDE
	{
		return NativeWndIml::IsZoomed();
	}
	STDMETHOD_(BOOL, IsWindowVisible)(THIS) SCONST OVERRIDE
	{
		return NativeWndIml::IsWindowVisible();
	}
	STDMETHOD_(BOOL, MoveWindow)
		(THIS_ int x, int y, int nWidth, int nHeight, BOOL bRepaint = TRUE) OVERRIDE
	{
		return NativeWndIml::MoveWindow(x, y, nWidth, nHeight, bRepaint);
	}
	STDMETHOD_(BOOL, MoveWindow2)(THIS_ LPCRECT lpRect, BOOL bRepaint = TRUE) OVERRIDE
	{
		return NativeWndIml::MoveWindow2(lpRect, bRepaint);
	}
	STDMETHOD_(BOOL, ShowWindow)(THIS_ int nCmdShow) OVERRIDE
	{
		return NativeWndIml::ShowWindow(nCmdShow);
	}

	STDMETHOD_(BOOL, UpdateWindow)(THIS) OVERRIDE
	{
		return NativeWndIml::UpdateWindow();
    }

	STDMETHOD_(int, SetWindowRgn)(THIS_ HRGN hRgn, BOOL bRedraw = TRUE) OVERRIDE
	{
		return NativeWndIml::SetWindowRgn(hRgn, bRedraw);
	}
	STDMETHOD_(BOOL, SetLayeredWindowAttributes)
		(THIS_ COLORREF crKey, BYTE byAlpha, DWORD dwFlags) OVERRIDE
	{
		return NativeWndIml::SetLayeredWindowAttributes(crKey, byAlpha, dwFlags);
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
		return NativeWndIml::UpdateLayeredWindow(hdcDst, pptDst, psize, hdcSrc, pptSrc, crKey, pblend,
			dwFlags);
	}

	STDMETHOD_(void, SetMsgHandler)(THIS_ FunMsgHandler fun, void *ctx) OVERRIDE
	{
		return NativeWndIml::SetMsgHandler(fun, ctx);
	}

	STDMETHOD_(MsgHandlerInfo *, GetMsgHandler)(THIS) OVERRIDE{
		return NativeWndIml::GetMsgHandler();
	}

};

SNSEND
#endif // __NativeWndImlPROXY__H__