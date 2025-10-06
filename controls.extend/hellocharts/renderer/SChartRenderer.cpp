#include "stdafx.h"
#include "SChartRenderer.h"
#include "../SChart.h"
#include "../model/SAxis.h"
#include "../computator/SChartComputator.h"

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// SChartRenderer implementation

SChartRenderer::SChartRenderer(ISChart* pChart)
    : m_pChart(pChart)
{
}

SChartRenderer::~SChartRenderer()
{
}

//////////////////////////////////////////////////////////////////////////
// SAxesRenderer implementation

SAxesRenderer::SAxesRenderer(ISChart* pChart)
    : m_pChart(pChart)
    , m_axesMarginLeft(DEFAULT_AXIS_MARGIN)
    , m_axesMarginTop(DEFAULT_AXIS_MARGIN)
    , m_axesMarginRight(DEFAULT_AXIS_MARGIN)
    , m_axesMarginBottom(DEFAULT_AXIS_MARGIN)
{
}

SAxesRenderer::~SAxesRenderer()
{
}

void SAxesRenderer::DrawInBackground(IRenderTarget* pRT)
{
    if (!m_pChart || !pRT)
        return;
        
    SChartData* pData = m_pChart->GetChartData();
    if (!pData || !pData->IsAxesEnabled())
        return;
        
    DrawAxisLines(pRT);
}

void SAxesRenderer::DrawInForeground(IRenderTarget* pRT)
{
    if (!m_pChart || !pRT)
        return;
        
    SChartData* pData = m_pChart->GetChartData();
    if (!pData || !pData->IsAxesEnabled())
        return;
        
    DrawAxisLabels(pRT);
}

void SAxesRenderer::OnChartSizeChanged(IRenderTarget* pRT)
{
    CalculateAxesMargins(pRT);
    
    if (m_pChart && m_pChart->GetChartComputator())
    {
        m_pChart->GetChartComputator()->SetChartAreaMargins(
            m_axesMarginLeft, m_axesMarginTop, m_axesMarginRight, m_axesMarginBottom);
    }
}

void SAxesRenderer::SetAxesMargin(int left, int top, int right, int bottom)
{
    m_axesMarginLeft = left;
    m_axesMarginTop = top;
    m_axesMarginRight = right;
    m_axesMarginBottom = bottom;
}

void SAxesRenderer::GetAxesMargin(int* pLeft, int* pTop, int* pRight, int* pBottom) const
{
    if (pLeft) *pLeft = m_axesMarginLeft;
    if (pTop) *pTop = m_axesMarginTop;
    if (pRight) *pRight = m_axesMarginRight;
    if (pBottom) *pBottom = m_axesMarginBottom;
}

void SAxesRenderer::DrawXAxis(IRenderTarget* pRT)
{
    if (!m_pChart || !pRT)
        return;
        
    SChartData* pData = m_pChart->GetChartData();
    SAxis* pAxisX = pData ? pData->GetAxisX() : NULL;
    if (!pAxisX)
        return;
        
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;
        
    CRect chartRect = pComputator->GetContentRectMinusAllMargins();

    // Add window client offset
    CRect rcClient;
    m_pChart->GetRect(&rcClient);
    chartRect.OffsetRect(rcClient.left, rcClient.top);

    // Draw axis line
    if (pAxisX->HasLines())
    {
        SAutoRefPtr<IPenS> pen,oldPen;
        pRT->CreatePen(PS_SOLID, pAxisX->GetLineColor(), 1, &pen);
        pRT->SelectObject(pen,(IRenderObj **)&oldPen);

        POINT pt1 = {chartRect.left, chartRect.bottom};
        POINT pt2 = {chartRect.right, chartRect.bottom};
        pRT->DrawLine(pt1, pt2);

        pRT->SelectObject(oldPen);
    }
}

void SAxesRenderer::DrawYAxis(IRenderTarget* pRT)
{
    if (!m_pChart || !pRT)
        return;
        
    SChartData* pData = m_pChart->GetChartData();
    SAxis* pAxisY = pData ? pData->GetAxisY() : NULL;
    if (!pAxisY)
        return;
        
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;
        
    CRect chartRect = pComputator->GetContentRectMinusAllMargins();

    // Add window client offset
    CRect rcClient;
    m_pChart->GetRect(&rcClient);
    chartRect.OffsetRect(rcClient.left, rcClient.top);

    // Draw axis line
    if (pAxisY->HasLines())
    {
        SAutoRefPtr<IPenS> pen,oldPen;
        pRT->CreatePen(PS_SOLID, pAxisY->GetLineColor(), 1, &pen);
        pRT->SelectObject(pen,(IRenderObj **)&oldPen);

        POINT pt1 = {chartRect.left, chartRect.top};
        POINT pt2 = {chartRect.left, chartRect.bottom};
        pRT->DrawLine(pt1, pt2);
        
        pRT->SelectObject(oldPen);
    }
}

void SAxesRenderer::DrawAxisLines(IRenderTarget* pRT)
{
    DrawXAxis(pRT);
    DrawYAxis(pRT);
}

