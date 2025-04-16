#ifndef __SACCPROXYWINDOW__H__
#define __SACCPROXYWINDOW__H__

#include "interface/saccproxy-i.h"

SNSBEGIN

#ifdef SOUI_ENABLE_ACC

/**
 * @class SAccProxyWindow
 * @brief Accessibility proxy window class implementing IAccProxy interface.
 */
class SOUI_EXP SAccProxyWindow : public TObjRefImpl<IAccProxy> {
  public:
    /**
     * @brief Constructor.
     * @param pWnd Pointer to the associated window.
     */
    SAccProxyWindow(IWindow *pWnd)
        : m_pWnd(pWnd)
    {
    }

    /**
     * @brief Destructor.
     */
    virtual ~SAccProxyWindow()
    {
    }

  public:
    /**
     * @brief Retrieves the name of the accessible object.
     * @param pszName Pointer to receive the name of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accName(BSTR *pszName);

    /**
     * @brief Retrieves the value of the accessible object.
     * @param pszValue Pointer to receive the value of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accValue(BSTR *pszValue);

    /**
     * @brief Retrieves the description of the accessible object.
     * @param pszDescription Pointer to receive the description of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accDescription(BSTR *pszDescription);

    /**
     * @brief Retrieves the role of the accessible object.
     * @param pvarRole Pointer to receive the role of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);

    /**
     * @brief Retrieves the state of the accessible object.
     * @param pvarState Pointer to receive the state of the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accState(VARIANT *pvarState);

    /**
     * @brief Retrieves the help string for the accessible object.
     * @param pszHelp Pointer to receive the help string.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accHelp(BSTR *pszHelp);

    /**
     * @brief Retrieves the help topic for the accessible object.
     * @param pszHelpFile Pointer to receive the help file name.
     * @param pidTopic Pointer to receive the help topic ID.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accHelpTopic(BSTR *pszHelpFile, long *pidTopic);

    /**
     * @brief Retrieves the keyboard shortcut for the accessible object.
     * @param pszKeyboardShortcut Pointer to receive the keyboard shortcut.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accKeyboardShortcut(BSTR *pszKeyboardShortcut);

    /**
     * @brief Retrieves the default action for the accessible object.
     * @param pszDefaultAction Pointer to receive the default action.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP get_accDefaultAction(BSTR *pszDefaultAction);

    /**
     * @brief Selects the accessible object.
     * @param flagsSelect Flags specifying the selection action.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP accSelect(long flagsSelect);

    /**
     * @brief Performs the default action for the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP accDoDefaultAction();

    /**
     * @brief Sets the name of the accessible object.
     * @param szName New name for the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP put_accName(BSTR szName);

    /**
     * @brief Sets the value of the accessible object.
     * @param szValue New value for the accessible object.
     * @return HRESULT indicating success or failure.
     */
    virtual STDMETHODIMP put_accValue(BSTR szValue);

  protected:
    IWindow *m_pWnd; // Pointer to the associated window
};

#endif // SOUI_ENABLE_ACC

SNSEND

#endif // __SACCPROXYWINDOW__H__