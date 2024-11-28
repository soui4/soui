#include <souistd.h>
#include <control/SStackView.h>

SNSBEGIN

SStackView::SStackView(void)
    : m_iSel(-2)
    , m_animator(this)
    , m_aniStyle(kAniNone)
    , m_bVertAni(FALSE)
    , m_isSamePageSize(TRUE)
{
    m_bClipClient = true;
    m_animator.addListener(this);
}

SStackView::~SStackView(void)
{
}

BOOL SStackView::SelectPage(int iSel, BOOL enableAnimate)
{
    if (iSel < -2 || iSel >= (int)GetChildrenCount())
    {
        return FALSE;
    }
    if (m_animator.isStarted())
        m_animator.end();
    if (iSel == m_iSel)
        return TRUE;
    if (!IsVisible(TRUE))
        enableAnimate = FALSE;
    if (m_iSel == -2)
    {
        for (UINT i = 0; i < GetChildrenCount(); i++)
        {
            IWindow *pChild = GetPage(i);
            pChild->SetVisible(FALSE, FALSE);
        }
        Invalidate();
    }
    m_animator.SetFrom(m_iSel);
    m_animator.SetTo(iSel);
    m_iSel = iSel;
    if (m_iSel != -2)
    {
        if (!m_isSamePageSize)
        {
            CRect rcLayout;
            GetChildrenLayoutRect(&rcLayout);
            IWindow *pPage = GetSelPage();
            if (pPage)
            {
                CSize szPage = GetChildSize(pPage);
                m_animator.SetOwnerSizeRange(rcLayout.Size(), szPage);
            }
        }
        else
        {
            m_animator.DisableOwnerResize();
        }
        m_animator.start(GetContainer());
        if (!enableAnimate)
        {
            m_animator.end();
        }
    }
    else
    {
        for (UINT i = 0; i < GetChildrenCount(); i++)
        {
            IWindow *pChild = GetPage(i);
            pChild->SetVisible(TRUE, FALSE);
        }
        Invalidate();
    }
    return TRUE;
}

IWindow *SStackView::GetSelPage(CTHIS) const
{
    if (m_iSel < 0)
        return NULL;
    return GetPage(m_iSel);
}

IWindow *SStackView::GetPage(CTHIS_ int iPage) const
{
    if (iPage < 0 || iPage >= (int)m_childs.GetCount())
        return NULL;
    return m_childs[iPage];
}

void SStackView::UpdateChildrenPosition(THIS)
{
    CRect rc;
    GetChildrenLayoutRect(&rc);
    if (m_animator.isRunning())
    {
        m_animator.OnNextFrame();
        if (m_animator.GetAniStyle() == kFadeInOut)
        {
            IWindow *pFrom = GetPage(m_animator.GetFrom());
            if (pFrom)
            {
                CSize szPage = GetChildSize(pFrom);
                pFrom->Move(CRect(rc.TopLeft(), szPage));
            }
            IWindow *pTo = GetPage(m_animator.GetTo());
            if (pTo)
            {
                CSize szPage = GetChildSize(pTo);
                pTo->Move(CRect(rc.TopLeft(), szPage));
            }
        }
        return;
    }
    if (m_iSel == -2)
    {
        for (UINT i = 0; i < m_childs.GetCount(); i++)
        {
            CSize szPage = GetChildSize(m_childs[i]);
            CRect rcPage(rc.TopLeft(), szPage);
            m_childs[i]->Move(&rcPage);
        }
    }
    else
    {
        IWindow *pPage = GetSelPage();
        if (pPage)
        {
            pPage->Move(&rc);
        }
    }
}

void SStackView::OnDestroy()
{
    m_animator.end();
    __baseCls::OnDestroy();
}

void SStackView::OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer)
{
    m_animator.end();
    __baseCls::OnContainerChanged(pOldContainer, pNewContainer);
}

void SStackView::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);
    m_animator.OnOwnerResize();
}

void SStackView::onAnimationEnd(THIS_ IValueAnimator *pAnimator)
{
    SASSERT(m_iSel != -2);
    IWindow *pFrom = GetPage(m_animator.GetFrom());
    if (pFrom)
        pFrom->SetVisible(FALSE, TRUE);
}

void SStackView::SetAniStyle(THIS_ StackViewAniStyle aniStyle)
{
    m_aniStyle = aniStyle;
}

void SStackView::SetAniDir(THIS_ BOOL bVert)
{
    m_bVertAni = bVert;
}

