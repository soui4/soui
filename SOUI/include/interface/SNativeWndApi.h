//! Add reference
/*!
 * @brief Increments the reference count of the object.
 *
 * @return long The new reference count.
 */
STDMETHOD_(long, AddRef)(THIS) PURE;

//! Release reference
/*!
 * @brief Decrements the reference count of the object.
 *
 * @return long The new reference count.
 */
STDMETHOD_(long, Release)(THIS) PURE;

//! Final release of the object
/*!
 * @brief Final release of the object, performing cleanup if necessary.
 */
STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

//////////////////////////////////////////////////////////////////////////

/**
 * @brief Creates a Native window.
 *
 * @param lpWindowName Name of the window.
 * @param dwStyle Window style.
 * @param dwExStyle Extended window style.
 * @param x X position of the window.
 * @param y Y position of the window.
 * @param nWidth Width of the window.
 * @param nHeight Height of the window.
 * @param hWndParent Handle to the parent window.
 * @param nID Control ID of the window.
 * @param lpParam Additional parameters.
 * @return HWND Handle to the created window.
 */
STDMETHOD_(HWND, CreateNative)
(THIS_ LPCTSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, int nID, LPVOID lpParam) PURE;

/**
 * @brief Retrieves the handle to the window.
 *
 * @return HWND Handle to the window.
 */
STDMETHOD_(HWND, GetHwnd)(THIS) PURE;

/**
 * @brief Subclasses an existing window with the SNativeWnd object.
 *
 * @param hWnd Handle to the window to subclass.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, SubclassWindow)(THIS_ HWND hWnd) PURE;

/**
 * @brief Unsubclasses the window.
 *
 * @param bForce If TRUE, forces the unsubclassing even if the current window procedure is not SNativeWnd's.
 * @return HWND Handle to the unsubclassed window.
 */
STDMETHOD_(HWND, UnsubclassWindow)(THIS_ BOOL bForce /*= FALSE*/) PURE;

/**
 * @brief Retrieves the current window message object.
 *
 * @return const MSG* Pointer to the current message object.
 */
STDMETHOD_(const MSG *, GetCurrentMessage)(CTHIS) SCONST PURE;

/**
 * @brief Retrieves the window ID.
 *
 * @return int Window ID.
 */
STDMETHOD_(int, GetDlgCtrlID)(CTHIS) SCONST PURE;

/**
 * @brief Retrieves the window style.
 *
 * @return DWORD Window style.
 */
STDMETHOD_(DWORD, GetStyle)(CTHIS) SCONST PURE;

/**
 * @brief Retrieves the extended window style.
 *
 * @return DWORD Extended window style.
 */
STDMETHOD_(DWORD, GetExStyle)(CTHIS) SCONST PURE;

/**
 * @brief Retrieves the long pointer value at the specified index.
 *
 * @param nIndex Index of the value to retrieve.
 * @return LONG_PTR Long pointer value.
 */
STDMETHOD_(LONG_PTR, GetWindowLongPtr)(CTHIS_ int nIndex) SCONST PURE;

/**
 * @brief Sets the long pointer value at the specified index.
 *
 * @param nIndex Index of the value to set.
 * @param dwNewLong New long pointer value.
 * @return LONG_PTR Previous long pointer value.
 */
STDMETHOD_(LONG_PTR, SetWindowLongPtr)(THIS_ int nIndex, LONG_PTR dwNewLong) PURE;

/**
 * @brief Retrieves the handle to the parent window.
 *
 * @return HWND Handle to the parent window.
 */
STDMETHOD_(HWND, GetParent)(THIS) PURE;

/**
 * @brief Sets the handle to the parent window.
 *
 * @param hWndNewParent Handle to the new parent window.
 * @return HWND Handle to the previous parent window.
 */
STDMETHOD_(HWND, SetParent)(THIS_ HWND hWndNewParent) PURE;

/**
 * @brief Checks if the window is enabled.
 *
 * @return BOOL TRUE if the window is enabled, FALSE otherwise.
 */
STDMETHOD_(BOOL, IsWindowEnabled)(CTHIS) SCONST PURE;

/**
 * @brief Modifies the window style.
 *
 * @param dwRemove Styles to remove.
 * @param dwAdd Styles to add.
 * @param nFlags Window positioning flags.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, ModifyStyle)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags /*=0*/) PURE;

/**
 * @brief Modifies the extended window style.
 *
 * @param dwRemove Styles to remove.
 * @param dwAdd Styles to add.
 * @param nFlags Window positioning flags.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, ModifyStyleEx)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags /*=0*/) PURE;

/**
 * @brief Sets the window position.
 *
 * @param hWndInsertAfter Handle to the window to insert after.
 * @param x X position of the window.
 * @param y Y position of the window.
 * @param cx Width of the window.
 * @param cy Height of the window.
 * @param nFlags Window positioning flags.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, SetWindowPos)
(THIS_ HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags) PURE;

/**
 * @brief Centers the window relative to another window.
 *
 * @param hWndCenter Handle to the window to center relative to.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, CenterWindow)(THIS_ HWND hWndCenter /*= NULL*/) PURE;

