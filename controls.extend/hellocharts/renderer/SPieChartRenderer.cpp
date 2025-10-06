#include "stdafx.h"
#include "SPieChartRenderer.h"
#include "../computator/SChartComputator.h"
#include "../SChart.h"
#include "../model/SPieChartData.h"
#include <math.h>

SNSBEGIN

// Static constants
const float SPieChartRenderer::DEFAULT_MAX_RADIUS = 0.0f;
const int SPieChartRenderer::DEFAULT_LABEL_TEXT_SIZE = 12;
const float SPieChartRenderer::LABEL_MARGIN_RATIO = 1.3f;

SPieChartRenderer::SPieChartRenderer(ISChart* pChart)
    : SChartRenderer(pChart)
    , m_maxRadius(DEFAULT_MAX_RADIUS)
    , m_labelTextSize(DEFAULT_LABEL_TEXT_SIZE)
{
}

SPieChartRenderer::~SPieChartRenderer()
{
}

void SPieChartRenderer::Draw(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SPieChartData* pData = dynamic_cast<SPieChartData*>(m_pChart->GetChartData());
    if (!pData || pData->GetValueCount() == 0)
        return;

    // Draw pie slices
    DrawSlices(pRT);

    // Draw center circle for donut charts
    if (pData->GetCenterCircleRatio() > 0.0f)
    {
        DrawCenterCircle(pRT);
    }
}

void SPieChartRenderer::DrawUnclipped(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SPieChartData* pData = dynamic_cast<SPieChartData*>(m_pChart->GetChartData());
    if (!pData || pData->GetValueCount() == 0)
        return;

    // Save current clipping region
    CRect oldClipRect;
    pRT->GetClipBox(&oldClipRect);

    // Expand clipping region to include the entire window area for external labels
    CRect rcClient;
    m_pChart->GetRect(&rcClient);
    CRect expandedClip(0, 0, rcClient.Width(), rcClient.Height());
    pRT->PushClipRect(&expandedClip, RGN_COPY);

    // Draw labels outside the clipped area
    if (pData->HasLabels())
    {
        DrawLabels(pRT);
    }

    // Restore original clipping region
    pRT->PopClip();
}

BOOL SPieChartRenderer::IsTouched(float touchX, float touchY)
{
    if (!m_pChart)
        return FALSE;

    SPieChartData* pData = dynamic_cast<SPieChartData*>(m_pChart->GetChartData());
    if (!pData || pData->GetValueCount() == 0)
        return FALSE;

    CPoint center = CalculatePieCenter();
    float radius = CalculatePieRadius();
    float innerRadius = radius * pData->GetCenterCircleRatio();

    // Calculate slice angles
    std::vector<float> startAngles, sweepAngles;
    CalculateSliceAngles(startAngles, sweepAngles);

    CPoint point((int)touchX, (int)touchY);
    // Check each slice
    for (size_t i = 0; i < pData->GetValueCount(); ++i)
    {
        if (IsPointInSlice(point, center, radius, startAngles[i], sweepAngles[i]))
        {
            // Check if point is not in inner circle (for donut charts)
            if (innerRadius > 0.0f)
            {
                float dx = (float)(point.x - center.x);
                float dy = (float)(point.y - center.y);
                float distance = sqrt(dx * dx + dy * dy);
                if (distance < innerRadius)
                    continue;
            }

            // Select this slice
            m_selectedValue.Set((int)i, 0);
            return TRUE;
        }
    }

    // No slice selected
    m_selectedValue.Clear();
    return FALSE;
}

void SPieChartRenderer::SelectValue(const SSelectedValue& selectedValue)
{
    m_selectedValue = selectedValue;
}

void SPieChartRenderer::DeselectValue()
{
    m_selectedValue.Clear();
}

