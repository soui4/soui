#include "souistd.h"
#include "control/STabCtrl.h"
#include "animation/SInterpolatorImpl.h"

SNSBEGIN

class STabSlider
    : public SWindow
    , public IAnimatorUpdateListener
    , public SAnimatorListener {
    DEF_SOBJECT(SWindow, L"tabslider")

  public:
    STabSlider(STabCtrl *pTabCtrl, int iFrom, int iTo, BOOL bDragMode = FALSE)
        : m_pTabCtrl(pTabCtrl)
        , m_iFrom(iFrom)
        , m_iTo(iTo)
        , m_bDragMode(bDragMode)
        , m_fDragFraction(0.f)
        , m_bCommitting(FALSE)
        , m_fCommitFrom(0.f)
        , m_fCommitTo(1.f)
    {
        SWindow::EnableAcc(FALSE);
        SASSERT(pTabCtrl);
        m_bClipClient = TRUE;
        pTabCtrl->m_aniSlider->addListener(this);
        pTabCtrl->m_aniSlider->addUpdateListener(this);

        m_bVertical = pTabCtrl->m_nTabAlign == STabCtrl::AlignLeft || pTabCtrl->m_nTabAlign == STabCtrl::AlignRight;
        pTabCtrl->GetRoot()->UpdateLayout();

        CRect rcPage;
        pTabCtrl->GetChildrenLayoutRect(&rcPage);
        pTabCtrl->InsertChild(this);
        SetVisible(FALSE);
        if (pTabCtrl->m_nTabAlign == STabCtrl::AlignMiddle)
        {
            CRect rcClient = pTabCtrl->GetClientRect();
            rcClient.DeflateRect(pTabCtrl->GetStyle().GetPadding());
            Move(rcClient);
        }
        else
        {
            Move(rcPage);
        }

        SWindow *pageFrom = pTabCtrl->GetItem(iFrom);
        SWindow *pageTo = pTabCtrl->GetItem(iTo);

        // render page from content to m_rtPageFrom
        GETRENDERFACTORY->CreateRenderTarget(&m_rtPageFrom, rcPage.Width(), rcPage.Height());
        m_rtPageFrom->BeginDraw();
        pageFrom->SetVisible(TRUE);
        pageTo->SetVisible(FALSE);
        m_rtPageFrom->SetViewportOrg(-rcPage.TopLeft());
        pageFrom->PaintBackground(m_rtPageFrom, &rcPage);
        pageFrom->SSendMessage(WM_ERASEBKGND, (WPARAM)m_rtPageFrom.Get());
        pageFrom->SSendMessage(WM_PAINT, (WPARAM)m_rtPageFrom.Get());
        pageFrom->PaintForeground(m_rtPageFrom, &rcPage);
        m_rtPageFrom->EndDraw();
        m_rtPageFrom->SetViewportOrg(CPoint());
        IBitmapS *pBmp = (IBitmapS *)m_rtPageFrom->GetCurrentObject(OT_BITMAP);

        // render page to content to m_rtPageTo
        GETRENDERFACTORY->CreateRenderTarget(&m_rtPageTo, rcPage.Width(), rcPage.Height());
        m_rtPageTo->BeginDraw();
        pageFrom->SetVisible(FALSE);
        pageTo->SetVisible(TRUE);
        m_rtPageTo->SetViewportOrg(-rcPage.TopLeft());
        pageTo->PaintBackground(m_rtPageTo, &rcPage);
        pageTo->SSendMessage(WM_ERASEBKGND, (WPARAM)m_rtPageTo.Get());
        pageTo->SSendMessage(WM_PAINT, (WPARAM)m_rtPageTo.Get());
        pageTo->PaintForeground(m_rtPageTo, &rcPage);
        m_rtPageTo->EndDraw();
        m_rtPageTo->SetViewportOrg(CPoint());

        pBmp = (IBitmapS *)m_rtPageTo->GetCurrentObject(OT_BITMAP);
        // hide page from and page to
        pageFrom->SetVisible(FALSE);
        pageTo->SetVisible(FALSE);
        // In drag mode the finger position drives the transition instead of the
        // animator; CommitDrag() starts it once the button is released.
        if (!m_bDragMode)
        {
            pTabCtrl->m_aniSlider->start(GetContainer());
        }
        SetVisible(TRUE, TRUE);
        SWindow::EnableAcc(TRUE);
    }

    virtual ~STabSlider()
    {
        m_pTabCtrl->m_aniSlider->removeListener(this);
        m_pTabCtrl->m_aniSlider->removeUpdateListener(this);
    }

    void Stop()
    {
        m_pTabCtrl->OnSliderFinish();
    }

    /** Drive the transition from the finger displacement (drag mode only). */
    void SetDragFraction(float f)
    {
        if (!m_bDragMode)
            return;
        if (f < 0.f)
            f = 0.f;
        if (f > 1.f)
            f = 1.f;
        if (f != m_fDragFraction)
        {
            m_fDragFraction = f;
            Invalidate();
        }
    }

    float GetDragFraction() const
    {
        return m_fDragFraction;
    }

    /** Finish a drag: animate from the current fraction to 1 (switch) or 0 (snap back). */
    void CommitDrag(BOOL bSwitch, long nDuration)
    {
        if (!m_bDragMode)
            return;
        m_bDragMode = FALSE;
        m_fCommitFrom = m_fDragFraction;
        m_fCommitTo = bSwitch ? 1.f : 0.f;
        m_bCommitting = TRUE;
        if (nDuration <= 0)
        {
            // No timeline needed (headless friendly): finish synchronously.
            Stop();
            return;
        }
        m_pTabCtrl->m_aniSlider->setDuration(nDuration);
        m_pTabCtrl->m_aniSlider->start(GetContainer());
    }

  protected:
    STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator *pAnimator) OVERRIDE
    {
        Invalidate();
    }
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator *pAnimator) OVERRIDE
    {
        Stop();
    }

    int GetItemState(int iItem) const
    {
        DWORD dwState = WndState_Normal;
        if (iItem == m_iTo)
        {
            dwState = WndState_PushDown;
        }
        return dwState;
    }
    void OnPaint(IRenderTarget *pRT)
    {
        CRect rcWnd = GetClientRect();
        float fraction = 0.f;
        if (m_bDragMode)
            fraction = m_fDragFraction;
        else if (m_bCommitting)
        {
            float f = m_pTabCtrl->m_aniSlider->getAnimatedFraction();
            fraction = m_fCommitFrom + (m_fCommitTo - m_fCommitFrom) * f;
        }
        else
            fraction = m_pTabCtrl->m_aniSlider->getAnimatedFraction();
        if (m_pTabCtrl->m_nTabAlign == STabCtrl::AlignMiddle)
        {
            int nHeight = rcWnd.Height();
            int nTabHeight = m_pTabCtrl->GetTitleRect().Height();
            int nPageHeight = rcWnd.Height() - nTabHeight * m_pTabCtrl->GetItemCount();
            int nPageFromHeight = nPageHeight * (1.0f - fraction);
            int nPageToHeight = nPageHeight - nPageFromHeight;

            CRect rcTab(rcWnd.TopLeft(), CSize(rcWnd.Width(), nTabHeight));
            // Draw all tab items before from and to
            int iTopItem = smin(m_iFrom, m_iTo);
            int iBottomItem = smax(m_iFrom, m_iTo);
            if (m_iFrom == m_iTo)
                return;
            IRenderTarget *pRtTop = m_iFrom < m_iTo ? m_rtPageFrom : m_rtPageTo;
            IRenderTarget *pRtBottom = m_iFrom > m_iTo ? m_rtPageFrom : m_rtPageTo;
            POINT ptViewOrg1, ptViewOrg2;
            pRtTop->GetViewportOrg(&ptViewOrg1);
            pRtBottom->GetViewportOrg(&ptViewOrg2);
            for (int i = 0; i <= iTopItem; i++)
            {
                int state = GetItemState(i);
                m_pTabCtrl->DrawItem(pRT, rcTab, i, state);
                rcTab.OffsetRect(0, nTabHeight);
            }
            // draw middle parts
            CRect rcCenter = rcWnd;
            rcCenter.top += nTabHeight * (iTopItem + 1);
            rcCenter.bottom -= (m_pTabCtrl->GetItemCount() - iBottomItem - 1) * nTabHeight;
            pRT->PushClipRect(rcCenter, RGN_AND);
            int offset = (m_iFrom < m_iTo ? fraction : (1 - fraction)) * nPageHeight;

            CRect rcItem = rcTab;
            rcItem.top -= offset;
            rcItem.bottom = rcItem.top + nPageHeight;
            // draw top page
            pRT->AlphaBlend(rcItem, pRtTop, CRect(CPoint(), rcItem.Size()), 0xff);

            rcTab.MoveToY(rcItem.bottom);
            // draw middle tabs
            for (int i = iTopItem + 1; i <= iBottomItem; i++)
            {
                int state = GetItemState(i);
                m_pTabCtrl->DrawItem(pRT, rcTab, i, state);
                rcTab.OffsetRect(0, nTabHeight);
            }
            // draw bottom page
            rcItem.MoveToY(rcTab.top);
            pRT->AlphaBlend(rcItem, pRtBottom, CRect(CPoint(), rcItem.Size()), 0xff);

            pRT->PopClip();
            // Draw all tab items after from and to
            rcTab.MoveToY(rcWnd.bottom - nTabHeight * (m_pTabCtrl->GetItemCount() - iBottomItem - 1));
            for (int i = iBottomItem + 1; i < m_pTabCtrl->GetItemCount(); i++)
            {
                int state = GetItemState(i);
                m_pTabCtrl->DrawItem(pRT, rcTab, i, state);
                rcTab.OffsetRect(0, nTabHeight);
            }
        }
        else
        {
            CRect rcPageFrom, rcPageTo;
            if (m_bVertical)
            {
                int nHeight = rcWnd.Height();
                rcPageTo = rcPageFrom = CRect(0, 0, rcWnd.Width(), nHeight);
                int nOffset = -nHeight * fraction;
                if (m_iFrom < m_iTo)
                {
                    rcPageTo.OffsetRect(0, nHeight);
                }
                else
                {
                    rcPageTo.OffsetRect(0, -nHeight);
                    nOffset *= -1;
                }
                rcPageTo.OffsetRect(0, nOffset);
                rcPageFrom.OffsetRect(0, nOffset);
            }
            else
            {
                int nWidth = rcWnd.Width();
                rcPageTo = rcPageFrom = CRect(0, 0, nWidth, rcWnd.Height());
                int nOffset = -nWidth * fraction;
                if (m_iFrom < m_iTo)
                {
                    rcPageTo.OffsetRect(nWidth, 0);
                }
                else
                {
                    rcPageTo.OffsetRect(-nWidth, 0);
                    nOffset *= -1;
                }
                rcPageTo.OffsetRect(nOffset, 0);
                rcPageFrom.OffsetRect(nOffset, 0);
            }
            rcPageFrom.OffsetRect(rcWnd.TopLeft());
            rcPageTo.OffsetRect(rcWnd.TopLeft());

            CRect rcSrc = rcWnd;
            rcSrc.MoveToXY(0, 0);
            pRT->AlphaBlend(rcPageFrom, m_rtPageFrom, rcSrc, 0xff);
            pRT->AlphaBlend(rcPageTo, m_rtPageTo, rcSrc, 0xff);
        }
    }

    void OnSize(UINT fType, CSize sz)
    {
        SWindow::OnSize(fType, sz);
        if (!m_rtPageFrom || !m_rtPageTo)
            return;
        Stop();
    }

    SAutoRefPtr<IRenderTarget> m_rtPageFrom, m_rtPageTo;
    int m_iFrom, m_iTo;
    bool m_bVertical;
    STabCtrl *m_pTabCtrl;
    BOOL m_bDragMode;      /**< TRUE while the finger position drives the transition */
    float m_fDragFraction; /**< fraction supplied by SetDragFraction() */
    BOOL m_bCommitting;    /**< TRUE while animating from m_fCommitFrom to m_fCommitTo */
    float m_fCommitFrom;   /**< fraction at the moment the button was released */
    float m_fCommitTo;     /**< 1 = switch page, 0 = snap back */
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SIZE(OnSize)
    SOUI_MSG_MAP_END()
};

