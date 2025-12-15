//////////////////////////////////////////////////////////////////////////
//  Class Name: SHostWnd
//    Description: Real Container of SWindow
//////////////////////////////////////////////////////////////////////////

#ifndef __SHOSTWND__H__
#define __SHOSTWND__H__

#include <core/SWndContainerImpl.h>
#include <core/SNativeWnd.h>
#include <core/SDropTargetDispatcher.h>
#include <event/SEventCrack.h>
#include <interface/stooltip-i.h>
#include <interface/shostwnd-i.h>
#include <interface/SHostPresenter-i.h>
#include <core/SCaret.h>
#include <core/SNcPainter.h>
#include <layout/SLayoutSize.h>
#include <helper/SplitString.h>
#include <helper/swndspy.h>
#include <helper/STimerGenerator.h>
#include <proxy/SNativeWndProxy.h>
SNSBEGIN

/**
 * @brief Macro to disable SWnd spy for release builds.
 */
#ifndef DISABLE_SWNDSPY
#ifndef _DEBUG
#define DISABLE_SWNDSPY 1
#else
#define DISABLE_SWNDSPY 0
#endif
#endif

/**
 * @class SHostWndAttr
 * @brief Class defining attributes for the host window.
 *
 * This class manages and configures the appearance and behavior of the host window.
 */
class SOUI_EXP SHostWndAttr : public TObjRefImpl<SObject> {
    DEF_SOBJECT(TObjRefImpl<SObject>, L"SHostWndAttr")

    /**
     * @enum WndType
     * @brief Enumeration for different types of windows.
     */
    enum WndType
    {
        WT_UNDEFINE = 0, /**< Undefined window type */
        WT_APPMAIN = 1,  /**< Application main window */
        WT_NORMAL = 2    /**< Normal window */
    };

    /**
     * @brief Friend class declaration allowing access to private members.
     */
    friend class SHostWnd;
    friend class SRootWindow;
    friend class SNcPainter;
    friend class SHostPresenter;

  public:
    /**
     * @brief Constructor for SHostWndAttr.
     */
    SHostWndAttr(void);

    /**
     * @brief Destructor for SHostWndAttr.
     */
    ~SHostWndAttr();

    /**
     * @brief Initializes the window attributes.
     */
    void Init();

    /**
     * @brief Gets the translation context.
     *
     * @return Translation context string.
     */
    virtual LPCWSTR GetTrCtx() const;

    /**
     * @brief Gets the maximum inset size of the window.
     *
     * @param nScale Scale factor.
     * @return Maximum inset rectangle.
     */
    CRect GetMaxInset(int nScale) const;

    /**
     * @brief Gets the margin of the window.
     *
     * @param nScale Scale factor.
     * @return Margin rectangle.
     */
    CRect GetMargin(int nScale) const;

    /**
     * @brief Gets the minimum size of the window.
     *
     * @param nScale Scale factor.
     * @return Minimum size.
     */
    CSize GetMinSize(int nScale) const;

    /**
     * @brief Sets whether the window is translucent.
     *
     * @param bTranslucent TRUE if the window is translucent, FALSE otherwise.
     */
    void SetTranslucent(bool bTranslucent);

    /**
     * @brief Sets the translation context.
     *
     * @param strTrCtx Translation context string.
     */
    void SetTrCtx(const SStringW &strTrCtx);

    /**
     * @brief Sets whether wheel messages are sent to the hover window.
     *
     * @param value TRUE to send wheel messages to the hover window, FALSE otherwise.
     */
    void SetSendWheel2Hover(bool value);

    /**
     * @brief Defines the attribute list for serialization and deserialization.
     */
    SOUI_ATTRS_BEGIN()
        ATTR_STRINGW(L"trCtx", m_strTrCtx, FALSE)
        ATTR_STRINGW(L"title", m_strTitle, FALSE)
        ATTR_LAYOUTSIZE4(L"maxInset", m_rcMaxInset, FALSE)
        ATTR_LAYOUTSIZE4(L"margin", m_rcMargin, FALSE)
        ATTR_LAYOUTSIZE2(L"minsize", m_szMin, FALSE)
        ATTR_DWORD(L"wndStyle", m_dwStyle, FALSE)
        ATTR_DWORD(L"wndStyleEx", m_dwExStyle, FALSE)
        ATTR_BOOL(L"resizable", m_bResizable, FALSE)
        ATTR_BOOL(L"translucent", m_bTranslucent, FALSE)
        ATTR_BOOL(L"autoShape", m_bAutoShape, FALSE)
        ATTR_BOOL(L"sendWheel2Hover", m_bSendWheel2Hover, FALSE)
        ATTR_BOOL(L"appWnd", m_bAppWnd, FALSE)
        ATTR_BOOL(L"toolWindow", m_bToolWnd, FALSE)
        ATTR_ICON(L"smallIcon", m_hAppIconSmall, FALSE)
        ATTR_ICON(L"bigIcon", m_hAppIconBig, FALSE)
        ATTR_BOOL(L"allowSpy", m_bAllowSpy, FALSE)
        ATTR_BOOL(L"hasMsgLoop", m_bHasMsgLoop, FALSE)
        ATTR_ENUM_BEGIN(L"wndType", WndType, FALSE)
            ATTR_ENUM_VALUE(L"undefine", WT_UNDEFINE)
            ATTR_ENUM_VALUE(L"appMain", WT_APPMAIN)
            ATTR_ENUM_VALUE(L"normal", WT_NORMAL)
        ATTR_ENUM_END(m_wndType)
    SOUI_ATTRS_END()

  protected:
    SLayoutSize m_rcMargin[4];   /**< Window stretch edge detection size */
    SLayoutSize m_szMin[2];      /**< Window minimum size */
    SLayoutSize m_rcMaxInset[4]; /**< Window maximum inset size when maximized. WS_OVERLAPPED style windows ignore this attribute */

