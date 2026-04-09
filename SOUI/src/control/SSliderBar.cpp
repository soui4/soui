#include "souistd.h"
#include "control/SSliderBar.h"
#include "animation/SInterpolatorImpl.h"
SNSBEGIN

#define TIMERID_NOTIFY1 1
#define TIMERID_DELAY1  2

//////////////////////////////////////////////////////////////////////////
//  SSliderBar
SSliderBar::SSliderBar()
    : m_bDrag(FALSE)
    , m_uHtPrev(-1)
    , m_pSkinThumb(GETBUILTINSKIN(SKIN_SYS_SLIDER_THUMB))
    , m_bThumbInRail(FALSE)
    , m_bDrawRail(TRUE)
    , m_bDragTip(FALSE)
    , m_byThumbAlphaAni(0xFF)
{
    m_evtSet.addEvent(EVENTID(EventSliderPos));
    m_evtSet.addEvent(EVENTID(EventSliderValueTip));
    m_thumbAni.Attach(new SByteAnimator);
    m_thumbAni->addUpdateListener(this);
}

SSliderBar::~SSliderBar()
{
}

int SSliderBar::HitTest(CPoint pt)
{
    CRect rc;
    CRect rcClient = GetClientRect();
    rc = GetPartRect(rcClient,SC_THUMB);
    if (rc.PtInRect(pt))
        return SC_THUMB;

    rc = GetPartRect(rcClient,SC_SELECT);
    if (rc.PtInRect(pt))
        return SC_SELECT;

    rc = GetPartRect(rcClient,SC_RAIL);
    if (rc.PtInRect(pt))
        return SC_RAIL;

    return -1;
}

SSliderBar::RANGE SSliderBar::_GetPartRange(int nLength, int nThumbSize, BOOL bThumbInRail, int nMin, int nMax, int nValue, UINT uSBCode) const
{
    int64_t nRailLen = nLength - nThumbSize;

    int nHalfThumb = nThumbSize / 2;
    int64_t nSelect = nRailLen * (nValue - nMin) / (nMax - nMin);

    RANGE rRet = { 0 };
    switch (uSBCode)
    {
    case SC_RAILBACK: //轨道背景
        rRet.value1 = 0;
        rRet.value2 = nLength;
        if (!bThumbInRail)
        {
            rRet.value1 += nHalfThumb;
            rRet.value2 -= nHalfThumb;
        }
        break;
    case SC_RAIL: //轨道区
        rRet.value1 = 0;
        rRet.value2 = nLength;
        rRet.value1 += nHalfThumb;
        rRet.value2 -= nHalfThumb;
        break;
    case SC_SELECT:
        rRet.value1 = 0;
        rRet.value2 = (int)nSelect;
        if (!bThumbInRail)
        {
            rRet.value1 += nHalfThumb;
        }
        rRet.value2 += nHalfThumb;
        break;
    case SC_THUMB:
        rRet.value1 = (int)nSelect;
        rRet.value2 = (int)nSelect + nThumbSize;
        break;
    }

    return rRet;
}

CRect SSliderBar::GetPartRect(UINT uSBCode) const
{
    CRect rcClient = GetClientRect();
    return GetPartRect(rcClient, uSBCode);
}

CRect SSliderBar::GetPartRect(const CRect &rcClient, UINT uSBCode) const
{
    int nValue = GetValue();
    SIZE szThumb = { 0 };
    SIZE szRail = m_pSkinBg->GetSkinSize();
    if (m_pSkinThumb)
        szThumb = m_pSkinThumb->GetSkinSize();
    if (IsVertical())
    {
        RANGE r = _GetPartRange(rcClient.Height(), szThumb.cy, m_bThumbInRail, m_nMinValue, m_nMaxValue, nValue, uSBCode);
        CRect rc(rcClient.left, rcClient.Height() - r.value2, rcClient.right, rcClient.Height() - r.value1);
        rc.OffsetRect(0, rcClient.top);
        int nSliderSize = smax(szThumb.cx, szRail.cx);
        rc.DeflateRect((rcClient.Width() - nSliderSize) / 2, 0);
        if (uSBCode != SC_THUMB)
        {
            rc.DeflateRect((nSliderSize - szRail.cx) / 2, 0);
        }
        return rc;
    }
    else
    {
        RANGE r = _GetPartRange(rcClient.Width(), szThumb.cx, m_bThumbInRail, m_nMinValue, m_nMaxValue, nValue, uSBCode);
        CRect rc(r.value1, rcClient.top, r.value2, rcClient.bottom);
        rc.OffsetRect(rcClient.left, 0);
        int nSliderSize = smax(szThumb.cy, szRail.cy);
        rc.DeflateRect(0, (rcClient.Height() - nSliderSize) / 2);
        if (uSBCode != SC_THUMB)
        {
            rc.DeflateRect(0, (nSliderSize - szRail.cy) / 2);
        }
        return rc;
    }
}

