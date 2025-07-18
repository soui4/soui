﻿//////////////////////////////////////////////////////////////////////////
//   File Name: SPanel.h
//////////////////////////////////////////////////////////////////////////
#include "souistd.h"
#include "core/SPanel.h"

#define DEF_UPDATEINTERVAL 0

SNSBEGIN

CRect SPanel::GetScrollBarRect(BOOL bVert) const
{
    CRect rcSb;
    if (!HasScrollBar(bVert))
        return rcSb;

    SWindow::GetClientRect(&rcSb);

    if (bVert)
    {
        rcSb.left = rcSb.right - GetSbWidth();
    }
    else
    {
        rcSb.top = rcSb.bottom - GetSbWidth();
    }
    if (HasScrollBar(!bVert))
    {
        if (bVert)
        {
            rcSb.bottom -= GetSbWidth();
        }
        else
        {
            rcSb.right -= GetSbWidth();
        }
    }

    if (bVert)
    {
        rcSb.top += m_nSbTop.toPixelSize(GetScale());
        rcSb.bottom -= m_nSbBottom.toPixelSize(GetScale());
    }
    else
    {
        rcSb.left += m_nSbLeft.toPixelSize(GetScale());
        rcSb.right -= m_nSbRight.toPixelSize(GetScale());
    }
    return rcSb;
}

ISkinObj *SPanel::GetScrollBarSkin(BOOL bVert) const
{
    return m_pSkinSb;
}

const SCROLLINFO *SPanel::GetScrollBarInfo(BOOL bVert) const
{
    return bVert ? (&m_siVer) : (&m_siHoz);
}

int SPanel::GetScrollBarArrowSize(BOOL bVert) const
{
    if (m_nSbArrowSize.isValid())
        return m_nSbArrowSize.toPixelSize(GetScale());
    SASSERT(m_pSkinSb);
    if (!m_pSkinSb->HasArrow())
        return 0;
    return m_pSkinSb->GetIdealSize();
}

void SPanel::OnScrollUpdatePart(BOOL bVert, int iPart)
{
    SScrollBarHandler &sbHandler = bVert ? m_sbVert : m_sbHorz;
    if (iPart == -1)
    {
        CRect rc = GetScrollBarRect(bVert);
        InvalidateRect(&rc);
    }
    else if (iPart == SB_THUMBTRACK)
    {
        CRect rcRail = sbHandler.GetPartRect(SScrollBarHandler::kSbRail);
        InvalidateRect(&rcRail);
    }
    else if (iPart == SB_PAGEDOWN || iPart == SB_PAGEUP)
    {
        CRect rc = sbHandler.GetPartRect(iPart);
        InvalidateRect(&rc);
    }
    else
    {
        CRect rc = sbHandler.GetPartRect(iPart);
        InvalidateRect(&rc);
    }
}

ISwndContainer *SPanel::GetScrollBarContainer()
{
    return GetContainer();
}

void SPanel::OnScrollUpdateThumbTrack(BOOL bVert, int nPos)
{
    SScrollBarHandler &sbHandler = bVert ? m_sbVert : m_sbHorz;
    SCROLLINFO &si = bVert ? m_siVer : m_siHoz;
    CRect rcOldThumb = sbHandler.GetPartRect(SB_THUMBTRACK);
    si.nTrackPos = nPos;
    CRect rcThumb = sbHandler.GetPartRect(SB_THUMBTRACK);
    CRect rcUnion;
    rcUnion.UnionRect(rcOldThumb, rcThumb);
    InvalidateRect(&rcUnion);
    OnScroll(bVert, SB_THUMBTRACK, nPos);
}

void SPanel::OnScrollCommand(BOOL bVert, int iCmd, int nPos)
{
    SASSERT(iCmd != SB_THUMBTRACK);
    OnScroll(bVert, iCmd, nPos);
}

void SPanel::OnScrollSetTimer(BOOL bVert, char id, UINT uElapse)
{
    SetTimer(id, uElapse);
}

void SPanel::OnScrollKillTimer(BOOL bVert, char id)
{
    KillTimer(id);
}

