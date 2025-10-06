#include "stdafx.h"
#include "SLineChartData.h"

SNSBEGIN

// Static constants
const float SLineChartData::DEFAULT_BASE_VALUE = 0.0f;
const COLORREF SLine::DEFAULT_LINE_COLOR = RGBA(51, 181, 229, 255);
const int SLine::DEFAULT_STROKE_WIDTH = 3;
const int SLine::DEFAULT_POINT_RADIUS = 4;
const int SLine::DEFAULT_AREA_TRANSPARENCY = 64;

//////////////////////////////////////////////////////////////////////////
// SLineChartData implementation

SLineChartData::SLineChartData()
    : SChartData()
    , m_baseValue(DEFAULT_BASE_VALUE)
{
}

SLineChartData::SLineChartData(const std::vector<SLine*>& lines)
    : SChartData()
    , m_baseValue(DEFAULT_BASE_VALUE)
{
    SetLines(lines);
}

SLineChartData::SLineChartData(const SLineChartData& other)
    : SChartData(other)
    , m_baseValue(other.m_baseValue)
{
    // Deep copy lines
    for (size_t i = 0; i < other.m_lines.size(); ++i)
    {
        if (other.m_lines[i])
        {
            m_lines.push_back(new SLine(*other.m_lines[i]));
        }
    }
}

SLineChartData::~SLineChartData()
{
    ClearLines();
}

void SLineChartData::SetLines(const std::vector<SLine*>& lines)
{
    ClearLines();
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (lines[i])
        {
            m_lines.push_back(new SLine(*lines[i]));
        }
    }
}

void SLineChartData::AddLine(SLine* pLine)
{
    if (pLine)
    {
        m_lines.push_back(pLine);
    }
}

void SLineChartData::ClearLines()
{
    for (size_t i = 0; i < m_lines.size(); ++i)
    {
        if (m_lines[i])
        {
            delete m_lines[i];
        }
    }
    m_lines.clear();
}

SLine* SLineChartData::GetLine(size_t index) const
{
    if (index < m_lines.size())
    {
        return m_lines[index];
    }
    return NULL;
}

void SLineChartData::InitializeAnimation()
{
    for (size_t i = 0; i < m_lines.size(); ++i)
    {
        if (m_lines[i])
        {
            m_lines[i]->InitializeAnimation();
        }
    }
}

void SLineChartData::Update(float scale)
{
    for (size_t i = 0; i < m_lines.size(); ++i)
    {
        if (m_lines[i])
        {
            m_lines[i]->Update(scale);
        }
    }
}

void SLineChartData::Finish()
{
    for (size_t i = 0; i < m_lines.size(); ++i)
    {
        if (m_lines[i])
        {
            m_lines[i]->Finish();
        }
    }
}

SLineChartData* SLineChartData::GenerateDummyData()
{
    SLineChartData* pData = new SLineChartData();
    
    // Create a simple line with 4 points
    std::vector<SPointValue*> values;
    values.push_back(new SPointValue(0.0f, 2.0f));
    values.push_back(new SPointValue(1.0f, 4.0f));
    values.push_back(new SPointValue(2.0f, 3.0f));
    values.push_back(new SPointValue(3.0f, 4.0f));
    
    SLine* pLine = new SLine(values);
    pLine->SetColor(SLine::DEFAULT_LINE_COLOR);
    pLine->SetHasPoints(TRUE);
    
    pData->AddLine(pLine);
    
    return pData;
}

//////////////////////////////////////////////////////////////////////////
// SLine implementation

SLine::SLine()
    : m_color(DEFAULT_LINE_COLOR)
    , m_strokeWidth(DEFAULT_STROKE_WIDTH)
    , m_bCubic(FALSE)
    , m_bFilled(FALSE)
    , m_bHasPoints(TRUE)
    , m_bHasLabels(FALSE)
    , m_pointRadius(DEFAULT_POINT_RADIUS)
    , m_areaTransparency(DEFAULT_AREA_TRANSPARENCY)
{
}

SLine::SLine(const std::vector<SPointValue*>& values)
    : m_color(DEFAULT_LINE_COLOR)
    , m_strokeWidth(DEFAULT_STROKE_WIDTH)
    , m_bCubic(FALSE)
    , m_bFilled(FALSE)
    , m_bHasPoints(TRUE)
    , m_bHasLabels(FALSE)
    , m_pointRadius(DEFAULT_POINT_RADIUS)
    , m_areaTransparency(DEFAULT_AREA_TRANSPARENCY)
{
    SetValues(values);
}

SLine::SLine(const SLine& other)
    : m_color(other.m_color)
    , m_strokeWidth(other.m_strokeWidth)
    , m_bCubic(other.m_bCubic)
    , m_bFilled(other.m_bFilled)
    , m_bHasPoints(other.m_bHasPoints)
    , m_bHasLabels(other.m_bHasLabels)
    , m_pointRadius(other.m_pointRadius)
    , m_areaTransparency(other.m_areaTransparency)
{
    // Deep copy values
    for (size_t i = 0; i < other.m_values.size(); ++i)
    {
        if (other.m_values[i])
        {
            m_values.push_back(new SPointValue(*other.m_values[i]));
        }
    }
}

SLine::~SLine()
{
    ClearValues();
}

void SLine::SetValues(const std::vector<SPointValue*>& values)
{
    ClearValues();
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (values[i])
        {
            m_values.push_back(new SPointValue(*values[i]));
        }
    }
}

void SLine::AddValue(SPointValue* pValue)
{
    if (pValue)
    {
        m_values.push_back(pValue);
    }
}

void SLine::ClearValues()
{
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        if (m_values[i])
        {
            delete m_values[i];
        }
    }
    m_values.clear();
}

SPointValue* SLine::GetValue(size_t index) const
{
    if (index < m_values.size())
    {
        return m_values[index];
    }
    return NULL;
}

void SLine::InitializeAnimation()
{
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        if (m_values[i])
        {
            m_values[i]->InitializeAnimation();
        }
    }
}

void SLine::Update(float scale)
{
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        if (m_values[i])
        {
            m_values[i]->Update(scale);
        }
    }
}

void SLine::Finish()
{
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        if (m_values[i])
        {
            m_values[i]->Finish();
        }
    }
}

SNSEND
