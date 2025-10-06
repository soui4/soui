#include "stdafx.h"
#include "SComboChartRenderer.h"
#include "../computator/SChartComputator.h"
#include "../SChart.h"
#include "../model/SComboChartData.h"
#include "../view/SComboChartView.h"
#include "../model/SAxis.h"
SNSBEGIN

// SComboLineChartAdapter implementation
SChartData* SComboChartRenderer::SComboLineChartAdapter::GetChartData() { return m_pComboView ? m_pComboView->GetLineChartDataForRenderer() : NULL; }
SChartRenderer* SComboChartRenderer::SComboLineChartAdapter::GetChartRenderer() { return NULL; }
void SComboChartRenderer::SComboLineChartAdapter::SetChartRenderer(SChartRenderer* pRenderer) {}
SAxesRenderer* SComboChartRenderer::SComboLineChartAdapter::GetAxesRenderer() { return m_pComboView ? m_pComboView->GetAxesRenderer() : NULL; }
SChartComputator* SComboChartRenderer::SComboLineChartAdapter::GetChartComputator() { return m_pComboView ? m_pComboView->GetChartComputator() : NULL; }
SChartTouchHandler* SComboChartRenderer::SComboLineChartAdapter::GetTouchHandler() { return m_pComboView ? m_pComboView->GetTouchHandler() : NULL; }
void SComboChartRenderer::SComboLineChartAdapter::AnimationDataUpdate(float scale) { if (m_pComboView) m_pComboView->AnimationDataUpdate(scale); }
void SComboChartRenderer::SComboLineChartAdapter::AnimationDataFinished() { if (m_pComboView) m_pComboView->AnimationDataFinished(); }
void SComboChartRenderer::SComboLineChartAdapter::StartDataAnimation(DWORD duration) { if (m_pComboView) m_pComboView->StartDataAnimation(duration); }
void SComboChartRenderer::SComboLineChartAdapter::CancelDataAnimation() { if (m_pComboView) m_pComboView->CancelDataAnimation(); }
BOOL SComboChartRenderer::SComboLineChartAdapter::IsDataAnimationRunning() { return m_pComboView ? m_pComboView->IsDataAnimationRunning() : FALSE; }
void SComboChartRenderer::SComboLineChartAdapter::SetViewportChangeListener(void* pListener) { if (m_pComboView) m_pComboView->SetViewportChangeListener(pListener); }
SViewport SComboChartRenderer::SComboLineChartAdapter::GetCurrentViewport() { return m_pComboView ? m_pComboView->GetCurrentViewport() : SViewport(); }
void SComboChartRenderer::SComboLineChartAdapter::SetCurrentViewport(const SViewport& viewport) { if (m_pComboView) m_pComboView->SetCurrentViewport(viewport); }
SViewport SComboChartRenderer::SComboLineChartAdapter::GetMaximumViewport() { return m_pComboView ? m_pComboView->GetMaximumViewport() : SViewport(); }
void SComboChartRenderer::SComboLineChartAdapter::SetMaximumViewport(const SViewport& viewport) { if (m_pComboView) m_pComboView->SetMaximumViewport(viewport); }
BOOL SComboChartRenderer::SComboLineChartAdapter::IsInteractive() { return m_pComboView ? m_pComboView->IsInteractive() : FALSE; }
void SComboChartRenderer::SComboLineChartAdapter::SetInteractive(BOOL bInteractive) { if (m_pComboView) m_pComboView->SetInteractive(bInteractive); }
BOOL SComboChartRenderer::SComboLineChartAdapter::IsZoomEnabled() { return m_pComboView ? m_pComboView->IsZoomEnabled() : FALSE; }
void SComboChartRenderer::SComboLineChartAdapter::SetZoomEnabled(BOOL bEnabled) { if (m_pComboView) m_pComboView->SetZoomEnabled(bEnabled); }
BOOL SComboChartRenderer::SComboLineChartAdapter::IsScrollEnabled() { return m_pComboView ? m_pComboView->IsScrollEnabled() : FALSE; }
void SComboChartRenderer::SComboLineChartAdapter::SetScrollEnabled(BOOL bEnabled) { if (m_pComboView) m_pComboView->SetScrollEnabled(bEnabled); }
SSelectedValue SComboChartRenderer::SComboLineChartAdapter::GetSelectedValue() { return m_pComboView ? m_pComboView->GetSelectedValue() : SSelectedValue(); }
BOOL SComboChartRenderer::SComboLineChartAdapter::SelectValue(float x, float y) { return m_pComboView ? m_pComboView->SelectValue(x, y) : FALSE; }
void SComboChartRenderer::SComboLineChartAdapter::ClearSelection() { if (m_pComboView) m_pComboView->ClearSelection(); }
void SComboChartRenderer::SComboLineChartAdapter::OnChartDataChange() { if (m_pComboView) m_pComboView->OnChartDataChange(); }
void SComboChartRenderer::SComboLineChartAdapter::OnChartSizeChanged() { if (m_pComboView) m_pComboView->OnChartSizeChanged(); }
void SComboChartRenderer::SComboLineChartAdapter::CallTouchListener() { if (m_pComboView) m_pComboView->CallTouchListener(); }
void SComboChartRenderer::SComboLineChartAdapter::GetRect(LPRECT pRect) { if (m_pComboView) m_pComboView->GetRect(pRect); }

