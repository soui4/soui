#include "stdafx.h"
#include "SAbstractChartView.h"
#include <math.h>

SNSBEGIN

// Static constants
const float SChartTouchHandler::MIN_ZOOM_SCALE = 1.0f;
const float SChartTouchHandler::MAX_ZOOM_SCALE = 20.0f;
const DWORD SChartTouchHandler::TOUCH_TIMEOUT = 300;

//////////////////////////////////////////////////////////////////////////
// SAbstractChartView implementation

SAbstractChartView::SAbstractChartView()
    : m_pChartComputator(NULL)
    , m_pAxesRenderer(NULL)
    , m_pChartRenderer(NULL)
    , m_pTouchHandler(NULL)
    , m_bInteractive(TRUE)
    , m_bZoomEnabled(TRUE)
    , m_bScrollEnabled(TRUE)
    , m_bContainerScrollEnabled(FALSE)
    , m_bTouchDown(FALSE)
    , m_lastTouchDistance(0.0f)
{
    m_bClipClient=TRUE;
    InitializeChart();
    m_animator.addUpdateListener(this);
	m_animator.addListener(this);
    AddEvent(EVENTID(SChartViewportChangedEvent));
    AddEvent(EVENTID(SChartSelectionChangedEvent));
    AddEvent(EVENTID(SChartDataAnimationStartEvent));
    AddEvent(EVENTID(SChartDataAnimationUpdateEvent));
    AddEvent(EVENTID(SChartDataAnimationEndEvent));
}

SAbstractChartView::~SAbstractChartView()
{
    if (m_pChartComputator)
    {
        delete m_pChartComputator;
        m_pChartComputator = NULL;
    }
    
    if (m_pAxesRenderer)
    {
        delete m_pAxesRenderer;
        m_pAxesRenderer = NULL;
    }
    
    if (m_pTouchHandler)
    {
        delete m_pTouchHandler;
        m_pTouchHandler = NULL;
    }
}

void SAbstractChartView::SetChartRenderer(SChartRenderer* pRenderer)
{
    m_pChartRenderer = pRenderer;
}

void SAbstractChartView::AnimationDataUpdate(float scale)
{
    OnAnimationUpdate(scale);
    Invalidate();
}

void SAbstractChartView::AnimationDataFinished()
{
	SChartData* pChartData = GetChartData();
	if (pChartData)
	{
		pChartData->Finish();
		Invalidate();
	}
}

void SAbstractChartView::StartDataAnimation(DWORD duration)
{
    if(m_animator.isStarted()){
        CancelDataAnimation();
    }
    // Initialize animation data before starting
    SChartData* pChartData = GetChartData();
    if (pChartData)
    {
        pChartData->InitializeAnimation();
    }
    m_animator.setDuration(duration);
    m_animator.start(GetContainer());
}

void SAbstractChartView::CancelDataAnimation()
{
    m_animator.end();
}

BOOL SAbstractChartView::IsDataAnimationRunning()
{
    return m_animator.isStarted();
}

SViewport SAbstractChartView::GetCurrentViewport()
{
    return m_pChartComputator ? m_pChartComputator->GetCurrentViewport() : SViewport();
}

void SAbstractChartView::SetCurrentViewport(const SViewport& viewport)
{
    if (m_pChartComputator)
    {
        SViewport constrainedViewport = m_pChartComputator->ConstrainViewport(viewport);
        m_pChartComputator->SetCurrentViewport(constrainedViewport);
        SChartViewportChangedEvent event(this);
        event.viewport = constrainedViewport;
        FireEvent(event);
        Invalidate();
    }
}

SViewport SAbstractChartView::GetMaximumViewport()
{
    return m_pChartComputator ? m_pChartComputator->GetMaximumViewport() : SViewport();
}

void SAbstractChartView::SetMaximumViewport(const SViewport& viewport)
{
    if (m_pChartComputator)
    {
        m_pChartComputator->SetMaximumViewport(viewport);
        // Also set as current viewport if current is not set
        if (!GetCurrentViewport().IsValid())
        {
            SetCurrentViewport(viewport);
        }
    }
}

SSelectedValue SAbstractChartView::GetSelectedValue()
{
    return m_pChartRenderer ? m_pChartRenderer->GetSelectedValue() : SSelectedValue();
}

BOOL SAbstractChartView::SelectValue(float x, float y)
{
    SSelectedValue currentSelection = GetSelectedValue();
    if (m_pChartRenderer && m_pChartRenderer->IsTouched(x, y))
    {
        // 选中了某个值
        CallTouchListener();
        Invalidate();
        return TRUE;
    }
    else if (currentSelection.IsSet())
    {
        CallTouchListener();
        Invalidate();
        return TRUE;
    }
    return FALSE;
}

void SAbstractChartView::ClearSelection()
{
    if (m_pChartRenderer)
    {
        m_pChartRenderer->ClearSelection();
        CallTouchListener();
        Invalidate();
    }
}

void SAbstractChartView::OnChartDataChange()
{
    if (m_pChartRenderer)
    {
        m_pChartRenderer->OnChartDataChanged();
    }
    Invalidate();
}

