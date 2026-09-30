///////////////////////////////////////////////////////////////////////
/** Class Name: STreeCtrl */
/** Creator: JinHui */
/** Version: 2012.12.16 - 1.1 - Create */
///////////////////////////////////////////////////////////////////////

#include "souistd.h"
#include "control/STreeCtrl.h"

SNSBEGIN

STreeCtrl::STreeCtrl()
    : m_nItemHei(20)
    , m_nIndent(18)
    , m_nItemMargin(4)
    , m_hSelItem(0)
    , m_hSelAnchor(0)
    , m_bMultiSel(FALSE)
    , m_bFullRowSel(TRUE)
    , m_hBandOldSel(0)
    , m_hHoverItem(0)
    , m_hCaptureItem(0)
    , m_pItemSkin(GETBUILTINSKIN(SKIN_SYS_LIST_ITEM))
    , m_pIconSkin(NULL)
    , m_pLineSkin(GETBUILTINSKIN(SKIN_SYS_TREE_LINES))
    , m_pToggleSkin(GETBUILTINSKIN(SKIN_SYS_TREE_TOGGLE))
    , m_pCheckSkin(GETBUILTINSKIN(SKIN_SYS_TREE_CHECKBOX))
    , m_crItemBg(RGBA(255, 255, 255, 255))
    , m_crItemSelBg(RGBA(0, 0, 136, 255))
    , m_nVisibleItems(0)
    , m_nContentWidth(0)
    , m_bCheckBox(FALSE)
    , m_bRightClickSel(FALSE)
    , m_uItemMask(0)
    , m_nItemOffset(0)
    , m_nItemHoverBtn(STVIBtn_None)
    , m_nItemPushDownBtn(STVIBtn_None)
    , m_bHasLines(FALSE)
    , m_pListener(NULL)
{
    m_bClipClient = TRUE;
    m_bFocusable = TRUE;
    m_evtSet.addEvent(EVENTID(EventTCSelChanging));
    m_evtSet.addEvent(EVENTID(EventTCSelChanged));
    m_evtSet.addEvent(EVENTID(EventTreeItemSelChanged));
    m_evtSet.addEvent(EVENTID(EventTCCheckState));
    m_evtSet.addEvent(EVENTID(EventTCExpand));
    m_evtSet.addEvent(EVENTID(EventTCDbClick));
    m_evtSet.addEvent(EVENTID(EventTCRClick));
}

STreeCtrl::~STreeCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////////////////

HSTREEITEM STreeCtrl::InsertItem(LPCTSTR lpszItem, HSTREEITEM hParent, HSTREEITEM hInsertAfter)
{
    return InsertItem(lpszItem, -1, -1, 0, hParent, hInsertAfter);
}

HSTREEITEM STreeCtrl::InsertItem(LPCTSTR lpszItem, int nImage, int nSelectedImage, HSTREEITEM hParent, HSTREEITEM hInsertAfter)
{
    return InsertItem(lpszItem, nImage, nSelectedImage, 0, hParent, hInsertAfter);
}

HSTREEITEM STreeCtrl::InsertItem(LPCTSTR lpszItem, int nImage, int nSelectedImage, LPARAM lParam, HSTREEITEM hParent, HSTREEITEM hInsertAfter)
{
    LPTVITEM pItemObj = new TVITEM();

    pItemObj->strText = lpszItem;
    pItemObj->nImage = nImage;
    pItemObj->nSelectedImage = nSelectedImage;
    pItemObj->lParam = lParam;

    return InsertItem(pItemObj, hParent, hInsertAfter);
}

BOOL STreeCtrl::RemoveItem(HSTREEITEM hItem)
{
    if (!hItem)
        return FALSE;
    HSTREEITEM hParent = GetParentItem(hItem);

    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);

    BOOL bVisible = pItem->bVisible;
    int nItemWidth = CalcMaxItemWidth(hItem);
    int nCheckBoxValue = pItem->nCheckBoxValue;
    if (bVisible)
    {
        if (GetChildItem(hItem) && pItem->bCollapsed == FALSE)
        {
            SetChildrenVisible(hItem, FALSE);
        }
    }

    if (IsAncestor(hItem, m_hHoverItem))
        m_hHoverItem = 0;
    if (IsAncestor(hItem, m_hSelItem))
    {
        m_hSelItem = 0;
        m_hSelAnchor = 0;
    }
    else if (IsAncestor(hItem, m_hSelAnchor))
    {
        // The shift-range anchor can be a fixed base independent of the
        // cursor (e.g. it lives in a sibling branch); clear it too so it
        // never dangles past the deletion. Keeping it in an else branch
        // preserves the invariant: cursor cleared => anchor cleared.
        m_hSelAnchor = 0;
    }
    if (IsAncestor(hItem, m_hCaptureItem))
        m_hCaptureItem = 0;

    // Drop the item (and its descendants) from the multi-selection map -
    // silently, the rows are going away (same convention as STreeView).
    // Filter on ancestry: a pre-order walk starting from the last child
    // would run past the subtree into the following siblings and wrongly
    // deselect them.
    {
        SArray<HSTREEITEM> arrStale;
        for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos;)
        {
            HSTREEITEM hSel;
            BOOL bVal;
            m_mapSelItems.GetNextAssoc(pos, hSel, bVal);
            if (hSel == hItem || IsAncestor(hItem, hSel))
                arrStale.Add(hSel);
        }
        for (int i = 0; i < (int)arrStale.GetCount(); i++)
        {
            m_mapSelItems.RemoveKey(arrStale[i]);
        }
    }

    DeleteItem(hItem);

    // Remove the parent node's expand flag
    if (hParent && !GetChildItem(hParent))
    {
        LPTVITEM pParent = GetItem(hParent);
        pParent->bHasChildren = FALSE;
        pParent->bCollapsed = FALSE;
        CalcItemContentWidth(pParent);
    }

    if (m_bCheckBox && hParent)
    {
        // If the parent node is also unselected or fully selected, no state change is needed; other cases require re-evaluation
        if (nCheckBoxValue != GetItem(hParent)->nCheckBoxValue || nCheckBoxValue == STVICheckBox_PartChecked)
            CheckState(hParent);
    }

    if (bVisible)
    {
        m_nVisibleItems--;

        // Recalculate the maximum x size
        if (nItemWidth == m_nContentWidth)
            UpdateContentWidth();

        UpdateScrollBar();
    }
    return TRUE;
}

void STreeCtrl::RemoveAllItems()
{
    DeleteAllItems();
    m_nVisibleItems = 0;
    m_hSelItem = 0;
    m_hSelAnchor = 0;
    m_hHoverItem = 0;
    m_hCaptureItem = 0;
    m_mapSelItems.RemoveAll();
    m_nContentWidth = 0;
    UpdateScrollBar();
}

HSTREEITEM STreeCtrl::GetRootItem() const
{
    return GetChildItem(STVI_ROOT);
}

HSTREEITEM STreeCtrl::GetNextSiblingItem(HSTREEITEM hItem) const
{
    return CSTree<LPTVITEM>::GetNextSiblingItem(hItem);
}

HSTREEITEM STreeCtrl::GetPrevSiblingItem(HSTREEITEM hItem) const
{
    return CSTree<LPTVITEM>::GetPrevSiblingItem(hItem);
}

HSTREEITEM STreeCtrl::GetChildItem(HSTREEITEM hItem, BOOL bFirst /**< =TRUE */) const
{
    return CSTree<LPTVITEM>::GetChildItem(hItem, bFirst);
}

HSTREEITEM STreeCtrl::GetParentItem(HSTREEITEM hItem) const
{
    return CSTree<LPTVITEM>::GetParentItem(hItem);
}

HSTREEITEM STreeCtrl::GetSelectedItem() const
{
    return m_hSelItem;
}

BOOL STreeCtrl::GetItemText(HSTREEITEM hItem, IStringT *strText) const
{
    if (hItem)
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem)
        {
            strText->Copy(&pItem->strText);
            return TRUE;
        }
    }
    return FALSE;
}

BOOL STreeCtrl::SetItemText(HSTREEITEM hItem, LPCTSTR lpszItem)
{
    if (hItem)
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem)
        {
            pItem->strText = lpszItem;
            CalcItemContentWidth(pItem); // If the new string is longer than the original, not recalculating will cause...
            return TRUE;
        }
    }
    return FALSE;
}

BOOL STreeCtrl::GetItemImage(HSTREEITEM hItem, int *nImage, int *nSelectedImage) const
{
    if (hItem)
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem)
        {
            if (nImage)
                *nImage = pItem->nImage;
            if (nSelectedImage)
                *nSelectedImage = pItem->nSelectedImage;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL STreeCtrl::SetItemImage(HSTREEITEM hItem, int nImage, int nSelectedImage)
{
    if (hItem)
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem)
        {
            pItem->nImage = nImage;
            pItem->nSelectedImage = nSelectedImage;
            return TRUE;
        }
    }
    return FALSE;
}

LPARAM STreeCtrl::GetItemData(HSTREEITEM hItem) const
{
    if (hItem)
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem)
            return pItem->lParam;
    }
    return 0;
}

