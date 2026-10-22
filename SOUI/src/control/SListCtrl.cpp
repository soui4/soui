#include "souistd.h"

#include "control/SListCtrl.h"

#pragma warning(disable : 4267 4018)

#define ITEM_MARGIN 4
SNSBEGIN
///////////////////////////////////////////////////////////////////////
/** SListCtrl */
SListCtrl::SListCtrl()
    : m_nHeaderHeight(20)
    , m_nItemHeight(20)
    , m_pHeader(NULL)
    , m_nSelectItem(-1)
    , m_nSelAnchor(-1)
    , m_nSelectColumn(-1)
    , m_iBandOldSel(-1)
    , m_crItemBg(RGBA(255, 255, 255, 255))
    , m_crItemBg2(RGBA(226, 226, 226, 255))
    , m_crItemSelBg(RGBA(57, 145, 209, 255))
    , m_crItemHotBg(RGBA(57, 145, 209, 128))
    , m_crText(RGBA(0, 0, 0, 255))
    , m_crSelText(RGBA(255, 255, 0, 255))
    , m_pItemSkin(GETBUILTINSKIN(SKIN_SYS_LIST_ITEM))
    , m_pIconSkin(NULL)
    , m_pCheckSkin(GETBUILTINSKIN(SKIN_SYS_CHECKBOX))
    , m_ptIcon(-1, -1)
    , m_ptText(-1, -1)
    , m_bHotTrack(FALSE)
    , m_bCheckBox(FALSE)
    , m_bMultiSelection(FALSE)
    , m_bEditLabel(FALSE)
    , m_pInplaceEdit(NULL)
    , m_nEditItem(-1)
    , m_nEditSubItem(-1)
{
    m_bClipClient = TRUE;
    m_bFocusable = TRUE;
    m_evtSet.addEvent(EVENTID(EventLCSelChanging));
    m_evtSet.addEvent(EVENTID(EventLCSelChanged));
    m_evtSet.addEvent(EVENTID(EventItemSelChanged));
    m_evtSet.addEvent(EVENTID(EventItemCheckState));
    m_evtSet.addEvent(EVENTID(EventLCDbClick));
    m_evtSet.addEvent(EVENTID(EventLCItemDeleted));
    m_evtSet.addEvent(EVENTID(EventLCRClick));
    m_evtSet.addEvent(EVENTID(EventLCBeginEditLabel));
    m_evtSet.addEvent(EVENTID(EventLCEndEditLabel));
}

SListCtrl::~SListCtrl()
{
}

int SListCtrl::InsertColumn(int nIndex, LPCTSTR pszText, int nWidth, UINT fmt, LPARAM lParam)
{
    SASSERT(m_pHeader);

    int nRet = m_pHeader->InsertItem(nIndex, pszText, nWidth, fmt, lParam);
    for (int i = 0; i < GetItemCount(); i++)
    {
        m_arrItems[i].arSubItems->SetCount(GetColumnCount());
    }
    UpdateScrollBar();
    return nRet;
}

BOOL SListCtrl::CreateChildren(SXmlNode xmlNode)
{
    SXmlNode xmlHeader = xmlNode.child(L"headerStyle");
    m_pHeader = sobj_cast<SHeaderCtrl>(CreateChildByName(xmlHeader.attribute(L"wndclass").as_string(SHeaderCtrl::GetClassName())));
    if (!m_pHeader)
        return FALSE;
    InsertChild(m_pHeader);
    m_pHeader->InitFromXml(&xmlHeader);
    xmlHeader.set_userdata(1);

    if (!__baseCls::CreateChildren(xmlNode))
        return FALSE;

    m_pHeader->GetEventSet()->subscribeEvent(EventHeaderItemChanging::EventID, Subscriber(&SListCtrl::OnHeaderSizeChanging, this));
    m_pHeader->GetEventSet()->subscribeEvent(EventHeaderItemSwap::EventID, Subscriber(&SListCtrl::OnHeaderSwap, this));

    return TRUE;
}

int SListCtrl::InsertItem(int nItem, LPCTSTR pszText, int nImage)
{
    if (GetColumnCount() == 0)
        return -1;
    if (nItem < 0 || nItem > GetItemCount())
        nItem = GetItemCount();

    DXLVITEM lvi;
    lvi.dwData = 0;
    lvi.arSubItems = new ArrSubItem();
    lvi.arSubItems->SetCount(GetColumnCount());

    DXLVSUBITEM &subItem = lvi.arSubItems->GetAt(0);
    subItem.strText = _tcsdup(pszText);
    subItem.cchTextMax = _tcslen(pszText);
    subItem.nImage = nImage;

    m_arrItems.InsertAt(nItem, lvi);
    // The selection record lives at the control level and must stay parallel
    // to m_arrItems; a fresh item starts unselected.
    m_arrSelFlags.InsertAt(nItem, FALSE);

    UpdateScrollBar();

    return nItem;
}

BOOL SListCtrl::SetItemData(int nItem, LPARAM dwData)
{
    if (nItem >= GetItemCount() || nItem < 0)
        return FALSE;

    m_arrItems[nItem].dwData = dwData;

    return TRUE;
}

LPARAM SListCtrl::GetItemData(int nItem)
{
    if (nItem >= GetItemCount() || nItem < 0)
        return 0;

    DXLVITEM &lvi = m_arrItems[nItem];

    return lvi.dwData;
}

BOOL SListCtrl::SetSubItem(int nItem, int nSubItem, const DXLVSUBITEM *plv)
{
    if (nItem >= GetItemCount() || nSubItem >= GetColumnCount() || nItem < 0)
        return FALSE;
    DXLVSUBITEM &lvsi_dst = m_arrItems[nItem].arSubItems->GetAt(nSubItem);
    if (plv->mask & S_LVIF_TEXT)
    {
        if (lvsi_dst.strText)
            free(lvsi_dst.strText);
        lvsi_dst.strText = _tcsdup(plv->strText);
        lvsi_dst.cchTextMax = _tcslen(plv->strText);
    }
    if (plv->mask & S_LVIF_IMAGE)
        lvsi_dst.nImage = plv->nImage;
    if (plv->mask & S_LVIF_INDENT)
        lvsi_dst.nIndent = plv->nIndent;
    RedrawItem(nItem);
    return TRUE;
}

BOOL SListCtrl::GetSubItem(int nItem, int nSubItem, DXLVSUBITEM *plv) const
{
    if (nItem >= GetItemCount() || nSubItem >= GetColumnCount() || nItem < 0)
        return FALSE;

    const DXLVSUBITEM &lvsi_src = m_arrItems[nItem].arSubItems->GetAt(nSubItem);
    if (plv->mask & S_LVIF_TEXT)
    {
        _tcscpy_s(plv->strText, plv->cchTextMax, lvsi_src.strText);
    }
    if (plv->mask & S_LVIF_IMAGE)
        plv->nImage = lvsi_src.nImage;
    if (plv->mask & S_LVIF_INDENT)
        plv->nIndent = lvsi_src.nIndent;
    return TRUE;
}

