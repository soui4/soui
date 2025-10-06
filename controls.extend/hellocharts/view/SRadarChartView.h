#pragma once

#include "SAbstractChartView.h"
#include "../model/SRadarChartData.h"
#include "../renderer/SRadarChartRenderer.h"

SNSBEGIN

// Forward declarations
class SRadarChartRenderer;

/**
 * @class SRadarChartView
 * @brief Radar chart view implementation
 */
class SRadarChartView : public SAbstractChartView
{
    DEF_SOBJECT(SAbstractChartView, L"radarChart")

public:
    SRadarChartView();
    virtual ~SRadarChartView();

    // SAbstractChartView interface
    virtual SChartData* GetChartData();
    virtual void CallTouchListener();

    /**
     * @brief Get radar chart data
     * @return Pointer to radar chart data
     */
    SRadarChartData* GetRadarChartData() const { return m_pData; }

    /**
     * @brief Set radar chart data
     * @param pData Pointer to radar chart data
     */
    void SetRadarChartData(SRadarChartData* pData);

    /**
     * @brief Set value select listener
     * @param pListener Pointer to value select listener
     */
    void SetOnValueSelectListener(IRadarChartOnValueSelectListener* pListener);

    /**
     * @brief Check if radar labels are enabled
     * @return TRUE if radar labels are enabled
     */
    BOOL IsRadarLabelsEnabled() const { return m_bRadarLabelsEnabled; }

    /**
     * @brief Set radar labels enabled state
     * @param bEnabled Radar labels enabled state
     */
    void SetRadarLabelsEnabled(BOOL bEnabled);

    /**
     * @brief Check if web lines are enabled
     * @return TRUE if web lines are enabled
     */
    BOOL IsWebLinesEnabled() const { return m_bWebLinesEnabled; }

    /**
     * @brief Set web lines enabled state
     * @param bEnabled Web lines enabled state
     */
    void SetWebLinesEnabled(BOOL bEnabled);

    /**
     * @brief Get number of web rings
     * @return Number of web rings
     */
    int GetWebRings() const { return m_webRings; }

    /**
     * @brief Set number of web rings
     * @param rings Number of web rings
     */
    void SetWebRings(int rings);

    /**
     * @brief Get maximum value for scaling
     * @return Maximum value
     */
    float GetMaxValue() const { return m_maxValue; }

    /**
     * @brief Set maximum value for scaling
     * @param maxValue Maximum value
     */
    void SetMaxValue(float maxValue);

protected:
    // SAbstractChartView interface
    virtual void InitializeRenderers();

    // Message handlers
    virtual void OnLButtonDown(UINT nFlags, CPoint point);

    /**
     * @brief Handle radar value selection
     * @param seriesIndex Selected series index
     * @param pValue Selected radar value
     */
    void OnRadarValueSelected(int seriesIndex, SRadarValue* pValue);

    /**
     * @brief Handle radar value deselection
     */
    void OnRadarValueDeselected();

private:
    SRadarChartData* m_pData;
    SRadarChartRenderer* m_pRadarChartRenderer;
    IRadarChartOnValueSelectListener* m_pOnValueSelectListener;
    
    // Chart properties
    BOOL m_bRadarLabelsEnabled;
    BOOL m_bWebLinesEnabled;
    int m_webRings;
    float m_maxValue;

    // Default values
    static const int DEFAULT_WEB_RINGS;
    static const float DEFAULT_MAX_VALUE;

    friend class SRadarChartValueSelectListener;
};

/**
 * @class SRadarChartValueSelectListener
 * @brief Default implementation of radar chart value select listener
 */
class SRadarChartValueSelectListener : public IRadarChartOnValueSelectListener
{
public:
    SRadarChartValueSelectListener(SRadarChartView* pView) : m_pView(pView) {}
    virtual ~SRadarChartValueSelectListener() {}

    virtual void OnValueSelected(int seriesIndex, SRadarValue* pValue)
    {
        if (m_pView)
        {
            m_pView->OnRadarValueSelected(seriesIndex, pValue);
        }
    }

    virtual void OnValueDeselected()
    {
        if (m_pView)
        {
            m_pView->OnRadarValueDeselected();
        }
    }

private:
    SRadarChartView* m_pView;
};

SNSEND
