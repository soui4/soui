/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserved.
 *
 * @file       SScrollBar.h
 * @brief      Scrollbar control
 * @version    v1.0
 * @author     soui
 * @date       2014-07-08
 *
 * Describe    Scrollbar control implementation
 */
#ifndef __SSCROLLBAR__H__
#define __SSCROLLBAR__H__

#include <core/SWnd.h>
#include <core/SScrollBarHandler.h>

SNSBEGIN

/**
 * @class     SScrollBar
 * @brief     Scrollbar control
 *
 * Describe    This class implements a scrollbar control.
 */
class SOUI_EXP SScrollBar
    : public SWindow
    , protected IScrollBarHost {
    DEF_SOBJECT(SWindow, L"scrollbar")

  public:
    /**
     * SScrollBar::SScrollBar
     * @brief    Constructor
     *
     * Describe  Initializes the scrollbar control.
     */
    SScrollBar();

    /**
     * SScrollBar::~SScrollBar
     * @brief    Destructor
     *
     * Describe  Cleans up the scrollbar control.
     */
    virtual ~SScrollBar();

  public:
    /**
     * SScrollBar::IsVertical
     * @brief    Checks if the scrollbar is vertical
     * @return   Returns TRUE if the scrollbar is vertical, FALSE otherwise
     *
     * Describe  Determines the orientation of the scrollbar.
     */
    BOOL IsVertical() const;

    /**
     * SScrollBar::SetPos
     * @brief    Sets the position of the scrollbar
     * @param    int nPos -- New position of the scrollbar
     * @return   Returns the new position of the scrollbar
     *
     * Describe  Updates the position of the scrollbar thumb.
     */
    int SetPos(int nPos);

    /**
     * SScrollBar::GetPos
     * @brief    Gets the current position of the scrollbar
     * @return   Returns the current position of the scrollbar
     *
     * Describe  Retrieves the current position of the scrollbar thumb.
     */
    int GetPos();

    /**
     * SScrollBar::GetMax
     * @brief    Gets the maximum position of the scrollbar
     * @return   Returns the maximum position of the scrollbar
     *
     * Describe  Retrieves the maximum position of the scrollbar thumb.
     */
    int GetMax();

    /**
     * SScrollBar::GetMin
     * @brief    Gets the minimum position of the scrollbar
     * @return   Returns the minimum position of the scrollbar
     *
     * Describe  Retrieves the minimum position of the scrollbar thumb.
     */
    int GetMin();

  protected:
    STDMETHOD_(CRect, GetScrollBarRect)(CTHIS_ BOOL bVert) SCONST OVERRIDE;
    STDMETHOD_(ISkinObj *, GetScrollBarSkin)(CTHIS_ BOOL bVert) SCONST OVERRIDE;
    STDMETHOD_(const SCROLLINFO *, GetScrollBarInfo)(CTHIS_ BOOL bVert) SCONST OVERRIDE;
    STDMETHOD_(int, GetScrollBarArrowSize)(CTHIS_ BOOL bVert) SCONST OVERRIDE;
    STDMETHOD_(void, OnScrollUpdatePart)(THIS_ BOOL bVert, int iPart) OVERRIDE;
    STDMETHOD_(void, OnScrollUpdateThumbTrack)(THIS_ BOOL bVert, int nPos) OVERRIDE;
    STDMETHOD_(ISwndContainer *, GetScrollBarContainer)(THIS) OVERRIDE;
    STDMETHOD_(BOOL, IsScrollBarEnable)(THIS_ BOOL bVertical) SCONST OVERRIDE;
    STDMETHOD_(void, OnScrollCommand)(THIS_ BOOL bVert, int iCmd, int nPos) OVERRIDE;
    STDMETHOD_(void, OnScrollSetTimer)(THIS_ BOOL bVert, char id, UINT uElapse) OVERRIDE;
    STDMETHOD_(void, OnScrollKillTimer)(THIS_ BOOL bVert, char id) OVERRIDE;
    STDMETHOD_(const IInterpolator *, GetScrollInterpolator)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetScrollFadeFrames)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(BYTE, GetScrollThumbTrackMinAlpha)(CTHIS) SCONST OVERRIDE;

  protected:
    /**
     * SScrollBar::OnInitFinished
     * @brief    Initialization after creation
     * @param    IXmlNode *pNode -- XML node containing initialization data
     *
     * Describe  Performs additional initialization after the control is created.
     */
    virtual void WINAPI OnInitFinished(IXmlNode *pNode);

    /**
     * SScrollBar::NotifySbCode
     * @brief    Notifies the parent window of scrollbar events
     * @param    int nCode -- Scrollbar notification code
     * @param    int nPos -- Current position of the scrollbar
     *
     * Describe  Sends notifications to the parent window about scrollbar events.
     */
    void NotifySbCode(int nCode, int nPos);

  protected:
    HRESULT OnAttrVertical(const SStringW &value, BOOL bLoading);

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"skin", m_pSkin, TRUE)                                 /**< Skin object */
        ATTR_UINT(L"arrowSize", m_uAllowSize, TRUE)                       /**< Arrow size */
        ATTR_INT(L"min", m_si.nMin, TRUE)                                 /**< Minimum position */
        ATTR_INT(L"max", m_si.nMax, TRUE)                                 /**< Maximum position */
        ATTR_INT(L"value", m_si.nPos, TRUE)                               /**< Current position */
        ATTR_INT(L"page", m_si.nPage, TRUE)                               /**< Page size */
        ATTR_CUSTOM(L"vertical", OnAttrVertical)                          /**< Vertical orientation */
        ATTR_INT(L"fadeFrames", m_fadeFrames, FALSE)                      /**< Fade frames */
        ATTR_INT(L"thumbTrackMinAlpha", m_byThumbTrackMinAlpha, FALSE)    /**< Minimum alpha for thumb tracking */
        ATTR_INTERPOLATOR(L"fadeInterpolator", m_fadeInterpolator, FALSE) /**< Fade interpolator */
        ATTR_CHAIN_PTR(m_fadeInterpolator, 0)                             /**< Chain pointer for fade interpolator */
    SOUI_ATTRS_END()

  protected:
    /**
     * SScrollBar::OnPaint
     * @brief    Paints the scrollbar
     * @param    IRenderTarget *pRT -- Rendering target
     *
     * Describe  Handles the painting of the scrollbar.
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * SScrollBar::OnLButtonUp
     * @brief    Handles left button up event
     * @param    UINT nFlags -- Virtual key flags
     * @param    CPoint point -- Mouse position
     *
     * Describe  Handles the left mouse button up event.
     */
    void OnLButtonUp(UINT nFlags, CPoint point);

    /**
     * SScrollBar::OnLButtonDown
     * @brief    Handles left button down event
     * @param    UINT nFlags -- Virtual key flags
     * @param    CPoint point -- Mouse position
     *
     * Describe  Handles the left mouse button down event.
     */
    void OnLButtonDown(UINT nFlags, CPoint point);

    /**
     * SScrollBar::OnMouseMove
     * @brief    Handles mouse move event
     * @param    UINT nFlags -- Virtual key flags
     * @param    CPoint point -- Mouse position
     *
     * Describe  Handles the mouse move event.
     */
    void OnMouseMove(UINT nFlags, CPoint point);

    /**
     * SScrollBar::OnTimer
     * @brief    Handles timer event
     * @param    char nIDEvent -- Timer ID
     *
     * Describe  Handles the timer event.
     */
    void OnTimer(char nIDEvent);

    /**
     * SScrollBar::OnMouseHover
     * @brief    Handles mouse hover event
     * @param    UINT nFlags -- Virtual key flags
     * @param    CPoint ptPos -- Mouse position
     *
     * Describe  Handles the mouse hover event.
     */
    void OnMouseHover(UINT nFlags, CPoint ptPos);

    /**
     * SScrollBar::OnMouseLeave
     * @brief    Handles mouse leave event
     *
     * Describe  Handles the mouse leave event.
     */
    void OnMouseLeave();

    /**
     * SScrollBar::OnSetScrollInfo
     * @brief    Handles setting scroll information
     * @param    UINT uMsg -- Message code
     * @param    WPARAM wParam -- Additional message-specific information
     * @param    LPARAM lParam -- Additional message-specific information
     * @return   Returns the result of the message processing
     *
     * Describe  Handles the setting of scroll information.
     */
    LRESULT OnSetScrollInfo(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * SScrollBar::OnGetScrollInfo
     * @brief    Handles getting scroll information
     * @param    UINT uMsg -- Message code
     * @param    WPARAM wParam -- Additional message-specific information
     * @param    LPARAM lParam -- Additional message-specific information
     * @return   Returns the result of the message processing
     *
     * Describe  Handles the retrieval of scroll information.
     */
    LRESULT OnGetScrollInfo(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * SScrollBar::OnDestroy
     * @brief    Handles destruction of the control
     *
     * Describe  Cleans up resources when the control is destroyed.
     */
    void OnDestroy();

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_MOUSEHOVER(OnMouseHover)
        MSG_WM_TIMER_EX(OnTimer)
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MESSAGE_HANDLER_EX(SBM_SETSCROLLINFO, OnSetScrollInfo)
        MESSAGE_HANDLER_EX(SBM_GETSCROLLINFO, OnGetScrollInfo)
    SOUI_MSG_MAP_END()

  protected:
    SAutoRefPtr<ISkinObj> m_pSkin; /**< Skin object */
    UINT m_uAllowSize;             /**< Arrow size */

    SCROLLINFO m_si; /**< Scroll information structure */

    int m_fadeFrames;                                      /**< Number of fade frames */
    mutable SAutoRefPtr<IInterpolator> m_fadeInterpolator; /**< Fade interpolator */
    BYTE m_byThumbTrackMinAlpha;                           /**< Minimum alpha for thumb tracking */
    SScrollBarHandler m_sbHandler;                         /**< Scrollbar handler */
};

SNSEND

#endif // __SSCROLLBAR__H__