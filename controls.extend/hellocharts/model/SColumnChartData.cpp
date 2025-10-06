#include "stdafx.h"
#include "SColumnChartData.h"
#include <time.h>

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// SSubcolumnValue implementation

SSubcolumnValue::SSubcolumnValue(float value)
    : SPointValue(0.0f, value)
    , m_diff(0.0f)
    , m_targetY(value)
{
}

SSubcolumnValue::SSubcolumnValue(float value, COLORREF color)
    : SPointValue(0.0f, value)
    , m_diff(0.0f)
    , m_targetY(value)
{
    SetColor(color);
}

void SSubcolumnValue::InitializeAnimation()
{
    // Set current Y to 0 and target to original value for grow-up animation
    m_targetY = GetY();
    SetY(0.0f);
}

//////////////////////////////////////////////////////////////////////////
// SColumn implementation

SColumn::SColumn(const std::vector<SSubcolumnValue*>& values)
    : m_values(values)
    , m_color(DEFAULT_COLUMN_COLOR)
    , m_strokeWidth(DEFAULT_COLUMN_STROKE_WIDTH)
    , m_bHasLabels(FALSE)
{
}

SColumn::~SColumn()
{
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        delete m_values[i];
    }
    m_values.clear();
}

SSubcolumnValue* SColumn::GetValue(size_t index) const
{
    if (index < m_values.size())
        return m_values[index];
    return NULL;
}

void SColumn::AddValue(SSubcolumnValue* pValue)
{
    if (pValue)
        m_values.push_back(pValue);
}

//////////////////////////////////////////////////////////////////////////
// SColumnChartData implementation

SColumnChartData::SColumnChartData()
    : m_bStacked(FALSE)
    , m_fillRatio(0.75f)
{
    SetBackgroundColor(RGBA(255, 255, 255, 255));
}

SColumnChartData::~SColumnChartData()
{
    for (size_t i = 0; i < m_columns.size(); ++i)
    {
        delete m_columns[i];
    }
    m_columns.clear();
}

SColumn* SColumnChartData::GetColumn(size_t index) const
{
    if (index < m_columns.size())
        return m_columns[index];
    return NULL;
}

void SColumnChartData::AddColumn(SColumn* pColumn)
{
    if (pColumn)
        m_columns.push_back(pColumn);
}

void SColumnChartData::InitializeAnimation()
{
    // Initialize animation for all columns
    for (size_t i = 0; i < m_columns.size(); ++i)
    {
        SColumn* pColumn = m_columns[i];
        if (pColumn)
        {
            for (size_t j = 0; j < pColumn->GetValueCount(); ++j)
            {
                SSubcolumnValue* pValue = pColumn->GetValue(j);
                if (pValue)
                {
                    pValue->InitializeAnimation();
                }
            }
        }
    }
}

void SColumnChartData::Update(float scale)
{
    // Animation update logic for columns
    for (size_t i = 0; i < m_columns.size(); ++i)
    {
        SColumn* pColumn = m_columns[i];
        if (pColumn)
        {
            for (size_t j = 0; j < pColumn->GetValueCount(); ++j)
            {
                SSubcolumnValue* pValue = pColumn->GetValue(j);
                if (pValue)
                {
                    float targetY = pValue->GetTargetY();
                    float currentY = pValue->GetY();
                    float newY = currentY + (targetY - currentY) * scale;
                    pValue->SetY(newY);
                }
            }
        }
    }
}

void SColumnChartData::Finish()
{
    // Animation finish logic for columns
    for (size_t i = 0; i < m_columns.size(); ++i)
    {
        SColumn* pColumn = m_columns[i];
        if (pColumn)
        {
            for (size_t j = 0; j < pColumn->GetValueCount(); ++j)
            {
                SSubcolumnValue* pValue = pColumn->GetValue(j);
                if (pValue)
                {
                    pValue->SetY(pValue->GetTargetY());
                }
            }
        }
    }
}

SColumnChartData* SColumnChartData::GenerateDummyData()
{
    SColumnChartData* pData = new SColumnChartData();
    
    // Create random data using traditional C++ rand() function
    srand((unsigned int)time(NULL));
    
    // Create 5 columns with single subcolumn each
    for (int i = 0; i < 5; ++i)
    {
        std::vector<SSubcolumnValue*> values;
        // Generate random value between 1.0f and 10.0f
        float value = 1.0f + (float)(rand() % 900) / 100.0f;
        SSubcolumnValue* pValue = new SSubcolumnValue(value);
        values.push_back(pValue);
        
        SColumn* pColumn = new SColumn(values);
        
        // Set different colors for each column
        COLORREF colors[] = {
            RGBA(51, 181, 229, 255),   // Blue
            RGBA(255, 87, 87, 255),    // Red
            RGBA(76, 175, 80, 255),    // Green
            RGBA(255, 193, 7, 255),    // Yellow
            RGBA(156, 39, 176, 255)    // Purple
        };
        pColumn->SetColor(colors[i % 5]);
        pColumn->SetHasLabels(TRUE);
        
        pData->AddColumn(pColumn);
    }
    
    return pData;
}

SNSEND