BOOL SListCtrl::SetSubItemText(int nItem, int nSubItem, LPCTSTR pszText)
{
    if (nItem < 0 || nItem >= GetItemCount())
        return FALSE;

    if (nSubItem < 0 || nSubItem >= GetColumnCount())
        return FALSE;

    DXLVSUBITEM &lvi = m_arrItems[nItem].arSubItems->GetAt(nSubItem);
    if (lvi.strText)
    {
        free(lvi.strText);
    }
    lvi.strText = _tcsdup(pszText);
    lvi.cchTextMax = _tcslen(pszText);

    CRect rcItem = GetItemRect(nItem, nSubItem);
    InvalidateRect(rcItem);
    return TRUE;
}

SStringT SListCtrl::GetSubItemText(int nItem, int nSubItem) const
{
    if (nItem >= GetItemCount() || nSubItem >= GetColumnCount() || nItem < 0)
        return _T("");

    const DXLVSUBITEM &lvsi_src = m_arrItems[nItem].arSubItems->GetAt(nSubItem);
    return lvsi_src.strText;
}

int SListCtrl::GetSelectedItem()
{
    return m_nSelectItem;
}

void SListCtrl::SetSelectedItem(int nItem)
{
    if (nItem != m_nSelectItem)
    {
        NotifySelChange(m_nSelectItem, nItem);
    }
}

int SListCtrl::GetSelectedColumn()
{
    return m_nSelectColumn;
}

void SListCtrl::SetSelectedColumn(int nColumn)
{
    if (nColumn != m_nSelectColumn && (nColumn == -1 || (nColumn >= 0 && nColumn < GetColumnCount())))
    {
        m_nSelectColumn = nColumn;
        Invalidate();
    }
}

int SListCtrl::GetItemCount() const
{
    if (GetColumnCount() <= 0)
        return 0;

    return m_arrItems.GetCount();
}

BOOL SListCtrl::SetItemCount(int nItems, int nGrowBy)
{
    int nOldCount = GetItemCount();
    if (nItems < nOldCount)
        return FALSE;

    BOOL bRet = m_arrItems.SetCount(nItems, nGrowBy);
    if (bRet)
    {
        BOOL bRetFlags = m_arrSelFlags.SetCount(nItems, nGrowBy);
        for (int i = nOldCount; i < nItems; i++)
        {
            DXLVITEM &lvi = m_arrItems[i];
            lvi.arSubItems = new ArrSubItem;
            lvi.arSubItems->SetCount(GetColumnCount());
            if (bRetFlags)
                m_arrSelFlags.SetAt(i, FALSE);
        }
    }
    UpdateScrollBar();

    return bRet;
}

CRect SListCtrl::GetListRect()
{
    CRect rcList;

    GetClientRect(&rcList);
    rcList.top += m_nHeaderHeight.toPixelSize(GetScale());

    return rcList;
}

///////////////////////////////////////////////////////////////////////
/** Update scroll bar */
void SListCtrl::UpdateScrollBar()
{
    CSize szView;
    szView.cx = m_pHeader->GetTotalWidth(false);
    int nMinWid = m_pHeader->GetTotalWidth(true);
    szView.cy = GetItemCount() * m_nItemHeight.toPixelSize(GetScale());

    CRect rcClient;
    SWindow::GetClientRect(&rcClient); // Do not compute scroll bar size
    rcClient.top += m_nHeaderHeight.toPixelSize(GetScale());
    if (rcClient.bottom < rcClient.top)
        rcClient.bottom = rcClient.top;
    CSize size = rcClient.Size();
    // Close scroll bar
    m_wBarVisible = SSB_NULL;

    if (size.cy < szView.cy || (size.cy < szView.cy + GetSbWidth() && size.cx < szView.cx))
    {
        // Need vertical scroll bar
        m_wBarVisible |= SSB_VERT;
        m_siVer.nMin = 0;
        m_siVer.nMax = szView.cy - 1;
        m_siVer.nPage = rcClient.Height();

        int horzSize = size.cx - GetSbWidth();
        if (horzSize < nMinWid)
        {
            // Less than the header's minimum width, horizontal scroll bar needed
            m_wBarVisible |= SSB_HORZ;
            m_siVer.nPage = size.cy - GetSbWidth() > 0 ? size.cy - GetSbWidth() : 0; // Note to also adjust the vertical scroll bar page info

            m_siHoz.nMin = 0;
            m_siHoz.nMax = szView.cx - 1;
            m_siHoz.nPage = (size.cx - GetSbWidth()) > 0 ? (size.cx - GetSbWidth()) : 0;
        }
        else
        {
            if (horzSize < szView.cx || m_pHeader->IsAutoResize())
            { // Greater than the minimum width but less than the current width, then adjust the header width.
                CRect rcHead = m_pHeader->GetWindowRect();
                rcHead.right = rcHead.left + horzSize;
                m_pHeader->Move(rcHead);
                szView.cx = horzSize;
            }
            // No horizontal scroll bar needed
            m_siHoz.nPage = szView.cx;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = m_siHoz.nPage - 1;
            m_siHoz.nPos = 0;
        }
    }
    else
    {
        // No vertical scroll bar needed
        m_siVer.nPage = size.cy;
        m_siVer.nMin = 0;
        m_siVer.nMax = size.cy - 1;
        m_siVer.nPos = 0;

        if (size.cx < nMinWid)
        {
            // Less than the header's minimum width, horizontal scroll bar needed
            m_wBarVisible |= SSB_HORZ;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = szView.cx - 1;
            m_siHoz.nPage = size.cx;
        }
        else
        {
            if (size.cx < szView.cx || m_pHeader->IsAutoResize())
            { // Greater than the minimum width but less than the current width, then adjust the header width.
                CRect rcHead = m_pHeader->GetWindowRect();
                rcHead.right = rcHead.left + size.cx;
                m_pHeader->Move(rcHead);
                szView.cx = size.cx;
            }
            // No horizontal scroll bar needed
            m_siHoz.nPage = szView.cx;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = m_siHoz.nPage - 1;
            m_siHoz.nPos = 0;
        }
    }

    // Adjust origin position as needed
    if (HasScrollBar(FALSE) && m_siHoz.nPos + m_siHoz.nPage > szView.cx)
    {
        m_siHoz.nPos = szView.cx - m_siHoz.nPage;
    }

    if (HasScrollBar(TRUE) && m_siVer.nPos + m_siVer.nPage > szView.cy)
    {
        m_siVer.nPos = szView.cy - m_siVer.nPage;
    }

    SetScrollPos(TRUE, m_siVer.nPos, TRUE);
    SetScrollPos(FALSE, m_siHoz.nPos, TRUE);

    // Recompute client and non-client areas
    SSendMessage(WM_NCCALCSIZE);

    Invalidate();
}