SIZE SStackView::MeasureChildren(int nParentWid, int nParentHei)
{
    if (!m_isSamePageSize)
    {
        if (m_animator.isRunning())
        {
            return m_animator.GetOwnerSize();
        }
        else
        {
            IWindow *pPage = GetSelPage();
            SIZE szRet = { 0 };
            pPage->GetDesiredSize(&szRet, nParentWid, nParentHei);
            return szRet;
        }
    }
    else
    {
        SIZE ret = { 0 };
        SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
        BOOL getSize = FALSE;
        while (pChild)
        {
            if (!pChild->GetLayoutParam()->IsMatchParent(Both))
            {
                SIZE szChild;
                pChild->GetDesiredSize(&szChild, nParentWid, nParentHei);
                if (ret.cx < szChild.cx)
                    ret.cx = szChild.cx;
                if (ret.cy < szChild.cy)
                    ret.cy = szChild.cy;
                getSize = TRUE;
            }
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        }
        if (!getSize)
        {
            SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
            if (pChild)
            {
                pChild->GetDesiredSize(&ret, nParentWid, nParentHei);
            }
        }
        return ret;
    }
}

BOOL SStackView::CreateChildren(SXmlNode xmlNode)
{
    BOOL bRet = __baseCls::CreateChildren(xmlNode);
    if (!bRet)
        return FALSE;
    if (m_iSel != -2)
    {
        int idx = 0;
        SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
        while (pChild)
        {
            pChild->SetVisible(idx == m_iSel);
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
            idx++;
        }
    }
    BuildChildsArray(FALSE);
    return TRUE;
}

void SStackView::OnAfterInsertChild(SWindow *pChild)
{
    // set default size to match parent
    if (m_isLoading)
        return;

    BuildChildsArray(TRUE);
}

void SStackView::OnAfterRemoveChild(SWindow *pChild)
{
    BuildChildsArray(TRUE);
}

void SStackView::BuildChildsArray(BOOL updateSel)
{
    if (m_animator.isStarted())
        m_animator.end();
    IWindow *pSel = GetSelPage();
    int nChilds = GetChildrenCount();
    m_childs.SetCount(nChilds);
    int iSel = -1;
    SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
    for (int i = 0; i < nChilds; i++)
    {
        SASSERT(pChild);
        m_childs[i] = pChild;
        if (pSel == pChild)
            iSel = i;
        pChild = pChild->GetWindow(GSW_NEXTSIBLING);
    }
    if (updateSel)
    {
        if (iSel != -1)
        {
            m_iSel = iSel;
        }
        else if (m_iSel >= 0)
        {
            m_iSel = -1;
        }
    }
}

IWindow *SStackView::GetIChild(THIS_ int iChild) const
{
    if (iChild < 0 || iChild > (int)m_childs.GetCount())
        return NULL;
    if (iChild == 0)
        return (IWindow *)this;
    return m_childs[iChild - 1];
}

StackViewAniStyle SStackView::GetChildAnimateStyle(int iChild) const
{
    IWindow *pChild = GetPage(iChild);
    if (!pChild)
        return kAniNone;
    SStackPage *pPage = sobj_cast<SStackPage>(pChild);
    if (pPage)
        return pPage->GetAnimateStyle();
    else
        return m_aniStyle;
}

BOOL SStackView::IsVertChildAnimate(int iChild) const
{
    IWindow *pChild = GetPage(iChild);
    if (!pChild)
        return FALSE;
    SStackPage *pPage = sobj_cast<SStackPage>(pChild);
    if (pPage)
        return pPage->IsVertAnimate();
    else
        return m_bVertAni;
}

CSize SStackView::GetChildSize(IWindow *pPage) const
{
    CRect rc;
    GetChildrenLayoutRect(&rc);
    CSize szPage = rc.Size();
    if (!m_isSamePageSize)
    {
        ILayoutParam *pLayoutParam = GetLayoutParam();
        SASSERT(pPage);
        if (pLayoutParam->IsWrapContent(Any))
        {
            int nParentWid = -1, nParentHei = -1;
            if (!pLayoutParam->IsWrapContent(Horz))
                nParentWid = rc.Width();
            if (!pLayoutParam->IsWrapContent(Vert))
                nParentHei = rc.Height();
            pPage->GetDesiredSize(&szPage, nParentWid, nParentHei);
        }
    }
    return szPage;
}

//////////////////////////////////////////////////////////////////////////
void SViewSwitchAnimator::start(THIS_ ITimelineHandlersMgr *pTimerlineMgr)
{
    if (isStarted())
        end();
    CRect rc;
    m_pOwner->GetChildrenLayoutRect(&rc);
    m_pFrom = m_pOwner->GetPage(m_iFrom);
    m_pTo = m_pOwner->GetPage(m_iTo);
    if (m_pTo)
    {
        m_pTo->SetVisible(TRUE, FALSE);
    }
    m_aniStyle = m_pOwner->GetChildAnimateStyle(m_iTo);
    m_bVertAni = m_pOwner->IsVertChildAnimate(m_iTo);

    if (m_aniStyle != kFadeInOut && m_aniStyle != kAniNone)
    {
        if (m_pTo)
            m_pTo->SetAlpha(255);
        if (m_pFrom)
            m_pFrom->SetAlpha(255);
    }
    else
    {
        if (m_pTo)
            m_pTo->Move(rc);
    }
    initEvaluator();
    __baseCls::start(pTimerlineMgr);
    if (m_aniStyle == kAniNone)
    {
        // end it right now.
        __baseCls::end();
    }
}