BOOL STreeCtrl::SetItemData(HSTREEITEM hItem, LPARAM lParam)
{
    if (hItem)
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem)
        {
            pItem->lParam = lParam;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL STreeCtrl::ItemHasChildren(HSTREEITEM hItem) const
{
    if (!hItem)
        return FALSE;

    return GetChildItem(hItem) != 0;
}

int STreeCtrl::GetCheckState(HSTREEITEM hItem) const
{
    if (!m_bCheckBox)
        return 0;

    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
    return pItem->nCheckBoxValue;
}

BOOL STreeCtrl::SetCheckState(HSTREEITEM hItem, BOOL bCheck)
{
    if (!m_bCheckBox)
        return FALSE;

    int nCheck = bCheck ? STVICheckBox_Checked : STVICheckBox_UnChecked;

    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
    if (pItem->nCheckBoxValue != nCheck)
    {
        pItem->nCheckBoxValue = nCheck;

        // Set descendant nodes
        if (CSTree<LPTVITEM>::GetChildItem(hItem))
            SetChildrenState(hItem, nCheck);

        // Check parent node state
        CheckState(GetParentItem(hItem));

        Invalidate();
    }

    return TRUE;
}

BOOL STreeCtrl::Expand(HSTREEITEM hItem, UINT nCode)
{
    BOOL bRet = FALSE;
    if (CSTree<LPTVITEM>::GetChildItem(hItem))
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
        if (nCode == TVE_COLLAPSE && !pItem->bCollapsed)
        {
            pItem->bCollapsed = TRUE;
            SetChildrenVisible(hItem, FALSE);
            bRet = TRUE;
        }
        if (nCode == TVE_EXPAND && pItem->bCollapsed)
        {
            pItem->bCollapsed = FALSE;
            SetChildrenVisible(hItem, TRUE);
            bRet = TRUE;
        }
        if (nCode == TVE_TOGGLE)
        {
            pItem->bCollapsed = !pItem->bCollapsed;
            SetChildrenVisible(hItem, !pItem->bCollapsed);
            bRet = TRUE;
        }
        if (bRet)
        {
            UpdateContentWidth();
            UpdateScrollBar();
        }
    }
    return bRet;
}

BOOL STreeCtrl::EnsureVisible(HSTREEITEM hItem)
{
    if (!VerifyItem(hItem))
        return FALSE;

    LPTVITEM pItem = GetItem(hItem);
    if (!pItem->bVisible)
    {
        HSTREEITEM hParent = GetParentItem(hItem);
        while (hParent)
        {
            LPTVITEM pParent = GetItem(hParent);
            if (pParent->bCollapsed)
                Expand(hParent, TVE_EXPAND);
            hParent = GetParentItem(hParent);
        }
    }
    int iVisible = GetItemShowIndex(hItem);
    int itemHei = m_nItemHei.toPixelSize(GetScale());
    int yOffset = iVisible * itemHei;
    if (yOffset + itemHei > m_siVer.nPos + m_rcClient.Height())
    {
        SetScrollPos(TRUE, yOffset + itemHei - m_rcClient.Height(), TRUE);
    }
    else if (yOffset < m_siVer.nPos)
    {
        SetScrollPos(TRUE, yOffset, TRUE);
    }
    return TRUE;
}

void STreeCtrl::PageUp()
{
    OnScroll(TRUE, SB_PAGEUP, 0);
}

void STreeCtrl::PageDown()
{
    OnScroll(TRUE, SB_PAGEDOWN, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL STreeCtrl::CreateChildren(SXmlNode xmlNode)
{
    if (!xmlNode)
        return FALSE;

    RemoveAllItems();
    ItemLayout();

    SXmlNode xmlItem = xmlNode.child(L"item");

    if (xmlItem)
        LoadBranch(STVI_ROOT, xmlItem);

    return TRUE;
}

void STreeCtrl::LoadBranch(HSTREEITEM hParent, SXmlNode xmlItem)
{
    while (xmlItem)
    {
        HSTREEITEM hItem = InsertItem(xmlItem, hParent);

        SXmlNode xmlChild = xmlItem.child(L"item");
        if (xmlChild)
        {
            LoadBranch(hItem, xmlChild);
            Expand(hItem, xmlItem.attribute(L"expand").as_bool(true) ? TVE_EXPAND : TVE_COLLAPSE);
        }
        xmlItem = xmlItem.next_sibling(L"item");
    }
}

void STreeCtrl::LoadItemAttribute(SXmlNode xmlItem, LPTVITEM pItem)
{
    for (SXmlAttr attr = xmlItem.first_attribute(); attr; attr = attr.next_attribute())
    {
        if (!_wcsicmp(attr.name(), L"text"))
            pItem->strText = S_CW2T(attr.value());
        else if (!_wcsicmp(attr.name(), L"img"))
            pItem->nImage = attr.as_int(0);
        else if (!_wcsicmp(attr.name(), L"selImg"))
            pItem->nSelectedImage = attr.as_int(0);
        else if (!_wcsicmp(attr.name(), L"data"))
            pItem->lParam = attr.as_uint(0);
    }
}

HSTREEITEM STreeCtrl::InsertItem(LPTVITEM pItemObj, HSTREEITEM hParent, HSTREEITEM hInsertAfter)
{
    SASSERT(pItemObj);

    CRect rcClient;
    GetClientRect(rcClient);

    pItemObj->nLevel = GetItemLevel(hParent) + 1;

    BOOL bCheckState = FALSE;

    if (hParent != STVI_ROOT)
    {
        LPTVITEM pParentItem = GetItem(hParent);
        if (pParentItem->bCollapsed || !pParentItem->bVisible)
            pItemObj->bVisible = FALSE;

        if (pParentItem->nCheckBoxValue != pItemObj->nCheckBoxValue)
            bCheckState = TRUE;

        if (!GetChildItem(hParent) && !pParentItem->bHasChildren)
        {
            pParentItem->bHasChildren = TRUE;
            CalcItemContentWidth(pParentItem);
        }
    }

    CalcItemContentWidth(pItemObj);

    HSTREEITEM hRet = CSTree<LPTVITEM>::InsertItem(pItemObj, hParent, hInsertAfter);
    pItemObj->hItem = hRet;
    OnInsertItem(pItemObj);
    if (bCheckState)
        CheckState(hParent);
    if (pItemObj->bVisible)
    {
        m_nVisibleItems++;

        int nViewWidth = CalcItemWidth(pItemObj);
        m_nContentWidth = smax(nViewWidth, m_nContentWidth);
        UpdateScrollBar();
    }

    return hRet;
}

HSTREEITEM STreeCtrl::InsertItem(SXmlNode xmlItem, HSTREEITEM hParent /**< =STVI_ROOT */, HSTREEITEM hInsertAfter /**< =STVI_LAST */)
{
    LPTVITEM pItemObj = new TVITEM();

    LoadItemAttribute(xmlItem, pItemObj);
    return InsertItem(pItemObj, hParent, hInsertAfter);
}

BOOL STreeCtrl::IsAncestor(HSTREEITEM hItem1, HSTREEITEM hItem2)
{
    while (hItem2)
    {
        if (hItem2 == hItem1)
            return TRUE;
        hItem2 = GetParentItem(hItem2);
    }
    return FALSE;
}

void STreeCtrl::SetChildrenVisible(HSTREEITEM hItem, BOOL bVisible)
{
    HSTREEITEM hChild = GetChildItem(hItem);
    while (hChild)
    {
        LPTVITEM pItem = GetItem(hChild);
        pItem->bVisible = bVisible;
        m_nVisibleItems += bVisible ? 1 : -1;
        if (!pItem->bCollapsed)
            SetChildrenVisible(hChild, bVisible);
        hChild = GetNextSiblingItem(hChild);
    }
}

void STreeCtrl::SetChildrenState(HSTREEITEM hItem, int nCheckValue)
{
    HSTREEITEM hChildItem = CSTree<LPTVITEM>::GetChildItem(hItem);
    while (hChildItem)
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hChildItem);
        pItem->nCheckBoxValue = nCheckValue;
        SetChildrenState(hChildItem, nCheckValue);
        hChildItem = CSTree<LPTVITEM>::GetNextSiblingItem(hChildItem);
    }
}

/** Return TRUE if descendant node states are consistent, otherwise return FALSE */
BOOL STreeCtrl::CheckChildrenState(HSTREEITEM hItem, BOOL bCheck)
{
    HSTREEITEM hChildItem = CSTree<LPTVITEM>::GetChildItem(hItem);
    while (hChildItem)
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hChildItem);

        int nCheckValue = bCheck ? STVICheckBox_Checked : STVICheckBox_UnChecked;
        // Current node inconsistent, return immediately
        if (pItem->nCheckBoxValue != nCheckValue)
            return FALSE;
        // Check child node inconsistent, return immediately
        else if (CheckChildrenState(hChildItem, bCheck) == FALSE)
            return FALSE;

        // Check child node sibling nodes
        hChildItem = CSTree<LPTVITEM>::GetNextSiblingItem(hChildItem);
    }
    return TRUE;
}

