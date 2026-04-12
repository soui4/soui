#include "souistd.h"
#include "msaa/SAccProxyCmnCtrl.h"
#include "control/SCmnCtrl.h"

SNSBEGIN
#ifdef SOUI_ENABLE_ACC

STDMETHODIMP SAccProxyButton::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
    return S_OK;
}

STDMETHODIMP SAccProxyCombobox::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_COMBOBOX;
    return S_OK;
}

STDMETHODIMP SAccProxyCheckButton::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_CHECKBUTTON;
    return S_OK;
}

STDMETHODIMP SAccProxyRadioButton::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_RADIOBUTTON;
    return S_OK;
}

STDMETHODIMP SAccProxyEdit::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_TEXT;
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////
SAccProxyProgress::SAccProxyProgress(IWindow *pWnd)
    : SAccProxyWindow(pWnd)
{
}

SAccProxyProgress::~SAccProxyProgress(void)
{
}

STDMETHODIMP SAccProxyProgress::get_accValue(BSTR *pszValue)
{
    SProgress *pProg = sobj_cast<SProgress>(m_pWnd);
    int nMin, nMax;
    pProg->GetRange(&nMin, &nMax);
    if (nMax == nMin)
        return E_FAIL;
    int nProg = (pProg->GetValue() - nMin) * 100 / (nMax - nMin);
    SStringW strValue = SStringW().Format(L"%d%%", nProg);
    *pszValue = ::SysAllocString(strValue);
    return S_OK;
}

STDMETHODIMP SAccProxyProgress::get_accName(BSTR *pszValue)
{
    return get_accValue(pszValue);
}

STDMETHODIMP SAccProxyProgress::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_PROGRESSBAR;
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SAccProxySlideBar::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_SLIDER;
    return S_OK;
}

STDMETHODIMP SAccProxyStatic::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_STATICTEXT;
    return S_OK;
}

STDMETHODIMP SAccProxyLink::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_LINK;
    return S_OK;
}

STDMETHODIMP SAccProxyImage::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_GRAPHIC;
    return S_OK;
}

STDMETHODIMP SAccProxyIcon::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_GRAPHIC;
    return S_OK;
}

STDMETHODIMP SAccProxyHrLine::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_SEPARATOR;
    return S_OK;
}

STDMETHODIMP SAccProxyGroup::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_GROUPING;
    return S_OK;
}

STDMETHODIMP SAccProxyActiveX::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_CLIENT;
    return S_OK;
}

STDMETHODIMP SAccProxyCalendar::get_accRole(VARIANT *pvarRole)
{
    // todo:hjx
    pvarRole->vt = VT_I4;
    // pvarRole->lVal = ROLE_SYSTEM_CALENDAR;
    return S_OK;
}

STDMETHODIMP SAccProxyCaption::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_TITLEBAR;
    return S_OK;
}

STDMETHODIMP SAccProxyListCtrl::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_LIST;
    return S_OK;
}

STDMETHODIMP SAccProxyDateTimePicker::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_COMBOBOX;
    return S_OK;
}

STDMETHODIMP SAccProxyDockBar::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_TOOLBAR;
    return S_OK;
}

STDMETHODIMP SAccProxyDropDown::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_COMBOBOX;
    return S_OK;
}

STDMETHODIMP SAccProxyFrame::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_CLIENT;
    return S_OK;
}

STDMETHODIMP SAccProxyHeaderCtrl::get_accRole(VARIANT *pvarRole)
{
    // todo:hjx
    pvarRole->vt = VT_I4;
    // pvarRole->lVal = ROLE_SYSTEM_LISTHEADER;
    return S_OK;
}

STDMETHODIMP SAccProxyHotKeyCtrl::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_TEXT;
    return S_OK;
}

STDMETHODIMP SAccProxyListView::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_LIST;
    return S_OK;
}

STDMETHODIMP SAccProxyListBox::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_LIST;
    return S_OK;
}

STDMETHODIMP SAccProxyMCListView::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_LIST;
    return S_OK;
}

STDMETHODIMP SAccProxyMenuBar::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_MENUBAR;
    return S_OK;
}

STDMETHODIMP SAccProxyRealWnd::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_CLIENT;
    return S_OK;
}

STDMETHODIMP SAccProxyScrollBar::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_SCROLLBAR;
    return S_OK;
}

STDMETHODIMP SAccProxySearchDropdownList::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_LIST;
    return S_OK;
}

STDMETHODIMP SAccProxySpinButtonCtrl::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_SPINBUTTON;
    return S_OK;
}

STDMETHODIMP SAccProxySplitWnd::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_PANE;
    return S_OK;
}

STDMETHODIMP SAccProxyStackView::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_PANE;
    return S_OK;
}

STDMETHODIMP SAccProxySwitch::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_CHECKBUTTON;
    return S_OK;
}

STDMETHODIMP SAccProxyTabCtrl::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_PAGETABLIST;
    return S_OK;
}

STDMETHODIMP SAccProxyTileView::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_LIST;
    return S_OK;
}

STDMETHODIMP SAccProxyToolBar::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_TOOLBAR;
    return S_OK;
}

STDMETHODIMP SAccProxyTreeCtrl::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_OUTLINE;
    return S_OK;
}

STDMETHODIMP SAccProxyTreeView::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_OUTLINE;
    return S_OK;
}

#endif // SOUI_ENABLE_ACC

SNSEND