/** Update header position */
void SListCtrl::UpdateHeaderCtrl()
{
    CRect rcClient;
    GetClientRect(&rcClient);
    CRect rcHeader(rcClient);
    rcHeader.bottom = rcHeader.top + m_nHeaderHeight.toPixelSize(GetScale());
    rcHeader.left -= m_ptOrigin.x;
    if (m_pHeader)
        m_pHeader->Move(rcHeader);
}

void SListCtrl::DeleteItem(int nItem)
{
    if (nItem >= 0 && nItem < GetItemCount())
    {
        DXLVITEM &lvi = m_arrItems[nItem];

        EventLCItemDeleted evt2(this);
        evt2.nItem = nItem;
        evt2.dwData = lvi.dwData;
        FireEvent(evt2);

        for (int i = 0; i < GetColumnCount(); i++)
        {
            DXLVSUBITEM &lvsi = lvi.arSubItems->GetAt(i);
            if (lvsi.strText)
                free(lvsi.strText);
        }
        delete lvi.arSubItems;
        m_arrItems.RemoveAt(nItem);
        // Keep the control-level selection record parallel to m_arrItems.
        m_arrSelFlags.RemoveAt(nItem);

        UpdateScrollBar();
    }
}

void SListCtrl::DeleteColumn(int iCol)
{
    if (m_pHeader->DeleteItem(iCol))
    {
        int nColumnCount = m_pHeader->GetItemCount();

        for (int i = 0; i < GetItemCount(); i++)
        {
            DXLVITEM &lvi = m_arrItems[i];

            if (0 == nColumnCount)
            {
                EventLCItemDeleted evt2(this);
                evt2.nItem = i;
                evt2.dwData = lvi.dwData;
                FireEvent(evt2);
            }

            DXLVSUBITEM &lvsi = lvi.arSubItems->GetAt(iCol);
            if (lvsi.strText)
                free(lvsi.strText);
            m_arrItems[i].arSubItems->RemoveAt(iCol);
        }
        UpdateScrollBar();
    }
}

void SListCtrl::DeleteAllItems()
{
    m_nSelectItem = -1;
    m_nSelAnchor = -1;
    m_arrSelFlags.RemoveAll();
    for (int i = 0; i < GetItemCount(); i++)
    {
        DXLVITEM &lvi = m_arrItems[i];

        EventLCItemDeleted evt2(this);
        evt2.nItem = i;
        evt2.dwData = lvi.dwData;
        FireEvent(evt2);

        for (int j = 0; j < GetColumnCount(); j++)
        {
            DXLVSUBITEM &lvsi = lvi.arSubItems->GetAt(j);
            if (lvsi.strText)
                free(lvsi.strText);
        }
        delete lvi.arSubItems;
    }
    m_arrItems.RemoveAll();

    UpdateScrollBar();
}

CRect SListCtrl::GetItemRect(int nItem, int nSubItem)
{
    if (!(nItem >= 0 && nItem < GetItemCount() && nSubItem >= 0 && nSubItem < GetColumnCount()))
        return CRect();

    CRect rcItem;
    int itemHeight = m_nItemHeight.toPixelSize(GetScale());
    rcItem.top = itemHeight * nItem;
    rcItem.bottom = rcItem.top + itemHeight;
    rcItem.left = 0;
    rcItem.right = 0;

    for (int nCol = 0; nCol < GetColumnCount(); nCol++)
    {
        SHDITEM hdi = { SHDI_WIDTH | SHDI_ORDER, 0 };

        m_pHeader->GetItem(nCol, &hdi);
        rcItem.left = rcItem.right;
        rcItem.right = rcItem.left + hdi.cx;
        if (hdi.iOrder == nSubItem)
            break;
    }

    CRect rcList = GetListRect();
    // Transform to window coordinates
    rcItem.OffsetRect(rcList.TopLeft());
    // Correct based on origin coordinates
    rcItem.OffsetRect(-m_ptOrigin);

    return rcItem;
}

///////////////////////////////////////////////////////////////////////
/** Automatically modify pt's position to an offset relative to the current item */
int SListCtrl::HitTest(const CPoint &pt)
{
    CRect rcList = GetListRect();

    CPoint pt2 = pt;
    pt2.y -= rcList.top - m_ptOrigin.y;

    int nRet = pt2.y / m_nItemHeight.toPixelSize(GetScale());
    if (nRet >= GetItemCount())
    {
        nRet = -1;
    }

    return nRet;
}

int SListCtrl::HitTest(const CPoint &pt, int *pnSubItem)
{
    int nItem = HitTest(pt);
    if (pnSubItem)
    {
        *pnSubItem = -1;
        if (nItem != -1 && m_pHeader)
        {
            CRect rcList = GetListRect();
            CPoint pt2 = pt;
            pt2.x -= rcList.left - m_ptOrigin.x;

            int x = 0;
            for (int nCol = 0; nCol < GetColumnCount(); nCol++)
            {
                SHDITEM hdi = { SHDI_WIDTH | SHDI_ORDER, 0 };
                m_pHeader->GetItem(nCol, &hdi);
                int colWidth = hdi.cx;
                if (pt2.x >= x && pt2.x < x + colWidth)
                {
                    *pnSubItem = hdi.iOrder;
                    break;
                }
                x += colWidth;
            }
        }
    }
    return nItem;
}

void SListCtrl::RedrawItem(int nItem)
{
    if (!IsVisible(TRUE))
        return;

    CRect rcList = GetListRect();

    int nTopItem = GetTopIndex();
    int nItemHeight = m_nItemHeight.toPixelSize(GetScale());
    int nPageItems = (rcList.Height() + nItemHeight - 1) / nItemHeight;

    if (nItem >= nTopItem && nItem < GetItemCount() && nItem <= nTopItem + nPageItems)
    {
        CRect rcItem(0, 0, rcList.Width(), nItemHeight);
        rcItem.OffsetRect(0, nItemHeight * nItem - m_ptOrigin.y);
        rcItem.OffsetRect(rcList.TopLeft());
        CRect rcDC;
        rcDC.IntersectRect(rcItem, rcList);
        IRenderTarget *pRT = GetRenderTarget(&rcDC, GRT_PAINTBKGND);
        SSendMessage(WM_ERASEBKGND, (WPARAM)pRT);

        DrawItem(pRT, rcItem, nItem);

        ReleaseRenderTarget(pRT);
    }
}

