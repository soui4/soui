#include "souistd.h"
#include "control/STabCtrl.h"
#include "animation/SInterpolatorImpl.h"

SNSBEGIN

class STabSlider
    : public SWindow
    , public ITimelineHandler {
    DEF_SOBJECT(SWindow, L"tabslider")

  public:
    STabSlider(STabCtrl *pTabCtrl, int iFrom, int iTo, int nSteps, int nType, IInterpolator *pInterpolator)
        : m_pTabCtrl(pTabCtrl)
        , m_aniInterpoloator(pInterpolator)
        , m_nSteps(nSteps)
        , m_iStep(0)
        , m_iFrom(iFrom)
        , m_iTo(iTo)
    {
        SASSERT(pTabCtrl);
        SASSERT(pInterpolator);
        m_bClipClient = TRUE;

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
        // pBmp->Save2(L"d:/pagefrom.png",Img_PNG);

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
        // pBmp->Save2(L"d:/pageTo.png",Img_PNG);
        // hide page from and page to
        pageFrom->SetVisible(FALSE);
        pageTo->SetVisible(FALSE);

        GetContainer()->RegisterTimelineHandler(this);
        SetVisible(TRUE, TRUE);
    }

    virtual ~STabSlider()
    {
    }

    STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE
    {
        if (++m_iStep > m_nSteps)
        {
            Stop();
        }
        else
        {
            InvalidateRect(NULL);
        }
    }

    void Stop()
    {
        m_pTabCtrl->OnSliderFinish();
    }

  protected:
    virtual void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer)
    {
        if (pOldContainer)
            pOldContainer->UnregisterTimelineHandler(this);
        if (pNewContainer)
            pNewContainer->RegisterTimelineHandler(this);
        SWindow::OnContainerChanged(pOldContainer, pNewContainer);
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
        float fraction = m_aniInterpoloator->getInterpolation(m_iStep / (float)m_nSteps);
        if (m_pTabCtrl->m_nTabAlign == STabCtrl::AlignMiddle)
        {
            int nHeight = rcWnd.Height();
            int nTabHeight = m_pTabCtrl->m_szTab[1].toPixelSize(m_pTabCtrl->GetScale());
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

    void OnDestroy()
    {
        GetContainer()->UnregisterTimelineHandler(this);
        SWindow::OnDestroy();
    }

    SAutoRefPtr<IRenderTarget> m_rtPageFrom, m_rtPageTo;
    int m_iFrom, m_iTo;
    int m_nSteps;
    int m_iStep;
    bool m_bVertical;
    SAutoRefPtr<IInterpolator> m_aniInterpoloator;
    STabCtrl *m_pTabCtrl;
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SIZE(OnSize)
        MSG_WM_DESTROY(OnDestroy)
    SOUI_MSG_MAP_END()
};

//////////////////////////////////////////////////////////////////////////
// STabCtrl

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
    , m_nAnimateSteps(0)
    , m_tabSlider(NULL)
    , m_txtDir(Text_Horz)
    , m_nAniamteType(0)
{
    m_ptText[0] = m_ptText[1] = SLayoutSize(-1.f, px);
    m_szTab[0] = m_szTab[1] = SLayoutSize(-1.f, px);

    m_bFocusable = TRUE;
    // create a linear animation interpolator
    m_aniInterpolator.Attach(SApplication::getSingleton().CreateInterpolatorByName(SLinearInterpolator::GetClassName()));

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

            GetItemRect(i, rcItem);
            if (rcItem.IsRectEmpty())
                continue;

            //画分隔线
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
    int nTabWidth = m_szTab[0].toPixelSize(GetScale());
    int nTabHeight = m_szTab[1].toPixelSize(GetScale());
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
}

BOOL STabCtrl::RemoveItem(int nIndex, int nSelPage /*=0*/)
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
}

void STabCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
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

STabPage *STabCtrl::GetPage(LPCTSTR pszName, BOOL bTitle /*=TRUE*/)
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
    SASSERT(m_tabSlider);
    DestroyChild(m_tabSlider);
    m_tabSlider = NULL;
    STabPage *pPage = m_lstPages[m_nCurrentPage];
    SASSERT(pPage);
    pPage->SetVisible(TRUE, TRUE);
}