void SPieChartRenderer::DrawSlices(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SPieChartData* pData = dynamic_cast<SPieChartData*>(m_pChart->GetChartData());
    if (!pData || pData->GetValueCount() == 0)
        return;

    CPoint center = CalculatePieCenter();
    float radius = CalculatePieRadius();
    float innerRadius = radius * pData->GetCenterCircleRatio();

    // Calculate slice angles
    std::vector<float> startAngles, sweepAngles;
    CalculateSliceAngles(startAngles, sweepAngles);

    // Draw each slice
    for (size_t i = 0; i < pData->GetValueCount(); ++i)
    {
        SSliceValue* pValue = pData->GetValue(i);
        if (pValue)
        {
            COLORREF color = pValue->GetColor();
            
            // Highlight selected slice
            if (m_selectedValue.IsSet() && m_selectedValue.GetFirstIndex() == (int)i)
            {
                // Make selected slice slightly brighter
                BYTE r = GetRValue(color);
                BYTE g = GetGValue(color);
                BYTE b = GetBValue(color);
                BYTE a = GetAValue(color);
                
                r = smin(255, r + 30);
                g = smin(255, g + 30);
                b = smin(255, b + 30);
                
                color = RGBA(r, g, b, a);
            }

            DrawSlice(pRT, center, radius, innerRadius, startAngles[i], sweepAngles[i], color);
        }
    }
}

CPoint SPieChartRenderer::CalculatePieCenter()
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

float SPieChartRenderer::CalculatePieRadius()
{
    if (!m_pChart)
        return 0.0f;

    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return 0.0f;

    CRect contentRect = pComputator->GetContentRect();

    float radius = smin(contentRect.Width(), contentRect.Height()) / 2.0f;

    // Apply maximum radius limit if set
    if (m_maxRadius > 0.0f && radius > m_maxRadius)
    {
        radius = m_maxRadius;
    }

    // Leave more margin for external labels and legend
    // Check if we have small slices that need external labels or legend
    SPieChartData* pData = dynamic_cast<SPieChartData*>(m_pChart->GetChartData());
    if (pData && pData->GetValueCount() > 0)
    {
        std::vector<float> startAngles, sweepAngles;
        CalculateSliceAngles(startAngles, sweepAngles);

        int smallSliceCount = 0;
        for (size_t i = 0; i < pData->GetValueCount(); ++i)
        {
            if (sweepAngles[i] < 8.0f) // Small slice threshold
                smallSliceCount++;
        }

        if (smallSliceCount > 0)
        {
            // Need more space for external labels or legend
            radius *= 0.65f; // Reduced from 0.8f to leave more space
        }
        else
        {
            radius *= 0.75f; // Standard margin
        }
    }
    else
    {
        radius *= 0.75f; // Default margin
    }

    return radius;
}

void SPieChartRenderer::CalculateSliceAngles(std::vector<float>& startAngles, std::vector<float>& sweepAngles)
{
    if (!m_pChart)
        return;

    SPieChartData* pData = dynamic_cast<SPieChartData*>(m_pChart->GetChartData());
    if (!pData || pData->GetValueCount() == 0)
        return;

    startAngles.clear();
    sweepAngles.clear();

    // Calculate total value
    float totalValue = 0.0f;
    for (size_t i = 0; i < pData->GetValueCount(); ++i)
    {
        SSliceValue* pValue = pData->GetValue(i);
        if (pValue)
        {
            totalValue += pValue->GetValue();
        }
    }

    if (totalValue <= 0.0f)
        return;

    // Calculate angles for each slice
    float currentAngle = -90.0f; // Start from top (12 o'clock)
    float sliceSpacing = pData->GetSliceSpacing();

    for (size_t i = 0; i < pData->GetValueCount(); ++i)
    {
        SSliceValue* pValue = pData->GetValue(i);
        if (pValue)
        {
            float sweepAngle = (pValue->GetValue() / totalValue) * 360.0f;
            
            // Apply slice spacing
            if (sliceSpacing > 0.0f && i > 0)
            {
                currentAngle += sliceSpacing;
                sweepAngle -= sliceSpacing;
            }

            startAngles.push_back(currentAngle);
            sweepAngles.push_back(sweepAngle);

            currentAngle += sweepAngle;
        }
    }
}