int SListCtrl::GetCountPerPage(BOOL bPartial)
{
    CRect rcClient = GetListRect();

    // calculate number of items per control height (include partial item)
    div_t divHeight = div(rcClient.Height(), m_nItemHeight.toPixelSize(GetScale()));

    // round up to nearest item count
    return smax((int)(bPartial && divHeight.rem > 0 ? divHeight.quot + 1 : divHeight.quot), 1);
}
BOOL SListCtrl::SortItems(PFNLVCOMPAREEX pfnCompare, void *pContext)
{
    qsort_s(m_arrItems.GetData(), m_arrItems.GetCount(), sizeof(DXLVITEM), pfnCompare, pContext);
    m_nSelectItem = -1;
    m_nSelAnchor = -1;
    m_nHoverItem = -1;
    // The comparison protocol carries no item identity, so the selection
    // flags cannot follow their items through the sort: clear the record.
    for (int i = 0; i < m_arrSelFlags.GetCount(); i++)
        m_arrSelFlags.SetAt(i, FALSE);
    InvalidateRect(GetListRect());
    return TRUE;
}

void SListCtrl::OnPaint(IRenderTarget *pRT)
{
    SPainter painter;
    BeforePaint(pRT, painter);
    CRect rcList = GetListRect();
    int nTopItem = GetTopIndex();
    pRT->PushClipRect(&rcList, RGN_AND);
    CRect rcItem(rcList);

    rcItem.bottom = rcItem.top;
    int nItemHeight = m_nItemHeight.toPixelSize(GetScale());
    rcItem.OffsetRect(0, -(m_ptOrigin.y % nItemHeight));
    for (int nItem = nTopItem; nItem <= (nTopItem + GetCountPerPage(TRUE)) && nItem < GetItemCount(); rcItem.top = rcItem.bottom, nItem++)
    {
        rcItem.bottom = rcItem.top + nItemHeight;

        DrawItem(pRT, rcItem, nItem);
    }
    DrawRubberBandSel(pRT);
    pRT->PopClip();
    AfterPaint(pRT, painter);
}

BOOL SListCtrl::HitCheckBox(const CPoint &pt)
{
    if (!m_bCheckBox || !m_pCheckSkin)
        return FALSE;

    CRect rect = GetListRect();
    rect.left += ITEM_MARGIN;
    rect.OffsetRect(-m_ptOrigin.x, 0);

    CSize sizeSkin = m_pCheckSkin->GetSkinSize();
    int nOffsetX = 3;
    CRect rcCheck;
    rcCheck.SetRect(0, 0, sizeSkin.cx, sizeSkin.cy);
    rcCheck.OffsetRect(rect.left + nOffsetX, 0);

    if (pt.x >= rcCheck.left && pt.x <= rcCheck.right)
        return TRUE;
    return FALSE;
}

void SListCtrl::DrawItem(IRenderTarget *pRT, CRect rcItem, int nItem)
{
    BOOL bTextColorChanged = FALSE;
    int nBgImg = 0;
    COLORREF crOldText = RGBA(0xFF, 0xFF, 0xFF, 0xFF);
    COLORREF crItemBg = m_crItemBg;
    COLORREF crText = m_crText;
    DXLVITEM lvItem = m_arrItems[nItem];
    CRect rcIcon, rcText;

    if (nItem % 2)
    {
        if (CR_INVALID != m_crItemBg2)
            crItemBg = m_crItemBg2;
    }

    if (GetSelected(nItem))
    { // Separated from the condition of the if below, so there is a distinction between sel and hot
        if (m_pItemSkin != NULL)
            nBgImg = 2;
        else if (CR_INVALID != m_crItemSelBg)
            crItemBg = m_crItemSelBg;

        if (CR_INVALID != m_crSelText)
            crText = m_crSelText;
    }
    else if (m_bHotTrack && nItem == m_nHoverItem)
    {
        if (m_pItemSkin != NULL)
            nBgImg = 1;
        else if (CR_INVALID != m_crItemHotBg)
            crItemBg = m_crItemHotBg;

        if (CR_INVALID != m_crSelText)
            crText = m_crSelText;
    }
    if (CR_INVALID != crItemBg) // Draw background first
        pRT->FillSolidRect(rcItem, crItemBg);

    if (m_pItemSkin != NULL) // If there is a skin, overlay the background
        m_pItemSkin->DrawByIndex(pRT, rcItem, nBgImg);

    // Keyboard cursor indicator: dotted frame around the cursor item in
    // multi-selection mode while the control has focus (parity with the
    // item-focus visuals of the SViewBase-family controls).
    if ((m_bMultiSelection || m_bCheckBox) && nItem == m_nSelectItem && IsFocused())
        DrawDefFocusRect(pRT, rcItem);

    // Add padding on the left
    rcItem.left += ITEM_MARGIN;

    if (CR_INVALID != crText)
    {
        bTextColorChanged = TRUE;
        crOldText = pRT->SetTextColor(crText);
    }

    CRect rcCol(rcItem);
    rcCol.right = rcCol.left;
    rcCol.OffsetRect(-m_ptOrigin.x, 0);
    int nItemHeight = m_nItemHeight.toPixelSize(GetScale());
    for (int nCol = 0; nCol < GetColumnCount(); nCol++)
    {
        CRect rcVisiblePart;

        SHDITEM hdi = { SHDI_WIDTH | SHDI_ORDER, 0 };
        m_pHeader->GetItem(nCol, &hdi);
        rcCol.left = rcCol.right;
        rcCol.right = rcCol.left + hdi.cx;

        rcVisiblePart.IntersectRect(rcItem, rcCol);

        if (rcVisiblePart.IsRectEmpty())
            continue;

        // Highlight the selected column
        if (hdi.iOrder == m_nSelectColumn)
        {
            if (CR_INVALID != m_crItemSelBg)
                pRT->FillSolidRect(rcVisiblePart, m_crItemSelBg);
        }

        // Draw checkbox
        if (nCol == 0 && m_bCheckBox && m_pCheckSkin)
        {
            CSize sizeSkin = m_pCheckSkin->GetSkinSize();
            int nOffsetX = 3;
            int nOffsetY = (nItemHeight - sizeSkin.cy) / 2;
            CRect rcCheck;
            rcCheck.SetRect(0, 0, sizeSkin.cx, sizeSkin.cy);
            rcCheck.OffsetRect(rcCol.left + nOffsetX, rcCol.top + nOffsetY);
            m_pCheckSkin->DrawByIndex(pRT, rcCheck, lvItem.bCheck ? 4 : 0);

            rcCol.left = sizeSkin.cx + 6 + rcCol.left;
        }

        DXLVSUBITEM &subItem = lvItem.arSubItems->GetAt(hdi.iOrder);

        if (subItem.nImage != -1 && m_pIconSkin)
        {
            int nOffsetX = m_ptIcon.x;
            int nOffsetY = m_ptIcon.y;
            CSize sizeSkin = m_pIconSkin->GetSkinSize();
            rcIcon.SetRect(0, 0, sizeSkin.cx, sizeSkin.cy);

            if (m_ptIcon.x == -1)
                nOffsetX = nItemHeight / 6;

            if (m_ptIcon.y == -1)
                nOffsetY = (nItemHeight - sizeSkin.cy) / 2;

            rcIcon.OffsetRect(rcCol.left + nOffsetX, rcCol.top + nOffsetY);
            m_pIconSkin->DrawByIndex(pRT, rcIcon, subItem.nImage);
        }

        UINT align = DT_SINGLELINE;
        rcText = rcCol;

        if (m_ptText.x == -1)
            rcText.left = rcIcon.Width() > 0 ? rcIcon.right + nItemHeight / 6 : rcCol.left;
        else
            rcText.left = rcCol.left + m_ptText.x;

        if (m_ptText.y == -1)
            align |= DT_VCENTER;
        else
            rcText.top = rcCol.top + m_ptText.y;

        pRT->DrawText(subItem.strText, subItem.cchTextMax, rcText, align);
    }

    if (bTextColorChanged)
        pRT->SetTextColor(crOldText);
}

