/**
 * @file SPropertyAnimator.h
 * @brief SOUI布局对象动画器实现 - 参考Android ObjectAnimator设计
 * @author SOUI group
 * @date 2024
 */

#ifndef __SLAYOUT_OBJECT_ANIMATOR__H__
#define __SLAYOUT_OBJECT_ANIMATOR__H__

#include <interface/slayout-i.h>
#include <valueAnimator/SValueAnimator.h>
#include <layout/SLayoutSize.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

typedef struct _PROPVALUE{
    PROP_TYPE nType;
    union{
        BYTE * bValue;
        COLORREF * crValue;
        int * nValue;
        short * sValue;
        float * fValue;
        LAYOUTSIZE * pLayoutSize;
        void* pValue; 
    };
}PROPVALUE;

/**
 * @class SPropertyValuesHolder
 * @brief 属性值持有者实现类 - 类似Android PropertyValuesHolder
 */
class SOUI_EXP SPropertyValuesHolder : public TObjRefImpl<IPropertyValuesHolder>
{
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
    STDMETHOD_(PROP_TYPE, GetValueType)(CTHIS) SCONST OVERRIDE{
        return m_value.nType;
    }
    STDMETHOD_(int, GetValueSize)(CTHIS) SCONST OVERRIDE{
        return m_valueSize;
    }
    STDMETHOD_(void,GetStartValue)(CTHIS_ void *pValue) SCONST OVERRIDE;
    STDMETHOD_(void,GetEndValue)(CTHIS_ void *pValue) SCONST OVERRIDE;
    STDMETHOD_(float, Fraction2Index)(CTHIS_ float fraction, int idx[2]) SCONST OVERRIDE;
    STDMETHOD_(BOOL,GetValueByIndex)(CTHIS_ int index, void *pValue, int valueSize) SCONST OVERRIDE;
    STDMETHOD_(int, GetKeyframeCount)(CTHIS) SCONST OVERRIDE{
        return m_valueCount;
    }
    // 静态工厂方法
    static SPropertyValuesHolder* ofByte(LPCWSTR propertyName, const BYTE *values, int count);
    static SPropertyValuesHolder* ofShort(LPCWSTR propertyName, const short *values, int count);
    static SPropertyValuesHolder* ofColorRef(LPCWSTR propertyName, const COLORREF *values, int count);
    static SPropertyValuesHolder* ofFloat(LPCWSTR propertyName, const float *values, int count);
    static SPropertyValuesHolder* ofInt(LPCWSTR propertyName, const int *values, int count);
    static SPropertyValuesHolder* ofLayoutSize(LPCWSTR propertyName, const LAYOUTSIZE *values, int count);
    static SPropertyValuesHolder* ofPosition(LPCWSTR propertyName, const void *values, int count, int valueSize);

private:


    SStringW m_propertyName;
    PROPVALUE m_value;

    int m_valueCount;
    int m_valueSize;  // 用于位置值的大小

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
class SOUI_EXP SPropertyAnimator : public SValueAnimator, protected IAnimatorListener
{
    DEF_SOBJECT(SValueAnimator, L"propertyAnimator")
public:
    SPropertyAnimator(IWindow *pTarget);
    virtual ~SPropertyAnimator();
    
    
    IWindow* GetTarget() const{
        return m_pTarget;
    }
    // 设置属性值持有者
    void SetPropertyValuesHolder(IPropertyValuesHolder *pHolder);
    void SetPropertyValuesHolders(IPropertyValuesHolder **pHolders, int count);
    
protected:
    // SValueAnimator接口实现
    STDMETHOD_(void, onEvaluateValue)(THIS_ float fraction) OVERRIDE;
protected:
    // IAnimatorListener接口实现
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator * pAnimator) OVERRIDE;
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator * pAnimator) OVERRIDE;
    STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator * pAnimator) OVERRIDE{}
public:
    // 静态工厂方法 - 类似Android ObjectAnimator
    static SPropertyAnimator *ofFloat(IWindow *pWnd, LPCWSTR propertyName, const float *values, int valueCount);
    static SPropertyAnimator *ofInt(IWindow *pWnd, LPCWSTR propertyName, const int *values, int valueCount);
    static SPropertyAnimator *ofLayoutSize(IWindow *pWnd, LPCWSTR propertyName, const SLayoutSize *values, int valueCount);
    static SPropertyAnimator *ofPosition(IWindow *pWnd, LPCWSTR propertyName, const void *values, int valueCount, int valueSize);
    static SPropertyAnimator *ofPropertyValuesHolder(IWindow *pWnd, IPropertyValuesHolder **propertyHolders, int holderCount);
private:
    SAutoRefPtr<IWindow> m_pTarget;
    SArray<SAutoRefPtr<IPropertyValuesHolder> > m_propertyHolders;
};

SNSEND

#endif // __SLAYOUT_OBJECT_ANIMATOR__H__
