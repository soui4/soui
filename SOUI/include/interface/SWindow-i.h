#ifndef __SWINDOW_I__H__
#define __SWINDOW_I__H__

#include <interface/sobject-i.h>
#include <interface/sstring-i.h>
#include <interface/SAnimation-i.h>
#include <interface/SRender-i.h>
#include <interface/SMatrix-i.h>
#include <interface/SWndContainer-i.h>
#include <interface/SScriptModule-i.h>
#include <interface/slayout-i.h>
SNSBEGIN

typedef struct ILayout ILayout;
typedef struct ILayoutParam ILayoutParam;
typedef struct IWindow IWindow;

//********************************************************************
/** Unified definition of custom messages used in soui */
//********************************************************************
enum
{
    UM_SOUI_BEGIN = (WM_USER + 10950),
    UM_SETSCALE,
    UM_SETLANGUAGE,
    UM_SETCOLORIZE,
    UM_UPDATEFONT,
    UM_GETDESIREDSIZE,                /**< wp=parent wid,lp=parent hei, return size */
    UM_MENUEVENT,                     /**< Simulate menu control events, wparam:0, lparam:EventArg * */
    UM_RUN_TASKS,                     /**< Execute asynchronous task */
    SPYMSG_BASE = UM_SOUI_BEGIN + 50, /**< Keep consistent with the old version (10000+1000) */
    SPYMSG_SETSPY = SPYMSG_BASE,      /**< Set the window handle for receiving SPY messages */
    SPYMSG_SWNDENUM,                  /**< Enumerate the window list, wparam:SWND, lparam:SWindow::GetWindow */
    SPYMSG_SWNDINFO,                  /**< Get window property, wparam:swnd */
    SPYMSG_HITTEST,                   /**< lparam:pos */
    UM_SOUI_END,
};

/** CancelCaptureMode reason constants */
#define CANCEL_REASON_SCROLL 1 /**< The parent container starts scrolling; notify child controls to cancel touch operations */

typedef BOOL (*FunSwndProc)(IWindow *pSwnd, UINT uMsg, WPARAM wp, LPARAM lp, LRESULT *pbHandled);

#define ICWND_FIRST ((SWindow *)-1) /**< Child window inserted at the beginning */
#define ICWND_LAST  NULL            /**< Child window inserted at the end */

