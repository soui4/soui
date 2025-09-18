#ifndef __SREALWND__H__
#define __SREALWND__H__

#include <core/SWnd.h>
#include <interface/SCtrls-i.h>
#include <proxy/SWindowProxy.h>

//////////////////////////////////////////////////////////////////////////
// Real Window Control
// Binding a real window
//
// Usage: <realwnd id=xx wndclass="edit" wndname="name" style="00000001" exstyle="00000000"/>
//

SNSBEGIN

/**
 * @class SRealWnd
 * @brief Real Window Control
 * @details A control that binds a real Windows window.
 */
class SOUI_EXP SRealWnd : public TWindowProxy<IRealWnd> {
    DEF_SOBJECT(SWindow, L"realwnd")

  public:
    /**
     * @brief Constructor
     */
    SRealWnd();

    /**
     * @brief Destructor
     */
    virtual ~SRealWnd();

  public:
    /**
     * @brief Get the class name of the real window
     * @return Pointer to the class name
     */
    STDMETHOD_(const IStringT *, GetRealClassName)(CTHIS) SCONST OVERRIDE
    {
        return &m_strClassName;
    }

    /**
     * @brief Get the name of the real window
     * @return Pointer to the window name
     */
    STDMETHOD_(const IStringT *, GetRealWindowName)(CTHIS) SCONST OVERRIDE
    {
        return &m_strWindowName;
    }

    /**
     * @brief Get the style of the real window
     * @return Window style
     */
    STDMETHOD_(DWORD, GetRealStyle)(CTHIS) SCONST OVERRIDE
    {
        return m_dwStyle;
    }

    /**
     * @brief Get the extended style of the real window
     * @return Extended window style
     */
    STDMETHOD_(DWORD, GetRealStyleEx)(CTHIS) SCONST OVERRIDE
    {
        return m_dwExStyle;
    }

    /**
     * @brief Get the parameters of the real window
     * @return Pointer to the XML node containing parameters
     */
    STDMETHOD_(IXmlNode *, GetRealParam)(CTHIS) OVERRIDE
    {
        return m_xmlParams.Root();
    }

    /**
     * @brief Get the handle of the real window
     * @param bAutoCreate Whether to automatically create the window if it doesn't exist
     * @return Handle to the real window
     */
    STDMETHOD_(HWND, GetRealHwnd)(THIS_ BOOL bAutoCreate = TRUE) OVERRIDE;

    /**
     * @brief Set the additional data for the real window
     * @param lpData Pointer to the additional data
     */
    STDMETHOD_(void, SetData)(THIS_ LPVOID lpData)
    {
        m_lpData = lpData;
    }

    /**
     * @brief Get the additional data for the real window
     * @return Pointer to the additional data
     */
    STDMETHOD_(LPVOID, GetData)(THIS)
    {
        return m_lpData;
    }

    SOUI_ATTRS_BEGIN()
        ATTR_STRINGT(L"wndclass", m_strClassName, FALSE) /**< Class name of the real window */
        ATTR_STRINGT(L"wndname", m_strWindowName, FALSE) /**< Name of the real window */
        ATTR_HEX(L"style", m_dwStyle, FALSE)             /**< Style of the real window */
        ATTR_HEX(L"exstyle", m_dwExStyle, FALSE)         /**< Extended style of the real window */
        ATTR_BOOL(L"init", m_bInit, FALSE)               /**< Initialization flag */
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Determine if the control needs to be redrawn when its state changes
     * @return TRUE if redraw is needed, FALSE otherwise
     */
    virtual BOOL NeedRedrawWhenStateChange();

    /**
     * @brief Initialize the control from an XML node
     * @param pNode XML node containing initialization parameters
     * @return TRUE if initialization is successful, FALSE otherwise
     */
    virtual BOOL WINAPI InitFromXml(IXmlNode *pNode);

    /**
     * @brief Handle layout changes
     * @param rcWnd New window rectangle
     * @return TRUE if layout is successful, FALSE otherwise
     */
    virtual BOOL OnRelayout(const CRect &rcWnd);

    /**
     * @brief Handle show window event
     * @param bShow Whether to show the window
     * @param nStatus Status code
     */
    void OnShowWindow(BOOL bShow, UINT nStatus);

    /**
     * @brief Handle destroy event
     */
    void OnDestroy();

    /**
     * @brief Paint the control
     * @param pRT Rendering target handle
     */
    void OnPaint(IRenderTarget *pRT)
    {
        // Implementation can be added here if needed
    }

    /**
     * @brief Show the real window
     */
    void ShowRealWindow();

    /**
     * @brief Initialize the real window
     * @return TRUE if initialization is successful, FALSE otherwise
     */
    BOOL InitRealWnd();

    /**
     * @brief Set the position of the real window
     * @param hRealWnd Handle to the real window
     * @param prc Rectangle for the window position
     */
    void SetRealWndPos(HWND hRealWnd, const CRect *prc);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SHOWWINDOW(OnShowWindow)
    SOUI_MSG_MAP_END()

  protected:
    SStringT m_strClassName;  /**< Class name of the real window */
    SStringT m_strWindowName; /**< Name of the real window */
    DWORD m_dwStyle;          /**< Style of the real window */
    DWORD m_dwExStyle;        /**< Extended style of the real window */
    SXmlDoc m_xmlParams;      /**< XML document containing parameters */

    BOOL m_bInit; /**< Initialization flag */

    HWND m_hRealWnd; /**< Handle to the real window */
    LPVOID m_lpData; /**< Pointer to additional data */

    SAutoRefPtr<IRealWndHandler> m_pRealWndHandler; /**< Pointer to the real window handler */
};

SNSEND

#endif // __SREALWND__H__