void STreeCtrl::CheckState(HSTREEITEM hItem)
{
    if (hItem)
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
        int nOldState = pItem->nCheckBoxValue;
        pItem->nCheckBoxValue = STVICheckBox_UnChecked;
        bool bHasUnChecked = false;
        bool bHasChecked = false;
        bool bHasPartChecked = false;

        HSTREEITEM hChild = GetChildItem(hItem);
        while (hChild)
        {
            LPTVITEM pChild = CSTree<LPTVITEM>::GetItem(hChild);
            if (pChild->nCheckBoxValue == STVICheckBox_UnChecked)
                bHasUnChecked = true;
            else if (pChild->nCheckBoxValue == STVICheckBox_Checked)
                bHasChecked = true;
            else if (pChild->nCheckBoxValue == STVICheckBox_PartChecked)
                bHasPartChecked = true;

            if (bHasPartChecked || (bHasUnChecked && bHasChecked))
                break; // Half-selected already determined, end the loop early
            hChild = GetNextSiblingItem(hChild);
        }

        if (bHasPartChecked || (bHasUnChecked && bHasChecked)) // If a child node is half-selected, the parent node must also be half-selected
            pItem->nCheckBoxValue = STVICheckBox_PartChecked;
        else if (bHasChecked && !bHasUnChecked) // If all child nodes are fully selected, the parent node is also fully selected
            pItem->nCheckBoxValue = STVICheckBox_Checked;

        if (pItem->nCheckBoxValue != nOldState)
            CheckState(GetParentItem(hItem));
    }
}

void STreeCtrl::ItemLayout()
{
    int nOffset = 0;
    CSize sizeSkin;

    m_uItemMask = 0;
    m_rcToggle.SetRect(0, 0, 0, 0);
    m_rcCheckBox.SetRect(0, 0, 0, 0);
    m_rcIcon.SetRect(0, 0, 0, 0);

    int nItemHei = m_nItemHei.toPixelSize(GetScale());
    // Compute position
    if (m_pToggleSkin || m_bHasLines)
    {
        m_uItemMask |= STVIMask_Toggle;
        CSize szToggle;
        if (m_bHasLines)
        {
            int nIndent = m_nIndent.toPixelSize(GetScale());
            szToggle = CSize(nIndent, nIndent);
        }
        else
        {
            szToggle = m_pToggleSkin->GetSkinSize();
        }
        m_rcToggle.SetRect(nOffset, (nItemHei - szToggle.cy) / 2, nOffset + szToggle.cx, nItemHei - (nItemHei - szToggle.cy) / 2);
        nOffset += szToggle.cx;
    }

    if (m_bCheckBox && m_pCheckSkin)
    {
        m_uItemMask |= STVIMask_CheckBox;
        sizeSkin = m_pCheckSkin->GetSkinSize();
        m_rcCheckBox.SetRect(nOffset, (nItemHei - sizeSkin.cy) / 2, nOffset + sizeSkin.cx, nItemHei - (nItemHei - sizeSkin.cy) / 2);
        nOffset += sizeSkin.cx;
    }

    if (m_pIconSkin)
    {
        m_uItemMask |= STVIMask_Icon;
        sizeSkin = m_pIconSkin->GetSkinSize();
        m_rcIcon.SetRect(nOffset, (nItemHei - sizeSkin.cy) / 2, nOffset + sizeSkin.cx, nItemHei - (nItemHei - sizeSkin.cy) / 2);
        nOffset += sizeSkin.cx;
    }

    m_nItemOffset = nOffset;
}

void STreeCtrl::CalcItemContentWidth(LPTVITEM pItem)
{
    SAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
    BeforePaintEx(pRT);

    int nTestDrawMode = GetTextAlign() & ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_BOTTOM);

    CRect rcTest;
    DrawText(pRT, pItem->strText, pItem->strText.GetLength(), rcTest, nTestDrawMode | DT_CALCRECT);

    pItem->nContentWidth = rcTest.Width() + m_nItemOffset + 2 * m_nItemMargin.toPixelSize(GetScale());
}

int STreeCtrl::CalcMaxItemWidth(HSTREEITEM hItem)
{
    int nItemWidth = 0, nChildrenWidth = 0;

    if (hItem != STVI_ROOT)
    {
        LPTVITEM pItem = GetItem(hItem);
        if (pItem->bVisible)
            nItemWidth = CalcItemWidth(pItem);
        else
            return 0;
    }
    HSTREEITEM hChild = GetChildItem(hItem);
    while (hChild)
    {
        nChildrenWidth = CalcMaxItemWidth(hChild);
        if (nChildrenWidth > nItemWidth)
            nItemWidth = nChildrenWidth;

        hChild = GetNextSiblingItem(hChild);
    }

    return nItemWidth;
}

void STreeCtrl::UpdateContentWidth()
{
    m_nContentWidth = CalcMaxItemWidth(STVI_ROOT);
}

int STreeCtrl::GetItemShowIndex(HSTREEITEM hItemObj)
{
    int iVisible = -1;
    HSTREEITEM hItem = GetNextItem(STVI_ROOT);
    while (hItem)
    {
        LPTVITEM pItem = GetItem(hItem);
        if (pItem->bVisible)
            iVisible++;
        if (hItem == hItemObj)
        {
            return iVisible;
        }
        if (pItem->bCollapsed)
        { // Skip collapsed items
            HSTREEITEM hChild = GetChildItem(hItem, FALSE);
            while (hChild)
            {
                hItem = hChild;
                hChild = GetChildItem(hItem, FALSE);
            }
        }
        hItem = GetNextItem(hItem);
    }
    return -1;
}

BOOL STreeCtrl::GetItemRect(LPTVITEM pItemObj, CRect &rcItem)
{
    if (pItemObj->bVisible == FALSE)
        return FALSE;

    CRect rcClient;
    GetClientRect(rcClient);
    int nItemHei = m_nItemHei.toPixelSize(GetScale());
    int iFirstVisible = m_siVer.nPos / nItemHei;
    int nPageItems = (rcClient.Height() + nItemHei - 1) / nItemHei + 1;

    int iVisible = -1;
    HSTREEITEM hItem = CSTree<LPTVITEM>::GetNextItem(STVI_ROOT);
    while (hItem)
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem->bVisible)
            iVisible++;
        if (iVisible > iFirstVisible + nPageItems)
            break;
        if (iVisible >= iFirstVisible && pItem == pItemObj)
        {
            CRect rcRet(m_nIndent.toPixelSize(GetScale()) * pItemObj->nLevel, 0, rcClient.Width(), nItemHei);
            rcRet.OffsetRect(rcClient.left - m_siHoz.nPos, rcClient.top - m_siVer.nPos + iVisible * nItemHei);
            rcItem = rcRet;
            return TRUE;
        }
        if (pItem->bCollapsed)
        { // Skip collapsed items
            HSTREEITEM hChild = GetChildItem(hItem, FALSE);
            while (hChild)
            {
                hItem = hChild;
                hChild = GetChildItem(hItem, FALSE);
            }
        }
        hItem = CSTree<LPTVITEM>::GetNextItem(hItem);
    }
    return FALSE;
}

/** Automatically modify pt's position to an offset relative to the current item */
HSTREEITEM STreeCtrl::HitTest(CPoint &pt)
{
    CRect rcClient;
    GetClientRect(&rcClient);
    CPoint pt2 = pt;
    pt2.y -= rcClient.top - m_siVer.nPos;
    int nItemHei = m_nItemHei.toPixelSize(GetScale());
    int nIndent = m_nIndent.toPixelSize(GetScale());
    int iItem = pt2.y / nItemHei;
    if (iItem >= m_nVisibleItems)
        return 0;

    HSTREEITEM hRet = 0;

    int iVisible = -1;
    HSTREEITEM hItem = CSTree<LPTVITEM>::GetNextItem(STVI_ROOT);
    while (hItem)
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem->bVisible)
            iVisible++;
        if (iVisible == iItem)
        {
            CRect rcItem(nIndent * pItem->nLevel, 0, rcClient.Width(), nItemHei);
            rcItem.OffsetRect(rcClient.left - m_siHoz.nPos, rcClient.top - m_siVer.nPos + iVisible * nItemHei);
            if (!m_bFullRowSel && pt.x - rcItem.left > pItem->nContentWidth)
            {
                // Text-highlight mode: the area beyond the item text does not
                // belong to the item, so clicking there must not select it.
                hRet = 0;
            }
            else
            {
                pt -= rcItem.TopLeft();
                hRet = hItem;
            }
            break;
        }
        if (pItem->bCollapsed)
        { // Skip collapsed items
            HSTREEITEM hChild = GetChildItem(hItem, FALSE);
            while (hChild)
            {
                hItem = hChild;
                hChild = GetChildItem(hItem, FALSE);
            }
        }
        hItem = CSTree<LPTVITEM>::GetNextItem(hItem);
    }
    return hRet;
}

void STreeCtrl::RedrawItem(HSTREEITEM hItem)
{
    if (!IsVisible(TRUE))
        return;
    CRect rcClient;
    GetClientRect(rcClient);
    int nItemHei = m_nItemHei.toPixelSize(GetScale());

    int iFirstVisible = m_siVer.nPos / nItemHei;
    int nPageItems = (rcClient.Height() + nItemHei - 1) / nItemHei + 1;
    int iItem = GetItemShowIndex(hItem);
    if (iItem != -1 && iItem >= iFirstVisible && iItem < iFirstVisible + nPageItems)
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);

        CRect rcItem(0, 0, CalcItemWidth(pItem), nItemHei);
        rcItem.OffsetRect(rcClient.left - m_siHoz.nPos, rcClient.top + nItemHei * iItem - m_siVer.nPos);
        InvalidateRect(&rcItem);
    }
}

