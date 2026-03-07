#include "souistd.h"
#include <control/SToolBar.h>
#include <helper/SMenu.h>
#include <helper/SDIBHelper.h>
#include <helper/slog.h>
#define kLogTag "SToolBar"

SNSBEGIN

SToolBar::SToolBar(void)
    : m_bVert(FALSE)
    , m_bTextIconVertical(FALSE)
    , m_nTextIconInterval(4)
    , m_iClickItem(-1)
    , m_iHoverItem(-1)
    , m_nVisibleItems(0)
    , m_dwDropBtnState(0)
    , m_nMoreButtonSize(0)
    , m_nMaxItemWidth(100)
    , m_bAnimate(TRUE)
    , m_nAniStep(25)
    , m_nItemSize(-2, SLayoutSize::defUnit)
{
    GetEventSet()->addEvent(EVENTID(EventToolBarCmd));
    GetEventSet()->addEvent(EVENTID(EventUpdateCmdTip));
    GetEventSet()->addEvent(EVENTID(EventUpdateCmdUI));
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
            item.strText = S_CW2T(GETSTRING(xmlItem.attribute(SToolBar_style::kStyle_text).as_string()));
            item.strTip = S_CW2T(GETSTRING(xmlItem.attribute(SToolBar_style::kStyle_tip).as_string()));
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
        else
        {
            SWindow *pChild = (SWindow *)SApplication::getSingleton().CreateWindowByName(xmlItem.name());
            if (pChild)
            {
                InsertChild(pChild);
                pChild->InitFromXml(&xmlItem);
                ToolBarItem item;
                item.nId = pChild->GetID();
                item.pChild = pChild;
                m_arrItems.Add(item);
            }
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

CSize SToolBar::GetItemSize(const CRect &rcWnd, IRenderTarget *pRT, int iItem) const
{
    CSize szRet;
    if (iItem < 0 || iItem >= m_arrItems.GetCount())
        return szRet;
    const ToolBarItem &item = m_arrItems[iItem];
    if (m_bVert)
    {
        if (IsSeparator(&item))
        {
            szRet.cy = GetSepWid();
        }
        else if (item.pChild)
        {
            item.pChild->GetDesiredSize(&szRet, rcWnd.Width(), SIZE_WRAP_CONTENT);
        }
        else
        {
            CSize szIcon;
            if (item.icon)
                szIcon = item.icon->Size();
            else if (m_skinIcons)
                szIcon = m_skinIcons[0]->GetSkinSize();
            if(m_nItemSize.isWrapContent())
                szRet.cy = szIcon.cy;
            else if(m_nItemSize.isMatchParent())
                szRet.cy = rcWnd.Width();
            else
                szRet.cy = m_nItemSize.toPixelSize(GetScale());
            szRet.cy += 8; //padding
            if ((item.dwStyle & BTNS_SHOWTEXT))
            {
                CSize szText;
                pRT->MeasureText(item.strText, item.strText.GetLength(), &szText);
                szRet.cy += szText.cy + m_nTextIconInterval;
            }
        }
        szRet.cx = rcWnd.Width();
    }
    else
    {
        if (IsSeparator(&item))
        {
            szRet = CSize(GetSepWid(), rcWnd.Height());
        }
        else if (item.pChild)
        {
            item.pChild->GetDesiredSize(&szRet, SIZE_WRAP_CONTENT, rcWnd.Height());
        }
        else
        {
            CSize szIcon;
            if (item.icon)
                szIcon = item.icon->Size();
            else if (m_skinIcons)
                szIcon = m_skinIcons[0]->GetSkinSize();

            if (m_bTextIconVertical && (item.dwStyle & BTNS_SHOWTEXT))
            {
                // For vertical text-icon arrangement, height should be larger to accommodate both
                CSize szTxt;
                pRT->MeasureText(item.strText, item.strText.GetLength(), &szTxt);
                szRet.cx = smax(rcWnd.Height(), szIcon.cx);
                szRet.cy = szIcon.cy + szTxt.cy + m_nTextIconInterval * 2;
            }
            else
            {
                // Default horizontal arrangement
                szRet.cy = rcWnd.Height();
                if(m_nItemSize.isWrapContent())
                    szRet.cx = szIcon.cx;
                else if(m_nItemSize.isMatchParent())
                    szRet.cx = rcWnd.Height();
                else
                    szRet.cx = m_nItemSize.toPixelSize(rcWnd.Width());
                if ((item.dwStyle & BTNS_SHOWTEXT))
                {
                    CSize szTxt;
                    pRT->MeasureText(item.strText, item.strText.GetLength(), &szTxt);
                    szTxt.cx = smin(szTxt.cx, m_nMaxItemWidth);
                    szRet.cx += szTxt.cx + m_nTextIconInterval;
                }
            }

            if ((item.dwStyle & TBSTYLE_DROPDOWN) && m_skinDropArrow)
            {
                CSize szDropArrow = m_skinDropArrow->GetSkinSize();
                szRet.cx += szDropArrow.cx;
            }
        }
    }
    return szRet;
}

SIZE SToolBar::MeasureContent(int nParentWid, int nParentHei)
{
    IRenderTarget *pRT = NULL;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
    BeforePaintEx(pRT);
    CSize szContent;
    CRect rcWnd(0, 0, nParentWid, nParentHei);
    for (UINT i = 0; i < m_arrItems.GetCount(); i++)
    {
        CSize szItem = GetItemSize(rcWnd, pRT, i);
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

int SToolBar::GetSepWid() const
{
    if (!m_skinSep)
        return 2;
    CSize szSep = m_skinSep->GetSkinSize();
    return m_bVert ? szSep.cy : szSep.cx;
}

BOOL SToolBar::IsSeparator(const ToolBarItem *pItem) const
{
    return pItem->dwStyle & TBSTYLE_SEP;
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
            IRenderTarget *pRT = NULL;
            GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
            BeforePaintEx(pRT);
            CSize szItem = GetItemSize(rc, pRT, i);
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
    IRenderTarget *pRT = NULL;
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
            CSize szItem = GetItemSize(rc, pRT, i);
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

void SToolBar::DrawItem(IRenderTarget *pRT, const CRect &rcItem, const ToolBarItem *pItem, int iItemIndex)
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
            szIcon = m_skinIcons[0]->GetSkinSize();

        // Calculate content area, reserve space for dropdown arrow if needed
        CRect rcContent = rcItem;
        CRect rcDropArrow;
        bool bHasDropDown = (pItem->dwStyle & TBSTYLE_DROPDOWN) != 0;
        int nDropArrowWidth = 0;
        int nDropArrowHeight = 0;

        if (bHasDropDown && m_skinDropArrow)
        {
            CSize szDropArrow = m_skinDropArrow->GetSkinSize();
            nDropArrowWidth = szDropArrow.cx;
            nDropArrowHeight = szDropArrow.cy;

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
                rcContent.bottom -= nDropArrowHeight;
                rcDropArrow.left = rcItem.left;
                rcDropArrow.right = rcItem.right;
                rcDropArrow.top = rcContent.bottom;
                rcDropArrow.bottom = rcItem.bottom;
            }
        }

        // Draw main button background with animation support
        if (m_skinState)
        {
            const ToolBarItem &item = m_arrItems[iItemIndex];
            if (item.byAlphaAni == 0xFF)
            { // Not in animation
                m_skinState->DrawByState(pRT, rcContent, pItem->dwState);
                if (bHasDropDown)
                {
                    m_skinState->DrawByState(pRT, rcDropArrow, pItem->dwState);
                }
            }
            else
            { // In animation
                BYTE byNewAlpha = item.byAlphaAni;
                if (pItem->dwState & WndState_Hover)
                {
                    // Get hover state - fade from normal to hover
                    m_skinState->DrawByState2(pRT, rcContent, WndState_Normal, (0xff - byNewAlpha));
                    m_skinState->DrawByState2(pRT, rcContent, WndState_Hover, byNewAlpha);

                    if (bHasDropDown)
                    {
                        m_skinState->DrawByState2(pRT, rcDropArrow, WndState_Normal, (0xff - byNewAlpha));
                        m_skinState->DrawByState2(pRT, rcDropArrow, WndState_Hover, byNewAlpha);
                    }
                }
                else
                {
                    // Leave hover state - fade from hover to normal
                    m_skinState->DrawByState2(pRT, rcContent, WndState_Hover, (0xff - byNewAlpha));
                    m_skinState->DrawByState2(pRT, rcContent, WndState_Normal, byNewAlpha);
                    if (bHasDropDown)
                    {
                        m_skinState->DrawByState2(pRT, rcDropArrow, WndState_Hover, (0xff - byNewAlpha));
                        m_skinState->DrawByState2(pRT, rcDropArrow, WndState_Normal, byNewAlpha);
                    }
                }
            }
        }
        // Icon is always displayed. Check if text should be shown.
        BOOL bShowText = (pItem->dwStyle & BTNS_SHOWTEXT) != 0 && (!m_bVert || m_bTextIconVertical);

        int iState = SState2Index::GetDefIndex(pItem->dwState, true);
        ISkinObj *_skinIcons = m_skinIcons[iState];
        if(!_skinIcons) _skinIcons = m_skinIcons[0];

        if (bShowText)
        {
            // Display both icon and text
            COLORREF crTxt = GetStyle().GetTextColor(iState);
            COLORREF crOld = pRT->GetTextColor();
            if (crTxt != CR_INVALID)
                pRT->SetTextColor(crTxt);

            CSize szTxt;
            pRT->MeasureText(pItem->strText, pItem->strText.GetLength(), &szTxt);

            if (m_bTextIconVertical)
            {
                // Vertical arrangement: icon above text
                int nInter = (rcContent.Height() - szIcon.cy - szTxt.cy) / 3;
                CRect rcIcon(rcContent.CenterPoint(), szIcon);
                rcIcon.OffsetRect(-szIcon.cx / 2, 0);
                rcIcon.MoveToY(rcContent.top + nInter);
                if (pItem->icon)
                    pRT->DrawBitmapEx(rcIcon, pItem->icon, CRect(CPoint(), szIcon), EM_STRETCH, 0xff);
                else if (_skinIcons && pItem->iIcon >= 0)
                    _skinIcons->DrawByIndex(pRT, rcIcon, pItem->iIcon);

                CRect rcTxt(rcContent.CenterPoint(), szTxt);
                rcTxt.OffsetRect(-szTxt.cx / 2, 0);
                rcTxt.MoveToY(rcIcon.bottom + nInter);
                pRT->TextOut(rcTxt.left, rcTxt.top, pItem->strText.c_str(), pItem->strText.GetLength());
            }
            else
            {
                // Horizontal arrangement: icon left of text
                int nInter = (rcContent.Height() - smax(szIcon.cy, szTxt.cy)) / 2;
                CRect rcIcon(rcContent.left + nInter, rcContent.top + nInter, rcContent.left + nInter + szIcon.cx, rcContent.top + nInter + szIcon.cy);
                if (pItem->icon)
                    pRT->DrawBitmapEx(rcIcon, pItem->icon, CRect(CPoint(), szIcon), EM_STRETCH, 0xff);
                else if (_skinIcons && pItem->iIcon >= 0)
                    _skinIcons->DrawByIndex(pRT, rcIcon, pItem->iIcon);

                CRect rcTxt(rcIcon.right + m_nTextIconInterval, rcContent.top + nInter, rcContent.right - nInter, rcContent.bottom - nInter);
                pRT->TextOut(rcTxt.left, rcTxt.top, pItem->strText.c_str(), pItem->strText.GetLength());
            }
            pRT->SetTextColor(crOld);
        }
        else
        {
            // Display icon only (always the case)
            CRect rcIcon(rcContent.CenterPoint(), szIcon);
            rcIcon.OffsetRect(-szIcon.cx / 2, -szIcon.cy / 2);
            if (pItem->icon)
                pRT->DrawBitmapEx(rcIcon, pItem->icon, CRect(CPoint(), szIcon), EM_STRETCH, 0xff);
            else if (_skinIcons && pItem->iIcon >= 0)
                _skinIcons->DrawByIndex(pRT, rcIcon, pItem->iIcon);
        }

        // Draw dropdown arrow if this is a dropdown item
        if (bHasDropDown && m_skinDropArrow)
        {
            CSize szArrow = m_skinDropArrow->GetSkinSize();
            CRect rcArrow(rcDropArrow.CenterPoint(), szArrow);
            rcArrow.OffsetRect(-szArrow.cx / 2, -szArrow.cy / 2);
            m_skinDropArrow->DrawByIndex(pRT, rcArrow, 1); // Use second state for dropdown items
        }
    }
    pRT->PopClip();
}

int SToolBar::OnCreate(void * pcs){
    int ret = __baseCls::OnCreate(pcs);
    if(ret != 0) return ret;
    if (m_skinIcons[0]  && !m_skinIcons[3])
    {
        SSkinImgList* pImg= sobj_cast<SSkinImgList>(m_skinIcons[0]);
        if (pImg)
        {
            SAutoRefPtr<IBitmapS> img=NULL;
            pImg->GetImage()->Clone(&img);
            if (img)
            {
                SDIBHelper::DisabledStyleImage(img);
                SSkinImgList* graySkin =new SSkinImgList();
                graySkin->SetImage(img);
                graySkin->SetStates(pImg->GetStates());
                m_skinIcons[3].Attach(graySkin);                
            }
        }
    }
    return 0;
}

void SToolBar::OnDestroy()
{
    __baseCls::OnDestroy();
    if (GetContainer())
    {
        GetContainer()->UnregisterTimelineHandler(this);
        GetContainer()->GetMsgLoop()->RemoveIdleHandler(this);    
    }
    m_iClickItem = -1;
    m_iHoverItem = -1;
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
            rcItem.bottom += IsSeparator(&item) ? nSep : GetItemSize(rcClient, pRT, i).cy;
        }
        else
        {
            rcItem.left = rcItem.right;
            rcItem.right += IsSeparator(&item) ? nSep : GetItemSize(rcClient, pRT, i).cx;
        }
        if (!item.pChild)
        {
            DrawItem(pRT, rcItem, &item, i);
        }
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
        if (m_arrItems[iItem].dwState & WndState_Disable)
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
        ToolBarItem &item = m_arrItems[m_iClickItem];
        if (item.dwStyle & TBSTYLE_CHECK)
        {
            if (item.dwState & WndState_Check)
                item.dwState ^= WndState_Check;
            else
                item.dwState |= WndState_Check;
        }
        item.dwState &= ~WndState_PushDown;
        CRect rc = GetItemRect(m_iClickItem);
        InvalidateRect(rc);
        int iHover = HitTest(pt);
        int iOldClick = m_iClickItem;
        m_iClickItem = -1;
        OnMouseMove(nFlags, pt);
        if (iHover == iOldClick)
        {
            // Check if clicked on dropdown arrow of a TBSTYLE_DROPDOWN item
            if (IsPointOnDropDownArrow(iOldClick, pt))
            {
                // Show dropdown menu for this item
                ToolBarItem &item = m_arrItems[iOldClick];
                if (item.dwStyle & TBSTYLE_DROPDOWN)
                {
                    CRect rcItem = GetItemRect(iOldClick);
                    ShowItemDropDownMenu(rcItem, iOldClick);
                }
            }
            else
            {
                // Normal item click
                ToolBarItem &item = m_arrItems[iOldClick];
                EventToolBarCmd evt(this);
                evt.iItem = iOldClick;
                evt.nCmdId = item.nId;
                evt.strText = &item.strText;
                evt.lParam = item.lParam;
                evt.bDropDown = FALSE;
                FireEvent(evt);
            }
        }
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
                OnItemLeave(m_iHoverItem);
                m_iHoverItem = -1;
            }
            if (iItem != -1)
            {
                if (OnItemHover(iItem))
                {
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
        }
        else
        {
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
    // Stop all item animations
    for (UINT i = 0; i < m_arrItems.GetCount(); i++)
    {
        StopItemAnimate(i);
    }
    UpdateVisibleItemCount();
}

void SToolBar::OnMouseLeave()
{
    if (m_iClickItem != -1)
        return;
    if (m_iHoverItem != -1)
    {
        OnItemLeave(m_iHoverItem);
        m_iHoverItem = -1;
    }
    if (m_dwDropBtnState & WndState_Hover)
    {
        m_dwDropBtnState = 0;
        InvalidateRect(GetDropdownButtonRect());
    }

    EventUpdateCmdTip evt(this);
    evt.iIndex = -1;
    evt.nCmdId = -1;
    evt.strTip = NULL;
    FireEvent(&evt);
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

void SToolBar::SetIconsSkin(SAutoRefPtr<ISkinObj> skinIcons, int iState)
{
    m_skinIcons[iState] = skinIcons;
    Invalidate();
}

void SToolBar::UpdateVisibleItemCount()
{
    IRenderTarget *pRT = NULL;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
    BeforePaintEx(pRT);
    // Calculate m_nMoreButtonSize dynamically based on drop arrow size
    m_nMoreButtonSize = 0;

    int nSep = GetSepWid();
    CRect rcClient = GetClientRect();
    CRect rcItem = rcClient;
    if (m_bVert)
    {
        rcItem.bottom = rcItem.top;
    }
    else
    {
        rcItem.right = rcItem.left;
    }
    m_nVisibleItems = m_arrItems.GetCount();
    for (int i = 0; i < m_arrItems.GetCount(); i++)
    {
        if (IsSeparator(&m_arrItems[i]))
        {
            if (m_bVert)
            {
                rcItem.top = rcItem.bottom;
                rcItem.bottom += nSep;
            }
            else
            {
                rcItem.left = rcItem.right;
                rcItem.right += nSep;
            }
        }
        else
        {
            CSize szItem = GetItemSize(rcClient, pRT, i);
            if (m_bVert)
            {
                rcItem.top = rcItem.bottom;
                rcItem.bottom += szItem.cy;
            }
            else
            {
                rcItem.left = rcItem.right;
                rcItem.right += szItem.cx;
            }
        }
        if (m_bVert)
        {
            if (rcItem.bottom > rcClient.bottom)
            {
                m_nVisibleItems = i;
                break;
            }
        }
        else
        {
            if (rcItem.right > rcClient.right)
            {
                m_nVisibleItems = i;
                break;
            }
        }
    }
    if (m_nVisibleItems < m_arrItems.GetCount())
    { // show more button for hidden items
        if (m_skinDropArrow)
        {
            CSize szDropArrow = m_skinDropArrow->GetSkinSize();
            m_nMoreButtonSize = szDropArrow.cx;
        }
        if (m_bVert)
        {
            if (rcClient.bottom - rcItem.top < m_nMoreButtonSize)
                m_nVisibleItems--;
        }
        else
        {
            if (rcClient.right - rcItem.left < m_nMoreButtonSize)
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
    if (m_skinState)
        m_skinState->DrawByState(pRT, rc, dwState);
    if (m_skinDropArrow)
    {
        CSize szArrow = m_skinDropArrow->GetSkinSize();
        CPoint ptCenter = rc.CenterPoint();
        CRect rcArrow(ptCenter, szArrow);
        rcArrow.OffsetRect(-szArrow.cx / 2, -szArrow.cy / 2);
        if (m_bVert)
        {
            rcArrow.MoveToX(rc.right - szArrow.cx);
        }
        else
        {
            rcArrow.MoveToY(rc.bottom - szArrow.cy);
        }
        m_skinDropArrow->DrawByIndex(pRT, rcArrow, 0);
    }
}

void SToolBar::ShowItemDropDownMenu(const CRect &rc, int iItem)
{
    if (iItem < 0 || iItem >= (int)m_arrItems.GetCount())
        return;

    const ToolBarItem &item = m_arrItems[iItem];

    EventToolBarCmd evt(this);
    evt.iItem = iItem;
    evt.nCmdId = item.nId;
    evt.strText = &item.strText;
    evt.lParam = item.lParam;
    evt.bDropDown = TRUE;
    FireEvent(evt);
}

void SToolBar::ShowDropDownMenu(const CRect &rc)
{
    SMenu menu;
    menu.SetIconSkin(m_skinIcons[0]);
    SXmlNode xmlNode = m_menuStyle.root().child(L"menuStyle");
    menu.LoadMenu2(&xmlNode);
    for (int i = m_nVisibleItems; i < m_arrItems.GetCount(); i++)
    {
        if (m_arrItems[i].pChild)
            continue;
        UINT uFlag = IsSeparator(&m_arrItems[i]) ? MF_SEPARATOR : MF_STRING;
        if (m_arrItems[i].dwState & WndState_Check)
            uFlag |= MF_CHECKED;
        if (m_arrItems[i].dwState & WndState_Disable)
            uFlag |= MF_GRAYED;
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
        evt.bDropDown = FALSE;
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

    if (!m_skinDropArrow)
        return FALSE;

    CSize szDropArrow = m_skinDropArrow->GetSkinSize();
    CRect rcItem = GetItemRect(iItem);
    CRect rcDropArrow;

    if (!m_bVert)
    {
        rcDropArrow.left = rcItem.right - szDropArrow.cx;
        rcDropArrow.right = rcItem.right;
        rcDropArrow.top = rcItem.top;
        rcDropArrow.bottom = rcItem.bottom;
    }
    else
    {
        rcDropArrow.left = rcItem.left;
        rcDropArrow.right = rcItem.right;
        rcDropArrow.top = rcItem.bottom - szDropArrow.cy;
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

BOOL SToolBar::OnItemHover(int iItem)
{
    if (iItem < 0 || iItem >= (int)m_arrItems.GetCount())
        return FALSE;
    BOOL ret = FALSE;
    ToolBarItem &item = m_arrItems[iItem];
    if (!((item.dwState & WndState_Disable) || (item.dwStyle & TBSTYLE_CHECK) && (item.dwState & WndState_Check) || item.pChild))
    {
        item.dwState |= WndState_Hover;
        CRect rc = GetItemRect(iItem);

        // Start animation if enabled
        if (m_bAnimate)
        {
            StopItemAnimate(iItem);
            item.byAlphaAni = 50; // ani alpha from 50 to 255
            GetContainer()->RegisterTimelineHandler(this);
        }

        InvalidateRect(rc);
        ret = TRUE;
    }
    EventUpdateCmdTip evt(this);
    evt.iIndex = iItem;
    evt.nCmdId = item.nId;
    if (!item.strTip.IsEmpty())
        evt.strTip = &item.strTip;
    else
        evt.strTip = &item.strText;
    FireEvent(&evt);
    return ret;
}

void SToolBar::OnItemLeave(int iItem)
{
    if (iItem < 0 || iItem >= (int)m_arrItems.GetCount())
        return;

    ToolBarItem &item = m_arrItems[iItem];
    if ((item.dwStyle & TBSTYLE_CHECK) && (item.dwState & WndState_Check))
        return;
    if (item.pChild)
        return;
    item.dwState &= ~WndState_Hover;
    CRect rc = GetItemRect(iItem);

    // Start animation if enabled
    if (m_bAnimate)
    {
        StopItemAnimate(iItem);
        item.byAlphaAni = 50; // ani alpha from 50 to 255 (but leaving hover)
        GetContainer()->RegisterTimelineHandler(this);
    }

    InvalidateRect(rc);
}

void SToolBar::StopItemAnimate(int iItem)
{
    if (iItem >= 0 && iItem < (int)m_arrItems.GetCount())
    {
        m_arrItems[iItem].byAlphaAni = 0xFF;
    }
}

void SToolBar::OnNextFrame()
{
    // Update animation for all items
    BOOL bAnyAnimating = FALSE;
    for (int i = 0; i < (int)m_arrItems.GetCount(); i++)
    {
        ToolBarItem &item = m_arrItems[i];
        if (item.byAlphaAni != 0xFF)
        {
            bAnyAnimating = TRUE;
            int byAlpha = item.byAlphaAni + m_nAniStep;
            if (byAlpha > 0xFF)
            {
                byAlpha = 0xFF;
            }
            item.byAlphaAni = (BYTE)byAlpha;
            InvalidateRect(GetItemRect(i));
        }
    }

    // Unregister if no more animations
    if (!bAnyAnimating && GetContainer())
    {
        GetContainer()->UnregisterTimelineHandler(this);
    }
}

void SToolBar::OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer)
{
    __baseCls::OnContainerChanged(pOldContainer, pNewContainer);
    if (pOldContainer)
    {
        pOldContainer->UnregisterTimelineHandler(this);
        pOldContainer->GetMsgLoop()->RemoveIdleHandler(this);
    }
    if (pNewContainer)
    {
        pNewContainer->RegisterTimelineHandler(this);
        pNewContainer->GetMsgLoop()->AddIdleHandler(this);
    }
}

BOOL SToolBar::OnIdle(int iRun)
{
    if (iRun == 0)
    { // update toolbar state, only for button item.
        for (size_t i = 0; i < m_arrItems.GetCount(); i++)
        {
            ToolBarItem &item = m_arrItems[i];
            if (IsSeparator(&item) || item.pChild)
                continue;
            EventUpdateCmdUI evt(this);
            evt.nCmdId = m_arrItems[i].nId;
            evt.iIndex = i;
            evt.bEnable = (item.dwState & WndState_Disable) ? FALSE : TRUE;
            evt.bChecked = (item.dwState & WndState_Check) ? TRUE : FALSE;
            if (FireEvent(evt))
            {
                DWORD dwState = item.dwState;
                if (evt.bEnable)
                    item.dwState &= ~WndState_Disable;
                else
                    item.dwState |= WndState_Disable;
                if (evt.bChecked)
                    item.dwState |= WndState_Check;
                else
                    item.dwState &= ~WndState_Check;
                if (item.dwState != dwState)
                    InvalidateRect(GetItemRect(i));
            }
        }
    }
    return TRUE;
}

BOOL SToolBar::SetItemChild(int iItem, SWindow *pChild)
{
    if (iItem < 0 || iItem >= (int)m_arrItems.GetCount())
        return FALSE;

    ToolBarItem &item = m_arrItems[iItem];

    // Remove old child if exists
    if (item.pChild)
    {
        // Destroy the child
        DestroyChild(item.pChild);
        item.pChild = NULL;
    }

    if (!pChild)
        return TRUE;

    // Set new child
    item.pChild = pChild;
    InsertChild(pChild);
    RequestRelayout();
    return TRUE;
}

SWindow *SToolBar::GetItemChild(int iItem) const
{
    if (iItem < 0 || iItem >= (int)m_arrItems.GetCount())
        return NULL;

    return m_arrItems[iItem].pChild;
}
void SToolBar::UpdateChildrenPosition()
{
    for (size_t i = 0; i < m_arrItems.GetCount(); i++)
    {
        ToolBarItem &item = m_arrItems[i];
        if (!item.pChild)
            continue;
        CRect rcItem = GetItemRect(i);
        item.pChild->OnRelayout(rcItem);
    }
}

SNSEND
