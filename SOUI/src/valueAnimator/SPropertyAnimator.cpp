/**
 * @file SLayoutObjectAnimator.cpp
 * @brief SOUI布局对象动画器实现 - 参考Android ObjectAnimator设计
 * @author SOUI group
 * @date 2024
 */

#include <souistd.h>
#include <valueAnimator/SPropertyAnimator.h>
#include <layout/SLinearLayoutParamStruct.h>
#include <layout/SGridLayoutParamStruct.h>
#include <layout/SouiLayoutParamStruct.h>
#include <layout/SAnchorLayoutParamStruct.h>

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// SPropertyValuesHolder实现

SPropertyValuesHolder::SPropertyValuesHolder()
{
    m_value.nType = PROP_TYPE_UNKNOWN;
    m_value.fValue = NULL;
    m_valueCount = 0;
}

SPropertyValuesHolder::~SPropertyValuesHolder()
{
    ClearValues();
}

void SPropertyValuesHolder::SetPropertyName(LPCWSTR propertyName)
{
    m_propertyName = propertyName;
}

LPCWSTR SPropertyValuesHolder::GetPropertyName() const
{
    return m_propertyName;
}

void SPropertyValuesHolder::SetByteValues(const BYTE *values, int count)
{
    ClearValues();
    if (values && count > 0)
    {
        m_value.nType = PROP_TYPE_BYTE;
        m_value.bValue = new BYTE[count];
        for (int i = 0; i < count; i++)
        {
            m_value.bValue[i] = values[i];
        }
        m_valueSize = sizeof(BYTE);
        m_valueCount = count;
    }
}

void SPropertyValuesHolder::SetShortValues(const short *values, int count)
{  
    ClearValues();
    if (values && count > 0)
    {
        m_value.nType = PROP_TYPE_SHORT;
        m_value.sValue = new short[count];
        for (int i = 0; i < count; i++)
        {
            m_value.sValue[i] = values[i];
        }
        m_valueSize = sizeof(short);
        m_valueCount = count;
    }
}

void SPropertyValuesHolder::SetColorRefValues(const COLORREF *values, int count)
{
    ClearValues();
    if (values && count > 0)
    {
        m_value.nType = PROP_TYPE_COLORREF;
        m_value.crValue = new COLORREF[count];
        for (int i = 0; i < count; i++)
        {
            m_value.crValue[i] = values[i];
        }
        m_valueSize = sizeof(COLORREF);
        m_valueCount = count;
    }
}

void SPropertyValuesHolder::SetFloatValues(const float *values, int count)
{
    ClearValues();
    if (values && count > 0)
    {
        m_value.nType = PROP_TYPE_FLOAT;
        m_value.fValue = new float[count];
        memcpy(m_value.fValue, values, sizeof(float) * count);
        m_valueSize = sizeof(float);
        m_valueCount = count;
    }
}

void SPropertyValuesHolder::SetIntValues(const int *values, int count)
{
    ClearValues();
    if (values && count > 0)
    {
        m_value.nType = PROP_TYPE_INT;
        m_value.nValue = new int[count];
        memcpy(m_value.nValue, values, sizeof(int) * count);
        m_valueSize = sizeof(int);
        m_valueCount = count;
    }
}

void SPropertyValuesHolder::SetLayoutSizeValues(const LAYOUTSIZE *values, int count)
{
    ClearValues();
    if (values && count > 0)
    {
        m_value.nType = PROP_TYPE_LAYOUT_SIZE;
        m_value.pLayoutSize = new LAYOUTSIZE[count];
        for (int i = 0; i < count; i++)
        {
            m_value.pLayoutSize[i] = values[i];
        }
        m_valueSize = sizeof(LAYOUTSIZE);
        m_valueCount = count;
    }
}

void SPropertyValuesHolder::SetPositionValues(const void *values, int count, int valueSize)
{
    ClearValues();
    if (values && count > 0)
    {
        m_value.nType = PROP_TYPE_VARIANT;
        m_value.pValue = malloc(valueSize * count);
        memcpy(m_value.pValue, values, valueSize * count);
        m_valueSize = valueSize;
        m_valueCount = count;
    }
}