///////////////////////////////////////////////////////////////////////
/** STabCtrl */

STabCtrl::STabCtrl()
    : m_nCurrentPage(0)
    , m_pSkinTab(GETBUILTINSKIN(SKIN_SYS_TAB_PAGE))
    , m_pSkinIcon(NULL)
    , m_pSkinTabInter(NULL)
    , m_pSkinFrame(NULL)
    , m_nTabInterSize(0, px)
    , m_nTabPos(0, px)
    , m_nHoverTabItem(-1)
    , m_nTabAlign(AlignTop)
    , m_tabSlider(NULL)
    , m_txtDir(Text_Horz)
    , m_bDragSwitchEnable(FALSE)
    , m_nDragThreshold(8.f, dp)
    , m_nDragSwitchRatio(35)
    , m_bDragFlingEnable(TRUE)
    , m_nDragCommitDuration(200)
    , m_gesture(this)
    , m_bDragArmed(FALSE)
    , m_bDragSwitching(FALSE)
    , m_bDragCommitting(FALSE)
    , m_bDragFling(FALSE)
    , m_nDragFrom(-1)
    , m_nDragTo(-1)
    , m_nDragSavedDuration(0)
{
    m_ptText[0] = m_ptText[1] = SLayoutSize(-1.f, px);
    m_szTab[0] = m_szTab[1] = SLayoutSize(-1.f, px);
    m_ptTextPad[0] = m_ptTextPad[1] = SLayoutSize(4.f, px);

    m_bFocusable = TRUE;
    m_aniSlider.Attach(new SFloatAnimator);
    m_aniSlider->setDuration(0); // default no animation
    m_evtSet.addEvent(EVENTID(EventTabSelChanging));
    m_evtSet.addEvent(EVENTID(EventTabSelChanged));
    m_evtSet.addEvent(EVENTID(EventTabItemHover));
    m_evtSet.addEvent(EVENTID(EventTabItemLeave));
}

