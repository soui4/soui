/**
 * @file SPropertyAnimator.h
 * @brief SOUI布局对象动画器实现 - 参考Android ObjectAnimator设计
 * @author SOUI group
 * @date 2024
 */

#ifndef __SLAYOUT_OBJECT_ANIMATOR__H__
#define __SLAYOUT_OBJECT_ANIMATOR__H__

#include <interface/slayout-i.h>
#include <interface/SPropertyAnimator-i.h>
#include <layout/SLayoutSize.h>
#include <helper/obj-ref-impl.hpp>
#include <proxy/SValueAnimatorProxy.h>

SNSBEGIN

typedef struct _PROPVALUE
{
    PROP_TYPE nType;
    union {
        BYTE *bValue;
        COLORREF *crValue;
        int *nValue;
        short *sValue;
        float *fValue;
        LAYOUTSIZE *pLayoutSize;
        void *pValue;
    };
    float *pWeights;
} PROPVALUE;

/**
 * @class SPropertyValuesHolder
 * @brief 属性值持有者实现类 - 类似Android PropertyValuesHolder
 */
class SOUI_EXP SPropertyValuesHolder : public TObjRefImpl<IPropertyValuesHolder> {
  public:
    SPropertyValuesHolder();
    virtual ~SPropertyValuesHolder();

    // IPropertyValuesHolder接口实现
    STDMETHOD_(void, SetPropertyName)(THIS_ LPCWSTR propertyName) OVERRIDE;
    STDMETHOD_(LPCWSTR, GetPropertyName)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(void, SetByteValues)(THIS_ const BYTE *values, int count) OVERRIDE;
    STDMETHOD_(void, SetShortValues)(THIS_ const short *values, int count) OVERRIDE;
    STDMETHOD_(void, SetColorRefValues)(THIS_ const COLORREF *values, int count) OVERRIDE;
    STDMETHOD_(void, SetFloatValues)(THIS_ const float *values, int count) OVERRIDE;
    STDMETHOD_(void, SetIntValues)(THIS_ const int *values, int count) OVERRIDE;
    STDMETHOD_(void, SetLayoutSizeValues)(THIS_ const LAYOUTSIZE *values, int count) OVERRIDE;
    STDMETHOD_(void, SetPositionValues)(THIS_ const void *values, int count, int valueSize) OVERRIDE;
    STDMETHOD_(BOOL, GetAnimatedValue)(CTHIS_ float fraction, void *pValue) SCONST OVERRIDE;
    STDMETHOD_(PROP_TYPE, GetValueType)(CTHIS) SCONST OVERRIDE
    {
        return m_value.nType;
    }
    STDMETHOD_(int, GetValueSize)(CTHIS) SCONST OVERRIDE
    {
        return m_valueSize;
    }
    STDMETHOD_(void, GetStartValue)(CTHIS_ void *pValue) SCONST OVERRIDE;
    STDMETHOD_(void, GetEndValue)(CTHIS_ void *pValue) SCONST OVERRIDE;
    STDMETHOD_(float, Fraction2Index)(CTHIS_ float fraction, int idx[2]) SCONST OVERRIDE;
    STDMETHOD_(BOOL, GetValueByIndex)(CTHIS_ int index, void *pValue, int valueSize) SCONST OVERRIDE;
    STDMETHOD_(int, GetKeyframeCount)(CTHIS) SCONST OVERRIDE
    {
        return m_valueCount;
    }
    STDMETHOD_(BOOL, SetKeyFrameWeights)(THIS_ const float *weights, int count) OVERRIDE;
    STDMETHOD_(BOOL, GetKeyFrameWeights)(CTHIS_ float *weights, int count) SCONST OVERRIDE;

