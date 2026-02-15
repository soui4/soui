#include "souistd.h"
#include <control/SToolBar.h>
#include <helper/SMenu.h>

SNSBEGIN

SToolBar::SToolBar(void)
    : m_bVert(FALSE)
    , m_iClickItem(-1)
    , m_iHoverItem(-1)
    , m_nVisibleItems(0)
    , m_nDropDownSize(15)
    , m_dwDropBtnState(0)
    , m_nMoreButtonSize(0)
    , m_nInterItemSep(4)
    , m_nMaxItemWidth(100)
{
    GetEventSet()->addEvent(EVENTID(EventToolBarCmd));
}

SToolBar::~SToolBar(void)
{
}

BOOL SToolBar::CreateChildren(SXmlNode xmlNode)
{
    SXmlNode xmlItem = xmlNode.child(SToolBar_style::kStyle_data).first_child();
    while (xmlItem)
    {
        if (_wcsicmp(xmlItem.name(), SToolBar_style::kStyle_item) == 0)
        { // add item
            ToolBarItem item;
            item.iIcon = xmlItem.attribute(SToolBar_style::kStyle_iconIndex).as_int(0);
            item.nId = xmlItem.attribute(SToolBar_style::kStyle_id).as_int(0);
            item.lParam = xmlItem.attribute(SToolBar_style::kStyle_dataAttr).as_uint(0);
            item.strText = S_CW2T(xmlItem.attribute(SToolBar_style::kStyle_text).as_string());
            item.strTip = S_CW2T(xmlItem.attribute(SToolBar_style::kStyle_tip).as_string());
            item.dwState = xmlItem.attribute(SToolBar_style::kStyle_disable).as_bool(false) ? WndState_Disable : WndState_Normal;

            // Parse style attributes
            item.dwStyle = TBSTYLE_BUTTON;
            if (xmlItem.attribute(SToolBar_style::kStyle_check).as_bool(false))
                item.dwStyle |= TBSTYLE_CHECK;
            if (xmlItem.attribute(SToolBar_style::kStyle_dropdown).as_bool(false))
                item.dwStyle |= TBSTYLE_DROPDOWN;
            if (xmlItem.attribute(SToolBar_style::kStyle_showText).as_bool(false))
                item.dwStyle |= BTNS_SHOWTEXT;

            if (xmlItem.attribute(SToolBar_style::kStyle_checked).as_bool(false))
                item.dwState |= WndState_Check;
            m_arrItems.Add(item);
        }
        else if (_wcsicmp(xmlItem.name(), SToolBar_style::kStyle_sep) == 0)
        {
            ToolBarItem item;
            item.nId = -1;
            item.dwStyle = TBSTYLE_SEP;
            m_arrItems.Add(item);
        }
        xmlItem = xmlItem.next_sibling();
    }

    // Load menu style template
    // Try to find menuStyle from XML first
    if (xmlNode.child(SToolBar_style::kStyle_menuStyle))
    {
        m_menuStyle.root().append_copy(xmlNode.child(SToolBar_style::kStyle_menuStyle));
    }
    return TRUE;
}

SIZE SToolBar::MeasureContent(int nParentWid, int nParentHei)
{
    IRenderTarget *pRT=NULL;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
    BeforePaintEx(pRT);
    CSize szContent;
    for (UINT i = 0; i < m_arrItems.GetCount(); i++)
    {
        CSize szItem = GetItemSize(pRT,i);
        if (m_bVert)
        {
            szContent.cx = smax(szContent.cx, szItem.cx);
            szContent.cy += szItem.cy;
        }
        else
        {
            szContent.cx += szItem.cx;
            szContent.cy = smax(szContent.cy, szItem.cy);
        }
    }
    pRT->Release();
    return szContent;
}

CSize SToolBar::GetItemSize(IRenderTarget *pRT, int iItem) const
{
    CRect rc = GetClientRect();
    CSize szRet;
    if (iItem<0 || iItem >= m_arrItems.GetCount())
        return szRet; 
    const ToolBarItem &item = m_arrItems[iItem];
    if (m_bVert)
    {
        szRet = CSize(rc.Width(), item.nId < 0 ? GetSepWid() : rc.Width());
    }
    else
    {
        szRet = CSize(rc.Height(), item.nId < 0 ? GetSepWid() : rc.Height());
        if (item.nId >= 0 && (item.dwStyle & BTNS_SHOWTEXT))
        {
            CSize szTxt;
            pRT->MeasureText(item.strText, item.strText.GetLength(), &szTxt);
            szTxt.cx = smin(szTxt.cx, m_nMaxItemWidth);
            szRet.cx += szTxt.cx + m_nInterItemSep;
        }
        
        if (item.nId >= 0 && (item.dwStyle & TBSTYLE_DROPDOWN))
        {
            szRet.cx += m_nDropDownSize;
        }
    }
    return szRet;
}