void STabCtrl::OnPaint(IRenderTarget *pRT)
{
    SPainter painter;
    BeforePaint(pRT, painter);

    CRect rcTitle = GetTitleRect();

    if (!rcTitle.IsRectEmpty() || (m_nTabAlign == AlignMiddle && !m_tabSlider))
    {
        CRect rcItem, rcItemPrev;
        CRect rcSplit;
        DWORD dwState;

        if (m_nTabAlign != AlignMiddle)
            pRT->PushClipRect(&rcTitle, RGN_AND);

        for (int i = 0; i < (int)GetItemCount(); i++)
        {
            dwState = WndState_Normal;
            if (i == m_nCurrentPage)
                dwState = WndState_PushDown;
            else if (i == m_nHoverTabItem)
                dwState = WndState_Hover;

            GetItemRect2(i, rcItem, pRT);
            if (rcItem.IsRectEmpty())
                continue;

            // Draw separator line
            if (i > 0 && m_pSkinTabInter)
            {
                rcSplit = rcItem;
                if (m_nTabAlign == AlignLeft)
                {
                    rcSplit.top = rcItemPrev.bottom;
                    rcSplit.bottom = rcSplit.top + m_nTabInterSize.toPixelSize(GetScale());
                }
                else
                {
                    rcSplit.left = rcItemPrev.right;
                    rcSplit.right = rcSplit.left + m_nTabInterSize.toPixelSize(GetScale());
                }
                m_pSkinTabInter->DrawByIndex(pRT, rcSplit, 0);
            }

            DrawItem(pRT, rcItem, i, dwState);
            rcItemPrev = rcItem;
        }
        if (m_nTabAlign != AlignMiddle)
            pRT->PopClip();
    }
    if (m_pSkinFrame)
    {
        CRect rcPage;
        GetChildrenLayoutRect(&rcPage);
        m_pSkinFrame->DrawByState(pRT, rcPage, WndState_Normal);
    }

    if (IsFocused() && IsFocusable() && m_bDrawFocusRect)
    {
        CRect rc;
        GetItemRect(m_nCurrentPage, rc);
        rc.DeflateRect(2, 2);
        DrawDefFocusRect(pRT, &rc);
    }
    AfterPaint(pRT, painter);
}

void STabCtrl::GetChildrenLayoutRect(RECT *prc) const
{
    CRect rcRet;
    GetClientRect(rcRet);
    rcRet.DeflateRect(GetStyle().GetPadding());
    CRect rcTitle = GetTitleRect();
    int nTabWidth = rcTitle.Width();
    int nTabHeight = rcTitle.Height();
    switch (m_nTabAlign)
    {
    case AlignLeft:
        rcRet.left += nTabWidth;
        break;
    case AlignRight:
        rcRet.right -= nTabWidth;
        break;
    case AlignTop:
        rcRet.top += nTabHeight;
        break;
    case AlignBottom:
        rcRet.bottom -= nTabHeight;
        break;
    case AlignMiddle:
        rcRet.top += (m_nCurrentPage + 1) * nTabHeight;
        rcRet.bottom -= (GetItemCount() - m_nCurrentPage - 1) * nTabHeight;
        break;
    }
    *prc = rcRet;
}

void STabCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
    SWindow::OnLButtonDown(nFlags, point);
    int iClickItem = HitTest(point);
    if (iClickItem != m_nCurrentPage)
    {
        SetCurSel(iClickItem);
    }
    // When this window itself is the hit target, the container's interception
    // chain does not include us: drive the same detector from the message map.
    m_bDragArmed = CanDragSwitch() && IsPointInPage(point);
    if (m_bDragArmed)
    {
        PrepareGesture();
        m_gesture.OnTouchEvent(WM_LBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));
    }
    else
    {
        m_gesture.Reset();
    }
}

void STabCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
    // Finish the drag before the base class releases capture: ReleaseCapture()
    // fires OnCaptureChanged(FALSE), which cancels an in-progress drag.
    if (m_bDragArmed)
        m_gesture.OnTouchEvent(WM_LBUTTONUP, nFlags, MAKELPARAM(point.x, point.y));
    m_bDragArmed = FALSE;
    m_gesture.Reset();
    __baseCls::OnLButtonUp(nFlags, point);
}

void STabCtrl::OnCaptureChanged(BOOL bCaptured)
{
    // Capture was taken away without an UP (focus loss, another window captured,
    // scrollbar grabbed...). Drop the drag state, otherwise the live slider keeps
    // both pages hidden until the next DOWN.
    if (!bCaptured && m_bDragSwitching)
        CancelDragSwitch();
    if (!bCaptured)
        m_bDragArmed = FALSE;
    __baseCls::OnCaptureChanged(bCaptured);
}

void STabCtrl::OnShowWindow(BOOL bShow, UINT nStatus)
{
    __baseCls::OnShowWindow(bShow, nStatus);
    // IsVisible(TRUE) also covers the ParentShow case, where bShow is TRUE but
    // an ancestor just got hidden.
    if (!IsVisible(TRUE) && m_bDragSwitching)
        CancelDragSwitch();
}

void STabCtrl::OnEnable(BOOL bEnable, UINT nStatus)
{
    __baseCls::OnEnable(bEnable, nStatus);
    if (IsDisabled(TRUE) && m_bDragSwitching)
        CancelDragSwitch();
}

BOOL STabCtrl::RemoveItem(int nIndex, int nSelPage /**< =0 */)
{
    STabPage *pTab = GetItem(nIndex);
    OnItemRemoved(pTab);

    DestroyChild(pTab);
    m_lstPages.RemoveAt(nIndex);

    if (m_nCurrentPage == nIndex)
    {
        if (nSelPage < 0)
            nSelPage = 0;
        if (nSelPage >= GetItemCount())
            nSelPage = GetItemCount() - 1;
        m_nCurrentPage = -1;
        SetCurSel(nSelPage);
    }
    else
    {
        if (m_nCurrentPage > nIndex)
            m_nCurrentPage--;
    }
    CRect rcTitle = GetTitleRect();
    InvalidateRect(rcTitle);
    accNotifyEvent(EVENT_OBJECT_REORDER);
    return TRUE;
}

void STabCtrl::RemoveAllItems(void)
{
    for (int i = GetItemCount() - 1; i >= 0; i--)
    {
        STabPage *pTab = GetItem(i);
        OnItemRemoved(pTab);
        DestroyChild(pTab);
        m_lstPages.RemoveAt(i);
    }
    m_nCurrentPage = -1;
    Invalidate();
    accNotifyEvent(EVENT_OBJECT_REORDER);
}

void STabCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
    // While dragging, skip hover bookkeeping so tab items do not flicker under
    // the finger and no hover events leak into the page switch.
    if (m_bDragArmed && m_gesture.OnTouchEvent(WM_MOUSEMOVE, nFlags, MAKELPARAM(point.x, point.y)))
        return;
    CRect rcItem;
    int nOldHover = m_nHoverTabItem;
    m_nHoverTabItem = -1;
    int nTabCount = GetItemCount();
    for (int i = 0; i < nTabCount; i++)
    {
        GetItemRect(i, rcItem);

        if (rcItem.PtInRect(point))
        {
            m_nHoverTabItem = i;
            break;
        }
    }
    if (m_nHoverTabItem != nOldHover)
    {
        if (nOldHover != -1)
        {
            if (nOldHover != m_nCurrentPage)
            {
                GetItemRect(nOldHover, rcItem);
                InvalidateRect(rcItem);
            }
            EventTabItemLeave evt(this);
            evt.iLeave = nOldHover;
            FireEvent(evt);
        }
        if (m_nHoverTabItem != -1)
        {
            if (m_nHoverTabItem != m_nCurrentPage)
            {
                GetItemRect(m_nHoverTabItem, rcItem);
                InvalidateRect(rcItem);
            }
            EventTabItemHover evt(this);
            evt.iHover = m_nHoverTabItem;
            FireEvent(evt);
        }
    }
}

void STabCtrl::OnDestroy()
{
    if (m_tabSlider)
    {
        m_tabSlider->Stop();
    }
    for (int i = GetItemCount() - 1; i >= 0; i--)
    {
        DestroyChild(m_lstPages[i]);
    }
    m_lstPages.RemoveAll();
    __baseCls::OnDestroy();
}

STabPage *STabCtrl::GetItem(int iPage)
{
    if (iPage < 0 || iPage >= (int)GetItemCount())
        return NULL;
    return m_lstPages[iPage];
}

int STabCtrl::GetPageIndex(LPCTSTR pszName, BOOL bTitle)
{
    if (bTitle)
    {
        for (UINT i = 0; i < m_lstPages.GetCount(); i++)
        {
            if (_tcscmp(m_lstPages[i]->GetTitle(), pszName) == 0)
                return i;
        }
    }
    else
    {
        SStringW strName = S_CT2W(pszName);
        for (UINT i = 0; i < m_lstPages.GetCount(); i++)
        {
            if (m_lstPages[i]->GetName() == strName)
                return i;
        }
    }
    return -1;
}

STabPage *STabCtrl::GetPage(LPCTSTR pszName, BOOL bTitle /**< =TRUE */)
{
    int iPage = GetPageIndex(pszName, bTitle);
    if (iPage == -1)
        return NULL;
    return m_lstPages[iPage];
}

IWindow *STabCtrl::GetPage(int iPage)
{
    if (iPage < 0 || iPage >= GetItemCount())
        return NULL;
    return m_lstPages[iPage];
}

void STabCtrl::OnSliderFinish()
{
    // Clear the drag-commit state even when the slider is already gone (e.g. a
    // subscriber re-entered SetCurSel() from EventTabSelChanged). Otherwise the
    // next programmatic switch would keep the temporary drag commit duration.
    if (m_bDragCommitting)
    {
        m_bDragCommitting = FALSE;
        m_aniSlider->setDuration(m_nDragSavedDuration);
    }
    // A late animation callback (e.g. the page list was emptied while the
    // commit animation was still running) must not touch a destroyed slider.
    if (!m_tabSlider)
        return;
    DestroyChild(m_tabSlider);
    m_tabSlider = NULL;
    if (m_nCurrentPage >= 0 && m_nCurrentPage < GetItemCount())
    {
        STabPage *pPage = m_lstPages[m_nCurrentPage];
        SASSERT(pPage);
        pPage->SetVisible(TRUE, TRUE);
    }
}

BOOL STabCtrl::SetCurSel(int nIndex)
{
    // A programmatic/click selection must abort an in-progress drag first,
    // otherwise the two transitions would fight over m_tabSlider.
    if (m_bDragSwitching)
        CancelDragSwitch();
    if (nIndex < 0 || nIndex > (int)GetItemCount() - 1 || (m_nCurrentPage == nIndex))
        return FALSE;
    int nOldPage = m_nCurrentPage;

    EventTabSelChanging evt(this);
    evt.bCancel = FALSE;
    evt.uOldSel = nOldPage;
    evt.uNewSel = nIndex;

    FireEvent(evt);

    if (evt.bCancel)
        return FALSE;

    CRect rcItem;

    GetItemRect(m_nCurrentPage, rcItem);
    InvalidateRect(rcItem);

    if (m_tabSlider)
    {
        m_tabSlider->Stop();
    }
    if (m_aniSlider->getDuration() > 0 && IsVisible(TRUE) && nOldPage != -1 && nIndex != -1)
    {
        m_tabSlider = new STabSlider(this, nOldPage, nIndex);
    }
    else
    {
        if (nOldPage != -1)
        {
            GetItem(nOldPage)->SetVisible(FALSE, TRUE);
        }
        if (nIndex != -1)
        {
            GetItem(nIndex)->SetVisible(TRUE, TRUE);
        }
    }
    m_nCurrentPage = nIndex;
    UpdateChildrenPosition();
    GetItemRect(m_nCurrentPage, rcItem);
    InvalidateRect(rcItem);

    EventTabSelChanged evt2(this);
    evt2.uNewSel = nIndex;
    evt2.uOldSel = nOldPage;

    FireEvent(evt2);
    accNotifyEvent(EVENT_OBJECT_SELECTION);
    return TRUE;
}

BOOL STabCtrl::SetCurSel(LPCTSTR pszName, BOOL bTitle /**< =TRUE */)
{
    int iPage = GetPageIndex(pszName, bTitle);
    if (iPage == -1)
        return FALSE;
    return SetCurSel(iPage);
}

BOOL STabCtrl::SetItemTitle(int nIndex, LPCTSTR lpszTitle)
{
    STabPage *pTab = GetItem(nIndex);
    if (pTab)
    {
        pTab->SetTitle(lpszTitle);

        CRect rcTitle = GetTitleRect();
        InvalidateRect(rcTitle);
        return TRUE;
    }

    return FALSE;
}

BOOL STabCtrl::CreateChildren(SXmlNode xmlNode)
{
    for (SXmlNode xmlChild = xmlNode.first_child(); xmlChild; xmlChild = xmlChild.next_sibling())
    {
        InsertItem(xmlChild, -1, TRUE);
    }

    if (m_nCurrentPage == -1 || m_nCurrentPage >= (int)m_lstPages.GetCount())
    {
        m_nCurrentPage = 0;
    }
    if (m_lstPages.GetCount() == 0)
    {
        m_nCurrentPage = -1;
    }

    if (m_nCurrentPage != -1)
    {
        GetItem(m_nCurrentPage)->SetVisible(TRUE);
    }
    return TRUE;
}

STabPage *STabCtrl::CreatePageFromXml(SXmlNode xmlPage)
{
    if (wcscmp(xmlPage.name(), STabPage::GetClassName()) != 0)
        return NULL;
    return (STabPage *)CreateChildByName(STabPage::GetClassName());
}

int STabCtrl::InsertItem(LPCWSTR lpContent, int iInsert /**< =-1 */)
{
    SXmlDoc xmlDoc;
    if (!xmlDoc.load_buffer(lpContent, wcslen(lpContent) * sizeof(wchar_t), xml_parse_default, sizeof(wchar_t) == 2 ? enc_utf16 : enc_utf32))
        return -1;
    return InsertItem(xmlDoc.root().first_child(), iInsert);
}

