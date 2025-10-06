#include "stdafx.h"
#include "SComboChartData.h"
#include "../model/SAxis.h"
SNSBEGIN

// Static constants
const int SComboChartData::DEFAULT_COMBO_TYPE = 0;
const float SComboChartData::DEFAULT_LINE_ALPHA = 1.0f;
const float SComboChartData::DEFAULT_COLUMN_ALPHA = 0.8f;

SComboChartData::SComboChartData()
    : m_pLineChartData(NULL)
    , m_pColumnChartData(NULL)
    , m_pSecondaryAxisY(NULL)
    , m_comboType(DEFAULT_COMBO_TYPE)
    , m_bUseSeparateYAxes(FALSE)
    , m_bSecondaryAxisOnRight(TRUE)
    , m_lineChartAlpha(DEFAULT_LINE_ALPHA)
    , m_columnChartAlpha(DEFAULT_COLUMN_ALPHA)
{
}

SComboChartData::SComboChartData(const SComboChartData& other)
    : SChartData(other)
    , m_pLineChartData(NULL)
    , m_pColumnChartData(NULL)
    , m_pSecondaryAxisY(NULL)
    , m_comboType(other.m_comboType)
    , m_bUseSeparateYAxes(other.m_bUseSeparateYAxes)
    , m_bSecondaryAxisOnRight(other.m_bSecondaryAxisOnRight)
    , m_lineChartAlpha(other.m_lineChartAlpha)
    , m_columnChartAlpha(other.m_columnChartAlpha)
{
    // Deep copy line chart data
    if (other.m_pLineChartData)
    {
        m_pLineChartData = new SLineChartData(*other.m_pLineChartData);
    }
    
    // Deep copy column chart data
    if (other.m_pColumnChartData)
    {
        m_pColumnChartData = new SColumnChartData(*other.m_pColumnChartData);
    }
    
    // Deep copy secondary Y axis
    if (other.m_pSecondaryAxisY)
    {
        m_pSecondaryAxisY = new SAxis(*other.m_pSecondaryAxisY);
    }
}

SComboChartData::~SComboChartData()
{
    if (m_pLineChartData)
    {
        delete m_pLineChartData;
        m_pLineChartData = NULL;
    }
    
    if (m_pColumnChartData)
    {
        delete m_pColumnChartData;
        m_pColumnChartData = NULL;
    }
    
    if (m_pSecondaryAxisY)
    {
        delete m_pSecondaryAxisY;
        m_pSecondaryAxisY = NULL;
    }
}

void SComboChartData::SetLineChartData(SLineChartData* pData)
{
    if (m_pLineChartData)
    {
        delete m_pLineChartData;
    }
    
    m_pLineChartData = pData;
    
    // If this is the first data set, copy axes from it
    if (m_pLineChartData && !GetAxisX() && !GetAxisY())
    {
        if (m_pLineChartData->GetAxisX())
        {
            SetAxisX(new SAxis(*m_pLineChartData->GetAxisX()));
        }
        if (m_pLineChartData->GetAxisY())
        {
            SetAxisY(new SAxis(*m_pLineChartData->GetAxisY()));
        }
    }
}

void SComboChartData::SetColumnChartData(SColumnChartData* pData)
{
    if (m_pColumnChartData)
    {
        delete m_pColumnChartData;
    }
    
    m_pColumnChartData = pData;
    
    // If this is the first data set, copy axes from it
    if (m_pColumnChartData && !GetAxisX() && !GetAxisY())
    {
        if (m_pColumnChartData->GetAxisX())
        {
            SetAxisX(new SAxis(*m_pColumnChartData->GetAxisX()));
        }
        if (m_pColumnChartData->GetAxisY())
        {
            SetAxisY(new SAxis(*m_pColumnChartData->GetAxisY()));
        }
    }
}

void SComboChartData::SetSecondaryAxisY(SAxis* pAxis)
{
    if (m_pSecondaryAxisY)
    {
        delete m_pSecondaryAxisY;
    }
    
    m_pSecondaryAxisY = pAxis;
}

void SComboChartData::SetLineChartAlpha(float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    m_lineChartAlpha = alpha;
}

void SComboChartData::SetColumnChartAlpha(float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    m_columnChartAlpha = alpha;
}

void SComboChartData::InitializeAnimation()
{
    // Initialize animation for both line and column chart data
    if (m_pLineChartData)
    {
        m_pLineChartData->InitializeAnimation();
    }

    if (m_pColumnChartData)
    {
        m_pColumnChartData->InitializeAnimation();
    }
}

void SComboChartData::Update(float scale)
{
    // Update both line and column chart data
    if (m_pLineChartData)
    {
        m_pLineChartData->Update(scale);
    }

    if (m_pColumnChartData)
    {
        m_pColumnChartData->Update(scale);
    }
}

void SComboChartData::Finish()
{
    // Finish animation for both line and column chart data
    if (m_pLineChartData)
    {
        m_pLineChartData->Finish();
    }

    if (m_pColumnChartData)
    {
        m_pColumnChartData->Finish();
    }
}

SNSEND