void STreeCtrl::DrawItem(IRenderTarget *pRT, const CRect &rc, HSTREEITEM hItem)
{
    CRect rcItemBg;
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);

    int nIndent = m_nIndent.toPixelSize(GetScale());
    int nItemHei = m_nItemHei.toPixelSize(GetScale());
    pRT->OffsetViewportOrg(rc.left + pItem->nLevel * nIndent, rc.top, NULL);

    rcItemBg.SetRect(m_nItemOffset + m_nItemMargin.toPixelSize(GetScale()), 0, pItem->nContentWidth, nItemHei);
    if (rcItemBg.right > rc.Width() - pItem->nLevel * nIndent)
        rcItemBg.right = rc.Width() - pItem->nLevel * nIndent;
    if (m_bFullRowSel)
    {
        // Full-row selection: the highlight covers the entire visible item row,
        // including the indent (tree lines) space before the item content.
        CRect rcClient;
        GetClientRect(&rcClient);
        rcItemBg.left = -pItem->nLevel * nIndent;
        rcItemBg.right = rcClient.Width() + m_siHoz.nPos - pItem->nLevel * nIndent;
    }
    // Draw background
    if (IsItemSelected(hItem))
    {
        if (m_pItemSkin != NULL)
        {
            // Draw with the selected state index (falls back to index 0 for
            // single-state skins dedicated to the selected look).
            int idx = SState2Index::GetDefIndex(WndState_Check, true);
            if (idx >= m_pItemSkin->GetStates())
                idx = 0;
            m_pItemSkin->DrawByIndex(pRT, rcItemBg, idx);
        }
        else if (CR_INVALID != m_crItemSelBg)
            pRT->FillSolidRect(rcItemBg, m_crItemSelBg);
    }

    if (pItem->bHasChildren && STVIMask_Toggle == (m_uItemMask & STVIMask_Toggle) && !m_bHasLines)
    {
        int nImage = SState2Index::GetDefIndex(pItem->dwToggleState, false);
        if (!pItem->bCollapsed)
            nImage += 3;
        m_pToggleSkin->DrawByIndex(pRT, m_rcToggle, nImage);
    }

    if (STVIMask_CheckBox == (m_uItemMask & STVIMask_CheckBox))
    {
        int nImage = SState2Index::GetDefIndex(pItem->dwCheckBoxState, false);
        if (pItem->nCheckBoxValue == STVICheckBox_Checked)
            nImage += 3;
        else if (pItem->nCheckBoxValue == STVICheckBox_PartChecked)
            nImage += 6;
        m_pCheckSkin->DrawByIndex(pRT, m_rcCheckBox, nImage);
    }

    if (STVIMask_Icon == (m_uItemMask & STVIMask_Icon) && (pItem->nSelectedImage != -1 || pItem->nImage != -1))
    {
        if (pItem->nSelectedImage != -1 && IsItemSelected(hItem))
            m_pIconSkin->DrawByIndex(pRT, m_rcIcon, pItem->nSelectedImage);
        else
            m_pIconSkin->DrawByIndex(pRT, m_rcIcon, pItem->nImage);
    }

    UINT align = DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;

    CRect rcText(m_nItemOffset + m_nItemMargin.toPixelSize(GetScale()), 0, pItem->nContentWidth, nItemHei);
    rcText.OffsetRect(m_nItemMargin.toPixelSize(GetScale()), 0);
    COLORREF crText = GetStyle().GetTextColor(IsItemSelected(hItem) ? 2 : 0);
    COLORREF crOldText = pRT->SetTextColor(crText);
    pRT->DrawText(pItem->strText, -1, rcText, align);
    pRT->SetTextColor(crOldText);

    pRT->OffsetViewportOrg(-rc.left - pItem->nLevel * m_nIndent.toPixelSize(GetScale()), -rc.top, NULL);
}

void STreeCtrl::DrawLines(IRenderTarget *pRT, const CRect &rc, HSTREEITEM hItem)
{
    if (m_nIndent.toPixelSize(GetScale()) == 0 || !m_pLineSkin || !m_bHasLines)
        return;
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
    SList<HSTREEITEM> lstParent;
    HSTREEITEM hParent = GetParentItem(hItem);
    while (hParent)
    {
        lstParent.AddHead(hParent);
        hParent = GetParentItem(hParent);
    }
    // draw parent flags.
    enum
    {
        plus,
        plus_join,
        plus_bottom,
        minus,
        minus_join,
        minus_bottom,
        line,
        line_join,
        line_bottom,
        line_root,
    }; // 10 line states

    int nIndent = m_nIndent.toPixelSize(GetScale());
    CRect rcLine = rc;
    rcLine.right = rcLine.left + nIndent;
    SPOSITION pos = lstParent.GetHeadPosition();
    while (pos)
    {
        HSTREEITEM hParent = lstParent.GetNext(pos);
        HSTREEITEM hNextSibling = GetNextSiblingItem(hParent);
        if (hNextSibling)
        {
            m_pLineSkin->DrawByIndex(pRT, rcLine, line);
        }
        rcLine.OffsetRect(nIndent, 0);
    }
    bool hasNextSibling = GetNextSiblingItem(hItem) != 0;
    bool hasPervSibling = GetPrevSiblingItem(hItem) != 0;
    bool hasChild = GetChildItem(hItem) != 0;
    bool hasParent = GetParentItem(hItem) != 0;
    int iLine = -1;
    if (hasChild)
    { // test if is collapsed
        if (pItem->bCollapsed)
        {
            if (!hasParent && !hasPervSibling) // no parent
                iLine = plus;
            else if (hasNextSibling)
                iLine = plus_join;
            else
                iLine = plus_bottom;
        }
        else
        {
            if (!hasParent && !hasPervSibling) // no parent
                iLine = minus;
            else if (hasNextSibling)
                iLine = minus_join;
            else
                iLine = minus_bottom;
        }
    }
    else
    {
        if (hasNextSibling)
        {
            if (!hasParent && !hasPervSibling)
                iLine = line_root;
            else
                iLine = line_join;
        }
        else
            iLine = line_bottom;
    }
    m_pLineSkin->DrawByIndex(pRT, rcLine, iLine);
}

int STreeCtrl::ItemHitTest(HSTREEITEM hItem, CPoint &pt) const
{
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
    int nHitTestBtn = STVIBtn_None;

    if (STVIMask_Toggle == (m_uItemMask & STVIMask_Toggle) && pItem->bHasChildren && m_rcToggle.PtInRect(pt))
        nHitTestBtn = STVIBtn_Toggle;
    else if (STVIMask_CheckBox == (m_uItemMask & STVIMask_CheckBox) && m_rcCheckBox.PtInRect(pt))
        nHitTestBtn = STVIBtn_CheckBox;

    return nHitTestBtn;
}

void STreeCtrl::ModifyToggleState(HSTREEITEM hItem, DWORD dwStateAdd, DWORD dwStateRemove)
{
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);

    pItem->dwToggleState |= dwStateAdd;
    pItem->dwToggleState &= ~dwStateRemove;

    CRect rcItem, rcUpdate = m_rcToggle;
    if (GetItemRect(pItem, rcItem))
    {
        rcUpdate.OffsetRect(rcItem.left, rcItem.top);
        InvalidateRect(rcUpdate);
    }
}

void STreeCtrl::ModifyChekcBoxState(HSTREEITEM hItem, DWORD dwStateAdd, DWORD dwStateRemove)
{
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);

    pItem->dwCheckBoxState |= dwStateAdd;
    pItem->dwCheckBoxState &= ~dwStateRemove;

    CRect rcItem, rcUpdate = m_rcCheckBox;
    if (GetItemRect(pItem, rcItem))
    {
        rcUpdate.OffsetRect(rcItem.left, rcItem.top);
        InvalidateRect(rcUpdate);
    }
}

void STreeCtrl::ItemLButtonDown(HSTREEITEM hItem, UINT nFlags, CPoint pt)
{
    int nHitTestBtn = ItemHitTest(hItem, pt);
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);

    // Clear the original pushdown button
    if (m_nItemPushDownBtn != nHitTestBtn)
    {
        if (m_nItemPushDownBtn == STVIBtn_Toggle && WndState_PushDown == (pItem->dwToggleState & WndState_PushDown))
        {
            ModifyToggleState(hItem, 0, WndState_PushDown);
        }

        if (m_nItemPushDownBtn == STVIBtn_CheckBox && WndState_PushDown == (pItem->dwCheckBoxState & WndState_PushDown))
        {
            ModifyChekcBoxState(hItem, 0, WndState_PushDown);
        }

        m_nItemPushDownBtn = nHitTestBtn;
    }

    // Set new pushdown button
    if (m_nItemPushDownBtn != STVIBtn_None)
    {
        if (m_nItemPushDownBtn == STVIBtn_Toggle && WndState_PushDown != (pItem->dwToggleState & WndState_PushDown))
        {
            ModifyToggleState(hItem, WndState_PushDown, 0);
            Expand(pItem->hItem, TVE_TOGGLE);
        }

        if (m_nItemPushDownBtn == STVIBtn_CheckBox && WndState_PushDown != (pItem->dwCheckBoxState & WndState_PushDown))
        {
            BOOL bCheck = pItem->nCheckBoxValue == STVICheckBox_Checked ? FALSE : TRUE;
            ModifyChekcBoxState(hItem, WndState_PushDown, 0);
            SetCheckState(pItem->hItem, bCheck);
        }
    }
}