void SListCtrl::OnDestroy()
{
    if (IsInplaceEditing())
        EndInplaceEdit(FALSE);
    DeleteAllItems();

    __baseCls::OnDestroy();
}

int SListCtrl::GetColumnCount() const
{
    if (!m_pHeader)
        return 0;

    return m_pHeader->GetItemCount();
}

int SListCtrl::GetTopIndex() const
{
    return m_ptOrigin.y / m_nItemHeight.toPixelSize(GetScale());
}

void SListCtrl::EnsureVisible(int nItem)
{
    if (nItem < 0 || nItem >= GetItemCount())
        return;

    int nItemHeight = m_nItemHeight.toPixelSize(GetScale());
    int nTop = GetTopIndex();
    int nPage = GetCountPerPage(TRUE);
    if (nItem >= nTop && nItem < nTop + nPage)
        return;

    if (nItem < nTop)
    {
        // Above the view: scroll so the item becomes the top row.
        OnScroll(TRUE, SB_THUMBPOSITION, nItem * nItemHeight);
    }
    else if (nItem == GetItemCount() - 1)
    {
        OnScroll(TRUE, SB_BOTTOM, 0);
    }
    else
    {
        // Below the view: scroll so the item becomes the last fully
        // visible row.
        int nPos = (nItem + 1) * nItemHeight - (int)m_siVer.nPage;
        OnScroll(TRUE, SB_THUMBPOSITION, smax(nPos, 0));
    }
}

UINT SListCtrl::OnGetDlgCode() const
{
    return SC_WANTARROWS | SC_WANTSYSKEY;
}

void SListCtrl::OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags)
{
    int nCount = GetItemCount();
    if (nCount == 0)
    {
        SetMsgHandled(FALSE);
        return;
    }

    BOOL bCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    BOOL bShiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    BOOL bMultiMode = m_bMultiSelection || m_bCheckBox;

    // F2 starts in-place editing of the cursor item's first subitem.
    if (nChar == VK_F2)
    {
        if (m_nSelectItem != -1)
            BeginInplaceEdit(m_nSelectItem, 0);
        return;
    }

    // SPACE toggles the cursor item in multi-selection mode (parity with
    // SListView::OnKeyDown); single-selection lets the dialog handle it.
    if (nChar == VK_SPACE)
    {
        if (bMultiMode && m_nSelectItem != -1)
        {
            NotifySelChange(m_nSelectItem, m_nSelectItem, MK_CONTROL);
            return;
        }
        SetMsgHandled(FALSE);
        return;
    }

    // Ctrl+A selects every item in multi-selection mode.
    if (bCtrlPressed && nChar == 'A')
    {
        if (bMultiMode)
        {
            for (int i = 0; i < nCount; i++)
            {
                SetItemSel(i, TRUE);
            }
            if (m_nSelectItem == -1)
                m_nSelectItem = 0;
            m_nSelAnchor = m_nSelectItem;
            Invalidate();
            return;
        }
        SetMsgHandled(FALSE);
        return;
    }

    // Navigation keys resolve to a new cursor position; with no cursor yet,
    // the navigation starts from the first / last item.
    int nNewSel = -1;
    switch (nChar)
    {
    case VK_DOWN:
        nNewSel = (m_nSelectItem == -1) ? 0 : smin(m_nSelectItem + 1, nCount - 1);
        break;
    case VK_UP:
        nNewSel = (m_nSelectItem == -1) ? 0 : smax(m_nSelectItem - 1, 0);
        break;
    case VK_PRIOR:
        nNewSel = (m_nSelectItem == -1) ? 0 : smax(m_nSelectItem - GetCountPerPage(TRUE), 0);
        break;
    case VK_NEXT:
        nNewSel = (m_nSelectItem == -1) ? nCount - 1 : smin(m_nSelectItem + GetCountPerPage(TRUE), nCount - 1);
        break;
    case VK_HOME:
        nNewSel = 0;
        break;
    case VK_END:
        nNewSel = nCount - 1;
        break;
    default:
        SetMsgHandled(FALSE);
        return;
    }

    EnsureVisible(nNewSel);

    if (bMultiMode && bCtrlPressed)
    {
        // Ctrl + navigation: only the keyboard cursor moves; the set and
        // therefore the selection feedback stay untouched. The anchor
        // follows the cursor like every other non-Shift change.
        if (nNewSel != m_nSelectItem)
        {
            int nOldCursor = m_nSelectItem;
            m_nSelectItem = nNewSel;
            m_nSelAnchor = nNewSel;
            if (nOldCursor != -1)
                RedrawItem(nOldCursor);
            RedrawItem(nNewSel);
        }
        return;
    }

    // Plain navigation replaces the selection with the cursor item;
    // Shift+navigation selects the anchored range [anchor..cursor] (the
    // anchor stays fixed, so the range can shrink as well as grow -
    // Explorer semantics, mirroring SListView::OnKeyDown). In single
    // selection mode the modifier is dropped: Shift just moves the
    // selection like a plain arrow.
    NotifySelChange(m_nSelectItem, nNewSel, (bMultiMode && bShiftPressed) ? MK_SHIFT : 0);
}

