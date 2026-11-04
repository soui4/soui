#include <souistd.h>
#include <core/SRootWindow.h>
#include <core/SHostWnd.h>
#include <layout/SAnchorLayout.h>

SNSBEGIN

///////////////////////////////////////////////////////////////////////
/** SRootWindow */
///////////////////////////////////////////////////////////////////////

/** How long the focus frame stays on the target window before it starts to fade away, in milliseconds. */
static const long kFocusAniHoldTime = 2000;

SRootWindow::SRootWindow()
    : m_pHostWnd(NULL)
    , m_hFocus(0)
{
    GetEventSet()->addEvent(EVENTID(EventInit));
    GetEventSet()->addEvent(EVENTID(EventExit));
    GetEventSet()->addEvent(EVENTID(EventMenuCmd));
    GetEventSet()->addEvent(EVENTID(EventUpdateCmdUI));
    GetEventSet()->addEvent(EVENTID(EventUpdateCmdTip));

    m_focusAnimator.Attach(new SRectAnimator());
    m_focusAnimator->setDuration(200);
    m_focusAnimator->addUpdateListener(this);
    m_focusAnimator->addListener(this);

    m_focusAlphaAnimator.Attach(new SByteAnimator());
    m_focusAlphaAnimator->setStart(255);
    m_focusAlphaAnimator->setEnd(0);
    m_focusAlphaAnimator->setDuration(kFocusAniHoldTime);
    m_focusAlphaAnimator->addUpdateListener(this);
    m_focusAlphaAnimator->addListener(this);
}

SRootWindow::~SRootWindow()
{
    // Both animators are registered in the container's timeline once started: detach them before the
    // root goes away, otherwise the timeline would keep pointers to destroyed animators.
    m_focusAnimator->removeUpdateListener(this);
    m_focusAnimator->removeListener(this);
    m_focusAnimator->end();

    m_focusAlphaAnimator->removeUpdateListener(this);
    m_focusAlphaAnimator->removeListener(this);
    m_focusAlphaAnimator->end();
}

void SRootWindow::OnAnimationInvalidate(bool bErase)
{
    if (bErase)
    {
        CRect rcWnd = GetClientRect();
        m_pHostWnd->m_memRT->BeginDraw();
        m_pHostWnd->m_memRT->ClearRect(rcWnd, 0);
        m_pHostWnd->m_memRT->EndDraw();
    }
    SWindow::OnAnimationInvalidate(bErase);
    if (!bErase)
        Update();
}

void SRootWindow::onAnimationUpdate(IValueAnimator *pAnimator)
{
    SRectAnimator *pRectAnimator = m_focusAnimator;
    SByteAnimator *pAlphaAnimator = m_focusAlphaAnimator;
    if (pAnimator == pRectAnimator)
    {
        // Repaint where the frame is now plus where it was painted last, so the frame it leaves behind
        // is erased as well; the window update region merges whatever the frames pile up before the
        // update.
        CRect rcFrame = pRectAnimator->getValue();
        if (rcFrame == m_rcFocusAni)
            return;
        InvalidateRect(&m_rcFocusAni);
        m_rcFocusAni = rcFrame;
        InvalidateRect(&m_rcFocusAni);
    }
    else if (pAnimator == pAlphaAnimator)
    {
        // The frame rests on its target and only its alpha changes: repaint it where it is.
        InvalidateRect(&m_rcFocusAni);
    }
}

void SRootWindow::onAnimationEnd(IValueAnimator *pAnimator)
{
    SRectAnimator *pRectAnimator = m_focusAnimator;
    SByteAnimator *pAlphaAnimator = m_focusAlphaAnimator;
    if (pAnimator == pRectAnimator)
    {
        // The frame has arrived: it stays on the target while the alpha animator waits out its start
        // delay, then fades away.
        m_focusAlphaAnimator->start(GetContainer());
    }
    else if (pAnimator == pAlphaAnimator)
    {
        // Faded out: forget the frame, this invalidate repaints the place it covered.
        InvalidateRect(&m_rcFocusAni);
        m_rcFocusAni.SetRectEmpty();
    }
}

void SRootWindow::OnAnimationStop(IAnimation *pAni)
{
    SWindow::OnAnimationStop(pAni);
    if (pAni == m_aniEnter || pAni == m_aniExit)
    {
        m_pHostWnd->m_AniState &= ~Ani_win;
        if (pAni == m_aniExit)
        {
            ULONG_PTR data = pAni->getUserData();
            if (data == kAni4Destroy)
                m_pHostWnd->SNativeWnd::DestroyWindow();
            else if (data == kAni4Hide)
            {
                m_pHostWnd->SNativeWnd::SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
            }
            pAni->setUserData(0);
        }
    }
}

void SRootWindow::BeforePaint(IRenderTarget *pRT, SPainter &painter) const
{
    m_pHostWnd->OnRootBeforePaint(this, pRT, painter);
}