// SComboColumnChartAdapter implementation
SChartData* SComboChartRenderer::SComboColumnChartAdapter::GetChartData() { return m_pComboView ? m_pComboView->GetColumnChartDataForRenderer() : NULL; }
SChartRenderer* SComboChartRenderer::SComboColumnChartAdapter::GetChartRenderer() { return NULL; }
void SComboChartRenderer::SComboColumnChartAdapter::SetChartRenderer(SChartRenderer* pRenderer) {}
SAxesRenderer* SComboChartRenderer::SComboColumnChartAdapter::GetAxesRenderer() { return m_pComboView ? m_pComboView->GetAxesRenderer() : NULL; }
SChartComputator* SComboChartRenderer::SComboColumnChartAdapter::GetChartComputator() { return m_pComboView ? m_pComboView->GetChartComputator() : NULL; }
SChartTouchHandler* SComboChartRenderer::SComboColumnChartAdapter::GetTouchHandler() { return m_pComboView ? m_pComboView->GetTouchHandler() : NULL; }
void SComboChartRenderer::SComboColumnChartAdapter::AnimationDataUpdate(float scale) { if (m_pComboView) m_pComboView->AnimationDataUpdate(scale); }
void SComboChartRenderer::SComboColumnChartAdapter::AnimationDataFinished() { if (m_pComboView) m_pComboView->AnimationDataFinished(); }
void SComboChartRenderer::SComboColumnChartAdapter::StartDataAnimation(DWORD duration) { if (m_pComboView) m_pComboView->StartDataAnimation(duration); }
void SComboChartRenderer::SComboColumnChartAdapter::CancelDataAnimation() { if (m_pComboView) m_pComboView->CancelDataAnimation(); }
BOOL SComboChartRenderer::SComboColumnChartAdapter::IsDataAnimationRunning() { return m_pComboView ? m_pComboView->IsDataAnimationRunning() : FALSE; }
void SComboChartRenderer::SComboColumnChartAdapter::SetViewportChangeListener(void* pListener) { if (m_pComboView) m_pComboView->SetViewportChangeListener(pListener); }
SViewport SComboChartRenderer::SComboColumnChartAdapter::GetCurrentViewport() { return m_pComboView ? m_pComboView->GetCurrentViewport() : SViewport(); }
void SComboChartRenderer::SComboColumnChartAdapter::SetCurrentViewport(const SViewport& viewport) { if (m_pComboView) m_pComboView->SetCurrentViewport(viewport); }
SViewport SComboChartRenderer::SComboColumnChartAdapter::GetMaximumViewport() { return m_pComboView ? m_pComboView->GetMaximumViewport() : SViewport(); }
void SComboChartRenderer::SComboColumnChartAdapter::SetMaximumViewport(const SViewport& viewport) { if (m_pComboView) m_pComboView->SetMaximumViewport(viewport); }
BOOL SComboChartRenderer::SComboColumnChartAdapter::IsInteractive() { return m_pComboView ? m_pComboView->IsInteractive() : FALSE; }
void SComboChartRenderer::SComboColumnChartAdapter::SetInteractive(BOOL bInteractive) { if (m_pComboView) m_pComboView->SetInteractive(bInteractive); }
BOOL SComboChartRenderer::SComboColumnChartAdapter::IsZoomEnabled() { return m_pComboView ? m_pComboView->IsZoomEnabled() : FALSE; }
void SComboChartRenderer::SComboColumnChartAdapter::SetZoomEnabled(BOOL bEnabled) { if (m_pComboView) m_pComboView->SetZoomEnabled(bEnabled); }
BOOL SComboChartRenderer::SComboColumnChartAdapter::IsScrollEnabled() { return m_pComboView ? m_pComboView->IsScrollEnabled() : FALSE; }
void SComboChartRenderer::SComboColumnChartAdapter::SetScrollEnabled(BOOL bEnabled) { if (m_pComboView) m_pComboView->SetScrollEnabled(bEnabled); }
SSelectedValue SComboChartRenderer::SComboColumnChartAdapter::GetSelectedValue() { return m_pComboView ? m_pComboView->GetSelectedValue() : SSelectedValue(); }
BOOL SComboChartRenderer::SComboColumnChartAdapter::SelectValue(float x, float y) { return m_pComboView ? m_pComboView->SelectValue(x, y) : FALSE; }
void SComboChartRenderer::SComboColumnChartAdapter::ClearSelection() { if (m_pComboView) m_pComboView->ClearSelection(); }
void SComboChartRenderer::SComboColumnChartAdapter::OnChartDataChange() { if (m_pComboView) m_pComboView->OnChartDataChange(); }
void SComboChartRenderer::SComboColumnChartAdapter::OnChartSizeChanged() { if (m_pComboView) m_pComboView->OnChartSizeChanged(); }
void SComboChartRenderer::SComboColumnChartAdapter::CallTouchListener() { if (m_pComboView) m_pComboView->CallTouchListener(); }
void SComboChartRenderer::SComboColumnChartAdapter::GetRect(LPRECT pRect) { if (m_pComboView) m_pComboView->GetRect(pRect); }