void SListCtrl::NotifySelChange(int nOldSel, int nNewSel, UINT nFlags)
{
    // Plain re-selecting the already-selected item (or clearing an already
    // empty selection) in single-selection mode changes nothing: skip the
    // anchor-style event pair entirely (same early-return semantics as
    // STreeView::SetSel / SViewBase::SetSel). Modifier-key paths are
    // excluded - they may flip other items, which is a real change.
    if (!m_bMultiSelection && (nFlags & (MK_CONTROL | MK_SHIFT)) == 0 && nNewSel == nOldSel)
        return;

    // The anchor-style LC SelChanging/SelChanged are a single-selection
    // protocol; multi-selection feedback is per-item only (via
    // NotifyItemSelState below).
    if (!m_bMultiSelection)
    {
        EventLCSelChanging evt1(this);
        evt1.bCancel = FALSE;
        evt1.nOldSel = nOldSel;
        evt1.nNewSel = nNewSel;

        FireEvent(evt1);
        if (evt1.bCancel)
            return;
    }

    if ((m_bMultiSelection || m_bCheckBox) && (nFlags & MK_CONTROL))
    {
        if (nNewSel != -1)
        {
            SetItemSel(nNewSel, !GetSelected(nNewSel));
            m_nSelectItem = nNewSel;
        }
    }
    else if ((m_bMultiSelection || m_bCheckBox) && (nFlags & MK_SHIFT))
    {
        // Shift+Click: select the anchored range [anchor..clicked]
        // (Explorer semantics: the anchor stays fixed, the set is
        // replaced, so the range can shrink as well as grow). The flags
        // are diffed instead of clear-all + refill, so rows already in
        // the desired state are never touched: growing the range only
        // flips/redraws the newly added rows, shrinking only the removed
        // ones.
        if (nNewSel != -1)
        {
            int nAnchor = (m_nSelAnchor != -1) ? m_nSelAnchor : nNewSel;
            int imax = (nAnchor > nNewSel) ? nAnchor : nNewSel;
            int imin = (imax == nAnchor) ? nNewSel : nAnchor;
            int nItemCount = GetItemCount();
            for (int i = 0; i < nItemCount; i++)
            {
                BOOL bInRange = (i >= imin && i <= imax);
                if (bInRange != m_arrSelFlags[i])
                    SetItemSel(i, bInRange);
            }
            // The clicked item becomes the new cursor; the anchor stays
            // fixed so a following Shift+Click can shrink the range.
            m_nSelectItem = nNewSel;
        }
    }
    else
    {
        ClearSelection();
        if (nNewSel != -1)
        {
            SetItemSel(nNewSel, TRUE);
            m_nSelectItem = nNewSel;
        }
    }

    if (!m_bMultiSelection)
    {
        EventLCSelChanged evt2(this);
        evt2.nOldSel = nOldSel;
        evt2.nNewSel = nNewSel;
        FireEvent(evt2);
    }

    // The range anchor follows the cursor on every non-Shift change; a
    // Shift+Click keeps it fixed as the base of the anchored range.
    if ((nFlags & MK_SHIFT) == 0)
        m_nSelAnchor = m_nSelectItem;
}

BOOL SListCtrl::OnScroll(BOOL bVertical, UINT uCode, int nPos)
{
    BOOL bRet = __baseCls::OnScroll(bVertical, uCode, nPos);

    // The editor is a plain child window: it would stay put while the items
    // scroll away underneath it, so commit and close it on any scroll.
    if (IsInplaceEditing())
        EndInplaceEdit(TRUE);

    if (bVertical)
    {
        m_ptOrigin.y = m_siVer.nPos;
    }
    else
    {
        m_ptOrigin.x = m_siHoz.nPos;
        // Handle column header scrolling
        UpdateHeaderCtrl();
    }

    Invalidate();
    if (uCode == SB_THUMBTRACK)
        ScrollUpdate();

    return bRet;
}

void SListCtrl::OnKillFocus(SWND wndFocus)
{
    __baseCls::OnKillFocus(wndFocus);
    // The keyboard cursor frame is drawn only while the control is focused.
    if (m_nSelectItem != -1)
        Invalidate();
}

void SListCtrl::OnLButtonDownEx(UINT nFlags, CPoint pt)
{
    __baseCls::OnLButtonDownEx(nFlags, pt);
    int nSubItem = -1;
    m_nHoverItem = HitTest(pt, &nSubItem);
    BOOL hitCheckBox = HitCheckBox(pt);

    // Selection and checkbox are independent: a checkbox click only flips
    // the check flag (plus the multi-selection sync), it never moves the
    // selection or the keyboard cursor.
    if (hitCheckBox)
        ToggleItemCheck(m_nHoverItem);
    else if (m_nHoverItem != m_nSelectItem && !m_bHotTrack)
        NotifySelChange(m_nSelectItem, m_nHoverItem, nFlags);
    else if (m_nHoverItem != -1 || m_nSelectItem != -1)
        NotifySelChange(m_nSelectItem, m_nHoverItem, nFlags);
}

void SListCtrl::OnLButtonUpEx(UINT nFlags, CPoint pt)
{
    __baseCls::OnLButtonUpEx(nFlags, pt);
}

void SListCtrl::OnLButtonDbClick(UINT nFlags, CPoint pt)
{
    __baseCls::OnLButtonDbClick(nFlags, pt);
    int nSubItem = -1;
    m_nHoverItem = HitTest(pt, &nSubItem);

    // In edit-label mode the double click starts label editing on the hit
    // subitem instead of selecting a column / firing the double-click event.
    if (m_bEditLabel && m_nHoverItem != -1 && nSubItem != -1 && BeginInplaceEdit(m_nHoverItem, nSubItem))
        return;

    // Select column
    SetSelectedColumn(nSubItem);

    if (m_nHoverItem != m_nSelectItem)
        NotifySelChange(m_nSelectItem, m_nHoverItem, nFlags);

    EventLCDbClick evt2(this);
    evt2.nCurSel = m_nHoverItem;
    evt2.pt = pt;
    FireEvent(evt2);
}

void SListCtrl::OnRButtonUp(UINT nFlags, CPoint pt)
{
    EventLCRClick evt2(this);
    evt2.nCurSel = HitTest(pt);
    evt2.pt = pt;
    FireEvent(evt2);
    __baseCls::OnRButtonUp(nFlags, pt);
}

void SListCtrl::UpdateChildrenPosition()
{
    __baseCls::UpdateChildrenPosition();
    UpdateHeaderCtrl();
}

void SListCtrl::OnSize(UINT nType, CSize size)
{
    if (IsInplaceEditing())
        EndInplaceEdit(TRUE);
    __baseCls::OnSize(nType, size);
    UpdateScrollBar();
    UpdateHeaderCtrl();
}

BOOL SListCtrl::OnHeaderClick(IEvtArgs *pEvt)
{
    return true;
}

BOOL SListCtrl::OnHeaderSizeChanging(IEvtArgs *pEvt)
{
    // Column widths changed: the editor rect would no longer match the
    // subitem, so commit and close it.
    if (IsInplaceEditing())
        EndInplaceEdit(TRUE);
    UpdateScrollBar();
    InvalidateRect(GetListRect());

    return true;
}

BOOL SListCtrl::OnHeaderSwap(IEvtArgs *pEvt)
{
    InvalidateRect(GetListRect());

    return true;
}

void SListCtrl::OnMouseMoveEx(UINT nFlags, CPoint pt)
{
    __baseCls::OnMouseMoveEx(nFlags, pt);
    int nHoverItem = HitTest(pt);
    if (m_bHotTrack && nHoverItem != m_nHoverItem)
    {
        m_nHoverItem = nHoverItem;
        Invalidate();
    }
}

