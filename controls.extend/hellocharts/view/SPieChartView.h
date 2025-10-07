#pragma once

#include "SAbstractChartView.h"
#include "../model/SPieChartData.h"

SNSBEGIN

// Forward declarations
class SPieChartRenderer;

/**
 * @class SPieChartView
 * @brief Pie chart view implementation
 */
class SPieChartView : public SAbstractChartView
{
    DEF_SOBJECT(SAbstractChartView, L"pieChart")

    friend class SPieChartValueSelectListener;
public:
    SPieChartView();
    virtual ~SPieChartView();

    // SAbstractChartView interface
    virtual SChartData* GetChartData() OVERRIDE;
    virtual void CallTouchListener() OVERRIDE;

    /**
     * @brief Get pie chart data
     * @return Pointer to pie chart data
     */
    SPieChartData* GetPieChartData() const { return m_pData; }

    /**
     * @brief Set pie chart data
     * @param pData Pointer to pie chart data
     */
    void SetPieChartData(SPieChartData* pData);

    /**
     * @brief Fire pie chart value select event
     * @param sliceIndex Slice index
     * @param pValue Slice value
     */
    void FireValueSelectEvent(int sliceIndex, SSliceValue* pValue);

    /**
     * @brief Get circle fill ratio (for donut charts)
     * @return Circle fill ratio
     */
    float GetCircleFillRatio() const { return m_circleFillRatio; }

    /**
     * @brief Set circle fill ratio (for donut charts)
     * @param ratio Circle fill ratio (0.0 = pie, >0.0 = donut)
     */
    void SetCircleFillRatio(float ratio);

    /**
     * @brief Check if labels are enabled
     * @return TRUE if labels are enabled
     */
    BOOL IsLabelsEnabled() const { return m_bLabelsEnabled; }

    /**
     * @brief Set labels enabled state
     * @param bEnabled Labels enabled state
     */
    void SetLabelsEnabled(BOOL bEnabled);

    /**
     * @brief Check if label lines are enabled
     * @return TRUE if label lines are enabled
     */
    BOOL IsLabelLinesEnabled() const { return m_bLabelLinesEnabled; }

    /**
     * @brief Set label lines enabled state
     * @param bEnabled Label lines enabled state
     */
    void SetLabelLinesEnabled(BOOL bEnabled);

protected:
    // SAbstractChartView interface
    virtual void InitializeRenderers() OVERRIDE;

    // Message handlers
    virtual void OnLButtonDown(UINT nFlags, CPoint point) OVERRIDE;

    /**
     * @brief Handle slice value selection
     * @param sliceIndex Selected slice index
     * @param pValue Selected slice value
     */
    void OnSliceValueSelected(int sliceIndex, SSliceValue* pValue);

    /**
     * @brief Handle slice value deselection
     */
    void OnSliceValueDeselected();

private:
    SPieChartData* m_pData;
    SPieChartRenderer* m_pPieChartRenderer;
    
    // Chart properties
    float m_circleFillRatio;
    BOOL m_bLabelsEnabled;
    BOOL m_bLabelLinesEnabled;

    // Default values
    static const float DEFAULT_CIRCLE_FILL_RATIO;
};



SNSEND
