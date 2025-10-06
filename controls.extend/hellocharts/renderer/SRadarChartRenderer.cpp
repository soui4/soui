#include "stdafx.h"
#include "SRadarChartRenderer.h"
#include "../computator/SChartComputator.h"
#include "../SChart.h"
#include "../model/SRadarChartData.h"
#include "../view/SRadarChartView.h"
#include <math.h>

SNSBEGIN

// Static constants
const float SRadarChartRenderer::DEFAULT_TOUCH_THRESHOLD = 20.0f;
const float SRadarChartRenderer::DEFAULT_SERIES_ALPHA = 0.7f;
const float SRadarChartRenderer::SELECTED_SERIES_ALPHA = 1.0f;

SRadarChartRenderer::SRadarChartRenderer(ISChart* pChart)
    : SChartRenderer(pChart)
    , m_pRadarChartView(NULL)
{
}

SRadarChartRenderer::~SRadarChartRenderer()
{
}

void SRadarChartRenderer::Draw(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SRadarChartData* pData = GetRadarChartData();
    if (!pData || pData->GetValueCount() == 0 || pData->GetAxesCount() == 0)
        return;

    // Draw radar web (background grid)
    if (pData->HasWebLines())
    {
        DrawRadarWeb(pRT);
    }

    // Draw radar axes
    DrawRadarAxes(pRT);

    // Draw radar data series
    DrawRadarSeries(pRT);
}

void SRadarChartRenderer::DrawUnclipped(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SRadarChartData* pData = GetRadarChartData();
    if (!pData || pData->GetValueCount() == 0 || pData->GetAxesCount() == 0)
        return;

    // Draw radar labels outside the clipped area
    if (pData->HasRadarLabels())
    {
        DrawRadarLabels(pRT);
    }
}

BOOL SRadarChartRenderer::IsTouched(float touchX, float touchY)
{
    if (!m_pChart)
        return FALSE;

    SRadarChartData* pData = GetRadarChartData();
    if (!pData || pData->GetValueCount() == 0)
        return FALSE;

    // Check each series for touch
    for (size_t i = 0; i < pData->GetValueCount(); ++i)
    {
        SRadarValue* pValue = pData->GetValue(i);
        if (pValue && IsPointNearSeries(touchX, touchY, pValue, DEFAULT_TOUCH_THRESHOLD))
        {
            // Select this series
            m_selectedValue.Set((int)i, 0);
            return TRUE;
        }
    }

    // No series selected
    m_selectedValue.Clear();
    return FALSE;
}

void SRadarChartRenderer::SelectValue(const SSelectedValue& selectedValue)
{
    m_selectedValue = selectedValue;
}

void SRadarChartRenderer::DeselectValue()
{
    m_selectedValue.Clear();
}

void SRadarChartRenderer::DrawRadarWeb(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SRadarChartData* pData = GetRadarChartData();
    if (!pData)
        return;

    CPoint center = CalculateRadarCenter();
    float radius = CalculateRadarRadius();
    int axesCount = (int)pData->GetAxesCount();
    int webRings = pData->GetWebRings();

    // Create pen for web lines
    SAutoRefPtr<IPenS> pen, oldPen;
    COLORREF webColor = RGBA(200, 200, 200, 255);
    pRT->CreatePen(PS_SOLID, 1, webColor, &pen);
    pRT->SelectObject(pen, (IRenderObj**)&oldPen);

    // Draw concentric polygons (web rings)
    for (int ring = 1; ring <= webRings; ++ring)
    {
        float ringRadius = radius * ring / webRings;
        
        if (axesCount >= 3)
        {
            // Draw polygon for this ring
            POINT* points = new POINT[axesCount + 1];
            
            for (int axis = 0; axis < axesCount; ++axis)
            {
                float angle = CalculateAxisAngle(axis, axesCount);
                CPoint axisPoint = CalculateAxisPoint(center, angle, ringRadius);
                points[axis].x = axisPoint.x;
                points[axis].y = axisPoint.y;
            }
            
            // Close the polygon
            points[axesCount] = points[0];
            
            pRT->DrawLines(points, axesCount + 1);
            delete[] points;
        }
    }

    pRT->SelectObject(oldPen);
}