void SSliderBar::DrawPos(IRenderTarget *pRT, const CRect& rcClient){
    if(!m_bDrawRail) 
        return;
    __baseCls::DrawPos(pRT,rcClient);
}
void SSliderBar::DrawExtend(IRenderTarget *pRT, const CRect& rcClient){
   if (!m_pSkinThumb)
        return;
    {
       CRect rcThumb = GetPartRect(rcClient,SC_THUMB);
        int nState = 0; // normal
        if (IsDisabled(TRUE) && m_pSkinThumb->GetStates() > 3)
            nState = 3;
        else if (m_bDrag)
            nState = 2; // pushback
        else if (m_uHtPrev == SC_THUMB)
            nState = 1; // hover
        
        if (m_byThumbAlphaAni == 0xFF)
        { //不在动画过程中
            m_pSkinThumb->DrawByIndex(pRT, rcThumb, nState);
        }
        else
        { //在动画过程中
            BYTE byNewAlpha = (BYTE)(((UINT)m_byThumbAlphaAni * m_pSkinThumb->GetAlpha()) >> 8);
            if (m_uHtPrev == SC_THUMB)
            {
                // enter hover
                m_pSkinThumb->DrawByIndex2(pRT, rcThumb, 0, m_pSkinThumb->GetAlpha());
                m_pSkinThumb->DrawByIndex2(pRT, rcThumb, 1, byNewAlpha);
            }
            else
            {
                // leave hover
                m_pSkinThumb->DrawByIndex2(pRT, rcThumb, 0, m_pSkinThumb->GetAlpha());
                m_pSkinThumb->DrawByIndex2(pRT, rcThumb, 1, m_pSkinThumb->GetAlpha() - byNewAlpha);
            }
        }
    }
}

void SSliderBar::OnLButtonUp(UINT nFlags, CPoint point)
{
    ReleaseCapture();
    if (m_bDrag)
    {
        m_bDrag = FALSE;
        CRect rcThumb = GetPartRect(SC_THUMB);
        InvalidateRect(rcThumb);
        NotifyPos(SBA_MOUSE_UP, m_nValue);
        if (m_bDragTip)
        {
            // hide tooltip
            GetContainer()->SetToolTip(NULL, TA_AUTO, NULL);
        }
    }
    OnMouseMove(nFlags, point);
}

void SSliderBar::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetCapture();

    UINT uHit = HitTest(point);
    if (uHit == SC_THUMB)
    {
        m_bDrag = TRUE;
        m_ptDrag = point;
        m_nDragValue = m_nValue;
        Invalidate();
        ShowValueInTip(m_nValue);
    }
    else
    {
        CRect rcRail = GetPartRect(SC_RAIL);
        int nValue = 0;
        if (IsVertical())
        {
            nValue = ((int64_t)(rcRail.bottom - point.y)) * (m_nMaxValue - m_nMinValue + 1) / rcRail.Height() + m_nMinValue;
        }
        else
        {
            nValue = ((int64_t)(point.x - rcRail.left)) * (m_nMaxValue - m_nMinValue + 1) / rcRail.Width() + m_nMinValue;
        }
        SetValue(nValue);
        Invalidate();

        m_bDrag = TRUE;
        m_ptDrag = point;
        m_nDragValue = m_nValue;

        NotifyPos(SBA_MOUSE_DOWN, m_nValue);
    }
}

