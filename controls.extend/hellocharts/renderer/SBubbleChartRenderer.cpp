#include "stdafx.h"
#include "SBubbleChartRenderer.h"
#include "../computator/SChartComputator.h"
#include "../SChart.h"
#include "../model/SBubbleChartData.h"
#include "../view/SBubbleChartView.h"
#include <math.h>

SNSBEGIN

// Static constants
const float SBubbleChartRenderer::DEFAULT_BUBBLE_ALPHA = 0.8f;
const float SBubbleChartRenderer::SELECTED_BUBBLE_ALPHA = 1.0f;

SBubbleChartRenderer::SBubbleChartRenderer(ISChart* pChart)
    : SChartRenderer(pChart)
    , m_pBubbleChartView(NULL)
{
}

SBubbleChartRenderer::~SBubbleChartRenderer()
{
}

void SBubbleChartRenderer::Draw(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SBubbleChartData* pData = GetBubbleChartData();
    if (!pData || pData->GetValueCount() == 0)
        return;

    // Draw bubbles
    DrawBubbles(pRT);
}

void SBubbleChartRenderer::DrawUnclipped(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SBubbleChartData* pData = GetBubbleChartData();
    if (!pData || pData->GetValueCount() == 0)
        return;

    // Draw bubble labels outside the clipped area
    if (pData->HasBubbleLabels())
    {
        DrawBubbleLabels(pRT);
    }
}

BOOL SBubbleChartRenderer::IsTouched(float touchX, float touchY)
{
    if (!m_pChart)
        return FALSE;

    SBubbleChartData* pData = GetBubbleChartData();
    if (!pData || pData->GetValueCount() == 0)
        return FALSE;

    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return FALSE;

    CRect rcClient;
    m_pChart->GetRect(&rcClient);
    touchX -= rcClient.left;
    touchY -= rcClient.top;
    // Check each bubble from largest to smallest (for proper hit testing)
    for (int i = (int)pData->GetValueCount() - 1; i >= 0; --i)
    {
        SBubbleValue* pValue = pData->GetValue(i);
        if (pValue)
        {
            float centerX = pComputator->ComputeRawX(pValue->GetX());
            float centerY = pComputator->ComputeRawY(pValue->GetY());
            float radius = CalculateBubbleRadius(pValue, pData);

            if (IsPointInBubble(touchX, touchY, centerX, centerY, radius))
            {
                // Select this bubble
                m_selectedValue.Set(i, 0);
                return TRUE;
            }
        }
    }

    // No bubble selected
    m_selectedValue.Clear();
    return FALSE;
}

void SBubbleChartRenderer::SelectValue(const SSelectedValue& selectedValue)
{
    m_selectedValue = selectedValue;
}

void SBubbleChartRenderer::DeselectValue()
{
    m_selectedValue.Clear();
}

void SBubbleChartRenderer::DrawBubbles(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SBubbleChartData* pData = GetBubbleChartData();
    if (!pData || pData->GetValueCount() == 0)
        return;

    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;

    // Get window client offset
    CRect rcClient;
    m_pChart->GetRect(&rcClient);

    // Draw each bubble
    for (size_t i = 0; i < pData->GetValueCount(); ++i)
    {
        SBubbleValue* pValue = pData->GetValue(i);
        if (pValue)
        {
            float centerX = pComputator->ComputeRawX(pValue->GetX()) + rcClient.left;
            float centerY = pComputator->ComputeRawY(pValue->GetY()) + rcClient.top;
            float radius = CalculateBubbleRadius(pValue, pData);

            BOOL isSelected = m_selectedValue.IsSet() && m_selectedValue.GetFirstIndex() == (int)i;
            DrawBubble(pRT, pValue, radius, centerX, centerY, isSelected);
        }
    }
}

