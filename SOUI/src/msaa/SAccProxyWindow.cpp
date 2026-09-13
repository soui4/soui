#include "souistd.h"
#include "msaa/SAccProxyWindow.h"
#include "control/SCmnCtrl.h"

SNSBEGIN
#ifdef SOUI_ENABLE_ACC
//////////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SAccProxyWindow::get_accName(BSTR *pszName)
{
    SStringA strTextA;
    m_pWnd->GetWindowTextU8(&strTextA, TRUE);
    SStringW strText;
    if (strTextA.IsEmpty())
    {
        // No window text: fall back to the object type (class name) so that
        // elements such as "window"/"img" show their type instead of "(nothing)".
        LPCWSTR pszClass = m_pWnd->GetObjectClass();
        strText = pszClass ? pszClass : L"";
        if (strText.IsEmpty())
            return E_INVALIDARG;
    }
    else
    {
        strText = S_CA2W(strTextA, CP_UTF8);
    }
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
    // Check whether it is an SButton control, since only SButton supports shortcuts
    SButton *pButton = sobj_cast<SButton>(m_pWnd);
    if (pButton)
    {
        // Get the m_accel member of SButton
        DWORD dwAccel = pButton->GetAccel();
        if (dwAccel == 0)
            return E_NOTIMPL;
        SAccelerator accel(dwAccel);
        *pszKeyboardShortcut = ::SysAllocString(S_CT2W(accel.FormatHotkey()));
        return S_OK;
    }
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

///////////////////////////////////////////////////////////////////////
/** AccChild (real child window) support -- default: the window's regular */
/** child windows. MVC virtual-list controls override these to expose their */
/** currently-visible item panels instead. */
///////////////////////////////////////////////////////////////////////
int STDMETHODCALLTYPE SAccProxyWindow::GetAccChildCount()
{
    return m_pWnd ? m_pWnd->GetChildrenCount() : 0;
}

IWindow *STDMETHODCALLTYPE SAccProxyWindow::GetAccChild(int iChild)
{
    SWindow *pWnd = (SWindow *)m_pWnd;
    return pWnd ? pWnd->GetChild(iChild) : NULL;
}

int STDMETHODCALLTYPE SAccProxyWindow::GetAccSelIndex()
{
    return 0;
}

///////////////////////////////////////////////////////////////////////
/** AccSubItem (self-drawn sub-item) support -- default: none. */
/** Composite controls override these to expose their drawn sub-items. */
///////////////////////////////////////////////////////////////////////
int STDMETHODCALLTYPE SAccProxyWindow::GetAccSubItemCount()
{
    return 0;
}

BSTR STDMETHODCALLTYPE SAccProxyWindow::GetAccSubItemName(int iChild)
{
    return NULL;
}

long STDMETHODCALLTYPE SAccProxyWindow::GetAccSubItemRole(int iChild)
{
    return 0;
}

DWORD STDMETHODCALLTYPE SAccProxyWindow::GetAccSubItemState(int iChild)
{
    return 0;
}

void STDMETHODCALLTYPE SAccProxyWindow::GetAccSubItemRect(int iChild, RECT *prc)
{
    prc->left = prc->top = prc->right = prc->bottom = 0;
}

int STDMETHODCALLTYPE SAccProxyWindow::GetAccSubItemSel()
{
    return -1;
}

STDMETHODIMP SAccProxyWindow::SetAccSubItemSel(int iChild)
{
    return E_NOTIMPL;
}

int STDMETHODCALLTYPE SAccProxyWindow::HitTestAccSubItem(long x, long y)
{
    return -1;
}

#endif
SNSEND