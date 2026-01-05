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
    m_value.pWeights = NULL;
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
        *(float *)pValue = InterpolateFloat(fraction);
        return TRUE;
    case PROP_TYPE_INT:
        *(int *)pValue = InterpolateInt(fraction);
        return TRUE;
    case PROP_TYPE_LAYOUT_SIZE:
        *(LAYOUTSIZE *)pValue = InterpolateLayoutSize(fraction);
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
    memcpy(pValue, (char *)m_value.fValue + (m_valueCount - 1) * m_valueSize, m_valueSize);
}

BOOL SPropertyValuesHolder::GetValueByIndex(int index, void *pValue, int valueSize) const
{
    if (!pValue || index < 0 || index >= m_valueCount)
        return FALSE;
    if (valueSize != m_valueSize)
        return FALSE;
    memcpy(pValue, (char *)m_value.pValue + index * valueSize, valueSize);
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
        if (m_value.pWeights)
        {
            delete[] m_value.pWeights;
            m_value.pWeights = NULL;
        }
        m_value.nType = PROP_TYPE_UNKNOWN;
        m_valueSize = 0;
        m_valueCount = 0;
        m_totalWeight = 0;
    }
}

float SPropertyValuesHolder::Fraction2Index(float fraction, int idx[2]) const
{
    if (!m_value.pWeights)
    {
        // 没有权重，使用均匀分布逻辑
        float segmentFraction = fraction * (m_valueCount - 1);
        idx[0] = (int)segmentFraction;
        idx[1] = idx[0] + 1;
        if (idx[1] >= m_valueCount)
            idx[1] = m_valueCount - 1;
        return segmentFraction - idx[0];
    }
    else
    {
        // 处理特殊情况：fraction为1.0时
        if (fraction >= 1.0f)
        {
            idx[0] = m_valueCount - 2;
            idx[1] = m_valueCount - 1;
            return 1.0f;
        }

        // 使用权重计算关键帧索引和片段分数
        float weightedFraction = fraction * m_totalWeight;
        float accumulatedWeight = 0.0f;
        for (int i = 0; i < m_valueCount - 1; i++)
        {
            accumulatedWeight += m_value.pWeights[i];
            if (weightedFraction <= accumulatedWeight)
            {
                idx[0] = i;
                idx[1] = i + 1;

                // 计算在当前段内的fraction
                float segmentStart = accumulatedWeight - m_value.pWeights[i];
                float segmentLength = m_value.pWeights[i];
                return (weightedFraction - segmentStart) / segmentLength;
            }
        }

        // 如果fraction超出了所有段，返回最后一段
        idx[0] = m_valueCount - 2;
        idx[1] = m_valueCount - 1;
        return 1.0f;
    }
}

int SPropertyValuesHolder::Fraction2FrameIndex(float fraction) const
{
    int idx[2];
    Fraction2Index(fraction, idx);
    if(fraction >= 1.0f)
        return idx[1];
    else
        return idx[0];
}

BOOL SPropertyValuesHolder::SetKeyFrameWeights(const float *weights, int count)
{
    // 权重数量应该是关键帧数减1，因为权重表示的是帧之间的段
    if (weights && count == m_valueCount - 1)
    {
        float totalWeight = 0.0f;
        for (int i = 0; i < count; i++)
        {
            if (weights[i] <= 0.0f)
                return FALSE;
            totalWeight += weights[i];
        }
        if (!m_value.pWeights)
        {
            m_value.pWeights = new float[count];
        }
        memcpy(m_value.pWeights, weights, sizeof(float) * count);
        m_totalWeight = totalWeight;
        return TRUE;
    }
    return FALSE;
}

BOOL SPropertyValuesHolder::GetKeyFrameWeights(float *weights, int count) const
{
    // 权重数量应该是关键帧数减1
    if (!weights || count != m_valueCount - 1)
        return FALSE;
    if (m_value.pWeights)
    {
        memcpy(weights, m_value.pWeights, sizeof(float) * count);
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            weights[i] = 1.0f;
        }
    }
    return TRUE;
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
    return RGBA(GetRValue(start) + BYTE((GetRValue(end) - GetRValue(start)) * fraction), GetGValue(start) + BYTE((GetGValue(end) - GetGValue(start)) * fraction), GetBValue(start) + BYTE((GetBValue(end) - GetBValue(start)) * fraction), GetAValue(start) + BYTE((GetAValue(end) - GetAValue(start)) * fraction));
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
SPropertyValuesHolder *SPropertyValuesHolder::ofByte(LPCWSTR propertyName, const BYTE *values, int count)
{
    SPropertyValuesHolder *pHolder = new SPropertyValuesHolder();
    pHolder->SetPropertyName(propertyName);
    pHolder->SetByteValues(values, count);
    return pHolder;
}

SPropertyValuesHolder *SPropertyValuesHolder::ofShort(LPCWSTR propertyName, const short *values, int count)
{
    SPropertyValuesHolder *pHolder = new SPropertyValuesHolder();
    pHolder->SetPropertyName(propertyName);
    pHolder->SetShortValues(values, count);
    return pHolder;
}

SPropertyValuesHolder *SPropertyValuesHolder::ofColorRef(LPCWSTR propertyName, const COLORREF *values, int count)
{
    SPropertyValuesHolder *pHolder = new SPropertyValuesHolder();
    pHolder->SetPropertyName(propertyName);
    pHolder->SetColorRefValues(values, count);
    return pHolder;
}

