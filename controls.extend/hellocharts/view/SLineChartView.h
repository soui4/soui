#pragma once

#include "SAbstractChartView.h"
#include "../model/SLineChartData.h"

SNSBEGIN

// Forward declarations
class SLineChartRenderer;

/**
 * @class SLineChartView
 * @brief Line chart view implementation
 */
class  SLineChartView : public SAbstractChartView
{
    DEF_SOBJECT(SAbstractChartView, L"lineChart")

public:
    SLineChartView();
    virtual ~SLineChartView();

    /**
     * @brief Get line chart data
     * @return Pointer to line chart data
     */
    SLineChartData* GetLineChartData() const { return m_pData; }

    /**
     * @brief Set line chart data
     * @param pData Pointer to line chart data
     */
    void SetLineChartData(SLineChartData* pData);

    // ISChart interface implementation
    virtual SChartData* GetChartData() OVERRIDE { return m_pData; }
    virtual void CallTouchListener() OVERRIDE;

    /**
     * @brief Fire line chart value select event
     * @param lineIndex Line index
     * @param pointIndex Point index
     * @param pValue Point value
     */
    void FireValueSelectEvent(int lineIndex, int pointIndex, SPointValue* pValue);

protected:
    virtual void InitializeRenderers() OVERRIDE;

private:
    SLineChartData* m_pData;

public:
    SOUI_ATTRS_BEGIN()
        // Add line chart specific attributes here if needed
    SOUI_ATTRS_END()
};



SNSEND