void STreeCtrl::ItemLButtonUp(HSTREEITEM hItem, UINT nFlags, CPoint pt)
{
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);

    if (m_nItemPushDownBtn != STVIBtn_None)
    {
        if (m_nItemPushDownBtn == STVIBtn_Toggle && WndState_PushDown == (pItem->dwToggleState & WndState_PushDown))
        {
            ModifyToggleState(hItem, 0, WndState_PushDown);
        }

        if (m_nItemPushDownBtn == STVIBtn_CheckBox && WndState_PushDown == (pItem->dwCheckBoxState & WndState_PushDown))
        {
            ModifyChekcBoxState(hItem, 0, WndState_PushDown);
            EventTCCheckState evt(this);
            evt.hItem = hItem;
            evt.uCheckState = pItem->dwCheckBoxState;
            FireEvent(&evt);
        }

        m_nItemPushDownBtn = STVIBtn_None;
    }
}

void STreeCtrl::ItemLButtonDbClick(HSTREEITEM hItem, UINT nFlags, CPoint pt)
{
    if (!hItem)
    {
        return;
    }
    int nHitTestBtn = ItemHitTest(hItem, pt);
    if (nHitTestBtn == STVIBtn_CheckBox)
        ItemLButtonDown(hItem, nFlags, pt);
    // Generate double-click event add by zhaosheng
    EventTCDbClick dbClick(this);
    dbClick.bCancel = FALSE;
    dbClick.hItem = hItem;
    FireEvent(&dbClick);
    if (!dbClick.bCancel)
    {
        Expand(hItem, TVE_TOGGLE);
    }
}

void STreeCtrl::ItemMouseMove(HSTREEITEM hItem, UINT nFlags, CPoint pt)
{
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);

    int nHitTestBtn = ItemHitTest(hItem, pt);

    if (nHitTestBtn != m_nItemHoverBtn)
    {
        if (m_nItemHoverBtn == STVIBtn_Toggle && WndState_Hover == (pItem->dwToggleState & WndState_Hover))
        {
            ModifyToggleState(hItem, 0, WndState_Hover);
        }

        if (m_nItemHoverBtn == STVIBtn_CheckBox && WndState_Hover == (pItem->dwCheckBoxState & WndState_Hover))
        {
            ModifyChekcBoxState(hItem, 0, WndState_Hover);
        }

        m_nItemHoverBtn = nHitTestBtn;
    }

    if (m_nItemHoverBtn != STVIBtn_None)
    {
        if (m_nItemHoverBtn == STVIBtn_Toggle && WndState_Hover != (pItem->dwToggleState & WndState_Hover))
        {
            ModifyToggleState(hItem, WndState_Hover, 0);
        }

        if (m_nItemHoverBtn == STVIBtn_CheckBox && WndState_Hover != (pItem->dwCheckBoxState & WndState_Hover))
        {
            ModifyChekcBoxState(hItem, WndState_Hover, 0);
        }
    }
}

void STreeCtrl::ItemMouseLeave(HSTREEITEM hItem)
{
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);

    if (m_nItemHoverBtn != STVIBtn_None)
    {
        if (m_nItemHoverBtn == STVIBtn_Toggle && WndState_Hover == (pItem->dwToggleState & WndState_Hover))
        {
            ModifyToggleState(hItem, 0, WndState_Hover);
        }

        if (m_nItemHoverBtn == STVIBtn_CheckBox && WndState_Hover == (pItem->dwCheckBoxState & WndState_Hover))
        {
            ModifyChekcBoxState(hItem, 0, WndState_Hover);
        }

        m_nItemHoverBtn = STVIBtn_None;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

void STreeCtrl::OnDestroy()
{
    DeleteAllItems();
    __baseCls::OnDestroy();
}

void STreeCtrl::OnPaint(IRenderTarget *pRT)
{
    if (IsUpdateLocked())
        return;

    CRect rcClient;
    SPainter painter;
    BeforePaint(pRT, painter);

    GetClientRect(rcClient);
    int nItemHei = m_nItemHei.toPixelSize(GetScale());
    int iFirstVisible = m_siVer.nPos / nItemHei;
    int nPageItems = (m_rcClient.Height() + nItemHei - 1) / nItemHei + 1;

    int iVisible = -1;
    HSTREEITEM hItem = CSTree<LPTVITEM>::GetNextItem(STVI_ROOT);
    while (hItem)
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem->bVisible)
            iVisible++;
        if (iVisible > iFirstVisible + nPageItems)
            break;
        if (iVisible >= iFirstVisible)
        {
            CRect rcItem(0, 0, CalcItemWidth(pItem), nItemHei);
            rcItem.OffsetRect(rcClient.left - m_siHoz.nPos, rcClient.top - m_siVer.nPos + iVisible * nItemHei);
            DrawItem(pRT, rcItem, hItem);
            DrawLines(pRT, rcItem, hItem);
        }
        if (pItem->bCollapsed)
        { // Skip collapsed items
            HSTREEITEM hChild = GetChildItem(hItem, FALSE);
            while (hChild)
            {
                hItem = hChild;
                hChild = GetChildItem(hItem, FALSE);
            }
        }
        hItem = CSTree<LPTVITEM>::GetNextItem(hItem);
    }
    DrawRubberBandSel(pRT);
    AfterPaint(pRT, painter);
}

void STreeCtrl::OnLButtonDownEx(UINT nFlags, CPoint pt)
{
    __baseCls::OnLButtonDownEx(nFlags, pt);
    m_hHoverItem = HitTest(pt);

    if (m_bMultiSel && m_hHoverItem && (nFlags & MK_CONTROL))
    {
        // Ctrl + click toggles the clicked item without touching the rest of
        // the selection set. In multi-selection mode the set lives entirely
        // in the map; the anchor follows the clicked item as the keyboard
        // cursor (it is not a selection record).
        if (IsItemSelected(m_hHoverItem))
            RemoveSelItem(m_hHoverItem);
        else
            AddSelItem(m_hHoverItem);
        m_hSelItem = m_hHoverItem;
        m_hSelAnchor = m_hHoverItem;
    }
    else if (m_bMultiSel && (nFlags & (MK_CONTROL | MK_SHIFT)))
    {
        // Ctrl/Shift in multi-selection mode never replaces the selection
        // here; a following rubber band with bAdd decides how the set grows.
    }
    else if (m_bMultiSel && m_hHoverItem)
    {
        // Plain click in multi-selection mode replaces the whole set; the
        // anchor only follows as the cursor. Clicking the sole selected
        // item keeps the set (the record does not change) - it just moves
        // the cursor.
        if (GetSelItemCount() != 1 || !IsItemSelected(m_hHoverItem))
            SelectItem(m_hHoverItem);
        else
        {
            m_hSelItem = m_hHoverItem;
            m_hSelAnchor = m_hHoverItem;
        }
    }
    else if (!m_bMultiSel && m_hHoverItem && !IsItemSelected(m_hHoverItem))
        SelectItem(m_hHoverItem);
    else if (!m_hHoverItem && (nFlags & (MK_CONTROL | MK_SHIFT)) == 0 && (m_hSelItem || GetSelItemCount() > 0))
    {
        // Plain click on blank area clears the selection. The anchor-style
        // TCSelChanging/TCSelChanged pair is a single-selection protocol;
        // multi-selection feedback is per-item only (via ClearSelItems).
        if (m_bMultiSel)
        {
            ClearSelItems();
        }
        else
        {
            EventTCSelChanging evt1(this);
            evt1.bCancel = FALSE;
            evt1.hOldSel = m_hSelItem;
            evt1.hNewSel = 0;
            FireEvent(&evt1);
            if (!evt1.bCancel)
            {
                EventTCSelChanged evt(this);
                evt.hOldSel = m_hSelItem;
                evt.hNewSel = 0;
                m_hSelItem = 0;
                ClearSelItems();
                FireEvent(&evt);
                if (evt.hOldSel)
                    RedrawItem(evt.hOldSel);
            }
        }
    }

    if (m_hHoverItem)
    {
        m_hCaptureItem = m_hHoverItem;
        ItemLButtonDown(m_hHoverItem, nFlags, pt);
    }
}

void STreeCtrl::OnRButtonDown(UINT nFlags, CPoint pt)
{
    CPoint pt2 = pt;
    if (!m_bRightClickSel)
    {
        return;
    }

    m_hHoverItem = HitTest(pt);

    if (!m_hHoverItem)
        return;
    if (m_bMultiSel)
    {
        if (GetSelItemCount() != 1 || !IsItemSelected(m_hHoverItem))
            SelectItem(m_hHoverItem);
        else
        {
            m_hSelItem = m_hHoverItem;
            m_hSelAnchor = m_hHoverItem;
        }
    }
    else if (!IsItemSelected(m_hHoverItem))
    {
        SelectItem(m_hHoverItem);
    }
}

void STreeCtrl::OnRButtonUp(UINT nFlags, CPoint pt)
{
    EventTCRClick evt(this);
    evt.pt = pt;
    evt.hItem = HitTest(pt);
    FireEvent(&evt);
    __baseCls::OnRButtonUp(nFlags, pt);
}

