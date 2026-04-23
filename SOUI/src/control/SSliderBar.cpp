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
    , m_bDrawValue(TRUE)
    , m_bDragTip(FALSE)
    , m_crSep(RGBA(0, 0, 0, 255))
    , m_byThumbAlphaAni(0xFF)
    , m_bEnableAnimate(TRUE)
{
    m_evtSet.addEvent(EVENTID(EventSliderPos));
    m_evtSet.addEvent(EVENTID(EventSliderValueTip));
    m_thumbAni.Attach(new SByteAnimator);
    m_thumbAni->addUpdateListener(this);
    m_thumbAni->setDuration(200);
    m_valueAni.Attach(new SIntAnimator());
    m_valueAni->setDuration(100);
    m_valueAni->addUpdateListener(this);
    m_valueAni->addListener(this);
}

SSliderBar::~SSliderBar()
{
}

int SSliderBar::HitTest(CPoint pt)
{
    CRect rc;
    CRect rcClient = GetClientRect();
    rc = GetPartRect(rcClient, SC_THUMB);
    if (rc.PtInRect(pt))
        return SC_THUMB;

    rc = GetPartRect(rcClient, SC_SELECT);
    if (rc.PtInRect(pt))
        return SC_SELECT;

    rc = GetPartRect(rcClient, SC_RAIL);
    if (rc.PtInRect(pt))
        return SC_RAIL;

    return SC_NULL;
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
        if (bThumbInRail)
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

void SSliderBar::DrawRail(IRenderTarget *pRT, const CRect &rcClient)
{
    if (!m_bDrawRail)
        return;
    __baseCls::DrawRail(pRT, rcClient);
}

void SSliderBar::DrawPos(IRenderTarget *pRT, const CRect &rcClient)
{
    if (!m_bDrawValue)
        return;
    __baseCls::DrawPos(pRT, rcClient);
}
void SSliderBar::DrawOthers(IRenderTarget *pRT, const CRect &rcClient)
{
    if (!m_segments.IsEmpty())
    {
        SIZE szThumb = { 0 };
        if (m_pSkinThumb)
            szThumb = m_pSkinThumb->GetSkinSize();
        int length = IsVertical() ? rcClient.Height() : rcClient.Width();
        int thumbSize = IsVertical() ? szThumb.cy : szThumb.cx;
        CRect rcRail = GetPartRect(rcClient, SC_RAIL);
        CRect rcSeg = rcRail;
        if (IsVertical())
            rcSeg.top = rcSeg.bottom;
        else
            rcSeg.right = rcSeg.left;
        SAutoRefPtr<IPenS> pen;
        SAutoRefPtr<IRenderObj> oldPen;
        pRT->CreatePen(PS_SOLID, m_crSep, 1, (IPenS **)&pen);
        pRT->SelectObject(pen, &oldPen);
        for (int i = 0; i < m_segments.GetCount(); i++)
        {
            SEGMENT &segment = m_segments.GetAt(i);
            RANGE r = _GetPartRange(length, thumbSize, FALSE, m_nMinValue, m_nMaxValue, segment.value, SC_SELECT);
            if (IsVertical())
            {
                rcSeg.bottom = rcSeg.top;
                rcSeg.top = rcRail.bottom - r.value2;
                pRT->DrawLine(rcSeg.TopLeft(), CPoint(rcSeg.right, rcSeg.top));
            }
            else
            {
                rcSeg.left = rcSeg.right;
                rcSeg.right = rcRail.left + r.value2;
                pRT->DrawLine(CPoint(rcSeg.right, rcSeg.top), rcSeg.BottomRight());
                if (m_bShowText)
                {
                    pRT->DrawText(segment.szDesc, -1, rcSeg, DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);
                }
            }
        }
        pRT->SelectObject(oldPen);
    }
    if (!m_pSkinThumb)
        return;
    CRect rcThumb = GetPartRect(rcClient, SC_THUMB);
    DWORD dwState = GetState();
    if((dwState & WndState_PushDown) && m_uHtPrev != SC_THUMB){
        dwState &= ~WndState_PushDown;
    }
    if (m_byThumbAlphaAni == 0xFF)
    { // 不在动画过程中
        m_pSkinThumb->DrawByState(pRT, rcThumb, dwState);
    }
    else
    { // 在动画过程中
        BYTE byAlpha = m_byThumbAlphaAni;
        if(GetState()&WndState_Hover)
        {
            // enter hover
            m_pSkinThumb->DrawByIndex2(pRT, rcThumb, 0, 255 - byAlpha);
            m_pSkinThumb->DrawByIndex2(pRT, rcThumb, 1, byAlpha);
        }
        else
        {
            // leave hover
            m_pSkinThumb->DrawByIndex2(pRT, rcThumb, 1, 255 - byAlpha);
            m_pSkinThumb->DrawByIndex2(pRT, rcThumb, 0, byAlpha);
        }
    }
}

void SSliderBar::OnLButtonUp(UINT nFlags, CPoint point)
{
    ReleaseCapture();
    if (m_bDrag)
    {
        m_bDrag = FALSE;
        CRect rcThumb = GetPartRect2(SC_THUMB);
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
        CRect rcRail = GetPartRect2(PC_RAIL);
        int nValue = 0;
        if (IsVertical())
        {
            nValue = ((int64_t)(rcRail.bottom - point.y)) * (m_nMaxValue - m_nMinValue + 1) / rcRail.Height() + m_nMinValue;
        }
        else
        {
            nValue = ((int64_t)(point.x - rcRail.left)) * (m_nMaxValue - m_nMinValue + 1) / rcRail.Width() + m_nMinValue;
        }

        // 处理分段逻辑
        if (m_segments.GetCount() > 0)
        {
            int iCurSegment = FindSegmentByValue(m_nValue);
            int nClickSegment = FindSegmentByValue(nValue);
            if (nClickSegment != iCurSegment)
            {
                nValue = m_segments[nClickSegment].value;
            }
        }

        if (m_bEnableAnimate)
        {
            if (m_valueAni->isRunning())
                m_valueAni->end();
            m_valueAni->setRange(m_nValue, nValue);
            // 注册到时间轴处理器并启动动画
            m_valueAni->start(GetContainer());
        }
        else
        {
            OnValueChanged(nValue, SBA_MOUSE_DOWN);
        }
    }
}

void SSliderBar::OnMouseMove(UINT nFlags, CPoint point)
{
    __baseCls::OnMouseMove(nFlags, point);
    if (m_bDrag)
    {
        CRect rcRail = GetPartRect2(SC_RAIL);

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
            OnValueChanged(nNewTrackPos, SBA_MOUSE_MOVING);
            ShowValueInTip(nNewTrackPos);
        }
    }
}

