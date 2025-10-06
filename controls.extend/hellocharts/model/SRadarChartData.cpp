#include "stdafx.h"
#include "SRadarChartData.h"

SNSBEGIN

// Static constants
const int SRadarChartData::DEFAULT_WEB_RINGS = 5;
const float SRadarChartData::DEFAULT_MAX_VALUE = 100.0f;

//////////////////////////////////////////////////////////////////////////
// SRadarChartData implementation

SRadarChartData::SRadarChartData()
    : m_bHasRadarLabels(TRUE)
    , m_bHasWebLines(TRUE)
    , m_webRings(DEFAULT_WEB_RINGS)
    , m_maxValue(DEFAULT_MAX_VALUE)
{
}

SRadarChartData::SRadarChartData(const SRadarChartData& other)
    : SChartData(other)
    , m_axesLabels(other.m_axesLabels)
    , m_bHasRadarLabels(other.m_bHasRadarLabels)
    , m_bHasWebLines(other.m_bHasWebLines)
    , m_webRings(other.m_webRings)
    , m_maxValue(other.m_maxValue)
{
    // Deep copy radar values
    for (size_t i = 0; i < other.m_values.size(); ++i)
    {
        if (other.m_values[i])
        {
            m_values.push_back(new SRadarValue(*other.m_values[i]));
        }
    }
}

SRadarChartData::~SRadarChartData()
{
    ClearValues();
}

void SRadarChartData::SetValues(const std::vector<SRadarValue*>& values)
{
    ClearValues();
    m_values = values;
}

void SRadarChartData::AddValue(SRadarValue* pValue)
{
    if (pValue)
    {
        m_values.push_back(pValue);
    }
}

SRadarValue* SRadarChartData::GetValue(size_t index) const
{
    if (index < m_values.size())
    {
        return m_values[index];
    }
    return NULL;
}

void SRadarChartData::ClearValues()
{
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        delete m_values[i];
    }
    m_values.clear();
}

void SRadarChartData::SetAxesLabels(const std::vector<SStringT>& labels)
{
    m_axesLabels = labels;
}

void SRadarChartData::AddAxisLabel(const SStringT& label)
{
    m_axesLabels.push_back(label);
}

void SRadarChartData::InitializeAnimation()
{
    // Initialize animation for all radar values
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        SRadarValue* pRadarValue = m_values[i];
        if (pRadarValue)
        {
            pRadarValue->InitializeAnimation();
        }
    }
}

void SRadarChartData::Update(float scale)
{
    // Animation update logic for radar values
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        SRadarValue* pRadarValue = m_values[i];
        if (pRadarValue)
        {
            pRadarValue->Update(scale);
        }
    }
}

void SRadarChartData::Finish()
{
    // Animation finish logic for radar values
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        SRadarValue* pRadarValue = m_values[i];
        if (pRadarValue)
        {
            pRadarValue->Finish();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// SRadarValue implementation

SRadarValue::SRadarValue()
    : m_color(RGBA(128, 128, 128, 255))
    , m_originColor(RGBA(128, 128, 128, 255))
    , m_diffColor(RGBA(0, 0, 0, 0))
{
}

SRadarValue::SRadarValue(const SStringT& label)
    : m_label(label)
    , m_color(RGBA(128, 128, 128, 255))
    , m_originColor(RGBA(128, 128, 128, 255))
    , m_diffColor(RGBA(0, 0, 0, 0))
{
}

SRadarValue::SRadarValue(const SStringT& label, COLORREF color)
    : m_label(label)
    , m_color(color)
    , m_originColor(color)
    , m_diffColor(RGBA(0, 0, 0, 0))
{
}

SRadarValue::SRadarValue(const SRadarValue& other)
    : m_axisValues(other.m_axisValues)
    , m_label(other.m_label)
    , m_color(other.m_color)
    , m_originColor(other.m_originColor)
    , m_diffColor(other.m_diffColor)
{
}

SRadarValue::~SRadarValue()
{
}

void SRadarValue::SetAxisValues(const std::vector<float>& values)
{
    m_axisValues = values;
}

void SRadarValue::AddAxisValue(float value)
{
    m_axisValues.push_back(value);
}

float SRadarValue::GetAxisValue(size_t index) const
{
    if (index < m_axisValues.size())
    {
        return m_axisValues[index];
    }
    return 0.0f;
}

void SRadarValue::SetAxisValue(size_t index, float value)
{
    if (index < m_axisValues.size())
    {
        m_axisValues[index] = value;
    }
    else
    {
        // Extend the vector if necessary
        while (m_axisValues.size() <= index)
        {
            m_axisValues.push_back(0.0f);
        }
        m_axisValues[index] = value;
    }
}

void SRadarValue::InitializeAnimation()
{
    // Initialize color animation - start from transparent and animate to target color
	m_originColor = RGBA(GetRValue(m_color), GetGValue(m_color), GetBValue(m_color), 0);  // Transparent
	m_diffColor = RGBA(0, 0, 0, GetAValue(m_color));  // Only animate alpha
	m_color = m_originColor;  // Start with transparent color
}

void SRadarValue::Update(float scale)
{
    if (scale < 0.0f) scale = 0.0f;
    if (scale > 1.0f) scale = 1.0f;

    BYTE r = GetRValue(m_originColor) + (BYTE)(GetRValue(m_diffColor) * scale);
    BYTE g = GetGValue(m_originColor) + (BYTE)(GetGValue(m_diffColor) * scale);
    BYTE b = GetBValue(m_originColor) + (BYTE)(GetBValue(m_diffColor) * scale);
    BYTE a = GetAValue(m_originColor) + (BYTE)(GetAValue(m_diffColor) * scale);

    m_color = RGBA(r, g, b, a);
}

void SRadarValue::Finish()
{
	m_color = RGBA(GetRValue(m_originColor), GetGValue(m_originColor), GetBValue(m_originColor), GetAValue(m_diffColor)); 
    m_diffColor = RGBA(0, 0, 0, 0);
}

SNSEND