void SRootWindow::AfterPaint(IRenderTarget *pRT, SPainter &painter) const
{
    m_pHostWnd->OnRootAfterPaint(this, pRT, painter);
}

void SRootWindow::BuildPainter(SPainter &painter) const
{
    m_pHostWnd->OnRootBuildPainter(this, painter);
}

void SRootWindow::UpdateLayout()
{
    if (!IsLayoutDirty())
        return;
    SIZE szAppSet = m_pHostWnd->m_szAppSetted;
    if ((szAppSet.cx <= 0 || szAppSet.cy <= 0) && GetLayoutParam()->IsWrapContent(Any))
    {
        int nWid = szAppSet.cx;
        if (nWid <= 0)
        {
            SLayoutSize layoutSize;
            GetLayoutParam()->GetSpecifiedSize(Horz, &layoutSize);
            nWid = layoutSize.toPixelSize(GetScale());
        }
        int nHei = szAppSet.cy;
        if (nHei <= 0)
        {
            SLayoutSize layoutSize;
            GetLayoutParam()->GetSpecifiedSize(Vert, &layoutSize);
            nHei = layoutSize.toPixelSize(GetScale());
        }
        CSize szRoot;
        GetDesiredSize(&szRoot, nWid, nHei);
        OnRelayout(CRect(CPoint(), szRoot));
    }
    else
    {
        SWindow::UpdateLayout();
    }
}

HRESULT SRootWindow::OnLanguageChanged()
{
    SStringT str = S_CW2T(tr(m_pHostWnd->m_hostAttr.m_strTitle));
    m_pHostWnd->SetWindowText(str);
    return 3;
}

void SRootWindow::OnScaleChanged(int scale)
{
    m_layoutDirty = dirty_self;
    GetScaleSkin(m_pSkinFocusAni, scale);
    SWindow::OnScaleChanged(scale);
    SWindow::InvalidateRect(NULL);
}

void SRootWindow::RequestRelayout(SWND hSource, BOOL bSourceResizable)
{
    SWindow *pSource = SWindowMgr::GetWindow(hSource);
    SASSERT(pSource);

    if (m_layoutDirty != dirty_self)
        m_layoutDirty = (hSource == m_swnd || bSourceResizable) ? dirty_self : dirty_child;

    CRect rcSour = pSource->GetWindowRect();
    while (rcSour.IsRectEmpty())
    {
        pSource = pSource->GetParent();
        if (!pSource)
            break;
        rcSour = pSource->GetWindowRect();
    }
    m_pHostWnd->OnRedraw(rcSour, FALSE);
}

void SRootWindow::SetHostWnd(SHostWnd *pHostWnd)
{
    m_pHostWnd = pHostWnd;
}

SHostWnd *SRootWindow::GetHostWnd() const
{
    return m_pHostWnd;
}

void SRootWindow::FireMenuCmd(int menuID)
{
    EventMenuCmd evt(this);
    evt.menuId = menuID;
    FireEvent(evt);
}

void SRootWindow::OnDecendantFocusChanged(SWND swnd, BOOL bSet)
{
    if (!m_pSkinFocusAni)
        return; // only focus-in matters, and without a skin the feature is off
    if (!bSet)
    {
        if (swnd == m_hFocus)
        {
            m_focusAnimator->end();
            m_focusAlphaAnimator->end();
            InvalidateRect(m_rcFocusAni);
            m_rcFocusAni.SetRectEmpty();
        }
    }
    else if (swnd != m_hFocus)
    {
        SWindow *pFocus = SWindowMgr::GetWindow(swnd);
        if (!pFocus)
            return;
        CRect rcTo;
        pFocus->GetVisibleRect(&rcTo);
        if (rcTo.IsRectEmpty())
            return; // focused window is scrolled/clipped out of sight
        CRect rcFrom;
        if (m_focusAnimator->isRunning())
            rcFrom = m_focusAnimator->getValue();
        else if (SWindow *pOldFocus = SWindowMgr::GetWindow(m_hFocus))
            pOldFocus->GetVisibleRect(&rcFrom);
        else
            GetVisibleRect(&rcFrom);
        m_hFocus = swnd;
        m_focusAnimator->setStart(rcFrom);
        m_focusAnimator->setEnd(rcTo);
        m_focusAnimator->start(GetContainer());
    }
}

void SRootWindow::DrawFocusAni(IRenderTarget *pRT)
{
    if (!m_pSkinFocusAni || m_rcFocusAni.IsRectEmpty())
        return; // nothing to show: without a skin the feature is off, without a rect the frame is gone
    // The frame is opaque while it travels and while it rests on the target; only the fade animator,
    // once its start delay is over and it is really running, dims it.
    BYTE byAlpha = 0xFF;
    if (!m_focusAnimator->isRunning() && m_focusAlphaAnimator->isRunning())
        byAlpha = m_focusAlphaAnimator->getValue();
    if (byAlpha)
        m_pSkinFocusAni->DrawByState2(pRT, &m_rcFocusAni, 0, byAlpha);
}

SNSEND
