#ifndef __SABSTRACTCHARTVIEW_H__
#define __SABSTRACTCHARTVIEW_H__

#include <core/SWnd.h>
#include <valueAnimator/SValueAnimator.h>
#include "../SChart.h"
#include "../computator/SChartComputator.h"
#include "../renderer/SChartRenderer.h"
#include "../event/SChartEvents.h"

SNSBEGIN

/**
 * @class SAbstractChartView
 * @brief Abstract base class for all chart views
 */
class  SAbstractChartView : public SWindow, public ISChart, protected IAnimatorUpdateListener, protected IAnimatorListener
{
    DEF_SOBJECT(SWindow, L"abstractChart")

public:
    SAbstractChartView();
    virtual ~SAbstractChartView();

    // ISChart interface implementation
    virtual SChartData* GetChartData() OVERRIDE = 0;
    virtual SChartRenderer* GetChartRenderer() OVERRIDE { return m_pChartRenderer; }
    virtual void SetChartRenderer(SChartRenderer* pRenderer) OVERRIDE;
    virtual SAxesRenderer* GetAxesRenderer() OVERRIDE { return m_pAxesRenderer; }
    virtual SChartComputator* GetChartComputator() OVERRIDE { return m_pChartComputator; }
    virtual SChartTouchHandler* GetTouchHandler() OVERRIDE { return m_pTouchHandler; }

    virtual void AnimationDataUpdate(float scale) OVERRIDE;
    virtual void AnimationDataFinished() OVERRIDE;
    virtual void StartDataAnimation(DWORD duration) OVERRIDE;
    virtual void CancelDataAnimation() OVERRIDE;
    virtual BOOL IsDataAnimationRunning() OVERRIDE;

    virtual SViewport GetCurrentViewport() OVERRIDE;
    virtual void SetCurrentViewport(const SViewport& viewport) OVERRIDE;
    virtual SViewport GetMaximumViewport() OVERRIDE;
    virtual void SetMaximumViewport(const SViewport& viewport) OVERRIDE;

    virtual BOOL IsInteractive() OVERRIDE { return m_bInteractive; }
    virtual void SetInteractive(BOOL bInteractive) OVERRIDE { m_bInteractive = bInteractive; }
    virtual BOOL IsZoomEnabled() OVERRIDE { return m_bZoomEnabled; }
    virtual void SetZoomEnabled(BOOL bEnabled) OVERRIDE { m_bZoomEnabled = bEnabled; }
    virtual BOOL IsScrollEnabled() OVERRIDE { return m_bScrollEnabled; }
    virtual void SetScrollEnabled(BOOL bEnabled) OVERRIDE { m_bScrollEnabled = bEnabled; }

    virtual SSelectedValue GetSelectedValue() OVERRIDE;
    virtual BOOL SelectValue(float x, float y) OVERRIDE;
    virtual void ClearSelection() OVERRIDE;

    virtual void OnChartDataChange() OVERRIDE;
    virtual void OnChartSizeChanged() OVERRIDE;
    virtual void GetRect(LPRECT pRect) OVERRIDE;
    virtual void CallTouchListener() OVERRIDE = 0;

protected:
    virtual void OnPaint(IRenderTarget* pRT) ;
    virtual void OnSize(UINT nType, CSize size) ;
    virtual BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) ;
    virtual void OnLButtonDown(UINT nFlags, CPoint point) ;
    virtual void OnLButtonUp(UINT nFlags, CPoint point) ;
    virtual void OnMouseMove(UINT nFlags, CPoint point) ;
    virtual int OnCreate(void *p);
    virtual void OnDestroy();

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_CREATE(OnCreate)
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SIZE(OnSize)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_DESTROY(OnDestroy)
       // MESSAGE_HANDLER_EX(WM_TOUCH, OnTouch)
    SOUI_MSG_MAP_END()
