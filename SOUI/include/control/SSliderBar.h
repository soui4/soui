/**
 * Copyright (C) 2014-2050 SOUI团队
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
    STDMETHOD_(BOOL, SetValue)(THIS_ int nValue)
    {
        return SProgress::SetValue(nValue);
    }

    STDMETHOD_(int, GetValue)(CTHIS) SCONST
    {
        return SProgress::GetValue();
    }

    STDMETHOD_(void, SetRange)(THIS_ int nMin, int nMax)
    {
        SProgress::SetRange(nMin, nMax);
    }

    STDMETHOD_(void, GetRange)(CTHIS_ int *pMin, int *pMax) SCONST
    {
        SProgress::GetRange(pMin, pMax);
    }

    STDMETHOD_(BOOL, IsVertical)(CTHIS) SCONST
    {
        return SProgress::IsVertical();
    }
};

/**
 * @class     SSliderBar
 * @brief     滑块工具条
 *
 * Describe   滑块工具条
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
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    SSliderBar();

    /**
     * SSliderBar::~SSliderBar
     * @brief    析构函数
     *
     * Describe  析构函数
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
     * @brief 设置分段信息
     * @param segments 分段数组
     * @param count 分段数量
     * @details 分段必须从小到大排列
     */
    STDMETHOD_(void, SetSegments)(THIS_ const SEGMENT *segments, int count) OVERRIDE;

    /**
     * @brief 获取分段数
     * @return 分段数量
     */
    STDMETHOD_(int, GetSegmentCount)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief 获取指定索引的分段信息
     * @param index 分段索引
     * @param pSegment 输出分段信息
     * @return 成功返回TRUE
     */
    STDMETHOD_(BOOL, GetSegment)(CTHIS_ int index, SEGMENT *pSegment) SCONST OVERRIDE;

    /**
     * @brief 根据值查找所在的分段
     * @param value 进度值
     * @return 分段索引，-1表示未找到
     */
    STDMETHOD_(int, FindSegmentByValue)(CTHIS_ int value) SCONST OVERRIDE;

  protected:
    BOOL m_bDrag;     /**< 是否允许拖动 */
    CPoint m_ptDrag;  /**< 拖动位置     */
    int m_nDragValue; /**< 拖动距离 */
    int m_uHtPrev;    /**< 上次的鼠标位置 */

    SAutoRefPtr<ISkinObj> m_pSkinThumb; /**< 皮肤对象 */
    BOOL m_bThumbInRail;                /**< 滑块包含在轨道中 */
    BOOL m_bDrawRail;                   /**< 绘制轨道 */
    BOOL m_bDragTip;                    /**< 拖动滑块时在tip中显示数值 */

    BYTE m_byThumbAlphaAni;                /**< 滑块透明度动画值 */
    SAutoRefPtr<SByteAnimator> m_thumbAni; /**< 滑块透明度动画器 */
    SAutoRefPtr<SIntAnimator> m_valueAni;  /**< 数值动画器 */

    BOOL m_bEnableAnimate;      /**< 启用动画标志 */
    SArray<SEGMENT> m_segments; /**< 分段数组 */
    COLORREF m_crSep;           /**< 分割线颜色 */
  protected:
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator *pAnimator) OVERRIDE;
    STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator *pAnimator) OVERRIDE;

  protected:
    STDMETHOD_(SIZE, GetDesiredSize)(THIS_ int nParentWid, int nParentHei);

    void OnColorize(COLORREF cr) override;
    void OnScaleChanged(int scale) override;
    SStringT GetTooltip(CPoint pt) const override;
    void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer) override;

    void DrawPos(IRenderTarget *pRT, const CRect &rcClient) override;
    void DrawOthers(IRenderTarget *pRT, const CRect &rcClient) override;
    void OnValueChanged(int nValue, int reason) override;

    CRect GetPartRect(const CRect &rcClient, UINT uSBCode) const override;
    RANGE _GetPartRange(int nLength, int nThumbSize, BOOL bThumbInRail, int nMin, int nMax, int nValue, UINT uSBCode) const;

    int HitTest(CPoint pt);

    void ShowValueInTip(int nValue);

    LRESULT NotifyPos(SliderBarAction action, int value);

  protected:
    /**
     * SSliderBar::OnLButtonUp
     * @brief    左键抬起
     * @param    UINT nFlags --  虚拟键标志
     * @param    CPoint point -- 坐标
     *
     * Describe  消息响应函数
     */
    void OnLButtonUp(UINT nFlags, CPoint point);

    /**
     * SSliderBar::OnLButtonDown
     * @brief    左键按下
     * @param    UINT nFlags --  虚拟键标志
     * @param    CPoint point -- 坐标
     *
     * Describe  消息响应函数
     */
    void OnLButtonDown(UINT nFlags, CPoint point);

    /**
     * SSliderBar::OnMouseMove
     * @brief    鼠标移动
     * @param    UINT nFlags --  虚拟键标志
     * @param    CPoint point -- 坐标
     *
     * Describe  消息响应函数
     */
    void OnMouseMove(UINT nFlags, CPoint point);

    /**
     * SSliderBar::OnMouseHover
     * @brief    鼠标移动事件
     * @param    UINT nFlags --  虚拟键标志
     * @param    CPoint point -- 坐标
     *
     * Describe  消息响应函数
     */
    void OnMouseHover(UINT nFlags, CPoint point);

    /**
     * SSliderBar::OnMouseLeave
     * @brief    鼠标移动事件
     *
     * Describe  消息响应函数
     */
    void OnMouseLeave();

    void OnDestroy();

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
        ATTR_BOOL(L"dragTip", m_bDragTip, TRUE)
        ATTR_BOOL(L"animate", m_bEnableAnimate, FALSE)
        ATTR_COLOR(L"sepColor", m_crSep, TRUE)
        ATTR_CHAIN_PREFIX(*m_thumbAni, 0, L"thumb:")
        ATTR_CHAIN_PREFIX(*m_valueAni, 0, L"value:")
    SOUI_ATTRS_END()
};

SNSEND

#endif // __SSLIDERBAR__H__