BOOL SPropertyValuesHolder::GetAnimatedValue(float fraction, void *pValue) const
{
    if (!pValue || m_valueCount == 0)
        return FALSE;

    switch (m_value.nType)
    {
    case PROP_TYPE_FLOAT:
        *(float*)pValue = InterpolateFloat(fraction);
        return TRUE;
    case PROP_TYPE_INT:
        *(int*)pValue = InterpolateInt(fraction);
        return TRUE;
    case PROP_TYPE_LAYOUT_SIZE:
        *(LAYOUTSIZE*)pValue = InterpolateLayoutSize(fraction);
        return TRUE;
    case PROP_TYPE_BYTE:
        *(BYTE *)pValue = InterpolateByte(fraction);
        return TRUE;
    case PROP_TYPE_SHORT:
        *(short *)pValue = InterpolateShort(fraction);
        break;
    case PROP_TYPE_COLORREF:
        *(COLORREF *)pValue = InterpolateColorRef(fraction);
        break;
    }
    return FALSE;
}

void SPropertyValuesHolder::GetStartValue(void *pValue) const
{
    if (!pValue || m_valueCount == 0)
        return;
    memcpy(pValue, m_value.fValue, m_valueSize);
}

void SPropertyValuesHolder::GetEndValue(void *pValue) const
{
    if (!pValue || m_valueCount < 2)
        return;
    memcpy(pValue, (char*)m_value.fValue + (m_valueCount - 1)*m_valueSize, m_valueSize);
}

BOOL SPropertyValuesHolder::GetValueByIndex(int index, void *pValue, int valueSize) const
{
    if (!pValue || index < 0 || index >= m_valueCount)
        return FALSE;
    if(valueSize != m_valueSize)
        return FALSE;
    memcpy(pValue, (char*)m_value.pValue + index * valueSize, valueSize);
    return TRUE;
}

void SPropertyValuesHolder::ClearValues()
{
    if (m_valueCount)
    {
        switch (m_value.nType)
        {
        case PROP_TYPE_BYTE:
            delete[] m_value.bValue;
            m_value.bValue = NULL;
            break;
        case PROP_TYPE_SHORT:
            delete[] m_value.sValue;
            m_value.sValue = NULL;
            break;
        case PROP_TYPE_COLORREF:
            delete[] m_value.crValue;
            m_value.crValue = NULL;
            break;
        case PROP_TYPE_FLOAT:
            delete[] m_value.fValue;
            m_value.fValue = NULL;
            break;
        case PROP_TYPE_INT:
            delete[] m_value.nValue;
            m_value.nValue = NULL;
            break;
        case PROP_TYPE_LAYOUT_SIZE:
            delete[] m_value.pLayoutSize;
            m_value.pLayoutSize = NULL;
            break;
        case PROP_TYPE_VARIANT:
            free(m_value.pValue);
            m_value.pValue = NULL;
            break;
        }
        m_value.nType = PROP_TYPE_UNKNOWN;
        m_valueSize = 0;
        m_valueCount = 0;
    }
}

float SPropertyValuesHolder::Fraction2Index(float fraction, int idx[2]) const
{
    float segmentFraction = fraction * (m_valueCount - 1);
    idx[0] = (int)segmentFraction;
    idx[1] = idx[0] + 1;
    if (idx[1] >= m_valueCount)
        idx[1] = m_valueCount - 1;
    return segmentFraction - idx[0];
}

BYTE SPropertyValuesHolder::InterpolateByte(float fraction) const
{
    int idx[2];
    fraction = Fraction2Index(fraction, idx);
    if (idx[0] == idx[1])
        return m_value.bValue[idx[0]];
    return (BYTE)(m_value.bValue[idx[0]] + (m_value.bValue[idx[1]] - m_value.bValue[idx[0]]) * fraction);
}

short SPropertyValuesHolder::InterpolateShort(float fraction) const
{
    int idx[2];
    fraction = Fraction2Index(fraction, idx);
    if (idx[0] == idx[1])
        return m_value.sValue[idx[0]];
    return (short)(m_value.sValue[idx[0]] + (m_value.sValue[idx[1]] - m_value.sValue[idx[0]]) * fraction);
}

static COLORREF _InterpolateCOLORREF(COLORREF start, COLORREF end, float fraction)
{
    return RGBA(
        GetRValue(start) + BYTE((GetRValue(end) - GetRValue(start)) * fraction),
        GetGValue(start) + BYTE((GetGValue(end) - GetGValue(start)) * fraction),
        GetBValue(start) + BYTE((GetBValue(end) - GetBValue(start)) * fraction),
        GetAValue(start) + BYTE((GetAValue(end) - GetAValue(start)) * fraction)
    );
}

