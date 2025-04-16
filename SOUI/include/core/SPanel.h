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
 #include <core/SScrollBarHandler.h>
 #include <interface/SCtrl-i.h>
 #include <proxy/SWindowProxy.h>
 
 SNSBEGIN
 
 /**
  * @class     SPanel
  * @brief     Panel with Scrollbar Support
  *
  * Description: Implements a panel with support for scrollbars in the non-client area.
  */
 class SOUI_EXP SPanel
     : public TWindowProxy<IPanel>
     , protected IScrollBarHost {
     DEF_SOBJECT(SWindow, L"div")
 
 public:
     /**
      * @brief Constructor for SPanel.
      */
     SPanel();
 
     /**
      * @brief Destructor for SPanel.
      */
     virtual ~SPanel() {}
 
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
 
 public:
     /**
      * @brief Gets the client rectangle.
      * @param pRect Pointer to receive the client rectangle.
      */
     virtual void WINAPI GetClientRect(LPRECT pRect) const;
 
     /**
      * @brief Gets the client rectangle.
      * @return Client rectangle.
      */
     virtual CRect GetClientRect() const;
 
 protected:
     // Inherited from IScrollBarHost
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
 
     /**
      * @brief Handles colorization events.
      * @param cr Color reference.
      */
     virtual void OnColorize(COLORREF cr);
 
     /**
      * @brief Handles scale change events.
      * @param nScale New scale value.
      */
     virtual void OnScaleChanged(int nScale);
 
     /**
      * @brief Handles container change events.
      * @param pOldContainer Pointer to the old container.
      * @param pNewContainer Pointer to the new container.
      */
     virtual void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer);
 
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
      * @brief Handles the scrollbar skin attribute.
      * @param strValue String value of the attribute.
      * @param bLoading TRUE if loading, FALSE otherwise.
      * @return Result of the attribute handling.
      */
     HRESULT OnAttrScrollbarSkin(SStringW strValue, BOOL bLoading);
 
 protected:
     SCROLLINFO m_siVer, m_siHoz; // Vertical and horizontal scroll information
     SAutoRefPtr<SSkinScrollbar> m_pSkinSb; // Scrollbar skin
 
 private: // Modified to private, derived classes can only use GetSbArrowSize(), GetSbWidth() to access
     SLayoutSize m_nSbArrowSize; // Arrow size for the scrollbar
     SLayoutSize m_nSbWid; // Width of the scrollbar
 
 protected:
     CRect m_rcClient; // Client rectangle
 
     SLayoutSize m_nSbLeft;   // Distance of the scrollbar from the left
     SLayoutSize m_nSbRight;  // Distance of the scrollbar from the right
     SLayoutSize m_nSbTop;    // Distance of the scrollbar from the top
     SLayoutSize m_nSbBottom; // Distance of the scrollbar from the bottom
 
     ScrollBarID m_dragSb, m_hitSb; // Dragging and hit scrollbar IDs
 
     UINT m_wBarVisible; // Scrollbar visibility information
     UINT m_wBarEnable;  // Scrollbar enable information
     SScrollBarHandler m_sbVert; // Vertical scrollbar handler
     SScrollBarHandler m_sbHorz; // Horizontal scrollbar handler
 
     DWORD m_dwUpdateTime; // Time of the last update
     DWORD m_dwUpdateInterval; // Update interval
 
     short m_zDelta; // Delta value for mouse wheel
     int m_nScrollSpeed; // Scroll speed
 
     mutable SAutoRefPtr<IInterpolator> m_fadeInterpolator; // Interpolator for fading
     int m_fadeFrames; // Number of fade frames
     BYTE m_bySbThumbTrackMinAlpha; // Minimum alpha value for thumb track
 
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
         ATTR_CHAIN_PTR(m_fadeInterpolator, 0)
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
     virtual BOOL OnNcHitTest(CPoint pt);
 
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
     virtual ~SScrollView() {}
 
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
     virtual BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos);
 
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
     SLayoutSize m_viewSize[2]; // View size in two dimensions
     CPoint m_ptOrigin; // Origin of the view
     SIZE m_szView; // Current size of the view
 };
 
 SNSEND
 
 #endif // __SPANEL__H__