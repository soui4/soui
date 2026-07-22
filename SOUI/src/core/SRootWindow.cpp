#include <souistd.h>
#include <core/SRootWindow.h>
#include <core/SHostWnd.h>
#include <layout/SAnchorLayout.h>

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// SRootWindow
//////////////////////////////////////////////////////////////////////////


SRootWindow::SRootWindow()
    : m_pHostWnd(NULL)
{
    GetEventSet()->addEvent(EVENTID(EventInit));
    GetEventSet()->addEvent(EVENTID(EventExit));
    GetEventSet()->addEvent(EVENTID(EventMenuCmd));
    GetEventSet()->addEvent(EVENTID(EventUpdateCmdUI));
    GetEventSet()->addEvent(EVENTID(EventUpdateCmdTip));
}

SRootWindow::~SRootWindow()
{
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

void SRootWindow::BuildPainter(SPainter &painter) const{
    m_pHostWnd->OnRootBuildPainter(this,painter);
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

void SRootWindow::OnLayoutFloatChild(SWindow* pChild, const CRect& rcLayout) {
    if(pChild->IsClass(SModalRoot::GetClassName()))
    {
        SModalRoot* pModalRoot = sobj_cast<SModalRoot>(pChild);
        pModalRoot->Move(rcLayout);
    }
    else
    {
		SWindow::OnLayoutFloatChild(pChild, rcLayout);
    }
}

SNSEND