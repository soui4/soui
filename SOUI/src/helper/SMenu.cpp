#include "souistd.h"
#include "SApp.h"
#include "helper/SMenu.h"
#include "helper/SplitString.h"
#include "gdialpha.h"
#include <souicoll.h>

SNSBEGIN

SMenuItemData::SMenuItemData()
    : iIcon(-1)
    , vHotKey(0)
    , dwUserData(0)
    , hIcon(NULL)
{
}

SMenuItemData::~SMenuItemData()
{
    if (hIcon)
        DestroyIcon(hIcon);
}

//////////////////////////////////////////////////////////////////////////

SMenuAttr::SMenuAttr()
    : m_pItemSkin(GETBUILTINSKIN(SKIN_SYS_MENU_SKIN))
    , m_pSepSkin(GETBUILTINSKIN(SKIN_SYS_MENU_SEP))
    , m_pCheckSkin(GETBUILTINSKIN(SKIN_SYS_MENU_CHECK))
    , m_pIconSkin(NULL)
    , m_nItemHei(25, SLayoutSize::defUnit)
    , m_nMaxWidth(250, SLayoutSize::defUnit)
    , m_nIconMargin(2, SLayoutSize::defUnit)
    , m_nTextMargin(5, SLayoutSize::defUnit)
    , m_scale(100)
{
    m_szIcon[0].setSize(CX_ICON, SLayoutSize::defUnit);
    m_szIcon[1].setSize(CY_ICON, SLayoutSize::defUnit);

    m_crTxtNormal = GetSysColor(COLOR_MENUTEXT) | 0xff000000;
    m_crTxtSel = GetSysColor(COLOR_HIGHLIGHTTEXT) | 0xff000000;
    m_crTxtGray = GetSysColor(COLOR_GRAYTEXT) | 0xff000000;
}

SMenuAttr::~SMenuAttr()
{
}

int SMenuAttr::GetTextMargin()
{
    return m_nTextMargin.toPixelSize(m_scale);
}

int SMenuAttr::GetIconMargin()
{
    return m_nIconMargin.toPixelSize(m_scale);
}

CSize SMenuAttr::GetIconSize()
{
    return CSize(m_szIcon[0].toPixelSize(m_scale), m_szIcon[1].toPixelSize(m_scale));
}

int SMenuAttr::GetItemHeight()
{
    return m_nItemHei.toPixelSize(m_scale);
}

int SMenuAttr::GetMaxWidth()
{
    if (m_nMaxWidth.isValid())
        return m_nMaxWidth.toPixelSize(m_scale);
    return -1;
}

void SMenuAttr::OnInitFinished(SXmlNode xmlNode)
{
    if (!m_nItemHei.isValid())
    {
        if (m_pItemSkin)
            m_nItemHei.setSize((float)m_pItemSkin->GetSkinSize().cy, dp);
        else
            m_nItemHei.setSize(30, dp);
    }
}

SAutoRefPtr<IFontS> SMenuAttr::GetFontPtr()
{
    SAutoRefPtr<IFontS> font = m_dpiFont.GetFontPtr();
    if (font)
        return font;
    return GETUIDEF->GetFont(FF_DEFAULTFONT, m_scale);
}

void SMenuAttr::SetScale(int scale)
{
    if (m_scale == scale)
        return;
    m_scale = scale;
    m_dpiFont.UpdateFont(m_scale);
    if (m_pIconSkin)
        m_pIconSkin = GETSKIN(m_pIconSkin->GetName(), m_scale);
    if (m_pItemSkin)
        m_pItemSkin = GETSKIN(m_pItemSkin->GetName(), m_scale);
    if (m_pSepSkin)
        m_pSepSkin = GETSKIN(m_pSepSkin->GetName(), m_scale);
    if (m_pCheckSkin)
        m_pCheckSkin = GETSKIN(m_pCheckSkin->GetName(), m_scale);
}

//////////////////////////////////////////////////////////////////////////

SMenuODWnd::SMenuODWnd(HWND hMenuOwner, SMenuAttr *pMenuAttr)
    : m_hMenuOwner(hMenuOwner)
    , m_attr(pMenuAttr)
{
}

SMenuODWnd::~SMenuODWnd()
{
}

void SMenuODWnd::OnInitMenu(HMENU menu)
{
    ::SendMessage(m_hMenuOwner, WM_INITMENU, (WPARAM)menu, 0);
}

