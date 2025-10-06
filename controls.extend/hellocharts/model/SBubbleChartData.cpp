#include "stdafx.h"
#include "SBubbleChartData.h"

SNSBEGIN

// Static constants
const int SBubbleChartData::DEFAULT_MIN_BUBBLE_RADIUS = 6;
const int SBubbleChartData::DEFAULT_MAX_BUBBLE_RADIUS = 50;
const int SBubbleChartData::DEFAULT_BUBBLE_SCALE = 0; // Linear scale

//////////////////////////////////////////////////////////////////////////
// SBubbleChartData implementation

SBubbleChartData::SBubbleChartData()
    : m_bHasBubbleLabels(FALSE)
    , m_minBubbleRadius(DEFAULT_MIN_BUBBLE_RADIUS)
    , m_maxBubbleRadius(DEFAULT_MAX_BUBBLE_RADIUS)
    , m_bubbleScale(DEFAULT_BUBBLE_SCALE)
{
}

SBubbleChartData::SBubbleChartData(const SBubbleChartData& other)
    : SChartData(other)
    , m_bHasBubbleLabels(other.m_bHasBubbleLabels)
    , m_minBubbleRadius(other.m_minBubbleRadius)
    , m_maxBubbleRadius(other.m_maxBubbleRadius)
    , m_bubbleScale(other.m_bubbleScale)
{
    // Deep copy bubble values
    for (size_t i = 0; i < other.m_values.size(); ++i)
    {
        if (other.m_values[i])
        {
            m_values.push_back(new SBubbleValue(*other.m_values[i]));
        }
    }
}

SBubbleChartData::~SBubbleChartData()
{
    ClearValues();
}

void SBubbleChartData::SetValues(const std::vector<SBubbleValue*>& values)
{
    ClearValues();
    m_values = values;
}

void SBubbleChartData::AddValue(SBubbleValue* pValue)
{
    if (pValue)
    {
        m_values.push_back(pValue);
    }
}

SBubbleValue* SBubbleChartData::GetValue(size_t index) const
{
    if (index < m_values.size())
    {
        return m_values[index];
    }
    return NULL;
}

void SBubbleChartData::ClearValues()
{
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        delete m_values[i];
    }
    m_values.clear();
}

void SBubbleChartData::InitializeAnimation()
{
    // Initialize animation for all bubbles
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        SBubbleValue* pBubble = m_values[i];
        if (pBubble)
        {
            pBubble->InitializeAnimation();
        }
    }
}

void SBubbleChartData::Update(float scale)
{
    // Animation update logic for bubbles
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        SBubbleValue* pBubble = m_values[i];
        if (pBubble)
        {
            pBubble->Update(scale);
        }
    }
}

void SBubbleChartData::Finish()
{
    // Animation finish logic for bubbles
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        SBubbleValue* pBubble = m_values[i];
        if (pBubble)
        {
            pBubble->Finish();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// SBubbleValue implementation

SBubbleValue::SBubbleValue()
    : m_x(0.0f)
    , m_y(0.0f)
    , m_z(0.0f)
    , m_color(RGBA(128, 128, 128, 255))
    , m_originColor(RGBA(128, 128, 128, 255))
    , m_diffColor(RGBA(0, 0, 0, 0))
{
}

SBubbleValue::SBubbleValue(float x, float y, float z)
    : m_x(x)
    , m_y(y)
    , m_z(z)
    , m_color(RGBA(128, 128, 128, 255))
    , m_originColor(RGBA(128, 128, 128, 255))
    , m_diffColor(RGBA(0, 0, 0, 0))
{
}

SBubbleValue::SBubbleValue(float x, float y, float z, COLORREF color)
    : m_x(x)
    , m_y(y)
    , m_z(z)
    , m_color(color)
    , m_originColor(color)
    , m_diffColor(RGBA(0, 0, 0, 0))
{
}

SBubbleValue::SBubbleValue(float x, float y, float z, COLORREF color, const SStringT& label)
    : m_x(x)
    , m_y(y)
    , m_z(z)
    , m_color(color)
    , m_label(label)
    , m_originColor(color)
    , m_diffColor(RGBA(0, 0, 0, 0))
{
}

SBubbleValue::SBubbleValue(const SBubbleValue& other)
    : m_x(other.m_x)
    , m_y(other.m_y)
    , m_z(other.m_z)
    , m_color(other.m_color)
    , m_label(other.m_label)
    , m_originColor(other.m_originColor)
    , m_diffColor(other.m_diffColor)
{
}

SBubbleValue::~SBubbleValue()
{
}

void SBubbleValue::InitializeAnimation()
{
    // Initialize color animation - start from transparent and animate to target color
    m_originColor = RGBA(GetRValue(m_color), GetGValue(m_color), GetBValue(m_color), 0);  // Transparent
    m_diffColor = RGBA(0, 0, 0, GetAValue(m_color));  // Only animate alpha
    m_color = m_originColor;  // Start with transparent color
}

void SBubbleValue::Update(float scale)
{
    if (scale < 0.0f) scale = 0.0f;
    if (scale > 1.0f) scale = 1.0f;

    BYTE r = GetRValue(m_originColor) + (BYTE)(GetRValue(m_diffColor) * scale);
    BYTE g = GetGValue(m_originColor) + (BYTE)(GetGValue(m_diffColor) * scale);
    BYTE b = GetBValue(m_originColor) + (BYTE)(GetBValue(m_diffColor) * scale);
    BYTE a = GetAValue(m_originColor) + (BYTE)(GetAValue(m_diffColor) * scale);

    m_color = RGBA(r, g, b, a);
}

void SBubbleValue::Finish()
{
	m_color = RGBA(GetRValue(m_originColor), GetGValue(m_originColor), GetBValue(m_originColor), GetAValue(m_diffColor)); 
	m_diffColor = RGBA(0, 0, 0, 0);
}

SNSEND