COLORREF SPropertyValuesHolder::InterpolateColorRef(float fraction) const
{
    int idx[2];
    fraction = Fraction2Index(fraction, idx);
    if (idx[0] == idx[1])
        return m_value.crValue[idx[0]];
    return _InterpolateCOLORREF(m_value.crValue[idx[0]], m_value.crValue[idx[1]], fraction);
}

float SPropertyValuesHolder::InterpolateFloat(float fraction) const
{
    int idx[2];
    fraction = Fraction2Index(fraction, idx);
    if (idx[0] == idx[1])
        return m_value.fValue[idx[0]];
    return m_value.fValue[idx[0]] + (m_value.fValue[idx[1]] - m_value.fValue[idx[0]]) * fraction;
}

int SPropertyValuesHolder::InterpolateInt(float fraction) const
{
    int idx[2];
    fraction = Fraction2Index(fraction, idx);
    if (idx[0] == idx[1])
        return m_value.nValue[idx[0]];
    return m_value.nValue[idx[0]] + (m_value.nValue[idx[1]] - m_value.nValue[idx[0]]) * fraction;
}

static SLayoutSize _InterpolateLayoutSize(const LAYOUTSIZE &start, const LAYOUTSIZE &end, float fraction)
{
    // 简单线性插值
    float startValue = start.fSize; // 使用scale=1进行插值计算
    float endValue = end.fSize;
    float interpValue = startValue + (endValue - startValue) * fraction;
    return SLayoutSize(interpValue, start.unit);
}

SLayoutSize SPropertyValuesHolder::InterpolateLayoutSize(float fraction) const
{
    int idx[2];
    fraction = Fraction2Index(fraction, idx);
    if (idx[0] == idx[1])
        return m_value.pLayoutSize[idx[0]];
    return _InterpolateLayoutSize(m_value.pLayoutSize[idx[0]], m_value.pLayoutSize[idx[1]], fraction);
}

// 静态工厂方法
SPropertyValuesHolder* SPropertyValuesHolder::ofByte(LPCWSTR propertyName, const BYTE *values, int count)
{
    SPropertyValuesHolder *pHolder = new SPropertyValuesHolder();
    pHolder->SetPropertyName(propertyName);
    pHolder->SetByteValues(values, count);
    return pHolder;
}

SPropertyValuesHolder* SPropertyValuesHolder::ofShort(LPCWSTR propertyName, const short *values, int count)
{
    SPropertyValuesHolder *pHolder = new SPropertyValuesHolder();
    pHolder->SetPropertyName(propertyName);
    pHolder->SetShortValues(values, count);
    return pHolder;
}

SPropertyValuesHolder* SPropertyValuesHolder::ofColorRef(LPCWSTR propertyName, const COLORREF *values, int count)
{
    SPropertyValuesHolder *pHolder = new SPropertyValuesHolder();
    pHolder->SetPropertyName(propertyName);
    pHolder->SetColorRefValues(values, count);
    return pHolder;
}

SPropertyValuesHolder* SPropertyValuesHolder::ofFloat(LPCWSTR propertyName, const float *values, int count)
{
    SPropertyValuesHolder *pHolder = new SPropertyValuesHolder();
    pHolder->SetPropertyName(propertyName);
    pHolder->SetFloatValues(values, count);
    return pHolder;
}

SPropertyValuesHolder* SPropertyValuesHolder::ofInt(LPCWSTR propertyName, const int *values, int count)
{
    SPropertyValuesHolder *pHolder = new SPropertyValuesHolder();
    pHolder->SetPropertyName(propertyName);
    pHolder->SetIntValues(values, count);
    return pHolder;
}

SPropertyValuesHolder* SPropertyValuesHolder::ofLayoutSize(LPCWSTR propertyName, const LAYOUTSIZE *values, int count)
{
    SPropertyValuesHolder *pHolder = new SPropertyValuesHolder();
    pHolder->SetPropertyName(propertyName);
    pHolder->SetLayoutSizeValues(values, count);
    return pHolder;
}