SPropertyValuesHolder *SPropertyValuesHolder::ofFloat(LPCWSTR propertyName, const float *values, int count)
{
    SPropertyValuesHolder *pHolder = new SPropertyValuesHolder();
    pHolder->SetPropertyName(propertyName);
    pHolder->SetFloatValues(values, count);
    return pHolder;
}

SPropertyValuesHolder *SPropertyValuesHolder::ofInt(LPCWSTR propertyName, const int *values, int count)
{
    SPropertyValuesHolder *pHolder = new SPropertyValuesHolder();
    pHolder->SetPropertyName(propertyName);
    pHolder->SetIntValues(values, count);
    return pHolder;
}

SPropertyValuesHolder *SPropertyValuesHolder::ofLayoutSize(LPCWSTR propertyName, const LAYOUTSIZE *values, int count)
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

SPropertyAnimator::SPropertyAnimator(IWindow *pTarget /*= NULL*/)
{
    _this_for_callback = (IPropertyAnimator *)this;
    m_pTarget = pTarget;
    addListener(this);
}

SPropertyAnimator::~SPropertyAnimator()
{
}

void SPropertyAnimator::onAnimationStart(IValueAnimator *pAnimator)
{
    if(!m_pTarget) 
        return;
    for (int i = 0; i < m_propertyHolders.GetCount(); i++)
    {
        m_pTarget->SetAnimatorValue(m_propertyHolders[i], 0.f, ANI_START);
    }
}

void SPropertyAnimator::onAnimationEnd(IValueAnimator *pAnimator)
{
    if(!m_pTarget) 
        return;
    for (int i = 0; i < m_propertyHolders.GetCount(); i++)
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

IPropertyValuesHolder *SPropertyAnimator::GetPropertyValuesHolderByName(LPCWSTR propertyName) const
{
    for (int i = 0; i < m_propertyHolders.GetCount(); i++)
    {
        if (wcscmp(m_propertyHolders[i]->GetPropertyName(), propertyName) == 0)
        {
            return m_propertyHolders[i];
        }
    }
    return NULL;
}

IPropertyValuesHolder *SPropertyAnimator::GetPropertyValuesHolderByIndex(int index) const
{
    if (index >= 0 && index < m_propertyHolders.GetCount())
    {
        return m_propertyHolders[index];
    }
    return NULL;
}

void SPropertyAnimator::copy(const IValueAnimator *pAnimator)
{
    const IPropertyAnimator *pPropAnimator = sobj_cast<const IPropertyAnimator>(pAnimator);
    if(pPropAnimator){
        SValueAnimator::copy(pAnimator);
        SetTarget(pPropAnimator->GetTarget());
        int count = pPropAnimator->GetPropertyValuesHolderCount();
        IPropertyValuesHolder **pHolders = new IPropertyValuesHolder *[count];
        for (int i = 0; i < count; i++)
        {
            pHolders[i] = pPropAnimator->GetPropertyValuesHolderByIndex(i);
        }
        SetPropertyValuesHolders(pHolders, count);
        delete[] pHolders;
    }
}

IValueAnimator * SPropertyAnimator::clone() const
{
    SPropertyAnimator *pAnimator = new SPropertyAnimator();
    pAnimator->copy((const IPropertyAnimator*)this);
    return (IValueAnimator *)(IPropertyAnimator*)pAnimator;
}

// 静态工厂方法
IPropertyAnimator *SPropertyAnimator::ofFloat(IWindow *pWnd, LPCWSTR propertyName, const float *values, int valueCount)
{
    SPropertyAnimator *pAnimator = new SPropertyAnimator(pWnd);
    SPropertyValuesHolder *pHolder = SPropertyValuesHolder::ofFloat(propertyName, values, valueCount);
    pAnimator->SetPropertyValuesHolder(pHolder);
    pHolder->Release();

    return pAnimator;
}

IPropertyAnimator *SPropertyAnimator::ofInt(IWindow *pWnd, LPCWSTR propertyName, const int *values, int valueCount)
{
    SPropertyAnimator *pAnimator = new SPropertyAnimator(pWnd);
    SPropertyValuesHolder *pHolder = SPropertyValuesHolder::ofInt(propertyName, values, valueCount);
    pAnimator->SetPropertyValuesHolder(pHolder);
    pHolder->Release();

    return pAnimator;
}

IPropertyAnimator *SPropertyAnimator::ofLayoutSize(IWindow *pWnd, LPCWSTR propertyName, const SLayoutSize *values, int valueCount)
{
    SPropertyAnimator *pAnimator = new SPropertyAnimator(pWnd);
    SPropertyValuesHolder *pHolder = SPropertyValuesHolder::ofLayoutSize(propertyName, values, valueCount);
    pAnimator->SetPropertyValuesHolder(pHolder);
    pHolder->Release();

    return pAnimator;
}

IPropertyAnimator *SPropertyAnimator::ofPosition(IWindow *pWnd, LPCWSTR propertyName, const void *values, int valueCount, int valueSize)
{
    SPropertyAnimator *pAnimator = new SPropertyAnimator(pWnd);

    SPropertyValuesHolder *pHolder = SPropertyValuesHolder::ofPosition(propertyName, values, valueCount, valueSize);
    pAnimator->SetPropertyValuesHolder(pHolder);
    pHolder->Release();
    return pAnimator;
}

IPropertyAnimator *SPropertyAnimator::ofPropertyValuesHolder(IWindow *pWnd, IPropertyValuesHolder **propertyHolders, int holderCount)
{
    SPropertyAnimator *pAnimator = new SPropertyAnimator(pWnd);
    pAnimator->SetPropertyValuesHolders(propertyHolders, holderCount);

    return pAnimator;
}

SNSEND