int SToolBar::GetSepWid() const
{
    if (!m_skinSep)
        return 2;
    CSize szSep = m_skinSep->GetSkinSize();
    return m_bVert ? szSep.cy : szSep.cx;
}

BOOL SToolBar::IsSeparator(const ToolBarItem *pItem) const
{
    return pItem->nId < 0;
}

CRect SToolBar::GetItemRect(int iItem) const
{
    if (iItem < 0 || iItem >= m_arrItems.GetCount())
        return CRect();
    int nSep = GetSepWid();
    CRect rc = GetClientRect();
    if (m_bVert)
    {
        rc.bottom = rc.top;
    }
    else
    {
        rc.right = rc.left;
    }
    for (int i = 0; i <= iItem; i++)
    {
        if (IsSeparator(&m_arrItems[i]))
        {
            if (m_bVert)
            {
                rc.top = rc.bottom;
                rc.bottom += nSep;
            }
            else
            {
                rc.left = rc.right;
                rc.right += nSep;
            }
        }
        else
        {
            IRenderTarget *pRT=NULL;
            GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
            BeforePaintEx(pRT);
            CSize szItem = GetItemSize(pRT,i);
            pRT->Release();
            if (m_bVert)
            {
                rc.top = rc.bottom;
                rc.bottom += szItem.cy;
            }
            else
            {
                rc.left = rc.right;
                rc.right += szItem.cx;
            }
        }
    }
    return rc;
}

int SToolBar::HitTest(CPoint pt) const
{
    int nSep = GetSepWid();
    CRect rc = GetClientRect();
    if (m_bVert)
    {
        rc.bottom = rc.top;
    }
    else
    {
        rc.right = rc.left;
    }
    int ret = -1;
    IRenderTarget *pRT=NULL;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
    BeforePaintEx(pRT);
    for (int i = 0; i < m_nVisibleItems; i++)
    {
        if (IsSeparator(&m_arrItems[i]))
        {
            if (m_bVert)
            {
                rc.top = rc.bottom;
                rc.bottom += nSep;
            }
            else
            {
                rc.left = rc.right;
                rc.right += nSep;
            }
        }
        else
        {
            CSize szItem = GetItemSize(pRT,i);
            if (m_bVert)
            {
                rc.top = rc.bottom;
                rc.bottom += szItem.cy;
            }
            else
            {
                rc.left = rc.right;
                rc.right += szItem.cx;
            }
        }
        if (rc.PtInRect(pt))
        {
            if (!IsSeparator(&m_arrItems[i]))
                ret = i;
            break;
        }
    }
    pRT->Release();
    return ret;
}

