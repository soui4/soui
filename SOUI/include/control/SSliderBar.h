/**
 * Copyright (C) 2014-2050 SOUI Team
 * All rights reserved.
 *
 * @file       SSliderBar.h
 * @brief
 * @version    v1.0
 * @author     soui
 * @date       2014-07-08
 *
 * Describe
 */
#ifndef __SSLIDERBAR__H__
#define __SSLIDERBAR__H__
#include <control/SCmnCtrl.h>
#include <valueAnimator/SValueAnimator.h>
#include <interface/SCtrls-i.h>
SNSBEGIN

template <class T>
class TProgressProxy
    : public T
    , public SProgress {
  public:
    TProgressProxy()
    {
    }

    STDMETHOD_(long, AddRef)(THIS) OVERRIDE
    {
        return SProgress::AddRef();
    }
    STDMETHOD_(long, Release)(THIS) OVERRIDE
    {
        return SProgress::Release();
    }
    STDMETHOD_(void, OnFinalRelease)(THIS) OVERRIDE
    {
        SProgress::OnFinalRelease();
    }

    STDMETHOD_(IWindow *, ToIWindow)(THIS) OVERRIDE
    {
        return this;
    }

    STDMETHOD_(HRESULT, QueryInterface)(REFGUID id, IObjRef **ppRet) OVERRIDE
    {
        if (id == T::GetIID())
        {
            *ppRet = (T *)this;
            AddRef();
            return S_OK;
        }
        else
        {
            return SProgress::QueryInterface(id, ppRet);
        }
    }
    STDMETHOD_(BOOL, SetValue)(THIS_ int nValue) OVERRIDE
    {
        return SProgress::SetValue(nValue);
    }

    STDMETHOD_(int, GetValue)(CTHIS) SCONST OVERRIDE
    {
        return SProgress::GetValue();
    }

    STDMETHOD_(void, SetRange)(THIS_ int nMin, int nMax) OVERRIDE
    {
        SProgress::SetRange(nMin, nMax);
    }

    STDMETHOD_(void, GetRange)(CTHIS_ int *pMin, int *pMax) SCONST OVERRIDE
    {
        SProgress::GetRange(pMin, pMax);
    }

    STDMETHOD_(BOOL, IsVertical)(CTHIS) SCONST OVERRIDE
    {
        return SProgress::IsVertical();
    }
};

/**
 * @class     SSliderBar
 * @brief     Slider toolbar
 *
 * Describe   Slider toolbar
 */
