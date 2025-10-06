#include "stdafx.h"
#include "SColumnChartView.h"
#include "../renderer/SColumnChartRenderer.h"

SNSBEGIN

SColumnChartView::SColumnChartView()
    : SAbstractChartView()
    , m_pData(NULL)
    , m_pOnValueSelectListener(NULL)
    , m_bStacked(FALSE)
    , m_fillRatio(0.75f)
{
    // Note: Don't set data in constructor, do it after window is fully initialized
}

SColumnChartView::~SColumnChartView()
{
    if (m_pData)
    {
        delete m_pData;
        m_pData = NULL;
    }
}

void SColumnChartView::SetColumnChartData(SColumnChartData* pData)
{
    if (m_pData && m_pData != pData)
    {
        delete m_pData;
    }
    
    m_pData = pData;

    // Refresh axes margins after data change
    RefreshAxesMargins();

    if (m_pData)
    {
        // Apply attributes to data
        m_pData->SetStacked(m_bStacked);
        m_pData->SetFillRatio(m_fillRatio);
        
        // Update viewport based on data
        if (m_pChartComputator)
        {
            // Calculate data bounds
            // X axis: add padding to show complete columns
            // Column centers are at 0, 1, 2, ... so we need -0.5 to columnCount-0.5
            float minX = -0.5f;
            float maxX = (float)m_pData->GetColumnCount() - 0.5f;
            float minY = 0.0f;
            float maxY = 0.0f;
            
            // Find min/max Y values
            for (size_t i = 0; i < m_pData->GetColumnCount(); ++i)
            {
                SColumn* pColumn = m_pData->GetColumn(i);
                if (pColumn)
                {
                    if (m_pData->IsStacked())
                    {
                        // For stacked columns, sum all values
                        float stackSum = 0.0f;
                        for (size_t j = 0; j < pColumn->GetValueCount(); ++j)
                        {
                            SSubcolumnValue* pValue = pColumn->GetValue(j);
                            if (pValue)
                            {
                                stackSum += pValue->GetY();
                            }
                        }
                        if (stackSum > maxY) maxY = stackSum;
                        if (stackSum < minY) minY = stackSum;
                    }
                    else
                    {
                        // For grouped columns, find individual max/min
                        for (size_t j = 0; j < pColumn->GetValueCount(); ++j)
                        {
                            SSubcolumnValue* pValue = pColumn->GetValue(j);
                            if (pValue)
                            {
                                float value = pValue->GetY();
                                if (value > maxY) maxY = value;
                                if (value < minY) minY = value;
                            }
                        }
                    }
                }
            }
            
            // Add some padding
            float yRange = maxY - minY;
            if (yRange > 0)
            {
                minY -= yRange * 0.1f;
                maxY += yRange * 0.1f;
            }
            else
            {
                minY = -1.0f;
                maxY = 1.0f;
            }
            
            // Set viewport
            SViewport viewport(minX, minY, maxX, maxY);
            m_pChartComputator->SetMaxViewport(viewport);
            m_pChartComputator->SetCurrentViewport(viewport);
        }
    }
    
    // Trigger repaint
    Invalidate();
}

void SColumnChartView::CallTouchListener()
{
    // Call the touch listener if available
    if (m_pOnValueSelectListener && m_pChartRenderer)
    {
        SColumnChartRenderer* pColumnRenderer = dynamic_cast<SColumnChartRenderer*>(m_pChartRenderer);
        if (pColumnRenderer)
        {
            // Get selected value from renderer
            SSelectedValue selectedValue = pColumnRenderer->GetSelectedValue();
            if (selectedValue.IsSet())
            {
                // Get the actual subcolumn value from the data
                SSubcolumnValue* pValue = NULL;
                if (m_pData && selectedValue.GetFirstIndex() >= 0 &&
                    selectedValue.GetFirstIndex() < (int)m_pData->GetColumns().size())
                {
                    const SColumn* pColumn = m_pData->GetColumns()[selectedValue.GetFirstIndex()];
                    if (pColumn && selectedValue.GetSecondIndex() >= 0 &&
                        selectedValue.GetSecondIndex() < (int)pColumn->GetValues().size())
                    {
                        pValue = pColumn->GetValues()[selectedValue.GetSecondIndex()];
                    }
                }

                // Call listener with column-specific parameters
                m_pOnValueSelectListener->OnValueSelected(
                    selectedValue.GetFirstIndex(),
                    selectedValue.GetSecondIndex(),
                    pValue
                );
            }
            else
            {
                m_pOnValueSelectListener->OnValueDeselected();
            }
        }
    }
}

void SColumnChartView::InitializeRenderers()
{
    SAbstractChartView::InitializeRenderers();

    // Create column chart renderer
    SColumnChartRenderer* pRenderer = new SColumnChartRenderer(this, this);
    SetChartRenderer(pRenderer);
}

void SColumnChartView::OnInitFinished(IXmlNode* xmlNode)
{
    SAbstractChartView::OnInitFinished(xmlNode);
    
    // Now that the window is fully initialized, set default data if none exists
    if (!m_pData)
    {
        SetColumnChartData(SColumnChartData::GenerateDummyData());
    }
}

void SColumnChartView::OnValueSelected(int columnIndex, int subcolumnIndex)
{
    if (m_pOnValueSelectListener && m_pData)
    {
        SColumn* pColumn = m_pData->GetColumn(columnIndex);
        if (pColumn)
        {
            SSubcolumnValue* pValue = pColumn->GetValue(subcolumnIndex);
            if (pValue)
            {
                m_pOnValueSelectListener->OnValueSelected(columnIndex, subcolumnIndex, pValue);
            }
        }
    }
}

void SColumnChartView::OnValueDeselected()
{
    if (m_pOnValueSelectListener)
    {
        m_pOnValueSelectListener->OnValueDeselected();
    }
}

SNSEND