void SToolBar::DrawItem(IRenderTarget *pRT, const CRect &rcItem, const ToolBarItem *pItem)
{
    pRT->PushClipRect(rcItem, RGN_AND);
    if (IsSeparator(pItem))
    {
        if (m_skinSep)
            m_skinSep->DrawByIndex(pRT, rcItem, 0);
    }
    else
    {
        CSize szIcon;
        if (pItem->icon)
            szIcon = pItem->icon->Size();
        else if (m_skinIcons)
            szIcon = m_skinIcons->GetSkinSize();

        // Calculate content area, reserve space for dropdown arrow if needed
        CRect rcContent = rcItem;
        CRect rcDropArrow;
        int nDropArrowWidth = 0;
        if (pItem->dwStyle & TBSTYLE_DROPDOWN)
        {
            nDropArrowWidth = m_nDropDownSize;
            if (!m_bVert)
            {
                rcContent.right -= nDropArrowWidth;
                rcDropArrow.left = rcContent.right;
                rcDropArrow.right = rcItem.right;
                rcDropArrow.top = rcItem.top;
                rcDropArrow.bottom = rcItem.bottom;
            }
            else
            {
                rcContent.bottom -= nDropArrowWidth;
                rcDropArrow.left = rcItem.left;
                rcDropArrow.right = rcItem.right;
                rcDropArrow.top = rcContent.bottom;
                rcDropArrow.bottom = rcItem.bottom;
            }
        }

        // Draw main button background
        if (m_skinState)
        {
            if ((pItem->dwStyle & TBSTYLE_DROPDOWN))
            {
                rcContent.right -= nDropArrowWidth;
                m_skinState->DrawByState(pRT, rcContent, pItem->dwState);
                m_skinState->DrawByState(pRT, rcDropArrow, pItem->dwState);
            }else{
                m_skinState->DrawByState(pRT, rcContent, pItem->dwState);
            }
        }    
        // Icon is always displayed. Check if text should be shown.
        BOOL bShowText = (pItem->dwStyle & BTNS_SHOWTEXT) != 0 && !m_bVert;

        if (bShowText)
        {
            // Display both icon and text
            int iState = SState2Index::GetDefIndex(pItem->dwState, true);
            COLORREF crTxt = GetStyle().GetTextColor(iState);
            COLORREF crOld = pRT->GetTextColor();
            if (crTxt != CR_INVALID)
                pRT->SetTextColor(crTxt);

            CSize szTxt;
            pRT->MeasureText(pItem->strText, pItem->strText.GetLength(), &szTxt);
            int nInter = (rcContent.Height() - szIcon.cy - szTxt.cy) / 3;
            CRect rcIcon(rcContent.CenterPoint(), szIcon);
            rcIcon.OffsetRect(-szIcon.cx / 2, 0);
            rcIcon.MoveToY(rcContent.top + nInter);
            if (pItem->icon)
                pRT->DrawBitmapEx(rcIcon, pItem->icon, CRect(CPoint(), szIcon), EM_STRETCH, 0xff);
            else if (m_skinIcons && pItem->iIcon >= 0)
                m_skinIcons->DrawByIndex(pRT, rcIcon, pItem->iIcon);

            CRect rcTxt(rcContent.CenterPoint(), szTxt);
            rcTxt.OffsetRect(-szTxt.cx / 2, 0);
            rcTxt.MoveToY(rcIcon.bottom + nInter);
            pRT->TextOut(rcTxt.left, rcTxt.top, pItem->strText.c_str(), pItem->strText.GetLength());
            pRT->SetTextColor(crOld);
        }
        else
        {
            // Display icon only (always the case)
            CRect rcIcon(rcContent.CenterPoint(), szIcon);
            rcIcon.OffsetRect(-szIcon.cx / 2, -szIcon.cy / 2);
            if (pItem->icon)
                pRT->DrawBitmapEx(rcIcon, pItem->icon, CRect(CPoint(), szIcon), EM_STRETCH, 0xff);
            else if (m_skinIcons && pItem->iIcon >= 0)
                m_skinIcons->DrawByIndex(pRT, rcIcon, pItem->iIcon);
        }

        // Draw dropdown arrow if this is a dropdown item
        if ((pItem->dwStyle & TBSTYLE_DROPDOWN) && m_skinDropArrow)
        {
            m_skinDropArrow->DrawByIndex(pRT, rcDropArrow, 0);
        }
    }
    pRT->PopClip();
}

void SToolBar::OnPaint(IRenderTarget *pRT)
{
    SPainter painter;
    BeforePaint(pRT, painter);
    CRect rcClient = GetClientRect();
    CRect rcItem = rcClient;
    int nSep = GetSepWid();
    if (m_bVert)
        rcItem.bottom = rcItem.top;
    else
        rcItem.right = rcItem.left;
    for (UINT i = 0; i < m_nVisibleItems; i++)
    {
        ToolBarItem &item = m_arrItems[i];
        if (m_bVert)
        {
            rcItem.top = rcItem.bottom;
            rcItem.bottom += item.nId < 0 ? nSep : GetItemSize(pRT,i).cy;
        }
        else
        {
            rcItem.left = rcItem.right;
            rcItem.right += item.nId < 0 ? nSep : GetItemSize(pRT,i).cx;
        }

        DrawItem(pRT, rcItem, &item);
    }
    if (HasDropDownButton())
    {
        CRect rc = GetDropdownButtonRect();
        DrawDropButton(pRT, rc, m_dwDropBtnState);
    }
    AfterPaint(pRT, painter);
}