SComboChartRenderer::SComboChartRenderer(ISChart* pChart)
    : SChartRenderer(pChart)
    , m_pComboChartView(NULL)
    , m_pLineChartRenderer(NULL)
    , m_pColumnChartRenderer(NULL)
    , m_pLineChartAdapter(NULL)
    , m_pColumnChartAdapter(NULL)
    , m_bIsLineChartSelection(FALSE)
    , m_bSubRenderersInitialized(FALSE)
{
}

SComboChartRenderer::~SComboChartRenderer()
{
    if (m_pLineChartRenderer)
    {
        delete m_pLineChartRenderer;
        m_pLineChartRenderer = NULL;
    }

    if (m_pColumnChartRenderer)
    {
        delete m_pColumnChartRenderer;
        m_pColumnChartRenderer = NULL;
    }

    if (m_pLineChartAdapter)
    {
        delete m_pLineChartAdapter;
        m_pLineChartAdapter = NULL;
    }

    if (m_pColumnChartAdapter)
    {
        delete m_pColumnChartAdapter;
        m_pColumnChartAdapter = NULL;
    }
}

void SComboChartRenderer::Draw(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SComboChartData* pData = GetComboChartData();
    if (!pData)
        return;

    // Initialize sub-renderers if needed
    if (!m_bSubRenderersInitialized)
    {
        InitializeSubRenderers();
    }

    // Setup sub-renderers with current data
    SetupLineChartRenderer();
    SetupColumnChartRenderer();

    // Draw based on combo type
    if (pData->GetComboType() == 0) // Columns behind lines
    {
        // Draw columns first (behind)
        if (pData->HasColumnData() && m_pColumnChartRenderer)
        {
            ApplyColumnChartAlpha(pData->GetColumnChartAlpha());
            m_pColumnChartRenderer->Draw(pRT);
        }
        
        // Draw lines second (in front)
        if (pData->HasLineData() && m_pLineChartRenderer)
        {
            ApplyLineChartAlpha(pData->GetLineChartAlpha());
            m_pLineChartRenderer->Draw(pRT);
        }
    }
    else // Lines behind columns
    {
        // Draw lines first (behind)
        if (pData->HasLineData() && m_pLineChartRenderer)
        {
            ApplyLineChartAlpha(pData->GetLineChartAlpha());
            m_pLineChartRenderer->Draw(pRT);
        }
        
        // Draw columns second (in front)
        if (pData->HasColumnData() && m_pColumnChartRenderer)
        {
            ApplyColumnChartAlpha(pData->GetColumnChartAlpha());
            m_pColumnChartRenderer->Draw(pRT);
        }
    }
}

void SComboChartRenderer::DrawUnclipped(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SComboChartData* pData = GetComboChartData();
    if (!pData)
        return;

    // Draw secondary Y axis if using separate axes
    if (pData->IsUsingSeparateYAxes() && pData->GetSecondaryAxisY())
    {
        DrawSecondaryYAxis(pRT);
    }

    // Draw unclipped content from sub-renderers
    if (pData->HasLineData() && m_pLineChartRenderer)
    {
        m_pLineChartRenderer->DrawUnclipped(pRT);
    }
    
    if (pData->HasColumnData() && m_pColumnChartRenderer)
    {
        m_pColumnChartRenderer->DrawUnclipped(pRT);
    }
}