class SOUI_EXP SSliderBar
    : public TProgressProxy<ISliderBar>
    , public IAnimatorUpdateListener
    , public SAnimatorListener {
    DEF_SOBJECT(SProgress, L"sliderbar")

    struct RANGE
    {
        int value1;
        int value2;
    };

  public:
    /**
     * SSliderBar::SSliderBar
     * @brief    Constructor
     *
     * Describe  Constructor
     */
    SSliderBar();

    /**
     * SSliderBar::~SSliderBar
     * @brief    Destructor
     *
     * Describe  Destructor
     */
    ~SSliderBar();

    enum
    {
        SC_RAIL = PC_RAIL,
        SC_SELECT = PC_SELECT,
        SC_THUMB,
        SC_RAILBACK,
        SC_NULL = -1,
    };

    STDMETHOD_(BOOL, SetValue)(THIS_ int nValue) OVERRIDE;

    /**
     * @brief Set segment information
     * @param segments Segment array
     * @param count Segment count
     * @details Segments must be arranged in ascending order
     */
    STDMETHOD_(void, SetSegments)(THIS_ const SEGMENT *segments, int count) OVERRIDE;

    /**
     * @brief Get the number of segments
     * @return Segment count
     */
    STDMETHOD_(int, GetSegmentCount)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Get the segment information at the specified index
     * @param index Segment index
     * @param pSegment Output segment information
     * @return Returns TRUE on success
     */
    STDMETHOD_(BOOL, GetSegment)(CTHIS_ int index, SEGMENT *pSegment) SCONST OVERRIDE;

    /**
     * @brief Find the segment containing the given value
     * @param value Progress value
     * @return Segment index, -1 if not found
     */
    STDMETHOD_(int, FindSegmentByValue)(CTHIS_ int value) SCONST OVERRIDE;

  protected:
    BOOL m_bDrag;     /**< Whether dragging is allowed */
    CPoint m_ptDrag;  /**< Drag position     */
    int m_nDragValue; /**< Drag distance */
    int m_uHtPrev;    /**< Last mouse position */

    SAutoRefPtr<ISkinObj> m_pSkinThumb; /**< Skin object */
    BOOL m_bThumbInRail;                /**< Slider is contained in the track */
    BOOL m_bDrawRail;                   /**< Draw track */
    BOOL m_bDrawValue;                  /**< Draw slider value */
    BOOL m_bDragTip;                    /**< Show value in tip while dragging the slider */

    BYTE m_byThumbAlphaAni;                /**< Slider opacity animation value */
    SAutoRefPtr<SByteAnimator> m_thumbAni; /**< Slider opacity animator */
    SAutoRefPtr<SIntAnimator> m_valueAni;  /**< Value animator */

    BOOL m_bEnableAnimate;      /**< Enable animation flag */
    SArray<SEGMENT> m_segments; /**< Segment array */
    COLORREF m_crSep;           /**< Divider line color */
  protected:
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator *pAnimator) OVERRIDE;
    STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator *pAnimator) OVERRIDE;

  protected:
    SIZE MeasureContent(int nParentWid, int nParentHei) override;
    void OnColorize(COLORREF cr) override;
    void OnScaleChanged(int scale) override;
    BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo) override;
    void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer) override;

    void DrawRail(IRenderTarget *pRT, const CRect &rcClient) override;
    void DrawPos(IRenderTarget *pRT, const CRect &rcClient) override;
    void DrawOthers(IRenderTarget *pRT, const CRect &rcClient) override;
    void OnValueChanged(int nValue, int reason) override;

    CRect GetPartRect(const CRect &rcClient, UINT uSBCode) const override;
    RANGE _GetPartRange(int nLength, int nThumbSize, BOOL bThumbInRail, int nMin, int nMax, int nValue, UINT uSBCode) const;
    BOOL GetSegmentRect(int iSeg, RECT &rc) const;
    CRect _GetSegmentRect(const CRect &rcRail, int r1, int r2) const;
    int HitTest(CPoint pt);

    void ShowValueInTip(int nValue);

    LRESULT NotifyPos(SliderBarAction action, int value);

  protected:
    /**
     * SSliderBar::OnLButtonUp
     * @brief    Left button up
     * @param    UINT nFlags --  Virtual key flags
     * @param    CPoint point -- Coordinates
     *
     * Describe  Message handler function
     */
    void OnLButtonUp(UINT nFlags, CPoint point);

    /**
     * SSliderBar::OnLButtonDown
     * @brief    Left button down
     * @param    UINT nFlags --  Virtual key flags
     * @param    CPoint point -- Coordinates
     *
     * Describe  Message handler function
     */
    void OnLButtonDown(UINT nFlags, CPoint point);

    /**
     * SSliderBar::OnMouseMove
     * @brief    Mouse move
     * @param    UINT nFlags --  Virtual key flags
     * @param    CPoint point -- Coordinates
     *
     * Describe  Message handler function
     */
    void OnMouseMove(UINT nFlags, CPoint point);

    /**
     * SSliderBar::OnMouseHover
     * @brief    Mouse move event
     * @param    UINT nFlags --  Virtual key flags
     * @param    CPoint point -- Coordinates
     *
     * Describe  Message handler function
     */
    void OnMouseHover(UINT nFlags, CPoint point);

    /**
     * SSliderBar::OnMouseLeave
     * @brief    Mouse move event
     *
     * Describe  Message handler function
     */
    void OnMouseLeave();

    void OnDestroy();

    virtual BOOL CancelCaptureMode(int reason) override;

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSEHOVER(OnMouseHover)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_DESTROY(OnDestroy)
    SOUI_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"thumbSkin", m_pSkinThumb, TRUE)
        ATTR_BOOL(L"thumbInRail", m_bThumbInRail, TRUE)
        ATTR_BOOL(L"drawRail", m_bDrawRail, TRUE)
        ATTR_BOOL(L"drawValue", m_bDrawValue, TRUE)
        ATTR_BOOL(L"dragTip", m_bDragTip, TRUE)
        ATTR_BOOL(L"animate", m_bEnableAnimate, FALSE)
        ATTR_COLOR(L"sepColor", m_crSep, TRUE)
        ATTR_CHAIN_PREFIX(*m_thumbAni, 0, L"thumb:")
        ATTR_CHAIN_PREFIX(*m_valueAni, 0, L"value:")
    SOUI_ATTRS_END()
};

SNSEND

#endif /**< __SSLIDERBAR__H__ */