void SRadarChartRenderer::DrawRadarAxes(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SRadarChartData* pData = GetRadarChartData();
    if (!pData)
        return;

    CPoint center = CalculateRadarCenter();
    float radius = CalculateRadarRadius();
    int axesCount = (int)pData->GetAxesCount();

    // Create pen for axis lines
    SAutoRefPtr<IPenS> pen, oldPen;
    COLORREF axisColor = RGBA(150, 150, 150, 255);
    pRT->CreatePen(PS_SOLID, 1, axisColor, &pen);
    pRT->SelectObject(pen, (IRenderObj**)&oldPen);

    // Draw axis lines from center to edge
    for (int axis = 0; axis < axesCount; ++axis)
    {
        float angle = CalculateAxisAngle(axis, axesCount);
        CPoint axisPoint = CalculateAxisPoint(center, angle, radius);
        
        POINT points[2] = {{center.x, center.y}, {axisPoint.x, axisPoint.y}};
        pRT->DrawLines(points, 2);
    }

    pRT->SelectObject(oldPen);
}

void SRadarChartRenderer::DrawRadarSeries(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SRadarChartData* pData = GetRadarChartData();
    if (!pData || pData->GetValueCount() == 0)
        return;

    // Draw each series
    for (size_t i = 0; i < pData->GetValueCount(); ++i)
    {
        SRadarValue* pValue = pData->GetValue(i);
        if (pValue)
        {
            BOOL isSelected = m_selectedValue.IsSet() && m_selectedValue.GetFirstIndex() == (int)i;
            DrawSingleSeries(pRT, pValue, (int)i, isSelected);
        }
    }
}

void SRadarChartRenderer::DrawSingleSeries(IRenderTarget* pRT, SRadarValue* pValue, int seriesIndex, BOOL isSelected)
{
    if (!pRT || !pValue || !m_pChart)
        return;

    SRadarChartData* pData = GetRadarChartData();
    if (!pData)
        return;

    CPoint center = CalculateRadarCenter();
    float radius = CalculateRadarRadius();
    int axesCount = (int)pData->GetAxesCount();
    float maxValue = pData->GetMaxValue();

    if (axesCount == 0 || maxValue <= 0.0f)
        return;

    // Calculate series alpha
    float alpha = isSelected ? SELECTED_SERIES_ALPHA : DEFAULT_SERIES_ALPHA;
    COLORREF seriesColor = pValue->GetColor();
    BYTE r = GetRValue(seriesColor);
    BYTE g = GetGValue(seriesColor);
    BYTE b = GetBValue(seriesColor);
	BYTE a = (BYTE)(GetAValue(seriesColor)* alpha);
    
    COLORREF fillColor = RGBA(r, g, b, (BYTE)(a * 0.3f)); // Semi-transparent fill
    COLORREF lineColor = RGBA(r, g, b, a);

    // Create points for the series polygon
    int pointCount = smin(axesCount, (int)pValue->GetAxisValueCount());
    if (pointCount < 3)
        return;

    POINT* points = new POINT[pointCount + 1];
    
    for (int axis = 0; axis < pointCount; ++axis)
    {
        float angle = CalculateAxisAngle(axis, axesCount);
        float value = pValue->GetAxisValue(axis);
        float normalizedValue = value / maxValue;
        if (normalizedValue > 1.0f) normalizedValue = 1.0f;
        if (normalizedValue < 0.0f) normalizedValue = 0.0f;
        
        float pointRadius = radius * normalizedValue;
        CPoint axisPoint = CalculateAxisPoint(center, angle, pointRadius);
        points[axis].x = axisPoint.x;
        points[axis].y = axisPoint.y;
    }
    
    // Close the polygon
    points[pointCount] = points[0];

    // Fill the polygon
    SAutoRefPtr<IBrushS> brush, oldBrush;
    pRT->CreateSolidColorBrush(fillColor, &brush);
    pRT->SelectObject(brush, (IRenderObj**)&oldBrush);
    
    pRT->FillPolygon(points, pointCount);
    pRT->SelectObject(oldBrush);

    // Draw the outline
    SAutoRefPtr<IPenS> pen, oldPen;
    int penWidth = isSelected ? 3 : 2;
    pRT->CreatePen(PS_SOLID, penWidth, lineColor, &pen);
    pRT->SelectObject(pen, (IRenderObj**)&oldPen);
    
    pRT->DrawLines(points, pointCount + 1);
    pRT->SelectObject(oldPen);

    // Draw data points
    for (int axis = 0; axis < pointCount; ++axis)
    {
        int pointSize = isSelected ? 6 : 4;
        CRect pointRect(points[axis].x - pointSize/2, points[axis].y - pointSize/2,
                       points[axis].x + pointSize/2, points[axis].y + pointSize/2);
        pRT->FillEllipse(&pointRect);
    }

    delete[] points;
}