void SToolBar::OnLButtonDown(UINT nFlags, CPoint pt)
{
    __baseCls::OnLButtonDown(nFlags, pt);
    int iItem = HitTest(pt);
    if (iItem != -1)
    {
        if (m_arrItems[iItem].dwState & (WndState_Disable | WndState_Check))
            return;
        m_iClickItem = iItem;
        m_arrItems[iItem].dwState |= WndState_PushDown;
        CRect rcItem = GetItemRect(iItem);
        InvalidateRect(rcItem);
    }
    else
    {
        CRect rc = GetDropdownButtonRect();
        if (rc.PtInRect(pt))
        {
            m_dwDropBtnState |= WndState_PushDown;
            InvalidateRect(rc);
        }
    }
}

void SToolBar::OnLButtonUp(UINT nFlags, CPoint pt)
{
    __baseCls::OnLButtonUp(nFlags, pt);
    if (m_iClickItem != -1)
    {
        m_arrItems[m_iClickItem].dwState &= ~WndState_PushDown;
        CRect rc = GetItemRect(m_iClickItem);
        InvalidateRect(rc);

        int iHover = HitTest(pt);
        if (iHover == m_iClickItem)
        {
            // Check if clicked on dropdown arrow of a TBSTYLE_DROPDOWN item
            if (IsPointOnDropDownArrow(m_iClickItem, pt))
            {
                // Show dropdown menu for this item
                ToolBarItem &item = m_arrItems[m_iClickItem];
                if (item.dwStyle & TBSTYLE_DROPDOWN)
                {
                    CRect rcItem = GetItemRect(m_iClickItem);
                    ShowItemDropDownMenu(rcItem, m_iClickItem);
                }
            }
            else
            {
                // Normal item click
                ToolBarItem &item = m_arrItems[m_iClickItem];
                EventToolBarCmd evt(this);
                evt.iItem = m_iClickItem;
                evt.nCmdId = item.nId;
                evt.strText = &item.strText;
                evt.lParam = item.lParam;
                FireEvent(evt);
            }
        }
        m_iClickItem = -1;
        OnMouseMove(nFlags, pt);
    }
    else if (m_dwDropBtnState & WndState_PushDown)
    {
        CRect rc = GetDropdownButtonRect();
        m_dwDropBtnState &= ~WndState_PushDown;
        InvalidateRect(rc);
        if (rc.PtInRect(pt))
        {
            m_dwDropBtnState |= WndState_Hover;
            ShowDropDownMenu(rc);
        }
    }
}

void SToolBar::OnMouseMove(UINT nFlags, CPoint pt)
{
    __baseCls::OnMouseMove(nFlags, pt);
    if (m_iClickItem == -1)
    {
        int iItem = HitTest(pt);
        if (iItem != m_iHoverItem)
        {
            if (m_iHoverItem != -1)
            {
                m_arrItems[m_iHoverItem].dwState &= ~WndState_Hover;
                CRect rc = GetItemRect(m_iHoverItem);
                InvalidateRect(rc);
                m_iHoverItem = -1;
            }
            if (iItem != -1)
            {
                if (!(m_arrItems[iItem].dwState & (WndState_Disable | WndState_Check)))
                {
                    m_arrItems[iItem].dwState |= WndState_Hover;
                    CRect rc = GetItemRect(iItem);
                    InvalidateRect(rc);
                    m_iHoverItem = iItem;
                }
            }
        }
        if (iItem == -1)
        {
            CRect rcDropButton = GetDropdownButtonRect();
            if (rcDropButton.PtInRect(pt))
            {
                m_dwDropBtnState |= WndState_Hover;
                InvalidateRect(rcDropButton);
            }
            else if (m_dwDropBtnState & WndState_Hover)
            {
                m_dwDropBtnState &= ~WndState_Hover;
                InvalidateRect(rcDropButton);
            }
        }else{
            if (m_dwDropBtnState & WndState_Hover)
            {
                CRect rcDropButton = GetDropdownButtonRect();
                m_dwDropBtnState &= ~WndState_Hover;
                InvalidateRect(rcDropButton);
            }
        }
    }
}

void SToolBar::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);
    UpdateVisibleItemCount();
}

