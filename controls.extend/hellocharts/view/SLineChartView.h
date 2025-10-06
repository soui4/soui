#pragma once

#include "SAbstractChartView.h"
#include "../model/SLineChartData.h"

SNSBEGIN

// Forward declarations
class SLineChartRenderer;
struct ILineChartOnValueSelectListener;

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
     * @brief Set value selection listener
     * @param pListener Pointer to value selection listener
     */
    void SetOnValueSelectListener(ILineChartOnValueSelectListener* pListener) { m_pOnValueSelectListener = pListener; }

    /**
     * @brief Get value selection listener
     * @return Pointer to value selection listener
     */
    void* GetOnValueSelectListener() const { return m_pOnValueSelectListener; }

protected:
    virtual void InitializeRenderers() OVERRIDE;

private:
    SLineChartData* m_pData;
    ILineChartOnValueSelectListener* m_pOnValueSelectListener;

public:
    SOUI_ATTRS_BEGIN()
        // Add line chart specific attributes here if needed
    SOUI_ATTRS_END()
};

/**
 * @brief Line chart value selection listener interface
 */
struct ILineChartOnValueSelectListener
{
    /**
     * @brief Called when a value is selected
     * @param lineIndex Index of the line
     * @param pointIndex Index of the point
     * @param pValue Pointer to the selected point value
     */
    virtual void OnValueSelected(int lineIndex, int pointIndex, SPointValue* pValue) = 0;

    /**
     * @brief Called when selection is cleared
     */
    virtual void OnValueDeselected() = 0;
};

/**
 * @class SDummyLineChartOnValueSelectListener
 * @brief Dummy implementation of line chart value selection listener
 */
class  SDummyLineChartOnValueSelectListener : public ILineChartOnValueSelectListener
{
public:
    virtual void OnValueSelected(int lineIndex, int pointIndex, SPointValue* pValue) OVERRIDE {}
    virtual void OnValueDeselected() OVERRIDE {}
};

SNSEND