CPoint SRadarChartRenderer::CalculateRadarCenter()
{
    if (!m_pChart)
        return CPoint(0, 0);

    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return CPoint(0, 0);

    CRect contentRect = pComputator->GetContentRect();
    CRect rcClient;
    m_pChart->GetRect(&rcClient);
    contentRect.OffsetRect(rcClient.left, rcClient.top);

    return CPoint(contentRect.left + contentRect.Width() / 2,
                  contentRect.top + contentRect.Height() / 2);
}

float SRadarChartRenderer::CalculateRadarRadius()
{
    if (!m_pChart)
        return 0.0f;

    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return 0.0f;

    CRect contentRect = pComputator->GetContentRect();
    
    float radius = smin(contentRect.Width(), contentRect.Height()) / 2.0f;
    
    // Leave margin for labels
    radius *= 0.7f;
    
    return radius;
}

float SRadarChartRenderer::CalculateAxisAngle(int axisIndex, int totalAxes)
{
    if (totalAxes <= 0)
        return 0.0f;
        
    // Start from top (270 degrees) and go clockwise
    float angleStep = 2.0f * 3.14159f / totalAxes;
    return -3.14159f / 2.0f + axisIndex * angleStep;
}

CPoint SRadarChartRenderer::CalculateAxisPoint(CPoint center, float angle, float radius)
{
    int x = (int)(center.x + cos(angle) * radius);
    int y = (int)(center.y + sin(angle) * radius);
    return CPoint(x, y);
}

SRadarChartData* SRadarChartRenderer::GetRadarChartData() const
{
    return m_pRadarChartView ? (SRadarChartData*)m_pRadarChartView->GetChartData() : NULL;
}

