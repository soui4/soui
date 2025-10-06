#include "stdafx.h"
#include "SLineChartRenderer.h"
#include "../view/SLineChartView.h"
#include <cmath>

SNSBEGIN

// Static constants
const float SLineChartRenderer::DEFAULT_TOUCH_TOLERANCE_RADIUS = 15.0f;
const int SLineChartRenderer::DEFAULT_SELECTED_POINT_RADIUS_INCREASE = 2;
const COLORREF SLineChartRenderer::DEFAULT_SELECTED_POINT_COLOR = RGBA(255, 100, 100, 255);

//////////////////////////////////////////////////////////////////////////
// SLineChartRenderer implementation

SLineChartRenderer::SLineChartRenderer(ISChart* pChart, SLineChartView* pLineChartView)
    : SChartRenderer(pChart)
    , m_pLineChartView(pLineChartView)
    , m_touchToleranceRadius(DEFAULT_TOUCH_TOLERANCE_RADIUS)
{
}

SLineChartRenderer::~SLineChartRenderer()
{
}

void SLineChartRenderer::Draw(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;
    DrawLines(pRT);
}

void SLineChartRenderer::DrawUnclipped(IRenderTarget* pRT)
{
    // Draw any unclipped elements here (like tooltips)
    // For now, this is empty
}

BOOL SLineChartRenderer::IsTouched(float touchX, float touchY)
{
    SLineChartData* pData = GetLineChartData();
    if (!pData || !m_pChart)
        return FALSE;
        
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return FALSE;
        
    // Check each line and point
    for (size_t lineIndex = 0; lineIndex < pData->GetLineCount(); ++lineIndex)
    {
        SLine* pLine = pData->GetLine(lineIndex);
        if (!pLine)
            continue;
            
        for (size_t pointIndex = 0; pointIndex < pLine->GetValueCount(); ++pointIndex)
        {
            SPointValue* pValue = pLine->GetValue(pointIndex);
            if (!pValue)
                continue;
                
            if (IsPointTouched(pValue, touchX, touchY, m_touchToleranceRadius))
            {
                m_selectedValue.Set((int)lineIndex, (int)pointIndex);
                return TRUE;
            }
        }
    }
    
    // No point was touched, clear selection
    m_selectedValue.Clear();
    return FALSE;
}

void SLineChartRenderer::DrawLines(IRenderTarget* pRT)
{
    SLineChartData* pData = GetLineChartData();
    if (!pData)
        return;
        
    for (size_t i = 0; i < pData->GetLineCount(); ++i)
    {
        SLine* pLine = pData->GetLine(i);
        if (pLine)
        {
            DrawLine(pRT, pLine, (int)i);
        }
    }
}

void SLineChartRenderer::DrawLine(IRenderTarget* pRT, SLine* pLine, int lineIndex)
{
    if (!pRT || !pLine || pLine->GetValueCount() < 2)
        return;
        
    // Draw line area (fill) first if enabled
    if (pLine->IsFilled())
    {
        DrawLineArea(pRT, pLine);
    }
    
    // Draw line path
    if (pLine->IsCubic())
    {
        DrawCubicLinePath(pRT, pLine);
    }
    else
    {
        DrawLinePath(pRT, pLine);
    }
    
    // Draw points if enabled
    if (pLine->HasPoints())
    {
        DrawLinePoints(pRT, pLine, lineIndex);
    }
    
    // Draw labels if enabled
    if (pLine->HasLabels())
    {
        DrawPointLabels(pRT, pLine);
    }
}

