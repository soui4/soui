//////////////////////////////////////////////////////////////////////////
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
    , m_bDragPending(FALSE)
    , m_bDragScrolling(FALSE)
    , m_bDragStarted(FALSE)
    , m_bItemDragScrollEnabled(TRUE)
    , m_fLastVelocityX(0.0f)
    , m_fLastVelocityY(0.0f)
    , m_nLastMoveTime(0)
    , m_fFlingVStartPos(0.0f)
    , m_fFlingVTargetPos(0.0f)
    , m_fFlingHStartPos(0.0f)
    , m_fFlingHTargetPos(0.0f)
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

BOOL SPanel::ScrollToPos(BOOL bVertical, int nPos)
{
    return OnScroll(bVertical, SB_THUMBTRACK, nPos);
}

void SPanel::NotifyScrollBarActivity(BOOL bVertical)
{
    if (bVertical)
        m_sbVert.OnMouseWheel();
    else
        m_sbHorz.OnMouseWheel();
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
    ClearDragState();
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

UINT SPanel::OnNcHitTest(const CPoint &pt)
{
    if (m_dragSb == SSB_VERT)
        return HTVSCROLL;
    else if (m_dragSb == SSB_HORZ)
        return HTHSCROLL;
    else
    {
        if (HasScrollBar(TRUE) && GetScrollBarRect(TRUE).PtInRect(pt))
            return HTVSCROLL;
        else if (HasScrollBar(FALSE) && GetScrollBarRect(FALSE).PtInRect(pt))
            return HTHSCROLL;
        else
            return __baseCls::OnNcHitTest(pt);
    }
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
        // Show scrollbar with fade-in and auto-hide on wheel scroll
        if (bVertScroll)
            m_sbVert.OnMouseWheel();
        else if (HasScrollBar(FALSE))
            m_sbHorz.OnMouseWheel();

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
    else if (cTimerID == IScrollBarHost::Timer_WheelHide)
    {
        // Route to both handlers; each decides whether to fade out
        m_sbVert.OnTimer(cTimerID);
        m_sbHorz.OnTimer(cTimerID);
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

void SPanel::ClearDragState(){
    if (m_dragSb != SSB_NULL)
    {
        OnNcLButtonUp(0, CPoint(-1, -1));
    }
    StopFlingAnimation();
    if (m_bDragPending || m_bDragScrolling)
    {
        m_bDragPending = FALSE;
        m_bDragScrolling = FALSE;
        m_bDragStarted = FALSE;
        ReleaseCapture();
    }
}

void SPanel::OnShowWindow(BOOL bShow, UINT nStatus)
{
    SWindow::OnShowWindow(bShow, nStatus);
    if (!IsVisible(TRUE))
    {
        ClearDragState();
    }
}

void SPanel::OnEnable(BOOL bEnable, UINT uStatus)
{
    SWindow::OnEnable(bEnable, uStatus);
    if (IsDisabled(TRUE))
    {
        ClearDragState();
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

// === Drag scroll implementation ===

void SPanel::StartDragPending(const CPoint &pt)
{
    m_bDragPending = TRUE;
    m_ptDragStart = pt;
    m_bDragStarted = FALSE;
    SetCapture();
}

BOOL SPanel::HandleMouseDrag(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lRet)
{
    CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

    // === Drag scrolling active: consume left button events ===
    if (m_bDragScrolling)
    {
        if (uMsg == WM_MOUSEMOVE)
        {
            int dx = 0, dy = 0;
            if (m_ptDragLast == CPoint(0, 0))
            {
                m_ptDragLast = pt;
                m_nLastMoveTime = 0;
                m_fLastVelocityX = 0.0f;
                m_fLastVelocityY = 0.0f;
            }
            else
            {
                dx = pt.x - m_ptDragLast.x;
                dy = pt.y - m_ptDragLast.y;
                m_ptDragLast = pt;

                if (m_nLastMoveTime > 0)
                {
                    uint64_t now = GetTickCount64();
                    uint64_t dt = now - m_nLastMoveTime;
                    if (dt > 0)
                    {
                        m_fLastVelocityX = (float)dx / (float)dt;
                        m_fLastVelocityY = (float)dy / (float)dt;
                    }
                }
                m_nLastMoveTime = GetTickCount64();
            }
            if (dy != 0)
                ScrollByDrag(TRUE, dy, m_fLastVelocityY);
            if (dx != 0)
                ScrollByDrag(FALSE, dx, m_fLastVelocityX);
        }
        else if (uMsg == WM_LBUTTONUP)
        {
            m_bDragScrolling = FALSE;
            m_ptDragLast = CPoint(0, 0);

            float fReleaseVX = 0.0f, fReleaseVY = 0.0f;
            if (m_nLastMoveTime > 0)
            {
                uint64_t now = GetTickCount64();
                if (now - m_nLastMoveTime < 100)
                {
                    fReleaseVX = m_fLastVelocityX;
                    fReleaseVY = m_fLastVelocityY;
                }
            }

            OnDragScrollEnd();
            ReleaseCapture();

            if (fabs(fReleaseVX) > 0.01f || fabs(fReleaseVY) > 0.01f)
                StartFlingAnimation(fReleaseVX, fReleaseVY);
            m_nLastMoveTime = 0;
            return TRUE;
        }

        // 拖动滚动中仅消费左键拖动相关消息
        if (uMsg == WM_MOUSEMOVE || uMsg == WM_LBUTTONUP)
            return TRUE;
        return FALSE;
    }

    // === Fling animation running: consume left button events to stop fling ===
    BOOL bFlingRunning = (m_pFlingAnimatorV && m_pFlingAnimatorV->isRunning())
                         || (m_pFlingAnimatorH && m_pFlingAnimatorH->isRunning());
    if (bFlingRunning)
    {
        if (uMsg == WM_LBUTTONDOWN)
        {
            StopFlingAnimation();
        }
        else if (uMsg == WM_MOUSEMOVE)
        {
            if (wParam & MK_LBUTTON)
            {
                StopFlingAnimation();
                m_bDragPending = FALSE;
                m_bDragScrolling = TRUE;
                m_ptDragLast = pt;
                m_ptDragStart = pt;
                m_bDragStarted = TRUE;
                m_nLastMoveTime = 0;
                m_fLastVelocityX = 0.0f;
                m_fLastVelocityY = 0.0f;
                OnDragScrollStart();
            }
        }

        // fling运行中仅消费左键相关消息
        if (uMsg == WM_LBUTTONDOWN || uMsg == WM_MOUSEMOVE)
            return TRUE;
        return FALSE;
    }

    // === Waiting for drag threshold ===
    if (m_bDragPending)
    {
        if (!m_bDragStarted)
        {
            m_ptDragStart = pt;
            m_bDragStarted = TRUE;
        }

        if (uMsg == WM_MOUSEMOVE)
        {
            int dx = abs(pt.x - m_ptDragStart.x);
            int dy = abs(pt.y - m_ptDragStart.y);
            if (dx > 8 || dy > 8)
            {
                if (OnDragCancelCapture(CANCEL_REASON_SCROLL))
                {
                    m_bDragPending = FALSE;
                    m_bDragScrolling = TRUE;
                    m_ptDragLast = pt;
                    m_nLastMoveTime = 0;
                    m_fLastVelocityX = 0.0f;
                    m_fLastVelocityY = 0.0f;
                    SetCapture();
                    OnDragScrollStart();
                    return TRUE;
                }
                else
                {
                    m_bDragPending = FALSE;
                    m_bDragStarted = FALSE;
                    return FALSE;
                }
            }
        }
        else if (uMsg == WM_LBUTTONUP)
        {
            m_bDragPending = FALSE;
            m_bDragStarted = FALSE;
            ReleaseCapture();
        }
    }

    return FALSE;
}

void SPanel::ScrollByDrag(BOOL bVert, int delta, float fVelocity)
{
    float fMultiplier = 1.0f;
    float fAbsV = (float)fabs(fVelocity);

    if (fAbsV > 5.0f)
        fMultiplier = 2.5f;
    else if (fAbsV > 3.0f)
        fMultiplier = 1.8f;
    else if (fAbsV > 1.5f)
        fMultiplier = 1.2f;
    else if (fAbsV < 0.3f && fAbsV > 0.0f)
        fMultiplier = 0.7f;

    OnDragScroll(bVert, (int)(delta * fMultiplier));
}

void SPanel::OnDragScrollStart()
{
    if (HasScrollBar(TRUE))
        NotifyScrollBarActivity(TRUE);
    if (HasScrollBar(FALSE))
        NotifyScrollBarActivity(FALSE);
}

void SPanel::OnDragScroll(BOOL bVert, int delta)
{
    if (HasScrollBar(bVert))
    {
        int nNewPos = GetScrollPos(bVert) - delta;
        ScrollToPos(bVert, nNewPos);
        OnScrollUpdatePart(bVert, -1);
        NotifyScrollBarActivity(bVert);
    }
}

void SPanel::OnDragScrollEnd()
{
}

BOOL SPanel::CancelCaptureMode(int reason)
{
    if (reason == CANCEL_REASON_SCROLL && (HasScrollBar(TRUE) || HasScrollBar(FALSE)))
        return FALSE;
    return __baseCls::CancelCaptureMode(reason);
}

void SPanel::OnFlingScroll()
{
    if (HasScrollBar(TRUE))
        NotifyScrollBarActivity(TRUE);
    if (HasScrollBar(FALSE))
        NotifyScrollBarActivity(FALSE);
}

BOOL SPanel::OnDragCancelCapture(int reason)
{
    return TRUE;
}

void SPanel::OnDragClearItemCapture()
{
}

void SPanel::StartFlingAnimation(float fVelocityX, float fVelocityY)
{
    StopFlingAnimation();

    int nVMin = 0, nVMax = 0, nHMin = 0, nHMax = 0;
    int nCurV = GetScrollPos(TRUE);
    int nCurH = GetScrollPos(FALSE);
    GetScrollRange(TRUE, &nVMin, &nVMax);
    GetScrollRange(FALSE, &nHMin, &nHMax);

    const float kDecelFactor = 2.0f;
    const long kMinDuration = 150;
    const long kMaxDuration = 500;

    if (fabs(fVelocityY) > 0.01f)
    {
        long nDuration = (long)(fabs(fVelocityY) * 200.0f);
        if (nDuration < kMinDuration) nDuration = kMinDuration;
        if (nDuration > kMaxDuration) nDuration = kMaxDuration;

        float fDistance = fVelocityY * (float)nDuration / kDecelFactor;
        m_fFlingVStartPos = (float)nCurV;
        m_fFlingVTargetPos = m_fFlingVStartPos - fDistance;

        if (m_fFlingVTargetPos < (float)nVMin) m_fFlingVTargetPos = (float)nVMin;
        if (m_fFlingVTargetPos > (float)nVMax) m_fFlingVTargetPos = (float)nVMax;

        if (fabs(m_fFlingVTargetPos - m_fFlingVStartPos) >= 1.0f)
        {
            SFloatAnimator *pAni = new SFloatAnimator();
            pAni->setRange(m_fFlingVStartPos, m_fFlingVTargetPos);
            pAni->setDuration(nDuration);
            pAni->setInterpolator(new SDecelerateInterpolator(kDecelFactor));
            pAni->addUpdateListener(this);
            pAni->addListener(this);
            m_pFlingAnimatorV.Attach(pAni);
            m_pFlingAnimatorV->start(GetTimelineHandlersMgr());
        }
    }

    if (fabs(fVelocityX) > 0.01f)
    {
        long nDuration = (long)(fabs(fVelocityX) * 200.0f);
        if (nDuration < kMinDuration) nDuration = kMinDuration;
        if (nDuration > kMaxDuration) nDuration = kMaxDuration;

        float fDistance = fVelocityX * (float)nDuration / kDecelFactor;
        m_fFlingHStartPos = (float)nCurH;
        m_fFlingHTargetPos = m_fFlingHStartPos - fDistance;

        if (m_fFlingHTargetPos < (float)nHMin) m_fFlingHTargetPos = (float)nHMin;
        if (m_fFlingHTargetPos > (float)nHMax) m_fFlingHTargetPos = (float)nHMax;

        if (fabs(m_fFlingHTargetPos - m_fFlingHStartPos) >= 1.0f)
        {
            SFloatAnimator *pAni = new SFloatAnimator();
            pAni->setRange(m_fFlingHStartPos, m_fFlingHTargetPos);
            pAni->setDuration(nDuration);
            pAni->setInterpolator(new SDecelerateInterpolator(kDecelFactor));
            pAni->addUpdateListener(this);
            pAni->addListener(this);
            m_pFlingAnimatorH.Attach(pAni);
            m_pFlingAnimatorH->start(GetTimelineHandlersMgr());
        }
    }

    OnFlingScroll();
}

void SPanel::StopFlingAnimation()
{
    if (m_pFlingAnimatorV && m_pFlingAnimatorV->isRunning())
        m_pFlingAnimatorV->end();
    m_pFlingAnimatorV = NULL;

    if (m_pFlingAnimatorH && m_pFlingAnimatorH->isRunning())
        m_pFlingAnimatorH->end();
    m_pFlingAnimatorH = NULL;
}

void SPanel::onAnimationUpdate(IValueAnimator *pAnimator)
{
    if (pAnimator == m_pFlingAnimatorV)
    {
        ScrollToPos(TRUE, (int)m_pFlingAnimatorV->getValue());
        if (HasScrollBar(TRUE))
            OnScrollUpdatePart(TRUE, -1);
        OnFlingScroll();
    }
    else if (pAnimator == m_pFlingAnimatorH)
    {
        ScrollToPos(FALSE, (int)m_pFlingAnimatorH->getValue());
        if (HasScrollBar(FALSE))
            OnScrollUpdatePart(FALSE, -1);
        OnFlingScroll();
    }
}

void SPanel::onAnimationEnd(IValueAnimator *pAnimator)
{
    if (pAnimator == m_pFlingAnimatorV)
        m_pFlingAnimatorV = NULL;
    else if (pAnimator == m_pFlingAnimatorH)
        m_pFlingAnimatorH = NULL;
}

BOOL SPanel::IsEnableDragMode() const {
	return  m_bItemDragScrollEnabled && (HasScrollBar(TRUE) || HasScrollBar(FALSE));
}

void SPanel::OnLButtonDown(UINT nFlags, CPoint pt)
{
    LRESULT lRet = 0;
    if (HandleMouseDrag(WM_LBUTTONDOWN, nFlags, MAKELPARAM(pt.x, pt.y), lRet))
    {
        return;
    }

    if (IsEnableDragMode())
    {
        StartDragPending(pt);
        return;
    }

    OnLButtonDownEx(nFlags, pt);
}

void SPanel::OnMouseMove(UINT nFlags, CPoint pt)
{
    LRESULT lRet = 0;
    if (HandleMouseDrag(WM_MOUSEMOVE, nFlags, MAKELPARAM(pt.x, pt.y), lRet))
    {
        return;
    }

    OnMouseMoveEx(nFlags, pt);
}

void SPanel::OnLButtonUp(UINT nFlags, CPoint pt)
{
    LRESULT lRet = 0;
    if (HandleMouseDrag(WM_LBUTTONUP, nFlags, MAKELPARAM(pt.x, pt.y), lRet))
    {
        return;
    }

    OnLButtonUpEx(nFlags, pt);
}

void SPanel::OnLButtonDownEx(UINT nFlags, CPoint pt)
{
    __baseCls::OnLButtonDown(nFlags, pt);
}

void SPanel::OnMouseMoveEx(UINT nFlags, CPoint pt)
{
    __baseCls::OnMouseMove(nFlags, pt);
}

void SPanel::OnLButtonUpEx(UINT nFlags, CPoint pt)
{
    __baseCls::OnLButtonUp(nFlags, pt);
}

SNSEND