void STreeCtrl::OnLButtonUpEx(UINT nFlags, CPoint pt)
{
    __baseCls::OnLButtonUpEx(nFlags, pt);
    m_hHoverItem = HitTest(pt);

    if (m_hCaptureItem)
    {
        ItemLButtonUp(m_hCaptureItem, nFlags, pt);
        m_hCaptureItem = 0;
    }
    else if (m_hHoverItem)
        ItemLButtonUp(m_hHoverItem, nFlags, pt);
}

void STreeCtrl::OnLButtonDbClick(UINT nFlags, CPoint pt)
{
    __baseCls::OnLButtonDbClick(nFlags, pt);
    m_hHoverItem = HitTest(pt);
    ItemLButtonDbClick(m_hHoverItem, nFlags, pt);
}

void STreeCtrl::OnMouseMoveEx(UINT nFlags, CPoint pt)
{
    __baseCls::OnMouseMoveEx(nFlags, pt);
    HSTREEITEM hHitTest = HitTest(pt);

    if (hHitTest != m_hHoverItem)
    {
        if (m_hHoverItem)
            ItemMouseLeave(m_hHoverItem);

        m_hHoverItem = hHitTest;
    }
    if (m_hHoverItem)
        ItemMouseMove(m_hHoverItem, nFlags, pt);
}

BOOL STreeCtrl::OnDragCancelCapture(int reason)
{
    if (m_hCaptureItem)
    {
        m_nItemPushDownBtn = STVIBtn_None;
        RedrawItem(m_hCaptureItem);
        m_hCaptureItem = 0;
    }
    return TRUE;
}

void STreeCtrl::OnDragClearItemCapture()
{
    m_hCaptureItem = 0;
}

void STreeCtrl::OnMouseLeave()
{
    if (m_hHoverItem)
    {
        ItemMouseLeave(m_hHoverItem);
        m_hHoverItem = 0;
    }
}

BOOL STreeCtrl::SelectItem(HSTREEITEM hItem, BOOL bNotify /**< =TRUE */)
{
    if (!VerifyItem(hItem))
        return FALSE;

    if (m_bMultiSel)
    {
        m_hSelItem = hItem;
        m_hSelAnchor = hItem;
        ClearSelItems();
        AddSelItem(hItem);
        return TRUE;
    }

    // Single selection mode: the anchor m_hSelItem is the selection record
    // (IsItemSelected reads it); the map is the multi-selection record only.
    if (IsItemSelected(hItem))
        return TRUE;

    HSTREEITEM hOldSel = m_hSelItem;

    if (bNotify)
    {
        EventTCSelChanging evt1(this);
        evt1.bCancel = FALSE;
        evt1.hOldSel = hOldSel;
        evt1.hNewSel = hItem;

        FireEvent(&evt1);
        if (evt1.bCancel)
            return FALSE;
    }

    m_hSelItem = hItem;
    m_hSelAnchor = hItem;

    if (hOldSel)
        RedrawItem(hOldSel);
    if (m_hSelItem)
        RedrawItem(m_hSelItem);
    if (bNotify)
    {
        EventTCSelChanged evt(this);
        evt.hOldSel = hOldSel;
        evt.hNewSel = hItem;
        FireEvent(&evt);
    }
    return TRUE;
}

int STreeCtrl::CalcItemWidth(const LPTVITEM pItemObj)
{
    return pItemObj->nContentWidth + pItemObj->nLevel * m_nIndent.toPixelSize(GetScale());
}

HSTREEITEM STreeCtrl::GetNextVisibleItem(HSTREEITEM hItem) const
{
    HSTREEITEM hRet = CSTree<LPTVITEM>::GetNextItem(hItem);
    while (hRet && !CSTree<LPTVITEM>::GetItem(hRet)->bVisible)
        hRet = CSTree<LPTVITEM>::GetNextItem(hRet);
    return hRet;
}

HSTREEITEM STreeCtrl::GetPrevVisibleItem(HSTREEITEM hItem) const
{
    HSTREEITEM hPrev = CSTree<LPTVITEM>::GetPrevSiblingItem(hItem);
    if (!hPrev)
    {
        // No previous sibling: the previous visible item is the parent
        // (0 for root-level items, which have no parent).
        return CSTree<LPTVITEM>::GetParentItem(hItem);
    }
    // Descend into the previous sibling's last visible descendant.
    HSTREEITEM hLast = hPrev;
    for (;;)
    {
        if (CSTree<LPTVITEM>::GetItem(hLast)->bCollapsed)
            break;
        HSTREEITEM hChild = CSTree<LPTVITEM>::GetChildItem(hLast, FALSE);
        while (hChild && !CSTree<LPTVITEM>::GetItem(hChild)->bVisible)
            hChild = CSTree<LPTVITEM>::GetPrevSiblingItem(hChild);
        if (!hChild)
            break;
        hLast = hChild;
    }
    return hLast;
}

HSTREEITEM STreeCtrl::GetVisibleItemByRow(int iRow) const
{
    if (iRow < 0)
        return 0;
    int iVisible = -1;
    HSTREEITEM hItem = CSTree<LPTVITEM>::GetNextItem(STVI_ROOT);
    while (hItem)
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem->bVisible)
        {
            iVisible++;
            if (iVisible == iRow)
                return hItem;
        }
        hItem = CSTree<LPTVITEM>::GetNextItem(hItem);
    }
    return 0;
}

HSTREEITEM STreeCtrl::GetLastVisibleItem() const
{
    HSTREEITEM hLast = 0;
    HSTREEITEM hItem = CSTree<LPTVITEM>::GetNextItem(STVI_ROOT);
    while (hItem)
    {
        if (CSTree<LPTVITEM>::GetItem(hItem)->bVisible)
            hLast = hItem;
        hItem = CSTree<LPTVITEM>::GetNextItem(hItem);
    }
    return hLast;
}

void STreeCtrl::SetSelRange(HSTREEITEM hAnchor, HSTREEITEM hCursor)
{
    if (!m_bMultiSel || !hCursor)
        return;
    if (!hAnchor)
        hAnchor = hCursor;

    // Determine the visible-order span [hLo..hHi]: walk forward from the
    // anchor looking for the cursor; if the cursor is not below it, walk
    // forward from the cursor instead (the anchor is below).
    BOOL bAnchorFirst = FALSE;
    HSTREEITEM h = hAnchor;
    while (h)
    {
        if (h == hCursor)
        {
            bAnchorFirst = TRUE;
            break;
        }
        h = GetNextVisibleItem(h);
    }
    HSTREEITEM hLo = bAnchorFirst ? hAnchor : hCursor;
    HSTREEITEM hHi = bAnchorFirst ? hCursor : hAnchor;

    // Collect the span once and mark it, so membership tests below are cheap.
    SMap<HSTREEITEM, BOOL> inSpan;
    SArray<HSTREEITEM> arrSpan;
    h = hLo;
    while (h)
    {
        inSpan[h] = TRUE;
        arrSpan.Add(h);
        if (h == hHi)
            break;
        h = GetNextVisibleItem(h);
    }

    // Deselect everything outside the span (snapshot first: RemoveSelItem
    // mutates the map while firing events).
    SArray<HSTREEITEM> arrStale;
    for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos;)
    {
        HSTREEITEM hSel;
        BOOL bVal;
        m_mapSelItems.GetNextAssoc(pos, hSel, bVal);
        if (!inSpan.Lookup(hSel))
            arrStale.Add(hSel);
    }
    for (int i = 0; i < (int)arrStale.GetCount(); i++)
    {
        RemoveSelItem(arrStale[i]);
    }
    // Select everything inside the span that is not selected yet.
    for (int i = 0; i < (int)arrSpan.GetCount(); i++)
    {
        if (!IsItemSelected(arrSpan[i]))
            AddSelItem(arrSpan[i]);
    }
}