void SLineChartRenderer::DrawLinePath(IRenderTarget* pRT, SLine* pLine)
{
    if (!pRT || !pLine || !m_pChart)
        return;
        
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;
        
    // Create pen for line
    SAutoRefPtr<IPenS> pen,oldPen;
    pRT->CreatePen(PS_SOLID, pLine->GetColor(), pLine->GetStrokeWidth(), &pen);
    pRT->SelectObject(pen,(IRenderObj**)&oldPen);
    
    // Draw line segments
    BOOL bFirst = TRUE;
    for (size_t i = 0; i < pLine->GetValueCount(); ++i)
    {
        SPointValue* pValue = pLine->GetValue(i);
        if (!pValue)
            continue;
            
        float pixelX = pComputator->ComputeRawX(pValue->GetX());
        float pixelY = pComputator->ComputeRawY(pValue->GetY());

        // Add window client offset for SOUI coordinate system
        CRect rcClient;
        m_pChart->GetRect(&rcClient);
        POINT point = {(int)(pixelX + rcClient.left), (int)(pixelY + rcClient.top)};

        if (bFirst)
        {
            pRT->MoveToEx(point, NULL);
            bFirst = FALSE;
        }
        else
        {
            pRT->LineTo(point);
        }
    }
    
    pRT->SelectObject(oldPen);
}

void SLineChartRenderer::DrawCubicLinePath(IRenderTarget* pRT, SLine* pLine)
{
    // For now, just draw as regular line path
    // In a more sophisticated implementation, this would use Bezier curves
    DrawLinePath(pRT, pLine);
}

void SLineChartRenderer::DrawLineArea(IRenderTarget* pRT, SLine* pLine)
{
    if (!pRT || !pLine || !m_pChart)
        return;
        
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;
        
    SLineChartData* pData = GetLineChartData();
    if (!pData)
        return;
        
    // Create brush for area fill
    COLORREF fillColor = pLine->GetColor();
    BYTE alpha = (BYTE)(255 - pLine->GetAreaTransparency());

    // Extract RGBA components and apply area transparency
    BYTE r = GetRValue(fillColor);
    BYTE g = GetGValue(fillColor);
    BYTE b = GetBValue(fillColor);
    BYTE originalAlpha = (fillColor >> 24) & 0xFF;

    // If original alpha is 0, it means RGB was used, set to full opacity
    if (originalAlpha == 0) originalAlpha = 255;

    // Apply area transparency to the alpha
    alpha = (BYTE)((originalAlpha * alpha) / 255);
    fillColor = RGBA(r, g, b, alpha);
    
    SAutoRefPtr<IBrushS> brush,oldBrush;
    pRT->CreateSolidColorBrush(fillColor, &brush);
    pRT->SelectObject(brush,(IRenderObj**)&oldBrush);
    
    // Create polygon points for filled area
    std::vector<POINT> points;

    // Get window client offset for SOUI coordinate system
    CRect rcClient;
    m_pChart->GetRect(&rcClient);

    // Add line points
    for (size_t i = 0; i < pLine->GetValueCount(); ++i)
    {
        SPointValue* pValue = pLine->GetValue(i);
        if (!pValue)
            continue;

        float pixelX = pComputator->ComputeRawX(pValue->GetX());
        float pixelY = pComputator->ComputeRawY(pValue->GetY());

        POINT pt1 = { (int)(pixelX + rcClient.left), (int)(pixelY + rcClient.top) };
        points.push_back(pt1);
    }
    
    // Add base line points (in reverse order)
    float basePixelY = pComputator->ComputeRawY(pData->GetBaseValue());
    for (int i = (int)pLine->GetValueCount() - 1; i >= 0; --i)
    {
        SPointValue* pValue = pLine->GetValue(i);
        if (!pValue)
            continue;
            
        float pixelX = pComputator->ComputeRawX(pValue->GetX());
        POINT pt2 = { (int)(pixelX + rcClient.left), (int)(basePixelY + rcClient.top) };
        points.push_back(pt2);
    }
    
    // Draw filled polygon
    if (points.size() >= 3)
    {
        pRT->FillPolygon(&points[0], points.size());
    }
    
    pRT->SelectObject(oldBrush);
}