void SToolBar::OnMouseLeave()
{
    if (m_iClickItem != -1)
        return;
    if (m_iHoverItem != -1)
    {
        m_arrItems[m_iHoverItem].dwState &= ~WndState_Hover;
        InvalidateRect(GetItemRect(m_iHoverItem));
        m_iHoverItem = -1;
    }
    if (m_dwDropBtnState & WndState_Hover)
    {
        m_dwDropBtnState = 0;
        InvalidateRect(GetDropdownButtonRect());
    }
}

BOOL SToolBar::UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
{
    int iItem = HitTest(pt);
    if (iItem == -1)
        return FALSE;
    tipInfo.dwCookie = iItem;
    tipInfo.rcTarget = GetItemRect(iItem);
    tipInfo.strTip = m_arrItems[iItem].strTip;
    if (tipInfo.strTip.IsEmpty())
        tipInfo.strTip = m_arrItems[iItem].strText;
    tipInfo.swnd = m_swnd;
    return TRUE;
}

void SToolBar::InsertItem(const ToolBarItem &item, int nPos /*=-1*/)
{
    if (nPos < 0 || nPos >= m_arrItems.GetCount())
    {
        nPos = m_arrItems.Add(item);
    }
    else
    {
        m_arrItems.InsertAt(nPos, item);
    }
    m_arrItems[nPos].dwState = 0;
    if (m_nVisibleItems == m_arrItems.GetCount() - 1)
        UpdateVisibleItemCount();
    Invalidate();
}

void SToolBar::SetIconsSkin(SAutoRefPtr<ISkinObj> skinIcons)
{
    m_skinIcons = skinIcons;
    Invalidate();
}

void SToolBar::UpdateVisibleItemCount()
{
    IRenderTarget *pRT=NULL;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
    BeforePaintEx(pRT);
    // Calculate m_nMoreButtonSize dynamically based on m_nDropDownSize
    m_nMoreButtonSize = 0;

    int nSep = GetSepWid();
    CRect rcClient = GetClientRect();
    CRect rc = rcClient;
    if (m_bVert)
    {
        rc.bottom = rc.top;
    }
    else
    {
        rc.right = rc.left;
    }
    m_nVisibleItems = m_arrItems.GetCount();
    for (int i = 0; i < m_arrItems.GetCount(); i++)
    {
        if (IsSeparator(&m_arrItems[i]))
        {
            if (m_bVert)
            {
                rc.top = rc.bottom;
                rc.bottom += nSep;
            }
            else
            {
                rc.left = rc.right;
                rc.right += nSep;
            }
        }
        else
        {
            CSize szItem = GetItemSize(pRT,i);
            if (m_bVert)
            {
                rc.top = rc.bottom;
                rc.bottom += szItem.cy;
            }
            else
            {
                rc.left = rc.right;
                rc.right += szItem.cx;
            }
        }
        if (m_bVert)
        {
            if (rc.bottom > rcClient.bottom)
            {
                m_nVisibleItems = i;
                break;
            }
        }
        else
        {
            if (rc.right > rcClient.right)
            {
                m_nVisibleItems = i;
                break;
            }
        }
    }
    if (m_nVisibleItems < m_arrItems.GetCount())
    { // show more button for hidden items
        m_nMoreButtonSize = m_nDropDownSize;
        if (m_bVert)
        {
            if (rcClient.bottom - rc.top < m_nMoreButtonSize)
                m_nVisibleItems--;
        }
        else
        {
            if (rcClient.right - rc.left < m_nMoreButtonSize)
                m_nVisibleItems--;
        }
        if (m_nVisibleItems < 0)
            m_nVisibleItems = 0;
    }
    pRT->Release();
}

BOOL SToolBar::HasDropDownButton() const
{
    return m_nVisibleItems < m_arrItems.GetCount();
}

CRect SToolBar::GetDropdownButtonRect() const
{
    CRect rc;
    if (HasDropDownButton())
    {
        CRect rcItem = GetItemRect(m_nVisibleItems - 1);
        rc = rcItem;
        if (m_bVert)
        {
            rc.top = rc.bottom;
            rc.bottom += m_nMoreButtonSize;
        }
        else
        {
            rc.left = rc.right;
            rc.right += m_nMoreButtonSize;
        }
    }
    return rc;
}