    // 静态工厂方法
    static SPropertyValuesHolder *ofByte(LPCWSTR propertyName, const BYTE *values, int count);
    static SPropertyValuesHolder *ofShort(LPCWSTR propertyName, const short *values, int count);
    static SPropertyValuesHolder *ofColorRef(LPCWSTR propertyName, const COLORREF *values, int count);
    static SPropertyValuesHolder *ofFloat(LPCWSTR propertyName, const float *values, int count);
    static SPropertyValuesHolder *ofInt(LPCWSTR propertyName, const int *values, int count);
    static SPropertyValuesHolder *ofLayoutSize(LPCWSTR propertyName, const LAYOUTSIZE *values, int count);
    static SPropertyValuesHolder *ofPosition(LPCWSTR propertyName, const void *values, int count, int valueSize);

  private:
    SStringW m_propertyName;
    PROPVALUE m_value;

    int m_valueCount;
    float m_totalWeight;
    int m_valueSize; // 用于位置值的大小

    void ClearValues();

    BYTE InterpolateByte(float fraction) const;
    short InterpolateShort(float fraction) const;
    COLORREF InterpolateColorRef(float fraction) const;
    float InterpolateFloat(float fraction) const;
    int InterpolateInt(float fraction) const;
    SLayoutSize InterpolateLayoutSize(float fraction) const;
};

/**
 * @class SPropertyAnimator
 * @brief 属性动画器实现类
 */
class SOUI_EXP SPropertyAnimator
    : public TValueAnimatorProxy<IPropertyAnimator>
    , protected IAnimatorListener {
    DEF_SOBJECT(SValueAnimator, L"propertyAnimator")
  public:
    SPropertyAnimator(IWindow *pTarget);
    virtual ~SPropertyAnimator();

  public:
    // IPropertyAnimator接口实现
    STDMETHOD_(IWindow *,GetTarget)(CTHIS) SCONST OVERRIDE
    {
        return m_pTarget;
    }
    STDMETHOD_(void, SetPropertyValuesHolder)(THIS_ IPropertyValuesHolder *pHolder) OVERRIDE;
    STDMETHOD_(void, SetPropertyValuesHolders)(THIS_ IPropertyValuesHolder **pHolders, int count) OVERRIDE;
    STDMETHOD_(IPropertyValuesHolder *, GetPropertyValuesHolderByName)(CTHIS_ LPCWSTR propertyName) SCONST OVERRIDE;
    STDMETHOD_(IPropertyValuesHolder *, GetPropertyValuesHolderByIndex)(CTHIS_ int index) SCONST OVERRIDE;
    STDMETHOD_(int, GetPropertyValuesHolderCount)(CTHIS) SCONST OVERRIDE{
        return m_propertyHolders.GetCount();
    }

  protected:
    // SValueAnimator接口实现
    STDMETHOD_(void, onEvaluateValue)(THIS_ float fraction) OVERRIDE;

  protected:
    // IAnimatorListener接口实现
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator *pAnimator) OVERRIDE;
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator *pAnimator) OVERRIDE;
    STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator *pAnimator) OVERRIDE
    {
    }

  public:
    // 静态工厂方法 - 类似Android ObjectAnimator
    static IPropertyAnimator *ofFloat(IWindow *pWnd, LPCWSTR propertyName, const float *values, int valueCount);
    static IPropertyAnimator *ofInt(IWindow *pWnd, LPCWSTR propertyName, const int *values, int valueCount);
    static IPropertyAnimator *ofLayoutSize(IWindow *pWnd, LPCWSTR propertyName, const SLayoutSize *values, int valueCount);
    static IPropertyAnimator *ofPosition(IWindow *pWnd, LPCWSTR propertyName, const void *values, int valueCount, int valueSize);
    static IPropertyAnimator *ofPropertyValuesHolder(IWindow *pWnd, IPropertyValuesHolder **propertyHolders, int holderCount);

  private:
    SAutoRefPtr<IWindow> m_pTarget;
    SArray<SAutoRefPtr<IPropertyValuesHolder>> m_propertyHolders;
};

SNSEND

#endif // __SLAYOUT_OBJECT_ANIMATOR__H__
