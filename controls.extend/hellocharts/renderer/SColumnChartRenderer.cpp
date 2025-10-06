#include "stdafx.h"
#include "SColumnChartRenderer.h"
#include "../view/SColumnChartView.h"

SNSBEGIN

SColumnChartRenderer::SColumnChartRenderer(ISChart* pChart, SColumnChartView* pColumnChartView)
    : SChartRenderer(pChart)
    , m_pColumnChartView(pColumnChartView)
{
}

SColumnChartRenderer::~SColumnChartRenderer()
{
}

void SColumnChartRenderer::Draw(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;
        
    SColumnChartData* pData = GetColumnChartData();
    if (!pData)
        return;
        
    // Draw all columns
    for (size_t i = 0; i < pData->GetColumnCount(); ++i)
    {
        SColumn* pColumn = pData->GetColumn(i);
        if (pColumn)
        {
            DrawColumn(pRT, pColumn, (int)i);
        }
    }
}

void SColumnChartRenderer::DrawUnclipped(IRenderTarget* pRT)
{
    // Column charts don't typically need unclipped drawing
}

BOOL SColumnChartRenderer::IsTouched(float touchX, float touchY)
{
    if (!m_pChart)
        return FALSE;
        
    SColumnChartData* pData = GetColumnChartData();
    if (!pData)
        return FALSE;
        
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return FALSE;
        
    // Check each column for touch
    for (size_t columnIndex = 0; columnIndex < pData->GetColumnCount(); ++columnIndex)
    {
        SColumn* pColumn = pData->GetColumn(columnIndex);
        if (!pColumn)
            continue;
            
        for (size_t subcolumnIndex = 0; subcolumnIndex < pColumn->GetValueCount(); ++subcolumnIndex)
        {
            CRect columnRect = CalculateColumnRect((int)columnIndex, (int)subcolumnIndex);
            
            // Add window client offset
            CRect rcClient;
            m_pChart->GetRect(&rcClient);
            columnRect.OffsetRect(rcClient.left, rcClient.top);
            
            if (columnRect.PtInRect(CPoint((int)touchX, (int)touchY)))
            {
                SSelectedValue selectedValue((int)columnIndex, (int)subcolumnIndex);
                SelectValue(selectedValue);
                return TRUE;
            }
        }
    }
    
    return FALSE;
}

void SColumnChartRenderer::SelectValue(const SSelectedValue& selectedValue)
{
    m_selectedValue = selectedValue;
    
    // Notify chart view about selection
    if (m_pColumnChartView)
    {
        // TODO: Implement selection callback
    }
}

void SColumnChartRenderer::DeselectValue()
{
    m_selectedValue.Clear();
}

SColumnChartData* SColumnChartRenderer::GetColumnChartData() const
{
    if (m_pColumnChartView)
    {
        return m_pColumnChartView->GetColumnChartData();
    }
    else if (m_pChart)
    {
        // For adapter mode (combo chart), get data directly from chart
        return (SColumnChartData*)m_pChart->GetChartData();
    }
    return NULL;
}

void SColumnChartRenderer::DrawColumn(IRenderTarget* pRT, SColumn* pColumn, int columnIndex)
{
    if (!pRT || !pColumn || !m_pChart)
        return;
        
    // Draw all subcolumns in this column
    for (size_t subcolumnIndex = 0; subcolumnIndex < pColumn->GetValueCount(); ++subcolumnIndex)
    {
        SSubcolumnValue* pValue = pColumn->GetValue(subcolumnIndex);
        if (pValue)
        {
            CRect columnRect = CalculateColumnRect(columnIndex, (int)subcolumnIndex);
            DrawSubcolumn(pRT, pValue, columnIndex, (int)subcolumnIndex, columnRect);
        }
    }
    
    // Draw labels if enabled
    if (pColumn->HasLabels())
    {
        DrawColumnLabels(pRT, pColumn, columnIndex);
    }
}

void SColumnChartRenderer::DrawSubcolumn(IRenderTarget* pRT, SSubcolumnValue* pValue, 
                                        int columnIndex, int subcolumnIndex, const CRect& columnRect)
{
    if (!pRT || !pValue || !m_pChart)
        return;
        
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;
        
    SColumnChartData* pData = GetColumnChartData();
    if (!pData)
        return;
        
    SColumn* pColumn = pData->GetColumn(columnIndex);
    if (!pColumn)
        return;
        
    // Check if this subcolumn is selected
    BOOL bSelected = (m_selectedValue.IsSet() && 
                     m_selectedValue.GetFirstIndex() == columnIndex &&
                     m_selectedValue.GetSecondIndex() == subcolumnIndex);
    
    // Get colors
    COLORREF fillColor = pValue->HasCustomColor() ? pValue->GetColor() : pColumn->GetColor();
    COLORREF strokeColor = fillColor;
    int strokeWidth = pColumn->GetStrokeWidth();
    
    if (bSelected)
    {
        strokeColor = DEFAULT_SELECTED_COLUMN_COLOR;
        strokeWidth += DEFAULT_SELECTED_COLUMN_STROKE_WIDTH_INCREASE;
    }
    
    // Add window client offset for SOUI coordinate system
    CRect rcClient;
    m_pChart->GetRect(&rcClient);
    CRect drawRect = columnRect;
    drawRect.OffsetRect(rcClient.left, rcClient.top);
    
    // Create brush and pen
    SAutoRefPtr<IBrushS> brush, oldBrush;
    SAutoRefPtr<IPenS> pen, oldPen;
    
    pRT->CreateSolidColorBrush(fillColor, &brush);
    pRT->CreatePen(PS_SOLID, strokeWidth, strokeColor, &pen);
    
    pRT->SelectObject(brush, (IRenderObj**)&oldBrush);
    pRT->SelectObject(pen, (IRenderObj**)&oldPen);
    
    // Draw rectangle
    pRT->FillSolidRect(&drawRect, fillColor);
    pRT->DrawRectangle(&drawRect);
    
    // Restore objects
    pRT->SelectObject(oldBrush);
    pRT->SelectObject(oldPen);
}