    WndType m_wndType;       /**< Main window flag, if set, the window sends WM_QUIT on close */
    BOOL m_bResizable;       /**< Flag indicating if the window is resizable */
    BOOL m_bAppWnd;          /**< Flag indicating if the window is an APP window (shown in taskbar) */
    BOOL m_bToolWnd;         /**< Flag indicating if the window has WS_ES_TOOLWINDOW style */
    BOOL m_bTranslucent;     /**< Flag indicating if the window is translucent */
    BOOL m_bAutoShape;       /**< Flag indicating if the window should auto-build shape for translucency (valid for Linux) */
    BOOL m_bAllowSpy;        /**< Flag indicating if spy is allowed */
    BOOL m_bSendWheel2Hover; /**< Flag indicating if wheel messages should be sent to the hover window */
    BOOL m_bHasMsgLoop;      /**< Flag indicating if the window has a message loop, affecting tooltip RelayEvent timing */
    DWORD m_dwStyle;
    DWORD m_dwExStyle;

    SStringW m_strTrCtx; /**< Translation context used in language translation */
    SStringW m_strTitle;
    HICON m_hAppIconSmall;
    HICON m_hAppIconBig;
};
/**
 * @class SRootWindow
 * @brief Root window class derived from SWindow.
 *
 * This class represents the root window in the SOUI framework and manages various window behaviors and events.
 */
class SOUI_EXP SRootWindow : public SWindow {
    // DEF_SOBJECT macro defines the type name of the class
    DEF_SOBJECT(SWindow, L"root")
    // Declare SHostWnd as a friend class to allow access to private members
    friend class SHostWnd;

  public:
    /**
     * @brief Constructor for SRootWindow.
     *
     * Initializes the SRootWindow object with a pointer to the host window.
     *
     * @param pHostWnd Pointer to the host window.
     */
    SRootWindow(SHostWnd *pHostWnd);

  public:
    /**
     * @brief Gets the pointer to the host window.
     *
     * @return Pointer to the host window.
     */
    SHostWnd *GetHostWnd() const;

    /**
     * @brief Fires a menu command event.
     *
     * @param menuID ID of the menu command to trigger.
     */
    void FireMenuCmd(int menuID);

  public:
    /**
     * @brief Updates the layout of the window.
     *
     * Overrides the base class method to update the layout.
     */
    STDMETHOD_(void, UpdateLayout)(THIS) OVERRIDE;

  protected:
    /**
     * @brief Called when an animation stops.
     *
     * Overrides the base class method to handle animation stop events.
     *
     * @param pAni Pointer to the animation object.
     */
    STDMETHOD_(void, OnAnimationStop)(THIS_ IAnimation *pAni) OVERRIDE;

    /**
     * @brief Called when an animation is invalidated.
     *
     * @param pAni Pointer to the animation object.
     * @param bErase Flag indicating whether to erase the animation.
     */
    virtual void OnAnimationInvalidate(IAnimation *pAni, bool bErase);

  protected: // SWindow virtual methods
    /**
     * @brief Called before painting the window.
     *
     * @param pRT Pointer to the render target.
     * @param painter Painter object.
     */
    virtual void BeforePaint(IRenderTarget *pRT, SPainter &painter);

    /**
     * @brief Called after painting the window.
     *
     * @param pRT Pointer to the render target.
     * @param painter Painter object.
     */
    virtual void AfterPaint(IRenderTarget *pRT, SPainter &painter);

    /**
     * @brief Checks if the window is a layered window.
     *
     * @return TRUE if the window is a layered window, FALSE otherwise.
     */
    virtual BOOL IsLayeredWindow() const
    {
        return FALSE;
    }

    /**
     * @brief Called when the language changes.
     *
     * @return HRESULT indicating the success or failure of the operation.
     */
    virtual HRESULT OnLanguageChanged();

    /**
     * @brief Called when the scale changes.
     *
     * @param scale New scale factor.
     */
    virtual void OnScaleChanged(int scale);

    /**
     * @brief Requests a relayout of the window.
     *
     * @param hSource Source window handle.
     * @param bSourceResizable Flag indicating if the source window is resizable.
     */
    virtual void RequestRelayout(SWND hSource, BOOL bSourceResizable);

  public:
    // Define attributes for enter and exit animations
    SOUI_ATTRS_BEGIN()
        ATTR_ANIMATION(L"enterAnimation", m_aniEnter, FALSE)
        ATTR_ANIMATION(L"exitAnimation", m_aniExit, FALSE)
    SOUI_ATTRS_END()

  protected:
    // Auto-managed pointers for enter and exit animations
    SAutoRefPtr<IAnimation> m_aniEnter, m_aniExit;
    // Pointer to the host window
    SHostWnd *m_pHostWnd;
};
class SDummyWnd;

/**
 * @class SHostWnd
 * @brief The main host window class responsible for managing the layout, events, and rendering of SOUI windows.
 *
 * This class acts as a container for SWindow objects and provides functionality for creating, destroying, and managing
 * child windows. It also handles various Windows messages and animations.
 *
 * @note Inherits from TNativeWndProxy<IHostWnd> and SwndContainerImpl.
 */