BOOL SComboChartRenderer::IsTouched(float touchX, float touchY)
{
    if (!m_pChart)
        return FALSE;

    SComboChartData* pData = GetComboChartData();
    if (!pData)
        return FALSE;

    // Check line chart first (usually has higher priority for selection)
    if (pData->HasLineData() && m_pLineChartRenderer)
    {
        if (m_pLineChartRenderer->IsTouched(touchX, touchY))
        {
            m_selectedValue = m_pLineChartRenderer->GetSelectedValue();
            m_bIsLineChartSelection = TRUE;
            return TRUE;
        }
    }

    // Check column chart
    if (pData->HasColumnData() && m_pColumnChartRenderer)
    {
        if (m_pColumnChartRenderer->IsTouched(touchX, touchY))
        {
            m_selectedValue = m_pColumnChartRenderer->GetSelectedValue();
            m_bIsLineChartSelection = FALSE;
            return TRUE;
        }
    }

    // No selection
    m_selectedValue.Clear();
    return FALSE;
}

void SComboChartRenderer::SelectValue(const SSelectedValue& selectedValue)
{
    m_selectedValue = selectedValue;
    
    // Forward selection to appropriate sub-renderer
    if (m_bIsLineChartSelection && m_pLineChartRenderer)
    {
        m_pLineChartRenderer->SelectValue(selectedValue);
    }
    else if (!m_bIsLineChartSelection && m_pColumnChartRenderer)
    {
        m_pColumnChartRenderer->SelectValue(selectedValue);
    }
}

void SComboChartRenderer::DeselectValue()
{
    m_selectedValue.Clear();
    
    // Forward deselection to both sub-renderers
    if (m_pLineChartRenderer)
    {
        m_pLineChartRenderer->DeselectValue();
    }
    
    if (m_pColumnChartRenderer)
    {
        m_pColumnChartRenderer->DeselectValue();
    }
}

void SComboChartRenderer::InitializeSubRenderers()
{
    if (!m_pChart || !m_pComboChartView)
        return;

    // Create adapters
    if (!m_pLineChartAdapter)
    {
        m_pLineChartAdapter = new SComboLineChartAdapter(m_pComboChartView);
    }

    if (!m_pColumnChartAdapter)
    {
        m_pColumnChartAdapter = new SComboColumnChartAdapter(m_pComboChartView);
    }

    // Create line chart renderer with adapter
    if (!m_pLineChartRenderer)
    {
        m_pLineChartRenderer = new SLineChartRenderer(m_pLineChartAdapter, NULL);
    }

    // Create column chart renderer with adapter
    if (!m_pColumnChartRenderer)
    {
        m_pColumnChartRenderer = new SColumnChartRenderer(m_pColumnChartAdapter, NULL);
    }

    m_bSubRenderersInitialized = TRUE;
}

SComboChartData* SComboChartRenderer::GetComboChartData() const
{
    return m_pComboChartView ? (SComboChartData*)m_pComboChartView->GetChartData() : NULL;
}

void SComboChartRenderer::SetupLineChartRenderer()
{
    if (!m_pLineChartRenderer)
        return;

    SComboChartData* pData = GetComboChartData();
    if (!pData || !pData->HasLineData())
        return;

    // Set the line chart renderer to use the line chart view interface
    // The renderer will access data through the combo chart view's GetLineChartDataForRenderer method
}

void SComboChartRenderer::SetupColumnChartRenderer()
{
    if (!m_pColumnChartRenderer)
        return;

    SComboChartData* pData = GetComboChartData();
    if (!pData || !pData->HasColumnData())
        return;

    // Set the column chart renderer to use the column chart view interface
    // The renderer will access data through the combo chart view's GetColumnChartDataForRenderer method
}

void SComboChartRenderer::ApplyLineChartAlpha(float alpha)
{
    SComboChartData* pData = GetComboChartData();
    if (!pData || !pData->HasLineData())
        return;

    SLineChartData* pLineData = pData->GetLineChartData();
    if (!pLineData)
        return;

    // Apply alpha to all line series
    for (size_t i = 0; i < pLineData->GetLineCount(); ++i)
    {
        SLine* pLine = pLineData->GetLine(i);
        if (pLine)
        {
            for (size_t j = 0; j < pLine->GetValueCount(); ++j)
            {
                SPointValue* pPoint = pLine->GetValue(j);
                if (pPoint)
                {
                    COLORREF color = pPoint->GetColor();
                    BYTE r = GetRValue(color);
                    BYTE g = GetGValue(color);
                    BYTE b = GetBValue(color);
                    BYTE a = (BYTE)(255 * alpha);
                    pPoint->SetColor(RGBA(r, g, b, a));
                }
            }
        }
    }
}

