#include "stdafx.h"
#include "SPieChartView.h"
#include "../renderer/SPieChartRenderer.h"

SNSBEGIN

// Static constants
const float SPieChartView::DEFAULT_CIRCLE_FILL_RATIO = 0.0f;

SPieChartView::SPieChartView()
    : m_pData(NULL)
    , m_pPieChartRenderer(NULL)
    , m_pOnValueSelectListener(NULL)
    , m_circleFillRatio(DEFAULT_CIRCLE_FILL_RATIO)
    , m_bLabelsEnabled(TRUE)
    , m_bLabelLinesEnabled(FALSE)
{
}

SPieChartView::~SPieChartView()
{
    if (m_pData)
    {
        delete m_pData;
        m_pData = NULL;
    }
}

SChartData* SPieChartView::GetChartData()
{
    return m_pData;
}

void SPieChartView::SetPieChartData(SPieChartData* pData)
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
        m_pData->SetHasLabels(m_bLabelsEnabled);
        m_pData->SetHasLabelLines(m_bLabelLinesEnabled);
        m_pData->SetCenterCircleRatio(m_circleFillRatio);
    }
    
    // Trigger repaint
    Invalidate();
}

void SPieChartView::SetOnValueSelectListener(IPieChartOnValueSelectListener* pListener)
{
    m_pOnValueSelectListener = pListener;
}

void SPieChartView::SetCircleFillRatio(float ratio)
{
    m_circleFillRatio = ratio;
    
    if (m_pData)
    {
        m_pData->SetCenterCircleRatio(ratio);
        Invalidate();
    }
}

void SPieChartView::SetLabelsEnabled(BOOL bEnabled)
{
    m_bLabelsEnabled = bEnabled;
    
    if (m_pData)
    {
        m_pData->SetHasLabels(bEnabled);
        Invalidate();
    }
}

void SPieChartView::SetLabelLinesEnabled(BOOL bEnabled)
{
    m_bLabelLinesEnabled = bEnabled;
    
    if (m_pData)
    {
        m_pData->SetHasLabelLines(bEnabled);
        Invalidate();
    }
}

void SPieChartView::InitializeRenderers()
{
    m_pPieChartRenderer = new SPieChartRenderer(this);
    m_pChartRenderer = m_pPieChartRenderer;
}

void SPieChartView::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_bInteractive && m_pPieChartRenderer)
    {
        if (m_pPieChartRenderer->IsTouched(point.x,point.y))
        {
            CallTouchListener();
            Invalidate();
        }
    }
    
    __baseCls::OnLButtonDown(nFlags, point);
}

void SPieChartView::CallTouchListener()
{
    if (m_pOnValueSelectListener && m_pPieChartRenderer)
    {
        SSelectedValue selectedValue = m_pPieChartRenderer->GetSelectedValue();
        if (selectedValue.IsSet())
        {
            // Get the actual slice value from the data
            SSliceValue* pValue = NULL;
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

void SPieChartView::OnSliceValueSelected(int sliceIndex, SSliceValue* pValue)
{
    // Default implementation - can be overridden by derived classes
    // This method is called by the default listener
}

void SPieChartView::OnSliceValueDeselected()
{
    // Default implementation - can be overridden by derived classes
    // This method is called by the default listener
}

SNSEND