int STabCtrl::InsertItem(SXmlNode xmlNode, int iInsert /**< =-1 */, BOOL bLoading /**< =FALSE */)
{
    if (_wcsicmp(xmlNode.name(), L"include") == 0)
    { // support include tag
        SStringT strSrc = S_CW2T(xmlNode.attribute(L"src").value());
        SXmlDoc xmlDoc;
        if (!LOADXML(xmlDoc, strSrc))
            return -1;
        return InsertItem(xmlDoc.root().first_child(), iInsert, bLoading);
    }
    if (wcscmp(xmlNode.name(), STabPage::GetClassName()) != 0)
        return -1;
    STabPage *pChild = CreatePageFromXml(xmlNode);
    if (!pChild)
        return -1;

    InsertChild(pChild);
    pChild->InitFromXml(&xmlNode);
    pChild->GetLayoutParam()->SetMatchParent(Both);

    if (iInsert == -1)
        iInsert = (int)m_lstPages.GetCount();
    m_lstPages.InsertAt(iInsert, pChild);

    if (!bLoading)
    {
        if (GetScale() != 100)
            pChild->SDispatchMessage(UM_SETSCALE, GetScale(), 0);
        CRect rcPage;
        GetChildrenLayoutRect(&rcPage);
        pChild->Move(&rcPage);
        pChild->SetVisible(FALSE, FALSE);
        if (m_nCurrentPage >= iInsert)
            m_nCurrentPage++;
        InvalidateRect(GetTitleRect());
        if (m_nCurrentPage == -1)
            SetCurSel(iInsert);
    }
    OnItemInserted(pChild);
    accNotifyEvent(EVENT_OBJECT_REORDER);
    return iInsert;
}

CRect STabCtrl::GetTitleRect() const
{
    CRect rcTitle;
    GetClientRect(rcTitle);
    int tabWid = rcTitle.Width();
    if (m_szTab[0].isSpecifiedSize())
        tabWid = m_szTab[0].toPixelSize(GetScale());
    else if (m_szTab[0].isWrapContent())
    {
        SASSERT(m_pSkinTab);
        if (m_pSkinTab)
            tabWid = m_pSkinTab->GetSkinSize().cx;
    }
    int tabHei = rcTitle.Height();
    if (m_szTab[1].isSpecifiedSize())
        tabHei = m_szTab[1].toPixelSize(GetScale());
    else if (m_szTab[1].isWrapContent())
    {
        SASSERT(m_pSkinTab);
        if (m_pSkinTab)
            tabHei = m_pSkinTab->GetSkinSize().cy;
    }
    switch (m_nTabAlign)
    {
    case AlignTop:
        rcTitle.bottom = rcTitle.top + tabHei;
        break;
    case AlignBottom:
        rcTitle.top = rcTitle.bottom - tabHei;
        break;
    case AlignLeft:
        rcTitle.right = rcTitle.left + tabWid;
        break;
    case AlignRight:
        rcTitle.left = rcTitle.right - tabWid;
        break;
    case AlignMiddle:
        rcTitle.bottom = rcTitle.top + tabHei;
        break;
    }
    return rcTitle;
}

int STabCtrl::MeasureTabWidth(IRenderTarget *pRT, int iPage) const
{ // wrapContent width: title text or icon extent plus textPadding-x on both sides
    if (iPage < 0 || iPage >= GetItemCount())
        return 0;
    int nScale = GetScale();
    int nPad = m_ptTextPad[0].toPixelSize(nScale);
    int nW = 0;
    LPCTSTR pszTitle = m_lstPages[iPage]->GetTitle();
    if (pszTitle && *pszTitle)
    {
        SStringT strTitle = pszTitle;
        SIZE szTxt = { 0, 0 };
        if (m_txtDir == Text_Vert)
            szTxt = MeasureTextV(pRT, strTitle);
        else
            pRT->MeasureText(strTitle, strTitle.GetLength(), &szTxt);
        nW = smax(nW, m_ptText[0].toPixelSize(nScale) + szTxt.cx);
    }
    if (m_pSkinIcon)
        nW = smax(nW, m_ptIcon[0].toPixelSize(nScale) + m_pSkinIcon->GetSkinSize().cx);
    return nW + nPad * 2;
}

int STabCtrl::MeasureTabHeight(IRenderTarget *pRT, int iPage) const
{ // wrapContent height: title text or icon extent plus textPadding-y on both sides
    if (iPage < 0 || iPage >= GetItemCount())
        return 0;
    int nScale = GetScale();
    int nPad = m_ptTextPad[1].toPixelSize(nScale);
    int nH = 0;
    LPCTSTR pszTitle = m_lstPages[iPage]->GetTitle();
    if (pszTitle && *pszTitle)
    {
        SStringT strTitle = pszTitle;
        SIZE szTxt = { 0, 0 };
        if (m_txtDir == Text_Vert)
            szTxt = MeasureTextV(pRT, strTitle);
        else
            pRT->MeasureText(strTitle, strTitle.GetLength(), &szTxt);
        nH = smax(nH, m_ptText[1].toPixelSize(nScale) + szTxt.cy);
    }
    if (m_pSkinIcon)
        nH = smax(nH, m_ptIcon[1].toPixelSize(nScale) + m_pSkinIcon->GetSkinSize().cy);
    return nH + nPad * 2;
}

BOOL STabCtrl::GetItemRect(int nIndex, CRect &rcItem) const
{
    SAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT);
    BeforePaintEx(pRT);
    return GetItemRect2(nIndex, rcItem, pRT);
}

