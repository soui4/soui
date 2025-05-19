#include "souistd.h"
#include "msaa/SAccProxyWindow.h"

SNSBEGIN
#ifdef SOUI_ENABLE_ACC
/////////////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SAccProxyWindow::get_accName(BSTR *pszName)
{
    SStringA strTextA;
    m_pWnd->GetWindowTextU8(&strTextA, TRUE);
    if (strTextA.IsEmpty())
        return E_INVALIDARG;
    SStringW strText = S_CA2W(strTextA, CP_UTF8);
    *pszName = ::SysAllocString(strText);
    return S_OK;
}

STDMETHODIMP SAccProxyWindow::get_accValue(BSTR *pszValue)
{
    SStringA strText;
    m_pWnd->GetWindowTextU8(&strText, TRUE);
    *pszValue = ::SysAllocString(S_CA2W(strText, CP_UTF8));
    return S_OK;
}

STDMETHODIMP SAccProxyWindow::get_accDescription(BSTR *pszDescription)
{
    return E_NOTIMPL;
}

STDMETHODIMP SAccProxyWindow::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_WINDOW;
    return S_OK;
}

STDMETHODIMP SAccProxyWindow::get_accState(VARIANT *pvarState)
{
    DWORD dwState = m_pWnd->GetState();

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;
    if (!m_pWnd->IsVisible(TRUE))
        pvarState->lVal = STATE_SYSTEM_INVISIBLE;
    else if (m_pWnd->IsDisabled(TRUE))
        pvarState->lVal = STATE_SYSTEM_UNAVAILABLE;
    else
    {
        if (dwState & WndState_PushDown)
            pvarState->lVal = STATE_SYSTEM_PRESSED;
        if (dwState & WndState_Check)
            pvarState->lVal |= STATE_SYSTEM_CHECKED;

        if (m_pWnd->GetContainer()->GetFocus() == m_pWnd->GetSwnd())
            pvarState->lVal |= STATE_SYSTEM_FOCUSED;
        if (m_pWnd->IsFocusable())
            pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;
    }

    return S_OK;
}

STDMETHODIMP SAccProxyWindow::get_accHelp(BSTR *pszHelp)
{
    return E_NOTIMPL;
}
STDMETHODIMP SAccProxyWindow::get_accHelpTopic(BSTR *pszHelpFile, long *pidTopic)
{
    return E_NOTIMPL;
}
STDMETHODIMP SAccProxyWindow::get_accKeyboardShortcut(BSTR *pszKeyboardShortcut)
{
    return E_NOTIMPL;
}
STDMETHODIMP SAccProxyWindow::get_accDefaultAction(BSTR *pszDefaultAction)
{
    *pszDefaultAction = ::SysAllocString(L"cmd");
    return S_OK;
}

STDMETHODIMP SAccProxyWindow::accSelect(long flagsSelect)
{
    if (((flagsSelect & SELFLAG_TAKEFOCUS) > 0) && (::GetFocus() == m_pWnd->GetContainer()->GetHostHwnd()))
    {
        m_pWnd->SetFocus();
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

STDMETHODIMP SAccProxyWindow::accDoDefaultAction()
{
    m_pWnd->FireCommand();
    return S_OK;
}
STDMETHODIMP SAccProxyWindow::put_accName(BSTR szName)
{
    m_pWnd->SetWindowText(S_CW2T(szName));
    return S_OK;
}

STDMETHODIMP SAccProxyWindow::put_accValue(BSTR szValue)
{
    m_pWnd->SetWindowText(S_CW2T(szValue));
    return S_OK;
}

#endif
SNSEND