SPropertyValuesHolder *SPropertyValuesHolder::ofPosition(LPCWSTR propertyName, const void *values, int count, int valueSize)
{
    SPropertyValuesHolder *pHolder = new SPropertyValuesHolder();
    pHolder->SetPropertyName(propertyName);
    pHolder->SetPositionValues(values, count, valueSize);
    return pHolder;
}
//////////////////////////////////////////////////////////////////////////
// SPropertyAnimator实现

SPropertyAnimator::SPropertyAnimator(IWindow *pTarget)
{
    m_pTarget = pTarget;
    addListener(this);
}

SPropertyAnimator::~SPropertyAnimator()
{
}

void SPropertyAnimator::onAnimationStart(IValueAnimator * pAnimator)
{
     for(int i=0;i<m_propertyHolders.GetCount();i++)
    {
        m_pTarget->SetAnimatorValue(m_propertyHolders[i], 0.f, ANI_START);         
    }
}

void SPropertyAnimator::onAnimationEnd(IValueAnimator * pAnimator)
{
    for(int i=0;i<m_propertyHolders.GetCount();i++)
    {
        m_pTarget->SetAnimatorValue(m_propertyHolders[i], 1.f, ANI_END);
    }
}

void SPropertyAnimator::onEvaluateValue(float fraction)
{
    if (!m_pTarget)
        return;

    for (int i = 0; i < m_propertyHolders.GetCount(); i++)
    {
        m_pTarget->SetAnimatorValue(m_propertyHolders[i], fraction, ANI_PROGRESS);
    }
}

void SPropertyAnimator::SetPropertyValuesHolder(IPropertyValuesHolder *pHolder)
{
    return SetPropertyValuesHolders(&pHolder, 1);
}

void SPropertyAnimator::SetPropertyValuesHolders(IPropertyValuesHolder **pHolders, int count)
{
    m_propertyHolders.RemoveAll();
    for (int i = 0; i < count; i++)
    {
        if (pHolders[i])
        {
            m_propertyHolders.Add(pHolders[i]);
        }
    }
}

// 静态工厂方法
SPropertyAnimator *SPropertyAnimator::ofFloat(IWindow *pWnd, LPCWSTR propertyName, const float *values, int valueCount)
{
    SPropertyAnimator *pAnimator = new SPropertyAnimator(pWnd);
    SPropertyValuesHolder *pHolder = SPropertyValuesHolder::ofFloat(propertyName, values, valueCount);
    pAnimator->SetPropertyValuesHolder(pHolder);
    pHolder->Release();

    return pAnimator;
}

SPropertyAnimator *SPropertyAnimator::ofInt(IWindow *pWnd, LPCWSTR propertyName, const int *values, int valueCount)
{
    SPropertyAnimator *pAnimator = new SPropertyAnimator(pWnd);
    SPropertyValuesHolder *pHolder = SPropertyValuesHolder::ofInt(propertyName, values, valueCount);
    pAnimator->SetPropertyValuesHolder(pHolder);
    pHolder->Release();

    return pAnimator;
}


SPropertyAnimator *SPropertyAnimator::ofLayoutSize(IWindow *pWnd, LPCWSTR propertyName, const SLayoutSize *values, int valueCount)
{
    SPropertyAnimator *pAnimator = new SPropertyAnimator(pWnd);
    SPropertyValuesHolder *pHolder = SPropertyValuesHolder::ofLayoutSize(propertyName, values, valueCount);
    pAnimator->SetPropertyValuesHolder(pHolder);
    pHolder->Release();

    return pAnimator;
}

SPropertyAnimator *SPropertyAnimator::ofPosition(IWindow *pWnd, LPCWSTR propertyName, const void *values, int valueCount, int valueSize)
{
    SPropertyAnimator *pAnimator = new SPropertyAnimator(pWnd);

    SPropertyValuesHolder *pHolder = SPropertyValuesHolder::ofPosition(propertyName, values, valueCount,valueSize);
    pAnimator->SetPropertyValuesHolder(pHolder);
    pHolder->Release();
    return pAnimator;
}

SPropertyAnimator *SPropertyAnimator::ofPropertyValuesHolder(IWindow *pWnd, IPropertyValuesHolder **propertyHolders, int holderCount)
{
    SPropertyAnimator *pAnimator = new SPropertyAnimator(pWnd);
    pAnimator->SetPropertyValuesHolders(propertyHolders, holderCount);

    return pAnimator;
}

SNSEND
