/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       Swnd.h
 * @brief      SOUI基础DUI窗口模块
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Description: This file defines the SWindow class, which is the base class for SOUI DUI windows.
 */

 #ifndef __SWND__H__
 #define __SWND__H__
 
 #include <core/SWindowMgr.h>
 #include <interface/SWndContainer-i.h>
 #include <interface/slayout-i.h>
 #include <interface/saccproxy-i.h>
 #include <interface/scaret-i.h>
 #include <helper/SwndMsgCracker.h>
 #include <layout/SLayoutSize.h>
 #include <event/SEventSlot.h>
 #include <event/SEvents.h>
 #include <event/SEventSet.h>
 #include <res.mgr/SUiDef.h>
 #include <core/SWndStyle.h>
 #include <core/SSkin.h>
 #include <animation/SAnimation.h>
 #include <interface/SWindow-i.h>
 
 #define SC_WANTARROWS  0x0001     /* Control wants arrow keys         */
 #define SC_WANTTAB     0x0002     /* Control wants tab keys           */
 #define SC_WANTRETURN  0x0004     /* Control wants return keys        */
 #define SC_WANTCHARS   0x0008     /* Want WM_CHAR messages            */
 #define SC_WANTALLKEYS 0xFFFF     /* Control wants all keys           */
 #define SC_WANTSYSKEY  0x80000000 /* System Key */
 
 #define ICWND_FIRST ((SWindow *)-1) /* 子窗口插入在开头 */
 #define ICWND_LAST  NULL            /* 子窗口插入在末尾 */
 
 #ifdef _DEBUG
 #define ASSERT_UI_THREAD() SNS::SWindow::TestMainThread()
 #else
 #define ASSERT_UI_THREAD()
 #endif
 
 SNSBEGIN
 
 /**
  * @enum NormalShow
  * @brief Flags for window show state.
  */
 enum {
     NormalShow = 0, /**< Normal show state */
     ParentShow = 1  /**< Parent show state */
 };
 
 /**
  * @enum NormalEnable
  * @brief Flags for window enable state.
  */
 enum {
     NormalEnable = 0, /**< Normal enable state */
     ParentEnable = 1  /**< Parent enable state */
 };
 
 /**
  * @enum WndState
  * @brief Window state flags.
  */
 enum WndState {
     WndState_Normal = 0x00000000UL,    /**< Normal state */
     WndState_Hover = 0x00000001UL,     /**< Hover state */
     WndState_PushDown = 0x00000002UL,  /**< Push down state */
     WndState_Check = 0x00000004UL,     /**< Check state */
     WndState_Invisible = 0x00000008UL, /**< Invisible state */
     WndState_Disable = 0x00000010UL    /**< Disable state */
 };
 
 /**
  * @struct SWNDMSG
  * @brief Structure representing a window message.
  */
 typedef struct SWNDMSG {
     UINT uMsg;   /**< Message identifier */
     WPARAM wParam; /**< Additional message-specific information */
     LPARAM lParam; /**< Additional message-specific information */
 } SWNDMSG, *PSWNDMSG;
 
 /**
  * @class SStateHelper
  * @brief Helper class for managing window states.
  */
 class SStateHelper {
 public:
     /**
      * @brief Marks a state as active.
      * @param dwState Current state flags.
      * @param state State to mark.
      */
     static void MarkState(DWORD &dwState, WndState state) {
         dwState |= state;
     }
 
     /**
      * @brief Clears a state.
      * @param dwState Current state flags.
      * @param state State to clear.
      */
     static void ClearState(DWORD &dwState, WndState state) {
         dwState &= ~state;
     }
 
     /**
      * @brief Tests if a state is active.
      * @param dwState Current state flags.
      * @param state State to test.
      * @return TRUE if the state is active, FALSE otherwise.
      */
     static bool TestState(DWORD dwState, WndState state) {
         return (dwState & state) == state;
     }
 };
 
 /**
  * @class STimerID
  * @brief Structure representing a timer ID.
  */
 class SOUI_EXP STimerID {
 public:
     DWORD swnd : 24;      /**< Window handle */
     DWORD uTimerID : 7;   /**< Timer ID */
     DWORD bSwndTimer : 1; /**< Flag indicating if it's a SWND timer */
 
     /**
      * @brief Constructor.
      * @param swnd_ Window handle.
      * @param timeId Timer ID.
      */
     STimerID(SWND swnd_, char timeId)
         : swnd(swnd_)
         , uTimerID(timeId)
         , bSwndTimer(1) {
         SASSERT(swnd < 0x00FFFFFF && timeId >= 0);
     }
 
     /**
      * @brief Constructor.
      * @param dwID Combined timer ID.
      */
     STimerID(DWORD dwID) {
         memcpy(this, &dwID, sizeof(DWORD));
     }
 
     /**
      * @brief Conversion operator to DWORD.
      * @return Combined timer ID as DWORD.
      */
     operator DWORD &() const {
         return *(DWORD *)this;
     }
 };
 
 /**
  * @class SPainter
  * @brief Helper class for painting.
  */
 class SOUI_EXP SPainter {
 public:
     SPainter()
         : oldTextColor(CR_INVALID) {}
 
     SAutoRefPtr<IFontS> oldFont; /**< Old font */
     COLORREF oldTextColor;       /**< Old text color */
 };
 
 /**
  * @enum GW_CODE
  * @brief Codes for getting related windows.
  */
 typedef enum tagGW_CODE {
     GSW_FIRSTCHILD = 0, /**< First child window */
     GSW_LASTCHILD,      /**< Last child window */
     GSW_PREVSIBLING,    /**< Previous sibling window */
     GSW_NEXTSIBLING,    /**< Next sibling window */
     GSW_PARENT,         /**< Parent window */
     GSW_OWNER           /**< Owner window */
 } GW_CODE;
 
 /**
  * @struct SwndToolTipInfo
  * @brief Information for window tooltips.
  */
 struct SwndToolTipInfo {
     SWND swnd;       /**< Window handle */
     DWORD dwCookie;  /**< Tooltip ID */
     CRect rcTarget;  /**< Tooltip target rectangle */
     SStringT strTip; /**< Tooltip text */
 };
 
 /**
  * @enum HRET_FLAG
  * @brief Flags for handling attributes.
  */
 enum {
     HRET_FLAG_STYLE = (1 << 16),        /**< Style attribute flag */
     HRET_FLAG_LAYOUT = (1 << 17),       /**< Layout attribute flag */
     HRET_FLAG_LAYOUT_PARAM = (1 << 18)  /**< Layout parameter attribute flag */
 };
 
 /**
  * @class STrText
  * @brief Class for handling text with translation support.
  */
 class SOUI_EXP STrText {
 public:
     /**
      * @brief Constructor.
      * @param pOwner Owner window.
      */
     STrText(SWindow *pOwner = NULL);
 
     /**
      * @brief Sets the owner window.
      * @param pOwner Owner window.
      */
     void SetOwner(SWindow *pOwner);
 
     /**
      * @brief Sets the text.
      * @param strText Text to set.
      * @param bAutoEscape Flag to automatically escape the text.
      */
     void SetText(const SStringT &strText, bool bAutoEscape = true);
 
     /**
      * @brief Gets the text.
      * @param bRawText Flag to get raw text.
      * @return Text string.
      */
     SStringT GetText(BOOL bRawText = FALSE) const;
 
     /**
      * @brief Translates the text.
      */
     void TranslateText();
 
     /**
      * @brief Escapes a string.
      * @param str String to escape.
      * @return Escaped string.
      */
     static SStringW EscapeString(const SStringW &str);
 
 protected:
     SWindow *pOwner;       /**< Owner window */
     SStringT strRaw;       /**< Raw text */
     bool bAutoEscape;      /**< Auto-escape flag */
     SStringT strTr;        /**< Translated text */
 };
 
 /**
  * @class SWindow
  * @brief Base class for SOUI DUI windows.
  *
  * This class provides the basic functionality for SOUI DUI windows, including layout,
  * event handling, and rendering.
  */
 class SOUI_EXP SWindow
     : public TObjRefImpl<SObjectImpl<IWindow>>
     , protected IAnimationListener {
     DEF_SOBJECT(SObjectImpl<IWindow>, L"window")
 
     friend class SwndLayoutBuilder;
     friend class SWindowRepos;
     friend class SHostWnd;
     friend class SwndContainerImpl;
     friend class FocusSearch;
     friend class SGridLayout;
     friend class SLinearLayout;
     friend class SouiLayout;
     friend class SHostProxy;
 
     class SAnimationHandler : public ITimelineHandler {
     private:
         SWindow *m_pOwner;               /**< Owner window */
         STransformation m_transform;     /**< Transformation */
         bool m_bFillAfter;               /**< Fill after flag */
         SWindow *m_pPrevSiblingBackup;   /**< Previous sibling backup */
 
     public:
         /**
          * @brief Constructor.
          * @param pOwner Owner window.
          */
         SAnimationHandler(SWindow *pOwner);
 
         /**
          * @brief Called when animation starts.
          */
         void OnAnimationStart();
 
         /**
          * @brief Called when animation stops.
          */
         void OnAnimationStop();
 
         /**
          * @brief Gets the transformation.
          * @return Transformation object.
          */
         const STransformation &GetTransformation() const;
 
         /**
          * @brief Gets the fill after flag.
          * @return Fill after flag.
          */
         bool getFillAfter() const;
 
     public:
         STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;
 
     protected:
         /**
          * @brief Called when the owner window is resized.
          * @param e Event arguments.
          * @return TRUE if handled, FALSE otherwise.
          */
         BOOL OnOwnerResize(IEvtArgs *e);
     };
 
 public:
     /**
      * @brief Constructor.
      */
     SWindow();
 
     /**
      * @brief Destructor.
      */
     virtual ~SWindow();
 
 public:
     /**
      * @brief Checks if the message is handled.
      * @return TRUE if the message is handled, FALSE otherwise.
      */
     BOOL IsMsgHandled() const;
 
     /**
      * @brief Sets the message handled flag.
      * @param bHandled Message handled flag.
      */
     void SetMsgHandled(BOOL bHandled);
 
 public:
     /**
      * @brief Gets the XML text from a node.
      * @param xmlNode XML node.
      * @return XML text.
      */
     static SStringW GetXmlText(const SXmlNode &xmlNode);
 
 public:
     STDMETHOD_(void, OnFinalRelease)(THIS);
 
     STDMETHOD_(SWND, GetSwnd)(THIS) SCONST OVERRIDE;
 
     STDMETHOD_(ILayout *, GetLayout)(THIS) OVERRIDE {
         return m_pLayout;
     }
 
     STDMETHOD_(ILayoutParam *, GetLayoutParam)(THIS) SCONST OVERRIDE {
         return m_pLayoutParam;
     }
 
     STDMETHOD_(BOOL, SetLayoutParam)(THIS_ ILayoutParam *pLayoutParam) OVERRIDE;
 
     STDMETHOD_(BOOL, IsFloat)(THIS) SCONST OVERRIDE;
 
     STDMETHOD_(BOOL, IsDisplay)(THIS) SCONST OVERRIDE;
 
     STDMETHOD_(void, SetWindowText)(THIS_ LPCTSTR lpszText) OVERRIDE;
 
     STDMETHOD_(void, SetWindowTextU8)(THIS_ LPCSTR lpszText) OVERRIDE;
 
     STDMETHOD_(void, SetToolTipText)(THIS_ LPCTSTR pszText) OVERRIDE;
     STDMETHOD_(void, SetToolTipTextU8)(THIS_ LPCSTR pszText) OVERRIDE;
 
     STDMETHOD_(BOOL, IsChecked)(THIS) SCONST OVERRIDE;
     STDMETHOD_(void, SetCheck)(THIS_ BOOL bCheck) OVERRIDE;
 
     STDMETHOD_(BOOL, IsDisabled)(THIS_ BOOL bCheckParent DEF_VAL(FALSE)) SCONST OVERRIDE;
 
     STDMETHOD_(void, EnableWindow)(THIS_ BOOL bEnable, BOOL bUpdate DEF_VAL(FALSE)) OVERRIDE;
 
     STDMETHOD_(BOOL, IsVisible)(THIS_ BOOL bCheckParent DEF_VAL(FALSE)) SCONST OVERRIDE;
 
     STDMETHOD_(void, SetVisible)(THIS_ BOOL bVisible, BOOL bUpdate DEF_VAL(FALSE)) OVERRIDE;
 
     STDMETHOD_(BOOL, IsMsgTransparent)(THIS) SCONST OVERRIDE;
 
     STDMETHOD_(ULONG_PTR, GetUserData)(THIS) SCONST OVERRIDE;
     STDMETHOD_(ULONG_PTR, SetUserData)(THIS_ ULONG_PTR uData) OVERRIDE;
     STDMETHOD_(void, GetWindowRect)(THIS_ LPRECT prect) SCONST OVERRIDE;
     STDMETHOD_(BOOL, IsVideoCanvas)(CTHIS) SCONST OVERRIDE;
     STDMETHOD_(void, GetVisibleRect)(CTHIS_ LPRECT prect) SCONST OVERRIDE;
     STDMETHOD_(void, GetClientRect)(THIS_ LPRECT prect) SCONST OVERRIDE;
     STDMETHOD_(BOOL, IsContainPoint)(THIS_ POINT pt, BOOL bClientOnly) SCONST OVERRIDE;
     STDMETHOD_(void, DoColorize)(THIS_ COLORREF cr) OVERRIDE;
     STDMETHOD_(COLORREF, GetColorizeColor)(THIS) SCONST OVERRIDE;
     STDMETHOD_(BOOL, Destroy)(THIS) OVERRIDE;
     STDMETHOD_(void, BringWindowToTop)(THIS) OVERRIDE;
     STDMETHOD_(UINT, GetChildrenCount)(THIS) SCONST OVERRIDE;
     STDMETHOD_(LRESULT, SSendMessage)
     (THIS_ UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0, BOOL *pbMsgHandled DEF_VAL(NULL)) OVERRIDE;
     STDMETHOD_(void, SDispatchMessage)
     (THIS_ UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0) OVERRIDE;
 
     STDMETHOD_(void, SetFocus)(THIS) OVERRIDE;
     STDMETHOD_(void, KillFocus)(THIS) OVERRIDE;
     STDMETHOD_(BOOL, IsFocused)(THIS) SCONST OVERRIDE;
 
     STDMETHOD_(void, Invalidate)(THIS) OVERRIDE;
     STDMETHOD_(void, InvalidateRect)(THIS_ LPCRECT lprect) OVERRIDE;
 
     STDMETHOD_(void, LockUpdate)(THIS) OVERRIDE;
     STDMETHOD_(void, UnlockUpdate)(THIS) OVERRIDE;
     STDMETHOD_(BOOL, IsUpdateLocked)(CTHIS_ BOOL bCheckParent DEF_VAL(FALSE)) SCONST OVERRIDE;
     STDMETHOD_(void, Update)(THIS_ BOOL bForce DEF_VAL(FALSE)) OVERRIDE;
     STDMETHOD_(void, Move)(THIS_ LPCRECT prect) OVERRIDE;
     STDMETHOD_(void, SetWindowRgn)(THIS_ IRegionS *pRgn, BOOL bRedraw DEF_VAL(TRUE)) OVERRIDE;
     STDMETHOD_(IRegionS *, GetWindowRgn)(THIS) SCONST OVERRIDE;
     STDMETHOD_(void, SetWindowPath)(THIS_ IPathS *pPath, BOOL bRedraw DEF_VAL(TRUE)) OVERRIDE;
     STDMETHOD_(IPathS *, GetWindowPath)(THIS) SCONST OVERRIDE;
 
     STDMETHOD_(BOOL, SetTimer)(THIS_ char id, UINT uElapse) OVERRIDE;
     STDMETHOD_(BOOL, KillTimer)(THIS_ char id) OVERRIDE;
 
     STDMETHOD_(SWND, GetCapture)(THIS) SCONST OVERRIDE;
     STDMETHOD_(SWND, SetCapture)(THIS) OVERRIDE;
     STDMETHOD_(BOOL, ReleaseCapture)(THIS) OVERRIDE;
 
     STDMETHOD_(void, SetAnimation)(THIS_ IAnimation *animation) OVERRIDE;
 
     STDMETHOD_(IAnimation *, GetAnimation)(THIS) SCONST OVERRIDE;
 
     STDMETHOD_(void, StartAnimation)(THIS_ IAnimation *animation) OVERRIDE;
 
     STDMETHOD_(void, ClearAnimation)(THIS) OVERRIDE;
 
     STDMETHOD_(void, SetAlpha)(THIS_ BYTE byAlpha) OVERRIDE;
 
     STDMETHOD_(BYTE, GetAlpha)(THIS) SCONST OVERRIDE;
 
     STDMETHOD_(void, SetMatrix)(THIS_ const IMatrix *mtx) OVERRIDE;
 
     STDMETHOD_(void, GetMatrix)(THIS_ IMatrix *mtx) SCONST OVERRIDE;
 
     STDMETHOD_(int, GetScale)(THIS) SCONST OVERRIDE;
 
     STDMETHOD_(BOOL, IsSiblingsAutoGroupped)(THIS) SCONST OVERRIDE;
 
     STDMETHOD_(void, RequestRelayout)(THIS) OVERRIDE;
     STDMETHOD_(void, UpdateLayout)(THIS) OVERRIDE;
 
     STDMETHOD_(UINT, OnGetDlgCode)(THIS) SCONST OVERRIDE;
 
     STDMETHOD_(BOOL, IsFocusable)(THIS) SCONST OVERRIDE;
 
     STDMETHOD_(BOOL, IsClipClient)(THIS) SCONST OVERRIDE;
     STDMETHOD_(BOOL, IsLayoutDirty)(THIS) SCONST OVERRIDE;
 
     STDMETHOD_(IWindow *, GetNextLayoutIChild)(THIS_ const IWindow *pCurChild) SCONST OVERRIDE;
 
     STDMETHOD_(void, UpdateChildrenPosition)(THIS) OVERRIDE;
 
     STDMETHOD_(IWindow *, GetIWindow)(THIS_ int uCode) SCONST OVERRIDE;
 
     STDMETHOD_(IWindow *, GetIChild)(THIS_ int iChild) SCONST OVERRIDE;
 
     STDMETHOD_(IWindow *, GetIParent)(THIS) SCONST OVERRIDE;
 
     STDMETHOD_(IWindow *, GetIRoot)(THIS) SCONST OVERRIDE;
 
     STDMETHOD_(BOOL, IsDescendant)(THIS_ const IWindow *pWnd) SCONST OVERRIDE;
 
     STDMETHOD_(BOOL, AdjustIZOrder)(THIS_ IWindow *pInsertAfter) OVERRIDE;
 
     STDMETHOD_(void, InsertIChild)
     (THIS_ IWindow *pNewChild, IWindow *pInsertAfter = ICWND_LAST) OVERRIDE;
 
     STDMETHOD_(BOOL, RemoveIChild)(THIS_ IWindow *pChild) OVERRIDE;
 
     STDMETHOD_(BOOL, DestroyIChild)(THIS_ IWindow *pChild) OVERRIDE;
 
     STDMETHOD_(void, DestroyAllChildren)(THIS) OVERRIDE;
 
     STDMETHOD_(IWindow *, FindIChildByID)(THIS_ int nId) OVERRIDE;
 
     STDMETHOD_(IWindow *, FindIChildByName)(THIS_ LPCWSTR pszName) OVERRIDE;
 
     STDMETHOD_(IWindow *, FindIChildByNameA)(THIS_ LPCSTR pszName) OVERRIDE;
 
     STDMETHOD_(ISwndContainer *, GetContainer)(THIS) OVERRIDE;
     STDMETHOD_(void, SetContainer)(THIS_ ISwndContainer *pContainer) OVERRIDE;
 
     STDMETHOD_(void, GetChildrenLayoutRect)(THIS_ RECT *prc) SCONST OVERRIDE;
 
     STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz, int nParentWid, int nParentHei) OVERRIDE;
 
     STDMETHOD_(void, Move2)(THIS_ int x, int y, int cx DEF_VAL(-1), int cy DEF_VAL(-1)) OVERRIDE;
 
     /**
      * @brief Retrieves the window text.
      * @param pBuf Buffer to receive the window text.
      * @param nBufLen Length of the buffer.
      * @param bRawText Flag to indicate if raw text should be retrieved.
      * @return Length of the text copied to the buffer.
      */
     STDMETHOD_(int, GetWindowText)(THIS_ TCHAR *pBuf, int nBufLen, BOOL bRawText) OVERRIDE;
 
     /**
      * @brief Retrieves the window text as a UTF-8 string.
      * @param pStr String object to receive the window text.
      * @param bRawText Flag to indicate if raw text should be retrieved.
      * @return Length of the text copied to the string object.
      */
     STDMETHOD_(int, GetWindowTextU8)(THIS_ IStringA *pStr, BOOL bRawText) OVERRIDE;
 
     /**
      * @brief Sets the event mute state.
      * @param bMute Flag to indicate if events should be muted.
      */
     STDMETHOD_(void, SetEventMute)(THIS_ BOOL bMute) OVERRIDE;
 
     /**
      * @brief Retrieves the current state of the window.
      * @return Current state flags.
      */
     STDMETHOD_(DWORD, GetState)(THIS) SCONST OVERRIDE;
 
     /**
      * @brief Modifies the state of the window.
      * @param dwStateAdd State flags to add.
      * @param dwStateRemove State flags to remove.
      * @param bUpdate Flag to indicate if the state change should be updated.
      * @return Modified state flags.
      */
     STDMETHOD_(DWORD, ModifyState)
     (THIS_ DWORD dwStateAdd, DWORD dwStateRemove, BOOL bUpdate DEF_VAL(FALSE)) OVERRIDE;
 
     /**
      * @brief Sets the owner window.
      * @param pOwner Pointer to the owner window.
      */
     STDMETHOD_(void, SetIOwner)(THIS_ IWindow *pOwner) OVERRIDE;
 
     /**
      * @brief Retrieves the owner window.
      * @return Pointer to the owner window.
      */
     STDMETHOD_(IWindow *, GetIOwner)(THIS) SCONST OVERRIDE;
 
     /**
      * @brief Creates child windows from XML.
      * @param pszXml XML string containing child window definitions.
      * @return TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, CreateChildrenFromXml)(THIS_ LPCWSTR pszXml) OVERRIDE;
 
     /**
      * @brief Creates child windows from a resource ID.
      * @param pszResId Resource ID of the XML string containing child window definitions.
      * @return TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, CreateChildrenFromResId)(THIS_ LPCTSTR pszResId) OVERRIDE;
 
     /**
      * @brief Initializes the window from an XML node.
      * @param pNode XML node containing window initialization data.
      * @return TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode *pNode) OVERRIDE;
 
     /**
      * @brief Retrieves an attribute value from the window.
      * @param pszName Name of the attribute.
      * @param strValue String object to receive the attribute value.
      * @return TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, GetAttribute)(THIS_ LPCWSTR pszName, IStringW *strValue) SCONST OVERRIDE;
 
     /**
      * @brief Retrieves the background color of the window.
      * @return Background color.
      */
     STDMETHOD_(COLORREF, GetBkgndColor)(THIS) SCONST OVERRIDE;
 
     /**
      * @brief Retrieves the selected sibling window in a group.
      * @return Pointer to the selected sibling window, or NULL if not in a group.
      */
     STDMETHOD_(IWindow *, GetISelectedSiblingInGroup)(THIS) OVERRIDE;
 
     /**
      * @brief Retrieves the selected child window in a group.
      * @return Pointer to the selected child window, or NULL if not in a group.
      */
     STDMETHOD_(IWindow *, GetISelectedChildInGroup)(THIS) OVERRIDE;
 
     /**
      * @brief Retrieves the window handle at a specified point.
      * @param pt Point to test.
      * @param bIncludeMsgTransparent Flag to include message-transparent windows.
      * @return Window handle at the specified point.
      */
     STDMETHOD_(SWND, SwndFromPoint)(THIS_ POINT *pt, BOOL bIncludeMsgTransparent DEF_VAL(FALSE)) SCONST OVERRIDE;
 
     /**
      * @brief Fires an event.
      * @param evt Event arguments.
      * @return TRUE if the event was handled, FALSE otherwise.
      */
     STDMETHOD_(BOOL, FireEvent)(THIS_ IEvtArgs *evt) OVERRIDE;
 
     /**
      * @brief Fires a command event.
      * @return TRUE if the command was handled, FALSE otherwise.
      */
     STDMETHOD_(BOOL, FireCommand)(THIS) OVERRIDE;
 
     /**
      * @brief Fires a context menu event.
      * @param pt Point where the context menu should be displayed.
      * @return TRUE if the context menu was handled, FALSE otherwise.
      */
     STDMETHOD_(BOOL, FireCtxMenu)(THIS_ POINT pt) OVERRIDE;
 
     /**
      * @brief Subscribes to an event.
      * @param evtId Event ID.
      * @param pSlot Event slot.
      * @return TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, SubscribeEvent)(THIS_ DWORD evtId, const IEvtSlot *pSlot) OVERRIDE;
 
     /**
      * @brief Unsubscribes from an event.
      * @param evtId Event ID.
      * @param pSlot Event slot.
      * @return TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, UnsubscribeEvent)(THIS_ DWORD evtId, const IEvtSlot *pSlot) OVERRIDE;
 
     /**
      * @brief Queries an interface.
      * @param id Interface ID.
      * @param ppRet Pointer to receive the interface pointer.
      * @return HRESULT indicating success or failure.
      */
     STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef **ppRet) OVERRIDE;
 
     /**
      * @brief Adds an event handler.
      * @param dwEventID Event ID.
      * @param pszEventHandlerName Event handler name.
      * @return TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, AddEvent)(THIS_ DWORD dwEventID, LPCWSTR pszEventHandlerName) OVERRIDE;
 
     /**
      * @brief Removes an event handler.
      * @param dwEventID Event ID.
      * @return TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, RemoveEvent)(THIS_ DWORD dwEventID) OVERRIDE;
 
     /**
      * @brief Processes a window message.
      * @param uMsg Message identifier.
      * @param wParam Additional message-specific information.
      * @param lParam Additional message-specific information.
      * @param lResult Pointer to receive the result of the message processing.
      * @return TRUE if the message was handled, FALSE otherwise.
      */
     STDMETHOD_(BOOL, SwndProc)(THIS_ UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult) OVERRIDE;
 
     /**
      * @brief Sets the window procedure.
      * @param swndProc Window procedure function.
      */
     STDMETHOD_(void, SetSwndProc)(THIS_ FunSwndProc swndProc) OVERRIDE;
 
     /**
      * @brief Retrieves the host window handle.
      * @return Host window handle.
      */
     STDMETHOD_(HWND, GetHostHwnd)(THIS) OVERRIDE;
 
     /**
      * @brief Retrieves the timeline handlers manager.
      * @return Pointer to the timeline handlers manager.
      */
     STDMETHOD_(ITimelineHandlersMgr *, GetTimelineHandlersMgr)(THIS) OVERRIDE;
 
     /**
      * @brief Registers a drop target for the window.
      * @param pDragTarget Pointer to the drop target.
      * @return TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, RegisterDragDrop)(THIS_ IDropTarget *pDragTarget) OVERRIDE;
 
     /**
      * @brief Unregisters a drop target for the window.
      * @return TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, UnregisterDragDrop)(THIS) OVERRIDE;
 
   public: // caret相关方法
     /**
      * @brief Creates a caret.
      * @param pBmp Bitmap for the caret.
      * @param nWid Width of the caret.
      * @param nHeight Height of the caret.
      * @return TRUE if successful, FALSE otherwise.
      */
     STDMETHOD_(BOOL, CreateCaret)(THIS_ HBITMAP pBmp, int nWid, int nHeight) OVERRIDE;
 
     /**
      * @brief Shows or hides the caret.
      * @param bShow Flag to indicate if the caret should be shown.
      */
     STDMETHOD_(void, ShowCaret)(THIS_ BOOL bShow) OVERRIDE;
 
     /**
      * @brief Sets the caret position.
      * @param x X-coordinate of the caret position.
      * @param y Y-coordinate of the caret position.
      */
     STDMETHOD_(void, SetCaretPos)(THIS_ int x, int y) OVERRIDE;
 
   public:
 #ifdef _WIN32
     /**
      * @brief Retrieves the accessible object for the window.
      * @return Pointer to the accessible object.
      */
     IAccessible *GetAccessible();
 #endif
     /**
      * @brief Retrieves the accessibility proxy for the window.
      * @return Pointer to the accessibility proxy.
      */
     IAccProxy *GetAccProxy();
 
     /**
      * @brief Notifies an accessibility event.
      * @param dwEvt Event ID.
      */
     void accNotifyEvent(DWORD dwEvt);
 
   public:
     /**
      * @brief Retrieves the current message being processed.
      * @return PSWNDMSG Pointer to the current message structure.
      */
     PSWNDMSG GetCurMsg(void) const
     {
         return m_pCurMsg;
     }
 
     /**
      * @brief Retrieves the container associated with this window.
      * @return const ISwndContainer* Pointer to the container object.
      */
     const ISwndContainer *GetContainer() const;
 
     /**
      * @brief Retrieves a window based on a given code.
      * @param uCode Code specifying the type of window to retrieve (e.g., first child, last child).
      * @return SWindow* Pointer to the requested window.
      */
     SWindow *GetWindow(int uCode) const;
 
     /**
      * @brief Retrieves a child window by index.
      * @param iChild Index of the child window.
      * @return SWindow* Pointer to the child window.
      */
     SWindow *GetChild(int iChild) const;
 
     /**
      * @brief Retrieves the parent window.
      * @return SWindow* Pointer to the parent window.
      */
     SWindow *GetParent() const;
 
     /**
      * @brief Retrieves the root window in the hierarchy.
      * @return SWindow* Pointer to the root window.
      */
     SWindow *GetRoot() const;
 
     /**
      * @brief Retrieves the next layout child after a specified child.
      * @param pCurChild Pointer to the current child window.
      * @return SWindow* Pointer to the next layout child.
      */
     SWindow *GetNextLayoutChild(const SWindow *pCurChild) const;
 
     /**
      * @brief Adjusts the Z-order of the window.
      * @param pInsertAfter Pointer to the window after which this window should be inserted.
      * @return BOOL TRUE if successful; otherwise, FALSE.
      * @details If `pInsertAfter` is not a sibling of this window, the function will fail.
      */
     BOOL AdjustZOrder(SWindow *pInsertAfter);
 
     /**
      * @brief Inserts a child window into the window tree.
      * @param pNewChild Pointer to the new child window.
      * @param pInsertAfter Pointer to the window after which the new child should be inserted.
      * @details The new child will not automatically enter the layout process.
      */
     void InsertChild(SWindow *pNewChild, SWindow *pInsertAfter = ICWND_LAST);
 
     /**
      * @brief Removes a child window from the window tree.
      * @param pChild Pointer to the child window to remove.
      * @return BOOL TRUE if successful; otherwise, FALSE.
      * @details The removed child will not be automatically released.
      */
     BOOL RemoveChild(SWindow *pChild);
 
     /**
      * @brief Destroys a child window.
      * @param pChild Pointer to the child window to destroy.
      * @return BOOL TRUE if successful; otherwise, FALSE.
      * @details Calls `RemoveChild` and then releases the child window.
      */
     BOOL DestroyChild(SWindow *pChild);
 
   public:
     /**
      * @brief Sets the transformation matrix for the window.
      * @param mtx Reference to the transformation matrix.
      */
     void SetMatrix(const SMatrix &mtx);
 
     /**
      * @brief Retrieves the text of the window.
      * @param bRawText Flag indicating whether to retrieve raw text.
      * @return SStringT Window text as a string.
      */
     virtual SStringT GetWindowText(BOOL bRawText = FALSE);
 
     /**
      * @brief Retrieves the tooltip text of the window.
      * @return SStringT Tooltip text as a string.
      */
     virtual SStringT GetToolTipText();
 
     /**
      * @brief Retrieves the event set associated with the window.
      * @return SEventSet* Pointer to the event set object.
      */
     SEventSet *GetEventSet()
     {
         return &m_evtSet;
     }
 
     /**
      * @brief Retrieves the style of the window.
      * @return SwndStyle& Reference to the style object.
      */
     const SwndStyle &GetStyle() const;
     SwndStyle &GetStyle();
 
     /**
      * @brief Sets the owner of the window.
      * @param pOwner Pointer to the new owner window.
      */
     void SetOwner(SWindow *pOwner);
 
     /**
      * @brief Retrieves the current owner of the window.
      * @return SWindow* Pointer to the owner window.
      */
     SWindow *GetOwner() const;
 
     /**
      * @brief Retrieves the text alignment of the window.
      * @return UINT Text alignment flags.
      */
     UINT GetTextAlign() const;
 
     /**
      * @brief Retrieves the bounding rectangle of the window.
      * @return CRect Bounding rectangle of the window.
      */
     CRect GetWindowRect() const;
 
     /**
      * @brief Retrieves the client rectangle of the window.
      * @return CRect Client rectangle of the window.
      */
     virtual CRect GetClientRect() const;
 
   public: // Window tree structure-related methods
     /**
      * @brief Finds a child window by its ID.
      * @param nID ID of the child window to find.
      * @param nDeep Depth of the search (-1 for unlimited depth).
      * @return SWindow* Pointer to the found child window.
      */
     SWindow *FindChildByID(int nID, int nDeep = -1);
 
     /**
      * @brief Template method to cast layout parameters to a specific type.
      * @tparam T Type of the layout parameter.
      * @return T* Pointer to the casted layout parameter.
      */
     template <class T>
     T *GetLayoutParamT()
     {
         return sobj_cast<T>(GetLayoutParam());
     }
 
     /**
      * @brief Finds a child window by its ID and casts it to a specific type.
      * @tparam T Type of the child window.
      * @param nID ID of the child window to find.
      * @param nDeep Depth of the search (-1 for unlimited depth).
      * @return T* Pointer to the found child window casted to type T.
      */
     template <class T>
     T *FindChildByID2(int nID, int nDeep = -1)
     {
         SWindow *pTarget = FindChildByID(nID, nDeep);
 
         if (!pTarget || !pTarget->IsClass(T::GetClassName()))
         {
             SSLOGW() << "FindChildByID2 Failed, no window of class " << T::GetClassName() << " with id of " << nID << " was found within " << nDeep << " levels";
             return NULL;
         }
         return (T *)pTarget;
     }
 
     /**
      * @brief Finds a child window by its name.
      * @param strName Name of the child window to find.
      * @param nDeep Depth of the search (-1 for unlimited depth).
      * @return SWindow* Pointer to the found child window.
      */
     SWindow *FindChildByName(LPCWSTR strName, int nDeep = -1);
 
     /**
      * @brief Overloaded method to find a child window by its name (ANSI version).
      * @param strName Name of the child window to find.
      * @param nDeep Depth of the search (-1 for unlimited depth).
      * @return SWindow* Pointer to the found child window.
      */
     SWindow *FindChildByName(LPCSTR strName, int nDeep = -1);
 
     /**
      * @brief Finds a child window by its name and casts it to a specific type.
      * @tparam T Type of the child window.
      * @param pszName Name of the child window to find.
      * @param nDeep Depth of the search (-1 for unlimited depth).
      * @return T* Pointer to the found child window casted to type T.
      */
     template <class T>
     T *FindChildByName2(LPCWSTR pszName, int nDeep = -1)
     {
         SWindow *pTarget = FindChildByName(pszName, nDeep);
         if (!pTarget || !pTarget->IsClass(T::GetClassName()))
         {
             SSLOGW() << "FindChildByName2 Failed, no window of class " << T::GetClassName() << " with name of " << pszName << " was found within " << nDeep << " levels";
             return NULL;
         }
         return (T *)pTarget;
     }
 
     /**
      * @brief Overloaded method to find a child window by its name and cast it to a specific type (ANSI version).
      * @tparam T Type of the child window.
      * @param pszName Name of the child window to find.
      * @param nDeep Depth of the search (-1 for unlimited depth).
      * @return T* Pointer to the found child window casted to type T.
      */
     template <class T>
     T *FindChildByName2(LPCSTR pszName, int nDeep = -1)
     {
         return FindChildByName2<T>(S_CA2W(pszName), nDeep);
     }
 
     /**
      * @brief Recursively finds a child window by its class type.
      * @tparam T Type of the child window.
      * @param nDeep Depth of the search (-1 for unlimited depth).
      * @return T* Pointer to the found child window casted to type T.
      */
     template <class T>
     T *FindChildByClass(int nDeep = -1) const
     {
         SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
         while (pChild)
         {
             if (pChild->IsClass(T::GetClassName()))
                 return (T *)pChild;
             pChild = pChild->GetWindow(GSW_NEXTSIBLING);
         }
 
         if (nDeep > 0)
             nDeep--;
         if (nDeep == 0)
             return NULL;
 
         pChild = GetWindow(GSW_FIRSTCHILD);
         while (pChild)
         {
             SWindow *pChildFind = pChild->FindChildByClass<T>(nDeep);
             if (pChildFind)
                 return (T *)pChildFind;
             pChild = pChild->GetWindow(GSW_NEXTSIBLING);
         }
 
         return NULL;
     }
 
 // Protected methods for internal use
 protected:
     // Get script module interface
     IScriptModule *GetScriptModule();
     
     // Invalidate a specific rectangle area of the window
     void InvalidateRect(const CRect &rect, BOOL bFromThis = TRUE, BOOL bClip = FALSE);
     
     // Get current transformation matrix
     STransformation GetTransformation() const;
     
     // Create child window from XML node
     BOOL CreateChild(SXmlNode xmlChild);
     
     // Create multiple child windows from XML nodes
     void CreateChilds(SXmlNode xmlNode);
 
 // Animation related callback functions
 protected:
     STDMETHOD_(void, OnAnimationStart)(THIS_ IAnimation *animation);  // Called when animation starts
     STDMETHOD_(void, OnAnimationStop)(THIS_ IAnimation *animation);   // Called when animation stops
     STDMETHOD_(void, OnAnimationRepeat)(THIS_ IAnimation *animation); // Called when animation repeats
     STDMETHOD_(void, OnAnimationPauseChange)(THIS_ IAnimation *animation, BOOL bPaused); // Called on pause state change
 
 // Virtual functions for override
 protected:
     virtual void OnAnimationInvalidate(IAnimation *pAni, bool bErase); // Called when animation needs redraw
     
     virtual void OnContentChanged(); // Called when window content changes
     
     /**
      * OoColorize
      * @brief Adjust window color tone
      */
     virtual void OnColorize(COLORREF cr);
     
     virtual BOOL OnSetCursor(const CPoint &pt); // Set cursor when hover
     
     virtual void OnStateChanging(DWORD dwOldState, DWORD dwNewState); // Called before state changes
     virtual void OnStateChanged(DWORD dwOldState, DWORD dwNewState);  // Called after state changes
     
     virtual void OnCaptureChanged(BOOL bCaptured); // Called when capture state changes
     
     /**
      * OnRelayout
      * @brief Handle window position changes
      */
     virtual BOOL OnRelayout(const CRect &rcWnd);
 
 // Public virtual functions
 public: 
     virtual SIZE MeasureContent(int nParentWid, int nParentHei); // Measure content size
     virtual SIZE MeasureChildren(int nParentWid, int nParentHei); // Measure children size
     
     /**
      * OnUpdateToolTip
      * @brief Handle tooltip updates
      */
     virtual BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo);
     
     /**
      * GetSelectedSiblingInGroup
      * @brief Get selected sibling in group
      */
     virtual SWindow *GetSelectedSiblingInGroup()
     {
         return NULL;
     }
     
     /**
      * GetSelectedChildInGroup
      * @brief Get selected child in group
      */
     virtual SWindow *GetSelectedChildInGroup();
 
     /**
      * CreateChildren
      * @brief Create child windows from XML node
      */
     virtual BOOL CreateChildren(SXmlNode xmlNode);
     
     virtual SWindow *CreateChildByName(LPCWSTR pszName); // Create child by name
     
     /**
      * RequestRelayout
      * @brief Request layout update
      */
     virtual void RequestRelayout(SWND hSource, BOOL bSourceResizable);
     
     virtual SStringW tr(const SStringW &strSrc) const; // Translation function
     
     virtual SWND SwndFromPoint(CPoint &pt, BOOL bIncludeMsgTransparent = false) const; // Get window from point
     
     virtual BOOL OnNcHitTest(CPoint pt); // Non-client area hit test
     
     /**
      * OnUpdateFloatPosition
      * @brief Update floating window position
      */
     virtual void OnUpdateFloatPosition(const CRect &rcParent){};
     
     /**
      * NeedRedrawWhenStateChange
      * @brief Determine if redraw is needed on state change
      */
     virtual BOOL NeedRedrawWhenStateChange();
     
     /**
      * GetTextRect
      * @brief Calculate text display rectangle
      */
     virtual void GetTextRect(LPRECT pRect);
     
     /**
      * DrawText
      * @brief Draw text content
      */
     virtual void DrawText(IRenderTarget *pRT, LPCTSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat);
     
     /**
      * DrawFocus
      * @brief Draw focus state
      */
     virtual void DrawFocus(IRenderTarget *pRT);
     
     /**
      * BeforePaint
      * @brief Prepare rendering environment
      */
     virtual void BeforePaint(IRenderTarget *pRT, SPainter &painter);
     
     /**
      * AfterPaint
      * @brief Restore rendering environment
      */
     virtual void AfterPaint(IRenderTarget *pRT, SPainter &painter);
     
     /**
      * GetTrCtx
      * @brief Get translation context
      */
     virtual LPCWSTR GetTrCtx() const;
 
 // Public methods related to rendering
 public:
     /**
      * RedrawRegion
      * @brief    Renders the content of the window and its child windows onto the RenderTarget.
      * @param    IRenderTarget * pRT -- Target RenderTarget.
      * @param    IRegion * pRgn -- Region to render; NULL renders the entire window.
      * @return   void
      *
      * Describe  This method redraws the specified region or the entire window if no region is provided.
      */
     void RedrawRegion(IRenderTarget *pRT, IRegionS *pRgn);
 
     /**
      * GetRenderTarget
      * @brief    Retrieves a memory DC compatible with the SWND window.
      * @param    LPCRECT pRc -- Target rectangle for the RenderTarget.
      * @param    DWORD gdcFlags -- Flags similar to OLEDCFLAGS.
      * @param    BOOL bClientRT -- Indicates whether the client area should be used.
      * @return   IRenderTarget *
      *
      * Describe  The returned RenderTarget must be released using ReleaseRenderTarget.
      */
     IRenderTarget *GetRenderTarget(LPCRECT pRc = NULL, GrtFlag gdcFlags = GRT_NODRAW, BOOL bClientRT = TRUE);
 
     /**
      * GetRenderTarget
      * @brief    Retrieves a memory DC compatible with the SWND window.
      * @param    DWORD gdcFlags -- Flags similar to OLEDCFLAGS.
      * @param    IRegion *pRgn -- Target region for the RenderTarget.
      * @return   IRenderTarget *
      *
      * Describe  The returned RenderTarget must be released using ReleaseRenderTarget.
      */
     IRenderTarget *GetRenderTarget(GrtFlag gdcFlags, IRegionS *pRgn);
 
     /**
      * ReleaseRenderTarget
      * @brief    Releases the RenderTarget obtained via GetRenderTarget.
      * @param    IRenderTarget * pRT -- RenderTarget to release.
      * @return   void
      *
      * Describe  This method ensures proper cleanup of the RenderTarget resources.
      */
     void ReleaseRenderTarget(IRenderTarget *pRT);
 
     /**
      * PaintBackground
      * @brief    Draws the background content of the window.
      * @param    IRenderTarget * pRT -- Target RenderTarget.
      * @param    LPRECT pRc -- Target rectangle within the window.
      * @return   void
      *
      * Describe  The target rectangle must lie within the window's bounds.
      */
     void PaintBackground(IRenderTarget *pRT, LPRECT pRc);
 
     /**
      * PaintForeground
      * @brief    Draws the foreground content of the window.
      * @param    IRenderTarget * pRT -- Target RenderTarget.
      * @param    LPRECT pRc -- Target rectangle within the window.
      * @param    SWindow *pStartFrom -- Starting window for drawing; defaults to root.
      * @return   void
      *
      * Describe  The target rectangle must lie within the window's bounds, excluding child windows.
      */
     void PaintForeground(IRenderTarget *pRT, LPRECT pRc, SWindow *pStartFrom = NULL);
 
     /**
      * BeforePaintEx
      * @brief    Prepares the drawing environment for the current window's RenderTarget, starting from the top-level window.
      * @param    IRenderTarget * pRT -- Target RenderTarget.
      * @return   void
      *
      * Describe  Typically used in conjunction with CreateRenderTarget.
      */
     void BeforePaintEx(IRenderTarget *pRT);
 
     /**
      * TransformPoint
      * @brief    Transforms a point based on the current window's transformation matrix.
      * @param    CPoint &pt -- Point to transform.
      * @return   void
      *
      * Describe  Applies the transformation matrix to the point.
      */
     void TransformPoint(CPoint &pt) const;
 
     /**
      * TransformPointEx
      * @brief    Extends the transformation of a point.
      * @param    CPoint &pt -- Point to transform.
      * @return   void
      *
      * Describe  Provides additional transformations beyond the basic TransformPoint.
      */
     void TransformPointEx(CPoint &pt) const;
 
     /**
      * FireEvent
      * @brief    Fires an event.
      * @param    SEvtArgs &evt -- Event arguments.
      * @return   BOOL -- Result of firing the event.
      *
      * Describe  Simplifies the process of firing events by wrapping the pointer-based method.
      */
     BOOL FireEvent(SEvtArgs &evt)
     {
         return FireEvent(&evt);
     }
 
 // Protected methods related to caching
 protected:
     /**
      * IsCacheDirty
      * @brief    Checks if the cache is marked as dirty.
      * @return   bool -- True if the cache is dirty.
      *
      * Describe  Determines whether the cache needs to be refreshed.
      */
     bool IsCacheDirty() const;
 
     /**
      * MarkCacheDirty
      * @brief    Marks the cache as dirty.
      * @param    bool bDirty -- Dirty state to set.
      * @return   void
      *
      * Describe  Sets the cache's dirty flag to indicate it needs updating.
      */
     void MarkCacheDirty(bool bDirty);
 
     /**
      * IsDrawToCache
      * @brief    Checks if the window content is being drawn to the cache.
      * @return   bool -- True if drawing to cache.
      *
      * Describe  Indicates whether the current rendering operation targets the cache.
      */
     virtual bool IsDrawToCache() const;
 
     /**
      * IsLayeredWindow
      * @brief    Determines if the window requires a render layer.
      * @return   BOOL -- TRUE if a render layer is needed.
      *
      * Describe  Indicates whether the window uses layered rendering.
      */
     virtual BOOL IsLayeredWindow() const;
 
     /**
      * DispatchPaint
      * @brief    Handles paint dispatching for the window.
      * @param    IRenderTarget * pRT -- Target RenderTarget.
      * @param    IRegion *pRgn -- Paint region.
      * @param    UINT iBeginZorder -- Beginning Z-order.
      * @param    UINT iEndZorder -- Ending Z-order.
      * @return   void
      *
      * Describe  Manages the painting process for the specified Z-order range.
      */
     virtual void DispatchPaint(IRenderTarget *pRT, IRegionS *pRgn, UINT iZorderBegin, UINT iZorderEnd);
 
     /**
      * OnCommitSurface
      * @brief    Commits surface changes.
      * @param    IRenderTarget * pRtDest -- Destination RenderTarget.
      * @param    LPCRECT pRcDest -- Destination rectangle.
      * @param    IRenderTarget *pRtSrc -- Source RenderTarget.
      * @param    LPCRECT pRcSrc -- Source rectangle.
      * @param    BYTE alpha -- Alpha value for blending.
      * @return   void
      *
      * Describe  Applies surface changes with optional alpha blending.
      */
     virtual void OnCommitSurface(IRenderTarget *pRtDest, LPCRECT pRcDest, IRenderTarget *pRtSrc, LPCRECT pRcSrc, BYTE alpha);
 
 // Protected helper functions
 protected:
     /**
      * _FindChildByID
      * @brief    Finds a child window by ID.
      * @param    int nID -- Child window ID.
      * @param    int nDeep -- Search depth; -1 for unlimited.
      * @return   SWindow * -- Found child window.
      *
      * Describe  Searches for a child window by its unique identifier.
      */
     virtual SWindow *_FindChildByID(int nID, int nDeep);
 
     /**
      * _FindChildByName
      * @brief    Finds a child window by name.
      * @param    const SStringW &strName -- Child window name.
      * @param    int nDeep -- Search depth; -1 for unlimited.
      * @return   SWindow * -- Found child window.
      *
      * Describe  Searches for a child window by its name attribute.
      */
     virtual SWindow *_FindChildByName(const SStringW &strName, int nDeep);
 
     /**
      * _GetCurrentRenderContainer
      * @brief    Retrieves the host window for the current render layer.
      * @return   SWindow * -- Host window for the render layer.
      *
      * Describe  Identifies the container responsible for rendering the current window.
      */
     SWindow *_GetCurrentLayeredWindow();
 
     /**
      * _ApplyMatrix
      * @brief    Applies a transformation matrix to the RenderTarget.
      * @param    IRenderTarget * pRT -- Target RenderTarget.
      * @param    SMatrix &oriMtx -- Original transformation matrix.
      * @return   bool -- True if the matrix was applied successfully.
      *
      * Describe  Modifies the RenderTarget's transformation to account for the window's matrix.
      */
     bool _ApplyMatrix(IRenderTarget *pRT, SMatrix &oriMtx);
 
     /**
      * _GetMatrixEx
      * @brief    Retrieves the extended transformation matrix for the window.
      * @return   SMatrix -- Extended transformation matrix.
      *
      * Describe  Provides the complete transformation matrix for the window.
      */
     SMatrix _GetMatrixEx() const;
 
     /**
      * _WndRectInRgn
      * @brief    Checks if the window rectangle lies within a specified region.
      * @param    const CRect &rc -- Window rectangle.
      * @param    const IRegionS *rgn -- Target region.
      * @return   bool -- True if the rectangle is within the region.
      *
      * Describe  Verifies whether the window's rectangle intersects with the given region.
      */
     bool _WndRectInRgn(const CRect &rc, const IRegionS *rgn) const;
 
     // Renders the client area of the window onto the RenderTarget.
     void _PaintClient(IRenderTarget *pRT);
 
     // Renders the non-client area of the window onto the RenderTarget.
     void _PaintNonClient(IRenderTarget *pRT);
 
     // Redraws the non-client area of the window.
     void _RedrawNonClient();
 
     // Renders a specific region of the window onto the RenderTarget.
     void _PaintRegion(IRenderTarget *pRT, IRegionS *pRgn, UINT iZorderBegin, UINT iZorderEnd);
 
     // Renders child windows within a specific Z-order range.
     void _PaintChildren(IRenderTarget *pRT, IRegionS *pRgn, UINT iBeginZorder, UINT iEndZorder);
 
     // Draws the default focus rectangle.
     void DrawDefFocusRect(IRenderTarget *pRT, CRect rc);
 
     // Updates the cache mode for the window.
     void UpdateCacheMode();
 
     // Tests if the current thread is the main UI thread.
     void TestMainThread();
 
     // Retrieves a scaled skin object based on the current scale factor.
     void GetScaleSkin(SAutoRefPtr<ISkinObj> &pSkin, int nScale);
 // Protected methods for handling messages
 protected:
     /**
      * OnCreate
      * @brief    Handles the creation of the window.
      * @param    LPVOID -- Pointer to creation data.
      * @return   int -- Result of the creation process.
      *
      * Describe  This method is called when the window is created.
      */
     int OnCreate(LPVOID);
 
     /**
      * OnSize
      * @brief    Handles the resizing of the window.
      * @param    UINT nType -- Resize type.
      * @param    CSize size -- New size of the window.
      * @return   void
      *
      * Describe  This method is called when the window is resized.
      */
     void OnSize(UINT nType, CSize size);
 
     /**
      * OnDestroy
      * @brief    Handles the destruction of the window.
      * @return   void
      *
      * Describe  This method is called when the window is destroyed.
      */
     void OnDestroy();
 
     /**
      * OnEraseBkgnd
      * @brief    Handles the erasing of the background.
      * @param    IRenderTarget *pRT -- RenderTarget for drawing.
      * @return   BOOL -- TRUE if the background was erased.
      *
      * Describe  This method is called to erase the background before painting.
      */
     BOOL OnEraseBkgnd(IRenderTarget *pRT);
 
     /**
      * OnPaint
      * @brief    Handles the painting of the window.
      * @param    IRenderTarget *pRT -- RenderTarget for drawing.
      * @return   void
      *
      * Describe  This method is called to paint the window content.
      */
     void OnPaint(IRenderTarget *pRT);
 
     /**
      * OnNcPaint
      * @brief    Handles the painting of the non-client area.
      * @param    IRenderTarget *pRT -- RenderTarget for drawing.
      * @return   void
      *
      * Describe  This method is called to paint the non-client area of the window.
      */
     void OnNcPaint(IRenderTarget *pRT);
 
     /**
      * OnShowWindow
      * @brief    Handles showing or hiding the window.
      * @param    BOOL bShow -- TRUE to show, FALSE to hide.
      * @param    UINT nStatus -- Show status flags.
      * @return   void
      *
      * Describe  This method is called when the window is shown or hidden.
      */
     void OnShowWindow(BOOL bShow, UINT nStatus);
 
     /**
      * OnEnable
      * @brief    Handles enabling or disabling the window.
      * @param    BOOL bEnable -- TRUE to enable, FALSE to disable.
      * @param    UINT nStatus -- Enable status flags.
      * @return   void
      *
      * Describe  This method is called when the window is enabled or disabled.
      */
     void OnEnable(BOOL bEnable, UINT nStatus);
 
     /**
      * OnLButtonDown
      * @brief    Handles the left mouse button down event.
      * @param    UINT nFlags -- Mouse flags.
      * @param    CPoint pt -- Mouse position.
      * @return   void
      *
      * Describe  This method is called when the left mouse button is pressed.
      */
     void OnLButtonDown(UINT nFlags, CPoint pt);
 
     /**
      * OnLButtonDbClick
      * @brief    Handles the left mouse button double-click event.
      * @param    UINT nFlags -- Mouse flags.
      * @param    CPoint point -- Mouse position.
      * @return   void
      *
      * Describe  This method is called when the left mouse button is double-clicked.
      */
     void OnLButtonDbClick(UINT nFlags, CPoint point);
 
     /**
      * OnLButtonUp
      * @brief    Handles the left mouse button up event.
      * @param    UINT nFlags -- Mouse flags.
      * @param    CPoint pt -- Mouse position.
      * @return   void
      *
      * Describe  This method is called when the left mouse button is released.
      */
     void OnLButtonUp(UINT nFlags, CPoint pt);
 
     /**
      * OnRButtonDown
      * @brief    Handles the right mouse button down event.
      * @param    UINT nFlags -- Mouse flags.
      * @param    CPoint point -- Mouse position.
      * @return   void
      *
      * Describe  This method is called when the right mouse button is pressed.
      */
     void OnRButtonDown(UINT nFlags, CPoint point);
 
     /**
      * OnRButtonUp
      * @brief    Handles the right mouse button up event.
      * @param    UINT nFlags -- Mouse flags.
      * @param    CPoint point -- Mouse position.
      * @return   void
      *
      * Describe  This method is called when the right mouse button is released.
      */
     void OnRButtonUp(UINT nFlags, CPoint point);
 
     /**
      * OnMouseHover
      * @brief    Handles the mouse hover event.
      * @param    UINT nFlags -- Mouse flags.
      * @param    CPoint ptPos -- Mouse position.
      * @return   void
      *
      * Describe  This method is called when the mouse hovers over the window.
      */
     void OnMouseHover(UINT nFlags, CPoint ptPos);
 
     /**
      * OnMouseMove
      * @brief    Handles the mouse move event.
      * @param    UINT nFlags -- Mouse flags.
      * @param    CPoint pt -- Mouse position.
      * @return   void
      *
      * Describe  This method is called when the mouse moves over the window.
      */
     void OnMouseMove(UINT nFlags, CPoint pt);
 
     /**
      * OnMouseLeave
      * @brief    Handles the mouse leave event.
      * @return   void
      *
      * Describe  This method is called when the mouse leaves the window.
      */
     void OnMouseLeave();
 
     /**
      * OnMouseWheel
      * @brief    Handles the mouse wheel event.
      * @param    UINT nFlags -- Mouse flags.
      * @param    short zDelta -- Wheel delta.
      * @param    CPoint pt -- Mouse position.
      * @return   BOOL -- TRUE if the event was handled.
      *
      * Describe  This method is called when the mouse wheel is moved.
      */
     BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
 
     /**
      * OnMouseClick
      * @brief    Handles general mouse click events.
      * @param    UINT uMsg -- Message identifier.
      * @param    WPARAM wParam -- Additional message parameter.
      * @param    LPARAM lParam -- Additional message parameter.
      * @return   LRESULT -- Result of the message processing.
      *
      * Describe  This method handles a range of mouse click messages.
      */
     LRESULT OnMouseClick(UINT uMsg, WPARAM wParam, LPARAM lParam);
 
     /**
      * OnSetFocus
      * @brief    Handles gaining focus.
      * @param    SWND wndOld -- Handle of the previous focused window.
      * @return   void
      *
      * Describe  This method is called when the window gains focus.
      */
     void OnSetFocus(SWND wndOld);
 
     /**
      * OnKillFocus
      * @brief    Handles losing focus.
      * @param    SWND wndFocus -- Handle of the new focused window.
      * @return   void
      *
      * Describe  This method is called when the window loses focus.
      */
     void OnKillFocus(SWND wndFocus);
 
     /**
      * OnSetScale
      * @brief    Handles setting the scale of the window.
      * @param    UINT uMsg -- Message identifier.
      * @param    WPARAM wParam -- Scale factor.
      * @param    LPARAM lParam -- Reserved.
      * @return   LRESULT -- Result of the message processing.
      *
      * Describe  This method adjusts the window's scale.
      */
     LRESULT OnSetScale(UINT uMsg, WPARAM wParam, LPARAM lParam);
 
     /**
      * OnSetLanguage
      * @brief    Handles setting the language of the window.
      * @param    UINT uMsg -- Message identifier.
      * @param    WPARAM wParam -- Language identifier.
      * @param    LPARAM lParam -- Reserved.
      * @return   LRESULT -- Result of the message processing.
      *
      * Describe  This method changes the language of the window.
      */
     LRESULT OnSetLanguage(UINT uMsg, WPARAM wParam, LPARAM lParam);
 
     /**
      * OnSetColorize
      * @brief    Handles setting the colorization of the window.
      * @param    UINT uMsg -- Message identifier.
      * @param    WPARAM wParam -- Colorization value.
      * @param    LPARAM lParam -- Reserved.
      * @return   LRESULT -- Result of the message processing.
      *
      * Describe  This method applies colorization to the window.
      */
     LRESULT OnSetColorize(UINT uMsg, WPARAM wParam, LPARAM lParam);
 
     /**
      * OnUpdateFont
      * @brief    Handles updating the font of the window.
      * @param    UINT uMsg -- Message identifier.
      * @param    WPARAM wParam -- Font update parameter.
      * @param    LPARAM lParam -- Reserved.
      * @return   LRESULT -- Result of the message processing.
      *
      * Describe  This method updates the font used by the window.
      */
     LRESULT OnUpdateFont(UINT uMsg, WPARAM wParam, LPARAM lParam);
 
     // Message map for associating messages with handlers
     SOUI_MSG_MAP_BEGIN()
         MSG_WM_PAINT_EX(OnPaint)
         MSG_WM_ERASEBKGND_EX(OnEraseBkgnd)
         MSG_WM_NCPAINT_EX(OnNcPaint)
         MSG_WM_CREATE(OnCreate)
         MSG_WM_SIZE(OnSize)
         MSG_WM_DESTROY(OnDestroy)
         MSG_WM_SHOWWINDOW(OnShowWindow)
         MSG_WM_ENABLE_EX(OnEnable)
         MESSAGE_RANGE_HANDLER_EX(WM_LBUTTONDOWN, WM_MBUTTONDBLCLK, OnMouseClick)
         MSG_WM_LBUTTONDOWN(OnLButtonDown)
         MSG_WM_LBUTTONUP(OnLButtonUp)
         MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
         MSG_WM_RBUTTONDOWN(OnRButtonDown)
         MSG_WM_RBUTTONUP(OnRButtonUp)
         MSG_WM_MOUSEMOVE(OnMouseMove)
         MSG_WM_MOUSEHOVER(OnMouseHover)
         MSG_WM_MOUSELEAVE(OnMouseLeave)
         MSG_WM_MOUSEWHEEL(OnMouseWheel)
         MSG_WM_SETFOCUS_EX(OnSetFocus)
         MSG_WM_KILLFOCUS_EX(OnKillFocus)
         MESSAGE_HANDLER_EX(UM_SETLANGUAGE, OnSetLanguage)
         MESSAGE_HANDLER_EX(UM_SETSCALE, OnSetScale)
         MESSAGE_HANDLER_EX(UM_SETCOLORIZE, OnSetColorize)
         MESSAGE_HANDLER_EX(UM_UPDATEFONT, OnUpdateFont)
     SOUI_MSG_MAP_END_BASE() // Messages are not passed to the base class.
 
 // Protected attribute handling functions
 protected:
     /**
      * OnAttrVisible
      * @brief    Handles the 'visible' attribute.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @return   HRESULT -- Result of attribute processing.
      *
      * Describe  This method processes the 'visible' attribute.
      */
     HRESULT OnAttrVisible(const SStringW &strValue, BOOL bLoading);
 
     /**
      * OnAttrEnable
      * @brief    Handles the 'enable' attribute.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @return   HRESULT -- Result of attribute processing.
      *
      * Describe  This method processes the 'enable' attribute.
      */
     HRESULT OnAttrEnable(const SStringW &strValue, BOOL bLoading);
 
     /**
      * OnAttrDisplay
      * @brief    Handles the 'display' attribute.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @return   HRESULT -- Result of attribute processing.
      *
      * Describe  This method processes the 'display' attribute.
      */
     HRESULT OnAttrDisplay(const SStringW &strValue, BOOL bLoading);
 
     /**
      * OnAttrCache
      * @brief    Handles the 'cache' attribute.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @return   HRESULT -- Result of attribute processing.
      *
      * Describe  This method processes the 'cache' attribute.
      */
     HRESULT OnAttrCache(const SStringW &strValue, BOOL bLoading);
 
     /**
      * OnAttrAlpha
      * @brief    Handles the 'alpha' attribute.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @return   HRESULT -- Result of attribute processing.
      *
      * Describe  This method processes the 'alpha' attribute.
      */
     HRESULT OnAttrAlpha(const SStringW &strValue, BOOL bLoading);
 
     /**
      * OnAttrSkin
      * @brief    Handles the 'skin' attribute.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @return   HRESULT -- Result of attribute processing.
      *
      * Describe  This method processes the 'skin' attribute.
      */
     HRESULT OnAttrSkin(const SStringW &strValue, BOOL bLoading);
 
     /**
      * OnAttrLayout
      * @brief    Handles the 'layout' attribute.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @return   HRESULT -- Result of attribute processing.
      *
      * Describe  This method processes the 'layout' attribute.
      */
     HRESULT OnAttrLayout(const SStringW &strValue, BOOL bLoading);
 
     /**
      * OnAttrClass
      * @brief    Handles the 'class' attribute.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @return   HRESULT -- Result of attribute processing.
      *
      * Describe  This method processes the 'class' attribute.
      */
     HRESULT OnAttrClass(const SStringW &strValue, BOOL bLoading);
 
     /**
      * OnAttrTrackMouseEvent
      * @brief    Handles the 'trackMouseEvent' attribute.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @return   HRESULT -- Result of attribute processing.
      *
      * Describe  This method processes the 'trackMouseEvent' attribute.
      */
     HRESULT OnAttrTrackMouseEvent(const SStringW &strValue, BOOL bLoading);
 
     /**
      * OnAttrVideoCanvas
      * @brief    Handles the 'videoCanvas' attribute.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @return   HRESULT -- Result of attribute processing.
      *
      * Describe  This method processes the 'videoCanvas' attribute.
      */
     HRESULT OnAttrVideoCanvas(const SStringW &strValue, BOOL bLoading);
 
     /**
      * OnAttrID
      * @brief    Handles the 'id' attribute.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @return   HRESULT -- Result of attribute processing.
      *
      * Describe  This method processes the 'id' attribute.
      */
     HRESULT OnAttrID(const SStringW &strValue, BOOL bLoading);
 
     /**
      * OnAttrName
      * @brief    Handles the 'name' attribute.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @return   HRESULT -- Result of attribute processing.
      *
      * Describe  This method processes the 'name' attribute.
      */
     HRESULT OnAttrName(const SStringW &strValue, BOOL bLoading);
 
     /**
      * OnAttrTip
      * @brief    Handles the 'tip' attribute.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @return   HRESULT -- Result of attribute processing.
      *
      * Describe  This method processes the 'tip' attribute.
      */
     HRESULT OnAttrTip(const SStringW &strValue, BOOL bLoading);
 
     /**
      * OnAttrText
      * @brief    Handles the 'text' attribute.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @return   HRESULT -- Result of attribute processing.
      *
      * Describe  This method processes the 'text' attribute.
      */
     HRESULT OnAttrText(const SStringW &strValue, BOOL bLoading);
 
     /**
      * DefAttributeProc
      * @brief    Default attribute processing function.
      * @param    const SStringW &strAttribName -- Attribute name.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @return   HRESULT -- Result of attribute processing.
      *
      * Describe  This method processes attributes not explicitly handled.
      */
     HRESULT DefAttributeProc(const SStringW &strAttribName, const SStringW &strValue, BOOL bLoading);
 
     /**
      * AfterAttribute
      * @brief    Called after an attribute is processed.
      * @param    const SStringW &strAttribName -- Attribute name.
      * @param    const SStringW &strValue -- Attribute value.
      * @param    BOOL bLoading -- TRUE during loading, FALSE otherwise.
      * @param    HRESULT hr -- Result of attribute processing.
      * @return   HRESULT -- Result of post-processing.
      *
      * Describe  This method performs additional processing after an attribute is set.
      */
     virtual HRESULT AfterAttribute(const SStringW &strAttribName, const SStringW &strValue, BOOL bLoading, HRESULT hr);
     SOUI_ATTRS_BEGIN()
         ATTR_CUSTOM(L"layout", OnAttrLayout)
         ATTR_CUSTOM(L"class", OnAttrClass) //解析style
         ATTR_CUSTOM(L"id", OnAttrID)
         ATTR_CUSTOM(L"name", OnAttrName)
         ATTR_CUSTOM(L"skin", OnAttrSkin)      //直接获得皮肤对象
         ATTR_SKIN(L"ncskin", m_pNcSkin, TRUE) //直接获得皮肤对象
         ATTR_INT(L"data", m_uData, 0)
         ATTR_CUSTOM(L"text", OnAttrText)
         ATTR_STRINGW(L"trCtx", m_strTrCtx, FALSE)
         ATTR_CUSTOM(L"enable", OnAttrEnable)
         ATTR_CUSTOM(L"visible", OnAttrVisible)
         ATTR_CUSTOM(L"show", OnAttrVisible)
         ATTR_CUSTOM(L"display", OnAttrDisplay)
         ATTR_CUSTOM(L"cache", OnAttrCache)
         ATTR_CUSTOM(L"alpha", OnAttrAlpha)
         ATTR_BOOL(L"layeredWindow", m_bLayeredWindow, TRUE)
         ATTR_CUSTOM(L"trackMouseEvent", OnAttrTrackMouseEvent)
         ATTR_CUSTOM(L"videoCanvas", OnAttrVideoCanvas)
         ATTR_CUSTOM(L"tip", OnAttrTip)
         ATTR_BOOL(L"msgTransparent", m_bMsgTransparent, FALSE)
         ATTR_LAYOUTSIZE(L"maxWidth", m_nMaxWidth, FALSE)
         ATTR_BOOL(L"clipClient", m_bClipClient, FALSE)
         ATTR_BOOL(L"focusable", m_bFocusable, FALSE)
         ATTR_BOOL(L"drawFocusRect", m_bDrawFocusRect, TRUE)
         ATTR_BOOL(L"hoverAware", m_bHoverAware, FALSE)
         ATTR_BOOL(L"float", m_bFloat, FALSE)
         ATTR_CHAIN(m_style, HRET_FLAG_STYLE)                   //交给SwndStyle处理
         ATTR_CHAIN_PTR(m_pLayout, HRET_FLAG_LAYOUT)            //交给Layout处理
         ATTR_CHAIN_PTR(m_pLayoutParam, HRET_FLAG_LAYOUT_PARAM) //交给LayoutParam处理
     SOUI_ATTRS_END()
 // Protected virtual methods for handling specific events and operations in SWindow class.
 
 /**
  * @brief Called when the language of the window changes.
  * @return HRESULT Result of the operation.
  */
 protected:
     virtual HRESULT OnLanguageChanged();
 
 /**
  * @brief Called when the scale of the window changes.
  * @param scale The new scale value.
  */
     virtual void OnScaleChanged(int scale);
 
 /**
  * @brief Called when the font of the window needs to be rebuilt.
  */
     virtual void OnRebuildFont();
 
 /**
  * @brief Called before a child window is inserted into this window.
  * @param pChild Pointer to the child window being inserted.
  */
     virtual void OnBeforeInsertChild(SWindow *pChild);
 
 /**
  * @brief Called after a child window has been inserted into this window.
  * @param pChild Pointer to the child window that was inserted.
  */
     virtual void OnAfterInsertChild(SWindow *pChild);
 
 /**
  * @brief Called before a child window is removed from this window.
  * @param pChild Pointer to the child window being removed.
  */
     virtual void OnBeforeRemoveChild(SWindow *pChild);
 
 /**
  * @brief Called after a child window has been removed from this window.
  * @param pChild Pointer to the child window that was removed.
  */
     virtual void OnAfterRemoveChild(SWindow *pChild);
 
 /**
  * @brief Called when the container of the window changes.
  * @param pOldContainer Pointer to the old container.
  * @param pNewContainer Pointer to the new container.
  */
     virtual void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer);
 
 // Private member variable representing the window's position in its container.
 private:
     CRect m_rcWindow; /**< 
                          The position of the window within its container. Adjusted to private to prevent direct access by derived classes.
                        */
 
 // Protected members related to layout and state management.
 
 /**
  * @brief Enumerates the types of layout dirty states.
  */
 protected:
     enum LayoutDirtyType {
         dirty_clean = 0, // Clean state (no layout issues).
         dirty_self = 1,  // Self-dirty state (this window needs re-layout).
         dirty_child = 2, // Child-dirty state (a child window needs re-layout).
     };
 
 /**
  * @brief Member variables representing various properties of the window.
  */
     SWND m_swnd;   /**< Window handle. */
     BOOL m_bFloat; /**< Indicates if the window position is fixed. */
 
     ISwndContainer *m_pContainer; /**< Pointer to the container object. */
     SEventSet m_evtSet;           /**< Event set for the window. */
 
     SAutoRefPtr<ILayout> m_pLayout;
     SAutoRefPtr<ILayoutParam> m_pLayoutParam;
 
     SWindow *m_pOwner;       /**< Pointer to the owner window. */
     SWindow *m_pParent;      /**< Pointer to the parent window. */
     SWindow *m_pFirstChild;  /**< Pointer to the first child window. */
     SWindow *m_pLastChild;   /**< Pointer to the last child window. */
     SWindow *m_pNextSibling; /**< Pointer to the next sibling window. */
     SWindow *m_pPrevSibling; /**< Pointer to the previous sibling window. */
     UINT m_nChildrenCount;   /**< Number of child windows. */
 
     SWNDMSG *m_pCurMsg; /**< Pointer to the current message being processed. */
 
     SwndStyle m_style;        /**< Window style, a collection of window attributes. */
     STrText m_strText;        /**< Window text. */
     STrText m_strToolTipText; /**< Tooltip text for the window. */
     SStringW m_strTrCtx;      /**< Translation context. If empty, uses the container's translation context. */
     UINT m_uZorder;           /**< Z-order of the window. */
     int m_nUpdateLockCnt;     /**< Update lock count. Prevents Invalidate messages to the host when locked. */
 
     BOOL m_dwState;         /**< State of the window during rendering. */
     BOOL m_bVisible;        /**< Visibility state of the window. */
     BOOL m_bDisable;        /**< Disabled state of the window. */
     BOOL m_bDisplay;        /**< Indicates if the window occupies space when hidden. */
     BOOL m_bClipClient;     /**< Flag indicating if client area clipping is enabled. */
     BOOL m_bMsgTransparent; /**< Message transparency flag. TRUE means no message processing. */
     BOOL m_bFocusable;      /**< Indicates if the window can receive focus. */
     BOOL m_bDrawFocusRect;  /**< Indicates if the default focus rectangle should be drawn. */
     BOOL m_bCacheDraw;      /**< Indicates if the window content is cached. */
     BOOL m_bCacheDirty;     /**< Indicates if the cache is dirty. */
     BOOL m_bLayeredWindow;  /**< Indicates if the window is layered. */
     BOOL m_isLoading;       /**< Loading state flag. */
     BOOL m_bHoverAware;     /**< Hover-aware state flag. */
     BOOL m_bMsgHandled;     /**< Message handled flag. */
 
     LayoutDirtyType m_layoutDirty;         /**< Layout dirty state. */
     SAutoRefPtr<IRenderTarget> m_cachedRT; /**< Cached render target for the window. */
     SAutoRefPtr<IRegionS> m_clipRgn;       /**< Clipping region for the window. */
     SAutoRefPtr<IPathS> m_clipPath;        /**< Clipping path for the window. */
     SAutoRefPtr<ISkinObj> m_pBgSkin;       /**< Background skin object. */
     SAutoRefPtr<ISkinObj> m_pNcSkin;       /**< Non-client area skin object. */
     ULONG_PTR m_uData;                     /**< User data for the window. */
 
     SLayoutSize m_nMaxWidth; /**< Maximum width of the window when calculating size automatically. */
 
     COLORREF m_crColorize; /**< Colorization value for the window. */
 
     SAutoRefPtr<IAnimation> m_animation; /**< Animation object. */
     SAnimationHandler m_animationHandler;
     STransformation m_transform;
     bool m_isAnimating;
     bool m_isDestroying;
 
     typedef struct GETRTDATA {
         CRect rcRT;                    /**< Valid range for GETRT calls. */
         GrtFlag gdcFlags;              /**< Drawing flags for GETRT. */
         SAutoRefPtr<IRegionS> rgn;     /**< IRegion object corresponding to rcRT. */
         SAutoRefPtr<IRenderTarget> rt; /**< Render target created during GetRenderTarget, used for caching during redraws. */
     } * PGETRTDATA;
 
     PGETRTDATA m_pGetRTData;
 
     SAutoRefPtr<IAttrStorage> m_attrStorage; /**< Attribute storage object. */
     SAutoRefPtr<ICaret> m_caret;
 
     FunSwndProc m_funSwndProc;

     #ifdef _WIN32
     SAutoRefPtr<IAccessible> m_pAcc;
     SAutoRefPtr<IAccProxy> m_pAccProxy;
     #endif//_WIN32
 #ifdef _DEBUG
     tid_t m_nMainThreadId; /**< ID of the main thread hosting the window. */
 #endif
 };
 
 /**
  * @brief A helper class to enable or disable private UI definitions for the host container.
  */
 class SOUI_EXP SAutoEnableHostPrivUiDef {
 public:
     /**
      * @brief Constructor that enables private UI definitions for the host container.
      * @param pOwner Pointer to the owner window.
      */
     SAutoEnableHostPrivUiDef(SWindow *pOwner)
         : m_pOwner(pOwner) {
         m_pOwner->GetContainer()->EnableHostPrivateUiDef(TRUE);
     }
 
     /**
      * @brief Destructor that disables private UI definitions for the host container.
      */
     ~SAutoEnableHostPrivUiDef() {
         m_pOwner->GetContainer()->EnableHostPrivateUiDef(FALSE);
     }
 
 protected:
     SWindow *m_pOwner; /**< Pointer to the owner window. */
 };
 
 SNSEND
 #endif // __SWND__H__