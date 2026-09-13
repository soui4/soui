#include "souistd.h"
#include "msaa/SAccProxyCmnCtrl.h"
#include "control/SCmnCtrl.h"
#include "control/STabCtrl.h"
#include "control/SHeaderCtrl.h"
#include "control/SListbox.h"
#include "control/STreeCtrl.h"
#include "control/SListView.h"
#include "control/SMCListView.h"
#include "control/STileView.h"
#include "control/STreeView.h"

SNSBEGIN
#ifdef SOUI_ENABLE_ACC

STDMETHODIMP SAccProxyButton::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
    return S_OK;
}

STDMETHODIMP SAccProxyCombobox::get_accValue(BSTR *pszValue)
{
    SComboBox *pCombo = sobj_cast<SComboBox>(m_pWnd);
    int nCurSel = pCombo->GetCurSel();
    if (nCurSel < 0)
        return E_FAIL;

    SStringT strText;
    pCombo->GetItemText(nCurSel, FALSE, &strText);
    *pszValue = ::SysAllocString(S_CT2W(strText));
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

////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SAccProxySlideBar::get_accValue(BSTR *pszValue)
{
    SSliderBar *pSlider = sobj_cast<SSliderBar>(m_pWnd);
    int nValue = pSlider->GetValue();
    SStringW strValue = SStringW().Format(L"%d", nValue);
    *pszValue = ::SysAllocString(strValue);
    return S_OK;
}

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
    pvarRole->vt = VT_I4;
    // ROLE_SYSTEM_CALENDAR(0x2D) is undefined in the swinx compatibility header oleacc.h and conflicts with the local
    // RADIOBUTTON=0x2D, so CLIENT is used as the calendar control role to identify this control.
    pvarRole->lVal = ROLE_SYSTEM_CLIENT;
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
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_CLIENT;
    return S_OK;
}

/** HeaderCtrl: column headers are drawn sub-items -- expose as virtual children. */
int STDMETHODCALLTYPE SAccProxyHeaderCtrl::GetAccSubItemCount()
{
    SHeaderCtrl *pHeader = sobj_cast<SHeaderCtrl>(m_pWnd);
    return pHeader ? (int)pHeader->GetItemCount() : 0;
}

BSTR STDMETHODCALLTYPE SAccProxyHeaderCtrl::GetAccSubItemName(int iChild)
{
    SHeaderCtrl *pHeader = sobj_cast<SHeaderCtrl>(m_pWnd);
    if (!pHeader)
        return NULL;
    SHDITEM hd;
    memset(&hd, 0, sizeof(hd));
    TCHAR szText[256];
    hd.mask = SHDI_TEXT;
    hd.pszText = szText;
    hd.cchMaxText = sizeof(szText) / sizeof(szText[0]);
    if (!pHeader->GetItem(iChild - 1, &hd))
        return NULL;
    return ::SysAllocString(S_CT2W(hd.pszText));
}

long STDMETHODCALLTYPE SAccProxyHeaderCtrl::GetAccSubItemRole(int iChild)
{
    return ROLE_SYSTEM_COLUMNHEADER;
}

DWORD STDMETHODCALLTYPE SAccProxyHeaderCtrl::GetAccSubItemState(int iChild)
{
    return STATE_SYSTEM_FOCUSABLE | STATE_SYSTEM_SELECTABLE;
}

void STDMETHODCALLTYPE SAccProxyHeaderCtrl::GetAccSubItemRect(int iChild, RECT *prc)
{
    prc->left = prc->top = prc->right = prc->bottom = 0;
    SHeaderCtrl *pHeader = sobj_cast<SHeaderCtrl>(m_pWnd);
    if (!pHeader)
        return;
    CRect rc = pHeader->GetItemRect(iChild - 1);
    prc->left = rc.left;
    prc->top = rc.top;
    prc->right = rc.right;
    prc->bottom = rc.bottom;
}

