#include "stdafx.h"
#include "SPieChartData.h"

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// SPieChartData implementation

SPieChartData::SPieChartData()
    : m_bHasLabels(FALSE)
    , m_bHasLabelLines(FALSE)
    , m_centerCircleRatio(0.0f)
    , m_centerCircleColor(RGBA(255, 255, 255, 255))
    , m_sliceSpacing(0.0f)
{
}

SPieChartData::SPieChartData(const SPieChartData& other)
    : SChartData(other)
    , m_bHasLabels(other.m_bHasLabels)
    , m_bHasLabelLines(other.m_bHasLabelLines)
    , m_centerCircleRatio(other.m_centerCircleRatio)
    , m_centerCircleColor(other.m_centerCircleColor)
    , m_sliceSpacing(other.m_sliceSpacing)
{
    // Deep copy slice values
    for (size_t i = 0; i < other.m_values.size(); ++i)
    {
        if (other.m_values[i])
        {
            m_values.push_back(new SSliceValue(*other.m_values[i]));
        }
    }
}

SPieChartData::~SPieChartData()
{
    ClearValues();
}

void SPieChartData::SetValues(const std::vector<SSliceValue*>& values)
{
    ClearValues();
    m_values = values;
}

void SPieChartData::AddValue(SSliceValue* pValue)
{
    if (pValue)
    {
        m_values.push_back(pValue);
    }
}

SSliceValue* SPieChartData::GetValue(size_t index) const
{
    if (index < m_values.size())
    {
        return m_values[index];
    }
    return NULL;
}

void SPieChartData::ClearValues()
{
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        delete m_values[i];
    }
    m_values.clear();
}

void SPieChartData::InitializeAnimation()
{
    // Initialize animation for all slices
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        SSliceValue* pSlice = m_values[i];
        if (pSlice)
        {
            pSlice->InitializeAnimation();
        }
    }
}

void SPieChartData::Update(float scale)
{
    // Animation update logic for pie slices
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        SSliceValue* pSlice = m_values[i];
        if (pSlice)
        {
            pSlice->Update(scale);
        }
    }
}

void SPieChartData::Finish()
{
    // Animation finish logic for pie slices
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        SSliceValue* pSlice = m_values[i];
        if (pSlice)
        {
            pSlice->Finish();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// SSliceValue implementation

SSliceValue::SSliceValue()
    : m_value(0.0f)
    , m_color(RGBA(128, 128, 128, 255))
    , m_originColor(RGBA(128, 128, 128, 255))
    , m_diffColor(RGBA(0, 0, 0, 0))
{
}

SSliceValue::SSliceValue(float value)
    : m_value(value)
    , m_color(RGBA(128, 128, 128, 255))
    , m_originColor(RGBA(128, 128, 128, 255))
    , m_diffColor(RGBA(0, 0, 0, 0))
{
}

SSliceValue::SSliceValue(float value, COLORREF color)
    : m_value(value)
    , m_color(color)
    , m_originColor(color)
    , m_diffColor(RGBA(0, 0, 0, 0))
{
}

SSliceValue::SSliceValue(float value, COLORREF color, const SStringT& label)
    : m_value(value)
    , m_color(color)
    , m_label(label)
    , m_originColor(color)
    , m_diffColor(RGBA(0, 0, 0, 0))
{
}

SSliceValue::SSliceValue(const SSliceValue& other)
    : m_value(other.m_value)
    , m_color(other.m_color)
    , m_label(other.m_label)
    , m_originColor(other.m_originColor)
    , m_diffColor(other.m_diffColor)
{
}

SSliceValue::~SSliceValue()
{
}

void SSliceValue::InitializeAnimation()
{
    // Initialize color animation - start from transparent and animate to target color
    m_originColor = RGBA(GetRValue(m_color), GetGValue(m_color), GetBValue(m_color), 0);  // Transparent
    m_diffColor = RGBA(0, 0, 0, GetAValue(m_color));  // Only animate alpha
    m_color = m_originColor;  // Start with transparent color
}

void SSliceValue::Update(float scale)
{
    // For pie chart animation, we can animate the slice value (size)
    // The slice will grow from 0 to its target value
    // This creates a "growing pie" effect

    // Note: The actual value is not changed, but we can use a temporary
    // animated value for rendering. For now, we'll animate the color
    // to create a fade-in effect

    // Interpolate color components
    int originR = GetRValue(m_originColor);
    int originG = GetGValue(m_originColor);
    int originB = GetBValue(m_originColor);
    int originA = GetAValue(m_originColor);

    int diffR = GetRValue(m_diffColor);
    int diffG = GetGValue(m_diffColor);
    int diffB = GetBValue(m_diffColor);
    int diffA = GetAValue(m_diffColor);

    int newR = originR + (int)(diffR * scale);
    int newG = originG + (int)(diffG * scale);
    int newB = originB + (int)(diffB * scale);
    int newA = originA + (int)(diffA * scale);

    m_color = RGBA(newR, newG, newB, newA);
}

void SSliceValue::Finish()
{
    // Set final color and reset animation properties
    SetOriginColor(m_color);
    SetDiffColor(RGBA(0, 0, 0, 0));
}

SNSEND