void SMenuODWnd::OnInitMenuPopup(HMENU menuPopup, UINT nIndex, BOOL bSysMenu)
{
    ::SendMessage(m_hMenuOwner, WM_INITMENUPOPUP, (WPARAM)menuPopup, MAKELPARAM(nIndex, bSysMenu));
}

void SMenuODWnd::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CRect rcItem = lpDrawItemStruct->rcItem;
    rcItem.MoveToXY(0, 0);
    SMenuItemData *pdmmi = (SMenuItemData *)lpDrawItemStruct->itemData;

    HDC dc(lpDrawItemStruct->hDC);
    SAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, rcItem.Width(), rcItem.Height());
    pRT->BeginDraw();
    if (pdmmi)
    {
        MENUITEMINFO mii = { sizeof(MENUITEMINFO), MIIM_FTYPE, 0 };
        GetMenuItemInfo((HMENU)lpDrawItemStruct->hwndItem, lpDrawItemStruct->itemID, FALSE, &mii);

        BOOL bDisabled = lpDrawItemStruct->itemState & ODS_GRAYED;
        BOOL bSelected = lpDrawItemStruct->itemState & ODS_SELECTED;
        BOOL bChecked = lpDrawItemStruct->itemState & ODS_CHECKED;
        BOOL bRadio = mii.fType & MFT_RADIOCHECK;
        m_attr->m_pItemSkin->DrawByIndex(pRT, rcItem, bSelected ? 1 : 0); // draw background

        // draw icon
        CRect rcIcon;
        int iconOffset = m_attr->GetIconMargin();

        rcIcon.left = rcItem.left + iconOffset;
        CSize szIcon = m_attr->GetIconSize();
        rcIcon.right = rcIcon.left + szIcon.cx;
        rcIcon.top = rcItem.top + (rcItem.Height() - szIcon.cy) / 2;
        rcIcon.bottom = rcIcon.top + szIcon.cy;
        if (bChecked)
        {
            if (m_attr->m_pCheckSkin)
            {
                m_attr->m_pCheckSkin->DrawByIndex(pRT, rcIcon, bRadio ? 1 : 0);
            }
        }
        else if (pdmmi->hIcon)
        {
            pRT->DrawIconEx(rcIcon.left, rcIcon.top, pdmmi->hIcon, rcIcon.Width(), rcIcon.Height(), DI_NORMAL);
        }
        else if (pdmmi->iIcon != -1 && m_attr->m_pIconSkin)
        {
            m_attr->m_pIconSkin->DrawByIndex(pRT, rcIcon, pdmmi->iIcon);
        }
        rcItem.left = rcIcon.right + iconOffset;

        // draw text
        CRect rcTxt = rcItem;
        rcTxt.DeflateRect(m_attr->GetTextMargin(), 0);

        COLORREF crOld = pRT->SetTextColor(bDisabled ? m_attr->m_crTxtGray : (bSelected ? m_attr->m_crTxtSel : m_attr->m_crTxtNormal));

        SAutoRefPtr<IFontS> oldFont;
        pRT->SelectObject(m_attr->GetFontPtr(), (IRenderObj **)&oldFont);
        SStringT strLeft, strRight;
        int pos = pdmmi->strText.ReverseFind('\t');
        if (pos != -1)
        {
            const int kArrowSize = 14; // arrow width.
            rcTxt.right -= kArrowSize * m_attr->GetScale() / 100;
            strLeft = pdmmi->strText.Left(pos);
            strRight = pdmmi->strText.Mid(pos + 1);
            pRT->DrawText(strLeft, strLeft.GetLength(), &rcTxt, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
            pRT->DrawText(strRight, strRight.GetLength(), &rcTxt, DT_SINGLELINE | DT_VCENTER | DT_RIGHT | DT_END_ELLIPSIS);
        }
        else
        {
            pRT->DrawText(pdmmi->strText, pdmmi->strText.GetLength(), &rcTxt, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
        }
        pRT->SelectObject(oldFont, NULL);

        pRT->SetTextColor(crOld);

        if (bSelected && m_attr->m_pItemSkin->GetStates() > 2)
        {
            // draw select mask
            CRect rcItem = lpDrawItemStruct->rcItem;
            rcItem.MoveToXY(0, 0);
            m_attr->m_pItemSkin->DrawByIndex(pRT, rcItem, 2);
        }
    }
    else // if(strcmp("sep",pXmlItem->Value())==0)
    {
        if (m_attr->m_pSepSkin)
            m_attr->m_pSepSkin->DrawByIndex(pRT, rcItem, 0); // draw seperator
    }
    rcItem = lpDrawItemStruct->rcItem;

    pRT->EndDraw();
    HDC hmemdc = pRT->GetDC(0);
    BitBlt(dc, rcItem.left, rcItem.top, rcItem.Width(), rcItem.Height(), hmemdc, 0, 0, SRCCOPY);
    pRT->ReleaseDC(hmemdc, &rcItem);
}

void SMenuODWnd::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
    if (lpMeasureItemStruct->CtlType != ODT_MENU)
        return;

    SMenuItemData *pdmmi = (SMenuItemData *)lpMeasureItemStruct->itemData;
    if (pdmmi)
    {
        // menu item
        lpMeasureItemStruct->itemHeight = m_attr->GetItemHeight();
        lpMeasureItemStruct->itemWidth = m_attr->GetIconSize().cx + m_attr->GetIconMargin() * 2;

        SAutoRefPtr<IRenderTarget> pRT;
        GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
        SAutoRefPtr<IFontS> oldFont;
        pRT->SelectObject(m_attr->GetFontPtr(), (IRenderObj **)&oldFont);
        SIZE szTxt;
        pRT->MeasureText(pdmmi->strText, pdmmi->strText.GetLength(), &szTxt);
        lpMeasureItemStruct->itemWidth += szTxt.cx + m_attr->GetTextMargin() * 2;
        int itemMaxWidth = m_attr->GetMaxWidth();
        if (itemMaxWidth != -1 && (int)lpMeasureItemStruct->itemWidth > itemMaxWidth)
            lpMeasureItemStruct->itemWidth = itemMaxWidth;
        pRT->SelectObject(oldFont, NULL);
    }
    else
    {
        // separator
        lpMeasureItemStruct->itemHeight = m_attr->m_pSepSkin ? m_attr->m_pSepSkin->GetSkinSize().cy : 3;
        lpMeasureItemStruct->itemWidth = 0;
    }
}