void STreeCtrl::OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_ESCAPE)
    {
        // Let the base class handle rubber band cancellation (SPanel).
        SetMsgHandled(FALSE);
        return;
    }

    SWindow *pOwner = GetOwner();
    if (pOwner && nChar == VK_RETURN)
    {
        pOwner->SSendMessage(WM_KEYDOWN, nChar, MAKELONG(nFlags, nRepCnt));
        return;
    }

    BOOL bCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    BOOL bShiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

    // SPACE toggles the item at the cursor in multi-selection mode.
    if (nChar == VK_SPACE && m_hSelItem)
    {
        if (m_bMultiSel)
        {
            if (IsItemSelected(m_hSelItem))
                RemoveSelItem(m_hSelItem);
            else
                AddSelItem(m_hSelItem);
            return;
        }
        SetMsgHandled(FALSE);
        return;
    }

    // Ctrl+A selects all items in multi-selection mode.
    if (bCtrlPressed && nChar == 'A')
    {
        if (m_bMultiSel)
        {
            ClearSelItems();
            HSTREEITEM hItem = GetVisibleItemByRow(0);
            while (hItem)
            {
                AddSelItem(hItem);
                hItem = GetNextVisibleItem(hItem);
            }
            return;
        }
        SetMsgHandled(FALSE);
        return;
    }

    HSTREEITEM hNewSel = 0;
    switch (nChar)
    {
    case VK_DOWN:
        hNewSel = m_hSelItem ? GetNextVisibleItem(m_hSelItem) : GetVisibleItemByRow(0);
        break;
    case VK_UP:
        if (m_hSelItem)
            hNewSel = GetPrevVisibleItem(m_hSelItem);
        break;
    case VK_PRIOR:
    {
        OnScroll(TRUE, SB_PAGEUP, 0);
        int nItemHei = m_nItemHei.toPixelSize(GetScale());
        hNewSel = GetVisibleItemByRow(m_siVer.nPos / nItemHei);
        break;
    }
    case VK_NEXT:
    {
        OnScroll(TRUE, SB_PAGEDOWN, 0);
        CRect rcClient;
        GetClientRect(rcClient);
        int nItemHei = m_nItemHei.toPixelSize(GetScale());
        int iRow = (m_siVer.nPos + rcClient.Height() - 1) / nItemHei;
        hNewSel = GetVisibleItemByRow(iRow);
        if (!hNewSel)
            hNewSel = GetLastVisibleItem();
        break;
    }
    case VK_HOME:
        OnScroll(TRUE, SB_TOP, 0);
        hNewSel = GetVisibleItemByRow(0);
        break;
    case VK_END:
        OnScroll(TRUE, SB_BOTTOM, 0);
        hNewSel = GetLastVisibleItem();
        break;
    case VK_LEFT:
        if (m_hSelItem)
        {
            if (CSTree<LPTVITEM>::GetChildItem(m_hSelItem) && !CSTree<LPTVITEM>::GetItem(m_hSelItem)->bCollapsed)
                Expand(m_hSelItem, TVE_COLLAPSE);
            else
                hNewSel = GetPrevVisibleItem(m_hSelItem);
        }
        break;
    case VK_RIGHT:
        if (m_hSelItem)
        {
            if (CSTree<LPTVITEM>::GetChildItem(m_hSelItem) && CSTree<LPTVITEM>::GetItem(m_hSelItem)->bCollapsed)
                Expand(m_hSelItem, TVE_EXPAND);
            else
                hNewSel = GetNextVisibleItem(m_hSelItem);
        }
        break;
    }

    if (hNewSel)
    {
        EnsureVisible(hNewSel);

        if (!m_bMultiSel)
        {
            // Single selection mode: move the selection (anchor events fire).
            SelectItem(hNewSel);
        }
        else if (bCtrlPressed)
        {
            // Ctrl + arrow: move the cursor without changing selection.
            m_hSelItem = hNewSel;
            m_hSelAnchor = hNewSel;
        }
        else if (bShiftPressed)
        {
            // Shift + arrow: move the anchored span to the new cursor (the
            // anchor stays fixed, so the span can shrink as well as grow -
            // Explorer semantics).
            SetSelRange(m_hSelAnchor, hNewSel);
            m_hSelItem = hNewSel;
        }
        else
        {
            // Plain arrow: replace the whole set with the new item.
            SelectItem(hNewSel);
        }
    }
    else
    {
        SetMsgHandled(FALSE);
    }
}

UINT STreeCtrl::OnGetDlgCode() const
{
    return SC_WANTARROWS | SC_WANTSYSKEY;
}

void STreeCtrl::SortChildren(HSTREEITEM hItem, FunTreeSortCallback sortFunc, void *pCtx)
{
    m_hHoverItem = 0;
    m_hCaptureItem = 0;
    CSTree<LPTVITEM>::SortChildren(hItem, sortFunc, pCtx);
}

BOOL STreeCtrl::VerifyItem(HSTREEITEM hItem) const
{
    if (!hItem)
        return FALSE;
#ifdef _DEBUG
    HSTREEITEM hRoot = CSTree<LPTVITEM>::GetRootItem(hItem);
    while (CSTree<LPTVITEM>::GetPrevSiblingItem(hRoot))
    {
        hRoot = CSTree<LPTVITEM>::GetPrevSiblingItem(hRoot);
    }
    return hRoot == GetRootItem();
#endif
    return TRUE;
}

void STreeCtrl::OnNodeFree(LPTVITEM &pItemData)
{
    if (m_pListener)
    {
        m_pListener->OnDeleteItem(this, pItemData->hItem, pItemData->lParam);
    }
    delete pItemData;
}

void STreeCtrl::OnInsertItem(LPTVITEM &pItemData)
{
    if (m_pListener)
    {
        m_pListener->OnInsertItem(this, pItemData->hItem);
    }
}

void STreeCtrl::SetListener(IListener *pListener)
{
    m_pListener = pListener;
}

void STreeCtrl::UpdateScrollBar()
{
    CRect rcClient;
    SWindow::GetClientRect(&rcClient);

    CSize size = rcClient.Size();
    CSize szView(m_nContentWidth, m_nVisibleItems * m_nItemHei.toPixelSize(GetScale()));

    m_wBarVisible = SSB_NULL; // Close scrollbar

    if (size.cy < szView.cy || (size.cy < szView.cy + GetSbWidth() && size.cx < szView.cx))
    {
        // Need vertical scrollbar
        m_wBarVisible |= SSB_VERT;
        m_siVer.nMin = 0;
        m_siVer.nMax = szView.cy - 1;
        m_siVer.nPage = size.cy;
        if (m_siVer.nPos + (int)m_siVer.nPage > m_siVer.nMax)
        {
            m_siVer.nPos = m_siVer.nMax - m_siVer.nPage;
        }
        if (size.cx < szView.cx + GetSbWidth())
        {
            // Need horizontal scrollbar
            m_wBarVisible |= SSB_HORZ;
            m_siVer.nPage = size.cy - GetSbWidth() > 0 ? size.cy - GetSbWidth() : 0;

            m_siHoz.nMin = 0;
            m_siHoz.nMax = szView.cx - 1;
            m_siHoz.nPage = size.cx - GetSbWidth() > 0 ? size.cx - GetSbWidth() : 0;
            if (m_siHoz.nPos + (int)m_siHoz.nPage > m_siHoz.nMax)
            {
                m_siHoz.nPos = m_siHoz.nMax - m_siHoz.nPage;
            }
        }
        else
        {
            // No horizontal scrollbar needed
            m_siHoz.nPage = size.cx;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = m_siHoz.nPage - 1;
            m_siHoz.nPos = 0;
        }
    }
    else
    {
        // No vertical scrollbar needed
        m_siVer.nPage = size.cy;
        m_siVer.nMin = 0;
        m_siVer.nMax = size.cy - 1;
        m_siVer.nPos = 0;

        if (size.cx < szView.cx)
        {
            // Need horizontal scrollbar
            m_wBarVisible |= SSB_HORZ;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = szView.cx - 1;
            m_siHoz.nPage = size.cx;
            if (m_siHoz.nPos + (int)m_siHoz.nPage > m_siHoz.nMax)
            {
                m_siHoz.nPos = m_siHoz.nMax - m_siHoz.nPage;
            }
        }
        // No horizontal scrollbar needed
        else
        {
            m_siHoz.nPage = size.cx;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = m_siHoz.nPage - 1;
            m_siHoz.nPos = 0;
        }
    }

    SetScrollPos(TRUE, m_siVer.nPos, TRUE);
    SetScrollPos(FALSE, m_siHoz.nPos, TRUE);

    SSendMessage(WM_NCCALCSIZE);
    Invalidate();
}

void STreeCtrl::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);
    UpdateScrollBar();
}

HSTREEITEM STreeCtrl::GetNextItem(HSTREEITEM hItem) const
{
    return CSTree<LPTVITEM>::GetNextItem(hItem);
}

void STreeCtrl::CalcItemWidth(IRenderTarget *pRT, HSTREEITEM hItem, DWORD dwFlags)
{
    LPTVITEM pItem = GetItem(hItem);
    CRect rcTest;
    DrawText(pRT, pItem->strText, pItem->strText.GetLength(), rcTest, dwFlags);
    pItem->nContentWidth = rcTest.Width() + m_nItemOffset + 2 * m_nItemMargin.toPixelSize(GetScale());
    HSTREEITEM hChild = GetChildItem(hItem);
    while (hChild)
    {
        CalcItemWidth(pRT, hChild, dwFlags);
        hChild = GetNextSiblingItem(hChild);
    }
}

void STreeCtrl::RecalcItemsWidth()
{
    SAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
    BeforePaintEx(pRT);
    int dwFlags = DT_CALCRECT | (GetTextAlign() & ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_BOTTOM));
    for (HSTREEITEM hItem = GetRootItem(); hItem; hItem = GetNextItem(hItem))
    {
        CalcItemWidth(pRT, hItem, dwFlags);
    }
}

void STreeCtrl::OnScaleChanged(int nScale)
{
    __baseCls::OnScaleChanged(nScale);
    GetScaleSkin(m_pLineSkin, nScale);
    GetScaleSkin(m_pItemSkin, nScale);
    GetScaleSkin(m_pToggleSkin, nScale);
    GetScaleSkin(m_pIconSkin, nScale);
    GetScaleSkin(m_pCheckSkin, nScale);
    ItemLayout();
    RecalcItemsWidth();
    UpdateContentWidth();
    UpdateScrollBar();
}

////////////////////////////////////////////////////////////////////////////////
/** Accessibility virtual children: expose the visible (expanded, bVisible) items */
/** in pre-order as ROLE_SYSTEM_OUTLINEITEM simple elements. */
////////////////////////////////////////////////////////////////////////////////

int STreeCtrl::GetAccItemCount()
{
    return m_nVisibleItems;
}