void SBubbleChartRenderer::DrawBubble(IRenderTarget* pRT, SBubbleValue* pValue, float radius,
                                     float centerX, float centerY, BOOL isSelected)
{
    if (!pRT || !pValue || radius <= 0.0f)
        return;

    // Calculate bubble color with alpha
    COLORREF bubbleColor = pValue->GetColor();
    float alpha = isSelected ? SELECTED_BUBBLE_ALPHA : DEFAULT_BUBBLE_ALPHA;
    
    BYTE r = GetRValue(bubbleColor);
    BYTE g = GetGValue(bubbleColor);
    BYTE b = GetBValue(bubbleColor);
    BYTE a = (BYTE)(GetAValue(bubbleColor)* alpha);
    
    bubbleColor = RGBA(r, g, b, a);

    // Create brush for bubble fill
    SAutoRefPtr<IBrushS> brush, oldBrush;
    pRT->CreateSolidColorBrush(bubbleColor, &brush);
    pRT->SelectObject(brush, (IRenderObj**)&oldBrush);

    // Create pen for bubble outline
    SAutoRefPtr<IPenS> pen, oldPen;
    COLORREF outlineColor = isSelected ? RGBA(255, 255, 255, 255) : RGBA(r/2, g/2, b/2, 255);
    int penWidth = isSelected ? 2 : 1;
    pRT->CreatePen(PS_SOLID, penWidth, outlineColor, &pen);
    pRT->SelectObject(pen, (IRenderObj**)&oldPen);

    // Draw bubble as filled circle
    CRect bubbleRect((int)(centerX - radius), (int)(centerY - radius),
                     (int)(centerX + radius), (int)(centerY + radius));
    pRT->FillEllipse(&bubbleRect);
    pRT->DrawEllipse(&bubbleRect);

    pRT->SelectObject(oldBrush);
    pRT->SelectObject(oldPen);
}

float SBubbleChartRenderer::CalculateBubbleRadius(SBubbleValue* pValue, SBubbleChartData* pData)
{
    if (!pValue || !pData)
        return 0.0f;

    float minZ, maxZ;
    FindMinMaxZ(pData, minZ, maxZ);

    if (maxZ <= minZ)
        return (float)pData->GetMinBubbleRadius();

    // Calculate radius based on Z value and scale type
    float normalizedZ = (pValue->GetZ() - minZ) / (maxZ - minZ);
    
    if (pData->GetBubbleScale() == 1) // Area scale
    {
        normalizedZ = sqrt(normalizedZ);
    }

    float minRadius = (float)pData->GetMinBubbleRadius();
    float maxRadius = (float)pData->GetMaxBubbleRadius();
    
    return minRadius + normalizedZ * (maxRadius - minRadius);
}

BOOL SBubbleChartRenderer::IsPointInBubble(float touchX, float touchY, float centerX, float centerY, float radius)
{
    float dx = touchX - centerX;
    float dy = touchY - centerY;
    float distance = sqrt(dx * dx + dy * dy);
    
    return distance <= radius;
}

void SBubbleChartRenderer::FindMinMaxZ(SBubbleChartData* pData, float& minZ, float& maxZ)
{
    if (!pData || pData->GetValueCount() == 0)
    {
        minZ = maxZ = 0.0f;
        return;
    }

    minZ = maxZ = pData->GetValue(0)->GetZ();
    
    for (size_t i = 1; i < pData->GetValueCount(); ++i)
    {
        SBubbleValue* pValue = pData->GetValue(i);
        if (pValue)
        {
            float z = pValue->GetZ();
            if (z < minZ) minZ = z;
            if (z > maxZ) maxZ = z;
        }
    }
}

SBubbleChartData* SBubbleChartRenderer::GetBubbleChartData() const
{
    return m_pBubbleChartView ? (SBubbleChartData*)m_pBubbleChartView->GetChartData() : NULL;
}

void SBubbleChartRenderer::DrawBubbleLabels(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SBubbleChartData* pData = GetBubbleChartData();
    if (!pData || pData->GetValueCount() == 0)
        return;

    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;

    // Get window client offset
    CRect rcClient;
    m_pChart->GetRect(&rcClient);

    // Set text color
    COLORREF textColor = RGBA(80, 80, 80, 255);
    COLORREF oldTextColor = pRT->SetTextColor(textColor);

    // Draw labels for each bubble
    for (size_t i = 0; i < pData->GetValueCount(); ++i)
    {
        SBubbleValue* pValue = pData->GetValue(i);
        if (pValue && !pValue->GetLabel().IsEmpty())
        {
            float centerX = pComputator->ComputeRawX(pValue->GetX()) + rcClient.left;
            float centerY = pComputator->ComputeRawY(pValue->GetY()) + rcClient.top;
            float radius = CalculateBubbleRadius(pValue, pData);

            // Position label above the bubble
            int labelX = (int)centerX;
            int labelY = (int)(centerY - radius - 15);

            CRect textRect(labelX - 40, labelY - 8, labelX + 40, labelY + 8);
            pRT->DrawText(pValue->GetLabel(), pValue->GetLabel().GetLength(),
                         &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }

    pRT->SetTextColor(oldTextColor);
}

SNSEND