void SMenuODWnd::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU menu)
{
    ::SendMessage(m_hMenuOwner, WM_MENUSELECT, MAKEWPARAM(nItemID, nFlags), (LPARAM)menu);
}

LRESULT SMenuODWnd::OnMenuChar(UINT nChar, UINT nFlags, HMENU hMenu)
{
    wchar_t cChar = tolower(nChar);

    int nMenuItems = ::GetMenuItemCount(hMenu);
    for (int i = 0; i < nMenuItems; i++)
    {
        MENUITEMINFO mi = { sizeof(mi), 0 };
        mi.fMask = MIIM_DATA;
        ::GetMenuItemInfo(hMenu, i, TRUE, &mi);
        SMenuItemData *pItemData = (SMenuItemData *)mi.dwItemData;
        if (pItemData && pItemData->vHotKey == cChar)
        {
            return MAKELONG(i, MNC_EXECUTE);
        }
    }
    return MAKELONG(0, MNC_IGNORE);
}

//////////////////////////////////////////////////////////////////////////

SMenu::SMenu(const SMenu &src)
    : m_hMenu(0)
    , m_bAttached(false)
{
    Attach(src.m_hMenu);
}

SMenu::SMenu(HMENU hMenu)
    : m_hMenu(0)
    , m_bAttached(false)
{
    if (hMenu)
    {
        Attach(hMenu);
    }
}

SMenu::~SMenu(void)
{
    if (!m_bAttached)
        DestroyMenu();
}

BOOL SMenu::LoadMenu2(IXmlNode *pXmlNode)
{
    if (!pXmlNode)
        return FALSE;
    SXmlNode xmlMenu(pXmlNode);
    SAutoRefPtr<SMenuAttr> pMenuAttr(new SMenuAttr, TRUE);
    pMenuAttr->InitFromXml(&xmlMenu);
    if (!pMenuAttr->m_pItemSkin)
    {
        SSLOGE() << "menu root doesn't has itemSkin defined";
        return FALSE;
    }

    SASSERT(m_hMenu == 0);
    m_hMenu = CreatePopupMenu();
    if (!m_hMenu)
    {
        return FALSE;
    }
    SetMenuContextHelpId(m_hMenu, xmlMenu.attribute(L"contextHelpId").as_uint(0));
    SetMenuAttr(m_hMenu, pMenuAttr);

    if (m_icons)
    {
        pMenuAttr->m_pIconSkin = m_icons;
    }
    BuildMenu(m_hMenu, xmlMenu);

    return TRUE;
}

