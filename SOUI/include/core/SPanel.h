/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SPanel.h
 * @brief      SOUI Panel with Scrollbar Support
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Description: Defines the SPanel class, which provides a panel with support for scrollbars in the non-client area.
 */

#ifndef __SPANEL__H__
#define __SPANEL__H__

#include <core/SWnd.h>
#include <core/SGestureDetector.h>
#include <core/SScrollBarHandler.h>
#include <interface/SCtrls-i.h>
#include <proxy/SWindowProxy.h>
#include <valueAnimator/SValueAnimator.h>
#include <interface/SValueAnimator-i.h>
#include <animation/SInterpolatorImpl.h>

SNSBEGIN

/**
 * @class     SPanel
 * @brief     Panel with Scrollbar Support
 *
 * Description: Implements a panel with support for scrollbars in the non-client area.
 *              Includes drag-to-scroll and fling animation support.
 */
class SOUI_EXP SPanel
    : public TWindowProxy<IPanel>
    , protected IScrollBarHost
    , public IAnimatorUpdateListener
    , public SAnimatorListener
    , public IGestureListener {
    DEF_SOBJECT(SWindow, L"div")

  public:
    /**
     * @brief Constructor for SPanel.
     */
    SPanel();

    /**
     * @brief Destructor for SPanel.
     */
    virtual ~SPanel()
    {
    }

  public:
    /**
     * @brief Shows or hides a scrollbar.
     * @param wBar Scrollbar identifier (SB_HORZ or SB_VERT).
     * @param bShow TRUE to show the scrollbar, FALSE to hide it.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, ShowScrollBar)(THIS_ int wBar, BOOL bShow) OVERRIDE;

    /**
     * @brief Enables or disables a scrollbar.
     * @param wBar Scrollbar identifier (SB_HORZ or SB_VERT).
     * @param bEnable TRUE to enable the scrollbar, FALSE to disable it.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, EnableScrollBar)(THIS_ int wBar, BOOL bEnable) OVERRIDE;

    /**
     * @brief Checks if a scrollbar is enabled.
     * @param bVertical TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @return TRUE if the scrollbar is enabled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsScrollBarEnable)(THIS_ BOOL bVertical) SCONST OVERRIDE;

    /**
     * @brief Sets the scroll information for a scrollbar.
     * @param si Scroll information structure.
     * @param bVertical TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     */
    STDMETHOD_(void, SetScrollInfo)(THIS_ SCROLLINFO si, BOOL bVertical) OVERRIDE;

    /**
     * @brief Sets the scroll position for a scrollbar.
     * @param bVertical TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param nNewPos New scroll position.
     * @param bRedraw TRUE to redraw the scrollbar, FALSE otherwise.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, SetScrollPos)(THIS_ BOOL bVertical, int nNewPos, BOOL bRedraw) OVERRIDE;

    /**
     * @brief Gets the scroll position for a scrollbar.
     * @param bVertical TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @return Current scroll position.
     */
    STDMETHOD_(int, GetScrollPos)(THIS_ BOOL bVertical) SCONST OVERRIDE;

    /**
     * @brief Sets the scroll range for a scrollbar.
     * @param bVertical TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param nMinPos Minimum scroll position.
     * @param nMaxPos Maximum scroll position.
     * @param bRedraw TRUE to redraw the scrollbar, FALSE otherwise.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, SetScrollRange)(THIS_ BOOL bVertical, int nMinPos, int nMaxPos, BOOL bRedraw) OVERRIDE;

    /**
     * @brief Gets the scroll range for a scrollbar.
     * @param bVertical TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param lpMinPos Pointer to receive the minimum scroll position.
     * @param lpMaxPos Pointer to receive the maximum scroll position.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, GetScrollRange)(THIS_ BOOL bVertical, LPINT lpMinPos, LPINT lpMaxPos) SCONST OVERRIDE;

    /**
     * @brief Checks if a scrollbar is present.
     * @param bVertical TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @return TRUE if the scrollbar is present, FALSE otherwise.
     */
    STDMETHOD_(BOOL, HasScrollBar)(THIS_ BOOL bVertical) SCONST OVERRIDE;

    /**
     * @brief Scroll to specified position via OnScroll(SB_THUMBTRACK).
     * @param bVertical TRUE for vertical, FALSE for horizontal.
     * @param nPos Target scroll position.
     * @return TRUE if scrolled, FALSE otherwise.
     */
    BOOL ScrollToPos(BOOL bVertical, int nPos);

    /**
     * @brief Notify scrollbar of scroll activity (show and reset auto-hide timer).
     * @param bVertical TRUE for vertical scrollbar, FALSE for horizontal.
     */
    void NotifyScrollBarActivity(BOOL bVertical);

    BOOL HandleMouseDrag(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lRet);
    BOOL IsItemDragScrollEnabled() const
    {
        return m_bItemDragScrollEnabled;
    }
    void StartDragPending(const CPoint &pt);
    void SetDragPending(BOOL bPending)
    {
        m_bDragPending = bPending;
    }

    /**
     * @brief Draws the rubber band selection rectangle (if active) on top of the client content.
     * @param pRT Render target of the view's OnPaint.
     * @details Derived view controls should call this at the end of their OnPaint implementation.
     */
    void DrawRubberBandSel(IRenderTarget *pRT);

    /**
     * @brief Checks if a rubber band (marquee) selection is currently in progress.
     * @return TRUE when the band is being dragged.
     */
    BOOL IsRubberBandSelActive() const
    {
        return m_bRubberBandActive;
    }

  public:
    /**
     * @brief Gets the client rectangle.
     * @param pRect Pointer to receive the client rectangle.
     */
    virtual void WINAPI GetClientRect(LPRECT pRect) const OVERRIDE;

    /**
     * @brief Gets the client rectangle.
     * @return Client rectangle.
     */
    virtual CRect GetClientRect() const OVERRIDE;

  protected:
    /** Inherited from IScrollBarHost */
    /**
     * @brief Gets the rectangle of a scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @return Rectangle of the scrollbar.
     */
    STDMETHOD_(CRect, GetScrollBarRect)(CTHIS_ BOOL bVert) SCONST OVERRIDE;

    /**
     * @brief Gets the skin for a scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @return Pointer to the scrollbar skin.
     */
    STDMETHOD_(ISkinObj *, GetScrollBarSkin)(CTHIS_ BOOL bVert) SCONST OVERRIDE;

    /**
     * @brief Gets the scroll information for a scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @return Pointer to the scroll information structure.
     */
    STDMETHOD_(const SCROLLINFO *, GetScrollBarInfo)(CTHIS_ BOOL bVert) SCONST OVERRIDE;

    /**
     * @brief Gets the arrow size for a scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @return Arrow size.
     */
    STDMETHOD_(int, GetScrollBarArrowSize)(CTHIS_ BOOL bVert) SCONST OVERRIDE;

    /**
     * @brief Updates a part of the scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param iPart Part to update.
     */
    STDMETHOD_(void, OnScrollUpdatePart)(THIS_ BOOL bVert, int iPart) OVERRIDE;

    /**
     * @brief Updates the thumb track of the scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param nPos New position of the thumb.
     */
    STDMETHOD_(void, OnScrollUpdateThumbTrack)(THIS_ BOOL bVert, int nPos) OVERRIDE;

    /**
     * @brief Gets the container for the scrollbar.
     * @return Pointer to the scrollbar container.
     */
    STDMETHOD_(ISwndContainer *, GetScrollBarContainer)(THIS) OVERRIDE;

    /**
     * @brief Handles scroll commands.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param iCmd Command identifier.
     * @param nPos Position for the command.
     */
    STDMETHOD_(void, OnScrollCommand)(THIS_ BOOL bVert, int iCmd, int nPos) OVERRIDE;

    /**
     * @brief Sets a timer for the scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param id Timer identifier.
     * @param uElapse Elapse time for the timer.
     */
    STDMETHOD_(void, OnScrollSetTimer)(THIS_ BOOL bVert, char id, UINT uElapse) OVERRIDE;

    /**
     * @brief Kills a timer for the scrollbar.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param id Timer identifier.
     */
    STDMETHOD_(void, OnScrollKillTimer)(THIS_ BOOL bVert, char id) OVERRIDE;

    /**
     * @brief Gets the interpolator for the scrollbar.
     * @return Pointer to the interpolator.
     */
    STDMETHOD_(const IInterpolator *, GetScrollInterpolator)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Gets the number of fade frames for the scrollbar.
     * @return Number of fade frames.
     */
    STDMETHOD_(int, GetScrollFadeFrames)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Gets the minimum alpha value for the scrollbar thumb track.
     * @return Minimum alpha value.
     */
    STDMETHOD_(BYTE, GetScrollThumbTrackMinAlpha)(CTHIS) SCONST OVERRIDE;

  protected:
    /**
     * @brief Gets the line size for scrolling.
     * @param bVertical TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @return Line size.
     */
    virtual int GetScrollLineSize(BOOL bVertical);

    /**
     * @brief Handles scroll events.
     * @param bVertical TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param uCode Scroll code.
     * @param nPos Position for the scroll.
     * @return TRUE if handled, FALSE otherwise.
     */
    virtual BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos);

    virtual void OnDragScrollStart();
    virtual void OnDragScroll(BOOL bVert, int delta);
    virtual void OnDragScrollEnd();
    virtual void OnFlingScroll();
    virtual BOOL OnDragCancelCapture(int reason);
    virtual void OnDragClearItemCapture();
    virtual BOOL CancelCaptureMode(int reason) override;

    /** === Unified gesture recognizer (SGestureDetector) ===
     * Drag threshold, velocity tracking, fling detection and UP/CANCEL state
     * now live in SGestureDetector. SPanel only maps the callbacks onto the
     * existing rubber-band / drag-scroll / fling business rules, so all four
     * MVC views and STabCtrl share one gesture state machine.
     */
    virtual void OnGestureDown(const CPoint &pt);
    virtual void OnGestureScroll(const CPoint &ptDown, const CPoint &ptCur, int dx, int dy);
    virtual void OnGestureFling(float fVelocityX, float fVelocityY);
    virtual void OnGestureUp(const CPoint &pt, float fVelocityX, float fVelocityY);
    virtual void OnGestureCancel();

    /** === Container-level gesture ownership (Android ViewGroup style) ===
     * A panel owns exactly one SGestureDetector. When a drag starts on a child
     * the host asks every ancestor through OnInterceptTouchEvent(); once a
     * panel claims it, the remaining stream arrives at OnTouchEvent() and is
     * fed into the SAME detector through HandleMouseDrag(). Derived classes
     * must not implement a second recognizer - they only override the policy
     * hooks below.
     */
    virtual BOOL OnInterceptTouchEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) OVERRIDE;
    virtual BOOL OnTouchEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) OVERRIDE;

    /** Opt in to claiming gestures that started on a child window. The default
     *  FALSE keeps the legacy "CancelCaptureMode veto" behaviour unchanged for
     *  every existing panel / MVC view. */
    virtual BOOL CanInterceptTouchGesture() const;
    /** Axis fed into the shared detector. AxisVertical for a vertical scroller. */
    virtual SGestureDetector::Axis GetGestureAxis() const;
    /** Whether this panel consumes the drag: axis arbitration + scroll edges. */
    virtual BOOL CanScrollByGesture(const CPoint &ptDelta) const;
    /** Called once when the panel takes a gesture over from a child. */
    virtual void OnGestureIntercepted();
    /** Called when an intercepted stream ends (UP / CANCELMODE). */
    virtual void OnTouchGestureEnd();

    /** TRUE while bVertical can still move in the direction of nDelta. */
    BOOL CanScrollTowardsDrag(BOOL bVertical, int nDelta) const;

    /** Injects touch slop (dp * scale), the axis and the fling threshold. */
    virtual void PrepareGesture();
    /** Aborts an armed/dragging gesture and resets the detector (e.g. on mouse leave). */
    void CancelDragGesture();
    /** WM_CANCELMODE: the container took the gesture away from us. */
    void OnCancelMode();

    /** === Rubber band (marquee) multi-selection support ===
     * When a derived control supports multi-selection, dragging in the client area
     * starts a rubber band selection. The rubber band takes priority over drag
     * scroll / fling (see HandleMouseDrag and CancelCaptureMode).
     */
    /**
     * @brief Checks whether rubber band selection is enabled for this control.
     * @return Derived multi-select capable controls return TRUE when multi-select is on.
     */
    virtual BOOL IsRubberBandSelEnabled() const;

    /**
     * @brief Called once when the rubber band actually starts (drag threshold passed).
     */
    virtual void OnRubberBandStart();

    /**
     * @brief Called on every mouse move while the rubber band is active.
     * @param rcBand Current band rectangle in client coordinates.
     * @param bAdd TRUE when Ctrl is held: items should be added to the current selection.
     */
    virtual void OnRubberBandSelect(const CRect &rcBand, BOOL bAdd);

    /**
     * @brief Called when the rubber band finishes (mouse up) or is cancelled.
     * @param rcBand Final band rectangle in client coordinates.
     * @param bCancelled TRUE when the band was cancelled instead of finished by mouse up.
     */
    virtual void OnRubberBandEnd(const CRect &rcBand, BOOL bCancelled);

  public:
    /**
     * @brief Enables or disables the rubber band gesture at runtime.
     *
     * The rubber band gesture is gated by both this flag (XML attr
     * bandEnable, default TRUE) and the control's multi-select state:
     * only when both are on does dragging start a band. With the band
     * disabled a multi-select control falls back to drag scroll / fling.
     */
    void EnableBandSel(BOOL bEnable)
    {
        m_bBandEnable = bEnable;
    }

    /**
     * @brief Checks whether the rubber band gesture is allowed (bandEnable attr).
     * @return TRUE unless bandEnable="0" was set or EnableBandSel(FALSE) was called.
     */
    BOOL IsBandSelEnabled() const
    {
        return m_bBandEnable;
    }

  private:
    BOOL StartRubberBandSel(const CPoint &pt, BOOL bAdd);
    void UpdateRubberBandSel(const CPoint &pt, BOOL bAdd);
    void UpdateBandAutoScrollTimer();
    void OnBandAutoScroll();
    void EndRubberBandSel(BOOL bCancelled);

  protected:
    /**
     * @brief Handles colorization events.
     * @param cr Color reference.
     */
    virtual void OnColorize(COLORREF cr) OVERRIDE;

    /**
     * @brief Handles scale change events.
     * @param nScale New scale value.
     */
    virtual void OnScaleChanged(int nScale) OVERRIDE;

    /**
     * @brief Handles container change events.
     * @param pOldContainer Pointer to the old container.
     * @param pNewContainer Pointer to the new container.
     */
    virtual void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer) OVERRIDE;

  protected:
    /**
     * @brief Gets the width of the scrollbar.
     * @return Scrollbar width.
     */
    int GetSbWidth() const;

    /**
     * @brief Updates the scrollbar.
     */
    void ScrollUpdate();

    /**
     * @brief start fling animation
     * @param fVelocityX X velocity of the fling.
     * @param fVelocityY Y velocity of the fling.
     */
    void StartFlingAnimation(float fVelocityX, float fVelocityY);

    /**
     * @brief Stops fling animation.
     */
    void StopFlingAnimation();

    /**
     * @brief Scrolls by drag.
     * @param bVert TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param delta Delta value for the scroll.
     * @param fVelocity Velocity of the scroll.
     */
    void ScrollByDrag(BOOL bVert, int delta, float fVelocity);

    /**
     * @brief Clears the drag state.
     */
    void ClearDragState();

    /**
     * @brief Handles the scrollbar skin attribute.
     * @param strValue String value of the attribute.
     * @param bLoading TRUE if loading, FALSE otherwise.
     * @return Result of the attribute handling.
     */
    HRESULT OnAttrScrollbarSkin(SStringW strValue, BOOL bLoading);

  protected:
    SCROLLINFO m_siVer, m_siHoz;           /**< Vertical and horizontal scroll information */
    SAutoRefPtr<SSkinScrollbar> m_pSkinSb; /**< Scrollbar skin */

  private:                      /**< Modified to private, derived classes can only use GetSbArrowSize(), GetSbWidth() to access */
    SLayoutSize m_nSbArrowSize; /**< Arrow size for the scrollbar */
    SLayoutSize m_nSbWid;       /**< Width of the scrollbar */

  protected:
    CRect m_rcClient; /**< Client rectangle */

    SLayoutSize m_nSbLeft;   /**< Distance of the scrollbar from the left */
    SLayoutSize m_nSbRight;  /**< Distance of the scrollbar from the right */
    SLayoutSize m_nSbTop;    /**< Distance of the scrollbar from the top */
    SLayoutSize m_nSbBottom; /**< Distance of the scrollbar from the bottom */

    ScrollBarID m_dragSb, m_hitSb; /**< Dragging and hit scrollbar IDs */

    UINT m_wBarVisible;         /**< Scrollbar visibility information */
    UINT m_wBarEnable;          /**< Scrollbar enable information */
    SScrollBarHandler m_sbVert; /**< Vertical scrollbar handler */
    SScrollBarHandler m_sbHorz; /**< Horizontal scrollbar handler */

    DWORD m_dwUpdateTime;     /**< Time of the last update */
    DWORD m_dwUpdateInterval; /**< Update interval */

    short m_zDelta;     /**< Delta value for mouse wheel */
    int m_nScrollSpeed; /**< Scroll speed */

    mutable SAutoRefPtr<IInterpolator> m_fadeInterpolator; /**< Interpolator for fading */
    int m_fadeFrames;                                      /**< Number of fade frames */
    BYTE m_bySbThumbTrackMinAlpha;                         /**< Minimum alpha value for thumb track */

    /** Drag scroll state: recognition is delegated to SGestureDetector. */
    SGestureDetector m_gesture;    /**< Shared GestureDetector + VelocityTracker */
    BOOL m_bDragPending;           /**< TRUE while armed, waiting for the touch slop */
    BOOL m_bDragScrolling;         /**< TRUE while drag scrolling is active */
    CPoint m_ptDragStart;          /**< Press point, also the rubber band anchor */
    BOOL m_bItemDragScrollEnabled; /**< Enable drag-scroll */
    DWORD m_dwGestureFlags;        /**< wParam of the current MOVE (MK_CONTROL for band) */
    BOOL m_bGestureFling;          /**< detector reported onFling for this gesture */

    /** Fling animation */
    SAutoRefPtr<SFloatAnimator> m_pFlingAnimatorV;
    float m_fFlingVStartPos;
    float m_fFlingVTargetPos;
    SAutoRefPtr<SFloatAnimator> m_pFlingAnimatorH;
    float m_fFlingHStartPos;
    float m_fFlingHTargetPos;

    /** Rubber band (marquee) multi-selection state */
    BOOL m_bRubberBandActive;          /**< TRUE while the rubber band is being dragged */
    BOOL m_bBandEnable;                /**< XML attr bandEnable: whether the rubber band gesture is allowed at all */
    CPoint m_ptBandStart;              /**< Anchor point (client coords) where the band started */
    CPoint m_ptBandAnchorContent;      /**< Anchor point in content coords (client + scroll pos), fixed at band start */
    CRect m_rcBand;                    /**< Current band rectangle in client coordinates */
    SAutoRefPtr<ISkinObj> m_pSkinBand; /**< Band skin (defaults to the builtin _skin.sys.selband); the only band rendering path */
    CPoint m_ptBandLast;               /**< Last raw mouse position during banding (unclamped, for auto scroll) */
    BOOL m_bBandAdd;                   /**< Ctrl (additive) state of the current band session */
    int m_nBandScrollV;                /**< Auto-scroll speed on the vertical axis in lines per tick (0 = off) */
    int m_nBandScrollH;                /**< Auto-scroll speed on the horizontal axis in lines per tick (0 = off) */
    BOOL m_bBandTimerOn;               /**< TRUE while the band auto-scroll timer is running */

    enum
    {
        Timer_BandAutoScroll = 103 /**< 100~102 are reserved by the scrollbar handlers */
    };

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"sbSkin", OnAttrScrollbarSkin)
        ATTR_LAYOUTSIZE(L"sbArrowSize", m_nSbArrowSize, FALSE)
        ATTR_LAYOUTSIZE(L"sbWid", m_nSbWid, TRUE)
        ATTR_INT(L"sbEnable", m_wBarEnable, TRUE)
        ATTR_UINT(L"updateInterval", m_dwUpdateInterval, FALSE)
        ATTR_UINT(L"scrollSpeed", m_nScrollSpeed, FALSE)

        ATTR_LAYOUTSIZE(L"sbLeft", m_nSbLeft, TRUE)
        ATTR_LAYOUTSIZE(L"sbRight", m_nSbRight, TRUE)
        ATTR_LAYOUTSIZE(L"sbTop", m_nSbTop, TRUE)
        ATTR_LAYOUTSIZE(L"sbBottom", m_nSbBottom, TRUE)

        ATTR_INT(L"sbFadeFrames", m_fadeFrames, FALSE)
        ATTR_INT(L"sbTrumbTrackMinAlpha", m_bySbThumbTrackMinAlpha, FALSE)
        ATTR_INTERPOLATOR(L"sbFadeInterpolator", m_fadeInterpolator, FALSE)
        ATTR_CHAIN_PTR_PREFIX(m_fadeInterpolator, 0, SAttrChainPrefix::FADE)
        ATTR_BOOL(L"enableDragScroll", m_bItemDragScrollEnabled, FALSE)
        ATTR_SKIN(L"bandSkin", m_pSkinBand, FALSE)
        ATTR_BOOL(L"bandEnable", m_bBandEnable, FALSE)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Handles the WM_CREATE message.
     * @param lp Create structure.
     * @return Result of the message handling.
     */
    int OnCreate(LPVOID);

    /**
     * @brief Handles the WM_DESTROY message.
     */
    void OnDestroy();

    /**
     * @brief Handles the WM_NCPAINT message.
     * @param pRT Pointer to the render target.
     */
    void OnNcPaint(IRenderTarget *pRT);

    /**
     * @brief Handles the WM_NCHITTEST message.
     * @param pt Point to test.
     * @return Hit test result.
     */
    virtual UINT OnNcHitTest(const CPoint &pt) OVERRIDE;

    /**
     * @brief Handles the WM_NCLBUTTONDOWN message.
     * @param nFlags Flags for the message.
     * @param point Point where the button was pressed.
     */
    void OnNcLButtonDown(UINT nFlags, CPoint point);

    /**
     * @brief Handles the WM_NCLBUTTONUP message.
     * @param nFlags Flags for the message.
     * @param pt Point where the button was released.
     */
    void OnNcLButtonUp(UINT nFlags, CPoint pt);

    /**
     * @brief Handles the WM_NCMOUSEMOVE message.
     * @param nFlags Flags for the message.
     * @param point Point where the mouse moved.
     */
    void OnNcMouseMove(UINT nFlags, CPoint point);

    /**
     * @brief Handles the WM_NCMOUSELEAVE message.
     */
    void OnNcMouseLeave();

    /**
     * @brief Handles the WM_NCCALCSIZE message.
     * @param bCalcValidRects TRUE if valid rectangle calculation is required.
     * @param lParam LPARAM for the message.
     * @return Result of the message handling.
     */
    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);

    /**
     * @brief Handles the WM_MOUSEWHEEL message.
     * @param nFlags Flags for the message.
     * @param zDelta Delta value for the mouse wheel.
     * @param pt Point where the mouse wheel was scrolled.
     * @return Result of the message handling.
     */
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    /**
     * @brief Handles the WM_TIMER message.
     * @param cTimerID Timer identifier.
     */
    void OnTimer(char cTimerID);

    /**
     * @brief Handles the WM_SHOWWINDOW message.
     * @param bShow TRUE if the window is shown, FALSE if hidden.
     * @param nStatus Status of the window.
     */
    void OnShowWindow(BOOL bShow, UINT nStatus);

    /**
     * @brief Handles the WM_ENABLE message.
     * @param bEnable TRUE if the window is enabled, FALSE if disabled.
     * @param uStatus Status of the window.
     */
    void OnEnable(BOOL bEnable, UINT uStatus);

    /**
     * @brief Handles the WM_VSCROLL message.
     * @param nSBCode Scroll code.
     * @param nPos Position for the scroll.
     * @param hwnd Handle to the window.
     */
    void OnVScroll(UINT nSBCode, UINT nPos, HWND hwnd);

    /**
     * @brief Handles the WM_HSCROLL message.
     * @param nSBCode Scroll code.
     * @param nPos Position for the scroll.
     * @param hwnd Handle to the window.
     */
    void OnHScroll(UINT nSBCode, UINT nPos, HWND hwnd);

    /** IAnimatorUpdateListener */
    STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator *pAnimator) OVERRIDE;
    /** SAnimatorListener */
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator *pAnimator) OVERRIDE;

    /**
     * @brief Ordinary MSG_WM_LBUTTONDOWN / MOUSEMOVE / LBUTTONUP handlers.
     *        Derived classes intercept the same messages in their own
     *        SOUI_MSG_MAP and chain with __baseCls::OnLButtonDown/Move/Up().
     * @return TRUE when a drag / fling / rubber band gesture consumed the event;
     *         the derived handler must then return without running its own
     *         click / hover logic. FALSE means a normal click: SPanel has
     *         already called the SWindow default handler.
     */
    BOOL OnLButtonDown(UINT nFlags, CPoint pt);
    BOOL OnMouseMove(UINT nFlags, CPoint pt);
    BOOL OnLButtonUp(UINT nFlags, CPoint pt);
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Checks if drag mode is enabled.
     */
    virtual BOOL IsEnableDragMode() const;

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_NCPAINT_EX(OnNcPaint)
        MSG_WM_NCCALCSIZE(OnNcCalcSize)
        MSG_WM_NCLBUTTONDOWN(OnNcLButtonDown)
        MSG_WM_NCLBUTTONUP(OnNcLButtonUp)
        MSG_WM_NCLBUTTONDBLCLK(OnNcLButtonDown)
        MSG_WM_NCMOUSEMOVE(OnNcMouseMove)
        MSG_WM_NCMOUSELEAVE(OnNcMouseLeave)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_TIMER_EX(OnTimer)
        MSG_WM_SHOWWINDOW(OnShowWindow)
        MSG_WM_ENABLE_EX(OnEnable)
        MSG_WM_VSCROLL(OnVScroll)
        MSG_WM_HSCROLL(OnHScroll)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_CANCELMODE(OnCancelMode)
        MSG_WM_KEYDOWN(OnKeyDown)
    SOUI_MSG_MAP_END()
};