void SViewSwitchAnimator::onEvaluateValue(THIS_ float fraction)
{
    if (m_isOwnerResize)
    {
        m_pOwner->RequestRelayout();
    }
    if (m_aniStyle == kAniNone)
        return;
    if (m_aniStyle == kFadeInOut)
    {
        BYTE a = m_evalAlphaFrom.evaluate(fraction);
        if (m_pFrom)
            m_pFrom->SetAlpha(a);
        BYTE a2 = m_evalAlphaTo.evaluate(fraction);
        if (m_pTo)
            m_pTo->SetAlpha(a2);
    }
    else
    {
        RECT rc1 = m_evalRcFrom.evaluate(fraction);
        if (m_pFrom)
            m_pFrom->Move(&rc1);
        RECT rc2 = m_evalRcTo.evaluate(fraction);
        if (m_pTo)
        {
            m_pTo->Move(&rc2);
        }
    }
}

void SViewSwitchAnimator::OnOwnerResize()
{
    if (!isStarted())
        return;
    initEvaluator();
}

void SViewSwitchAnimator::initEvaluator()
{
    CRect rc;
    m_pOwner->GetChildrenLayoutRect(&rc);

    if (m_aniStyle == kFadeInOut)
    {
        m_evalAlphaFrom.setRange(255, 0);
        m_evalAlphaTo.setRange(0, 255);
    }
    else if (m_aniStyle == kMoveInOut)
    {
        if (m_bVertAni)
        {
            if (m_iFrom < m_iTo)
            {
                // to => from => hide
                CRect rc2 = rc;
                rc2.top = rc2.bottom;
                m_evalRcFrom.setRange(rc, rc2);
                CRect rc3 = rc;
                rc3.bottom = rc3.top;
                m_evalRcTo.setRange(rc3, rc);
            }
            else
            {
                // hide <= from <= to
                CRect rc2 = rc;
                rc2.bottom = rc2.top;
                m_evalRcFrom.setRange(rc, rc2);
                CRect rc3 = rc;
                rc3.top = rc3.bottom;
                m_evalRcTo.setRange(rc3, rc);
            }
        }
        else
        {
            if (m_iFrom < m_iTo)
            {
                // to => from => hide
                CRect rc2 = rc;
                rc2.left = rc2.right;
                m_evalRcFrom.setRange(rc, rc2);
                CRect rc3 = rc;
                rc3.right = rc3.left;
                m_evalRcTo.setRange(rc3, rc);
            }
            else
            {
                // hide <= from <= to
                CRect rc2 = rc;
                rc2.right = rc2.left;
                m_evalRcFrom.setRange(rc, rc2);
                CRect rc3 = rc;
                rc3.left = rc3.right;
                m_evalRcTo.setRange(rc3, rc);
            }
        }
    }
    else if (m_aniStyle == kPushInOut)
    {
        if (m_bVertAni)
        {
            if (m_iFrom < m_iTo)
            {
                CRect rc2 = rc;
                rc2.OffsetRect(0, rc.Height());
                m_evalRcFrom.setRange(rc, rc2);
                CRect rc3 = rc;
                rc3.OffsetRect(0, -rc.Height());
                m_evalRcTo.setRange(rc3, rc);
            }
            else
            {
                CRect rc2 = rc;
                rc2.OffsetRect(0, -rc.Height());
                m_evalRcFrom.setRange(rc, rc2);
                CRect rc3 = rc;
                rc3.OffsetRect(0, rc.Height());
                m_evalRcTo.setRange(rc3, rc);
            }
        }
        else
        {
            if (m_iFrom < m_iTo)
            {
                CRect rc2 = rc;
                rc2.OffsetRect(rc.Width(), 0);
                m_evalRcFrom.setRange(rc, rc2);
                CRect rc3 = rc;
                rc3.OffsetRect(-rc.Width(), 0);
                m_evalRcTo.setRange(rc3, rc);
            }
            else
            {
                CRect rc2 = rc;
                rc2.OffsetRect(-rc.Width(), 0);
                m_evalRcFrom.setRange(rc, rc2);
                CRect rc3 = rc;
                rc3.OffsetRect(rc.Width(), 0);
                m_evalRcTo.setRange(rc3, rc);
            }
        }
    }
}

void SViewSwitchAnimator::SetOwnerSizeRange(SIZE szFrom, SIZE szTo)
{
    m_evalOwnerSize.setRange(szFrom, szTo);
    m_isOwnerResize = szFrom.cx != szTo.cx || szFrom.cy != szTo.cy;
}

SIZE SViewSwitchAnimator::GetOwnerSize() const
{
    return m_evalOwnerSize.evaluate(getAnimatedFraction());
}

SNSEND