void SToolBar::DrawDropButton(IRenderTarget *pRT, const CRect &rc, DWORD dwState)
{
    if(m_skinState)
        m_skinState->DrawByState(pRT, rc, dwState);
    if (m_skinDropArrow)
    {
        CSize szArrow = m_skinDropArrow->GetSkinSize();
        CPoint ptCenter = rc.CenterPoint();
        CRect rcArrow(ptCenter, szArrow);
        rcArrow.OffsetRect(-szArrow.cx / 2, -szArrow.cy / 2);
        m_skinDropArrow->DrawByIndex(pRT, rcArrow, 0);
    }
}

void SToolBar::ShowItemDropDownMenu(const CRect &rc, int iItem)
{
    if (iItem < 0 || iItem >= (int)m_arrItems.GetCount())
        return;

    const ToolBarItem &item = m_arrItems[iItem];

    // Fire event - applications can handle this in their event handler
    EventToolBarCmd evt(this);
    evt.iItem = iItem;
    evt.nCmdId = item.nId;
    evt.strText = &item.strText;
    evt.lParam = item.lParam;
    FireEvent(evt);

    // Applications can provide their own dropdown menu logic
    // by reacting to this event in their handler
}

void SToolBar::ShowDropDownMenu(const CRect &rc)
{
    SMenu menu;
    menu.SetIconSkin(m_skinIcons);
    SXmlNode xmlNode = m_menuStyle.root().child(L"menuStyle");
    menu.LoadMenu2(&xmlNode);
    for (int i = m_nVisibleItems; i < m_arrItems.GetCount(); i++)
    {
        UINT uFlag = IsSeparator(&m_arrItems[i]) ? MF_SEPARATOR : MF_STRING;
        if (m_arrItems[i].dwState & WndState_Check)
            uFlag |= MF_CHECKED;
        menu.AppendMenu(uFlag, i, m_arrItems[i].strText, m_arrItems[i].iIcon);
    }
    CRect rcHost(rc);
    GetContainer()->FrameToHost(rcHost);
    CPoint pt = rc.BottomRight();
    ::ClientToScreen(GetContainer()->GetHostHwnd(), &pt);

    UINT uCmd = menu.TrackPopupMenu(TPM_TOPALIGN | TPM_RIGHTALIGN | TPM_RETURNCMD, pt.x, pt.y, NULL);
    if (uCmd)
    {
        ToolBarItem &item = m_arrItems[uCmd];
        EventToolBarCmd evt(this);
        evt.iItem = uCmd;
        evt.nCmdId = item.nId;
        evt.strText = &item.strText;
        evt.lParam = item.lParam;
        FireEvent(evt);
    }
}

int SToolBar::GetItemByID(int nID) const
{
    int iRet = -1;
    for (UINT i = 0; i < m_arrItems.GetCount(); i++)
    {
        if (m_arrItems[i].nId == nID)
        {
            iRet = i;
            break;
        }
    }
    return iRet;
}

void SToolBar::SetMenuStyle(SXmlNode xmlMenuStyle)
{
    m_menuStyle.Reset();
    if (xmlMenuStyle)
    {
        m_menuStyle.root().append_copy(xmlMenuStyle);
    }
}

BOOL SToolBar::IsItemShowText(int nItemId) const
{
    int iItem = GetItemByID(nItemId);
    if (iItem == -1)
        return FALSE;
    return (m_arrItems[iItem].dwStyle & BTNS_SHOWTEXT) != 0;
}

BOOL SToolBar::IsDropDownItem(int nItemId) const
{
    int iItem = GetItemByID(nItemId);
    if (iItem == -1)
        return FALSE;
    return (m_arrItems[iItem].dwStyle & TBSTYLE_DROPDOWN) != 0;
}

BOOL SToolBar::IsPointOnDropDownArrow(int iItem, CPoint pt) const
{
    if (iItem < 0 || iItem >= (int)m_arrItems.GetCount())
        return FALSE;

    const ToolBarItem &item = m_arrItems[iItem];
    if (!(item.dwStyle & TBSTYLE_DROPDOWN))
        return FALSE;

    CRect rcItem = GetItemRect(iItem);
    CRect rcDropArrow;

    if (!m_bVert)
    {
        rcDropArrow.left = rcItem.right - m_nDropDownSize;
        rcDropArrow.right = rcItem.right;
        rcDropArrow.top = rcItem.top;
        rcDropArrow.bottom = rcItem.bottom;
    }
    else
    {
        rcDropArrow.left = rcItem.left;
        rcDropArrow.right = rcItem.right;
        rcDropArrow.top = rcItem.bottom - m_nDropDownSize;
        rcDropArrow.bottom = rcItem.bottom;
    }

    return rcDropArrow.PtInRect(pt);
}