BOOL SMenu::LoadMenu(LPCTSTR resId)
{
    SASSERT(!::IsMenu(m_hMenu));

    SXmlDoc xmlDoc;
    if (!LOADXML(xmlDoc, resId))
        return FALSE;

    SXmlNode xmlMenu = xmlDoc.root().child(L"menu");
    if (!xmlMenu)
        return FALSE;

    return LoadMenu2(&xmlMenu);
}

BOOL SMenu::LoadMenuU8(THIS_ LPCSTR resId)
{
    SStringT strResId = S_CA2T(resId, CP_UTF8);
    return LoadMenu(strResId);
}

void SMenu::InitMenuItemData(SMenuItemData *itemInfo, const SStringW &strTextW)
{
    SMenuAttr *pMenuAttr = GetMenuAttr(m_hMenu);
    itemInfo->strText = S_CW2T(TR(strTextW, pMenuAttr->m_strTrCtx));
    //查找快捷键
    int iHotKey = strTextW.Find(L"&");
    if (iHotKey != -1 && iHotKey < strTextW.GetLength() - 1)
    {
        itemInfo->vHotKey = tolower(strTextW[iHotKey + 1]);
    }
    else
    {
        itemInfo->vHotKey = 0;
    }
}

BOOL SMenu::InsertMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem, LPCTSTR strText, int iIcon, HICON hIcon)
{
    nFlags |= MF_OWNERDRAW;
    if (nFlags & MF_SEPARATOR)
    {
        return ::InsertMenu(m_hMenu, nPosition, nFlags, (UINT_PTR)0, (LPCTSTR)NULL);
    }

    SMenuItemData *pMenuData = new SMenuItemData;
    pMenuData->iIcon = iIcon;
    pMenuData->hIcon = hIcon;
    InitMenuItemData(pMenuData, S_CT2W(strText));

    if (!::InsertMenu(m_hMenu, nPosition, nFlags, nIDNewItem, (LPCTSTR)pMenuData))
    {
        delete pMenuData;
        return FALSE;
    }
    if (nFlags & MF_POPUP)
    {
        HMENU hSubMenu = (HMENU)nIDNewItem;
        SMenuAttr *pMenuAttr = GetMenuAttr(hSubMenu);
        if (!pMenuAttr)
        {
            pMenuAttr = GetMenuAttr(m_hMenu);
            SetMenuAttr(hSubMenu, pMenuAttr);
        }
    }
    return TRUE;
}

void SMenu::UpdateScale(int nScale)
{
    SASSERT(IsMenu(m_hMenu));
    SMenuAttr *pMenuAttr = GetMenuAttr(m_hMenu);
    SASSERT(pMenuAttr);
    pMenuAttr->SetScale(nScale);
}

UINT SMenu::TrackPopupMenu(UINT uFlags, int x, int y, HWND hWnd, LPCRECT prcRect, int nScale)
{
    SASSERT(IsMenu(m_hMenu));

    UpdateScale(nScale);

    SMenuODWnd menuOwner(hWnd, GetMenuAttr(m_hMenu));
    menuOwner.CreateNative(NULL, WS_POPUP, WS_EX_NOACTIVATE, 0, 0, 0, 0, NULL, 0, NULL);
    if (IsWindow(hWnd) && IsWindowEnabled(hWnd))
        ::SetForegroundWindow(hWnd);
    UINT uNewFlags = uFlags | TPM_RETURNCMD;
    UINT uRet = ::TrackPopupMenu(m_hMenu, uNewFlags, x, y, 0, menuOwner.m_hWnd, prcRect);
    menuOwner.DestroyWindow();
    if (uRet && !(uFlags & TPM_RETURNCMD))
        ::SendMessage(hWnd, WM_COMMAND, uRet, 0);
    return uRet;
}

