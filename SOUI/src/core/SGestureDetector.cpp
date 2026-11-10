/**
 * @file       SGestureDetector.pp
 * @brief      Android GestureDetector / VelocityTracker style drag recognizer.
 */
#include <souistd.h>
#include <core/SGestureDetector.h>

SNSBEGIN


static DWORD Ts32Diff(DWORD ts2, DWORD ts1) {
    // Unsigned 32-bit subtraction is well defined modulo 2^32: it handles the
    // GetTickCount() 49-day wraparound and returns 0 when both ticks are equal.
    return ts2 - ts1;
}

SGestureDetector::SGestureDetector(IGestureListener *pListener)
    : m_pListener(pListener)
    , m_axis(AxisHorizontal)
    , m_nTouchSlop(8)
    , m_fMinFlingVelocity(0.5f) // px/ms
    , m_bDown(FALSE)
    , m_bDragging(FALSE)
    , m_nSampleCount(0)
    , m_ptDown(0, 0)
    , m_ptLast(0, 0)
    , m_fVelocityX(0.f)
    , m_fVelocityY(0.f)
{
}

/** Called from SWindow::OnInterceptTouchEvent. Never consumes the event. */
BOOL SGestureDetector::OnInterceptTouchEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_LBUTTONDOWN)
    {
        FeedDown(PointOf(lParam));
        return FALSE; // ACTION_DOWN is never intercepted
    }
    if (uMsg == WM_MOUSEMOVE)
    {
        if (!m_bDown || !(wParam & MK_LBUTTON))
            return FALSE;
        return ShouldStartDrag(PointOf(lParam)); // pure probe, no state change
    }
    return FALSE;
}

/** Called from SWindow::OnTouchEvent or the window's own message map. */
BOOL SGestureDetector::OnTouchEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CPoint pt = PointOf(lParam);
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
        FeedDown(pt);
        return FALSE;
    case WM_MOUSEMOVE:
        if (!m_bDown || !(wParam & MK_LBUTTON))
            return FALSE;
        {
            int dx = pt.x - m_ptLast.x;
            int dy = pt.y - m_ptLast.y;
            AddSample(pt);
            m_ptLast = pt;
            UpdateVelocity();
            if (!m_bDragging && ShouldStartDrag(pt))
                m_bDragging = TRUE;
            if (m_bDragging && m_pListener)
                m_pListener->OnGestureScroll(m_ptDown, pt, dx, dy);
        }
        return m_bDragging;
    case WM_LBUTTONUP:
    {
        // A stray UP without a matching DOWN must not fire OnGestureUp: owners
        // use that callback to release capture and start a fling animation.
        if (!m_bDown)
            return FALSE;
        BOOL bWasDragging = m_bDragging;
        float fVx = m_fVelocityX, fVy = m_fVelocityY;
        // VelocityTracker data is only valid while the samples are fresh.
        if (m_nSampleCount == 0 || Ts32Diff(GetTickCount() , m_samples[m_nSampleCount - 1].t) > kVelocityWindowMs)
        {
            fVx = 0.f;
            fVy = 0.f;
        }
        if (m_pListener)
        {
            if (bWasDragging)
            {
                float fAxisV = fVx;
                if (m_axis == AxisVertical)
                    fAxisV = fVy;
                else if (m_axis == AxisBoth)
                    fAxisV = ((fVx < 0 ? -fVx : fVx) > (fVy < 0 ? -fVy : fVy)) ? fVx : fVy;
                if (fAxisV > m_fMinFlingVelocity || fAxisV < -m_fMinFlingVelocity)
                    m_pListener->OnGestureFling(fVx, fVy);
            }
            m_pListener->OnGestureUp(pt, fVx, fVy);
        }
        Reset();
        return bWasDragging;
    }
    case WM_CANCELMODE:
    {
        BOOL bWasDragging = m_bDragging;
        if (bWasDragging && m_pListener)
            m_pListener->OnGestureCancel();
        Reset();
        return bWasDragging;
    }
    default:
        return FALSE;
    }
}

void SGestureDetector::Reset()
{
    m_bDown = FALSE;
    m_bDragging = FALSE;
    m_nSampleCount = 0;
    m_fVelocityX = 0.f;
    m_fVelocityY = 0.f;
}

BOOL SGestureDetector::ShouldStartDrag(const CPoint &pt) const
{
    int dx = Abs(pt.x - m_ptDown.x);
    int dy = Abs(pt.y - m_ptDown.y);
    // A two-axis scroll panel moves X and Y at the same time, so no axis may
    // be voted out by the "dominant axis" rule.
    if (m_axis == AxisBoth)
        return dx > m_nTouchSlop || dy > m_nTouchSlop;
    int nAxis = (m_axis == AxisHorizontal) ? dx : dy;
    int nCross = (m_axis == AxisHorizontal) ? dy : dx;
    // The active axis must dominate so a vertical list inside a page can
    // still scroll without accidentally switching pages.
    return nAxis > m_nTouchSlop && nAxis >= nCross;
}

void SGestureDetector::FeedDown(const CPoint &pt)
{
    Reset();
    m_bDown = TRUE;
    m_ptDown = pt;
    m_ptLast = pt;
    AddSample(pt);
    if (m_pListener)
        m_pListener->OnGestureDown(pt);
}

void SGestureDetector::AddSample(const CPoint &pt)
{
    DWORD now = GetTickCount();
    int nKeep = 0;
    for (int i = 0; i < m_nSampleCount; ++i)
    {
        if (Ts32Diff(now , m_samples[i].t) <= kVelocityWindowMs)
            m_samples[nKeep++] = m_samples[i];
    }
    m_nSampleCount = nKeep;
    if (m_nSampleCount >= kMaxSamples)
    {
        for (int i = 1; i < m_nSampleCount; ++i)
            m_samples[i - 1] = m_samples[i];
        --m_nSampleCount;
    }
    Sample &s = m_samples[m_nSampleCount++];
    s.t = now;
    s.x = pt.x;
    s.y = pt.y;
}

void SGestureDetector::UpdateVelocity()
{
    m_fVelocityX = 0.f;
    m_fVelocityY = 0.f;
    if (m_nSampleCount < 2)
        return;
    const Sample &a = m_samples[0];
    const Sample &b = m_samples[m_nSampleCount - 1];
    ULONGLONG dt = b.t - a.t;
    if (dt == 0)
        return;
    m_fVelocityX = (float)(b.x - a.x) / (float)dt;
    m_fVelocityY = (float)(b.y - a.y) / (float)dt;
}

SNSEND
