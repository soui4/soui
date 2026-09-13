#ifndef __SHOSTWND_I__H__
#define __SHOSTWND_I__H__
#include <interface/SNativeWnd-i.h>
#include <interface/SNcPainter-i.h>
#include <interface/SMsgLoop-i.h>

SNSBEGIN

typedef struct IWindow IWindow;
typedef struct IApplication IApplication;
typedef struct IHostPresenter IHostPresenter;
typedef struct IHostWnd IHostWnd;
typedef struct _EventHandlerInfo
{
    FunCallback fun;
    void *ctx;
} EventHandlerInfo;

#undef INTERFACE
#define INTERFACE IHostWnd
DECLARE_INTERFACE_(IHostWnd, INativeWnd)
{
#include "SNativeWndApi.h"
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Create window
     * @param hWndParent parent window
     * @param dwStyle style
     * @param dwExStyle exStyle
     * @param x
     * @param y
     * @param nWidth
     * @param nHeight
     * @param xmlInit init xml node
     * @return HWND
     */
    STDMETHOD_(HWND, CreateEx)
    (THIS_ HWND hWndParent, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, IXmlNode *xmlInit DEF_VAL(NULL)) PURE;

    /**
     * @brief Create window
     * @param hWndParent
     * @param x
     * @param y
     * @param nWidth
     * @param nHeight
     * @return HWND
     */
    STDMETHOD_(HWND, Create)
    (THIS_ HWND hWndParent, int x DEF_VAL(0), int y DEF_VAL(0), int nWidth DEF_VAL(0), int nHeight DEF_VAL(0)) PURE;

    /**
     * @brief Attaches the host window to an existing external HWND.
     *
     * Instead of creating a new HWND via Create/CreateEx, this method subclasses the
     * provided external HWND and runs the same SOUI initialization flow (loading the
     * layout XML, building the SOUI window tree, and hooking up the message map so
     * that SOUI messages are routed correctly).
     *
     * Unlike Create/CreateEx, when the SHostWnd is later destroyed via DestroyWindow,
     * it only unsubclasses the external HWND and performs internal SOUI cleanup. The
     * external HWND itself is NOT destroyed and remains fully owned by its original
     * creator.
     *
     * @param hWnd Handle to an existing, valid window to attach SOUI to.
     * @param xmlInit Optional XML node used for initialization. Same semantics as the
     *                xmlInit parameter of CreateEx. If NULL, the layout configured via
     *                SetLayoutId / the constructor is used instead.
     * @return The attached HWND (same as @p hWnd) on success, or NULL on failure.
     *         Failures include an invalid @p hWnd, the SHostWnd already being bound
     *         to a window, or a failure during subclassing or SOUI initialization.
     */
    STDMETHOD_(BOOL, Attach)(THIS_ HWND hWnd, IXmlNode * xmlInit DEF_VAL(NULL)) PURE;

    /**
     * @brief detach from external HWND
     */
    STDMETHOD_(BOOL, Detach)(THIS) PURE;

    /**
     * @brief Set the window layout resource ID
     * @param pszLayoutId layout resource ID
     * @return
     */
    STDMETHOD_(void, SetLayoutId)(THIS_ LPCTSTR pszLayoutId) PURE;

    /**
     * @brief Initialize window from XML
     * @param pNode XML data
     * @return TRUE-success
     */
    STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode * pNode) PURE;

    /**
     * @brief Get Dui Root
     * @return Dui Root
     */
    STDMETHOD_(IWindow *, GetIRoot)(THIS) PURE;

    /**
     * @brief Query the window's semi-transparent flag
     * @return TRUE-window is semi-transparent
     */
    STDMETHOD_(BOOL, IsTranslucent)(CTHIS) SCONST PURE;

    /**
     * @brief Get the window's presenter object
     * @return IHostPresenter* - presenter object
     */
    STDMETHOD_(IHostPresenter *, GetPresenter)(THIS) PURE;

    /**
     * @brief Set the window's presenter object
     * @param pPresenter presenter object
     * @return
     */
    STDMETHOD_(void, SetPresenter)(THIS_ IHostPresenter * pPresenter) PURE;

    /**
     * @brief Get the msgloop object the window belongs to
     * @param
     * @return
     */
    STDMETHOD_(IMessageLoop *, GetMsgLoop)(THIS) PURE;

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
     * @brief Find a child window by Name
     * @param pszName LPCSTR--child window Name(utf8)
     * @return
     */
    STDMETHOD_(IWindow *, FindIChildByNameA)(THIS_ LPCSTR pszName) PURE;

    /**
     * @brief Get the non-client area painter object
     * @return INcPainter *
     */
    STDMETHOD_(INcPainter *, GetNcPainter)(THIS) PURE;

    /**
     * @brief Set the event handler object
     * @param fun event handler object
     * @param ctx event handler object Context
     * @return
     */
    STDMETHOD_(void, SetEventHandler)(THIS_ FunCallback fun, void *ctx) PURE;

    /**
     * @brief Get the event handler object
     * @return EventHandlerInfo*
     */
    STDMETHOD_(EventHandlerInfo *, GetEventHandler)(THIS) PURE;

    /**
     * @brief Animate show/hide window
     * @param dwTime
     * @param dwFlags
     * @return
     * @remark Refer to the API AnimateWindow
     */
    STDMETHOD_(BOOL, AnimateHostWindow)(THIS_ DWORD dwTime, DWORD dwFlags) PURE;

    /**
     * @brief Enable DragDrop support for the window
     * @return
     */
    STDMETHOD_(void, EnableDragDrop)(THIS) PURE;

    /**
     * @brief Show or hide the HostWnd
     * @param uShowCmd same as the ShowWindow parameter
     * @param bWaitAniDone flag to wait for the animation to complete when the window has enter/exit animations configured
     * @return
     */
    STDMETHOD_(void, ShowHostWnd)(THIS_ int uShowCmd, BOOL bWaitAniDone) PURE;

    /**
     * @brief Whether to enable the private UIDef object defined in the layout
     * @param BOOL bEnable TRUE-enable, FALSE-disable
     * @return void
     * @remark Enable it when the program needs to re-fetch data from the private UIDef, and disable it after use
     */
    STDMETHOD_(void, EnablePrivateUiDef)(THIS_ BOOL bEnable) PURE;

    /**
     * @brief Set the window's DPI scale
     * @param int nScale DPI scale, based on 100
     * @param LPCRECT pDestRect window coordinates after scaling
     * @return void
     */
    STDMETHOD_(void, SetScale)(THIS_ int nScale, LPCRECT pDestRect) PURE;
};