SNSEND

#include <proxy/SPanelProxy.h>

SNSBEGIN

/**
 * @class     SScrollView
 * @brief     Scrollable Panel
 *
 * Description: Implements a scrollable panel that supports scrolling content.
 */
class SOUI_EXP SScrollView : public TPanelProxy<IScrollView> {
    DEF_SOBJECT(SPanel, L"scrollview")

  public:
    /**
     * @brief Constructor for SScrollView.
     */
    SScrollView();

    /**
     * @brief Destructor for SScrollView.
     */
    virtual ~SScrollView()
    {
    }

  public:
    /**
     * @brief Gets the size of the view.
     * @param szView Pointer to receive the view size.
     */
    STDMETHOD_(void, GetViewSize)(CTHIS_ SIZE *szView) SCONST OVERRIDE;

    /**
     * @brief Sets the size of the view.
     * @param szView New size of the view.
     */
    STDMETHOD_(void, SetViewSize)(THIS_ SIZE szView) OVERRIDE;

    /**
     * @brief Gets the origin of the view.
     * @param ptOri Pointer to receive the origin of the view.
     */
    STDMETHOD_(void, GetViewOrigin)(CTHIS_ POINT *ptOri) SCONST OVERRIDE;

    /**
     * @brief Sets the origin of the view.
     * @param pt New origin of the view.
     */
    STDMETHOD_(void, SetViewOrigin)(THIS_ POINT pt) OVERRIDE;

