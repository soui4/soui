#ifndef __SACCPROXYCMNCTRL__H__
#define __SACCPROXYCMNCTRL__H__

#include "SAccProxyWindow.h"

SNSBEGIN

#ifdef SOUI_ENABLE_ACC

/**
 * @class SAccProxyButton
 * @brief Accessibility proxy class for button controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyButton : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyButton(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyCheckButton
 * @brief Accessibility proxy class for check button controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyCheckButton : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyCheckButton(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyRadioButton
 * @brief Accessibility proxy class for radio button controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyRadioButton : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyRadioButton(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyCombobox
 * @brief Accessibility proxy class for combo box controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyCombobox : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyCombobox(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyEdit
 * @brief Accessibility proxy class for edit controls.
 *        Inherits from SAccProxyWindow and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxyEdit : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyEdit(IWindow *pWnd)
        : SAccProxyWindow(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxyProgress
 * @brief Accessibility proxy class for progress controls.
 *        Inherits from SAccProxyWindow and overrides methods for name, value, and role.
 */
class SOUI_EXP SAccProxyProgress : public SAccProxyWindow {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyProgress(IWindow *pWnd);

    /**
     * @brief Destructor.
     */
    virtual ~SAccProxyProgress(void);

  public:
    /**
     * @brief Retrieves the name of the accessible object.
     * @param pszValue Pointer to receive the name of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accName(BSTR *pszValue);

    /**
     * @brief Retrieves the value of the accessible object.
     * @param pszValue Pointer to receive the value of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accValue(BSTR *pszValue);

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

/**
 * @class SAccProxySlideBar
 * @brief Accessibility proxy class for slider bar controls.
 *        Inherits from SAccProxyProgress and overrides the get_accRole method.
 */
class SOUI_EXP SAccProxySlideBar : public SAccProxyProgress {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxySlideBar(IWindow *pWnd)
        : SAccProxyProgress(pWnd)
    {
    }

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

#endif // SOUI_ENABLE_ACC

SNSEND

#endif // __SACCPROXYCMNCTRL__H__