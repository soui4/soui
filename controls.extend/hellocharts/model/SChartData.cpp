#include "stdafx.h"
#include "SChartData.h"
#include "SAxis.h"

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// SChartData implementation

SChartData::SChartData()
    : m_backgroundColor(RGBA(255, 255, 255, 255))
    , m_bAxesEnabled(TRUE)
    , m_bValueSelectionEnabled(TRUE)
    , m_bValueLabelsEnabled(FALSE)
    , m_pAxisX(NULL)
    , m_pAxisY(NULL)
{
}

SChartData::SChartData(const SChartData& other)
    : m_backgroundColor(other.m_backgroundColor)
    , m_bAxesEnabled(other.m_bAxesEnabled)
    , m_bValueSelectionEnabled(other.m_bValueSelectionEnabled)
    , m_bValueLabelsEnabled(other.m_bValueLabelsEnabled)
    , m_pAxisX(NULL)
    , m_pAxisY(NULL)
{
    if (other.m_pAxisX)
    {
        m_pAxisX = new SAxis(*other.m_pAxisX);
    }
    if (other.m_pAxisY)
    {
        m_pAxisY = new SAxis(*other.m_pAxisY);
    }
}

SChartData::~SChartData()
{
    if (m_pAxisX)
    {
        delete m_pAxisX;
        m_pAxisX = NULL;
    }
    if (m_pAxisY)
    {
        delete m_pAxisY;
        m_pAxisY = NULL;
    }
}

void SChartData::SetAxisX(SAxis* pAxis)
{
    if (m_pAxisX != pAxis)
    {
        if (m_pAxisX)
        {
            delete m_pAxisX;
        }
        m_pAxisX = pAxis;
    }
}

void SChartData::SetAxisY(SAxis* pAxis)
{
    if (m_pAxisY != pAxis)
    {
        if (m_pAxisY)
        {
            delete m_pAxisY;
        }
        m_pAxisY = pAxis;
    }
}

//////////////////////////////////////////////////////////////////////////
// SSelectedValue implementation

SSelectedValue::SSelectedValue()
    : m_bIsSet(FALSE)
    , m_firstIndex(-1)
    , m_secondIndex(-1)
{
}

SSelectedValue::SSelectedValue(int firstIndex, int secondIndex)
    : m_bIsSet(TRUE)
    , m_firstIndex(firstIndex)
    , m_secondIndex(secondIndex)
{
}

SSelectedValue::SSelectedValue(const SSelectedValue& other)
    : m_bIsSet(other.m_bIsSet)
    , m_firstIndex(other.m_firstIndex)
    , m_secondIndex(other.m_secondIndex)
{
}

void SSelectedValue::Set(int firstIndex, int secondIndex)
{
    m_bIsSet = TRUE;
    m_firstIndex = firstIndex;
    m_secondIndex = secondIndex;
}

void SSelectedValue::Clear()
{
    m_bIsSet = FALSE;
    m_firstIndex = -1;
    m_secondIndex = -1;
}

//////////////////////////////////////////////////////////////////////////
// SViewport implementation

SViewport::SViewport()
    : m_left(0.0f)
    , m_top(0.0f)
    , m_right(0.0f)
    , m_bottom(0.0f)
{
}

SViewport::SViewport(float left, float top, float right, float bottom)
    : m_left(left)
    , m_top(top)
    , m_right(right)
    , m_bottom(bottom)
{
}

SViewport::SViewport(const SViewport& other)
    : m_left(other.m_left)
    , m_top(other.m_top)
    , m_right(other.m_right)
    , m_bottom(other.m_bottom)
{
}

void SViewport::Set(float left, float top, float right, float bottom)
{
    m_left = left;
    m_top = top;
    m_right = right;
    m_bottom = bottom;
}

BOOL SViewport::Contains(float x, float y) const
{
    return x >= m_left && x <= m_right && y >= m_top && y <= m_bottom;
}

BOOL SViewport::IsValid() const
{
    return m_right > m_left && m_bottom > m_top;
}

//////////////////////////////////////////////////////////////////////////
// SPointValue implementation

SPointValue::SPointValue()
    : m_x(0.0f)
    , m_y(0.0f)
    , m_color(RGBA(0, 0, 0, 255))
    , m_bHasCustomColor(FALSE)
    , m_originX(0.0f)
    , m_originY(0.0f)
    , m_diffX(0.0f)
    , m_diffY(0.0f)
{
}

SPointValue::SPointValue(float x, float y)
    : m_x(x)
    , m_y(y)
    , m_color(RGBA(0, 0, 0, 255))
    , m_bHasCustomColor(FALSE)
    , m_originX(x)
    , m_originY(y)
    , m_diffX(0.0f)
    , m_diffY(0.0f)
{
}

SPointValue::SPointValue(float x, float y, const SStringT& label)
    : m_x(x)
    , m_y(y)
    , m_label(label)
    , m_color(RGBA(0, 0, 0, 255))
    , m_bHasCustomColor(FALSE)
    , m_originX(x)
    , m_originY(y)
    , m_diffX(0.0f)
    , m_diffY(0.0f)
{
}

SPointValue::SPointValue(const SPointValue& other)
    : m_x(other.m_x)
    , m_y(other.m_y)
    , m_label(other.m_label)
    , m_color(other.m_color)
    , m_bHasCustomColor(other.m_bHasCustomColor)
    , m_originX(other.m_originX)
    , m_originY(other.m_originY)
    , m_diffX(other.m_diffX)
    , m_diffY(other.m_diffY)
{
}

SPointValue::~SPointValue()
{
}

void SPointValue::InitializeAnimation()
{
    // Set origin to current position and diff to target position
    m_originX = 0.0f;  // Start from origin
    m_originY = 0.0f;
    m_diffX = m_x;     // Animate to current position
    m_diffY = m_y;

    // Reset current position to origin for animation
    m_x = m_originX;
    m_y = m_originY;
}

void SPointValue::Update(float scale)
{
    m_x = m_originX + m_diffX * scale;
    m_y = m_originY + m_diffY * scale;
}

void SPointValue::Finish()
{
    SetOriginX(m_x);
    SetOriginY(m_y);
    SetDiffX(0.0f);
    SetDiffY(0.0f);
}

SNSEND