protected:
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator * pAnimator) OVERRIDE;
    STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator * pAnimator) OVERRIDE{}
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator * pAnimator) OVERRIDE;
    STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator * pAnimator) OVERRIDE;
    // Axes margin refresh
    virtual void RefreshAxesMargins();

    virtual void OnAnimationUpdate(float scale);
    // Touch handling
    virtual BOOL HandleTouch(UINT nFlags, CPoint point){
        return FALSE;
    }
    virtual void HandleZoom(float scaleFactor, CPoint center){

    }
    virtual void HandleScroll(CPoint delta){
        
    }

    // Initialization
    virtual void InitializeChart();
    virtual void InitializeRenderers();

protected:
    SChartComputator* m_pChartComputator;
    SAxesRenderer* m_pAxesRenderer;
    SChartRenderer* m_pChartRenderer;
    SChartTouchHandler* m_pTouchHandler;

    // Chart properties
    BOOL m_bInteractive;
    BOOL m_bZoomEnabled;
    BOOL m_bScrollEnabled;
    BOOL m_bContainerScrollEnabled;

    // Animation
    SFloatAnimator m_animator;

    // Touch handling
    BOOL m_bTouchDown;
    CPoint m_lastTouchPoint;
    float m_lastTouchDistance;

    // Default values
    static const COLORREF DEFAULT_BACKGROUND_COLOR = RGBA(255, 255, 255,255);
    static const DWORD DEFAULT_ANIMATION_DURATION = 300;

public:
    SOUI_ATTRS_BEGIN()
        ATTR_BOOL(L"interactive", m_bInteractive, TRUE)
        ATTR_BOOL(L"zoomEnabled", m_bZoomEnabled, TRUE)
        ATTR_BOOL(L"scrollEnabled", m_bScrollEnabled, TRUE)
        ATTR_BOOL(L"containerScrollEnabled", m_bContainerScrollEnabled, FALSE)
        ATTR_CHAIN(m_animator, 0)
    SOUI_ATTRS_END()
};

/**
 * @class SChartTouchHandler
 * @brief Handles touch events for charts
 */
class  SChartTouchHandler
{
public:
    SChartTouchHandler(ISChart* pChart);
    virtual ~SChartTouchHandler();

    /**
     * @brief Handle touch event
     * @param nFlags Touch flags
     * @param point Touch point
     * @return TRUE if event was handled
     */
    virtual BOOL HandleTouchEvent(UINT nFlags, CPoint point);

    /**
     * @brief Handle mouse wheel event
     * @param nFlags Wheel flags
     * @param zDelta Wheel delta
     * @param point Wheel point
     * @return TRUE if event was handled
     */
    virtual BOOL HandleMouseWheel(UINT nFlags, short zDelta, CPoint point);

    /**
     * @brief Check if zoom is enabled
     * @return TRUE if zoom is enabled
     */
    BOOL IsZoomEnabled() const { return m_bZoomEnabled; }

    /**
     * @brief Set zoom enabled state
     * @param bEnabled Zoom enabled state
     */
    void SetZoomEnabled(BOOL bEnabled) { m_bZoomEnabled = bEnabled; }

    /**
     * @brief Check if scroll is enabled
     * @return TRUE if scroll is enabled
     */
    BOOL IsScrollEnabled() const { return m_bScrollEnabled; }

    /**
     * @brief Set scroll enabled state
     * @param bEnabled Scroll enabled state
     */
    void SetScrollEnabled(BOOL bEnabled) { m_bScrollEnabled = bEnabled; }

    /**
     * @brief Handle scroll gesture
     * @param delta Scroll delta
     */
    void HandleScroll(CPoint delta);

    /**
     * @brief Handle zoom gesture
     * @param scaleFactor Zoom scale factor
     * @param center Zoom center point
     */
    void HandleZoom(float scaleFactor, CPoint center);

protected:
    ISChart* m_pChart;
    BOOL m_bZoomEnabled;
    BOOL m_bScrollEnabled;
    
    // Touch state
    BOOL m_bTouchDown;
    CPoint m_lastTouchPoint;
    CPoint m_touchDownPoint;
    DWORD m_touchDownTime;
    
    // Zoom state
    float m_lastDistance;
    CPoint m_zoomCenter;
    
    // Constants
    static const float MIN_ZOOM_SCALE;
    static const float MAX_ZOOM_SCALE;
    static const DWORD TOUCH_TIMEOUT;
};

SNSEND

#endif//__SABSTRACTCHARTVIEW_H__