class SOUI_EXP SHostWnd
    : public TNativeWndProxy<IHostWnd>
    , public SwndContainerImpl {
    friend class SDummyWnd;   /**< Friend class used for handling WM_PAINT messages in translucent windows. */
    friend class SRootWindow; /**< Friend class representing the root window. */
    friend class SNcPainter;  /**< Friend class for non-client area painting. */

  protected:
    SDummyWnd *m_dummyWnd;   /**< A dummy window used to handle WM_PAINT messages for translucent windows. */
    SHostWndAttr m_hostAttr; /**< Host attributes corresponding to the SOUI node in XML. */
    SStringT m_strXmlLayout; /**< Layout resource name (e.g., xml:main_wnd). */

    BOOL m_bTrackFlag;      /**< Mouse tracking flag. */
    BOOL m_bNeedRepaint;    /**< Indicates if a repaint is required. */
    BOOL m_bNeedAllRepaint; /**< Indicates if a full repaint is required. */

    SAutoRefPtr<IToolTip> m_pTipCtrl; /**< Interface for tooltip control. */

    SAutoRefPtr<IRegionS> m_rgnInvalidate;      /**< Invalid region requiring redraw. */
    SAutoRefPtr<IRenderTarget> m_memRT;         /**< Rendering cache. */
    SAutoRefPtr<IUiDefInfo> m_privateUiDefInfo; /**< Local UI definition information. */
    int m_cEnableUiDefCount;                    /**< Count of enabled local UI definitions. */
    SAutoRefPtr<IScriptModule> m_pScriptModule; /**< Script module for executing scripts. */
    SAutoRefPtr<SNcPainter> m_pNcPainter;       /**< Non-client area painter. */

    MSG m_msgMouse;      /**< Last mouse press message. */
    CSize m_szAppSetted; /**< Window size set by the application layer. */
    CSize m_szPrev;      /**< Previous window size. */
    int m_nAutoSizing;   /**< Auto-sizing trigger count for WM_SIZE messages. */
    BOOL m_bResizing;    /**< Indicates if resizing is in progress. */

    SAutoRefPtr<IAnimation> m_hostAnimation; /**< Host animation object. */
    DWORD m_AniState; /**< Animation state. */
    BOOL m_bFirstShow; /**< Indicates if it's the first time the window is shown. */
    tid_t m_dwThreadID; /**< Thread ID. */
    SRootWindow *m_pRoot; /**< Pointer to the root window. */

    EventHandlerInfo m_evtHandler;           /**< Event handler information. */
    SAutoRefPtr<IHostPresenter> m_presenter; /**< Presenter for rendering. */

    SCriticalSection m_cs;             /**< Critical section for thread synchronization. */
    SList<IRunnable *> m_runnables;    /**< List of runnable tasks. */
    SCriticalSection m_csRunningQueue; /**< Critical section for running queue synchronization. */
    SList<IRunnable *> m_runningQueue; /**< Queue of currently running tasks. */
    IXmlNode *m_xmlInit;               /**< Initial XML node. */
    static BOOL s_HideLocalUiDef;      /**< Global flag to hide local UI definitions. */
    static int s_TaskQueueBufSize;     /**< Buffer size for asynchronous task queues (default: 5). */

  public:
    /**
     * @brief Constructs a SHostWnd object with an optional resource name.
     * @param pszResName Resource name for the layout (can be NULL).
     */
    SHostWnd(LPCWSTR pszResName = NULL);

    /**
     * @brief Constructs a SHostWnd object with an optional resource name (ANSI version).
     * @param pszResName Resource name for the layout (can be NULL).
     */
    SHostWnd(LPCSTR pszResName);

    /**
     * @brief Destructor for SHostWnd.
     */
    virtual ~SHostWnd();

    /**
     * @brief Sets whether to hide local UI definitions globally.
     * @param bHide TRUE to hide, FALSE otherwise.
     */
    static void SetHideLocalUiDef(BOOL bHide);

    /**
     * @brief Sets the buffer size for asynchronous task queues.
     * @param nBufSize Buffer size for task queues.
     */
    static void SetTaskQueueBufSize(int nBufSize);

  public:
    enum
    {
        kPulseTimer = 4321,    /**< SOUI timer ID (do not use in applications). */
        kNcCheckTimer = 4322,  /**< Timer ID for non-client area checks. */
        kNcCheckInterval = 50, /**< Interval for non-client area checks in milliseconds. */
        kTaskTimer = 4323,     /**< Timer ID for task execution. */
        kTaskInterval = 100    /**< Interval for task execution in milliseconds. */
    };

    /**
     * @brief Initializes the host window from an XML node.
     * @param pNode Pointer to the XML node.
     * @return TRUE if initialization succeeds, FALSE otherwise.
     */
    STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode *pNode) OVERRIDE;

    /**
     * @brief Destroys the host window.
     * @return TRUE if destruction succeeds, FALSE otherwise.
     */
    STDMETHOD_(BOOL, DestroyWindow)(THIS) OVERRIDE;

    /**
     * @brief Sets the layout ID for the host window.
     * @param pszLayoutId Layout resource identifier.
     */
    STDMETHOD_(void, SetLayoutId)(THIS_ LPCTSTR pszLayoutId) OVERRIDE
    {
        m_strXmlLayout = pszLayoutId;
    }

    /**
     * @brief Gets the root window interface.
     * @return Pointer to the root window interface.
     */
    STDMETHOD_(IWindow *, GetIRoot)(THIS) OVERRIDE
    {
        return m_pRoot;
    }

    /**
     * @brief Checks if the host window is translucent.
     * @return TRUE if translucent, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsTranslucent)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Gets the presenter interface for rendering.
     * @return Pointer to the presenter interface.
     */
    STDMETHOD_(IHostPresenter *, GetPresenter)(THIS) OVERRIDE;

    /**
     * @brief Sets the presenter for rendering.
     * @param pPresenter Pointer to the presenter interface.
     */
    STDMETHOD_(void, SetPresenter)(THIS_ IHostPresenter *pPresenter) OVERRIDE;

    /**
     * @brief Gets the message loop interface.
     * @return Pointer to the message loop interface.
     */
    STDMETHOD_(IMessageLoop *, GetMsgLoop)(THIS) OVERRIDE;

    /**
     * @brief Finds a child window by its ID.
     *
     * @param nId The ID of the child window to find.
     * @return Pointer to the found child window, or NULL if not found.
     */
    STDMETHOD_(IWindow *, FindIChildByID)(THIS_ int nId) OVERRIDE
    {
        return m_pRoot->FindIChildByID(nId);
    }

    /**
     * @brief Finds a child window by its name (Unicode version).
     *
     * @param pszName The name of the child window to find.
     * @return Pointer to the found child window, or NULL if not found.
     */
    STDMETHOD_(IWindow *, FindIChildByName)(THIS_ LPCWSTR pszName) OVERRIDE
    {
        return m_pRoot->FindIChildByName(pszName);
    }

    /**
     * @brief Finds a child window by its name (ANSI version).
     *
     * @param pszName The name of the child window to find.
     * @return Pointer to the found child window, or NULL if not found.
     */
    STDMETHOD_(IWindow *, FindIChildByNameA)(THIS_ LPCSTR pszName) OVERRIDE
    {
        return m_pRoot->FindIChildByNameA(pszName);
    }

    /**
     * @brief Gets the non-client area painter for the host window.
     *
     * @return Pointer to the non-client area painter.
     */
    STDMETHOD_(INcPainter *, GetNcPainter)(THIS) OVERRIDE
    {
        return m_pNcPainter;
    }

    /**
     * @brief Shows or hides the host window.
     *
     * @param nCmdShow Command to show or hide the window.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, ShowWindow)(THIS_ int nCmdShow) OVERRIDE;

    /**
     * @brief Creates the host window with extended styles.
     *
     * @param hWndParent Handle to the parent window.
     * @param dwStyle Window style.
     * @param dwExStyle Extended window style.
     * @param x X position of the window.
     * @param y Y position of the window.
     * @param nWidth Width of the window.
     * @param nHeight Height of the window.
     * @param xmlInit Optional XML node for initialization.
     * @return Handle to the created window.
     */
    STDMETHOD_(HWND, CreateEx)
    (THIS_ HWND hWndParent, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, IXmlNode *xmlInit DEF_VAL(NULL)) OVERRIDE;

    /**
     * @brief Creates the host window.
     *
     * @param hWndParent Handle to the parent window.
     * @param x X position of the window (default: 0).
     * @param y Y position of the window (default: 0).
     * @param nWidth Width of the window (default: 0).
     * @param nHeight Height of the window (default: 0).
     * @return Handle to the created window.
     */
    STDMETHOD_(HWND, Create)
    (THIS_ HWND hWndParent, int x = 0, int y = 0, int nWidth = 0, int nHeight = 0) OVERRIDE;

    /**
     * @brief Sets the event handler for the host window.
     *
     * @param fun Callback function for handling events.
     * @param ctx Context pointer passed to the callback function.
     */
    STDMETHOD_(void, SetEventHandler)(THIS_ FunCallback fun, void *ctx) OVERRIDE;

    /**
     * @brief Gets the event handler information for the host window.
     *
     * @return Pointer to the event handler information.
     */
    STDMETHOD_(EventHandlerInfo *, GetEventHandler)(THIS) OVERRIDE;

    /**
     * @brief Animates the host window.
     *
     * @param dwTime Duration of the animation in milliseconds.
     * @param dwFlags Animation flags.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, AnimateHostWindow)(THIS_ DWORD dwTime, DWORD dwFlags) OVERRIDE;

    /**
     * @brief Enables drag-and-drop functionality for the host window.
     */
    STDMETHOD_(void, EnableDragDrop)(THIS) OVERRIDE;

    /**
     * @brief Shows the host window with optional animation.
     *
     * @param uShowCmd Command to show the window.
     * @param bWaitAniDone Wait for the animation to complete before returning.
     */
    STDMETHOD_(void, ShowHostWnd)(THIS_ int uShowCmd, BOOL bWaitAniDone) OVERRIDE;

    /**
     * @brief Enables or disables private UI definitions for the host window.
     *
     * @param bEnable TRUE to enable, FALSE to disable.
     */
    STDMETHOD_(void, EnablePrivateUiDef)(THIS_ BOOL bEnable) OVERRIDE;

    /**
     * @brief Sets the scale factor for the host window.
     *
     * @param nScale Scale factor.
     * @param pDestRect Destination rectangle.
     */
    STDMETHOD_(void, SetScale)(THIS_ int nScale, LPCRECT pDestRect) OVERRIDE;

    /**
     * @brief Finds a child window by its name (Unicode version).
     *
     * @param strName The name of the child window to find.
     * @param nDeep Depth of the search (default: -1, search all levels).
     * @return Pointer to the found child window, or NULL if not found.
     */
    SWindow *FindChildByName(LPCWSTR strName, int nDeep = -1)
    {
        return GetRoot()->FindChildByName(strName, nDeep);
    }

    /**
     * @brief Finds a child window by its name (ANSI version).
     *
     * @param strName The name of the child window to find.
     * @param nDeep Depth of the search (default: -1, search all levels).
     * @return Pointer to the found child window, or NULL if not found.
     */
    SWindow *FindChildByName(LPCSTR strName, int nDeep = -1)
    {
        return GetRoot()->FindChildByName(strName, nDeep);
    }

    /**
     * @brief Finds a child window by its name (Unicode version) with template support.
     *
     * @tparam T Type of the child window to find.
     * @param pszName The name of the child window to find.
     * @param nDeep Depth of the search (default: -1, search all levels).
     * @return Pointer to the found child window, or NULL if not found.
     */
    template <class T>
    T *FindChildByName2(LPCWSTR pszName, int nDeep = -1)
    {
        return GetRoot()->FindChildByName2<T>(pszName, nDeep);
    }

    /**
     * @brief Finds a child window by its name (ANSI version) with template support.
     *
     * @tparam T Type of the child window to find.
     * @param pszName The name of the child window to find.
     * @param nDeep Depth of the search (default: -1, search all levels).
     * @return Pointer to the found child window, or NULL if not found.
     */
    template <class T>
    T *FindChildByName2(LPCSTR pszName, int nDeep = -1)
    {
        return GetRoot()->FindChildByName2<T>(pszName, nDeep);
    }

    /**
     * @brief Finds a child window by its ID.
     *
     * @param nID The ID of the child window to find.
     * @param nDeep Depth of the search (default: -1, search all levels).
     * @return Pointer to the found child window, or NULL if not found.
     */
    SWindow *FindChildByID(int nID, int nDeep = -1)
    {
        return GetRoot()->FindChildByID(nID, nDeep);
    }

    /**
     * @brief Finds a child window by its ID with template support.
     *
     * @tparam T Type of the child window to find.
     * @param nID The ID of the child window to find.
     * @param nDeep Depth of the search (default: -1, search all levels).
     * @return Pointer to the found child window, or NULL if not found.
     */
    template <class T>
    T *FindChildByID2(int nID, int nDeep = -1)
    {
        return GetRoot()->FindChildByID2<T>(nID, nDeep);
    }

    /**
     * @brief Finds a child window by its class.
     *
     * @tparam T Type of the child window to find.
     * @param nDeep Depth of the search (default: -1, search all levels).
     * @return Pointer to the found child window, or NULL if not found.
     */
    template <class T>
    T *FindChildByClass(int nDeep = -1) const
    {
        return GetRoot()->FindChildByClass<T>(nDeep);
    }

    /**
     * @brief Gets the root window.
     *
     * @return Pointer to the root window.
     */
    SWindow *GetRoot() const
    {
        return m_pRoot;
    }

    /**
     * @brief Gets the native window interface.
     *
     * @return Pointer to the native window interface.
     */
    SNativeWnd *GetNative()
    {
        return this;
    }

    /**
     * @brief Gets the window rectangle.
     *
     * @return Rectangle representing the window's position and size.
     */
    CRect GetWindowRect() const;

    /**
     * @brief Gets the client rectangle.
     *
     * @return Rectangle representing the client area's position and size.
     */
    CRect GetClientRect() const;

    /**
     * @brief Gets the host window attributes.
     *
     * @return Reference to the host window attributes.
     */
    SHostWndAttr &GetHostAttr()
    {
        return m_hostAttr;
    }

    /**
     * @brief Gets the tooltip interface.
     *
     * @return Pointer to the tooltip interface.
     */
    IToolTip *GetToolTip() const
    {
        return m_pTipCtrl;
    }

    /**
     * @brief Starts an animation for the host window.
     *
     * @param pAni Pointer to the animation object.
     * @return TRUE if successful, FALSE otherwise.
     */
    bool StartHostAnimation(IAnimation *pAni);

    /**
     * @brief Stops the current animation for the host window.
     *
     * @return TRUE if successful, FALSE otherwise.
     */
    bool StopHostAnimation();

    /**
     * @brief Updates the auto-size count.
     *
     * @param bInc TRUE to increment, FALSE to decrement.
     */
    void UpdateAutoSizeCount(bool bInc);

  protected:
    /**
     * @brief Handler class for host window animations.
     *
     * This class implements the ITimelineHandler interface to handle animation frames.
     */
    class SHostAnimationHandler : public ITimelineHandler {
      public:
        /**
         * @brief Pointer to the host window.
         */
        SHostWnd *m_pHostWnd;

        /**
         * @brief Initial rectangle of the host window.
         */
        CRect m_rcInit;

      protected:
        /**
         * @brief Called on each frame of the animation.
         */
        STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;
    } m_hostAnimationHandler;

    /**
     * @brief Called when the host window animation starts.
     *
     * @param pAni Pointer to the animation object.
     */
    virtual void OnHostAnimationStarted(IAnimation *pAni);

    /**
     * @brief Called when the host window animation stops.
     *
     * @param pAni Pointer to the animation object.
     */
    virtual void OnHostAnimationStoped(IAnimation *pAni);

  protected: // 辅助函数
    /**
     * @brief Redraws a specific region of the window.
     *
     * @param pRgn Region to redraw.
     * @param rcInvalid Invalid rectangle.
     */
    void _RedrawRegion(IRegionS *pRgn, CRect &rcInvalid);

    /**
     * @brief Redraws the entire window.
     */
    void _Redraw();

    /**
     * @brief Restores the click state of the window.
     */
    void _RestoreClickState();

    /**
     * @brief Invalidates a specific region of the window.
     *
     * @param prc Rectangle to invalidate.
     */
    void _Invalidate(LPCRECT prc);

    /**
     * @brief Sets tooltip information for the window.
     *
     * @param info Tooltip information.
     * @param bNcTip Flag indicating if the tooltip is for the non-client area.
     */
    void _SetToolTipInfo(const SwndToolTipInfo *info, BOOL bNcTip);

    /**
     * @brief Initializes the host window.
     */
    void _Init();

    /**
     * @brief Excludes the video canvas from painting.
     *
     * @param pRT Render target.
     */
    void _ExcludeVideoCanvasFromPaint(IRenderTarget *pRT);

    /**
     * @brief Paints the video canvas in the foreground.
     *
     * @param pRT Render target.
     */
    void _PaintVideoCanvasForeground(IRenderTarget *pRT);

  protected:
    //////////////////////////////////////////////////////////////////////////
    // 消息处理

    /**
     * @brief Handles the WM_PRINT message.
     *
     * @param dc Device context.
     * @param uFlags Print flags.
     */
    void OnPrint(HDC dc, UINT uFlags = 0);

    /**
     * @brief Handles the WM_PAINT message.
     *
     * @param dc Device context.
     */
    void OnPaint(HDC dc);

    /**
     * @brief Handles the WM_ERASEBKGND message.
     *
     * @param dc Device context.
     * @return TRUE if the background is erased, FALSE otherwise.
     */
    BOOL OnEraseBkgnd(HDC dc);

    /**
     * @brief Handles the WM_CREATE message.
     *
     * @param lpCreateStruct Create structure.
     * @return 0 on success, non-zero on failure.
     */
    int OnCreate(LPCREATESTRUCT lpCreateStruct);

    /**
     * @brief Handles the WM_DESTROY message.
     */
    void OnDestroy();

    /**
     * @brief Handles the WM_SIZE message.
     *
     * @param nType Size type.
     * @param size New size of the window.
     */
    void OnSize(UINT nType, CSize size);

    /**
     * @brief Handles the WM_MOUSEMOVE message.
     *
     * @param nFlags Mouse flags.
     * @param point Mouse position.
     */
    void OnMouseMove(UINT nFlags, CPoint point);

    /**
     * @brief Handles the WM_MOUSELEAVE message.
     */
    void OnMouseLeave();

    /**
     * @brief Handles the WM_SETCURSOR message.
     *
     * @param hWnd Handle to the window.
     * @param nHitTest Hit test code.
     * @param message Message identifier.
     * @return TRUE if the cursor is set, FALSE otherwise.
     */
    BOOL OnSetCursor(HWND hWnd, UINT nHitTest, UINT message);

    /**
     * @brief Handles the WM_TIMER message.
     *
     * @param idEvent Timer identifier.
     */
    void OnTimer(UINT_PTR idEvent);

    /**
     * @brief Handles mouse events.
     *
     * @param uMsg Message identifier.
     * @param wParam WPARAM.
     * @param lParam LPARAM.
     * @return LRESULT.
     */
    LRESULT OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles key events.
     *
     * @param uMsg Message identifier.
     * @param wParam WPARAM.
     * @param lParam LPARAM.
     * @return LRESULT.
     */
    LRESULT OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles the WM_ACTIVATEAPP message.
     *
     * @param uMsg Message identifier.
     * @param wParam WPARAM.
     * @param lParam LPARAM.
     * @return LRESULT.
     */
    LRESULT OnActivateApp(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles the WM_MOUSEWHEEL message.
     *
     * @param nFlags Mouse flags.
     * @param zDelta Wheel delta.
     * @param pt Mouse position.
     * @return TRUE if handled, FALSE otherwise.
     */
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    /**
     * @brief Handles the WM_ACTIVATE message.
     *
     * @param nState Activation state.
     * @param bMinimized Minimized flag.
     * @param wndOther Other window handle.
     */
    void OnActivate(UINT nState, BOOL bMinimized, HWND wndOther);

    /**
     * @brief Handles the WM_GETMINMAXINFO message.
     *
     * @param lpMMI Min-max information.
     */
    void OnGetMinMaxInfo(LPMINMAXINFO lpMMI);

    /**
     * @brief Handles the WM_SETFOCUS message.
     *
     * @param wndOld Previous window handle.
     */
    void OnSetFocus(HWND wndOld);

    /**
     * @brief Handles the WM_KILLFOCUS message.
     *
     * @param wndFocus New focus window handle.
     */
    void OnKillFocus(HWND wndFocus);

    /**
     * @brief Updates the presenter.
     *
     * @param dc Device context.
     * @param pRT Render target.
     * @param rc Rectangle.
     * @param byAlpha Alpha value.
     * @param uFlag Flags.
     */
    void UpdatePresenter(HDC dc, IRenderTarget *pRT, LPCRECT rc, BYTE byAlpha = 255, UINT uFlag = 0);

    /**
     * @brief Handles the WM_CAPTURECHANGED message.
     *
     * @param wnd New capture window handle.
     */
    void OnCaptureChanged(HWND wnd);

    /**
     * @brief Handles custom menu events.
     *
     * @param uMsg Message identifier.
     * @param wParam WPARAM.
     * @param lParam LPARAM.
     * @return LRESULT.
     */
    LRESULT OnMenuExEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles the WM_WINDOWPOSCHANGING message.
     *
     * @param lpWndPos Window position structure.
     */
    void OnWindowPosChanging(LPWINDOWPOS lpWndPos);

    /**
     * @brief Handles the WM_WINDOWPOSCHANGED message.
     *
     * @param lpWndPos Window position structure.
     */
    void OnWindowPosChanged(LPWINDOWPOS lpWndPos);

    /**
     * @brief Handles the WM_GETOBJECT message.
     *
     * @param uMsg Message identifier.
     * @param wParam WPARAM.
     * @param lParam LPARAM.
     * @return LRESULT.
     */
    LRESULT OnGetObject(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles the WM_SYSCOMMAND message.
     *
     * @param nID System command identifier.
     * @param lParam LPARAM.
     */
    void OnSysCommand(UINT nID, CPoint lParam);

    /**
     * @brief Handles the WM_COMMAND message.
     *
     * @param uNotifyCode Notification code.
     * @param nID Control ID.
     * @param wndCtl Control window handle.
     */
    void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);

#if (!DISABLE_SWNDSPY)
  protected:
    /**
     * @brief Handles the SPYMSG_SETSPY message.
     *
     * @param uMsg Message identifier.
     * @param wParam WPARAM.
     * @param lParam LPARAM.
     * @return LRESULT.
     */
    LRESULT OnSpyMsgSetSpy(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles the SPYMSG_SWNDENUM message.
     *
     * @param uMsg Message identifier.
     * @param wParam WPARAM.
     * @param lParam LPARAM.
     * @return LRESULT.
     */
    LRESULT OnSpyMsgSwndEnum(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles the SPYMSG_SWNDINFO message.
     *
     * @param uMsg Message identifier.
     * @param wParam WPARAM.
     * @param lParam LPARAM.
     * @return LRESULT.
     */
    LRESULT OnSpyMsgSwndSpy(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles the SPYMSG_HITTEST message.
     *
     * @param uMsg Message identifier.
     * @param wParam WPARAM.
     * @param lParam LPARAM.
     * @return LRESULT.
     */
    LRESULT OnSpyMsgHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handle to the spy window.
     */
    HWND m_hSpyWnd;
#endif

  public: // IContainer
    /**
     * @brief Fires an event in the container.
     *
     * @param evt Pointer to the event arguments.
     * @return TRUE if the event was handled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, OnFireEvent)(IEvtArgs *evt) OVERRIDE;

    /**
     * @brief Gets the rectangle of the container.
     *
     * @param ret Pointer to the rectangle to be filled.
     */
    STDMETHOD_(void, GetContainerRect)(CTHIS_ RECT *ret) const OVERRIDE;

    /**
     * @brief Gets the handle to the host window.
     *
     * @return Handle to the host window.
     */
    STDMETHOD_(HWND, GetHostHwnd)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Gets the translator context for the container.
     *
     * @return Pointer to the translator context string.
     */
    STDMETHOD_(LPCWSTR, GetTranslatorContext)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Updates the specified region in the container.
     *
     * @param rgn Pointer to the region to update.
     */
    STDMETHOD_(void, UpdateRegion)(IRegionS *rgn) OVERRIDE;

    /**
     * @brief Redraws the specified rectangle in the container.
     *
     * @param rc Pointer to the rectangle to redraw.
     * @param bClip Flag indicating whether to clip the redraw.
     */
    STDMETHOD_(void, OnRedraw)(LPCRECT rc, BOOL bClip) OVERRIDE;

    /**
     * @brief Releases the capture from the container.
     *
     * @return TRUE if the capture was released, FALSE otherwise.
     */
    STDMETHOD_(BOOL, OnReleaseSwndCapture)() OVERRIDE;

    /**
     * @brief Sets the capture to the specified window.
     *
     * @param swnd Handle to the window to capture.
     * @return Handle to the previously captured window.
     */
    STDMETHOD_(SWND, OnSetSwndCapture)(SWND swnd) OVERRIDE;

    /**
     * @brief Checks if wheel messages are sent to the hover window.
     *
     * @return TRUE if wheel messages are sent to the hover window, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsSendWheel2Hover)() const OVERRIDE;

    /**
     * @brief Updates the window.
     *
     * @param bForce Flag indicating whether to force the update.
     * @return TRUE if the update was successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, UpdateWindow)(BOOL bForce DEF_VAL(TRUE)) OVERRIDE;

    /**
     * @brief Updates the tooltip for the container.
     */
    STDMETHOD_(void, UpdateTooltip)() OVERRIDE;

    /**
     * @brief Sets the tooltip for the container.
     *
     * @param rc Rectangle for the tooltip.
     * @param tipAlign Alignment of the tooltip.
     * @param pszTip Tooltip text.
     */
    STDMETHOD_(void, SetToolTip)(THIS_ LPCRECT rc, UINT tipAlign, LPCTSTR pszTip) OVERRIDE;

    /**
     * @brief Registers a timeline handler.
     *
     * @param pHandler Pointer to the timeline handler.
     * @return TRUE if the handler was registered, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RegisterTimelineHandler)(THIS_ ITimelineHandler *pHandler) OVERRIDE;

    /**
     * @brief Unregisters a timeline handler.
     *
     * @param pHandler Pointer to the timeline handler.
     * @return TRUE if the handler was unregistered, FALSE otherwise.
     */
    STDMETHOD_(BOOL, UnregisterTimelineHandler)(THIS_ ITimelineHandler *pHandler) OVERRIDE;

    /**
     * @brief Registers a value animator.
     *
     * @param pAnimator Pointer to the value animator.
     * @return TRUE if the animator was registered, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RegisterValueAnimator)(THIS_ IValueAnimator *pAnimator) OVERRIDE;

    /**
     * @brief Unregisters a value animator.
     *
     * @param pAnimator Pointer to the value animator.
     * @return TRUE if the animator was unregistered, FALSE otherwise.
     */
    STDMETHOD_(BOOL, UnregisterValueAnimator)(THIS_ IValueAnimator *pAnimator) OVERRIDE;

    /**
     * @brief Enables or disables host private UI definitions.
     *
     * @param bEnable TRUE to enable, FALSE to disable.
     */
    STDMETHOD_(void, EnableHostPrivateUiDef)(THIS_ BOOL bEnable) OVERRIDE;

    /**
     * @brief Gets the script module for the container.
     *
     * @return Pointer to the script module.
     */
    STDMETHOD_(IScriptModule *, GetScriptModule)() OVERRIDE;

    /**
     * @brief Gets the scale factor for the container.
     *
     * @return Scale factor.
     */
    STDMETHOD_(int, GetScale)() const OVERRIDE;

    /**
     * @brief Enables or disables IME (Input Method Editor).
     *
     * @param bEnable TRUE to enable, FALSE to disable.
     */
    STDMETHOD_(void, EnableIME)(BOOL bEnable) OVERRIDE;

    /**
     * @brief Updates the cursor for the container.
     */
    STDMETHOD_(void, OnUpdateCursor)() OVERRIDE;

    /**
     * @brief Posts a task to the container.
     *
     * @param runable Pointer to the runnable task.
     * @param bAsync Flag indicating whether the task should be run asynchronously.
     * @return TRUE if the task was posted, FALSE otherwise.
     */
    STDMETHOD_(BOOL, PostTask)(THIS_ IRunnable *runable, BOOL bAsync DEF_VAL(TRUE)) OVERRIDE;

    /**
     * @brief Removes tasks for the specified object.
     *
     * @param pObj Pointer to the object.
     * @return Number of tasks removed.
     */
    STDMETHOD_(int, RemoveTasksForObject)(THIS_ void *pObj) OVERRIDE;
  protected:
    /**
     * @brief Creates a tooltip for the container.
     *
     * @return Pointer to the created tooltip.
     */
    virtual IToolTip *CreateTooltip() const;

    /**
     * @brief Destroys the specified tooltip.
     *
     * @param pTooltip Pointer to the tooltip to destroy.
     */
    virtual void DestroyTooltip(IToolTip *pTooltip) const;

  protected:
    /**
     * @brief Loads the layout from a resource ID.
     *
     * @param xmlDoc XML document.
     * @return TRUE if the layout was loaded successfully, FALSE otherwise.
     */
    virtual BOOL OnLoadLayoutFromResourceID(SXmlDoc &xmlDoc);

    /**
     * @brief Gets the initial XML node.
     *
     * @param xmlDoc XML document.
     * @return Initial XML node.
     */
    virtual SXmlNode OnGetInitXmlNode(SXmlDoc &xmlDoc);

    /**
     * @brief Handles user-defined XML node.
     *
     * @param xmlUser User-defined XML node.
     */
    virtual void OnUserXmlNode(SXmlNode xmlUser);

    /**
     * @brief Creates the root window.
     *
     * @return Pointer to the created root window.
     */
    virtual SRootWindow *CreateRoot();

    /**
     * @brief Called when the scale of the window changes.
     * */
    virtual void OnScaleChanged(int nScale);
  public:
    /**
     * @brief Handles the resize event of the root window.
     *
     * @param e Pointer to the event arguments.
     * @return TRUE if the event was handled, FALSE otherwise.
     */
    virtual BOOL onRootResize(IEvtArgs *e);

  public: // 事件处理接口
    /**
     * @brief Handles an event.
     *
     * @param pEvt Pointer to the event arguments.
     * @return TRUE if the event was handled, FALSE otherwise.
     */
    virtual BOOL _HandleEvent(IEvtArgs *pEvt);

  protected:
    /**
     * @brief Handles the WM_SETLANGUAGE message.
     *
     * @param uMsg Message identifier.
     * @param wp WPARAM.
     * @param lp LPARAM.
     * @return LRESULT.
     */
    LRESULT OnSetLanguage(UINT uMsg, WPARAM wp, LPARAM lp);

    /**
     * @brief Handles the WM_UPDATEFONT message.
     *
     * @param uMsg Message identifier.
     * @param wp WPARAM.
     * @param lp LPARAM.
     * @return LRESULT.
     */
    LRESULT OnUpdateFont(UINT uMsg, WPARAM wp, LPARAM lp);

    /**
     * @brief Handles the UM_RUN_TASKS message.
     *
     * @param uMsg Message identifier.
     * @param wp WPARAM.
     * @param lp LPARAM.
     * @return LRESULT.
     */
    LRESULT OnRunTasks(UINT uMsg, WPARAM wp, LPARAM lp);

    BEGIN_MSG_MAP_EX(SHostWnd)
        MSG_WM_SIZE(OnSize)
        MSG_WM_PRINT(OnPrint)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_ACTIVATE(OnActivate)
        MSG_WM_SETFOCUS(OnSetFocus)
        MSG_WM_KILLFOCUS(OnKillFocus)
        MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST, WM_KEYLAST, OnKeyEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_IME_STARTCOMPOSITION, WM_IME_KEYLAST, OnKeyEvent)
        MESSAGE_HANDLER_EX(WM_IME_CHAR, OnKeyEvent)
        MESSAGE_HANDLER_EX(WM_IME_REQUEST, OnKeyEvent)
        MESSAGE_HANDLER_EX(WM_ACTIVATEAPP, OnActivateApp)
        MSG_WM_SETCURSOR(OnSetCursor)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
        MSG_WM_CAPTURECHANGED(OnCaptureChanged)
        MESSAGE_HANDLER_EX(UM_MENUEVENT, OnMenuExEvent)
        MSG_WM_WINDOWPOSCHANGING(OnWindowPosChanging)
        MSG_WM_WINDOWPOSCHANGED(OnWindowPosChanged)
        MESSAGE_HANDLER_EX(WM_GETOBJECT, OnGetObject)
        MSG_WM_COMMAND(OnCommand)
        MSG_WM_SYSCOMMAND(OnSysCommand)
        MESSAGE_HANDLER_EX(UM_UPDATEFONT, OnUpdateFont)
        MESSAGE_HANDLER_EX(UM_SETLANGUAGE, OnSetLanguage)
        MESSAGE_HANDLER_EX(UM_RUN_TASKS, OnRunTasks)
        CHAIN_MSG_MAP_MEMBER(*m_pNcPainter)
#if (!DISABLE_SWNDSPY)
        MESSAGE_HANDLER_EX(SPYMSG_SETSPY, OnSpyMsgSetSpy)
        MESSAGE_HANDLER_EX(SPYMSG_SWNDENUM, OnSpyMsgSwndEnum)
        MESSAGE_HANDLER_EX(SPYMSG_SWNDINFO, OnSpyMsgSwndSpy)
        MESSAGE_HANDLER_EX(SPYMSG_HITTEST, OnSpyMsgHitTest)
#endif // DISABLE_SWNDSPY
        REFLECT_NOTIFY_CODE(NM_CUSTOMDRAW)
    END_MSG_MAP()
};

SNSEND
#endif // __SHOSTWND__H__