const IInterpolator *SPanel::GetScrollInterpolator() const
{
    if (m_fadeFrames > 0 && !m_fadeInterpolator)
    {
        m_fadeInterpolator.Attach(CREATEINTERPOLATOR(L"Accelerate"));
    }
    return m_fadeInterpolator;
}

int SPanel::GetScrollFadeFrames() const
{
    return m_fadeFrames;
}

BYTE SPanel::GetScrollThumbTrackMinAlpha() const
{
    return m_bySbThumbTrackMinAlpha;
}

SPanel::SPanel()
    : m_dragSb(SSB_NULL)
    , m_hitSb(SSB_NULL)
    , m_wBarVisible(0)
    , m_wBarEnable(SSB_BOTH)
    , m_dwUpdateInterval(DEF_UPDATEINTERVAL)
    , m_nScrollSpeed(10)
    , m_zDelta(0)
    , m_sbVert(this, true)
    , m_sbHorz(this, false)
    , m_fadeFrames(0)
    , m_bySbThumbTrackMinAlpha(128)
{
    m_nSbWid.setInvalid();
    m_nSbArrowSize.setInvalid();

    ISkinObj *pSkin = GETBUILTINSKIN(SKIN_SYS_SCROLLBAR);
    if (pSkin && pSkin->IsClass(SSkinScrollbar::GetClassName()))
        m_pSkinSb = (SSkinScrollbar *)pSkin;

    memset(&m_siHoz, 0, sizeof(SCROLLINFO));
    memset(&m_siVer, 0, sizeof(SCROLLINFO));
    m_siHoz.nTrackPos = (DWORD)-1;
    m_siVer.nTrackPos = (DWORD)-1;
}

BOOL SPanel::ShowScrollBar(int wBar, BOOL bShow)
{
    if (bShow)
        m_wBarVisible |= wBar;
    else
        m_wBarVisible &= ~wBar;
    SSendMessage(WM_NCCALCSIZE);
    Invalidate();
    return TRUE;
}

BOOL SPanel::EnableScrollBar(int wBar, BOOL bEnable)
{
    if (bEnable)
        m_wBarEnable |= wBar;
    else
        m_wBarEnable &= ~wBar;
    if (wBar & SSB_VERT)
    {
        CRect rcSb = GetScrollBarRect(TRUE);
        InvalidateRect(rcSb);
    }
    if (wBar & SSB_HORZ)
    {
        CRect rcSb = GetScrollBarRect(FALSE);
        InvalidateRect(rcSb);
    }
    return TRUE;
}

BOOL SPanel::IsScrollBarEnable(BOOL bVertical) const
{
    return m_wBarEnable & (bVertical ? SSB_VERT : SSB_HORZ);
}

void SPanel::SetScrollInfo(SCROLLINFO si, BOOL bVertical)
{
    SCROLLINFO *psi = bVertical ? (&m_siVer) : (&m_siHoz);
    if (si.fMask & SIF_RANGE)
    {
        psi->nMin = si.nMin;
        psi->nMax = si.nMax;
    }
    if (si.fMask & SIF_PAGE)
        psi->nPage = si.nPage;
    if (si.fMask & SIF_POS)
        psi->nPos = si.nPos;
    if (si.fMask & SIF_TRACKPOS)
        psi->nTrackPos = si.nTrackPos;
    if (si.nPage == 0)
        psi->nPos = 0;
    else
    {
        if (psi->nPos > (psi->nMax - (int)psi->nPage + 1))
            psi->nPos = (psi->nMax - psi->nPage + 1);
        if (psi->nPos < psi->nMin)
            psi->nPos = psi->nMin;
    }
}