#undef INTERFACE
#define INTERFACE IWindow
DECLARE_INTERFACE_(IWindow, IObject)
{
    DEF_OBJ_BASE(IWindow, Window)
#include <interface/SobjectApi.h>

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Query the control interface
     * @param id REFGUID--Control interface ID
     * @param [out] ppRet IObjRef * *--Control interface
     * @return S_OK--success
     */
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef * *ppRet) PURE;

    /**
     * @brief Get the control container interface
     * @return ISwndContainer *--Control container interface
     */
    STDMETHOD_(ISwndContainer *, GetContainer)(THIS) PURE;

    /**
     * @brief Set the control container interface
     * @param pContainer ISwndContainer *--Control container interface
     * @return
     * @remark Internal call
     */
    STDMETHOD_(void, SetContainer)(THIS_ ISwndContainer * pContainer) PURE;

    /**
     * @brief Get the SOUI container handle
     * @return SWND--SOUI container handle
     */
    STDMETHOD_(SWND, GetSwnd)(CTHIS) SCONST PURE;

    /**
     * @brief Get the layout object
     * @return ILayout *--Layout object
     */
    STDMETHOD_(ILayout *, GetLayout)(THIS) PURE;

    /**
     * @brief Set the layout object
     * @param pLayout ILayout *--Layout object
     * @return
     */
    STDMETHOD_(void, SetLayout)(THIS_ ILayout * pLayout) PURE;

    /**
     * @brief Get the layout parameter object
     * @return ILayoutParam *--Layout parameter object
     */
    STDMETHOD_(ILayoutParam *, GetLayoutParam)(CTHIS) SCONST PURE;

    /**
     * @brief Set the layout parameter object
     * @param pLayoutParam ILayoutParam *--Layout parameter object
     * @return TRUE--success, FALSE--failure
     * @remark The setting succeeds only when the layout parameter object matches the window's layout type
     */
    STDMETHOD_(BOOL, SetLayoutParam)(THIS_ ILayoutParam * pLayoutParam) PURE;

    /**
     * @brief Get the flag indicating whether the current window participates in auto layout
     * @return TRUE--participates in layout, FALSE--does not participate; the user must lay out manually in code
     */
    STDMETHOD_(BOOL, IsFloat)(CTHIS) SCONST PURE;

    /**
     * @brief Get the hidden placeholder flag
     * @return TRUE--window occupies its layout position when hidden, FALSE--not occupied when hidden
     */
    STDMETHOD_(BOOL, IsDisplay)(CTHIS) SCONST PURE;

    /**
     * @brief Get whether the window handles mouse messages
     * @return TRUE--mouse messages are passed through to the parent window
     */
    STDMETHOD_(BOOL, IsMsgTransparent)(CTHIS) SCONST PURE;

    /**
     * @brief Get whether the window automatically clips the client area
     * @return TRUE--automatically clip the client area
     * @remark Clipping the client area restricts the drawing of the window and its children to within the current window's client area, but adds one extra clipping operation
     */
    STDMETHOD_(BOOL, IsClipClient)(CTHIS) SCONST PURE;

    /**
     * @brief Set tooltip
     * @param pszText LPCTSTR--tooltip string
     * @return
     * @remark Supports multilingual
     */
    STDMETHOD_(void, SetToolTipText)(THIS_ LPCTSTR pszText) PURE;

    /**
     * @brief Set tooltip
     * @param pszText LPCSTR--tooltip string(utf8)
     * @return
     * @remark Supports multilingual
     */
    STDMETHOD_(void, SetToolTipTextU8)(THIS_ LPCSTR pszText) PURE;

    /**
     * @brief Get the window's check state flag
     * @return TRUE--checked
     */
    STDMETHOD_(BOOL, IsChecked)(CTHIS) SCONST PURE;

    /**
     * @brief Set the window check state
     * @param bCheck BOOL--Check state
     * @return
     */
    STDMETHOD_(void, SetCheck)(THIS_ BOOL bCheck) PURE;

    /**
     * @brief Get the window's disabled state
     * @return TRUE--window disabled
     */
    STDMETHOD_(BOOL, IsDisabled)(CTHIS_ BOOL bCheckParent) SCONST PURE;

    /**
     * @brief Set the window enabled state
     * @param bEnable BOOL--Enable window
     * @param bUpdate BOOL--Also refresh window flag
     * @return
     */
    STDMETHOD_(void, EnableWindow)(THIS_ BOOL bEnable, BOOL bUpdate) PURE;

    /**
     * @brief Get the window's visible flag
     * @return TRUE--visible
     */
    STDMETHOD_(BOOL, IsVisible)(CTHIS_ BOOL bCheckParent) SCONST PURE;

    /**
     * @brief Set the window visible flag
     * @param bVisible BOOL--Visibility
     * @param bUpdate  BOOL--Also refresh window flag
     * @return
     */
    STDMETHOD_(void, SetVisible)(THIS_ BOOL bVisible, BOOL bUpdate) PURE;

    /**
     * @brief Get the window's user data
     * @return ULONG_PTR--User data
     */
    STDMETHOD_(ULONG_PTR, GetUserData)(CTHIS) SCONST PURE;

    /**
     * @brief Set the window user data
     * @param uData ULONG_PTR--New user data
     * @return ULONG_PTR--Original user data
     */
    STDMETHOD_(ULONG_PTR, SetUserData)(THIS_ ULONG_PTR uData) PURE;

    /**
     * @brief Perform color tinting
     * @param cr COLORREF--Target hue
     * @return
     */
    STDMETHOD_(void, DoColorize)(THIS_ COLORREF cr) PURE;

    /**
     * @brief Get the hue of the current color tint
     * @return COLORREF--Hue
     */
    STDMETHOD_(COLORREF, GetColorizeColor)(CTHIS) SCONST PURE;

    /**
     * @brief Send a message
     * @param uMsg UINT-Message type
     * @param wParam WPARAM--Parameter 1
     * @param lParam LPARAM--Parameter 2
     * @param [out] pbMsgHandled BOOL*--Message handled flag
     * @return
     */
    STDMETHOD_(LRESULT, SSendMessage)
    (THIS_ UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL * pbMsgHandled DEF_VAL(NULL)) PURE;

    /**
     * @brief Dispatch a message to the current window and all its child windows
     * @param uMsg UINT-Message type
     * @param wParam WPARAM--Parameter 1
     * @param lParam LPARAM--Parameter 2
     * @return
     */
    STDMETHOD_(void, SDispatchMessage)(THIS_ UINT uMsg, WPARAM wParam DEF_VAL(0), LPARAM lParam DEF_VAL(0)) PURE;

    /**
     * @brief Get whether the window is allowed to be set as focus
     * @return TRUE--allowed
     */
    STDMETHOD_(BOOL, IsFocusable)(CTHIS) SCONST PURE;

    /**
     * @brief Determine whether it is the focus window
     * @return TRUE--Focus window
     */
    STDMETHOD_(BOOL, IsFocused)(CTHIS) SCONST PURE;

    /**
     * @brief Set the current window as the focus window
     * @return
     */
    STDMETHOD_(void, SetFocus)(THIS) PURE;

    /**
     * @brief Cancel focus
     * @return
     */
    STDMETHOD_(void, KillFocus)(THIS) PURE;

    /**
     * @brief Refresh the current window's client area
     * @return
     */
    STDMETHOD_(void, Invalidate)(THIS) PURE;

    /**
     * @brief Refresh the specified position of the current window
     * @param lprect LPCRECT--Window position
     * @return
     * @remark May include the non-client area, but does not refresh positions outside the window
     */
    STDMETHOD_(void, InvalidateRect)(THIS_ LPCRECT lprect) PURE;

    /**
     * @brief Lock refresh, interrupt refresh calls
     * @return
     */
    STDMETHOD_(void, LockUpdate)(THIS) PURE;

    /**
     * @brief Unlock refresh, resume refresh calls
     * @return
     */
    STDMETHOD_(void, UnlockUpdate)(THIS) PURE;

    /**
     * @brief Get the refresh lock state
     * @return TRUE--refresh is currently locked
     */
    STDMETHOD_(BOOL, IsUpdateLocked)(CTHIS_ BOOL bCheckParent DEF_VAL(FALSE)) SCONST PURE;

    /**
     * @brief Request to update the window immediately
     * @param BOOL bForce, TRUE-Force refresh, FALSE-Refresh only if there is an invalid region
     * @return
     * @remark Normally, to refresh a window you should use Invalidate/InvalidateRect, so that all requests in the system are repainted on the next refresh.
      However, this may cause a delay. To repaint immediately, you can call Update
     */
    STDMETHOD_(void, Update)(THIS_ BOOL bForce DEF_VAL(FALSE)) PURE;

    /**
     * @brief Set the window display region
     * @param pRgn IRegionS*--Display region
     * @param bRedraw BOOL--Redraw
     * @return
     * @remark The top-left corner of the region is (0,0); during display, its position is adjusted according to the window's real coordinates
     */
    STDMETHOD_(void, SetWindowRgn)(THIS_ IRegionS * pRgn, BOOL bRedraw) PURE;

    /**
     * @brief Get the window display region
     * @return IRegionS*--Display region
     */
    STDMETHOD_(IRegionS *, GetWindowRgn)(CTHIS) SCONST PURE;

    /**
     * @brief Set the window display region
     * @param pRgn IPathS*--Display region
     * @param bRedraw BOOL--Redraw
     * @return
     * @remark The top-left corner of the region is (0,0); during display, its position is adjusted according to the window's real coordinates
     */
    STDMETHOD_(void, SetWindowPath)(THIS_ IPathS * pPath, BOOL bRedraw) PURE;

    /**
     * @brief Get the window display region
     * @return IPathS*--Display region
     */
    STDMETHOD_(IPathS *, GetWindowPath)(CTHIS) SCONST PURE;

    /**
     * Move2
     * @brief    Move the window to the specified position
     * @param    int x --  left
     * @param    int y --  top
     * @param    int cx --  width
     * @param    int cy --  height
     * @return   void
     *
     * Describe
     * @see     Move(LPRECT prect)
     */
    STDMETHOD_(void, Move2)(THIS_ int x, int y, int cx DEF_VAL(-1), int cy DEF_VAL(-1)) PURE;

    /**
     * @brief Move the window to the specified position
     * @param prect LPCRECT--Window coordinates
     * @return
     * remark Coordinates of the container within the container
     */
    STDMETHOD_(void, Move)(THIS_ LPCRECT prect) PURE;

    /**
     * @brief Get the window display position, including the window's non-client area
     * @param [out] LPRECT--Window display position
     * @remark The top-left corner of a SOUI window is not (0,0), but coordinates within the host window
     */
    STDMETHOD_(void, GetWindowRect)(CTHIS_ LPRECT prect) SCONST PURE;

    /**
     * @brief Get the window display position, excluding the window's non-client area
     * @param [out] LPRECT--Window client area display position
     * @remark The top-left corner of a SOUI window is not (0,0), but coordinates within the host window
     */
    STDMETHOD_(void, GetClientRect)(CTHIS_ LPRECT prect) SCONST PURE;

    /**
     * @brief Get the window display position after being clipped by the parent window
     * @param [out] LPRECT--Window client area display position
     * @remark Does not account for clipping of child windows and sibling windows
     */
    STDMETHOD_(void, GetVisibleRect)(CTHIS_ LPRECT prect) SCONST PURE;
    /**
     * @brief Check whether a point is within the window range
     * @param pt POINT--Coordinates to check
     * @param bClientOnly BOOL--Test client area flag
     * @return TRUE--Within the window range
     */
    STDMETHOD_(BOOL, IsContainPoint)(CTHIS_ POINT pt, BOOL bClientOnly) SCONST PURE;

    /**
     * @brief Check the window pointed to by pt
     * @param [in,out] pt POINT *--Test coordinates
     * @param bIncludeMsgTransparent BOOL--Test message pass-through window flag
     * @return SWND--Topmost window containing the specified coordinates
     */
    STDMETHOD_(SWND, SwndFromPoint)(CTHIS_ POINT * pt, BOOL bIncludeMsgTransparent DEF_VAL(FALSE)) SCONST PURE;

    /**
     * @brief Set the container timer
     * @param id char--Timer ID, range 0-127
     * @param uElapse UINT--Timer delay
     * @return TRUE--success
     */
    STDMETHOD_(BOOL, SetTimer)(THIS_ char id, UINT uElapse) PURE;

    /**
     * @brief Remove the timer created by SetTimer
     * @param id char--Timer ID created by SetTimer
     * @return
     */
    STDMETHOD_(BOOL, KillTimer)(THIS_ char id) PURE;

    /**
     * @brief Get the mouse capture
     * @return SWND--Mouse capture window handle
     */
    STDMETHOD_(SWND, GetCapture)(CTHIS) SCONST PURE;

    /**
     * @brief Set the container as mouse capture
     * @return SWND--Previous mouse capture window handle
     */
    STDMETHOD_(SWND, SetCapture)(THIS) PURE;

    /**
     * @brief Release the mouse capture
     * @return TRUE--Released successfully
     */
    STDMETHOD_(BOOL, ReleaseCapture)(THIS) PURE;

    /**
     * @brief Set a window animation
     * @param animation IAnimation *--Animation object
     * @return
     */
    STDMETHOD_(void, SetAnimation)(THIS_ IAnimation * animation) PURE;

    /**
     * @brief Start an animation object immediately
     * @param animation IAnimation *--Animation object
     * @param bStartNow BOOL--Start immediately flag
     * @return
     * @remark Unlike setAnimation, this starts immediately instead of waiting for the animation object's delayed start time
     */
    STDMETHOD_(void, StartAnimation)(THIS_ IAnimation * animation, BOOL bStartNow DEF_VAL(FALSE)) PURE;

    /**
     * @brief Get the currently running animation object
     * @return IAnimation *--Animation object
     */
    STDMETHOD_(IAnimation *, GetAnimation)(CTHIS) SCONST PURE;

    /**
     * @brief Clear the animation
     * @return
     */
    STDMETHOD_(void, ClearAnimation)(THIS) PURE;

    /**
     * @brief Set the window opacity
     * @param byAlpha BYTE--Opacity, [0,255]
     * @return
     */
    STDMETHOD_(void, SetAlpha)(THIS_ BYTE byAlpha) PURE;

    /**
     * @brief Get the window opacity
     * @return BYTE--Window opacity
     */
    STDMETHOD_(BYTE, GetAlpha)(CTHIS) SCONST PURE;

    /**
     * @brief Set the window transformation matrix
     * @param mtx const IMatrix *--Transformation matrix
     * @return
     */
    STDMETHOD_(void, SetMatrix)(THIS_ const IMatrix *mtx, BOOL bInvalidate DEF_VAL(TRUE)) PURE;

    /**
     * @brief Get the window transformation matrix
     */
    STDMETHOD_(void, GetMatrix)(CTHIS_ IMatrix * mtx) SCONST PURE;

    /**
     * @brief Get the dpi zoom factor
     * @return int--dpi zoom factor, base is 100
     */
    STDMETHOD_(int, GetScale)(CTHIS) SCONST PURE;

    /**
     * @brief Request re-layout
     * @return
     */
    STDMETHOD_(void, RequestRelayout)(THIS) PURE;

    /**
     * @brief Re-layout the current window's child windows; only takes effect when the window layout is dirty
     * @return
     */
    STDMETHOD_(void, UpdateLayout)(THIS) PURE;

    /**
     * @brief Force re-layout of child windows
     * @return
     */
    STDMETHOD_(void, UpdateChildrenPosition)(THIS) PURE;

    /**
     * @brief Get the layout dirty flag
     * @return TRUE--Layout is dirty
     */
    STDMETHOD_(BOOL, IsLayoutDirty)(CTHIS) SCONST PURE;

    /**
     * @brief Get the window's flag for responding to the keyboard
     * @return UINT--Flag bit for responding to keyboard
     */
    STDMETHOD_(UINT, OnGetDlgCode)(CTHIS) SCONST PURE;

    /**
     * @brief Get the root window
     * @return IWindow *--Root window object
     */
    STDMETHOD_(IWindow *, GetIRoot)(CTHIS) SCONST PURE;

    /**
     * @brief Get the parent window
     * @return IWindow *--Parent window object
     */
    STDMETHOD_(IWindow *, GetIParent)(CTHIS) SCONST PURE;

    /**
     * @brief Get the window associated with the current window in the DOM tree according to uCode
     * @param uCode UINT--Association type
     * @return IWindow *--Window associated with the current window in the DOM tree
     */
    STDMETHOD_(IWindow *, GetIWindow)(CTHIS_ int uCode) SCONST PURE;

    /**
     * @brief Get the child window
     * @param iChild int--Child window index, [0,childCount]
     * @return IWindow *--Child window
     * @remark iChild==0 returns self, 1 returns the first child window
     */
    STDMETHOD_(IWindow *, GetIChild)(CTHIS_ int iChild) SCONST PURE;

    /**
     * @brief Get the number of child windows
     * @return UINT--Number of child windows
     */
    STDMETHOD_(UINT, GetChildrenCount)(CTHIS) SCONST PURE;

    /**
     * @brief Determine whether a window is a descendant window of the current window
     * @param pTest const IWindow *--Test window
     * @return TRUE--Is a descendant window
     */
    STDMETHOD_(BOOL, IsDescendant)(CTHIS_ const IWindow *pTest) SCONST PURE;

    /**
     * @brief Set the window's Owner
     * @param pOwner IWindow *--Owner window
     * @return
     */
    STDMETHOD_(void, SetIOwner)(THIS_ IWindow * pOwner) PURE;

    /**
     * @brief Get the Owner window
     * @return IWindow *--Owner window
     */
    STDMETHOD_(IWindow *, GetIOwner)(CTHIS) SCONST PURE;

    /**
     * @brief Move the current window to the top of the z-order among sibling windows
     * @param
     * @return
     */
    STDMETHOD_(void, BringWindowToTop)(THIS) PURE;

    /**
     * AdjustZOrder
     * @brief    Adjust window Z-order
     * @param    SWindow *pInsertAfter --  Insert after this window
     * @return   bool, returns failure if pInsertAfter is not a sibling window of this
     *
     * Describe  pInsertAfter can be NULL, or a sibling window at the same level as this
     */
    STDMETHOD_(BOOL, AdjustIZOrder)(THIS_ IWindow * pInsertAfter) PURE;

    /**
     * InsertChild
     * @brief    Insert a child window into the window tree
     * @param    SWindow * pNewChild --  Child window object
     * @param    SWindow * pInsertAfter --  Insertion position
     * @return   void
     *
     * Describe  Generally used during UI initialization; the inserted window does not automatically enter the layout flow
     */
    STDMETHOD_(void, InsertIChild)(THIS_ IWindow * pNewChild, IWindow * pInsertAfter DEF_VAL(ICWND_LAST)) PURE;

    /**
     * RemoveChild
     * @brief    Remove a child window object from the window tree
     * @param    SWindow * pChild --  Child window object
     * @return   BOOL
     *
     * Describe  The child window is not automatically released
     */
    STDMETHOD_(BOOL, RemoveIChild)(THIS_ IWindow * pChild) PURE;

    /**
     * CreateChildrenFromXml
     * @brief    Create child windows from XML
     * @param    LPCWSTR pszXml --  Valid utf16-encoded XML string
     * @return   BOOL Whether creation succeeded
     *
     * Describe
     */
    STDMETHOD_(BOOL, CreateChildrenFromXml)(THIS_ LPCWSTR pszXml) PURE;

    /**
     * CreateChildrenFromResId
     * @brief    Create child windows from XML resource
     * @param    LPCTSTR pszResId --  XML resource ID
     * @return   BOOL Whether creation succeeded
     *
     * Describe
     */
    STDMETHOD_(BOOL, CreateChildrenFromResId)(THIS_ LPCTSTR pszResId) PURE;

    /**
     * InitFromResId
     * @brief    Create child windows from XML resource
     * @param    LPCTSTR pszResId --  XML resource ID
     * @return   BOOL Whether creation succeeded
     *
     * Describe
     */
    STDMETHOD_(BOOL, InitFromResId)(THIS_ LPCTSTR pszResId) PURE;

    /**
     * @brief Find a child window by ID
     * @param nId int--child window ID
     * @return IWindow*--matched window
     * @remark Uses breadth-first search to find the matching child window
     */
    STDMETHOD_(IWindow *, FindIChildByID)(THIS_ int nId) PURE;

    /**
     * @brief Find a child window by Name
     * @param pszName LPCWSTR--child window Name
     * @return IWindow*--matched window
     * @remark Uses breadth-first search to find the matching child window
     */
    STDMETHOD_(IWindow *, FindIChildByName)(THIS_ LPCWSTR pszName) PURE;

    /**
     * @brief Find child window by Name
     * @param pszName LPCSTR--Child window Name
     * @return IWindow*--Matched window
     * @remark Uses breadth-first search to find matching child windows
     */
    STDMETHOD_(IWindow *, FindIChildByNameA)(THIS_ LPCSTR pszName) PURE;

    /**
     * DestroyChild
     * @brief    Destroy a child window
     * @param    SWindow * pChild --  Child window object
     * @return   BOOL
     *
     * Describe  First calls RemoveChild, then calls pChild->Release to free the child window object
     */
    STDMETHOD_(BOOL, DestroyIChild)(THIS_ IWindow * pChild) PURE;

    /**
     * @brief Destroy all child windows
     * @return
     */
    STDMETHOD_(void, DestroyAllChildren)(THIS) PURE;

    /**
     * @brief Destroy the window
     * @return TRUE--success
     */
    STDMETHOD_(BOOL, Destroy)(THIS) PURE;

    /**
     * @brief Get the next window object that participates in layout for the specified window
     * @param pCurChild const IWindow *--Current window
     * @return IWindow *--Next layout window
     */
    STDMETHOD_(IWindow *, GetNextLayoutIChild)(CTHIS_ const IWindow *pCurChild) SCONST PURE;

    /**
     * GetChildrenLayoutRect
     * @brief    Get the layout space of child windows
     * @param  LPRECT prc--Layout position
     * @return   void
     *
     * Describe  Usually the client area, but controls such as tab and group differ
     */
    STDMETHOD_(void, GetChildrenLayoutRect)(CTHIS_ RECT * prc) SCONST PURE;

    /**
     * GetDesiredSize
     * @brief    When the window size is not specified, compute the desired window size via e.g. skin
     * @param    int nParentWid -- Container width, <0 means the container width depends on the current window width
     * @param    int nParentHei -- Container height, <0 means the container height depends on the current window height
     * @return   CSize
     *
     * Describe
     */
    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE * pSize, int nParentWid, int nParentHei) PURE;

    /**
     * @brief Get the window background color
     * @return COLORREF--Window background color
     */
    STDMETHOD_(COLORREF, GetBkgndColor)(CTHIS) SCONST PURE;

    /**
     * @brief Set the window display text
     * @param lpszText LPCTSTR--Window display text
     * @return
     * @remark Supports automatic translation
     */
    STDMETHOD_(void, SetWindowText)(THIS_ LPCTSTR lpszText) PURE;

    /**
     * @brief Set the window display text
     * @param lpszText LPCSTR--Window display text(utf8)
     * @return
     * @remark Supports automatic translation
     */
    STDMETHOD_(void, SetWindowTextU8)(THIS_ LPCSTR lpszText) PURE;

    /**
     * @brief Get the window display text
     * @param pBuf TCHAR*--Buffer
     * @param nBufLen int--Buffer length
     * @param bRawText BOOL--Flag to get text before translation
     * @return int--Length of copied text; returns total buffer length when pBuf is NULL
     */
    STDMETHOD_(int, GetWindowText)(THIS_ TCHAR * pBuf, int nBufLen, BOOL bRawText) SCONST PURE;

    STDMETHOD_(int, GetWindowTextU8)(THIS_ IStringA * pStr, BOOL bRawText) SCONST PURE;

    /**
     * @brief Get the window state
     * @return DWORD -- Window state
     */
    STDMETHOD_(DWORD, GetState)(CTHIS) SCONST PURE;

    /**
     * @brief Modify the window state
     * @param dwStateAdd DWORD--State bit to add
     * @param dwStateRemove DWORD--State bit to remove
     * @param bUpdate BOOL--Refresh window
     * @return DWORD--New state
     */
    STDMETHOD_(DWORD, ModifyState)(THIS_ DWORD dwStateAdd, DWORD dwStateRemove, BOOL bUpdate DEF_VAL(FALSE)) PURE;

    /**
     * GetISelectedSiblingInGroup
     * @brief    Get the window in a selected state within a group
     * @return   SWindow *
     *
     * Describe  Returns NULL when the window is not in a group
     */
    STDMETHOD_(IWindow *, GetISelectedSiblingInGroup)(THIS) PURE;

    /**
     * GetSelectedChildInGroup
     * @brief    Get the child window in selected state
     * @return   IWindow * -- Window in selected state
     * Describe
     */
    STDMETHOD_(IWindow *, GetISelectedChildInGroup)(THIS) PURE;

    /**
     * @brief Flag indicating auto-grouping of sibling windows of the same type
     * @return TRUE--Auto group
     * @remark When auto-grouped windows receive direction key presses from the user, the windows within the group gain focus in cyclic order
     */
    STDMETHOD_(BOOL, IsSiblingsAutoGroupped)(CTHIS) SCONST PURE;

    /** Caret-related methods */

    /**
     * @brief Create the cursor
     * @param pBmp Bitmap for creating the cursor, can be null
     * @param nWid int--Cursor width
     * @param nHeight int--Cursor height
     * @return TRUE--success
     */
    STDMETHOD_(BOOL, CreateCaret)(THIS_ HBITMAP pBmp, int nWid, int nHeight) PURE;

    /**
     * @brief Control cursor show/hide
     * @param bShow BOOL--Show flag
     * @return
     */
    STDMETHOD_(void, ShowCaret)(THIS_ BOOL bShow) PURE;

    /**
     * @brief Set the cursor display position
     * @param x int-X
     * @param y int-Y
     * @return
     */
    STDMETHOD_(void, SetCaretPos)(THIS_ int x, int y) PURE;

    /**
     * @brief Set whether the current control is allowed to emit events
     * @param bMute BOOL-Flag indicating whether events are allowed to be emitted
     * @return
     */
    STDMETHOD_(void, SetEventMute)(THIS_ BOOL bMute) PURE;

    /**
     * @brief Subscribe to window events
     * @param evtId DWORD--Event ID
     * @param pSlot const IEvtSlot *--Event handling object
     * @return TRUE--success
     */
    STDMETHOD_(BOOL, SubscribeEvent)(THIS_ DWORD evtId, const IEvtSlot *pSlot) PURE;

    /**
     * @brief Unsubscribe from window events
     * @param evtId DWORD--Event ID
     * @param pSlot const IEvtSlot *--Event handling object
     * @return TRUE--success
     */
    STDMETHOD_(BOOL, UnsubscribeEvent)(THIS_ DWORD evtId, const IEvtSlot *pSlot) PURE;

    /**
     * @brief Add a window event
     * @param evtId DWORD--Event ID
     * @param LPCWSTR pszEventHandlerName--String-identified event handler (used by scripts)
     * @return TRUE--success
     */
    STDMETHOD_(BOOL, AddEvent)(THIS_ DWORD dwEventID, LPCWSTR pszEventHandlerName) PURE;

    /**
     * @brief Remove a window event
     * @param evtId DWORD--Event ID
     * @return TRUE--success
     */
    STDMETHOD_(BOOL, RemoveEvent)(THIS_ DWORD dwEventID) PURE;
    /**
     * @brief  Fire an event to the application layer
     * @param evt IEvtArgs *--Event object
     * @return TRUE--success
     */
    STDMETHOD_(BOOL, FireEvent)(THIS_ IEvtArgs * evt) PURE;

    /**
     * FireCommand
     * @brief    Activate the window's EVT_CMD event
     * @return   BOOL-- true:EVT_CMD event was handled
     *
     * Describe
     */
    STDMETHOD_(BOOL, FireCommand)(THIS) PURE;

    /**
     * FireCtxMenu
     * @brief    Activate the context menu event
     * @param    CPoint pt --  Mouse click position
     * @return   BOOL -- true:context menu event handled externally
     *
     * Describe
     */
    STDMETHOD_(BOOL, FireCtxMenu)(THIS_ POINT pt) PURE;

    /**
     * SwndProc
     * @brief    Default message processing function
     * @param    UINT uMsg --  Message type
     * @param    WPARAM wParam --  Parameter 1
     * @param    LPARAM lParam --  Parameter 2
     * @param    LRESULT & lResult --  Message return value
     * @return   BOOL Whether handled
     *
     * Describe  Messages not handled in the message map table are processed by this function
     */
    STDMETHOD_(BOOL, SwndProc)(THIS_ UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT * lResult) PURE;

    /**
     * SetSwndProc
     * @brief    Set message handling function
     * @param    FunSwndProc swndProc --  message handling function
     * @return   void
     *
     * Describe
     */
    STDMETHOD_(void, SetSwndProc)(THIS_ FunSwndProc swndProc) PURE;

    /**
     * GetHostHwnd
     * @brief    Get host Hwnd
     * @return   HWND -- host hwnd
     *
     * Describe
     */
    STDMETHOD_(HWND, GetHostHwnd)(THIS) SCONST PURE;

    /**
     * GetTimelineHandlersMgr
     * @brief    Get ITimelineHandlersMgr
     * @return   ITimelineHandlersMgr* -- timeline manager
     *
     * Describe
     */
    STDMETHOD_(ITimelineHandlersMgr *, GetTimelineHandlersMgr)(THIS) PURE;

    /**
     * IsVideoCanvas
     * @brief    Query whether the window is a video window
     * @return   BOOL -- TRUE: is a video window
     *
     * Describe
     */
    STDMETHOD_(BOOL, IsVideoCanvas)(CTHIS) SCONST PURE;

    /**
     * RegisterDragDrop
     * @brief    Register the window as a drop target
     * @param    IDropTarget * pDragTarget --  drop target object
     * @return   BOOL -- TRUE: success
     *
     * Describe
     */
    STDMETHOD_(BOOL, RegisterDragDrop)(THIS_ IDropTarget * pDragTarget) PURE;

    /**
     * UnregisterDragDrop
     * @brief    Unregister the drop target
     * @return   BOOL -- TRUE: success
     *
     * Describe
     */
    STDMETHOD_(BOOL, UnregisterDragDrop)(THIS) PURE;

    /**
     * @brief Set the layer the window belongs to
     * @param nLayer int--layer the window belongs to
     * @return
     */
    STDMETHOD_(void, SetLayer)(THIS_ int nLayer) PURE;

    /**
     * @brief Get the layer where the window is located
     * @return int--the layer where the window is located
     */
    STDMETHOD_(int, GetLayer)(CTHIS) SCONST PURE;

    /**
     * @brief Update the property animator state
     * @param pHolder IPropertyValuesHolder*--property value holder
     * @param fraction float--animation progress (0.0-1.0)
     * @param state ANI_STATE--animation state (ANI_START/ANI_PROGRESS/ANI_END)
     */
    STDMETHOD_(BOOL, SetAnimatorValue)(THIS_ IPropertyValuesHolder * pHolder, float fraction, ANI_STATE state) PURE;

    /**
     * @brief Set the window property animation matrix center
     * @param x float--center X coordinate, 0.5f means window center
     * @param y float--center Y coordinate, 0.5f means window center
     */
    STDMETHOD_(void, SetPivot)(THIS_ float x, float y) PURE;

    /**
     * @brief Get the window property animation matrix center
     * @param x float*--center X coordinate, 0.5f means window center
     * @param y float*--center Y coordinate, 0.5f means window center
     */
    STDMETHOD_(void, GetPivot)(CTHIS_ float *x, float *y) SCONST PURE;
};

SNSEND
#endif /**< __SWINDOW_I__H__ */