void SSliderBar::OnMouseHover(UINT uHitTest, CPoint point)
{
    __baseCls::OnMouseHover(uHitTest, point);
    if (m_bDrag)
        return;
    m_uHtPrev = HitTest(point);
    // 悬停状态改变，启动动画
    if(m_byThumbAlphaAni == 255){
        m_thumbAni->setRange(50, 255);
    }else{
        m_thumbAni->setRange(m_byThumbAlphaAni, 255);
    }
    m_thumbAni->start(GetContainer());
}

void SSliderBar::OnMouseLeave()
{
    __baseCls::OnMouseLeave();
    if (m_bDrag)
        return;
    // 悬停状态改变，启动动画
    m_uHtPrev = SC_NULL;
    if(m_byThumbAlphaAni == 255){
        m_thumbAni->setRange(50, 255);
    }else{
        m_thumbAni->setRange(m_byThumbAlphaAni, 255);
    }
    m_thumbAni->start(GetContainer());
}

void SSliderBar::OnValueChanged(int nValue, int reason)
{
    __baseCls::OnValueChanged(nValue, reason);
    if (reason != -1)
    {
        NotifyPos((SliderBarAction)reason, nValue);
    }
}
LRESULT SSliderBar::NotifyPos(SliderBarAction action, int nPos)
{
    EventSliderPos evt(this);
    evt.nPos = nPos;
    evt.action = action;
    return FireEvent(evt);
}