BOOL SPanel::SetScrollPos(BOOL bVertical, int nNewPos, BOOL bRedraw)
{
    SCROLLINFO *psi = bVertical ? (&m_siVer) : (&m_siHoz);

    // make sure nNewPos is valid.
    if (nNewPos < psi->nMin)
        nNewPos = psi->nMin;
    if (nNewPos > psi->nMax - (int)psi->nPage + 1)
        nNewPos = psi->nMax - psi->nPage + 1;

    LockUpdate();
    BOOL bRet = OnScroll(bVertical, SB_THUMBPOSITION, nNewPos);
    UnlockUpdate();
    if (bRet)
    {
        if (bRedraw)
        {
            CRect rcSb = GetScrollBarRect(!!bVertical);
            InvalidateRect(rcSb);
        }
        Invalidate();
    }
    return bRet;
}

int SPanel::GetScrollPos(BOOL bVertical) const
{
    const SCROLLINFO &si = bVertical ? m_siVer : m_siHoz;
    return si.nTrackPos != -1 ? si.nTrackPos : si.nPos;
}

BOOL SPanel::SetScrollRange(BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw)
{
    SCROLLINFO *psi = bVertical ? (&m_siVer) : (&m_siHoz);

    bRedraw = bRedraw || psi->nMin != nMinPos || psi->nMax != nMaxPos;
    psi->nMin = nMinPos;
    psi->nMax = nMaxPos;

    if (psi->nPos < psi->nMin)
        psi->nPos = psi->nMin;
    if (psi->nPos > psi->nMax - (int)psi->nPage + 1)
        psi->nPos = psi->nMax - psi->nPage + 1;

    if (bRedraw)
    {
        CRect rcSb = GetScrollBarRect(!!bVertical);
        InvalidateRect(rcSb);
    }
    return TRUE;
}

BOOL SPanel::GetScrollRange(BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) const
{
    const SCROLLINFO *psi = bVertical ? (&m_siVer) : (&m_siHoz);
    if (lpMaxPos)
        *lpMaxPos = psi->nMax;
    if (lpMinPos)
        *lpMinPos = psi->nMin;
    return TRUE;
}

BOOL SPanel::HasScrollBar(BOOL bVertical) const
{
    return m_wBarVisible & (bVertical ? SSB_VERT : SSB_HORZ);
}

int SPanel::OnCreate(LPVOID)
{
    int nRet = __baseCls::OnCreate(NULL);
    if (nRet != 0)
        return nRet;
    return m_pSkinSb ? 0 : -1;
}

void SPanel::OnDestroy()
{
    m_sbHorz.OnDestroy();
    m_sbVert.OnDestroy();
    SWindow::OnDestroy();
}

void SPanel::OnNcPaint(IRenderTarget *pRT)
{
    __baseCls::OnNcPaint(pRT);
    CRect rcDest;
    //绘制滚动条
    if (HasScrollBar(TRUE))
    {
        m_sbVert.OnDraw(pRT, SB_LINEUP);
        m_sbVert.OnDraw(pRT, SScrollBarHandler::kSbRail);
        m_sbVert.OnDraw(pRT, SB_THUMBTRACK);
        m_sbVert.OnDraw(pRT, SB_LINEDOWN);
    }
    if (HasScrollBar(FALSE))
    {
        m_sbHorz.OnDraw(pRT, SB_LINEUP);
        m_sbHorz.OnDraw(pRT, SScrollBarHandler::kSbRail);
        m_sbHorz.OnDraw(pRT, SB_THUMBTRACK);
        m_sbHorz.OnDraw(pRT, SB_LINEDOWN);
    }

    if (HasScrollBar(TRUE) && HasScrollBar(FALSE))
    {
        CRect rcDest;
        __baseCls::GetClientRect(&rcDest);
        rcDest.left = rcDest.right - GetSbWidth();
        rcDest.top = rcDest.bottom - GetSbWidth();
        m_pSkinSb->DrawByState(pRT, rcDest, SB_CORNOR);
    }
}

void SPanel::GetClientRect(LPRECT pRect) const
{
    *pRect = m_rcClient;
}

CRect SPanel::GetClientRect() const
{
    return m_rcClient;
}

BOOL SPanel::OnNcHitTest(CPoint pt)
{
    if (m_dragSb != SSB_NULL)
        return TRUE;
    return !m_rcClient.PtInRect(pt);
}