BOOL SListCtrl::IsRubberBandSelEnabled() const
{
    return m_bMultiSelection && IsBandSelEnabled();
}

void SListCtrl::OnRubberBandStart()
{
    m_iBandOldSel = m_nSelectItem;
    m_arrBandSnapshot.RemoveAll();
    for (int i = 0; i < GetItemCount(); i++)
    {
        m_arrBandSnapshot.Add(m_arrSelFlags[i]);
    }
}

void SListCtrl::OnRubberBandSelect(const CRect &rcBand, BOOL bAdd)
{
    int nCount = GetItemCount();
    if (nCount == 0)
        return;

    // Map the band vertically onto the row range (rows span the whole width).
    int nItemHei = m_nItemHeight.toPixelSize(GetScale());
    if (nItemHei <= 0)
        return;
    CRect rcList = GetListRect();
    int iFirst = (rcBand.top - rcList.top + m_ptOrigin.y) / nItemHei;
    int iLast = (rcBand.bottom - 1 - rcList.top + m_ptOrigin.y) / nItemHei;
    iFirst = smin(smax(iFirst, 0), nCount - 1);
    iLast = smin(smax(iLast, iFirst), nCount - 1);

    // Per-item realtime feedback: every row whose selection flips fires
    // EventItemSelChanged (via SetItemSel). The anchor-style SelChanged
    // is a single-selection event and is never fired in the band path.
    for (int i = 0; i < nCount; i++)
    {
        BOOL bWant = (i >= iFirst && i <= iLast) || (bAdd && m_arrSelFlags[i]);
        SetItemSel(i, bWant);
    }
    m_nSelectItem = iLast;
}

void SListCtrl::OnRubberBandEnd(const CRect &rcBand, BOOL bCancelled)
{
    if (bCancelled)
    {
        // ESC / external cancel: restore the selection snapshot taken at
        // band start. Each restored row fires its per-item event
        // (EventItemSelChanged) below; the anchor-style SelChanged is a
        // single-selection event and is not fired for multi-selection bands.
        int nCount = smin(GetItemCount(), (int)m_arrBandSnapshot.GetCount());
        for (int i = 0; i < nCount; i++)
        {
            SetItemSel(i, m_arrBandSnapshot[i]);
        }
        m_nSelectItem = m_iBandOldSel;
        m_nSelAnchor = m_nSelectItem;
        m_arrBandSnapshot.RemoveAll();
        Invalidate();
    }
    else
    {
        // Normal end: the cursor stayed on the last banded row; the range
        // anchor follows it so the next Shift+Click works from there.
        m_nSelAnchor = m_nSelectItem;
    }
}

void SListCtrl::OnMouseLeave()
{
    if (m_bHotTrack)
    {
        m_nHoverItem = -1;
        Invalidate();
    }
    __baseCls::OnMouseLeave();
}

void SListCtrl::SetItemSel(int iItem, BOOL bSelected)
{
    if (iItem < 0 || iItem >= GetItemCount())
        return;

    // The selection record lives in the control-level flag array, parallel
    // to m_arrItems; repaint and fire the per-item event only on a flip.
    if (m_arrSelFlags[iItem] == bSelected)
        return;
    m_arrSelFlags.SetAt(iItem, bSelected);
    RedrawItem(iItem);
    NotifyItemSelState(iItem, bSelected);
}

void SListCtrl::ClearSelection()
{
    for (int i = 0; i < GetItemCount(); i++)
    {
        SetItemSel(i, FALSE);
    }
}

BOOL SListCtrl::GetSelected(int nItem) const
{
    if (nItem < 0 || nItem >= m_arrSelFlags.GetCount())
        return FALSE;

    return m_arrSelFlags[nItem];
}

UINT SListCtrl::GetItemState(int nItem, UINT nMask) const
{
    UINT nState = 0;
    if (nMask & S_LVIS_FOCUSED && nItem == m_nSelectItem)
        nState |= S_LVIS_FOCUSED;
    if (nMask & S_LVIS_SELECTED && GetSelected(nItem))
        nState |= S_LVIS_SELECTED;
    return nState;
}

void SListCtrl::SetItemState(int nItem, UINT nState, UINT nMask)
{
    if (nItem < 0 || nItem >= GetItemCount())
        return;

    if (nMask & S_LVIS_SELECTED)
        SetItemSel(nItem, (nState & S_LVIS_SELECTED) != 0);
    if (nMask & S_LVIS_FOCUSED && m_nSelectItem != nItem)
    {
        // Move the keyboard cursor without touching the selection set.
        int nOldCursor = m_nSelectItem;
        m_nSelectItem = nItem;
        if (nOldCursor != -1)
            RedrawItem(nOldCursor);
        RedrawItem(nItem);
    }
}

BOOL SListCtrl::GetCheck(int nItem) const
{
    if (nItem < 0 || nItem >= GetItemCount())
        return FALSE;

    return m_arrItems[nItem].bCheck;
}

BOOL SListCtrl::SetCheck(int nItem, BOOL bCheck)
{
    if (!m_bCheckBox)
        return FALSE;

    if (nItem < 0 || nItem >= GetItemCount())
        return FALSE;

    DXLVITEM &lvItem = m_arrItems[nItem];
    if (lvItem.bCheck != bCheck)
    {
        lvItem.bCheck = bCheck;
        RedrawItem(nItem);
        NotifyItemCheckState(nItem, bCheck);
    }

    return TRUE;
}

int SListCtrl::GetCheckedItemCount()
{
    int ret = 0;

    for (int i = 0; i < GetItemCount(); i++)
    {
        if (m_arrItems[i].bCheck)
            ret++;
    }

    return ret;
}

UINT SListCtrl::GetSelectedCount() const
{
    UINT nCount = 0;
    for (int i = 0; i < m_arrSelFlags.GetCount(); i++)
    {
        if (m_arrSelFlags[i])
            nCount++;
    }
    return nCount;
}

int SListCtrl::GetSelItems(int nMaxItems, int *rgIndex) const
{
    int nCount = 0;
    for (int i = 0; i < GetItemCount() && nCount < nMaxItems; i++)
    {
        if (m_arrSelFlags[i])
            rgIndex[nCount++] = i;
    }
    return nCount;
}

SPOSITION SListCtrl::GetFirstSelectedItemPosition() const
{
    for (int i = 0; i < GetItemCount(); i++)
    {
        if (m_arrSelFlags[i])
            return (SPOSITION)(INT_PTR)(i + 1);
    }
    return NULL;
}

