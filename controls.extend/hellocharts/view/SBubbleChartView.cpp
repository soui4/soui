#include "stdafx.h"
#include "SBubbleChartView.h"
#include "../renderer/SBubbleChartRenderer.h"

SNSBEGIN

// Static constants
const int SBubbleChartView::DEFAULT_MIN_BUBBLE_RADIUS = 6;
const int SBubbleChartView::DEFAULT_MAX_BUBBLE_RADIUS = 50;
const int SBubbleChartView::DEFAULT_BUBBLE_SCALE = 0;

SBubbleChartView::SBubbleChartView()
    : m_pData(NULL)
    , m_pBubbleChartRenderer(NULL)
    , m_pOnValueSelectListener(NULL)
    , m_bBubbleLabelsEnabled(FALSE)
    , m_minBubbleRadius(DEFAULT_MIN_BUBBLE_RADIUS)
    , m_maxBubbleRadius(DEFAULT_MAX_BUBBLE_RADIUS)
    , m_bubbleScale(DEFAULT_BUBBLE_SCALE)
{
}

SBubbleChartView::~SBubbleChartView()
{
    if (m_pData)
    {
        delete m_pData;
        m_pData = NULL;
    }
}

SChartData* SBubbleChartView::GetChartData()
{
    return m_pData;
}

void SBubbleChartView::SetBubbleChartData(SBubbleChartData* pData)
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
        m_pData->SetHasBubbleLabels(m_bBubbleLabelsEnabled);
        m_pData->SetMinBubbleRadius(m_minBubbleRadius);
        m_pData->SetMaxBubbleRadius(m_maxBubbleRadius);
        m_pData->SetBubbleScale(m_bubbleScale);
        
        // Calculate viewport based on bubble data
        if (m_pData->GetValueCount() > 0)
        {
            float minX = m_pData->GetValue(0)->GetX();
            float maxX = minX;
            float minY = m_pData->GetValue(0)->GetY();
            float maxY = minY;
            
            for (size_t i = 1; i < m_pData->GetValueCount(); ++i)
            {
                SBubbleValue* pValue = m_pData->GetValue(i);
                if (pValue)
                {
                    float x = pValue->GetX();
                    float y = pValue->GetY();
                    
                    if (x < minX) minX = x;
                    if (x > maxX) maxX = x;
                    if (y < minY) minY = y;
                    if (y > maxY) maxY = y;
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
            SetCurrentViewport(viewport);
        }
    }
    
    // Trigger repaint
    Invalidate();
}

void SBubbleChartView::SetOnValueSelectListener(IBubbleChartOnValueSelectListener* pListener)
{
    m_pOnValueSelectListener = pListener;
}

void SBubbleChartView::SetBubbleLabelsEnabled(BOOL bEnabled)
{
    m_bBubbleLabelsEnabled = bEnabled;
    
    if (m_pData)
    {
        m_pData->SetHasBubbleLabels(bEnabled);
        Invalidate();
    }
}

void SBubbleChartView::SetMinBubbleRadius(int radius)
{
    m_minBubbleRadius = radius;
    
    if (m_pData)
    {
        m_pData->SetMinBubbleRadius(radius);
        Invalidate();
    }
}

void SBubbleChartView::SetMaxBubbleRadius(int radius)
{
    m_maxBubbleRadius = radius;
    
    if (m_pData)
    {
        m_pData->SetMaxBubbleRadius(radius);
        Invalidate();
    }
}

void SBubbleChartView::SetBubbleScale(int scale)
{
    m_bubbleScale = scale;
    
    if (m_pData)
    {
        m_pData->SetBubbleScale(scale);
        Invalidate();
    }
}

void SBubbleChartView::InitializeRenderers()
{
    m_pBubbleChartRenderer = new SBubbleChartRenderer(this);
    m_pBubbleChartRenderer->SetBubbleChartView(this);
    m_pChartRenderer = m_pBubbleChartRenderer;
}

void SBubbleChartView::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_bInteractive && m_pBubbleChartRenderer)
    {
        if (m_pBubbleChartRenderer->IsTouched(point.x, point.y))
        {
            CallTouchListener();
            Invalidate();
        }
    }
    
    __baseCls::OnLButtonDown(nFlags, point);
}

void SBubbleChartView::CallTouchListener()
{
    if (m_pOnValueSelectListener && m_pBubbleChartRenderer)
    {
        SSelectedValue selectedValue = m_pBubbleChartRenderer->GetSelectedValue();
        if (selectedValue.IsSet())
        {
            // Get the actual bubble value from the data
            SBubbleValue* pValue = NULL;
            if (m_pData && selectedValue.GetFirstIndex() >= 0 && 
                selectedValue.GetFirstIndex() < (int)m_pData->GetValueCount())
            {
                pValue = m_pData->GetValue(selectedValue.GetFirstIndex());
            }
            
            m_pOnValueSelectListener->OnValueSelected(
                selectedValue.GetFirstIndex(),
                pValue
            );
        }
        else
        {
            m_pOnValueSelectListener->OnValueDeselected();
        }
    }
}

void SBubbleChartView::OnBubbleValueSelected(int bubbleIndex, SBubbleValue* pValue)
{
    // Default implementation - can be overridden by derived classes
    // This method is called by the default listener
}

void SBubbleChartView::OnBubbleValueDeselected()
{
    // Default implementation - can be overridden by derived classes
    // This method is called by the default listener
}

SNSEND