void SMenu::BuildMenu(HMENU menuPopup, SXmlNode xmlNode)
{
    SXmlNode xmlItem = xmlNode.first_child();

    while (xmlItem)
    {
        if (wcsicmp(L"item", xmlItem.name()) == 0)
        {
            SMenuItemData *pdmmi = new SMenuItemData;
            pdmmi->iIcon = xmlItem.attribute(L"icon").as_int(-1);
            pdmmi->dwUserData = xmlItem.append_attribute(L"userData").as_uint();
            SStringW strText = xmlItem.attribute(L"text").as_string();
            if (strText.IsEmpty())
            {
                strText = SWindow::GetXmlText(xmlItem);
            }
            strText = TR(GETSTRING(strText), GetMenuAttr(menuPopup)->m_strTrCtx);
            strText = STrText::EscapeString(strText);
            InitMenuItemData(pdmmi, strText);

            int nID = xmlItem.attribute(L"id").as_int(0);
            BOOL bCheck = xmlItem.attribute(L"check").as_bool(false);
            BOOL bRadio = xmlItem.attribute(L"radio").as_bool(false);
            BOOL bDisable = xmlItem.attribute(L"disable").as_bool(false);

            SStringW str;
            xmlItem.ToString(&str);

            SXmlNode xmlChild = xmlItem.first_child();
            while (xmlChild && xmlChild.type() == node_pcdata)
                xmlChild = xmlChild.next_sibling();

            if (!xmlChild && !xmlItem.attribute(L"popup").as_bool(false))
            {
                UINT uFlag = MF_OWNERDRAW;
                if (bCheck)
                    uFlag |= MF_CHECKED;
                if (bDisable)
                    uFlag |= MF_GRAYED;
                if (bRadio)
                    uFlag |= MFT_RADIOCHECK | MF_CHECKED;
                ::AppendMenu(menuPopup, uFlag, (UINT_PTR)nID, (LPCTSTR)pdmmi);
            }
            else
            {
                HMENU hSubMenu = ::CreatePopupMenu();
                SMenuAttr *pMenuAttr = GetMenuAttr(m_hMenu);
                SetMenuAttr(hSubMenu, pMenuAttr);
                SetMenuContextHelpId(hSubMenu, xmlItem.attribute(L"contextHelpId").as_uint(0));

                UINT uFlag = MF_OWNERDRAW | MF_POPUP;
                if (bDisable)
                    uFlag |= MF_GRAYED;
                ::AppendMenu(menuPopup, uFlag, (UINT_PTR)hSubMenu, (LPCTSTR)pdmmi);
                BuildMenu(hSubMenu, xmlItem); // build sub menu
            }
        }
        else if (wcscmp(L"sep", xmlItem.name()) == 0)
        {
            ::AppendMenu(menuPopup, MF_SEPARATOR | MF_OWNERDRAW, (UINT_PTR)0, (LPCTSTR)NULL);
        }
        xmlItem = xmlItem.next_sibling();
    }
}

void SMenu::FreeMenuItemData(HMENU hMemu)
{
    SMenuAttr *pMenuAttr = GetMenuAttr(hMemu);
    pMenuAttr->Release();

    int cItems = ::GetMenuItemCount(hMemu);
    for (int i = 0; i < cItems; i++)
    {
        MENUITEMINFO mii = { sizeof(mii), MIIM_DATA | MIIM_SUBMENU, 0 };
        GetMenuItemInfo(hMemu, i, TRUE, &mii);
        if (mii.dwItemData)
        {
            SMenuItemData *pData = (SMenuItemData *)mii.dwItemData;
            delete pData;
        }
        if (mii.hSubMenu)
        {
            FreeMenuItemData(mii.hSubMenu);
        }
    }
}

void SMenu::DestroyMenu()
{
    SASSERT(!m_bAttached);
    if (::IsMenu(m_hMenu))
    {
        // free item data
        FreeMenuItemData(m_hMenu);
        ::DestroyMenu(m_hMenu);
    }
    m_hMenu = NULL;
}

BOOL SMenu::ModifyMenuString(UINT uPosition, UINT uFlags, LPCTSTR lpItemString)
{
    MENUITEMINFO mi = { sizeof(mi), 0 };
    mi.fMask = MIIM_DATA;
    if (!::GetMenuItemInfo(m_hMenu, uPosition, uFlags & MF_BYPOSITION, &mi))
        return FALSE;
    SMenuItemData *pItemData = (SMenuItemData *)mi.dwItemData;
    pItemData->strText = lpItemString;
    return TRUE;
}

BOOL SMenu::DeleteMenu(UINT uPosition, UINT uFlags)
{
    MENUITEMINFO mi = { sizeof(mi), MIIM_DATA, 0 };
    if (!::GetMenuItemInfo(m_hMenu, uPosition, uFlags & MF_BYPOSITION, &mi))
        return FALSE;
    SMenuItemData *pItemData = (SMenuItemData *)mi.dwItemData;
    if (::DeleteMenu(m_hMenu, uPosition, uFlags))
    {
        delete pItemData;
        return TRUE;
    }
    return FALSE;
}