BOOL STabCtrl::GetItemRect2(int nIndex, CRect &rcItem, IRenderTarget *pRT) const
{
    if (nIndex < 0 || nIndex >= (int)GetItemCount())
        return FALSE;
    CRect rcTitle = GetTitleRect();
    int nScale = GetScale();
    int nInter = m_nTabInterSize.toPixelSize(nScale);
    int nTabPos = m_nTabPos.toPixelSize(nScale);
    if (m_nTabAlign == AlignMiddle)
    {
        // middle, each tab is same height
        CRect rcClient = GetClientRect();
        int nTabHeight = rcTitle.Height();
        rcItem = CRect(0, 0, rcTitle.Width(), nTabHeight);
        if (nIndex <= m_nCurrentPage)
        {
            rcItem.OffsetRect(0, nTabHeight * nIndex);
        }
        else
        {
            rcItem.OffsetRect(0, rcClient.Height() - nTabHeight * (GetItemCount() - nIndex));
        }
        rcItem.OffsetRect(rcClient.left, rcClient.top);
        return TRUE;
    }
    else
    {
        bool bVert = (m_nTabAlign == AlignLeft || m_nTabAlign == AlignRight);
        int nStripAxis = bVert ? 1 : 0;
        int nCrossAxis = 1 - nStripAxis;
        int nCross;
        if (m_szTab[nCrossAxis].isWrapContent())
        { // wrapContent: all tabs share the max measured size
            nCross = 0;
            for (int i = 0; i < GetItemCount(); i++)
                nCross = smax(nCross, (nCrossAxis == 0) ? MeasureTabWidth(pRT, i) : MeasureTabHeight(pRT, i));
        }
        else if (m_szTab[nCrossAxis].isMatchParent())
        { // matchParent: tabs span the whole title strip, clipped by IntersectRect below
            nCross = (nCrossAxis == 0) ? rcTitle.Width() : rcTitle.Height();
        }
        else
            nCross = m_szTab[nCrossAxis].toPixelSize(nScale);
        int nStrip, nOffset;
        if (m_szTab[nStripAxis].isWrapContent())
        { // wrapContent: each tab fits its own title text
            nOffset = nTabPos;
            nStrip = 0;
            for (int i = 0; i <= nIndex; i++)
            {
                int nSize = (nStripAxis == 0) ? MeasureTabWidth(pRT, i) : MeasureTabHeight(pRT, i);
                if (i == nIndex)
                    nStrip = nSize;
                else
                    nOffset += nSize + nInter;
            }
        }
        else
        { // fixed size, or matchParent: tabs evenly divide the whole strip
            nStrip = m_szTab[nStripAxis].toPixelSize(nScale);
            nOffset = nTabPos + nIndex * (nStrip + nInter);
            if (m_szTab[nStripAxis].isMatchParent())
            {
                int nStripLen = (nStripAxis == 0) ? rcTitle.Width() : rcTitle.Height();
                nStrip = smax(0, (nStripLen - nTabPos - (GetItemCount() - 1) * nInter) / GetItemCount());
                nOffset = nTabPos + nIndex * (nStrip + nInter);
            }
        }
        if (bVert)
            rcItem = CRect(rcTitle.TopLeft(), CSize(nCross, nStrip));
        else
            rcItem = CRect(rcTitle.TopLeft(), CSize(nStrip, nCross));
        if (bVert)
            rcItem.OffsetRect(0, nOffset);
        else
            rcItem.OffsetRect(nOffset, 0);
        rcItem.IntersectRect(rcItem, rcTitle);
        return TRUE;
    }
}

void STabCtrl::DrawItem(IRenderTarget *pRT, const CRect &rcItem, int iItem, DWORD dwState)
{
    if (rcItem.IsRectEmpty())
        return;
    if (m_pSkinTab)
        m_pSkinTab->DrawByState(pRT, rcItem, dwState);
    int iState = SState2Index::GetDefIndex(dwState, true);
    // Get font and color from style according to state
    IFontPtr font = m_style.GetTextFont(iState);
    COLORREF crTxt = m_style.GetTextColor(iState);
    SAutoRefPtr<IFontS> oldFont;
    if (font)
        pRT->SelectObject(font, (IRenderObj **)&oldFont);
    COLORREF crOld = 0;
    if (crTxt != CR_INVALID)
        crOld = pRT->SetTextColor(crTxt);

    CRect rcIcon(CPoint(m_ptIcon[0].toPixelSize(GetScale()), m_ptIcon[1].toPixelSize(GetScale())) + rcItem.TopLeft(), CSize(0, 0));
    if (m_pSkinIcon)
    {
        rcIcon.right = rcIcon.left + m_pSkinIcon->GetSkinSize().cx;
        rcIcon.bottom = rcIcon.top + m_pSkinIcon->GetSkinSize().cy;
        int iIcon = GetItem(iItem)->GetIconIndex();
        if (iIcon == -1)
            iIcon = iItem;
        m_pSkinIcon->DrawByIndex(pRT, rcIcon, iIcon);
    }

    if (m_ptText[0].toPixelSize(GetScale()) > 0 && m_ptText[1].toPixelSize(GetScale()) > 0)
    { // Draw text starting from the specified position
        if (m_txtDir == Text_Horz)
            pRT->TextOut(rcItem.left + m_ptText[0].toPixelSize(GetScale()), rcItem.top + m_ptText[1].toPixelSize(GetScale()), GetItem(iItem)->GetTitle(), -1);
        else
            TextOutV(pRT, rcItem.left + m_ptText[0].toPixelSize(GetScale()), rcItem.top + m_ptText[1].toPixelSize(GetScale()), GetItem(iItem)->GetTitle());
    }
    else
    {
        CRect rcText = rcItem;
        UINT alignStyle = m_style.GetTextAlign();
        UINT align = alignStyle;
        if (m_ptText[0].toPixelSize(GetScale()) < 0 && m_ptText[1].toPixelSize(GetScale()) > 0)
        { // Y offset specified, X centered
            rcText.top += m_ptText[1].toPixelSize(GetScale());
            align = alignStyle & (DT_CENTER | DT_RIGHT | DT_SINGLELINE | DT_END_ELLIPSIS);
        }
        else if (m_ptText[0].toPixelSize(GetScale()) > 0 && m_ptText[1].toPixelSize(GetScale()) < 0)
        { // X offset specified, Y centered
            rcText.left += m_ptText[0].toPixelSize(GetScale());
            align = alignStyle & (DT_VCENTER | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS);
        }

        if (m_txtDir == Text_Horz)
            pRT->DrawText(GetItem(iItem)->GetTitle(), -1, &rcText, align);
        else
            DrawTextV(pRT, rcText, GetItem(iItem)->GetTitle());
    }

    // Restore font and color
    if (font)
        pRT->SelectObject(oldFont, NULL);
    if (crTxt != CR_INVALID)
        pRT->SetTextColor(crOld);
}

BOOL STabCtrl::UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
{
    int iItem = HitTest(pt);
    if (iItem == -1)
        return FALSE;
    if (GetItem(iItem)->GetToolTipText().IsEmpty())
        return FALSE;
    tipInfo.swnd = m_swnd;
    tipInfo.dwCookie = iItem;
    GetItemRect(iItem, tipInfo.rcTarget);
    tipInfo.strTip = GetItem(iItem)->GetToolTipText();
    return TRUE;
}

void STabCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_LEFT || nChar == VK_UP)
    {
        if (!SetCurSel(m_nCurrentPage - 1))
            SetCurSel(GetItemCount() - 1);
    }
    else if (nChar == VK_RIGHT || nChar == VK_DOWN)
    {
        if (!SetCurSel(m_nCurrentPage + 1))
            SetCurSel(0);
    }
    else if (nChar == VK_HOME)
    {
        SetCurSel(0);
    }
    else if (nChar == VK_END)
    {
        SetCurSel(GetItemCount() - 1);
    }
    else
    {
        SetMsgHandled(FALSE);
    }
}

int STabCtrl::HitTest(CPoint pt)
{
    int nTabCount = GetItemCount();
    for (int i = 0; i < nTabCount; i++)
    {
        CRect rcItem;
        GetItemRect(i, rcItem);

        if (rcItem.PtInRect(pt))
        {
            return i;
        }
    }
    return -1;
}

void STabCtrl::UpdateChildrenPosition()
{
    CRect rcPage;
    GetChildrenLayoutRect(&rcPage);
    for (size_t i = 0; i < m_lstPages.GetCount(); i++)
    {
        m_lstPages[i]->Move(rcPage);
    }
}

void STabCtrl::BeforePaint(IRenderTarget *pRT, SPainter &painter) const
{
    IFontPtr pFont = m_style.GetTextFont(0);
    if (pFont)
        pRT->SelectObject(pFont, (IRenderObj **)&painter.oldFont);

    COLORREF crTxt = m_style.GetTextColor(0);
    if (crTxt != CR_INVALID)
        painter.oldTextColor = pRT->SetTextColor(crTxt);
}