void SPanel::OnNcLButtonDown(UINT nFlags, CPoint point)
{
    SetCapture();
    if (m_sbVert.OnMouseDown(point))
        m_dragSb = SSB_VERT;
    else if (m_sbHorz.OnMouseDown(point))
        m_dragSb = SSB_HORZ;
    else
        m_dragSb = SSB_NULL;
}

void SPanel::OnNcLButtonUp(UINT nFlags, CPoint pt)
{
    if (m_dragSb != SSB_NULL)
    {
        SScrollBarHandler &sbHandler = m_dragSb == SSB_VERT ? m_sbVert : m_sbHorz;
        sbHandler.OnMouseUp(pt);
        m_dragSb = SSB_NULL;
        OnNcMouseMove(nFlags, pt);
    }
    ReleaseCapture();
}

void SPanel::OnNcMouseMove(UINT nFlags, CPoint point)
{
    if (m_dragSb != SSB_NULL)
    {
        SScrollBarHandler &sbHandler = m_dragSb == SSB_VERT ? m_sbVert : m_sbHorz;
        CRect rcSb = GetScrollBarRect(m_dragSb == SSB_VERT);
        BOOL bInSbNew = rcSb.PtInRect(point);
        BOOL bInSbOld = sbHandler.GetHitPart() != -1;
        if (bInSbNew != bInSbOld)
        {
            if (bInSbOld)
                sbHandler.OnMouseLeave();
            else
                sbHandler.OnMouseHover(point);
        }
        sbHandler.OnMouseMove(point);

        m_hitSb = bInSbNew ? m_dragSb : SSB_NULL;
    }
    else
    {
        if (m_sbVert.HitTest(point) != -1)
        {
            if (m_hitSb == SSB_NULL)
                m_sbVert.OnMouseHover(point);
            else
                m_sbVert.OnMouseMove(point);
            m_hitSb = SSB_VERT;
        }
        else if (m_sbHorz.HitTest(point) != -1)
        {
            if (m_hitSb == SSB_NULL)
                m_sbHorz.OnMouseHover(point);
            else
                m_sbHorz.OnMouseMove(point);
            m_hitSb = SSB_HORZ;
        }
        else
        {
            if (m_hitSb != SSB_NULL)
            {
                SScrollBarHandler &sbHandler = m_hitSb == SSB_VERT ? m_sbVert : m_sbHorz;
                sbHandler.OnMouseLeave();
            }
            m_hitSb = SSB_NULL;
        }
    }
}

void SPanel::OnNcMouseLeave()
{
    if (m_dragSb != SSB_NULL)
    {
        if (m_hitSb == m_dragSb)
        {
            SScrollBarHandler &sbHandler = m_dragSb == SSB_VERT ? m_sbVert : m_sbHorz;
            sbHandler.OnMouseLeave();
            m_hitSb = SSB_NULL;
        }
    }
    else if (m_hitSb != SSB_NULL)
    {
        SScrollBarHandler &sbHandler = m_hitSb == SSB_VERT ? m_sbVert : m_sbHorz;
        sbHandler.OnMouseLeave();
        m_hitSb = SSB_NULL;
    }
}

//滚动条显示或者隐藏时发送该消息
LRESULT SPanel::OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
{
    SWindow::GetClientRect(&m_rcClient);

    if (HasScrollBar(TRUE))
        m_rcClient.right -= GetSbWidth();
    if (HasScrollBar(FALSE))
        m_rcClient.bottom -= GetSbWidth();
    InvalidateRect(NULL);
    return 0;
}