void SRadarChartRenderer::DrawRadarLabels(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SRadarChartData* pData = GetRadarChartData();
    if (!pData)
        return;

    CPoint center = CalculateRadarCenter();
    float radius = CalculateRadarRadius();
    int axesCount = (int)pData->GetAxesCount();

    // Set text color
    COLORREF textColor = RGBA(80, 80, 80, 255);
    COLORREF oldTextColor = pRT->SetTextColor(textColor);

    // Draw axis labels
    for (int axis = 0; axis < axesCount; ++axis)
    {
        if (axis < (int)pData->GetAxesLabels().size())
        {
            float angle = CalculateAxisAngle(axis, axesCount);
            float labelRadius = radius * 1.15f; // Position labels outside the radar
            CPoint labelPoint = CalculateAxisPoint(center, angle, labelRadius);

            SStringT label = pData->GetAxesLabels()[axis];

            // Adjust text position based on angle to avoid overlap
            int textWidth = 60;
            int textHeight = 16;
            CRect textRect;

            if (cos(angle) > 0.5f) // Right side
            {
                textRect.SetRect(labelPoint.x, labelPoint.y - textHeight/2,
                               labelPoint.x + textWidth, labelPoint.y + textHeight/2);
            }
            else if (cos(angle) < -0.5f) // Left side
            {
                textRect.SetRect(labelPoint.x - textWidth, labelPoint.y - textHeight/2,
                               labelPoint.x, labelPoint.y + textHeight/2);
            }
            else // Top or bottom
            {
                textRect.SetRect(labelPoint.x - textWidth/2, labelPoint.y - textHeight/2,
                               labelPoint.x + textWidth/2, labelPoint.y + textHeight/2);
            }

            pRT->DrawText(label, label.GetLength(),
                         &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }

    pRT->SetTextColor(oldTextColor);
}

BOOL SRadarChartRenderer::IsPointNearSeries(float touchX, float touchY, SRadarValue* pValue, float threshold)
{
    if (!pValue || !m_pChart)
        return FALSE;

    SRadarChartData* pData = GetRadarChartData();
    if (!pData)
        return FALSE;

    CPoint center = CalculateRadarCenter();
    float radius = CalculateRadarRadius();
    int axesCount = (int)pData->GetAxesCount();
    float maxValue = pData->GetMaxValue();

    if (axesCount == 0 || maxValue <= 0.0f)
        return FALSE;

    int pointCount = smin(axesCount, (int)pValue->GetAxisValueCount());
    if (pointCount < 2)
        return FALSE;

    // Check distance to each line segment of the series
    for (int axis = 0; axis < pointCount; ++axis)
    {
        int nextAxis = (axis + 1) % pointCount;

        // Calculate first point
        float angle1 = CalculateAxisAngle(axis, axesCount);
        float value1 = pValue->GetAxisValue(axis);
        float normalizedValue1 = value1 / maxValue;
        if (normalizedValue1 > 1.0f) normalizedValue1 = 1.0f;
        if (normalizedValue1 < 0.0f) normalizedValue1 = 0.0f;
        float pointRadius1 = radius * normalizedValue1;
        CPoint point1 = CalculateAxisPoint(center, angle1, pointRadius1);

        // Calculate second point
        float angle2 = CalculateAxisAngle(nextAxis, axesCount);
        float value2 = pValue->GetAxisValue(nextAxis);
        float normalizedValue2 = value2 / maxValue;
        if (normalizedValue2 > 1.0f) normalizedValue2 = 1.0f;
        if (normalizedValue2 < 0.0f) normalizedValue2 = 0.0f;
        float pointRadius2 = radius * normalizedValue2;
        CPoint point2 = CalculateAxisPoint(center, angle2, pointRadius2);

        // Check distance to this line segment
        float distance = CalculateDistanceToLine(touchX, touchY,
                                                (float)point1.x, (float)point1.y,
                                                (float)point2.x, (float)point2.y);

        if (distance <= threshold)
            return TRUE;
    }

    return FALSE;
}

float SRadarChartRenderer::CalculateDistanceToLine(float px, float py, float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;

    if (dx == 0.0f && dy == 0.0f)
    {
        // Point to point distance
        dx = px - x1;
        dy = py - y1;
        return sqrt(dx * dx + dy * dy);
    }

    // Calculate the parameter t for the closest point on the line segment
    float t = ((px - x1) * dx + (py - y1) * dy) / (dx * dx + dy * dy);

    // Clamp t to the line segment
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    // Calculate the closest point on the line segment
    float closestX = x1 + t * dx;
    float closestY = y1 + t * dy;

    // Calculate distance from touch point to closest point
    dx = px - closestX;
    dy = py - closestY;
    return sqrt(dx * dx + dy * dy);
}

SNSEND