SIZE SSliderBar::MeasureContent(int wid, int hei)
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

SStringT SSliderBar::GetTooltip(CPoint pt) const
{
    if (m_segments.IsEmpty())
        return __baseCls::GetTooltip(pt);
    int iSegment = FindSegmentByValue(m_nValue);
    if (iSegment == -1)
        return SStringT();
    return m_segments[iSegment].szDesc;
}

void SSliderBar::OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer)
{
    if (pOldContainer)
    {
        m_thumbAni->end();
        m_valueAni->end();
    }
    __baseCls::OnContainerChanged(pOldContainer, pNewContainer);
}

BOOL SSliderBar::SetValue(THIS_ int nValue)
{
    if (m_bDrag)
        return FALSE;
    if (m_bEnableAnimate && m_valueAni->isRunning())
    {
        return FALSE;
    }
    return __baseCls::SetValue(nValue);
}

void SSliderBar::ShowValueInTip(int nValue)
{
    if (m_bDragTip)
    {
        CRect rcThumb = GetPartRect2(SC_THUMB);
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

void SSliderBar::onAnimationEnd(IValueAnimator *p)
{
    if (p != m_valueAni)
        return;
    int nAniValue = m_valueAni->getValue();
    OnValueChanged(nAniValue, SBA_MOUSE_DOWN);
}

void SSliderBar::onAnimationUpdate(IValueAnimator *p)
{
    if (p == m_valueAni)
    {
        int nAniValue = m_valueAni->getValue();
        if (nAniValue != m_nValue)
        {
            m_nValue = nAniValue;
            Invalidate();
        }
    }
    else if (p == m_thumbAni)
    {
        BYTE byAlpha = m_thumbAni->getValue();
        if (byAlpha == m_byThumbAlphaAni)
            return;
        m_byThumbAlphaAni = byAlpha;
        CRect rcThumb = GetPartRect2(SC_THUMB);
        InvalidateRect(rcThumb);
    }
}

void SSliderBar::OnDestroy()
{
    m_thumbAni->end();
    m_valueAni->end();
    __baseCls::OnDestroy();
}

void SSliderBar::SetSegments(const SEGMENT *segments, int count)
{
    m_segments.RemoveAll();
    m_segments.SetCount(count);
    for (int i = 0; i < count; i++)
    {
        m_segments.SetAt(i, segments[i]);
    }
}

int SSliderBar::GetSegmentCount() const
{
    return m_segments.GetCount();
}

BOOL SSliderBar::GetSegment(int index, SEGMENT *pSegment) const
{
    if (index < 0 || index >= m_segments.GetCount() || !pSegment)
        return FALSE;
    *pSegment = m_segments[index];
    return TRUE;
}

int SSliderBar::FindSegmentByValue(int value) const
{
    if (m_segments.GetCount() == 0)
        return -1;

    // 找到值所在的分段
    // 分段范围：[当前分段value, 下一分段value)
    for (int i = 0; i < m_segments.GetCount(); i++)
    {
        int nSegStart = m_segments[i].value;
        int nSegEnd = (i + 1 < m_segments.GetCount()) ? m_segments[i + 1].value : m_nMaxValue + 1;

        if (value >= nSegStart && value < nSegEnd)
        {
            return i;
        }
    }

    // 如果值大于等于最后一个分段的起点，归属于最后一个分段
    if (m_segments.GetCount() > 0 && value >= m_segments[m_segments.GetCount() - 1].value)
    {
        return m_segments.GetCount() - 1;
    }

    return -1;
}

SNSEND