/**
 * @brief Destroys the window.
 *
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, DestroyWindow)(THIS) PURE;

/**
 * @brief Checks if the handle is a valid window handle.
 *
 * @return BOOL TRUE if valid, FALSE otherwise.
 */
STDMETHOD_(BOOL, IsWindow)(CTHIS) SCONST PURE;

/**
 * @brief Invalidates the entire window.
 *
 * @param bErase Flag indicating if the background should be erased.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, Invalidate)(THIS_ BOOL bErase /*= TRUE*/) PURE;

/**
 * @brief Invalidates a specified rectangle of the window.
 *
 * @param lpRect Rectangle to invalidate.
 * @param bErase Flag indicating if the background should be erased.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, InvalidateRect)(THIS_ LPCRECT lpRect, BOOL bErase /* = TRUE*/) PURE;

/**
 * @brief Retrieves the window rectangle.
 *
 * @param[out] lpRect Rectangle to receive the window rectangle.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, GetWindowRect)(CTHIS_ LPRECT lpRect) SCONST PURE;

/**
 * @brief Retrieves the client rectangle.
 *
 * @param[out] lpRect Rectangle to receive the client rectangle.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, GetClientRect)(CTHIS_ LPRECT lpRect) SCONST PURE;

/**
 * @brief Converts client coordinates to screen coordinates.
 *
 * @param[in][out] lpPoint Point to convert.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, ClientToScreen)(CTHIS_ LPPOINT lpPoint) SCONST PURE;

/**
 * @brief Converts client rectangle coordinates to screen coordinates.
 *
 * @param[in][out] lpRect Rectangle to convert.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, ClientToScreen2)(CTHIS_ LPRECT lpRect) SCONST PURE;

/**
 * @brief Converts screen coordinates to client coordinates.
 *
 * @param[in][out] lpPoint Point to convert.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, ScreenToClient)(CTHIS_ LPPOINT lpPoint) SCONST PURE;

/**
 * @brief Converts screen rectangle coordinates to client coordinates.
 *
 * @param[in][out] lpRect Rectangle to convert.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, ScreenToClient2)(CTHIS_ LPRECT lpRect) SCONST PURE;

/**
 * @brief Maps a set of points from one window to another.
 *
 * @param hWndTo Handle to the destination window.
 * @param[in][out] lpPoint Array of points to map.
 * @param nCount Number of points.
 * @return int Number of points mapped.
 */
STDMETHOD_(int, MapWindowPoints)(CTHIS_ HWND hWndTo, LPPOINT lpPoint, UINT nCount) SCONST PURE;

/**
 * @brief Maps a rectangle from one window to another.
 *
 * @param hWndTo Handle to the destination window.
 * @param[in][out] lpRect Rectangle to map.
 * @return int Number of points mapped.
 */
STDMETHOD_(int, MapWindowRect)(CTHIS_ HWND hWndTo, LPRECT lpRect) SCONST PURE;

/**
 * @brief Sets a timer for the window.
 *
 * @param nIDEvent Timer ID.
 * @param nElapse Elapse time in milliseconds.
 * @param lpfnTimer Timer callback function, if NULL, a WM_TIMER message is generated.
 * @return UINT_PTR Timer ID.
 */
STDMETHOD_(UINT_PTR, SetTimer)
(THIS_ UINT_PTR nIDEvent, UINT nElapse, void(CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD) /*= NULL*/) PURE;

/**
 * @brief Kills a timer for the window.
 *
 * @param nIDEvent Timer ID.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, KillTimer)(THIS_ UINT_PTR nIDEvent) PURE;

/**
 * @brief Retrieves a device context for the window.
 *
 * @return HDC Handle to the device context.
 */
STDMETHOD_(HDC, GetDC)(THIS) PURE;

/**
 * @brief Retrieves a device context for the entire window.
 *
 * @return HDC Handle to the device context.
 */
STDMETHOD_(HDC, GetWindowDC)(THIS) PURE;

/**
 * @brief Releases a device context.
 *
 * @param hDC Handle to the device context to release.
 * @return int Number of times the device context was released.
 */
STDMETHOD_(int, ReleaseDC)(THIS_ HDC hDC) PURE;

/**
 * @brief Creates a caret for the window.
 *
 * @param hBitmap Handle to the bitmap for the caret.
 * @param nWidth Width of the caret.
 * @param nHeight Height of the caret.
 * * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, CreateCaret)(THIS_ HBITMAP hBitmap, int nWidth, int nHeight) PURE;

/**
 * @brief Hides the caret.
 *
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, HideCaret)(THIS) PURE;

/**
 * @brief Shows the caret.
 *
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, ShowCaret)(THIS) PURE;

/**
 * @brief Retrieves the handle to the window that has captured the mouse.
 *
 * @return HWND Handle to the capturing window.
 */
STDMETHOD_(HWND, GetCapture)(THIS) PURE;

/**
 * @brief Sets the window to capture the mouse input.
 *
 * @return HWND Handle to the previous capturing window.
 */
STDMETHOD_(HWND, SetCapture)(THIS) PURE;