BOOL SPanel::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    BOOL bVertScroll = HasScrollBar(TRUE);

    m_zDelta += zDelta;
    short delta = m_zDelta / WHEEL_DELTA;
    m_zDelta %= WHEEL_DELTA;

    short nLines = delta > 0 ? delta : (-delta);
    short i = 0;
    for (; i < nLines; i++)
    {
        // 返回FALSE代表无法再再进行滚动,可以透传给父窗口
        if (!OnScroll(bVertScroll, delta > 0 ? SB_LINEUP : SB_LINEDOWN, 0))
        {
            break;
        }
        ScrollUpdate();
    }
    if (i < nLines)
    { // 返回FALSE代表无法再再进行滚动,可以透传给父窗口
        delta = (nLines - i) * WHEEL_DELTA * (delta > 0 ? 1 : -1);
        SWindow::OnMouseWheel(nFlags, delta, pt);
    }
    if (nLines != 0)
    {
        HWND hHost = GetContainer()->GetHostHwnd();
        ::SendMessage(hHost, WM_MOUSEMOVE, nFlags, MAKELPARAM(pt.x, pt.y));
        ::SendMessage(hHost, WM_SETCURSOR, WPARAM(hHost), MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
    }
    return FALSE;
}

BOOL SPanel::OnScroll(BOOL bVertical, UINT uCode, int nPos)
{
    SCROLLINFO *psi = bVertical ? (&m_siVer) : (&m_siHoz);
    int nNewPos = psi->nPos;
    switch (uCode)
    {
    case SB_LINEUP:
        nNewPos -= GetScrollLineSize(bVertical);
        break;
    case SB_LINEDOWN:
        nNewPos += GetScrollLineSize(bVertical);
        break;
    case SB_PAGEUP:
        nNewPos -= psi->nPage;
        break;
    case SB_PAGEDOWN:
        nNewPos += psi->nPage;
        break;
    case SB_THUMBTRACK:
        nNewPos = nPos;
        break;
    case SB_THUMBPOSITION:
        nNewPos = nPos;
        psi->nTrackPos = -1;
        break;
    case SB_TOP:
        nNewPos = psi->nMin;
        break;
    case SB_BOTTOM:
        nNewPos = psi->nMax - psi->nPage + 1;
        break;
    }

    if (nNewPos < psi->nMin)
        nNewPos = psi->nMin;
    if (nNewPos > psi->nMax - (int)psi->nPage + 1)
        nNewPos = psi->nMax - psi->nPage + 1;
    if (psi->nPage == 0)
        nNewPos = 0;

    if (nNewPos == psi->nPos)
        return FALSE;

    psi->nPos = nNewPos;
    if (IsVisible(TRUE) && HasScrollBar(bVertical) && uCode != SB_THUMBTRACK)
    {
        OnScrollUpdatePart(!!bVertical, SB_THUMBTRACK);
    }
    Invalidate();
    return TRUE;
}

void SPanel::OnTimer(char cTimerID)
{
    if (cTimerID == IScrollBarHost::Timer_Go || cTimerID == IScrollBarHost::Timer_Wait)
    {
        SASSERT(m_dragSb != SSB_NULL);
        SScrollBarHandler &sbHandler = m_dragSb == SSB_VERT ? m_sbVert : m_sbHorz;
        sbHandler.OnTimer(cTimerID);
    }
}

void SPanel::ScrollUpdate()
{
    DWORD dwTime = GetTickCount();
    if (dwTime - m_dwUpdateTime >= m_dwUpdateInterval)
    {
        Update();
        m_dwUpdateTime = dwTime;
    }
}

void SPanel::OnShowWindow(BOOL bShow, UINT nStatus)
{
    SWindow::OnShowWindow(bShow, nStatus);
    if (!IsVisible(TRUE) && m_dragSb != SSB_NULL)
    { //隐藏窗口时正有可能正在拖动滚动条，需要处理一下。
        OnNcLButtonUp(0, CPoint(-1, -1));
    }
}

void SPanel::OnEnable(BOOL bEnable, UINT uStatus)
{
    SWindow::OnEnable(bEnable, uStatus);
    if (IsDisabled(TRUE) && m_dragSb != SSB_NULL)
    {
        OnNcLButtonUp(0, CPoint(-1, -1));
    }
}

HRESULT SPanel::OnAttrScrollbarSkin(SStringW strValue, BOOL bLoading)
{
    ISkinObj *pSbSkin = GETSKIN(strValue, GetScale());
    if (!pSbSkin || !pSbSkin->IsClass(SSkinScrollbar::GetClassName()))
        return E_FAIL;
    m_pSkinSb = (SSkinScrollbar *)pSbSkin;
    return bLoading ? S_FALSE : S_OK;
}