void STabCtrl::TextOutV(IRenderTarget *pRT, int x, int y, const SStringT &strText)
{
    SStringT strTmp = strText;
    LPTSTR p = strTmp.GetBuffer(0);
    while (*p)
    {
        LPTSTR p2 = SStringT::_tchar_traits::CharNext(p);
        SIZE szWord;
        pRT->MeasureText(p, (int)(p2 - p), &szWord);
        pRT->TextOut(x, y, p, (int)(p2 - p));
        p = p2;
        y += szWord.cy;
    }
    strTmp.ReleaseBuffer();
}

SIZE STabCtrl::MeasureTextV(IRenderTarget *pRT, const SStringT &strText) const
{
    SIZE szRet = { 0, 0 };
    SStringT strTmp = strText;
    LPTSTR p = strTmp.GetBuffer(0);
    while (*p)
    {
        LPTSTR p2 = SStringT::_tchar_traits::CharNext(p);
        SIZE szWord;
        pRT->MeasureText(p, (int)(p2 - p), &szWord);
        szRet.cx = smax(szRet.cx, szWord.cx);
        szRet.cy += szWord.cy;
        p = p2;
    }
    strTmp.ReleaseBuffer();

    return szRet;
}

void STabCtrl::DrawTextV(IRenderTarget *pRT, CRect rcText, const SStringT &strText)
{ // only support horizontal and vertical center
    SIZE szText = MeasureTextV(pRT, strText);
    TextOutV(pRT, rcText.left + (rcText.Width() - szText.cx) / 2, rcText.top + (rcText.Height() - szText.cy) / 2, strText);
}

void STabCtrl::OnColorize(COLORREF cr)
{
    __baseCls::OnColorize(cr);
    if (m_pSkinIcon)
        m_pSkinIcon->OnColorize(cr);
    if (m_pSkinTab)
        m_pSkinTab->OnColorize(cr);
    if (m_pSkinTabInter)
        m_pSkinTabInter->OnColorize(cr);
    if (m_pSkinFrame)
        m_pSkinFrame->OnColorize(cr);
}

void STabCtrl::OnScaleChanged(int nScale)
{
    __baseCls::OnScaleChanged(nScale);
    if (m_pSkinIcon)
        GetScaleSkin(m_pSkinIcon, nScale);
    if (m_pSkinTab)
        GetScaleSkin(m_pSkinTab, nScale);
    if (m_pSkinTabInter)
        GetScaleSkin(m_pSkinTabInter, nScale);
    if (m_pSkinFrame)
        GetScaleSkin(m_pSkinFrame, nScale);
}

HRESULT STabCtrl::OnLanguageChanged()
{
    __baseCls::OnLanguageChanged();
    for (int i = 0; i < (int)m_lstPages.GetCount(); i++)
    {
        STabPage *pPage = m_lstPages.GetAt(i);
        pPage->m_strTitle.TranslateText();
    }
    InvalidateRect(GetTitleRect());
    return HRESULT(3);
}

void STabCtrl::OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer)
{
    // A gesture is bound to the container that owns the capture/timeline.
    // Abort it before the container goes away.
    if (m_bDragSwitching || m_bDragArmed)
        CancelDragSwitch();
    if (m_aniSlider->isRunning())
        m_aniSlider->end();
    __baseCls::OnContainerChanged(pOldContainer, pNewContainer);
}

int STabCtrl::GetCurSel() const
{
    return m_nCurrentPage;
}

int STabCtrl::GetItemCount(THIS) const
{
    return (int)m_lstPages.GetCount();
}

HRESULT STabCtrl::OnAttrTabAlign(const SStringW &strValue, BOOL bLoading)
{
    int nAlign = AlignTop;
    if (strValue.CompareNoCase(L"top") == 0)
        nAlign = AlignTop;
    else if (strValue.CompareNoCase(L"left") == 0)
        nAlign = AlignLeft;
    else if (strValue.CompareNoCase(L"right") == 0)
        nAlign = AlignRight;
    else if (strValue.CompareNoCase(L"bottom") == 0)
        nAlign = AlignBottom;
    else if (strValue.CompareNoCase(L"middle") == 0)
        nAlign = AlignMiddle;
    if (!bLoading)
    {
        UpdateChildrenPosition();
        return S_OK;
    }
    else
    {
        m_nTabAlign = nAlign;
        return S_FALSE;
    }
}

///////////////////////////////////////////////////////////////////////
/** === Android ViewPager-style drag-to-switch implementation ===
 * The container decides gesture ownership (OnInterceptTouchEvent), the shared
 * SGestureDetector supplies touch slop / axis arbitration / velocity, and this
 * class only maps the gesture to page selection and STabSlider's snapshots.
 */

BOOL STabCtrl::CanDragSwitch() const
{
    return m_bDragSwitchEnable && GetItemCount() > 1 && m_nCurrentPage >= 0;
}

BOOL STabCtrl::IsPointInPage(const CPoint &pt) const
{
    // Host coordinate system: GetChildrenLayoutRect() and the lParam delivered
    // by SwndContainerImpl share it (TransformPointEx does not shift the origin).
    CRect rcPage;
    GetChildrenLayoutRect(&rcPage);
    return rcPage.PtInRect(pt);
}

void STabCtrl::PrepareGesture()
{
    m_gesture.SetAxis(IsDragAxisVertical() ? SGestureDetector::AxisVertical : SGestureDetector::AxisHorizontal);
    m_gesture.SetTouchSlop(m_nDragThreshold.toPixelSize(GetScale()));
}

BOOL STabCtrl::IsDragAxisVertical() const
{
    // Same axis as STabSlider::m_bVertical, so the transition follows the finger.
    return m_nTabAlign == AlignLeft || m_nTabAlign == AlignRight;
}

int STabCtrl::GetTouchInterceptAxis() const
{
    // Declaring our own axis keeps CancelCaptureMode()'s reason correct even
    // when the drag is diagonal or changes direction mid-gesture.
    return IsDragAxisVertical() ? CANCEL_REASON_SCROLL_V : CANCEL_REASON_SCROLL_H;
}

int STabCtrl::GetDragPageSize() const
{
    CRect rcPage;
    GetChildrenLayoutRect(&rcPage);
    return IsDragAxisVertical() ? rcPage.Height() : rcPage.Width();
}