void SComboChartRenderer::ApplyColumnChartAlpha(float alpha)
{
    SComboChartData* pData = GetComboChartData();
    if (!pData || !pData->HasColumnData())
        return;

    SColumnChartData* pColumnData = pData->GetColumnChartData();
    if (!pColumnData)
        return;

    // Apply alpha to all column series
    for (size_t i = 0; i < pColumnData->GetColumnCount(); ++i)
    {
        SColumn* pColumn = pColumnData->GetColumn(i);
        if (pColumn)
        {
            for (size_t j = 0; j < pColumn->GetValueCount(); ++j)
            {
                SSubcolumnValue* pValue = pColumn->GetValue(j);
                if (pValue)
                {
                    COLORREF color = pValue->GetColor();
                    BYTE r = GetRValue(color);
                    BYTE g = GetGValue(color);
                    BYTE b = GetBValue(color);
                    BYTE a = (BYTE)(255 * alpha);
                    pValue->SetColor(RGBA(r, g, b, a));
                }
            }
        }
    }
}

void SComboChartRenderer::DrawSecondaryYAxis(IRenderTarget* pRT)
{
    if (!pRT || !m_pChart)
        return;

    SComboChartData* pData = GetComboChartData();
    if (!pData || !pData->GetSecondaryAxisY())
        return;

    SChartComputator* pComputator = m_pChart->GetChartComputator();
    if (!pComputator)
        return;

    CRect rcClient;
    m_pChart->GetRect(&rcClient);
    CRect contentRect = pComputator->GetContentRect();
    contentRect.OffsetRect(rcClient.left, rcClient.top);

    SAxis* pSecondaryAxis = pData->GetSecondaryAxisY();

    // Set text color
    COLORREF textColor = pSecondaryAxis->GetTextColor();
    COLORREF oldTextColor = pRT->SetTextColor(textColor);

    // Create pen for axis line
    SAutoRefPtr<IPenS> pen, oldPen;
    COLORREF lineColor = pSecondaryAxis->GetLineColor();
    pRT->CreatePen(PS_SOLID, 1, lineColor, &pen);
    pRT->SelectObject(pen, (IRenderObj**)&oldPen);

    // Determine axis position (left or right)
    int axisX = pData->IsSecondaryAxisOnRight() ? contentRect.right : contentRect.left;

    // Draw axis line
    if (pSecondaryAxis->HasLines())
    {
        POINT axisLine[2] = {{axisX, contentRect.top}, {axisX, contentRect.bottom}};
        pRT->DrawLines(axisLine, 2);
    }

    // Draw axis values and labels
    for (size_t i = 0; i < pSecondaryAxis->GetValueCount(); ++i)
    {
        SAxisValue* pAxisValue = pSecondaryAxis->GetValue(i);
        if (pAxisValue)
        {
            // Calculate Y position for this axis value
            SViewport viewport = pComputator->GetCurrentViewport();
            float normalizedY = (pAxisValue->GetValue() - viewport.GetTop()) /
                               (viewport.GetBottom() - viewport.GetTop());

            int pixelY = contentRect.bottom - (int)(normalizedY * contentRect.Height()) + rcClient.top;

            // Draw grid line if enabled
            if (pSecondaryAxis->HasLines())
            {
                POINT gridLine[2] = {{contentRect.left, pixelY}, {contentRect.right, pixelY}};
                pRT->DrawLines(gridLine, 2);
            }

            // Draw axis label
            SStringT label = pAxisValue->GetLabel();
            if (!label.IsEmpty())
            {
                CRect textRect;
                if (pData->IsSecondaryAxisOnRight())
                {
                    // Right side - text to the right of axis
                    textRect.SetRect(axisX + 5, pixelY - 8, axisX + 60, pixelY + 8);
                    pRT->DrawText(label, label.GetLength(), &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                }
                else
                {
                    // Left side - text to the left of axis
                    textRect.SetRect(axisX - 60, pixelY - 8, axisX - 5, pixelY + 8);
                    pRT->DrawText(label, label.GetLength(), &textRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
                }
            }
        }
    }

    pRT->SelectObject(oldPen);
    pRT->SetTextColor(oldTextColor);
}

SNSEND