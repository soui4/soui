#ifndef __SACTIVEX__H__
#define __SACTIVEX__H__

#include "core/SWnd.h"
#include <mshtmhst.h>

SNSBEGIN

class SAxContainerImpl;

/**
 * @class SActiveX
 * @brief Class for embedding ActiveX controls in SUI windows.
 * @details This class provides the functionality to embed and manage ActiveX controls within SUI windows.
 * It handles the creation, activation, and rendering of ActiveX controls.
 */
class SOUI_EXP SActiveX : public SWindow {
    friend class SAxContainerImpl;
    DEF_SOBJECT(SWindow, L"activex")
  public:
    /**
     * @brief Default constructor.
     */
    explicit SActiveX();

    /**
     * @brief Destructor.
     */
    virtual ~SActiveX();

    /**
     * @brief Retrieves the IUnknown interface of the ActiveX control.
     * @return Pointer to the IUnknown interface.
     */
    IUnknown *GetIUnknown();

  protected:
    /**
     * @brief Called when the ActiveX control is activated.
     * @param pUnknwn Pointer to the IUnknown interface of the ActiveX control.
     */
    virtual void OnAxActivate(IUnknown *pUnknwn)
    {
    }

    /**
     * @brief Called when the window is created.
     * @param lp Creation parameters.
     * @return Result of the creation process.
     */
    int OnCreate(LPVOID lp);

    /**
     * @brief Called when the window size changes.
     * @param nType Type of size change.
     * @param size New size of the window.
     */
    void OnSize(UINT nType, CSize size);

    /**
     * @brief Called when the window needs to be painted.
     * @param pRT Pointer to the rendering target.
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Called when a mouse event occurs.
     * @param uMsg Message identifier.
     * @param wp WPARAM value.
     * @param lp LPARAM value.
     * @return Result of the message processing.
     */
    LRESULT OnMouseEvent(UINT uMsg, WPARAM wp, LPARAM lp);

    /**
     * @brief Called when a keyboard event occurs.
     * @param uMsg Message identifier.
     * @param wp WPARAM value.
     * @param lp LPARAM value.
     * @return Result of the message processing.
     */
    LRESULT OnKeyEvent(UINT uMsg, WPARAM wp, LPARAM lp);

    /**
     * @brief Called when the window is shown or hidden.
     * @param bShow TRUE if the window is being shown, FALSE if hidden.
     * @param nStatus Status of the show operation.
     */
    void OnShowWindow(BOOL bShow, UINT nStatus);

    /**
     * @brief Retrieves the dialog code for the window.
     * @return Dialog code indicating the type of input the window wants.
     */
    virtual UINT WINAPI OnGetDlgCode() const OVERRIDE
    {
        return SC_WANTALLKEYS;
    }

    /**
     * @brief Checks if the window can receive focus.
     * @return TRUE if the window can receive focus, FALSE otherwise.
     */
    virtual BOOL WINAPI IsFocusable() const OVERRIDE
    {
        return TRUE;
    }

    /**
     * @brief Handles the "clsID" attribute.
     * @param strValue The CLSID value as a string.
     * @param bLoading TRUE if the attribute is being loaded, FALSE otherwise.
     * @return HRESULT indicating success or failure.
     */
    HRESULT OnAttrClsid(const SStringW &strValue, BOOL bLoading);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST, WM_KEYLAST, OnKeyEvent)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        MSG_WM_SHOWWINDOW(OnShowWindow)
    SOUI_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"clsID", OnAttrClsid)
        ATTR_DWORD(L"clsCtx", m_clsCtx, FALSE)
        ATTR_UINT(L"delay", m_bDelayInit, FALSE)
    SOUI_ATTRS_END()

    /**
     * @brief Called when the initialization of the ActiveX control is finished.
     */
    virtual void OnInitActiveXFinished()
    {
    }

    /**
     * @brief Initializes the ActiveX control.
     * @return TRUE if initialization is successful, FALSE otherwise.
     */
    BOOL InitActiveX();

    /**
     * @brief Sets the visibility of the ActiveX control.
     * @param bVisible TRUE to make the control visible, FALSE to hide it.
     */
    void SetActiveXVisible(BOOL bVisible);

    /**
     * @brief Sets the external UI handler for the ActiveX control.
     * @param pUiHandler Pointer to the IDocHostUIHandler interface.
     */
    void SetExternalUIHandler(IDocHostUIHandler *pUiHandler);

  protected:
    /**
     * @brief Pointer to the internal ActiveX container implementation.
     */
    SAxContainerImpl *m_axContainer;

    /**
     * @brief CLSID of the ActiveX control.
     */
    CLSID m_clsid;

    /**
     * @brief Context in which the ActiveX control should be created.
     */
    DWORD m_clsCtx;

    /**
     * @brief Flag indicating whether the initialization of the ActiveX control should be delayed.
     */
    BOOL m_bDelayInit;
};

SNSEND

#endif // __SACTIVEX__H__