HSTREEITEM STreeCtrl::GetAccVisibleItem(int nIndex)
{
    if (nIndex < 0)
        return 0;
    int iVisible = -1;
    HSTREEITEM hItem = GetNextItem(STVI_ROOT);
    while (hItem)
    {
        LPTVITEM pItem = GetItem(hItem);
        if (pItem->bVisible)
            iVisible++;
        if (iVisible == nIndex)
            return hItem;
        if (pItem->bCollapsed)
        { // Skip collapsed items
            HSTREEITEM hChild = GetChildItem(hItem, FALSE);
            while (hChild)
            {
                hItem = hChild;
                hChild = GetChildItem(hItem, FALSE);
            }
        }
        hItem = GetNextItem(hItem);
    }
    return 0;
}

BOOL STreeCtrl::GetAccItemRect(HSTREEITEM hItem, CRect &rcItem)
{
    if (!hItem)
        return FALSE;
    LPTVITEM pItem = GetItem(hItem);
    if (!pItem)
        return FALSE;
    return GetItemRect(pItem, rcItem);
}

BOOL STreeCtrl::GetAccItemExpanded(HSTREEITEM hItem)
{
    if (!hItem)
        return FALSE;
    LPTVITEM pItem = GetItem(hItem);
    return pItem && pItem->bHasChildren && !pItem->bCollapsed;
}

void STreeCtrl::AddSelItem(HSTREEITEM hItem)
{
    if (!hItem)
        return;

    // Already in the multi-selection map: the state does not change and no
    // per-item event fires.
    const ItemSelectionMap::CPair *pPair = m_mapSelItems.Lookup(hItem);
    if (pPair && pPair->m_value)
        return;

    m_mapSelItems[hItem] = TRUE;
    RedrawItem(hItem);

    EventTreeItemSelChanged evt(this);
    evt.hItem = hItem;
    evt.bSelected = TRUE;
    FireEvent(&evt);
}

void STreeCtrl::RemoveSelItem(HSTREEITEM hItem)
{
    if (!hItem)
        return;

    // Not in the multi-selection map: nothing to remove, no event.
    if (!m_mapSelItems.RemoveKey(hItem))
        return;
    RedrawItem(hItem);

    EventTreeItemSelChanged evt(this);
    evt.hItem = hItem;
    evt.bSelected = FALSE;
    FireEvent(&evt);
}

void STreeCtrl::ClearSelItems()
{
    // Route through RemoveSelItem so every deselected item fires its
    // per-item selection-state event. Keys are collected first because
    // RemoveSelItem mutates the map while firing events.
    SArray<HSTREEITEM> arrSel;
    for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos;)
    {
        HSTREEITEM hItem;
        BOOL bVal;
        m_mapSelItems.GetNextAssoc(pos, hItem, bVal);
        arrSel.Add(hItem);
    }
    for (int i = 0; i < (int)arrSel.GetCount(); i++)
    {
        RemoveSelItem(arrSel[i]);
    }
}

BOOL STreeCtrl::IsRubberBandSelEnabled() const
{
    return m_bMultiSel && IsBandSelEnabled();
}

void STreeCtrl::OnRubberBandStart()
{
    m_hBandOldSel = m_hSelItem;
    m_arrBandSnapshot.RemoveAll();
    for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos;)
    {
        HSTREEITEM hItem;
        BOOL bVal;
        m_mapSelItems.GetNextAssoc(pos, hItem, bVal);
        m_arrBandSnapshot.Add(hItem);
    }
}

void STreeCtrl::OnRubberBandSelect(const CRect &rcBand, BOOL bAdd)
{
    if (m_nVisibleItems == 0)
        return;
    int nItemHei = m_nItemHei.toPixelSize(GetScale());
    if (nItemHei <= 0)
        return;

    if (!bAdd)
        ClearSelItems();

    CRect rcClient;
    GetClientRect(rcClient);

    int iVisible = -1;
    HSTREEITEM hLast = 0;
    HSTREEITEM hItem = CSTree<LPTVITEM>::GetNextItem(STVI_ROOT);
    while (hItem)
    {
        LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem->bVisible)
            iVisible++;
        if (iVisible < 0)
        {
            hItem = CSTree<LPTVITEM>::GetNextItem(hItem);
            continue;
        }
        int nTop = rcClient.top - m_siVer.nPos + iVisible * nItemHei;
        if (nTop > rcBand.bottom)
            break; // rows below the band, positions keep increasing
        CRect rcItem(0, nTop, 0, nTop + nItemHei);
        rcItem.left = rcClient.left - m_siHoz.nPos;
        rcItem.right = rcItem.left + CalcItemWidth(pItem);
        CRect rcInter;
        if (rcInter.IntersectRect(rcItem, rcBand))
        {
            AddSelItem(hItem);
            hLast = hItem;
        }
        else if (!bAdd && IsItemSelected(hItem))
        {
            RemoveSelItem(hItem);
        }
        if (pItem->bCollapsed)
        { // Skip collapsed items
            HSTREEITEM hChild = GetChildItem(hItem, FALSE);
            while (hChild)
            {
                hItem = hChild;
                hChild = GetChildItem(hItem, FALSE);
            }
        }
        hItem = CSTree<LPTVITEM>::GetNextItem(hItem);
    }

    // The cursor follows the last item hit by the band (same convention as
    // the other multi-selection controls), so keyboard navigation continues
    // from the band's end.
    if (hLast)
        m_hSelItem = hLast;
}

void STreeCtrl::OnRubberBandEnd(const CRect &rcBand, BOOL bCancelled)
{
    if (bCancelled)
    {
        // ESC / external cancel: restore the selection taken at band start.
        // Every restored item fires its per-item event
        // (EventTreeItemSelChanged) via AddSelItem / RemoveSelItem; the
        // anchor-style SelChanged is a single-selection event and is not
        // fired for multi-selection bands.
        ClearSelItems();
        for (int i = 0; i < (int)m_arrBandSnapshot.GetCount(); i++)
        {
            AddSelItem(m_arrBandSnapshot[i]);
        }
        m_arrBandSnapshot.RemoveAll();
        m_hSelItem = m_hBandOldSel;
        m_hSelAnchor = m_hSelItem;
        Invalidate();
    }
    else
    {
        // Normal end: the cursor stayed on the last banded item; the range
        // anchor follows it so the next Shift+arrow works from there.
        m_hSelAnchor = m_hSelItem;
    }
}

/**
 * @brief Gets all rubber band selected items.
 * @param items Output array of item handles.
 * @param nMaxCount Maximum number of handles to retrieve.
 * @return Number of handles retrieved.
 */
int STreeCtrl::GetSelItems(HSTREEITEM *items, int nMaxCount) const
{
    if (!m_bMultiSel)
    {
        // Single-selection mode: report the anchor.
        if (m_hSelItem && nMaxCount > 0)
        {
            items[0] = m_hSelItem;
            return 1;
        }
        return 0;
    }
    int i = 0;
    for (SPOSITION pos = m_mapSelItems.GetStartPosition(); pos && nMaxCount > 0;)
    {
        HSTREEITEM hItem;
        BOOL bVal;
        m_mapSelItems.GetNextAssoc(pos, hItem, bVal);
        items[i++] = hItem;
        nMaxCount--;
    }
    return i;
}

/**
 * @brief Enables or disables multiple selection (rubber band marquee).
 * @param enable TRUE to enable multiple selection, FALSE otherwise.
 */
void STreeCtrl::EnableMultiSelection(BOOL enable)
{
    if (enable && !m_bMultiSel)
    {
        // The map becomes the only selection record (all queries read it
        // exclusively in multi mode). Make sure it holds the anchor's item -
        // a pure record move, no event, no repaint (the item is already
        // drawn selected). The anchor keeps its cursor role (keyboard
        // navigation / shift-range base) but is never a selection record
        // in multi mode.
        if (m_hSelItem)
            m_mapSelItems[m_hSelItem] = TRUE;
        m_hSelAnchor = m_hSelItem;
    }
    else if (!enable && m_bMultiSel)
    {
        int nCount = m_mapSelItems.GetCount();
        if (nCount == 1)
        {
            // The sole selected item keeps its state: just move the record
            // from the map to the single-selection anchor. No event, no
            // repaint.
            m_hSelItem = m_mapSelItems.GetAt(m_mapSelItems.GetStartPosition())->m_key;
            m_mapSelItems.RemoveAll();
            m_hSelAnchor = m_hSelItem;
        }
        else if (nCount > 1)
        {
            // Several items selected: the whole selection is cleared,
            // including the cursor anchor.
            ClearSelItems();
            m_hSelItem = 0;
            m_hSelAnchor = 0;
        }
        // nCount == 0: the anchor is kept unchanged.
    }
    m_bMultiSel = enable;
}

BOOL STreeCtrl::IsItemSelected(HSTREEITEM hItem) const
{
    if (!m_bMultiSel)
        return hItem != 0 && hItem == m_hSelItem;
    // Multi-selection mode: the map is the only selection record; the
    // anchor (m_hSelItem) is just the keyboard cursor there.
    return m_mapSelItems.Lookup(hItem) != NULL;
}

int STreeCtrl::GetSelItemCount() const
{
    if (m_bMultiSel)
        return (int)m_mapSelItems.GetCount();
    return m_hSelItem ? 1 : 0;
}
SNSEND
