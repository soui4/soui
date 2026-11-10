/**
 * @file       SGestureDetector.h
 * @brief      Android GestureDetector / VelocityTracker style drag recognizer.
 *
 */

#ifndef __SGESTUREDETECTOR__H__
#define __SGESTUREDETECTOR__H__

#include <windows.h>
#include <core/SWnd.h>

SNSBEGIN

/**
 * @brief Gesture callbacks, mirroring android.view.GestureDetector.OnGestureListener.
 * @details All callbacks are optional; override only what the control needs.
 */
class IGestureListener {
  public:
    virtual ~IGestureListener()
    {
    }
    virtual void OnGestureDown(const CPoint &)
    {
    }
    virtual void OnGestureScroll(const CPoint &, const CPoint &, int, int)
    {
    }
    virtual void OnGestureFling(float, float)
    {
    }
    virtual void OnGestureUp(const CPoint &, float, float)
    {
    }
    virtual void OnGestureCancel()
    {
    }
};

/**
 * @brief Drag / scroll / fling recognizer shared by interception and touch handling.
 *
 * Equivalent of ViewConfiguration.getScaledTouchSlop() + GestureDetector +
 * VelocityTracker. The touch slop is supplied by the owner (convert dp with
 * SLayoutSize::toPixelSize(GetScale())), so this class stays window-agnostic
 * and can be exercised without a running container.
 *
 * OnInterceptTouchEvent() is a pure probe: DOWN records the start point, MOVE
 * only answers "should my parent take over?" and never fires callbacks. After
 * the container intercepts, OnTouchEvent() continues from the very same state,
 * so neither the start point nor the velocity samples are duplicated or lost.
 *
 * AxisHorizontal / AxisVertical apply the "active axis dominates" rule meant
 * for page switching. AxisBoth accepts a drag on either axis and reports both
 * components, matching SPanel's drag-to-scroll (X and Y are scrolled together).
 */
class SOUI_EXP SGestureDetector {
  public:
    enum Axis
    {
        AxisHorizontal = 0,
        AxisVertical = 1,
        AxisBoth = 2, /**< Any axis may start the drag; used by two-axis scroll panels. */
    };

    enum
    {
        kVelocityWindowMs = 100, /**< Same 100 ms validity window as SPanel.cpp:1108-1115 */
        kMaxSamples = 16,
    };

    SGestureDetector(IGestureListener *pListener);

    void SetListener(IGestureListener *pListener)
    {
        m_pListener = pListener;
    }
    void SetAxis(Axis axis)
    {
        m_axis = axis;
    }
    void SetTouchSlop(int nPx)
    {
        m_nTouchSlop = nPx > 0 ? nPx : 1;
    }
    void SetMinFlingVelocity(float fPxPerMs)
    {
        m_fMinFlingVelocity = fPxPerMs;
    }

    BOOL IsDragging() const
    {
        return m_bDragging;
    }
    BOOL IsDown() const
    {
        return m_bDown;
    }
    const CPoint &GetDownPoint() const
    {
        return m_ptDown;
    }
    const CPoint &GetCurrentPoint() const
    {
        return m_ptLast;
    }
    float GetVelocityX() const
    {
        return m_fVelocityX;
    }
    float GetVelocityY() const
    {
        return m_fVelocityY;
    }

    /** Called from SWindow::OnInterceptTouchEvent. Never consumes the event. */
    BOOL OnInterceptTouchEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /** Called from SWindow::OnTouchEvent or the window's own message map. */
    BOOL OnTouchEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void Reset();

  private:
    struct Sample
    {
        DWORD t;
        int x, y;
    };

    static CPoint PointOf(LPARAM lParam)
    {
        return CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    }

    static int Abs(int v)
    {
        return v < 0 ? -v : v;
    }

    BOOL ShouldStartDrag(const CPoint &pt) const;

    void FeedDown(const CPoint &pt);

    void AddSample(const CPoint &pt);
    void UpdateVelocity();

  private:
    IGestureListener *m_pListener;
    Axis m_axis;
    int m_nTouchSlop;
    float m_fMinFlingVelocity;
    BOOL m_bDown;
    BOOL m_bDragging;
    int m_nSampleCount;
    Sample m_samples[kMaxSamples];
    CPoint m_ptDown;
    CPoint m_ptLast;
    float m_fVelocityX;
    float m_fVelocityY;
};

SNSEND
#endif /**< __SGESTUREDETECTOR__H__ */