#undef INTERFACE
#define INTERFACE IHostDialog
DECLARE_INTERFACE_(IHostDialog, IHostWnd)
{
#include "SNativeWndApi.h"

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Create window
     * @param hWndParent parent window
     * @param dwStyle style
     * @param dwExStyle exStyle
     * @param x
     * @param y
     * @param nWidth
     * @param nHeight
     * @param xmlInit init xml node
     * @return HWND
     */
    STDMETHOD_(HWND, CreateEx)
    (THIS_ HWND hWndParent, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, IXmlNode *xmlInit DEF_VAL(NULL)) PURE;

    /**
     * @brief Create window
     * @param hWndParent
     * @param x
     * @param y
     * @param nWidth
     * @param nHeight
     * @return HWND
     */
    STDMETHOD_(HWND, Create)
    (THIS_ HWND hWndParent, int x DEF_VAL(0), int y DEF_VAL(0), int nWidth DEF_VAL(0), int nHeight DEF_VAL(0)) PURE;

    /**
     * @brief Set the window layout resource ID
     * @param pszLayoutId layout resource ID
     * @return
     */
    STDMETHOD_(void, SetLayoutId)(THIS_ LPCTSTR pszLayoutId) PURE;

    /**
     * @brief Initialize window from XML
     * @param pNode XML data
     * @return TRUE-success
     */
    STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode * pNode) PURE;

    /**
     * @brief Get Dui Root
     * @return Dui Root
     */
    STDMETHOD_(IWindow *, GetIRoot)(THIS) PURE;

    /**
     * @brief Query the window's semi-transparent flag
     * @return TRUE-window is semi-transparent
     */
    STDMETHOD_(BOOL, IsTranslucent)(CTHIS) SCONST PURE;

    /**
     * @brief Get the window's presenter object
     * @return IHostPresenter* - presenter object
     */
    STDMETHOD_(IHostPresenter *, GetPresenter)(THIS) PURE;

    /**
     * @brief Set the window's presenter object
     * @param pPresenter presenter object
     * @return
     */
    STDMETHOD_(void, SetPresenter)(THIS_ IHostPresenter * pPresenter) PURE;

    /**
     * @brief Get the msgloop object the window belongs to
     * @param
     * @return
     */
    STDMETHOD_(IMessageLoop *, GetMsgLoop)(THIS) PURE;

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
     * @brief Find a child window by Name
     * @param pszName LPCSTR--child window Name(utf8)
     * @return
     */
    STDMETHOD_(IWindow *, FindIChildByNameA)(THIS_ LPCSTR pszName) PURE;

    /**
     * @brief Get the non-client area painter object
     * @return INcPainter *
     */
    STDMETHOD_(INcPainter *, GetNcPainter)(THIS) PURE;

    /**
     * @brief Set the event handler object
     * @param fun event handler object
     * @param ctx event handler object Context
     * @return
     */
    STDMETHOD_(void, SetEventHandler)(THIS_ FunCallback fun, void *ctx) PURE;

    /**
     * @brief Get the event handler object
     * @return EventHandlerInfo*
     */
    STDMETHOD_(EventHandlerInfo *, GetEventHandler)(THIS) PURE;

    /**
     * @brief Animate show/hide window
     * @param dwTime
     * @param dwFlags
     * @return
     * @remark Refer to the API AnimateWindow
     */
    STDMETHOD_(BOOL, AnimateHostWindow)(THIS_ DWORD dwTime, DWORD dwFlags) PURE;

    /**
     * @brief Enable DragDrop support for the window
     * @return
     */
    STDMETHOD_(void, EnableDragDrop)(THIS) PURE;

    /**
     * @brief Show or hide the HostWnd
     * @param uShowCmd same as the ShowWindow parameter
     * @param bWaitAniDone flag to wait for the animation to complete when the window has enter/exit animations configured
     * @return
     */
    STDMETHOD_(void, ShowHostWnd)(THIS_ int uShowCmd, BOOL bWaitAniDone) PURE;

    /**
     * @brief Whether to enable the private UIDef object defined in the layout
     * @param BOOL bEnable TRUE-enable, FALSE-disable
     * @return void
     * @remark Enable it when the program needs to re-fetch data from the private UIDef, and disable it after use
     */
    STDMETHOD_(void, EnablePrivateUiDef)(THIS_ BOOL bEnable) PURE;

    /**
     * @brief Set the window's DPI scale
     * @param int nScale DPI scale, based on 100
     * @param LPCRECT pDestRect window coordinates after scaling
     * @return void
     */
    STDMETHOD_(void, SetScale)(THIS_ int nScale, LPCRECT pDestRect) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Start a modal window
     * @param hParent window's owner
     * @return
     */
    STDMETHOD_(INT_PTR, DoModal)(THIS_ HWND hParent DEF_VAL(NULL), DWORD dwStyle DEF_VAL(WS_POPUP | WS_CLIPCHILDREN), DWORD dwExStyle DEF_VAL(0)) PURE;

    /**
     * @brief Exit the current modal window
     * @param nResult return value of DoModal
     * @return
     */
    STDMETHOD_(void, EndDialog)(THIS_ INT_PTR nResult) PURE;
};

SNSEND

#endif /**< __SHOSTWND_I__H__ */