void SAbstractChartView::OnChartSizeChanged()
{
    SAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
    BeforePaintEx(pRT);
    if (m_pAxesRenderer)
    {
        m_pAxesRenderer->OnChartSizeChanged(pRT);
    }
    
    if (m_pChartRenderer)
    {
        m_pChartRenderer->OnChartSizeChanged();
    }
}

void SAbstractChartView::GetRect(LPRECT pRect)
{
    GetWindowRect(pRect);
}
void SAbstractChartView::OnPaint(IRenderTarget* pRT)
{
    if (!IsVisible() || !pRT)
        return;
        
    CRect rcClient = GetClientRect();
    
    // Fill background
    SChartData* pData = GetChartData();
    COLORREF bgColor = pData ? pData->GetBackgroundColor() : DEFAULT_BACKGROUND_COLOR;
    pRT->FillSolidRect(&rcClient, bgColor);
    
    if (!IsDisabled())
    {
        // Draw axes background
        if (m_pAxesRenderer)
        {
            m_pAxesRenderer->DrawInBackground(pRT);
        }
        
        // Clip to chart area and draw chart content
        if (m_pChartRenderer && m_pChartComputator)
        {
            CRect chartRect = m_pChartComputator->GetContentRectMinusAllMargins();
            chartRect.OffsetRect(rcClient.left, rcClient.top);
            pRT->PushClipRect(&chartRect, RGN_AND);
            m_pChartRenderer->Draw(pRT);
            pRT->PopClip();
            
            // Draw unclipped elements
            m_pChartRenderer->DrawUnclipped(pRT);
        }
        
        // Draw axes foreground
        if (m_pAxesRenderer)
        {
            m_pAxesRenderer->DrawInForeground(pRT);
        }
    }
}

void SAbstractChartView::OnSize(UINT nType, CSize size)
{
    __baseCls::OnSize(nType, size);
    
    if (m_pChartComputator)
    {
        CRect rcClient = GetClientRect();
        m_pChartComputator->SetContentRect(rcClient.Width(), rcClient.Height(), 
                                          0, 0, 0, 0);
    }
    
    OnChartSizeChanged();
}

BOOL SAbstractChartView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if (m_bInteractive && m_bZoomEnabled && m_pTouchHandler)
    {
        return m_pTouchHandler->HandleMouseWheel(nFlags, zDelta, pt);
    }
    
    return __baseCls::OnMouseWheel(nFlags, zDelta, pt);
}

void SAbstractChartView::OnLButtonDown(UINT nFlags, CPoint point)
{
    __baseCls::OnLButtonDown(nFlags, point);
    if (m_bInteractive)
    {
        m_bTouchDown = TRUE;
        m_lastTouchPoint = point;        
        if (m_pTouchHandler)
        {
            m_pTouchHandler->HandleTouchEvent(nFlags, point);
        }
    }
}

void SAbstractChartView::OnLButtonUp(UINT nFlags, CPoint point)
{
    __baseCls::OnLButtonUp(nFlags, point);
    if (m_bTouchDown)
    {
        m_bTouchDown = FALSE;        
        if (m_pTouchHandler)
        {
            m_pTouchHandler->HandleTouchEvent(nFlags, point);
        }
        
        // Check for value selection
        SelectValue((float)point.x, (float)point.y);
    }
    
}

void SAbstractChartView::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_bTouchDown && m_bInteractive && m_pTouchHandler)
    {
        m_pTouchHandler->HandleTouchEvent(nFlags, point);
        m_lastTouchPoint = point;
    }
    
    __baseCls::OnMouseMove(nFlags, point);
}

int SAbstractChartView::OnCreate(void *p)
{
    int ret = __baseCls::OnCreate(p);
    if(ret!=0) return ret;
    InitializeChart();
    return 0;
}

void SAbstractChartView::OnDestroy()
{
    // Stop any running animation
    CancelDataAnimation();

    __baseCls::OnDestroy();
}



void SAbstractChartView::InitializeChart()
{
    m_pChartComputator = new SChartComputator();
    m_pAxesRenderer = new SAxesRenderer(this);
    m_pTouchHandler = new SChartTouchHandler(this);
    
    InitializeRenderers();
}

void SAbstractChartView::InitializeRenderers()
{
    // Override in derived classes
}

//////////////////////////////////////////////////////////////////////////
// SChartTouchHandler implementation

SChartTouchHandler::SChartTouchHandler(ISChart* pChart)
    : m_pChart(pChart)
    , m_bZoomEnabled(TRUE)
    , m_bScrollEnabled(TRUE)
    , m_bTouchDown(FALSE)
    , m_touchDownTime(0)
    , m_lastDistance(0.0f)
{
}

SChartTouchHandler::~SChartTouchHandler()
{
}