BOOL STabCtrl::OnInterceptTouchEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!CanDragSwitch())
        return FALSE;
    if (uMsg == WM_LBUTTONDOWN)
    {
        // Never arm a gesture on the tab header: tapping a tab keeps its
        // "select this page" behaviour.
        m_bDragArmed = IsPointInPage(CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
        if (m_bDragArmed)
            PrepareGesture();
        else
            m_gesture.Reset();
    }
    if (!m_bDragArmed)
        return FALSE;
    if (!m_gesture.OnInterceptTouchEvent(uMsg, wParam, lParam))
        return FALSE;
    if (uMsg == WM_MOUSEMOVE)
    {
        // ViewPager's "canScroll at the edge" rule: a boundary drag must not be
        // claimed here. Otherwise the container has already taken capture by the
        // time StartDragSwitch() finds there is no neighbour page, and the whole
        // gesture is swallowed with no visual feedback. Declining lets the next
        // candidate outwards (an enclosing tabctrl) take it instead.
        CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        const CPoint &ptDown = m_gesture.GetDownPoint();
        int nDelta = IsDragAxisVertical() ? (pt.y - ptDown.y) : (pt.x - ptDown.x);
        // Same direction rule as StartDragSwitch(): left/up -> next, right/down -> previous.
        int iTo = nDelta < 0 ? m_nCurrentPage + 1 : m_nCurrentPage - 1;
        if (iTo < 0 || iTo >= GetItemCount())
            return FALSE;
    }
    return TRUE;
}

BOOL STabCtrl::OnTouchEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!CanDragSwitch() || !m_bDragArmed)
        return FALSE;
    BOOL bRet = m_gesture.OnTouchEvent(uMsg, wParam, lParam);
    if (uMsg == WM_LBUTTONUP || uMsg == WM_CANCELMODE)
    {
        m_bDragArmed = FALSE;
        m_gesture.Reset();
    }
    return bRet;
}

void STabCtrl::OnGestureScroll(const CPoint &ptDown, const CPoint &ptCur, int, int)
{
    if (!m_bDragSwitching)
        StartDragSwitch(ptDown, ptCur);
    if (m_bDragSwitching)
        UpdateDragSwitch(ptDown, ptCur);
}

void STabCtrl::OnGestureFling(float, float)
{
    m_bDragFling = TRUE;
}

void STabCtrl::OnGestureUp(const CPoint &, float fVelocityX, float fVelocityY)
{
    EndDragSwitch(IsDragAxisVertical() ? fVelocityY : fVelocityX);
}

void STabCtrl::OnGestureCancel()
{
    CancelDragSwitch();
}

void STabCtrl::StartDragSwitch(const CPoint &ptDown, const CPoint &ptCur)
{
    m_nDragFrom = m_nCurrentPage;
    int nDelta = IsDragAxisVertical() ? (ptCur.y - ptDown.y) : (ptCur.x - ptDown.x);
    // Drag left/up -> next page, drag right/down -> previous page.
    int iTo = nDelta < 0 ? m_nDragFrom + 1 : m_nDragFrom - 1;
    if (iTo < 0 || iTo >= GetItemCount())
        return; // boundary: no neighbour to reveal, leave the gesture inactive

    // Capture ownership was already settled by SwndContainerImpl::DispatchTouchEvent
    // (CancelCaptureMode + WM_CANCELMODE + SetCapture). Only visual setup remains.
    if (m_aniSlider->isRunning())
        m_aniSlider->end();
    if (m_tabSlider)
        m_tabSlider->Stop();

    m_nDragSavedDuration = m_aniSlider->getDuration();
    m_nDragTo = iTo;
    m_bDragSwitching = TRUE;
    m_bDragFling = FALSE;
    m_tabSlider = new STabSlider(this, m_nDragFrom, iTo, TRUE);
    UpdateDragSwitch(ptDown, ptCur);
}

void STabCtrl::UpdateDragSwitch(const CPoint &ptDown, const CPoint &ptCur)
{
    int nAxis = IsDragAxisVertical() ? (ptCur.y - ptDown.y) : (ptCur.x - ptDown.x);
    // Progress is positive only while moving towards the target page.
    int nProgress = (m_nDragTo > m_nDragFrom) ? -nAxis : nAxis;
    if (nProgress < 0)
        nProgress = 0;
    int nPage = GetDragPageSize();
    float f = nPage > 0 ? (float)nProgress / (float)nPage : 0.f;
    if (f > 1.f)
        f = 1.f;
    if (m_tabSlider)
        m_tabSlider->SetDragFraction(f);
}

void STabCtrl::EndDragSwitch(float fAxisVelocity)
{
    if (!m_bDragSwitching)
    {
        m_bDragFling = FALSE;
        return;
    }
    m_bDragSwitching = FALSE;

    float f = m_tabSlider ? m_tabSlider->GetDragFraction() : 0.f;
    float fVelocity = (m_nDragTo > m_nDragFrom) ? -fAxisVelocity : fAxisVelocity;
    int nRatio = m_nDragSwitchRatio;
    if (nRatio < 1)
        nRatio = 1;
    if (nRatio > 100)
        nRatio = 100;

    // ViewPager.determineTargetPage(): distance ratio first, then fling velocity.
    BOOL bSwitch = (f * 100.f >= (float)nRatio);
    if (!bSwitch && m_bDragFlingEnable && m_bDragFling && fVelocity > 0.5f && f > 0.02f)
        bSwitch = TRUE;

    if (bSwitch)
    {
        EventTabSelChanging evt(this);
        evt.bCancel = FALSE;
        evt.uOldSel = m_nCurrentPage;
        evt.uNewSel = m_nDragTo;
        FireEvent(evt);
        if (evt.bCancel)
            bSwitch = FALSE;
    }

    int nOld = m_nCurrentPage;
    if (bSwitch)
    {
        m_nCurrentPage = m_nDragTo;
        UpdateChildrenPosition();
        CRect rcItem;
        GetItemRect(nOld, rcItem);
        InvalidateRect(rcItem);
        GetItemRect(m_nCurrentPage, rcItem);
        InvalidateRect(rcItem);
    }

    if (m_tabSlider)
    {
        // OnSliderFinish() restores the original animator duration.
        m_bDragCommitting = TRUE;
        m_tabSlider->CommitDrag(bSwitch, m_nDragCommitDuration);
    }
    else if (m_bDragCommitting)
    {
        m_bDragCommitting = FALSE;
        m_aniSlider->setDuration(m_nDragSavedDuration);
    }

    // SelectionChanged must be delivered AFTER the drag commit. Subscribers
    // (e.g. fundholdings' three-page "swipe to next fund" container) re-enter
    // SetCurSel() to reset the baseline page; firing the event earlier stops
    // and destroys the drag slider before CommitDrag(), swallowing the
    // follow-the-finger animation and leaving m_bDragCommitting inconsistent.
    // This also matches SetCurSel()'s "build the slider, then notify" ordering.
    if (bSwitch)
    {
        EventTabSelChanged evt2(this);
        evt2.uNewSel = m_nCurrentPage;
        evt2.uOldSel = nOld;
        FireEvent(evt2);
        accNotifyEvent(EVENT_OBJECT_SELECTION);
    }
    m_bDragFling = FALSE;
    m_nDragFrom = -1;
    m_nDragTo = -1;
}

void STabCtrl::CancelDragSwitch()
{
    m_bDragSwitching = FALSE;
    m_bDragArmed = FALSE;
    m_bDragFling = FALSE;
    m_nDragFrom = -1;
    m_nDragTo = -1;
    m_gesture.Reset();
    if (m_bDragCommitting)
    {
        m_bDragCommitting = FALSE;
        m_aniSlider->setDuration(m_nDragSavedDuration);
    }
    if (m_aniSlider->isRunning())
        m_aniSlider->end();
    if (m_tabSlider)
        m_tabSlider->Stop();
}

SNSEND