void SPieChartRenderer::DrawSlice(IRenderTarget* pRT, CPoint center, float radius, float innerRadius,
                                  float startAngle, float sweepAngle, COLORREF color)
{
    if (!pRT || sweepAngle <= 0.0f)
        return;

    // Create brush for filling
    SAutoRefPtr<IBrushS> brush, oldBrush;
    pRT->CreateSolidColorBrush(color, &brush);
    pRT->SelectObject(brush, (IRenderObj**)&oldBrush);

    // Create pen for outline
    SAutoRefPtr<IPenS> pen, oldPen;
    COLORREF outlineColor = RGBA(255, 255, 255, 255); // White outline
    pRT->CreatePen(PS_SOLID, 1, outlineColor, &pen);
    pRT->SelectObject(pen, (IRenderObj**)&oldPen);

    // Draw pie slice using FillArc
    CRect pieRect(center.x - (int)radius, center.y - (int)radius,
                  center.x + (int)radius, center.y + (int)radius);

    if (innerRadius > 0.0f)
    {
        // Draw donut slice - fill outer arc then fill inner arc with background color
        pRT->FillArc(&pieRect, startAngle, sweepAngle);

        // Draw inner circle to create donut effect
        CRect innerRect(center.x - (int)innerRadius, center.y - (int)innerRadius,
                       center.x + (int)innerRadius, center.y + (int)innerRadius);

        // Create brush with background color for inner circle
        SAutoRefPtr<IBrushS> innerBrush, oldInnerBrush;
        SPieChartData* pData = dynamic_cast<SPieChartData*>(m_pChart->GetChartData());
        COLORREF innerColor = pData ? pData->GetCenterCircleColor() : RGBA(255, 255, 255, 255);
        pRT->CreateSolidColorBrush(innerColor, &innerBrush);
        pRT->SelectObject(innerBrush, (IRenderObj**)&oldInnerBrush);

        pRT->FillArc(&innerRect, startAngle, sweepAngle);
        pRT->SelectObject(oldInnerBrush);
    }
    else
    {
        // Draw full pie slice
        pRT->FillArc(&pieRect, startAngle, sweepAngle);
    }

    // Draw outline
    pRT->DrawArc(&pieRect, startAngle, sweepAngle, FALSE);

    pRT->SelectObject(oldBrush);
    pRT->SelectObject(oldPen);
}

void SPieChartRenderer::DrawLabels(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SPieChartData* pData = dynamic_cast<SPieChartData*>(m_pChart->GetChartData());
    if (!pData || pData->GetValueCount() == 0)
        return;

    CPoint center = CalculatePieCenter();
    float radius = CalculatePieRadius();

    // Calculate slice angles and total value
    std::vector<float> startAngles, sweepAngles;
    CalculateSliceAngles(startAngles, sweepAngles);

    float totalValue = CalculateTotalValue(pData);

    // Separate slices into large and small categories
    std::vector<size_t> largeSlices, smallSlices;
    const float SMALL_SLICE_THRESHOLD = 8.0f; // Less than 8 degrees is considered small

    for (size_t i = 0; i < pData->GetValueCount(); ++i)
    {
        if (sweepAngles[i] >= SMALL_SLICE_THRESHOLD)
        {
            largeSlices.push_back(i);
        }
        else
        {
            smallSlices.push_back(i);
        }
    }

    // Draw labels for large slices (inside the pie)
    for (size_t i = 0; i < largeSlices.size(); ++i)
    {
        size_t idx = largeSlices[i];
        SSliceValue* pValue = pData->GetValue(idx);
        if (pValue && !pValue->GetLabel().IsEmpty())
        {
            float angle = startAngles[idx] + sweepAngles[idx] / 2.0f;
            DrawLargeSliceLabel(pRT, center, radius, angle, pValue, sweepAngles[idx]);
        }
    }

    // Draw labels for small slices (outside with leader lines)
    DrawSmallSliceLabels(pRT, center, radius, startAngles, sweepAngles, smallSlices, pData);

    // Draw legend for very small slices if needed
    if (smallSlices.size() > 3) // If too many small slices, show legend instead
    {
        DrawLegend(pRT, smallSlices, pData);
    }
}