int SListCtrl::GetNextSelectedItem(SPOSITION &pos) const
{
    // The cursor encodes (item index + 1), so a valid position is never NULL.
    int i = (int)(INT_PTR)pos - 1;
    if (i < 0 || i >= GetItemCount() || !m_arrSelFlags[i])
    {
        pos = NULL;
        return -1;
    }

    int nRet = i;
    pos = NULL;
    for (int j = i + 1; j < GetItemCount(); j++)
    {
        if (m_arrSelFlags[j])
        {
            pos = (SPOSITION)(INT_PTR)(j + 1);
            break;
        }
    }
    return nRet;
}

int SListCtrl::GetFirstCheckedItem()
{
    int ret = -1;
    for (int i = 0; i < GetItemCount(); i++)
    {
        if (m_arrItems[i].bCheck)
        {
            ret = i;
            break;
        }
    }

    return ret;
}

int SListCtrl::GetLastCheckedItem()
{
    int ret = -1;
    for (int i = GetItemCount() - 1; i >= 0; i--)
    {
        if (m_arrItems[i].bCheck)
        {
            ret = i;
            break;
        }
    }

    return ret;
}

void SListCtrl::ToggleItemCheck(int nItem)
{
    if (!m_bCheckBox || nItem < 0 || nItem >= GetItemCount())
        return;

    // The clicked item flips its check state; in multi-selection mode a
    // click on a selected item applies the new state to every selected
    // item. The selection record and the keyboard cursor stay untouched.
    DXLVITEM &lvItem = m_arrItems[nItem];
    BOOL bNewCheck = lvItem.bCheck ? FALSE : TRUE;
    lvItem.bCheck = bNewCheck;
    RedrawItem(nItem);
    NotifyItemCheckState(nItem, bNewCheck);
    if (m_bMultiSelection && m_arrSelFlags[nItem])
    {
        for (int i = 0; i < GetItemCount(); i++)
        {
            DXLVITEM &item = m_arrItems[i];
            if (i == nItem || !m_arrSelFlags[i] || item.bCheck == bNewCheck)
                continue;
            item.bCheck = bNewCheck;
            RedrawItem(i);
            NotifyItemCheckState(i, bNewCheck);
        }
    }
}

SHeaderCtrl *SListCtrl::GetHeaderCtrl() const
{
    return m_pHeader;
}

//////////////////////////////////////////////////////////////////////////
// In-place label editing

BOOL SListCtrl::GetInplaceEditRect(int nItem, int nSubItem, CRect &rcEdit)
{
    rcEdit = GetItemRect(nItem, nSubItem);
    if (rcEdit.IsRectEmpty())
        return FALSE;

    // Replicate the text rect offsets of DrawItem for the target column.
    if (nSubItem == 0 && m_bCheckBox && m_pCheckSkin)
    {
        CSize sizeSkin = m_pCheckSkin->GetSkinSize();
        rcEdit.left += sizeSkin.cx + 6;
    }

    const DXLVSUBITEM &subItem = m_arrItems[nItem].arSubItems->GetAt(nSubItem);
    BOOL bHasIcon = subItem.nImage != -1 && m_pIconSkin != NULL;
    if (m_ptText.x == -1)
    {
        if (bHasIcon)
        {
            int nItemHeight = m_nItemHeight.toPixelSize(GetScale());
            CSize sizeSkin = m_pIconSkin->GetSkinSize();
            int nOffsetX = m_ptIcon.x == -1 ? nItemHeight / 6 : m_ptIcon.x;
            rcEdit.left += nOffsetX + sizeSkin.cx + nItemHeight / 6;
        }
        // Without an icon the text starts at the column left edge.
    }
    else
    {
        rcEdit.left += m_ptText.x;
    }
    // The right edge stays at the column's right edge.
    return rcEdit.right > rcEdit.left;
}

BOOL SListCtrl::BeginInplaceEdit(int nItem, int nSubItem)
{
    if (!m_bEditLabel || IsInplaceEditing())
        return FALSE;
    if (!(nItem >= 0 && nItem < GetItemCount() && nSubItem >= 0 && nSubItem < GetColumnCount()))
        return FALSE;

    CRect rcEdit;
    if (!GetInplaceEditRect(nItem, nSubItem, rcEdit))
        return FALSE;

    // Let the application cancel the session or style the editor.
    SStringW strStyle;
    EventLCBeginEditLabel evtBegin(this);
    evtBegin.nItem = nItem;
    evtBegin.nSubItem = nSubItem;
    evtBegin.strStyle = &strStyle;
    evtBegin.bCancel = FALSE;
    FireEvent(evtBegin);
    if (evtBegin.bCancel)
        return FALSE;

    SInplaceEdit *pEdit = new SInplaceEdit(this);
    InsertIChild(pEdit);
    pEdit->GetStyle().SetScale(GetScale());

    if (strStyle.IsEmpty())
    {
        pEdit->ApplyDefaultStyle();
        pEdit->InitFromXml(NULL);
    }
    else
    {
        SXmlDoc xmlDoc;
        xmlDoc.LoadBufferInplace((void *)strStyle.c_str(), strStyle.GetLength() * sizeof(wchar_t), xml_parse_default, sizeof(wchar_t) == 2 ? enc_utf16 : enc_utf32);
        pEdit->InitFromXml(xmlDoc.Root()->FirstChild());
    }

    pEdit->SetWindowText(GetSubItemText(nItem, nSubItem));
    pEdit->Move(rcEdit);
    pEdit->SetVisible(TRUE, TRUE);
    pEdit->SetFocus();
    pEdit->SetSel(0, -1, TRUE);

    m_pInplaceEdit = pEdit;
    m_nEditItem = nItem;
    m_nEditSubItem = nSubItem;
    return TRUE;
}

void SListCtrl::EndInplaceEdit(BOOL bSave)
{
    if (!m_pInplaceEdit)
        return;

    int nEditItem = m_nEditItem;
    int nEditSubItem = m_nEditSubItem;
    SStringT strText = m_pInplaceEdit->GetWindowText();

    // Detach first: handing the focus back (or destroying the editor)
    // re-enters OnInplaceEditEnd through WM_KILLFOCUS.
    SInplaceEdit *pEdit = m_pInplaceEdit;
    m_pInplaceEdit = NULL;
    m_nEditItem = -1;
    m_nEditSubItem = -1;

    if (bSave)
        SetSubItemText(nEditItem, nEditSubItem, strText);

    EventLCEndEditLabel evt(this);
    evt.nItem = nEditItem;
    evt.nSubItem = nEditSubItem;
    evt.strText = &strText;
    evt.bCancel = !bSave;
    FireEvent(evt);

    // Hand the focus back to the control before destroying the editor so
    // the pending WM_KILLFOCUS finds a detached session (no-op).
    SetFocus();

    pEdit->SetVisible(FALSE, TRUE);
    pEdit->Destroy();

    RedrawItem(nEditItem);
}

void SListCtrl::OnInplaceEditEnd(BOOL bSave)
{
    EndInplaceEdit(bSave);
}

SNSEND
