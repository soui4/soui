/**
 * @file       SNativeWnd.h
 * @brief      Interface and Implementation for Native Window Handling
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/01
 *
 * @details    This file defines the SNativeWndHelper class and the SNativeWnd class,
 * which provide interfaces and implementations for handling native Windows.
 */

 #ifndef __SNATIVEWND__H__
 #define __SNATIVEWND__H__
 
 #include <interface/SNativeWnd-i.h>
 #include <helper/SCriticalSection.h>
 #include <helper/obj-ref-impl.hpp>
 #include <windows.h>
 #include <soui_exp.h>
 
 SNSBEGIN
 
 /**
  * @class SNativeWndHelper
  * @brief Helper class for managing native window resources.
  *
  * @details This class provides methods for initializing and managing resources related to native windows,
  * such as heap management, shared pointers, and window class registration.
  */
 class SOUI_EXP SNativeWndHelper {
   public:
     /**
      * @brief Retrieves the handle to the heap.
      *
      * @return HANDLE Handle to the heap.
      */
     HANDLE GetHeap()
     {
         return m_hHeap;
     }
 
     /**
      * @brief Locks a shared pointer.
      *
      * @param p Pointer to the shared object.
      */
     void LockSharePtr(void *p);
 
     /**
      * @brief Unlocks a shared pointer.
      */
     void UnlockSharePtr();
 
     /**
      * @brief Retrieves the shared pointer.
      *
      * @return void* Pointer to the shared object.
      */
     void *GetSharePtr()
     {
         return m_sharePtr;
     }
 
     /**
      * @brief Retrieves the application instance handle.
      *
      * @return HINSTANCE Handle to the application instance.
      */
     HINSTANCE GetAppInstance()
     {
         return m_hInst;
     }
 
     /**
      * @brief Retrieves the atom for the simple window class.
      *
      * @return ATOM Atom for the simple window class.
      */
     ATOM GetSimpleWndAtom()
     {
         return m_atom;
     }
 
     /**
      * @brief Initializes the helper with the application instance and window class name.
      *
      * @param hInst Handle to the application instance.
      * @param pszClassName Name of the window class.
      * @param bImeApp Flag indicating if the application uses IME.
      * @return BOOL TRUE if initialization is successful, FALSE otherwise.
      */
     BOOL Init(HINSTANCE hInst, LPCTSTR pszClassName, BOOL bImeApp);
 
     /**
      * @brief unregister window class.      
     */
     BOOL Uninit();
   public:
     /**
      * @brief Retrieves the singleton instance of SNativeWndHelper.
      *
      * @return SNativeWndHelper* Pointer to the singleton instance.
      */
     static SNativeWndHelper *instance()
     {
         static SNativeWndHelper _this;
         return &_this;
     }
 
   private:
     SNativeWndHelper();
     ~SNativeWndHelper();
 
     HANDLE m_hHeap;           //!< Handle to the heap.
     SCriticalSection m_cs;    //!< Critical section for thread-safe operations.
     void *m_sharePtr;         //!< Shared pointer.
     ATOM m_atom;              //!< Atom for the simple window class.
     HINSTANCE m_hInst;        //!< Handle to the application instance.
 };
 
 struct tagThunk;
 
 /**
  * @class SNativeWnd
  * @brief Class for managing native window operations.
  *
  * @details This class provides methods for creating, subclassing, and managing native Windows,
  * as well as handling window messages and notifications.
  */
 class SOUI_EXP SNativeWnd : public TObjRefImpl<INativeWnd> {
   public:
     SNativeWnd();
     virtual ~SNativeWnd(void);
 
     /**
      * @brief Registers a simple window class.
      *
      * @param hInst Handle to the application instance.
      * @param pszSimpleWndName Name of the simple window class.
      * @param bImeWnd Flag indicating if the window uses IME.
      * @return ATOM Atom for the registered window class.
      */
     static ATOM RegisterSimpleWnd(HINSTANCE hInst, LPCTSTR pszSimpleWndName, BOOL bImeWnd);
 
     /**
      * @brief Initializes the window class.
      *
      * @param hInst Handle to the application instance.
      * @param pszSimpleWndName Name of the simple window class.
      * @param bImeWnd Flag indicating if the window uses IME.
      */
     static void InitWndClass(HINSTANCE hInst, LPCTSTR pszSimpleWndName, BOOL bImeWnd);
 
     /**
      * @brief Retrieves the window ID.
      *
      * @return int Window ID.
      */
     STDMETHOD_(int, GetID)(THIS) SCONST
     {
         return GetDlgCtrlID();
     }
 
     /**
      * @brief Sets the window ID.
      *
      * @param nID New window ID.
      */
     STDMETHOD_(void, SetID)(THIS_ int nID)
     {
         SetWindowLongPtr(GWL_ID, nID);
     }
 
     /**
      * @brief Creates a native window.
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
      * @param lpParam Pointer to creation parameters.
      * @return HWND Handle to the created window.
      */
     STDMETHOD_(HWND, CreateNative)
     (THIS_ LPCTSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, int nID DEF_VAL(0), LPVOID lpParam DEF_VAL(0)) OVERRIDE;
 
     /**
      * @brief Retrieves the handle to the window.
      *
      * @return HWND Handle to the window.
      */
     STDMETHOD_(HWND, GetHwnd)(THIS) OVERRIDE;
 
     /**
      * @brief Subclasses an existing window.
      *
      * @param hWnd Handle to the window to subclass.
      * @return BOOL TRUE--SUCCEEDED, FALSE--FAILED.
      */
     STDMETHOD_(BOOL, SubclassWindow)(THIS_ HWND hWnd) OVERRIDE;
 
     /**
      * @brief Unsubclasses a window.
      *
      * @param bForce Flag indicating if the unsubclassing should be forced.
      * @return HWND Handle to the unsubclassed window.
      */
     STDMETHOD_(HWND, UnsubclassWindow)(THIS_ BOOL bForce DEF_VAL(FALSE)) OVERRIDE;
 
     /**
      * @brief Retrieves the current message being processed.
      *
      * @return const MSG* Pointer to the current message.
      */
     STDMETHOD_(const MSG *, GetCurrentMessage)(THIS) SCONST OVERRIDE;
 
     /**
      * @brief Retrieves the dialog control ID.
      *
      * @return int Dialog control ID.
      */
     STDMETHOD_(int, GetDlgCtrlID)(THIS) SCONST OVERRIDE;
 
     /**
      * @brief Retrieves the window style.
      *
      * @return DWORD Window style.
      */
     STDMETHOD_(DWORD, GetStyle)(THIS) SCONST OVERRIDE;
 
     /**
      * @brief Retrieves the extended window style.
      *
      * @return DWORD Extended window style.
      */
     STDMETHOD_(DWORD, GetExStyle)(THIS) SCONST OVERRIDE;
 
     /**
      * @brief Retrieves the long pointer value at the specified index.
      *
      * @param nIndex Index of the value to retrieve.
      * @return LONG_PTR Long pointer value.
      */
     STDMETHOD_(LONG_PTR, GetWindowLongPtr)(THIS_ int nIndex) SCONST OVERRIDE;
 
     /**
      * @brief Sets the long pointer value at the specified index.
      *
      * @param nIndex Index of the value to set.
      * @param dwNewLong New long pointer value.
      * @return LONG_PTR Previous long pointer value.
      */
     STDMETHOD_(LONG_PTR, SetWindowLongPtr)(THIS_ int nIndex, LONG_PTR dwNewLong) OVERRIDE;
 
     /**
      * @brief Retrieves the handle to the parent window.
      *
      * @return HWND Handle to the parent window.
      */
     STDMETHOD_(HWND, GetParent)(THIS) OVERRIDE;
 
     /**
      * @brief Sets the handle to the parent window.
      *
      * @param hWndNewParent Handle to the new parent window.
      * @return HWND Handle to the previous parent window.
      */
     STDMETHOD_(HWND, SetParent)(THIS_ HWND hWndNewParent) OVERRIDE;
 
     /**
      * @brief Checks if the window is enabled.
      *
      * @return BOOL TRUE if the window is enabled, FALSE otherwise.
      */
     STDMETHOD_(BOOL, IsWindowEnabled)(THIS) SCONST OVERRIDE;
 
     /**
      * @brief Modifies the window style.
      *
      * @param dwRemove Styles to remove.
      * @param dwAdd Styles to add.
      * @param nFlags Flags for modification.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, ModifyStyle)
     (THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags DEF_VAL(0)) OVERRIDE;
 
     /**
      * @brief Modifies the extended window style.
      *
      * @param dwRemove Styles to remove.
      * @param dwAdd Styles to add.
      * @param nFlags Flags for modification.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, ModifyStyleEx)
     (THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags DEF_VAL(0)) OVERRIDE;
 
     /**
      * @brief Sets the window position.
      *
      * @param hWndInsertAfter Handle to the window to insert after.
      * @param x X position of the window.
      * @param y Y position of the window.
      * @param cx Width of the window.
      * @param cy Height of the window.
      * @param nFlags Flags for positioning.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, SetWindowPos)
     (THIS_ HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags) OVERRIDE;
 
     /**
      * @brief Centers the window relative to another window.
      *
      * @param hWndCenter Handle to the window to center relative to.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, CenterWindow)(THIS_ HWND hWndCenter DEF_VAL(0)) OVERRIDE;
 
     /**
      * @brief Destroys the window.
      *
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, DestroyWindow)(THIS) OVERRIDE;
 
     /**
      * @brief Checks if the handle is a valid window handle.
      *
      * @return BOOL TRUE if valid, FALSE otherwise.
      */
     STDMETHOD_(BOOL, IsWindow)(THIS) SCONST OVERRIDE;
 
     /**
      * @brief Invalidates the entire window.
      *
      * @param bErase Flag indicating if the background should be erased.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, Invalidate)(THIS_ BOOL bErase DEF_VAL(TRUE)) OVERRIDE;
 
     /**
      * @brief Invalidates a specified rectangle of the window.
      *
      * @param lpRect Rectangle to invalidate.
      * @param bErase Flag indicating if the background should be erased.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, InvalidateRect)(THIS_ LPCRECT lpRect, BOOL bErase DEF_VAL(TRUE)) OVERRIDE;
 
     /**
      * @brief Retrieves the window rectangle.
      *
      * @param lpRect Rectangle to receive the window rectangle.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, GetWindowRect)(THIS_ LPRECT lpRect) SCONST OVERRIDE;
 
     /**
      * @brief Retrieves the client rectangle.
      *
      * @param lpRect Rectangle to receive the client rectangle.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, GetClientRect)(THIS_ LPRECT lpRect) SCONST OVERRIDE;
 
     /**
      * @brief Converts client coordinates to screen coordinates.
      *
      * @param lpPoint Point to convert.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, ClientToScreen)(THIS_ LPPOINT lpPoint) SCONST OVERRIDE;
 
     /**
      * @brief Converts client rectangle coordinates to screen coordinates.
      *
      * @param lpRect Rectangle to convert.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, ClientToScreen2)(THIS_ LPRECT lpRect) SCONST OVERRIDE;
 
     /**
      * @brief Converts screen coordinates to client coordinates.
      *
      * @param lpPoint Point to convert.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, ScreenToClient)(THIS_ LPPOINT lpPoint) SCONST OVERRIDE;
 
     /**
      * @brief Converts screen rectangle coordinates to client coordinates.
      *
      * @param lpRect Rectangle to convert.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, ScreenToClient2)(THIS_ LPRECT lpRect) SCONST OVERRIDE;
 
     /**
      * @brief Maps points from one window to another.
      *
      * @param hWndTo Handle to the destination window.
      * @param lpPoint Points to map.
      * @param nCount Number of points.
      * @return int Number of points mapped.
      */
     STDMETHOD_(int, MapWindowPoints)
     (THIS_ HWND hWndTo, LPPOINT lpPoint, UINT nCount) SCONST OVERRIDE;
 
     /**
      * @brief Maps a rectangle from one window to another.
      *
      * @param hWndTo Handle to the destination window.
      * @param lpRect Rectangle to map.
      * @return int Number of points mapped.
      */
     STDMETHOD_(int, MapWindowRect)(THIS_ HWND hWndTo, LPRECT lpRect) SCONST OVERRIDE;
 
     /**
      * @brief Sets a timer for the window.
      *
      * @param nIDEvent Timer ID.
      * @param nElapse Elapse time in milliseconds.
      * @param lpfnTimer Timer callback function.
      * @return UINT_PTR Timer ID.
      */
     STDMETHOD_(UINT_PTR, SetTimer)
     (THIS_ UINT_PTR nIDEvent, UINT nElapse, void(CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD) DEF_VAL(NULL)) OVERRIDE;
 
     /**
      * @brief Kills a timer for the window.
      *
      * @param nIDEvent Timer ID.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, KillTimer)(THIS_ UINT_PTR nIDEvent) OVERRIDE;
 
     /**
      * @brief Retrieves a device context for the window.
      *
      * @return HDC Handle to the device context.
      */
     STDMETHOD_(HDC, GetDC)(THIS) OVERRIDE;
 
     /**
      * @brief Retrieves a device context for the entire window.
      *
      * @return HDC Handle to the device context.
      */
     STDMETHOD_(HDC, GetWindowDC)(THIS) OVERRIDE;
 
     /**
      * @brief Releases a device context.
      *
      * @param hDC Handle to the device context to release.
      * @return int Number of times the device context was released.
      */
     STDMETHOD_(int, ReleaseDC)(THIS_ HDC hDC) OVERRIDE;
 
     /**
      * @brief Creates a caret for the window.
      *
      * @param hBitmap Handle to the bitmap for the caret.
      * @param nWidth Width of the caret.
      * @param nHeight Height of the caret.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, CreateCaret)(THIS_ HBITMAP hBitmap, int nWidth, int nHeight) OVERRIDE;
 
     /**
      * @brief Hides the caret.
      *
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, HideCaret)(THIS) OVERRIDE;
 
     /**
      * @brief Shows the caret.
      *
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, ShowCaret)(THIS) OVERRIDE;
 
     /**
      * @brief Retrieves the handle to the window that has captured the mouse.
      *
      * @return HWND Handle to the capturing window.
      */
     STDMETHOD_(HWND, GetCapture)(THIS) OVERRIDE;
 
     /**
      * @brief Sets the window to capture the mouse.
      *
      * @return HWND Handle to the previous capturing window.
      */
     STDMETHOD_(HWND, SetCapture)(THIS) OVERRIDE;
 
     /**
      * @brief Releases the mouse capture.
      *
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, ReleaseCapture)(THIS) OVERRIDE;
 
     /**
      * @brief Sets the input focus to the window.
      *
      * @return HWND Handle to the previous focused window.
      */
     STDMETHOD_(HWND, SetFocus)(THIS) OVERRIDE;
 
     /**
      * @brief Sends a message to the window.
      *
      * @param message Message to send.
      * @param wParam WPARAM for the message.
      * @param lParam LPARAM for the message.
      * @return LRESULT Result of the message processing.
      */
     STDMETHOD_(LRESULT, SendMessage)
     (THIS_ UINT message, WPARAM wParam DEF_VAL(0), LPARAM lParam DEF_VAL(0)) OVERRIDE;
 
     /**
      * @brief Posts a message to the window.
      *
      * @param message Message to post.
      * @param wParam WPARAM for the message.
      * @param lParam LPARAM for the message.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, PostMessage)
     (THIS_ UINT message, WPARAM wParam DEF_VAL(0), LPARAM lParam DEF_VAL(0)) OVERRIDE;
 
     /**
      * @brief Sends a notify message to the window.
      *
      * @param message Message to send.
      * @param wParam WPARAM for the message.
      * @param lParam LPARAM for the message.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, SendNotifyMessage)
     (THIS_ UINT message, WPARAM wParam DEF_VAL(0), LPARAM lParam DEF_VAL(0)) OVERRIDE;
 
     /**
      * @brief Sets the window text.
      *
      * @param lpszString String to set as the window text.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, SetWindowText)(THIS_ LPCTSTR lpszString) OVERRIDE;
 
     /**
      * @brief Retrieves the window text.
      *
      * @param lpszStringBuf Buffer to receive the window text.
      * @param nMaxCount Maximum number of characters to copy to the buffer.
      * @return int Length of the string copied to the buffer, not including the terminating null character.
      */
     STDMETHOD_(int, GetWindowText)(THIS_ LPTSTR lpszStringBuf, int nMaxCount) SCONST OVERRIDE;
 
     /**
      * @brief Checks if the window is iconic (minimized).
      *
      * @return BOOL TRUE if the window is iconic, FALSE otherwise.
      */
     STDMETHOD_(BOOL, IsIconic)(THIS) SCONST OVERRIDE;
 
     /**
      * @brief Checks if the window is zoomed (maximized).
      *
      * @return BOOL TRUE if the window is zoomed, FALSE otherwise.
      */
     STDMETHOD_(BOOL, IsZoomed)(THIS) SCONST OVERRIDE;
 
     /**
      * @brief Checks if the window is visible.
      *
      * @return BOOL TRUE if the window is visible, FALSE otherwise.
      */
     STDMETHOD_(BOOL, IsWindowVisible)(THIS) SCONST OVERRIDE;
 
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
     (THIS_ int x, int y, int nWidth, int nHeight, BOOL bRepaint DEF_VAL(TRUE)) OVERRIDE;
 
     /**
      * @brief Moves the window to the specified position and resizes it using a rectangle.
      *
      * @param lpRect Rectangle defining the new position and size of the window.
      * @param bRepaint Flag indicating if the window should be repainted.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, MoveWindow2)(THIS_ LPCRECT lpRect, BOOL bRepaint DEF_VAL(TRUE)) OVERRIDE;
 
     /**
      * @brief Sets the show state of the window.
      *
      * @param nCmdShow Command to show the window (e.g., SW_SHOW, SW_HIDE).
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, ShowWindow)(THIS_ int nCmdShow) OVERRIDE;
 
     /**
      * @brief Sets the window region.
      *
      * @param hRgn Handle to the region.
      * @param bRedraw Flag indicating if the window should be redrawn.
      * @return int Previous region handle.
      */
     STDMETHOD_(int, SetWindowRgn)(THIS_ HRGN hRgn, BOOL bRedraw DEF_VAL(TRUE)) OVERRIDE;
 
     /**
      * @brief Sets the layered window attributes.
      *
      * @param crKey Color key for transparency.
      * @param bAlpha Alpha value for transparency.
      * @param dwFlags Flags for the layered window attributes.
      * @return BOOL TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, SetLayeredWindowAttributes)
     (THIS_ COLORREF crKey, BYTE bAlpha, DWORD dwFlags) OVERRIDE;
 
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
     (THIS_ HDC hdcDst, POINT *pptDst, SIZE *psize, HDC hdcSrc, POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags) OVERRIDE;
 
     /**
      * @brief Sets the message handler for the window.
      *
      * @param fun Function pointer to the message handler.
      * @param ctx Context pointer for the message handler.
      */
     STDMETHOD_(void, SetMsgHandler)(THIS_ FunMsgHandler fun, void *ctx) OVERRIDE;
 
     /**
      * @brief Retrieves the message handler for the window.
      *
      * @return MsgHandlerInfo* Pointer to the message handler information.
      */
     STDMETHOD_(MsgHandlerInfo *, GetMsgHandler)(THIS) OVERRIDE;
 
     /**
      * @brief Calls the default window procedure.
      *
      * @return LRESULT Result of the default window procedure.
      */
     LRESULT DefWindowProc();
 
     /**
      * @brief Forwards notifications to the appropriate handler.
      *
      * @param uMsg Message identifier.
      * @param wParam WPARAM for the message.
      * @param lParam LPARAM for the message.
      * @param bHandled Flag indicating if the message was handled.
      * @return LRESULT Result of the message processing.
      */
     LRESULT ForwardNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
 
     /**
      * @brief Reflects notifications to the appropriate handler.
      *
      * @param uMsg Message identifier.
      * @param wParam WPARAM for the message.
      * @param lParam LPARAM for the message.
      * @param bHandled Flag indicating if the message was handled.
      * @return LRESULT Result of the message processing.
      */
     LRESULT ReflectNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
 
     /**
      * @brief Default reflection handler for notifications.
      *
      * @param hWnd Handle to the window.
      * @param uMsg Message identifier.
      * @param wParam WPARAM for the message.
      * @param lParam LPARAM for the message.
      * @param lResult Result of the message processing.
      * @return BOOL TRUE if the message was handled, FALSE otherwise.
      */
     static BOOL DefaultReflectionHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult);
 
   public: // EXTRACT FROM BEGIN_MSG_MAP_EX and END_MSG_MAP
     /**
      * @brief Processes a window message.
      *
      * @param hWnd Handle to the window.
      * @param uMsg Message identifier.
      * @param wParam WPARAM for the message.
      * @param lParam LPARAM for the message.
      * @param lResult Result of the message processing.
      * @param dwMsgMapID Message map ID.
      * @return BOOL TRUE if the message was handled, FALSE otherwise.
      */
     virtual BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult, DWORD dwMsgMapID = 0);
 
   protected:
     /**
      * @brief Calls the default window procedure with specific parameters.
      *
      * @param uMsg Message identifier.
      * @param wParam WPARAM for the message.
      * @param lParam LPARAM for the message.
      * @return LRESULT Result of the default window procedure.
      */
     LRESULT DefWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
 
     /**
      * @brief Handles the final message for the window.
      *
      * @param hWnd Handle to the window.
      */
     virtual void OnFinalMessage(HWND hWnd);
 
     const MSG *m_pCurrentMsg;  //!< Pointer to the current message.
     BOOL m_bDestoryed;         //!< Flag indicating if the window is destroyed.
     MsgHandlerInfo m_msgHandlerInfo;  //!< Message handler information.
 
   public:
     HWND m_hWnd;  //!< Handle to the window.
 
   protected:
     /**
      * @brief Window procedure callback function.
      *
      * @param hWnd Handle to the window.
      * @param uMsg Message identifier.
      * @param wParam WPARAM for the message.
      * @param lParam LPARAM for the message.
      * @return LRESULT Result of the message processing.
      */
     static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
 
     /**
      * @brief Start window procedure callback function (executed once).
      *
      * @param hWnd Handle to the window.
      * @param uMsg Message identifier.
      * @param wParam WPARAM for the message.
      * @param lParam LPARAM for the message.
      * @return LRESULT Result of the message processing.
      */
     static LRESULT CALLBACK StartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
     tagThunk *m_pThunk;         //!< Thunk structure.
     WNDPROC m_pfnSuperWindowProc;  //!< Pointer to the superclass window procedure.
 };
 
 SNSEND
 #endif // __SNATIVEWND__H__