int SToolBar::GetItemID(int nIndex) const
{
    if (nIndex < 0 || nIndex >= (int)m_arrItems.GetCount())
        return -1;
    return m_arrItems[nIndex].nId;
}

int SToolBar::CommandToIndex(int nID) const
{
    return GetItemByID(nID);
}

BOOL SToolBar::GetItemInfo(int nIndex, ToolBarItem *pItem) const
{
    if (!pItem || nIndex < 0 || nIndex >= (int)m_arrItems.GetCount())
        return FALSE;
    *pItem = m_arrItems[nIndex];
    return TRUE;
}

BOOL SToolBar::SetItemInfo(int nIndex, const ToolBarItem *pItem)
{
    if (!pItem || nIndex < 0 || nIndex >= (int)m_arrItems.GetCount())
        return FALSE;
    m_arrItems[nIndex] = *pItem;
    return TRUE;
}

void SToolBar::AddButton(int nID, int nImage, LPCTSTR lpszText, int dwStyle)
{
    ToolBarItem item;
    item.nId = nID;
    item.iIcon = nImage;
    item.strText = lpszText ? lpszText : _T("");
    item.dwStyle = dwStyle;
    item.dwState = WndState_Normal;
    InsertItem(item, -1);
}

BOOL SToolBar::DeleteButton(int nIndex)
{
    if (nIndex < 0 || nIndex >= (int)m_arrItems.GetCount())
        return FALSE;
    m_arrItems.RemoveAt(nIndex);
    Invalidate();
    return TRUE;
}

BOOL SToolBar::EnableButton(int nID, BOOL bEnable)
{
    int iItem = CommandToIndex(nID);
    if (iItem == -1)
        return FALSE;

    if (bEnable)
        m_arrItems[iItem].dwState &= ~WndState_Disable;
    else
        m_arrItems[iItem].dwState |= WndState_Disable;

    Invalidate();
    return TRUE;
}

BOOL SToolBar::CheckButton(int nID, BOOL bCheck)
{
    int iItem = CommandToIndex(nID);
    if (iItem == -1)
        return FALSE;

    if (bCheck)
        m_arrItems[iItem].dwState |= WndState_Check;
    else
        m_arrItems[iItem].dwState &= ~WndState_Check;

    Invalidate();
    return TRUE;
}

BOOL SToolBar::PressButton(int nID, BOOL bPress)
{
    int iItem = CommandToIndex(nID);
    if (iItem == -1)
        return FALSE;

    if (bPress)
        m_arrItems[iItem].dwState |= WndState_PushDown;
    else
        m_arrItems[iItem].dwState &= ~WndState_PushDown;

    Invalidate();
    return TRUE;
}

BOOL SToolBar::IsButtonEnabled(int nID) const
{
    int iItem = CommandToIndex(nID);
    if (iItem == -1)
        return FALSE;
    return (m_arrItems[iItem].dwState & WndState_Disable) == 0;
}

BOOL SToolBar::IsButtonChecked(int nID) const
{
    int iItem = CommandToIndex(nID);
    if (iItem == -1)
        return FALSE;
    return (m_arrItems[iItem].dwState & WndState_Check) != 0;
}

BOOL SToolBar::IsButtonPressed(int nID) const
{
    int iItem = CommandToIndex(nID);
    if (iItem == -1)
        return FALSE;
    return (m_arrItems[iItem].dwState & WndState_PushDown) != 0;
}

int SToolBar::GetButtonStyle(int nIndex) const
{
    if (nIndex < 0 || nIndex >= (int)m_arrItems.GetCount())
        return 0;
    return m_arrItems[nIndex].dwStyle;
}

void SToolBar::SetButtonStyle(int nIndex, int nStyle)
{
    if (nIndex < 0 || nIndex >= (int)m_arrItems.GetCount())
        return;
    m_arrItems[nIndex].dwStyle = nStyle;
    Invalidate();
}

SNSEND
