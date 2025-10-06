#include "stdafx.h"
#include "SComboChartView.h"
#include "../renderer/SComboChartRenderer.h"

SNSBEGIN

// Static constants
const int SComboChartView::DEFAULT_COMBO_TYPE = 0;
const float SComboChartView::DEFAULT_LINE_ALPHA = 1.0f;
const float SComboChartView::DEFAULT_COLUMN_ALPHA = 0.8f;

SComboChartView::SComboChartView()
    : m_pData(NULL)
    , m_pComboChartRenderer(NULL)
    , m_pOnValueSelectListener(NULL)
    , m_comboType(DEFAULT_COMBO_TYPE)
    , m_bUseSeparateYAxes(FALSE)
    , m_bSecondaryAxisOnRight(TRUE)
    , m_lineChartAlpha(DEFAULT_LINE_ALPHA)
    , m_columnChartAlpha(DEFAULT_COLUMN_ALPHA)
{
}

SComboChartView::~SComboChartView()
{
    if (m_pData)
    {
        delete m_pData;
        m_pData = NULL;
    }
}

SChartData* SComboChartView::GetChartData()
{
    return m_pData;
}

void SComboChartView::SetComboChartData(SComboChartData* pData)
{
    if (m_pData)
    {
        delete m_pData;
    }
    
    m_pData = pData;

    // Refresh axes margins after data change
    RefreshAxesMargins();

    if (m_pData)
    {
        // Apply current settings to data
        m_pData->SetComboType(m_comboType);
        m_pData->SetUseSeparateYAxes(m_bUseSeparateYAxes);
        m_pData->SetSecondaryAxisOnRight(m_bSecondaryAxisOnRight);
        m_pData->SetLineChartAlpha(m_lineChartAlpha);
        m_pData->SetColumnChartAlpha(m_columnChartAlpha);
        
        // Calculate combined viewport
        CalculateCombinedViewport();
    }
    
    // Trigger repaint
    Invalidate();
}

void SComboChartView::SetOnValueSelectListener(IComboChartOnValueSelectListener* pListener)
{
    m_pOnValueSelectListener = pListener;
}

void SComboChartView::SetComboType(int type)
{
    m_comboType = type;
    
    if (m_pData)
    {
        m_pData->SetComboType(type);
        Invalidate();
    }
}

void SComboChartView::SetUseSeparateYAxes(BOOL bUseSeparate)
{
    m_bUseSeparateYAxes = bUseSeparate;
    
    if (m_pData)
    {
        m_pData->SetUseSeparateYAxes(bUseSeparate);
        Invalidate();
    }
}

void SComboChartView::SetSecondaryAxisOnRight(BOOL bOnRight)
{
    m_bSecondaryAxisOnRight = bOnRight;
    
    if (m_pData)
    {
        m_pData->SetSecondaryAxisOnRight(bOnRight);
        Invalidate();
    }
}

void SComboChartView::SetLineChartAlpha(float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    m_lineChartAlpha = alpha;
    
    if (m_pData)
    {
        m_pData->SetLineChartAlpha(alpha);
        Invalidate();
    }
}

void SComboChartView::SetColumnChartAlpha(float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    m_columnChartAlpha = alpha;
    
    if (m_pData)
    {
        m_pData->SetColumnChartAlpha(alpha);
        Invalidate();
    }
}

SLineChartData* SComboChartView::GetLineChartDataForRenderer() const
{
    return m_pData ? m_pData->GetLineChartData() : NULL;
}

SColumnChartData* SComboChartView::GetColumnChartDataForRenderer() const
{
    return m_pData ? m_pData->GetColumnChartData() : NULL;
}

void SComboChartView::InitializeRenderers()
{
    m_pComboChartRenderer = new SComboChartRenderer(this);
    m_pComboChartRenderer->SetComboChartView(this);
    m_pChartRenderer = m_pComboChartRenderer;
}

void SComboChartView::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_bInteractive && m_pComboChartRenderer)
    {
        if (m_pComboChartRenderer->IsTouched((float)point.x, (float)point.y))
        {
            CallTouchListener();
            Invalidate();
        }
    }
    
    __baseCls::OnLButtonDown(nFlags, point);
}