BOOL STabCtrl::SetCurSel(int nIndex)
{
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
    if (m_nAnimateSteps && IsVisible(TRUE) && nOldPage != -1 && nIndex != -1)
    {
        m_tabSlider = new STabSlider(this, nOldPage, nIndex, m_nAnimateSteps, m_nAniamteType, m_aniInterpolator);
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
    return TRUE;
}

BOOL STabCtrl::SetCurSel(LPCTSTR pszName, BOOL bTitle /*=TRUE */)
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

int STabCtrl::InsertItem(LPCWSTR lpContent, int iInsert /*=-1*/)
{
    SXmlDoc xmlDoc;
    if (!xmlDoc.load_buffer(lpContent, wcslen(lpContent) * sizeof(wchar_t), xml_parse_default, sizeof(wchar_t) == 2 ? enc_utf16 : enc_utf32))
        return -1;
    return InsertItem(xmlDoc.root().first_child(), iInsert);
}

int STabCtrl::InsertItem(SXmlNode xmlNode, int iInsert /*=-1*/, BOOL bLoading /*=FALSE*/)
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
    return iInsert;
}

CRect STabCtrl::GetTitleRect()
{
    CRect rcTitle;
    GetClientRect(rcTitle);
    switch (m_nTabAlign)
    {
    case AlignTop:
        rcTitle.bottom = rcTitle.top + m_szTab[1].toPixelSize(GetScale());
        break;
    case AlignBottom:
        rcTitle.top = rcTitle.bottom - m_szTab[1].toPixelSize(GetScale());
        break;
    case AlignLeft:
        rcTitle.right = rcTitle.left + m_szTab[0].toPixelSize(GetScale());
        break;
    case AlignRight:
        rcTitle.left = rcTitle.right - m_szTab[0].toPixelSize(GetScale());
        break;
    }
    return rcTitle;
}

BOOL STabCtrl::GetItemRect(int nIndex, CRect &rcItem)
{
    if (nIndex < 0 || nIndex >= (int)GetItemCount())
        return FALSE;
    if (m_nTabAlign == AlignMiddle)
    {
        // middle, each tab is same height
        CRect rcClient = GetClientRect();
        int nTabHeight = m_szTab[1].toPixelSize(GetScale());
        rcItem = CRect(0, 0, rcClient.Width(), nTabHeight);
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
        CRect rcTitle = GetTitleRect();

        rcItem = CRect(rcTitle.TopLeft(), CSize(m_szTab[0].toPixelSize(GetScale()), m_szTab[1].toPixelSize(GetScale())));

        switch (m_nTabAlign)
        {
        case AlignTop:
        case AlignBottom:
            rcItem.OffsetRect(m_nTabPos.toPixelSize(GetScale()) + nIndex * (rcItem.Width() + m_nTabInterSize.toPixelSize(GetScale())), 0);
            break;
        case AlignLeft:
        case AlignRight:
            rcItem.OffsetRect(0, m_nTabPos.toPixelSize(GetScale()) + nIndex * (rcItem.Height() + m_nTabInterSize.toPixelSize(GetScale())));
            break;
        }
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
    //根据状态从style中获得字体，颜色
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
    { //从指定位置开始绘制文字
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
        { //指定了Y偏移，X居中
            rcText.top += m_ptText[1].toPixelSize(GetScale());
            align = alignStyle & (DT_CENTER | DT_RIGHT | DT_SINGLELINE | DT_END_ELLIPSIS);
        }
        else if (m_ptText[0].toPixelSize(GetScale()) > 0 && m_ptText[1].toPixelSize(GetScale()) < 0)
        { //指定了X偏移，Y居中
            rcText.left += m_ptText[0].toPixelSize(GetScale());
            align = alignStyle & (DT_VCENTER | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS);
        }

        if (m_txtDir == Text_Horz)
            pRT->DrawText(GetItem(iItem)->GetTitle(), -1, &rcText, align);
        else
            DrawTextV(pRT, rcText, GetItem(iItem)->GetTitle());
    }

    //恢复字体，颜色
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

void STabCtrl::OnInitFinished(THIS_ IXmlNode *xmlNode)
{
    if (m_pSkinTab)
    {
        SIZE sz = m_pSkinTab->GetSkinSize();
        if (SLayoutSize::fequal(m_szTab[0].fSize, -1.f))
            m_szTab[0] = SLayoutSize((float)sz.cx, SLayoutSize::defUnit);
        if (SLayoutSize::fequal(m_szTab[1].fSize, -1.f))
            m_szTab[1] = SLayoutSize((float)sz.cy, SLayoutSize::defUnit);
    }
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

void STabCtrl::BeforePaint(IRenderTarget *pRT, SPainter &painter)
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

SIZE STabCtrl::MeasureTextV(IRenderTarget *pRT, const SStringT &strText)
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

int STabCtrl::GetCurSel() const
{
    return m_nCurrentPage;
}

int STabCtrl::GetItemCount(THIS) const
{
    return (int)m_lstPages.GetCount();
}

LRESULT STabCtrl::OnAttrTabAlign(const SStringW &strValue, BOOL bLoading)
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

SNSEND