void SSliderBar::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_bDrag)
    {
        CRect rcRail = GetPartRect(SC_RAIL);

        int nInterHei = (IsVertical() ? rcRail.Height() : rcRail.Width());
        int nDragLen = IsVertical() ? (m_ptDrag.y - point.y) : (point.x - m_ptDrag.x);
        int nSlide = nDragLen * (m_nMaxValue - m_nMinValue + 1) / nInterHei;

        int nNewTrackPos = m_nDragValue + nSlide;
        if (nNewTrackPos < m_nMinValue)
        {
            nNewTrackPos = m_nMinValue;
        }
        else if (nNewTrackPos > m_nMaxValue)
        {
            nNewTrackPos = m_nMaxValue;
        }
        if (nNewTrackPos != m_nValue)
        {
            m_nValue = nNewTrackPos;
            Invalidate();
            NotifyPos(SBA_MOUSE_MOVING, m_nValue);
            ShowValueInTip(nNewTrackPos);
        }
    }
    else
    {
        int uHit = HitTest(point);
        if (uHit != m_uHtPrev && (m_uHtPrev == SC_THUMB || uHit == SC_THUMB))
        {
            // 悬停状态改变，启动动画
            m_uHtPrev = uHit;
            m_thumbAni->setRange(50, 255);
            m_thumbAni->start(GetContainer());
        }
    }
}

void SSliderBar::OnMouseLeave()
{
    if (!m_bDrag && m_uHtPrev == SC_THUMB)
    {
        // 悬停状态改变，启动动画
        m_uHtPrev = -1;
        m_thumbAni->setRange(50, 255);
        m_thumbAni->start(GetContainer());
    }
}

LRESULT SSliderBar::NotifyPos(SliderBarAction action, int nPos)
{
    EventSliderPos evt(this);
    evt.nPos = nPos;
    evt.action = action;
    return FireEvent(evt);
}

SIZE SSliderBar::GetDesiredSize(int wid, int hei)
{
    if (!m_pSkinBg)
        return CSize();
    CSize szRet;
    SIZE sizeBg = m_pSkinBg->GetSkinSize();
    SIZE sizeThumb = { 0 };
    if (m_pSkinThumb)
        sizeThumb = m_pSkinThumb->GetSkinSize();

    if (IsVertical())
    {
        szRet.cx = smax(sizeBg.cx, sizeThumb.cx);
        szRet.cy = 100;
    }
    else
    {
        szRet.cy = smax(sizeBg.cy, sizeThumb.cy);
        szRet.cx = 100;
    }
    return szRet;
}

void SSliderBar::OnColorize(COLORREF cr)
{
    __baseCls::OnColorize(cr);
    if (m_pSkinThumb)
        m_pSkinThumb->OnColorize(cr);
}

void SSliderBar::OnScaleChanged(int scale)
{
    __baseCls::OnScaleChanged(scale);
    GetScaleSkin(m_pSkinThumb, scale);
}

void SSliderBar::OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer){
    if(pOldContainer){
        m_thumbAni->end();
    }
    __baseCls::OnContainerChanged(pOldContainer,pNewContainer);
}
void SSliderBar::OnSetAnimateStep(int nStep){
    __baseCls::OnSetAnimateStep(nStep);
    m_thumbAni->setDuration(nStep*10);
}

BOOL SSliderBar::SetValue(THIS_ int nValue)
{
    if (m_bDrag)
        return FALSE;
    
    // 调用基类实现，基类现在处理动画
    return __baseCls::SetValue(nValue);
}

void SSliderBar::ShowValueInTip(int nValue)
{
    if (m_bDragTip)
    {
        CRect rcThumb = GetPartRect(SC_THUMB);
        rcThumb.InflateRect(5, 5);
        SStringT buf = SStringT().Format(_T("%d"), nValue);
        EventSliderValueTip evt(this);
        evt.nPos = nValue;
        evt.buf = &buf;
        FireEvent(&evt);
        UINT tipAlign = IsVertical() ? (TA_X_RIGHT | TA_Y_CENTER) : (TA_X_CENTER | TA_Y_TOP);
        GetContainer()->SetToolTip(&rcThumb, tipAlign, buf);
    }
}

void SSliderBar::onAnimationUpdate(IValueAnimator *pAnimator){
    __baseCls::onAnimationUpdate(pAnimator);
    if(pAnimator == m_thumbAni){
        BYTE byAlpha = m_thumbAni->getValue();
        if(byAlpha == m_byThumbAlphaAni)
            return;
        m_byThumbAlphaAni = byAlpha;
        CRect rcThumb = GetPartRect(SC_THUMB);
        InvalidateRect(rcThumb);
    }
}

void SSliderBar::OnDestroy()
{
    m_thumbAni->end();
    __baseCls::OnDestroy();
}

SNSEND