void SLineChartRenderer::DrawLinePoints(IRenderTarget* pRT, SLine* pLine, int lineIndex)
{
    if (!pRT || !pLine || !m_pChart)
        return;
        
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;
        
    for (size_t pointIndex = 0; pointIndex < pLine->GetValueCount(); ++pointIndex)
    {
        SPointValue* pValue = pLine->GetValue(pointIndex);
        if (!pValue)
            continue;
            
        BOOL bSelected = (m_selectedValue.IsSet() && 
                         m_selectedValue.GetFirstIndex() == lineIndex &&
                         m_selectedValue.GetSecondIndex() == (int)pointIndex);
        
        COLORREF pointColor = pValue->HasCustomColor() ? pValue->GetColor() : pLine->GetColor();
        int pointRadius = pLine->GetPointRadius();
        
        if (bSelected)
        {
            pointColor = DEFAULT_SELECTED_POINT_COLOR;
            pointRadius += DEFAULT_SELECTED_POINT_RADIUS_INCREASE;
        }
        
        DrawPoint(pRT, pValue, pointRadius, pointColor, bSelected);
    }
}

void SLineChartRenderer::DrawPoint(IRenderTarget* pRT, SPointValue* pValue, int pointRadius, 
                                  COLORREF color, BOOL bSelected)
{
    if (!pRT || !pValue || !m_pChart)
        return;
        
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;
        
    float pixelX = pComputator->ComputeRawX(pValue->GetX());
    float pixelY = pComputator->ComputeRawY(pValue->GetY());

    // Add window client offset for SOUI coordinate system
    CRect rcClient;
    m_pChart->GetRect(&rcClient);
    pixelX += rcClient.left;
    pixelY += rcClient.top;

    CRect pointRect((int)pixelX - pointRadius, (int)pixelY - pointRadius,
                   (int)pixelX + pointRadius, (int)pixelY + pointRadius);
    
    // Create brush and pen
    SAutoRefPtr<IBrushS> brush,oldBrush;
    pRT->CreateSolidColorBrush(color, &brush);
    pRT->SelectObject(brush,(IRenderObj**)&oldBrush);
    
    SAutoRefPtr<IPenS> pen,oldPen;
    pRT->CreatePen(PS_SOLID, color, 1, &pen);
    pRT->SelectObject(pen,(IRenderObj**)&oldPen);
    
    // Draw circle
    pRT->FillEllipse(&pointRect);
    pRT->DrawEllipse(&pointRect);
    
    pRT->SelectObject(oldBrush);
    pRT->SelectObject(oldPen);
}

void SLineChartRenderer::DrawPointLabels(IRenderTarget* pRT, SLine* pLine)
{
    if (!pRT || !pLine || !m_pChart)
        return;
        
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;
        
    for (size_t i = 0; i < pLine->GetValueCount(); ++i)
    {
        SPointValue* pValue = pLine->GetValue(i);
        if (pValue && !pValue->GetLabel().IsEmpty())
        {
            DrawPointLabel(pRT, pValue);
        }
    }
}

void SLineChartRenderer::DrawPointLabel(IRenderTarget* pRT, SPointValue* pValue)
{
    if (!pRT || !pValue || !m_pChart)
        return;
        
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;
        
    float pixelX = pComputator->ComputeRawX(pValue->GetX());
    float pixelY = pComputator->ComputeRawY(pValue->GetY());
    
    CRect textRect((int)pixelX - 50, (int)pixelY - 30, 
                  (int)pixelX + 50, (int)pixelY - 10);
    
    pRT->DrawText(pValue->GetLabel(), pValue->GetLabel().GetLength(), 
                 &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

BOOL SLineChartRenderer::IsPointTouched(SPointValue* pValue, float touchX, float touchY, float tolerance)
{
    if (!pValue || !m_pChart)
        return FALSE;
        
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return FALSE;
        
    float pixelX = pComputator->ComputeRawX(pValue->GetX());
    float pixelY = pComputator->ComputeRawY(pValue->GetY());
    
    float distance = CalculateDistance(pixelX, pixelY, touchX, touchY);
    return distance <= tolerance;
}

float SLineChartRenderer::CalculateDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

SLineChartData* SLineChartRenderer::GetLineChartData() const
{
    if (m_pLineChartView)
    {
        return m_pLineChartView->GetLineChartData();
    }
    else if (m_pChart)
    {
        // For adapter mode (combo chart), get data directly from chart
        return (SLineChartData*)m_pChart->GetChartData();
    }
    return NULL;
}

SNSEND