int SPanel::GetScrollLineSize(BOOL bVertical)
{
    return m_nScrollSpeed;
}

void SPanel::OnVScroll(UINT nSBCode, UINT nPos, HWND)
{
    OnScroll(TRUE, nSBCode, nPos);
}

void SPanel::OnHScroll(UINT nSBCode, UINT nPos, HWND)
{
    OnScroll(FALSE, nSBCode, nPos);
}

void SPanel::OnColorize(COLORREF cr)
{
    __baseCls::OnColorize(cr);
    if (m_pSkinSb)
        m_pSkinSb->OnColorize(cr);
}

void SPanel::OnScaleChanged(int nScale)
{
    __baseCls::OnScaleChanged(nScale);
    if (m_pSkinSb)
    {
        SAutoRefPtr<ISkinObj> pSkin = (ISkinObj *)m_pSkinSb;
        GetScaleSkin(pSkin, nScale);
        m_pSkinSb = (SSkinScrollbar *)(ISkinObj *)pSkin;
        SSendMessage(WM_NCCALCSIZE);
    }
}

int SPanel::GetSbWidth() const
{
    if (m_nSbWid.isValid())
        return m_nSbWid.toPixelSize(GetScale());
    SASSERT(m_pSkinSb);
    return m_pSkinSb->GetIdealSize();
}

void SPanel::OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer)
{
    m_sbHorz.OnContainerChanged(pOldContainer, pNewContainer);
    m_sbVert.OnContainerChanged(pOldContainer, pNewContainer);
    SWindow::OnContainerChanged(pOldContainer, pNewContainer);
}

//////////////////////////////////////////////////////////////////////////
SScrollView::SScrollView()
{
    m_ptOrigin = CPoint();
    m_szView = CSize();
    m_bClipClient = TRUE;
    m_viewSize[0].setInvalid();
    m_viewSize[1].setInvalid();

    GetEventSet()->addEvent(EVENTID(EventScrollViewOriginChanged));
    GetEventSet()->addEvent(EVENTID(EventScrollViewSizeChanged));
    GetEventSet()->addEvent(EVENTID(EventScroll));
}

void SScrollView::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);
    UpdateScrollBar();
}

void SScrollView::OnViewOriginChanged(CPoint ptOld, CPoint ptNew)
{
    UpdateChildrenPosition();
    EventScrollViewOriginChanged evt(this);
    evt.ptOldOrigin = ptOld;
    evt.ptNewOrigin = ptNew;
    FireEvent(evt);
}

void SScrollView::OnViewSizeChanged(CSize szOld, CSize szNew)
{
    EventScrollViewSizeChanged evt(this);
    evt.szOldViewSize = szOld;
    evt.szNewViewSize = szNew;
    FireEvent(evt);
}

void SScrollView::SetViewOrigin(POINT pt)
{
    if (pt.x < 0)
        pt.x = 0;
    if (pt.y < 0)
        pt.y = 0;
    if (pt.x > (int)(m_siHoz.nMax - m_siHoz.nPage + 1))
        pt.x = (int)(m_siHoz.nMax - m_siHoz.nPage + 1);
    if (pt.y > (int)(m_siVer.nMax - m_siVer.nPage + 1))
        pt.y = (int)(m_siVer.nMax - m_siVer.nPage + 1);
    if (m_ptOrigin.x == pt.x && m_ptOrigin.y == pt.y)
        return;
    POINT ptOld = m_ptOrigin;
    m_ptOrigin = pt;
    if (GetScrollPos(FALSE) != pt.x)
        SetScrollPos(FALSE, m_ptOrigin.x, TRUE);
    if (GetScrollPos(TRUE) != pt.y)
        SetScrollPos(TRUE, m_ptOrigin.y, TRUE);

    m_layoutDirty = dirty_self;

    OnViewOriginChanged(ptOld, pt);

    Invalidate();
}

void SScrollView::GetViewOrigin(POINT *ptOri) const
{
    SASSERT(ptOri);
    *ptOri = m_ptOrigin;
}