void SComboChartView::CallTouchListener()
{
    if (m_pOnValueSelectListener && m_pComboChartRenderer)
    {
        SSelectedValue selectedValue = m_pComboChartRenderer->GetSelectedValue();
        if (selectedValue.IsSet())
        {
            int chartType = m_pComboChartRenderer->IsLineChartSelection() ? 0 : 1;
            m_pOnValueSelectListener->OnValueSelected(
                chartType,
                selectedValue.GetFirstIndex(),
                selectedValue.GetSecondIndex()
            );
        }
        else
        {
            m_pOnValueSelectListener->OnValueDeselected();
        }
    }
}

void SComboChartView::OnComboValueSelected(int chartType, int valueIndex, int subValueIndex)
{
    // Default implementation - can be overridden by derived classes
    // This method is called by the default listener
}

void SComboChartView::OnComboValueDeselected()
{
    // Default implementation - can be overridden by derived classes
    // This method is called by the default listener
}

void SComboChartView::CalculateCombinedViewport()
{
    if (!m_pData)
        return;

    float minX = 0.0f, maxX = 0.0f, minY = 0.0f, maxY = 0.0f;
    BOOL hasData = FALSE;

    // Calculate viewport from line chart data
    if (m_pData->HasLineData())
    {
        SLineChartData* pLineData = m_pData->GetLineChartData();
        if (pLineData && pLineData->GetLineCount() > 0)
        {
            for (size_t i = 0; i < pLineData->GetLineCount(); ++i)
            {
                SLine* pLine = pLineData->GetLine(i);
                if (pLine && pLine->GetValueCount() > 0)
                {
                    for (size_t j = 0; j < pLine->GetValueCount(); ++j)
                    {
                        SPointValue* pPoint = pLine->GetValue(j);
                        if (pPoint)
                        {
                            float x = pPoint->GetX();
                            float y = pPoint->GetY();
                            
                            if (!hasData)
                            {
                                minX = maxX = x;
                                minY = maxY = y;
                                hasData = TRUE;
                            }
                            else
                            {
                                if (x < minX) minX = x;
                                if (x > maxX) maxX = x;
                                if (y < minY) minY = y;
                                if (y > maxY) maxY = y;
                            }
                        }
                    }
                }
            }
        }
    }

    // Calculate viewport from column chart data
    if (m_pData->HasColumnData())
    {
        SColumnChartData* pColumnData = m_pData->GetColumnChartData();
        if (pColumnData && pColumnData->GetColumnCount() > 0)
        {
            for (size_t i = 0; i < pColumnData->GetColumnCount(); ++i)
            {
                SColumn* pColumn = pColumnData->GetColumn(i);
                if (pColumn && pColumn->GetValueCount() > 0)
                {
                    for (size_t j = 0; j < pColumn->GetValueCount(); ++j)
                    {
                        SSubcolumnValue* pValue = pColumn->GetValue(j);
                        if (pValue)
                        {
                            float x = (float)i; // Column index as X
                            float y = pValue->GetY();
                            
                            if (!hasData)
                            {
                                minX = maxX = x;
                                minY = maxY = y;
                                hasData = TRUE;
                            }
                            else
                            {
                                if (x < minX) minX = x;
                                if (x > maxX) maxX = x;
                                if (y < minY) minY = y;
                                if (y > maxY) maxY = y;
                            }
                        }
                    }
                }
            }
        }
    }

    if (hasData)
    {
        // Add some padding
        float paddingX = (maxX - minX) * 0.1f;
        float paddingY = (maxY - minY) * 0.1f;
        
        if (paddingX == 0.0f) paddingX = 1.0f;
        if (paddingY == 0.0f) paddingY = 1.0f;
        
        SViewport viewport(minX - paddingX, minY - paddingY,
                         maxX + paddingX, maxY + paddingY);
        SetMaximumViewport(viewport);
        SetCurrentViewport(viewport);
    }
}

SNSEND