/**
 * @brief Releases the mouse capture.
 *
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, ReleaseCapture)(THIS) PURE;

/**
 * @brief Sets the input focus to the window.
 *
 * @return HWND Handle to the previous focused window.
 */
STDMETHOD_(HWND, SetFocus)(THIS) PURE;

/**
 * @brief Sends a message to the window.
 *
 * @param message Message ID.
 * @param wParam WPARAM for the message.
 * @param lParam LPARAM for the message.
 * @return LRESULT Result of the message processing.
 */
STDMETHOD_(LRESULT, SendMessage)
(THIS_ UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) PURE;

/**
 * @brief Posts a message to the window.
 *
 * @param message Message ID.
 * @param wParam WPARAM for the message.
 * @param lParam LPARAM for the message.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, PostMessage)
(THIS_ UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) PURE;

/**
 * @brief Sends a notify message to the window.
 *
 * @param message Message ID.
 * @param wParam WPARAM for the message.
 * @param lParam LPARAM for the message.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, SendNotifyMessage)
(THIS_ UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) PURE;

/**
 * @brief Sets the window text.
 *
 * @param lpszString String to set as the window text.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, SetWindowText)(THIS_ LPCTSTR lpszString) PURE;

/**
 * @brief Retrieves the window text.
 *
 * @param lpszStringBuf Buffer to receive the window text.
 * @param nMaxCount Maximum number of characters to copy to the buffer.
 * @return int Length of the string copied to the buffer, not including the terminating null character.
 */
STDMETHOD_(int, GetWindowText)(CTHIS_ LPTSTR lpszStringBuf, int nMaxCount) SCONST PURE;

/**
 * @brief Checks if the window is iconic (minimized).
 *
 * @return BOOL TRUE if the window is iconic, FALSE otherwise.
 */
STDMETHOD_(BOOL, IsIconic)(CTHIS) SCONST PURE;

/**
 * @brief Checks if the window is zoomed (maximized).
 *
 * @return BOOL TRUE if the window is zoomed, FALSE otherwise.
 */
STDMETHOD_(BOOL, IsZoomed)(CTHIS) SCONST PURE;

/**
 * @brief Checks if the window is visible.
 *
 * @return BOOL TRUE if the window is visible, FALSE otherwise.
 */
STDMETHOD_(BOOL, IsWindowVisible)(CTHIS) SCONST PURE;

/**
 * @brief Moves the window to the specified position and resizes it.
 *
 * @param x X position of the window.
 * @param y Y position of the window.
 * @param nWidth Width of the window.
 * @param nHeight Height of the window.
 * @param bRepaint Flag indicating if the window should be repainted.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, MoveWindow)
(THIS_ int x, int y, int nWidth, int nHeight, BOOL bRepaint /*= TRUE*/) PURE;

/**
 * @brief Moves the window to the specified position and resizes it using a rectangle.
 *
 * @param lpRect Rectangle defining the new position and size of the window.
 * @param bRepaint Flag indicating if the window should be repainted.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, MoveWindow2)(THIS_ LPCRECT lpRect, BOOL bRepaint /*= TRUE*/) PURE;

/**
 * @brief Sets the show state of the window.
 *
 * @param nCmdShow Command to show the window (e.g., SW_SHOW, SW_HIDE).
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, ShowWindow)(THIS_ int nCmdShow) PURE;

/**
 * @brief Sets the window region.
 *
 * @param hRgn Handle to the region.
 * @param bRedraw Flag indicating if the window should be redrawn.
 * @return int Previous region handle.
 */
STDMETHOD_(int, SetWindowRgn)(THIS_ HRGN hRgn, BOOL bRedraw /*=TRUE*/) PURE;

/**
 * @brief Sets the layered window attributes.
 *
 * @param crKey Color key for transparency.
 * @param bAlpha Alpha value for transparency.
 * @param dwFlags Flags for the layered window attributes.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, SetLayeredWindowAttributes)
(THIS_ COLORREF crKey, BYTE bAlpha, DWORD dwFlags) PURE;

/**
 * @brief Updates the layered window.
 *
 * @param hdcDst Destination device context.
 * @param pptDst Destination point.
 * @param psize Size of the destination.
 * @param hdcSrc Source device context.
 * @param pptSrc Source point.
 * @param crKey Color key for transparency.
 * @param pblend Blend function.
 * @param dwFlags Flags for the update.
 * @return BOOL TRUE if successful, FALSE otherwise.
 */
STDMETHOD_(BOOL, UpdateLayeredWindow)
(THIS_ HDC hdcDst, POINT *pptDst, SIZE *psize, HDC hdcSrc, POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags) PURE;

/**
 * @brief Sets the message handler for the window.
 *
 * @param fun Function pointer to the message handler.
 * @param ctx Context pointer for the message handler.
 */
STDMETHOD_(void, SetMsgHandler)(THIS_ FunMsgHandler fun, void *ctx) PURE;

/**
 * @brief Retrieves the message handler for the window.
 *
 * @return MsgHandlerInfo* Pointer to the message handler information.
 */
STDMETHOD_(MsgHandlerInfo *, GetMsgHandler)(THIS) PURE;