void SScrollView::SetViewSize(SIZE szView)
{
    if (szView.cx == m_szView.cx && szView.cy == m_szView.cy)
        return;

    CSize oldViewSize = m_szView;
    m_szView = szView;
    UpdateScrollBar();
    OnViewSizeChanged(oldViewSize, szView);
}

void SScrollView::GetViewSize(SIZE *szView) const
{
    SASSERT(szView);
    *szView = m_szView;
}

void SScrollView::UpdateScrollBar()
{
    CRect rcClient;
    SWindow::GetClientRect(&rcClient);

    CSize size = rcClient.Size();
    m_wBarVisible = SSB_NULL;     //关闭滚动条
    CPoint ptOrigin = m_ptOrigin; // backup

    if (size.cy < m_szView.cy || (size.cy < m_szView.cy + GetSbWidth() && size.cx < m_szView.cx))
    {
        //需要纵向滚动条
        m_wBarVisible |= SSB_VERT;
        m_siVer.nMin = 0;
        m_siVer.nMax = m_szView.cy - 1;
        m_siVer.nPage = size.cy;
        if (m_siVer.nPos + (int)m_siVer.nPage > m_siVer.nMax)
        {
            m_siVer.nPos = m_siVer.nMax - m_siVer.nPage;
            m_ptOrigin.y = m_siVer.nPos;
        }
        if (size.cx < m_szView.cx + GetSbWidth())
        {
            //需要横向滚动条
            m_wBarVisible |= SSB_HORZ;
            m_siVer.nPage = size.cy - GetSbWidth() > 0 ? size.cy - GetSbWidth() : 0;

            m_siHoz.nMin = 0;
            m_siHoz.nMax = m_szView.cx - 1;
            m_siHoz.nPage = size.cx - GetSbWidth() > 0 ? size.cx - GetSbWidth() : 0;
            if (m_siHoz.nPos + (int)m_siHoz.nPage > m_siHoz.nMax)
            {
                m_siHoz.nPos = m_siHoz.nMax - m_siHoz.nPage;
                m_ptOrigin.x = m_siHoz.nPos;
            }
        }
        else
        {
            //不需要横向滚动条
            m_siHoz.nPage = size.cx;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = m_siHoz.nPage - 1;
            m_siHoz.nPos = 0;
            m_ptOrigin.x = 0;
        }
    }
    else
    {
        //不需要纵向滚动条
        m_siVer.nPage = size.cy;
        m_siVer.nMin = 0;
        m_siVer.nMax = size.cy - 1;
        m_siVer.nPos = 0;
        m_ptOrigin.y = 0;

        if (size.cx < m_szView.cx)
        {
            //需要横向滚动条
            m_wBarVisible |= SSB_HORZ;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = m_szView.cx - 1;
            m_siHoz.nPage = size.cx;
            if (m_siHoz.nPos + (int)m_siHoz.nPage > m_siHoz.nMax)
            {
                m_siHoz.nPos = m_siHoz.nMax - m_siHoz.nPage;
                m_ptOrigin.x = m_siHoz.nPos;
            }
        }
        //不需要横向滚动条
        else
        {
            m_siHoz.nPage = size.cx;
            m_siHoz.nMin = 0;
            m_siHoz.nMax = m_siHoz.nPage - 1;
            m_siHoz.nPos = 0;
            m_ptOrigin.x = 0;
        }
    }

    SetScrollPos(TRUE, m_siVer.nPos, TRUE);
    SetScrollPos(FALSE, m_siHoz.nPos, TRUE);

    SSendMessage(WM_NCCALCSIZE);

    if (m_ptOrigin.x != ptOrigin.x || m_ptOrigin.y != ptOrigin.y)
    {
        m_layoutDirty = dirty_self;
        OnViewOriginChanged(ptOrigin, m_ptOrigin);
    }
    Invalidate();
}