int STDMETHODCALLTYPE SAccProxyHeaderCtrl::GetAccSubItemSel()
{
    return -1;
}

STDMETHODIMP SAccProxyHeaderCtrl::SetAccSubItemSel(int iChild)
{
    return E_NOTIMPL;
}

int STDMETHODCALLTYPE SAccProxyHeaderCtrl::HitTestAccSubItem(long x, long y)
{
    SHeaderCtrl *pHeader = sobj_cast<SHeaderCtrl>(m_pWnd);
    if (!pHeader)
        return -1;
    CPoint pt(x, y);
    int count = (int)pHeader->GetItemCount();
    for (int i = 0; i < count; i++)
    {
        if (pHeader->GetItemRect(i).PtInRect(pt))
            return i + 1;
    }
    return -1;
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

/** ListBox: items are drawn sub-items -- expose them as virtual children. */
int STDMETHODCALLTYPE SAccProxyListBox::GetAccSubItemCount()
{
    SListBox *pList = sobj_cast<SListBox>(m_pWnd);
    return pList ? pList->GetCount() : 0;
}

BSTR STDMETHODCALLTYPE SAccProxyListBox::GetAccSubItemName(int iChild)
{
    SListBox *pList = sobj_cast<SListBox>(m_pWnd);
    if (!pList)
        return NULL;
    int i = iChild - 1;
    if (i < 0 || i >= pList->GetCount())
        return NULL;
    SStringT str = pList->GetText(i, FALSE);
    return ::SysAllocString(S_CT2W(str));
}

long STDMETHODCALLTYPE SAccProxyListBox::GetAccSubItemRole(int iChild)
{
    return ROLE_SYSTEM_LISTITEM;
}

DWORD STDMETHODCALLTYPE SAccProxyListBox::GetAccSubItemState(int iChild)
{
    SListBox *pList = sobj_cast<SListBox>(m_pWnd);
    DWORD dwState = STATE_SYSTEM_FOCUSABLE | STATE_SYSTEM_SELECTABLE;
    if (pList && iChild - 1 == pList->GetCurSel())
        dwState |= STATE_SYSTEM_SELECTED;
    return dwState;
}

void STDMETHODCALLTYPE SAccProxyListBox::GetAccSubItemRect(int iChild, RECT *prc)
{
    prc->left = prc->top = prc->right = prc->bottom = 0;
    SListBox *pList = sobj_cast<SListBox>(m_pWnd);
    if (!pList)
        return;
    int i = iChild - 1;
    if (i < 0 || i >= pList->GetCount())
        return;
    CRect rcClient;
    pList->GetClientRect(&rcClient);
    int nItemHei = pList->GetItemHeight();
    CRect rcItem(0, 0, rcClient.Width(), nItemHei);
    rcItem.OffsetRect(0, nItemHei * i - pList->m_siVer.nPos);
    rcItem.OffsetRect(rcClient.TopLeft());
    prc->left = rcItem.left;
    prc->top = rcItem.top;
    prc->right = rcItem.right;
    prc->bottom = rcItem.bottom;
}

int STDMETHODCALLTYPE SAccProxyListBox::GetAccSubItemSel()
{
    SListBox *pList = sobj_cast<SListBox>(m_pWnd);
    if (!pList)
        return -1;
    int s = pList->GetCurSel();
    return s >= 0 ? s + 1 : -1;
}

STDMETHODIMP SAccProxyListBox::SetAccSubItemSel(int iChild)
{
    SListBox *pList = sobj_cast<SListBox>(m_pWnd);
    if (!pList)
        return E_INVALIDARG;
    return pList->SetCurSel(iChild - 1, TRUE) ? S_OK : E_INVALIDARG;
}

int STDMETHODCALLTYPE SAccProxyListBox::HitTestAccSubItem(long x, long y)
{
    SListBox *pList = sobj_cast<SListBox>(m_pWnd);
    if (!pList)
        return -1;
    CRect rcClient;
    pList->GetClientRect(&rcClient);
    int nItemHei = pList->GetItemHeight();
    if (nItemHei <= 0)
        return -1;
    int i = (y - rcClient.top + pList->m_siVer.nPos) / nItemHei;
    if (i < 0 || i >= pList->GetCount())
        return -1;
    return i + 1;
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

STDMETHODIMP SAccProxySpinButtonCtrl::get_accValue(BSTR *pszValue)
{
    SSpinButtonCtrl *pSpin = sobj_cast<SSpinButtonCtrl>(m_pWnd);
    int nValue = pSpin->GetValue();
    SStringW strValue = SStringW().Format(L"%d", nValue);
    *pszValue = ::SysAllocString(strValue);
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

/** TabCtrl: tab headers are drawn sub-items -- expose as virtual children. */
int STDMETHODCALLTYPE SAccProxyTabCtrl::GetAccSubItemCount()
{
    STabCtrl *pTab = sobj_cast<STabCtrl>(m_pWnd);
    return pTab ? pTab->GetItemCount() : 0;
}

BSTR STDMETHODCALLTYPE SAccProxyTabCtrl::GetAccSubItemName(int iChild)
{
    STabCtrl *pTab = sobj_cast<STabCtrl>(m_pWnd);
    if (!pTab)
        return NULL;
    STabPage *pPage = pTab->GetItem(iChild - 1);
    if (!pPage)
        return NULL;
    return ::SysAllocString(S_CT2W(pPage->GetTitle()));
}

long STDMETHODCALLTYPE SAccProxyTabCtrl::GetAccSubItemRole(int iChild)
{
    return ROLE_SYSTEM_PAGETAB;
}

DWORD STDMETHODCALLTYPE SAccProxyTabCtrl::GetAccSubItemState(int iChild)
{
    STabCtrl *pTab = sobj_cast<STabCtrl>(m_pWnd);
    DWORD dwState = STATE_SYSTEM_FOCUSABLE | STATE_SYSTEM_SELECTABLE;
    if (pTab && iChild - 1 == pTab->GetCurSel())
        dwState |= STATE_SYSTEM_SELECTED;
    return dwState;
}

void STDMETHODCALLTYPE SAccProxyTabCtrl::GetAccSubItemRect(int iChild, RECT *prc)
{
    prc->left = prc->top = prc->right = prc->bottom = 0;
    STabCtrl *pTab = sobj_cast<STabCtrl>(m_pWnd);
    if (!pTab)
        return;
    CRect rcItem;
    if (pTab->GetItemRect(iChild - 1, rcItem))
    {
        prc->left = rcItem.left;
        prc->top = rcItem.top;
        prc->right = rcItem.right;
        prc->bottom = rcItem.bottom;
    }
}

int STDMETHODCALLTYPE SAccProxyTabCtrl::GetAccSubItemSel()
{
    STabCtrl *pTab = sobj_cast<STabCtrl>(m_pWnd);
    if (!pTab)
        return -1;
    int s = pTab->GetCurSel();
    return s >= 0 ? s + 1 : -1;
}

STDMETHODIMP SAccProxyTabCtrl::SetAccSubItemSel(int iChild)
{
    STabCtrl *pTab = sobj_cast<STabCtrl>(m_pWnd);
    if (!pTab)
        return E_INVALIDARG;
    return pTab->SetCurSel(iChild - 1) ? S_OK : E_INVALIDARG;
}

int STDMETHODCALLTYPE SAccProxyTabCtrl::HitTestAccSubItem(long x, long y)
{
    STabCtrl *pTab = sobj_cast<STabCtrl>(m_pWnd);
    if (!pTab)
        return -1;
    CPoint pt(x, y);
    int count = pTab->GetItemCount();
    for (int i = 0; i < count; i++)
    {
        CRect rcItem;
        if (pTab->GetItemRect(i, rcItem) && rcItem.PtInRect(pt))
            return i + 1;
    }
    return -1;
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

/** TreeCtrl: tree items are drawn sub-items -- expose them as virtual children. */
int STDMETHODCALLTYPE SAccProxyTreeCtrl::GetAccSubItemCount()
{
    STreeCtrl *pTree = sobj_cast<STreeCtrl>(m_pWnd);
    return pTree ? pTree->GetAccItemCount() : 0;
}

BSTR STDMETHODCALLTYPE SAccProxyTreeCtrl::GetAccSubItemName(int iChild)
{
    STreeCtrl *pTree = sobj_cast<STreeCtrl>(m_pWnd);
    if (!pTree)
        return NULL;
    HSTREEITEM hItem = pTree->GetAccVisibleItem(iChild - 1);
    if (!hItem)
        return NULL;
    SStringT strText;
    if (!pTree->GetItemText(hItem, strText))
        return NULL;
    return ::SysAllocString(S_CT2W(strText));
}

long STDMETHODCALLTYPE SAccProxyTreeCtrl::GetAccSubItemRole(int iChild)
{
    return ROLE_SYSTEM_OUTLINEITEM;
}

DWORD STDMETHODCALLTYPE SAccProxyTreeCtrl::GetAccSubItemState(int iChild)
{
    STreeCtrl *pTree = sobj_cast<STreeCtrl>(m_pWnd);
    DWORD dwState = STATE_SYSTEM_FOCUSABLE | STATE_SYSTEM_SELECTABLE;
    if (!pTree)
        return dwState;
    HSTREEITEM hItem = pTree->GetAccVisibleItem(iChild - 1);
    if (!hItem)
        return dwState;
    if (pTree->GetAccItemExpanded(hItem))
        dwState |= STATE_SYSTEM_EXPANDED;
    if (pTree->GetSelectedItem() == hItem)
        dwState |= STATE_SYSTEM_SELECTED;
    return dwState;
}

void STDMETHODCALLTYPE SAccProxyTreeCtrl::GetAccSubItemRect(int iChild, RECT *prc)
{
    prc->left = prc->top = prc->right = prc->bottom = 0;
    STreeCtrl *pTree = sobj_cast<STreeCtrl>(m_pWnd);
    if (!pTree)
        return;
    HSTREEITEM hItem = pTree->GetAccVisibleItem(iChild - 1);
    if (!hItem)
        return;
    CRect rcItem;
    if (pTree->GetAccItemRect(hItem, rcItem))
    {
        prc->left = rcItem.left;
        prc->top = rcItem.top;
        prc->right = rcItem.right;
        prc->bottom = rcItem.bottom;
    }
}

int STDMETHODCALLTYPE SAccProxyTreeCtrl::GetAccSubItemSel()
{
    STreeCtrl *pTree = sobj_cast<STreeCtrl>(m_pWnd);
    if (!pTree)
        return -1;
    HSTREEITEM hSel = pTree->GetSelectedItem();
    if (!hSel)
        return -1;
    CRect rcDummy;
    for (int i = 0; i < pTree->GetAccItemCount(); i++)
    {
        if (pTree->GetAccVisibleItem(i) == hSel)
            return i + 1;
    }
    return -1;
}

STDMETHODIMP SAccProxyTreeCtrl::SetAccSubItemSel(int iChild)
{
    STreeCtrl *pTree = sobj_cast<STreeCtrl>(m_pWnd);
    if (!pTree)
        return E_INVALIDARG;
    HSTREEITEM hItem = pTree->GetAccVisibleItem(iChild - 1);
    if (!hItem)
        return E_INVALIDARG;
    return pTree->SelectItem(hItem) ? S_OK : E_INVALIDARG;
}

int STDMETHODCALLTYPE SAccProxyTreeCtrl::HitTestAccSubItem(long x, long y)
{
    STreeCtrl *pTree = sobj_cast<STreeCtrl>(m_pWnd);
    if (!pTree)
        return -1;
    CPoint pt(x, y);
    int nCount = pTree->GetAccItemCount();
    for (int i = 0; i < nCount; i++)
    {
        CRect rcItem;
        if (pTree->GetAccItemRect(pTree->GetAccVisibleItem(i), rcItem) && rcItem.PtInRect(pt))
            return i + 1;
    }
    return -1;
}

STDMETHODIMP SAccProxyTreeView::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_OUTLINE;
    return S_OK;
}

///////////////////////////////////////////////////////////////////////
/** MVC view proxies: expose the currently-visible item panels as real child */
/** windows (AccChild), just like the underlying views do. */
///////////////////////////////////////////////////////////////////////
int STDMETHODCALLTYPE SAccProxyListView::GetAccChildCount()
{
    SListView *pView = sobj_cast<SListView>(m_pWnd);
    return pView ? pView->GetVisibleAccChildCount() : 0;
}

IWindow *STDMETHODCALLTYPE SAccProxyListView::GetAccChild(int iChild)
{
    SListView *pView = sobj_cast<SListView>(m_pWnd);
    return pView ? pView->GetVisibleAccChild(iChild) : NULL;
}

int STDMETHODCALLTYPE SAccProxyListView::GetAccSelIndex()
{
    SListView *pView = sobj_cast<SListView>(m_pWnd);
    return pView ? pView->GetVisibleAccSelIndex() : 0;
}

int STDMETHODCALLTYPE SAccProxyMCListView::GetAccChildCount()
{
    SMCListView *pView = sobj_cast<SMCListView>(m_pWnd);
    return pView ? pView->GetVisibleAccChildCount() : 0;
}

IWindow *STDMETHODCALLTYPE SAccProxyMCListView::GetAccChild(int iChild)
{
    SMCListView *pView = sobj_cast<SMCListView>(m_pWnd);
    return pView ? pView->GetVisibleAccChild(iChild) : NULL;
}

int STDMETHODCALLTYPE SAccProxyMCListView::GetAccSelIndex()
{
    SMCListView *pView = sobj_cast<SMCListView>(m_pWnd);
    return pView ? pView->GetVisibleAccSelIndex() : 0;
}

int STDMETHODCALLTYPE SAccProxyTileView::GetAccChildCount()
{
    STileView *pView = sobj_cast<STileView>(m_pWnd);
    return pView ? pView->GetVisibleAccChildCount() : 0;
}

IWindow *STDMETHODCALLTYPE SAccProxyTileView::GetAccChild(int iChild)
{
    STileView *pView = sobj_cast<STileView>(m_pWnd);
    return pView ? pView->GetVisibleAccChild(iChild) : NULL;
}

int STDMETHODCALLTYPE SAccProxyTileView::GetAccSelIndex()
{
    STileView *pView = sobj_cast<STileView>(m_pWnd);
    return pView ? pView->GetVisibleAccSelIndex() : 0;
}

int STDMETHODCALLTYPE SAccProxyTreeView::GetAccChildCount()
{
    STreeView *pView = sobj_cast<STreeView>(m_pWnd);
    return pView ? pView->GetVisibleAccChildCount() : 0;
}

IWindow *STDMETHODCALLTYPE SAccProxyTreeView::GetAccChild(int iChild)
{
    STreeView *pView = sobj_cast<STreeView>(m_pWnd);
    return pView ? pView->GetVisibleAccChild(iChild) : NULL;
}

int STDMETHODCALLTYPE SAccProxyTreeView::GetAccSelIndex()
{
    STreeView *pView = sobj_cast<STreeView>(m_pWnd);
    return pView ? pView->GetVisibleAccSelIndex() : 0;
}

#endif /**< SOUI_ENABLE_ACC */

SNSEND
