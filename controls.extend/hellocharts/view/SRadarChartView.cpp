#include "stdafx.h"
#include "SRadarChartView.h"
#include "../renderer/SRadarChartRenderer.h"

SNSBEGIN

// Static constants
const int SRadarChartView::DEFAULT_WEB_RINGS = 5;
const float SRadarChartView::DEFAULT_MAX_VALUE = 100.0f;

SRadarChartView::SRadarChartView()
    : m_pData(NULL)
    , m_pRadarChartRenderer(NULL)
    , m_bRadarLabelsEnabled(TRUE)
    , m_bWebLinesEnabled(TRUE)
    , m_webRings(DEFAULT_WEB_RINGS)
    , m_maxValue(DEFAULT_MAX_VALUE)
{
    AddEvent(EVENTID(SRadarChartValueSelectEvent));
}

SRadarChartView::~SRadarChartView()
{
    if (m_pData)
    {
        delete m_pData;
        m_pData = NULL;
    }
}

SChartData* SRadarChartView::GetChartData()
{
    return m_pData;
}

void SRadarChartView::SetRadarChartData(SRadarChartData* pData)
{
    CancelDataAnimation();
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
        m_pData->SetHasRadarLabels(m_bRadarLabelsEnabled);
        m_pData->SetHasWebLines(m_bWebLinesEnabled);
        m_pData->SetWebRings(m_webRings);
        m_pData->SetMaxValue(m_maxValue);
        
        // For radar charts, we don't need to calculate viewport like other charts
        // The radar is always centered and scaled based on maxValue
        SViewport viewport(-1.0f, -1.0f, 1.0f, 1.0f);
        SetMaximumViewport(viewport);
        SetCurrentViewport(viewport);
    }
    
    // Trigger repaint
    Invalidate();
}



void SRadarChartView::SetRadarLabelsEnabled(BOOL bEnabled)
{
    m_bRadarLabelsEnabled = bEnabled;
    
    if (m_pData)
    {
        m_pData->SetHasRadarLabels(bEnabled);
        Invalidate();
    }
}

void SRadarChartView::SetWebLinesEnabled(BOOL bEnabled)
{
    m_bWebLinesEnabled = bEnabled;
    
    if (m_pData)
    {
        m_pData->SetHasWebLines(bEnabled);
        Invalidate();
    }
}

void SRadarChartView::SetWebRings(int rings)
{
    m_webRings = rings;
    
    if (m_pData)
    {
        m_pData->SetWebRings(rings);
        Invalidate();
    }
}

void SRadarChartView::SetMaxValue(float maxValue)
{
    m_maxValue = maxValue;
    
    if (m_pData)
    {
        m_pData->SetMaxValue(maxValue);
        Invalidate();
    }
}

void SRadarChartView::InitializeRenderers()
{
    m_pRadarChartRenderer = new SRadarChartRenderer(this);
    m_pRadarChartRenderer->SetRadarChartView(this);
    m_pChartRenderer = m_pRadarChartRenderer;
}

void SRadarChartView::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_bInteractive && m_pRadarChartRenderer)
    {
        if (m_pRadarChartRenderer->IsTouched((float)point.x, (float)point.y))
        {
            CallTouchListener();
            Invalidate();
        }
    }
    
    __baseCls::OnLButtonDown(nFlags, point);
}

void SRadarChartView::CallTouchListener()
{
    if (m_pRadarChartRenderer)
    {
        SSelectedValue selectedValue = m_pRadarChartRenderer->GetSelectedValue();
        if (selectedValue.IsSet())
        {
            // 有选中值，触发select事件
            SRadarValue* pValue = NULL;
            if (m_pData && selectedValue.GetFirstIndex() >= 0 &&
                selectedValue.GetFirstIndex() < (int)m_pData->GetValueCount())
            {
                pValue = m_pData->GetValue(selectedValue.GetFirstIndex());
            }

            // For radar chart, we use valueIndex and axisIndex (0 for now)
            FireValueSelectEvent(selectedValue.GetFirstIndex(), 0, pValue);
        }
        else
        {
            // 没有选中值，触发deselect事件（传递无效参数）
            FireValueSelectEvent(-1, -1, NULL);
        }
    }
}

void SRadarChartView::FireValueSelectEvent(int valueIndex, int axisIndex, SRadarValue* pValue)
{
    SRadarChartValueSelectEvent evt(this);
    evt.valueIndex = valueIndex;
    evt.axisIndex = axisIndex;
    evt.pValue = pValue;
    FireEvent(evt);
}

SNSEND
