#include "stdafx.h"
#include "SLineChartView.h"
#include "../renderer/SLineChartRenderer.h"
#include <float.h>
SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// SLineChartView implementation

SLineChartView::SLineChartView()
    : SAbstractChartView()
    , m_pData(NULL)
    , m_pOnValueSelectListener(NULL)
{
    // Create default dummy data
    SetLineChartData(SLineChartData::GenerateDummyData());
}

SLineChartView::~SLineChartView()
{
    if (m_pData)
    {
        delete m_pData;
        m_pData = NULL;
    }
}

void SLineChartView::SetLineChartData(SLineChartData* pData)
{
    if (m_pData != pData)
    {
        if (m_pData)
        {
            delete m_pData;
        }
        
        m_pData = pData ? pData : SLineChartData::GenerateDummyData();

        // Refresh axes margins after data change
        RefreshAxesMargins();

        // Calculate viewport based on data
        if (m_pData && m_pData->GetLineCount() > 0)
        {
            float minX = FLT_MAX, maxX = -FLT_MAX;
            float minY = FLT_MAX, maxY = -FLT_MAX;
            
            for (size_t i = 0; i < m_pData->GetLineCount(); ++i)
            {
                SLine* pLine = m_pData->GetLine(i);
                if (pLine)
                {
                    for (size_t j = 0; j < pLine->GetValueCount(); ++j)
                    {
                        SPointValue* pValue = pLine->GetValue(j);
                        if (pValue)
                        {
                            minX = (std::min)(minX, pValue->GetX());
                            maxX = (std::max)(maxX, pValue->GetX());
                            minY = (std::min)(minY, pValue->GetY());
                            maxY = (std::max)(maxY, pValue->GetY());
                        }
                    }
                }
            }
            
            // Add some padding
            float paddingX = (maxX - minX) * 0.1f;
            float paddingY = (maxY - minY) * 0.1f;
            
            if (paddingX == 0.0f) paddingX = 1.0f;
            if (paddingY == 0.0f) paddingY = 1.0f;
            
            SViewport viewport(minX - paddingX, minY - paddingY, 
                              maxX + paddingX, maxY + paddingY);
            SetMaximumViewport(viewport);
        }
        
        OnChartDataChange();
    }
}

void SLineChartView::CallTouchListener()
{
    if (!m_pOnValueSelectListener)
        return;
        
    ILineChartOnValueSelectListener* pListener = m_pOnValueSelectListener;
    
    SSelectedValue selectedValue = GetSelectedValue();
    
    if (selectedValue.IsSet() && m_pData)
    {
        int lineIndex = selectedValue.GetFirstIndex();
        int pointIndex = selectedValue.GetSecondIndex();
        
        SLine* pLine = m_pData->GetLine(lineIndex);
        if (pLine)
        {
            SPointValue* pValue = pLine->GetValue(pointIndex);
            if (pValue)
            {
                pListener->OnValueSelected(lineIndex, pointIndex, pValue);
                return;
            }
        }
    }
    
    pListener->OnValueDeselected();
}

void SLineChartView::InitializeRenderers()
{
    SAbstractChartView::InitializeRenderers();

    // Create line chart renderer
    SLineChartRenderer* pRenderer = new SLineChartRenderer(this, this);
    SetChartRenderer(pRenderer);
}

SNSEND