BOOL SMenu::AppendMenu(UINT uFlags, UINT_PTR uIDNewItem, LPCTSTR lpNewItem, int iIcon, HICON hIcon)
{
    return InsertMenu(-1, uFlags, uIDNewItem, lpNewItem, iIcon, hIcon);
}

BOOL SMenu::CheckMenuItem(UINT uIdCheckItem, UINT uCheck)
{
    return ::CheckMenuItem(m_hMenu, uIdCheckItem, uCheck);
}

BOOL SMenu::CheckMenuRadioItem(THIS_ UINT idFirst, UINT idLast, UINT idCheck, UINT uFlags)
{
    return ::CheckMenuRadioItem(m_hMenu, idFirst, idLast, idCheck, uFlags);
}

SMenuAttr *SMenu::GetMenuAttr(HMENU hMenu) const
{
    SASSERT(::IsMenu(hMenu));
    MENUINFO mi = { sizeof(mi), MIM_MENUDATA, 0 };
    GetMenuInfo(hMenu, &mi);
    return (SMenuAttr *)mi.dwMenuData;
}

void SMenu::SetMenuAttr(HMENU hMenu, SMenuAttr *pMenuAttr) const
{
    MENUINFO mi = { sizeof(mi), MIM_MENUDATA, 0 };
    mi.dwMenuData = (ULONG_PTR)pMenuAttr;
    pMenuAttr->AddRef();
    ::SetMenuInfo(hMenu, &mi);
}

BOOL SMenu::Attach(HMENU hMenu)
{
    SASSERT(m_hMenu == NULL);
    SMenuAttr *pMenuAttr = GetMenuAttr(hMenu);
    if (IsBadReadPtr(pMenuAttr, sizeof(SMenuAttr)))
        return FALSE;
    m_hMenu = hMenu;
    m_bAttached = true;
    return TRUE;
}

HMENU SMenu::Detach()
{
    SASSERT(m_bAttached);
    HMENU hRet = m_hMenu;
    m_hMenu = NULL;
    m_bAttached = false;
    return hRet;
}

BOOL SMenu::SetMenuUserData(UINT uPosition, UINT uFlags, ULONG_PTR ulUserData)
{
    MENUITEMINFO mi = { sizeof(mi), MIIM_DATA, 0 };
    if (!::GetMenuItemInfo(m_hMenu, uPosition, uFlags, &mi))
        return FALSE;
    SMenuItemData *pmid = (SMenuItemData *)mi.dwItemData;
    pmid->dwUserData = ulUserData;
    return TRUE;
}

ULONG_PTR SMenu::GetMenuUserData(UINT uPosition, UINT uFlags)
{
    MENUITEMINFO mi = { sizeof(mi), MIIM_DATA, 0 };
    if (!::GetMenuItemInfo(m_hMenu, uPosition, uFlags, &mi))
        return 0;
    SMenuItemData *pmid = (SMenuItemData *)mi.dwItemData;
    return pmid->dwUserData;
}

void SMenu::SetIconSkin(ISkinObj *icons)
{
    SASSERT(!m_hMenu);
    m_icons = icons;
}

HMENU SMenu::GetHMenu() const
{
    return m_hMenu;
}

DWORD SMenu::GetContextHelpId() const
{
    return ::GetMenuContextHelpId(m_hMenu);
}

void SMenu::SetContextHelpId(THIS_ DWORD dwId)
{
    ::SetMenuContextHelpId(m_hMenu, dwId);
}

HMENU SMenu::GetSubMenu(THIS_ int nPos)
{
    return ::GetSubMenu(m_hMenu, nPos);
}

BOOL SMenu::GetMenuString(THIS_ UINT uPosition, UINT uFlags, IStringT *lpItemString)
{
    MENUITEMINFO mi = { sizeof(mi), 0 };
    mi.fMask = MIIM_DATA;
    if (!::GetMenuItemInfo(m_hMenu, uPosition, uFlags & MF_BYPOSITION, &mi))
        return FALSE;
    SMenuItemData *pItemData = (SMenuItemData *)mi.dwItemData;
    lpItemString->Copy(&pItemData->strText);
    return TRUE;
}

int SMenu::GetMenuItemCount() const
{
    return ::GetMenuItemCount(m_hMenu);
}

SNSEND