BOOL SChartTouchHandler::HandleTouchEvent(UINT nFlags, CPoint point)
{
    if (!m_pChart)
        return FALSE;

    if (nFlags & MK_LBUTTON)
    {
        if (!m_bTouchDown)
        {
            // Touch down
            m_bTouchDown = TRUE;
            m_touchDownPoint = point;
            m_lastTouchPoint = point;
            m_touchDownTime = GetTickCount();
        }
        else
        {
            // Touch move - handle scrolling
            if (m_bScrollEnabled)
            {
                CPoint delta = point - m_lastTouchPoint;
                HandleScroll(delta);
                m_lastTouchPoint = point;
            }
        }
    }
    else
    {
        // Touch up
        if (m_bTouchDown)
        {
            m_bTouchDown = FALSE;

            // Check if this was a quick tap (for selection)
            DWORD touchDuration = GetTickCount() - m_touchDownTime;
            CPoint touchDistance = point - m_touchDownPoint;

            if (touchDuration < TOUCH_TIMEOUT &&
                abs(touchDistance.x) < 10 && abs(touchDistance.y) < 10)
            {
                // This was a tap, let the chart handle value selection
                return FALSE;
            }
        }
    }

    return TRUE;
}

BOOL SChartTouchHandler::HandleMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
    if (!m_pChart || !m_bZoomEnabled)
        return FALSE;

    float scaleFactor = 1.0f + (zDelta > 0 ? 0.1f : -0.1f);
    HandleZoom(scaleFactor, point);

    return TRUE;
}

void SChartTouchHandler::HandleScroll(CPoint delta)
{
    if (!m_pChart)
        return;
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;

    SViewport currentViewport = pComputator->GetCurrentViewport();

    // Convert pixel delta to viewport delta
    float deltaX = pComputator->ComputeRawDistanceX((float)delta.x) -
                   pComputator->ComputeRawDistanceX(0.0f);
    float deltaY = pComputator->ComputeRawDistanceY((float)delta.y) -
                   pComputator->ComputeRawDistanceY(0.0f);
    if (isnan(deltaX) || isnan(deltaY))
        return;
    // Apply scroll
    SViewport newViewport(
        currentViewport.GetLeft() - deltaX,
        currentViewport.GetTop() - deltaY,
        currentViewport.GetRight() + deltaX,
        currentViewport.GetBottom() + deltaY
    );

    m_pChart->SetCurrentViewport(newViewport);
}

void SChartTouchHandler::HandleZoom(float scaleFactor, CPoint center)
{
    if (!m_pChart)
        return;

    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;

    SViewport currentViewport = pComputator->GetCurrentViewport();
    SViewport maxViewport = pComputator->GetMaximumViewport();

    // Limit zoom scale to prevent over-zooming
    float currentScale = maxViewport.GetWidth() / currentViewport.GetWidth();
    float newScale = currentScale * scaleFactor;

    if (newScale < MIN_ZOOM_SCALE)
    {
        scaleFactor = MIN_ZOOM_SCALE / currentScale;
    }
    else if (newScale > MAX_ZOOM_SCALE)
    {
        scaleFactor = MAX_ZOOM_SCALE / currentScale;
    }


    // Calculate new viewport size
    float centerX = currentViewport.GetCenterX();
    float centerY = currentViewport.GetCenterY();
    float newWidth = currentViewport.GetWidth() / scaleFactor;
    float newHeight = currentViewport.GetHeight() / scaleFactor;

    // Calculate new viewport bounds centered on zoom point
    float newLeft = centerX - newWidth / 2.0f;
    float newTop = centerY - newHeight / 2.0f;
    float newRight = centerX + newWidth / 2.0f;
    float newBottom = centerY + newHeight / 2.0f;

    SViewport newViewport(newLeft, newTop, newRight, newBottom);
    m_pChart->SetCurrentViewport(newViewport);
}

void SAbstractChartView::onAnimationStart(THIS_ IValueAnimator * pAnimator)
{
    SChartDataAnimationStartEvent evt(this);
    evt.dwDuration = pAnimator->getDuration();
    FireEvent(evt);
}

void SAbstractChartView::onAnimationEnd(THIS_ IValueAnimator * pAnimator)
{
    AnimationDataFinished();
    SChartDataAnimationEndEvent evt(this);
    FireEvent(evt);
}
void SAbstractChartView::onAnimationUpdate(THIS_ IValueAnimator * pAnimator)
{
    float scale = pAnimator->getAnimatedFraction();
    OnAnimationUpdate(scale);
    SChartDataAnimationUpdateEvent evt(this);
    evt.scale = scale;
    FireEvent(evt);
}
void SAbstractChartView::RefreshAxesMargins()
{
    // Recalculate axes margins based on current data
    if (m_pAxesRenderer)
    {
        SAutoRefPtr<IRenderTarget> pRT;
        GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
        m_pAxesRenderer->OnChartSizeChanged(pRT);
    }
    // Invalidate to trigger repaint with new margins
    Invalidate();
}

void SAbstractChartView::OnAnimationUpdate(float scale)
{
    SChartData* pChartData = GetChartData();
    if (pChartData)
    {
        pChartData->Update(scale);
        Invalidate();
    }
}

SNSEND