void SColumnChartRenderer::DrawColumnLabels(IRenderTarget* pRT, SColumn* pColumn, int columnIndex)
{
    if (!pRT || !pColumn || !m_pChart)
        return;

    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;
    COLORREF oldTextColor = pRT->GetTextColor();
    // Draw value labels on top of each subcolumn
    for (size_t subcolumnIndex = 0; subcolumnIndex < pColumn->GetValueCount(); ++subcolumnIndex)
    {
        SSubcolumnValue* pValue = pColumn->GetValue(subcolumnIndex);
        if (pValue)
        {
            CRect columnRect = CalculateColumnRect(columnIndex, (int)subcolumnIndex);

            // Add window client offset
            CRect rcClient;
            m_pChart->GetRect(&rcClient);
            columnRect.OffsetRect(rcClient.left, rcClient.top);

            // Format value text
            SStringT labelText;
            labelText.Format(_T("%.1f"), pValue->GetY());

            // Calculate label position (above the column)
            int labelX = columnRect.left + columnRect.Width() / 2;
            int labelY = columnRect.top - 5; // 5 pixels above the column

            // Set text color (use column color or default)
            COLORREF textColor = pColumn->GetColor();
            // Make text color darker for better visibility
            BYTE r = GetRValue(textColor);
            BYTE g = GetGValue(textColor);
            BYTE b = GetBValue(textColor);
            BYTE a = GetAValue(textColor);
            textColor = RGBA((int)(r * 0.7f), (int)(g * 0.7f),(int)( b * 0.7f), a);

            // Draw text
            pRT->SetTextColor(textColor);
            CRect textRect(labelX - 30, labelY - 15, labelX + 30, labelY + 5);
            pRT->DrawText(labelText, labelText.GetLength(), &textRect,
                         DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }
    pRT->SetTextColor(oldTextColor);
}

CRect SColumnChartRenderer::CalculateColumnRect(int columnIndex, int subcolumnIndex)
{
    if (!m_pChart)
        return CRect(0, 0, 0, 0);
        
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return CRect(0, 0, 0, 0);
        
    SColumnChartData* pData = GetColumnChartData();
    if (!pData)
        return CRect(0, 0, 0, 0);
        
    SColumn* pColumn = pData->GetColumn(columnIndex);
    if (!pColumn)
        return CRect(0, 0, 0, 0);
        
    SSubcolumnValue* pValue = pColumn->GetValue(subcolumnIndex);
    if (!pValue)
        return CRect(0, 0, 0, 0);
        
    float columnWidth = CalculateColumnWidth();
    float columnSpacing = columnWidth * (1.0f - pData->GetFillRatio());
    float actualColumnWidth = columnWidth - columnSpacing;
    
    // Calculate X position
    // In Android version, column centers are at integer positions (0, 1, 2, ...)
    float centerX = (float)columnIndex;
    float pixelCenterX = pComputator->ComputeRawX(centerX);
    
    int left = (int)(pixelCenterX - actualColumnWidth / 2);
    int right = (int)(pixelCenterX + actualColumnWidth / 2);
    
    // Calculate Y position
    float baseY = 0.0f;  // Base line (bottom of column)
    float valueY = pValue->GetY(); // Top of column

    int bottom = (int)pComputator->ComputeRawY(baseY);
    int top = (int)pComputator->ComputeRawY(valueY);

    // ComputeRawY already handles coordinate transformation correctly
    // For positive values: top < bottom (top is higher on screen)
    // For negative values: top > bottom (top is lower on screen)
    // No need to swap coordinates
    
    return CRect(left, top, right, bottom);
}

float SColumnChartRenderer::CalculateColumnWidth()
{
    if (!m_pChart)
        return 1.0f;

    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return 1.0f;

    SColumnChartData* pData = GetColumnChartData();
    if (!pData || pData->GetColumnCount() == 0)
        return 1.0f;

    // Calculate width based on content rect (pixel space) and number of columns
    // This is the correct approach: divide pixel width by number of columns
    CRect contentRect = pComputator->GetContentRect();
    float contentWidth = (float)contentRect.Width();
    float columnCount = (float)pData->GetColumnCount();

    if (columnCount > 0 && contentWidth > 0)
    {
        return contentWidth / columnCount;
    }

    return 1.0f;
}

SNSEND