void SAxesRenderer::DrawAxisLabels(IRenderTarget* pRT)
{
    if (!m_pChart || !pRT)
        return;
        
    SChartData* pData = m_pChart->GetChartData();
    if (!pData)
        return;
        
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;
        
    CRect chartRect = pComputator->GetContentRectMinusAllMargins();
    CRect rcClient;
    m_pChart->GetRect(&rcClient);
    chartRect.OffsetRect(rcClient.left, rcClient.top);
    // Draw X axis labels
    SAxis* pAxisX = pData->GetAxisX();
    if (pAxisX)
    {
        COLORREF crTextColor = pAxisX->GetTextColor();
        COLORREF crOldTextColor = pRT->SetTextColor(crTextColor);
        const std::vector<SAxisValue*>& xValues = pAxisX->GetValues();
        for (size_t i = 0; i < xValues.size(); ++i)
        {
            SAxisValue* pValue = xValues[i];
            if (pValue && !pValue->GetLabel().IsEmpty())
            {
                float pixelX = pComputator->ComputeRawX(pValue->GetValue());
                // Add window client offset for X coordinate
                int adjustedPixelX = (int)pixelX + rcClient.left;
                CRect textRect(adjustedPixelX - 50, chartRect.bottom + 5,
                              adjustedPixelX + 50, chartRect.bottom + 25);

                pRT->DrawText(pValue->GetLabel(), pValue->GetLabel().GetLength(),
                             &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }
        pRT->SetTextColor(crOldTextColor);
    }
    
    // Draw Y axis labels
    SAxis* pAxisY = pData->GetAxisY();
    if (pAxisY)
    {
        COLORREF crTextColor = pAxisY->GetTextColor();
        COLORREF crOldTextColor = pRT->SetTextColor(crTextColor);
        const std::vector<SAxisValue*>& yValues = pAxisY->GetValues();
        for (size_t i = 0; i < yValues.size(); ++i)
        {
            SAxisValue* pValue = yValues[i];
            if (pValue && !pValue->GetLabel().IsEmpty())
            {
                float pixelY = pComputator->ComputeRawY(pValue->GetValue());
                // Add window client offset for Y coordinate
                int adjustedPixelY = (int)pixelY + rcClient.top;
                CRect textRect(chartRect.left - 60, adjustedPixelY - 10,
                              chartRect.left - 5, adjustedPixelY + 10);

                pRT->DrawText(pValue->GetLabel(), pValue->GetLabel().GetLength(),
                             &textRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
            }
        }
        pRT->SetTextColor(crOldTextColor);
    }
}

void SAxesRenderer::CalculateAxesMargins(IRenderTarget* pRT)
{
    if (!m_pChart)
    {
        m_axesMarginLeft = DEFAULT_AXIS_MARGIN;
        m_axesMarginTop = DEFAULT_AXIS_MARGIN;
        m_axesMarginRight = DEFAULT_AXIS_MARGIN;
        m_axesMarginBottom = DEFAULT_AXIS_MARGIN;
        return;
    }

    SChartData* pData = m_pChart->GetChartData();
    if (!pData || !pData->IsAxesEnabled())
    {
        m_axesMarginLeft = 10;
        m_axesMarginTop = 10;
        m_axesMarginRight = 10;
        m_axesMarginBottom = 10;
        return;
    }

    // Calculate margins based on axis content and estimated label sizes
    int leftMargin = 40;   // Base margin for Y axis
    int topMargin = 30;    // Base margin for title
    int rightMargin = 30;  // Base margin
    int bottomMargin = 40; // Base margin for X axis

    // Y axis (left side) - calculate margin based on actual label content
    SAxis* pAxisY = pData->GetAxisY();
    if (pAxisY && pAxisY->GetValueCount() > 0)
    {
        int maxLabelWidth = 0;

        // Check actual labels if available
        for (size_t i = 0; i < pAxisY->GetValueCount(); ++i)
        {
            SAxisValue* pValue = pAxisY->GetValue(i);
            if (pValue)
            {
                SStringT labelText = pValue->GetLabel();
                if (!labelText.IsEmpty())
                {
                    CSize szText;
                    pRT->MeasureText(labelText, labelText.GetLength(), &szText);
                    maxLabelWidth = smax(maxLabelWidth, szText.cx);
                }
            }
        }

        if (maxLabelWidth > 0)
        {
            leftMargin = maxLabelWidth + 20; // Add padding
        }
        else
        {
            // Fallback: estimate based on typical numeric values
            leftMargin = 6 * (DEFAULT_AXIS_TEXT_SIZE * 6 / 10) + 20; // "-1000.0" + padding
        }

        leftMargin = smax(leftMargin, 60); // Ensure minimum margin
        leftMargin = smin(leftMargin, 120); // Cap maximum margin
    }

    // X axis (bottom) - estimate margin based on label height
    SAxis* pAxisX = pData->GetAxisX();
    if (pAxisX && pAxisX->GetValueCount() > 0)
    {
        SAxisValue *pValue = pAxisY->GetValue(0);
        if (pValue)
        {
            SStringT labelText = pValue->GetLabel();
            if (!labelText.IsEmpty())
            {
                CSize szText;
                pRT->MeasureText(labelText, labelText.GetLength(), &szText);
                bottomMargin = szText.cy + 20; // Add padding
            }
        }else{
            // Standard text height plus padding
            bottomMargin = DEFAULT_AXIS_TEXT_SIZE + 20;
        }
        bottomMargin = smax(bottomMargin, 40); // Ensure minimum margin
    }

    // For combo charts or charts that might need secondary Y axis,
    // use a more generous right margin
    if (pAxisY && pAxisY->GetValueCount() > 0)
    {
        // Use a reasonable right margin that can accommodate secondary axis labels
        rightMargin = smax(rightMargin, leftMargin / 2);
    }

    m_axesMarginLeft = leftMargin;
    m_axesMarginTop = topMargin;
    m_axesMarginRight = rightMargin;
    m_axesMarginBottom = bottomMargin;
}

SNSEND