void SPieChartRenderer::DrawSliceLabel(IRenderTarget* pRT, CPoint center, float radius,
                                       float angle, SSliceValue* pValue)
{
    if (!pRT || !pValue)
        return;

    // Calculate label position - place labels inside the pie chart at 70% radius
    float labelRadius = radius * 0.7f; // 将标签放在饼图内部70%的位置
    float angleRad = angle * 3.14159f / 180.0f;

    int labelX = (int)(center.x + cos(angleRad) * labelRadius);
    int labelY = (int)(center.y + sin(angleRad) * labelRadius);

    // Set text color - use white for better contrast on colored slices
    COLORREF textColor = RGBA(255, 255, 255, 255);
    COLORREF oldTextColor = pRT->SetTextColor(textColor);

    // Draw label text with smaller bounds to fit inside slices
    CRect textRect(labelX - 30, labelY - 8, labelX + 30, labelY + 8);
    pRT->DrawText(pValue->GetLabel(), pValue->GetLabel().GetLength(),
                 &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pRT->SetTextColor(oldTextColor);
}

void SPieChartRenderer::DrawLargeSliceLabel(IRenderTarget* pRT, CPoint center, float radius,
                                           float angle, SSliceValue* pValue, float sweepAngle)
{
    if (!pRT || !pValue)
        return;

    // For large slices, show label inside with percentage
    float labelRadius = radius * 0.65f; // Slightly closer to center for better fit
    float angleRad = angle * 3.14159f / 180.0f;

    int labelX = (int)(center.x + cos(angleRad) * labelRadius);
    int labelY = (int)(center.y + sin(angleRad) * labelRadius);

    // Set text color - use white for better contrast
    COLORREF textColor = RGBA(255, 255, 255, 255);
    COLORREF oldTextColor = pRT->SetTextColor(textColor);

    // Calculate percentage
    SPieChartData* pData = dynamic_cast<SPieChartData*>(m_pChart->GetChartData());
    float totalValue = CalculateTotalValue(pData);
    float percentage = (pValue->GetValue() / totalValue) * 100.0f;

    // Create label text with percentage
    SStringT labelText;
    if (sweepAngle >= 30.0f) // Large enough for both label and percentage
    {
        labelText.Format(_T("%s\n%.1f%%"), pValue->GetLabel().GetBuffer(), percentage);
        CRect textRect(labelX - 35, labelY - 12, labelX + 35, labelY + 12);
        pRT->DrawText(labelText, labelText.GetLength(),
                     &textRect, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
    }
    else // Medium size, show only percentage
    {
        labelText.Format(_T("%.1f%%"), percentage);
        CRect textRect(labelX - 25, labelY - 8, labelX + 25, labelY + 8);
        pRT->DrawText(labelText, labelText.GetLength(),
                     &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    pRT->SetTextColor(oldTextColor);
}

void SPieChartRenderer::DrawSmallSliceLabels(IRenderTarget* pRT, CPoint center, float radius,
                                            const std::vector<float>& startAngles,
                                            const std::vector<float>& sweepAngles,
                                            const std::vector<size_t>& smallSlices,
                                            SPieChartData* pData)
{
    if (!pRT || !pData || smallSlices.empty())
        return;

    // Don't draw external labels if there are too many small slices (will be shown in legend)
    if (smallSlices.size() > 3)
        return;

    // Get control bounds to ensure labels stay within bounds
    CRect rcClient;
    m_pChart->GetRect(&rcClient);

    float totalValue = CalculateTotalValue(pData);
    // Reduce outer radius to keep labels within control bounds
    float outerRadius = radius * 1.25f; // Reduced from 1.4f to 1.25f

    // Set text color for external labels
    COLORREF textColor = RGBA(80, 80, 80, 255);
    COLORREF oldTextColor = pRT->SetTextColor(textColor);

    // Create pen for leader lines
    SAutoRefPtr<IPenS> pen, oldPen;
    COLORREF lineColor = RGBA(120, 120, 120, 255);
    pRT->CreatePen(PS_SOLID, 1, lineColor, &pen);
    pRT->SelectObject(pen, (IRenderObj**)&oldPen);

    for (size_t i = 0; i < smallSlices.size(); ++i)
    {
        size_t idx = smallSlices[i];
        SSliceValue* pValue = pData->GetValue(idx);
        if (!pValue || pValue->GetLabel().IsEmpty())
            continue;

        float angle = startAngles[idx] + sweepAngles[idx] / 2.0f;
        float angleRad = angle * 3.14159f / 180.0f;

        // Calculate positions
        float innerRadius = radius * 1.05f; // Start of leader line
        int innerX = (int)(center.x + cos(angleRad) * innerRadius);
        int innerY = (int)(center.y + sin(angleRad) * innerRadius);

        int outerX = (int)(center.x + cos(angleRad) * outerRadius);
        int outerY = (int)(center.y + sin(angleRad) * outerRadius);

        // Draw leader line
        POINT points[2];
        points[0].x = innerX;
        points[0].y = innerY;
        points[1].x = outerX;
        points[1].y = outerY;
        pRT->DrawLines(points, 2);

        // Calculate percentage and create label
        float percentage = (pValue->GetValue() / totalValue) * 100.0f;
        SStringT labelText;
        labelText.Format(_T("%s (%.1f%%)"), pValue->GetLabel().GetBuffer(), percentage);

        // Position text based on angle to avoid overlap and stay within bounds
        int textWidth = 70; // Reduced width to fit better
        int textHeight = 16;
        CRect textRect;

        // Calculate text position with boundary checking
        if (cos(angleRad) >= 0) // Right side
        {
            int textLeft = outerX + 5;
            int textRight = textLeft + textWidth;

            // Ensure text doesn't go beyond right edge
            if (textRight > rcClient.Width() - 10)
            {
                textRight = rcClient.Width() - 10;
                textLeft = textRight - textWidth;
            }

            textRect.SetRect(textLeft, outerY - textHeight/2,
                           textRight, outerY + textHeight/2);
        }
        else // Left side
        {
            int textRight = outerX - 5;
            int textLeft = textRight - textWidth;

            // Ensure text doesn't go beyond left edge
            if (textLeft < 10)
            {
                textLeft = 10;
                textRight = textLeft + textWidth;
            }

            textRect.SetRect(textLeft, outerY - textHeight/2,
                           textRight, outerY + textHeight/2);
        }

        // Ensure text doesn't go beyond top/bottom edges
        if (textRect.top < 10)
        {
            textRect.OffsetRect(0, 10 - textRect.top);
        }
        if (textRect.bottom > rcClient.Height() - 10)
        {
            textRect.OffsetRect(0, rcClient.Height() - 10 - textRect.bottom);
        }

        pRT->DrawText(labelText, labelText.GetLength(),
                     &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    pRT->SelectObject(oldPen);
    pRT->SetTextColor(oldTextColor);
}

float SPieChartRenderer::CalculateTotalValue(SPieChartData* pData)
{
    if (!pData)
        return 1.0f;

    float total = 0.0f;
    for (size_t i = 0; i < pData->GetValueCount(); ++i)
    {
        SSliceValue* pValue = pData->GetValue(i);
        if (pValue)
        {
            total += pValue->GetValue();
        }
    }

    return total > 0.0f ? total : 1.0f;
}

void SPieChartRenderer::DrawLegend(IRenderTarget* pRT, const std::vector<size_t>& smallSlices,
                                  SPieChartData* pData)
{
    if (!pRT || !pData || smallSlices.empty())
        return;

    // Get chart content rect to position legend
    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;

    CRect contentRect = pComputator->GetContentRect();
    CRect rcClient;
    m_pChart->GetRect(&rcClient);
    contentRect.OffsetRect(rcClient.left, rcClient.top);

    // Position legend at the right side, but ensure it fits within control bounds
    int legendWidth = 110;
    int legendX = smin(contentRect.right - 10, rcClient.Width() - legendWidth - 10);
    int legendY = contentRect.top + 20;

    // Ensure legend doesn't start too far right
    if (legendX < contentRect.left + (int)(CalculatePieRadius() * 2.2f))
    {
        legendX = contentRect.left + (int)(CalculatePieRadius() * 2.2f);
    }
    int itemHeight = 20;
    int colorBoxSize = 12;

    // Set text color for legend
    COLORREF textColor = RGBA(80, 80, 80, 255);
    COLORREF oldTextColor = pRT->SetTextColor(textColor);

    float totalValue = CalculateTotalValue(pData);

    // Draw legend title
    SStringT titleText = _T("Legend:");
    CRect titleRect(legendX, legendY, legendX + 100, legendY + 16);
    pRT->DrawText(titleText, titleText.GetLength(),
                 &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    legendY += 25;

    // Draw legend items for small slices
    for (size_t i = 0; i < smallSlices.size(); ++i)
    {
        size_t idx = smallSlices[i];
        SSliceValue* pValue = pData->GetValue(idx);
        if (!pValue)
            continue;

        // Draw color box
        SAutoRefPtr<IBrushS> brush, oldBrush;
        pRT->CreateSolidColorBrush(pValue->GetColor(), &brush);
        pRT->SelectObject(brush, (IRenderObj**)&oldBrush);

        CRect colorRect(legendX, legendY + 2, legendX + colorBoxSize, legendY + 2 + colorBoxSize);
        pRT->FillSolidRect(&colorRect, pValue->GetColor());
        pRT->SelectObject(oldBrush);

        // Draw border around color box
        SAutoRefPtr<IPenS> pen, oldPen;
        COLORREF borderColor = RGBA(120, 120, 120, 255);
        pRT->CreatePen(PS_SOLID, 1, borderColor, &pen);
        pRT->SelectObject(pen, (IRenderObj**)&oldPen);

        // Draw rectangle border using lines
        POINT borderPoints[5];
        borderPoints[0].x = colorRect.left;
        borderPoints[0].y = colorRect.top;
        borderPoints[1].x = colorRect.right-1;
        borderPoints[1].y = colorRect.top;
        borderPoints[2].x = colorRect.right-1;
        borderPoints[2].y = colorRect.bottom-1;
        borderPoints[3].x = colorRect.left;
        borderPoints[3].y = colorRect.bottom-1;
        borderPoints[4].x = colorRect.left;
        borderPoints[4].y = colorRect.top;
        pRT->DrawLines(borderPoints, 5);
        pRT->SelectObject(oldPen);

        // Draw label and percentage
        float percentage = (pValue->GetValue() / totalValue) * 100.0f;
        SStringT labelText;
        labelText.Format(_T("%s (%.1f%%)"), pValue->GetLabel().GetBuffer(), percentage);

        // Ensure text rect stays within control bounds
        int textLeft = legendX + colorBoxSize + 5;
        int textRight = smin(textLeft + 95, rcClient.Width() - 5);
        int textBottom = smin(legendY + itemHeight, rcClient.Height() - 5);

        CRect textRect(textLeft, legendY, textRight, textBottom);
        pRT->DrawText(labelText, labelText.GetLength(),
                     &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        legendY += itemHeight;

        // Stop drawing if we're running out of vertical space
        if (legendY + itemHeight > rcClient.Height() - 10)
            break;
    }

    pRT->SetTextColor(oldTextColor);
}

void SPieChartRenderer::DrawCenterCircle(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SPieChartData* pData = dynamic_cast<SPieChartData*>(m_pChart->GetChartData());
    if (!pData)
        return;

    CPoint center = CalculatePieCenter();
    float radius = CalculatePieRadius();
    float innerRadius = radius * pData->GetCenterCircleRatio();

    if (innerRadius <= 0.0f)
        return;

    // Create brush for center circle
    SAutoRefPtr<IBrushS> brush, oldBrush;
    pRT->CreateSolidColorBrush(pData->GetCenterCircleColor(), &brush);
    pRT->SelectObject(brush, (IRenderObj**)&oldBrush);

    // Draw center circle
    CRect circleRect(center.x - (int)innerRadius, center.y - (int)innerRadius,
                     center.x + (int)innerRadius, center.y + (int)innerRadius);
    pRT->FillEllipse(&circleRect);

    pRT->SelectObject(oldBrush);
}

BOOL SPieChartRenderer::IsPointInSlice(CPoint point, CPoint center, float radius,
                                       float startAngle, float sweepAngle)
{
    // Check if point is within radius
    float dx = (float)(point.x - center.x);
    float dy = (float)(point.y - center.y);
    float distance = sqrt(dx * dx + dy * dy);

    if (distance > radius)
        return FALSE;

    // Check if point is within angle range
    float pointAngle = CalculateAngle(center, point);
    float normalizedStart = NormalizeAngle(startAngle);
    float normalizedEnd = NormalizeAngle(startAngle + sweepAngle);

    if (normalizedStart <= normalizedEnd)
    {
        return pointAngle >= normalizedStart && pointAngle <= normalizedEnd;
    }
    else
    {
        // Angle range crosses 0 degrees
        return pointAngle >= normalizedStart || pointAngle <= normalizedEnd;
    }
}

float SPieChartRenderer::CalculateAngle(CPoint center, CPoint point)
{
    float dx = (float)(point.x - center.x);
    float dy = (float)(point.y - center.y);

    float angle = atan2(dy, dx) * 180.0f / 3.14159f;
    return NormalizeAngle(angle);
}

float SPieChartRenderer::NormalizeAngle(float angle)
{
    while (angle < 0.0f)
        angle += 360.0f;
    while (angle >= 360.0f)
        angle -= 360.0f;
    return angle;
}

SNSEND