  protected:
    /**
     * @brief Called when the view size changes.
     * @param szOld Old size of the view.
     * @param szNew New size of the view.
     */
    virtual void OnViewSizeChanged(CSize szOld, CSize szNew);

    /**
     * @brief Called when the view origin changes.
     * @param ptOld Old origin of the view.
     * @param ptNew New origin of the view.
     */
    virtual void OnViewOriginChanged(CPoint ptOld, CPoint ptNew);

  protected:
    /**
     * @brief Gets the layout rectangle of the children.
     * @param prc Pointer to receive the layout rectangle.
     */
    STDMETHOD_(void, GetChildrenLayoutRect)(THIS_ RECT *prc) SCONST OVERRIDE;

    /**
     * @brief Updates the position of the children.
     */
    STDMETHOD_(void, UpdateChildrenPosition)(THIS) OVERRIDE;

    /**
     * @brief Handles scroll events.
     * @param bVertical TRUE for vertical scrollbar, FALSE for horizontal scrollbar.
     * @param uCode Scroll code.
     * @param nPos Position for the scroll.
     * @return TRUE if handled, FALSE otherwise.
     */
    virtual BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos) OVERRIDE;

    /**
     * @brief Updates the scrollbar.
     */
    virtual void UpdateScrollBar();

  protected:
    /**
     * @brief Handles the view size attribute.
     * @param strValue String value of the attribute.
     * @param bLoading TRUE if loading, FALSE otherwise.
     * @return Result of the attribute handling.
     */
    HRESULT OnAttrViewSize(const SStringW &strValue, BOOL bLoading);

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"origin-x", m_ptOrigin.x, FALSE)
        ATTR_INT(L"origin-y", m_ptOrigin.y, FALSE)
        ATTR_CUSTOM(L"viewSize", OnAttrViewSize)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Updates the view size.
     */
    void UpdateViewSize();

    /**
     * @brief Handles the WM_SIZE message.
     * @param nType Type of size change.
     * @param size New size of the window.
     */
    void OnSize(UINT nType, CSize size);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_SIZE(OnSize)
    SOUI_MSG_MAP_END()

  protected:
    SLayoutSize m_viewSize[2]; /**< View size in two dimensions */
    CPoint m_ptOrigin;         /**< Origin of the view */
    SIZE m_szView;             /**< Current size of the view */
};

SNSEND

#endif /**< __SPANEL__H__ */