BOOL SScrollView::OnScroll(BOOL bVertical, UINT uCode, int nPos)
{
    BOOL bRet = __baseCls::OnScroll(bVertical, uCode, nPos);
    if (bRet)
    {
        int nPos = GetScrollPos(bVertical);
        CPoint ptOrigin = m_ptOrigin;

        if (bVertical)
            ptOrigin.y = nPos;
        else
            ptOrigin.x = nPos;

        if (ptOrigin != m_ptOrigin)
            SetViewOrigin(ptOrigin);

        if (uCode == SB_THUMBTRACK)
            ScrollUpdate();

        EventScroll evt(this);
        evt.bVertical = bVertical;
        evt.nSbCode = uCode;
        evt.nPos = nPos;
        FireEvent(evt);
    }
    return bRet;
}

void SScrollView::UpdateViewSize()
{
    if (!(m_viewSize[0].isValid() && m_viewSize[1].isValid()))
        return;
    CRect rcWnd = SWindow::GetClientRect();
    CRect rcPadding = GetStyle().GetPadding();
    rcWnd.DeflateRect(rcPadding);

    CSize szView;
    if (m_viewSize[0].isMatchParent())
        szView.cx = rcWnd.Width();
    else if (m_viewSize[0].isWrapContent())
        szView.cx = -1;
    else
        szView.cx = m_viewSize[0].toPixelSize(GetScale());

    if (m_viewSize[1].isMatchParent())
        szView.cy = rcWnd.Height();
    else if (m_viewSize[1].isWrapContent())
        szView.cy = -1;
    else
        szView.cy = m_viewSize[1].toPixelSize(GetScale());

    if (m_viewSize[0].isWrapContent() || m_viewSize[1].isWrapContent())
    {
        CSize szCalc = GetLayout()->MeasureChildren(this, szView.cx, szView.cy);
        if (m_viewSize[0].isWrapContent())
            szView.cx = szCalc.cx;
        if (m_viewSize[1].isWrapContent())
            szView.cy = szCalc.cy;
    }

    if (szView.cy > rcWnd.Height() && m_viewSize[0].isMatchParent())
    {
        szView.cx -= GetSbWidth();
        if (m_viewSize[1].isWrapContent())
        {
            // recalc height
            CSize szCalc = GetLayout()->MeasureChildren(this, szView.cx, -1);
            szView.cy = szCalc.cy;
        }
    }
    else if (szView.cx > rcWnd.Width() && m_viewSize[1].isMatchParent())
    {
        szView.cy -= GetSbWidth();
        if (m_viewSize[0].isWrapContent())
        {
            // recalc width
            CSize szCalc = GetLayout()->MeasureChildren(this, -1, szView.cy);
            szView.cx = szCalc.cx;
        }
    }
    if (!m_viewSize[0].isSpecifiedSize())
        szView.cx += rcPadding.left + rcPadding.right;
    if (!m_viewSize[1].isSpecifiedSize())
        szView.cy += rcPadding.top + rcPadding.bottom;

    SetViewSize(szView);
}

HRESULT SScrollView::OnAttrViewSize(const SStringW &strValue, BOOL bLoading)
{
    SStringWList values;
    if (SplitString(strValue, L',', values) != 2)
        return E_INVALIDARG;
    m_viewSize[0] = GETLAYOUTSIZE(values[0]);
    m_viewSize[1] = GETLAYOUTSIZE(values[1]);

    if (!bLoading)
    {
        UpdateViewSize();
    }
    return S_FALSE;
}

void SScrollView::GetChildrenLayoutRect(RECT *prc) const
{
    CRect rcRet;
    __baseCls::GetChildrenLayoutRect(&rcRet);
    CRect rcPadding = GetStyle().GetPadding();
    rcRet.OffsetRect(-m_ptOrigin.x, -m_ptOrigin.y);
    rcRet.right = rcRet.left + m_szView.cx - rcPadding.left - rcPadding.right;
    rcRet.bottom = rcRet.top + m_szView.cy - rcPadding.top - rcPadding.bottom;
    *prc = rcRet;
}

void SScrollView::UpdateChildrenPosition()
{
    UpdateViewSize();
    __baseCls::UpdateChildrenPosition();
}

SNSEND