#pragma once

#include "SAbstractChartView.h"
#include "../model/SComboChartData.h"
#include "../renderer/SComboChartRenderer.h"

SNSBEGIN

// Forward declarations
class SComboChartRenderer;

/**
 * @class SComboChartView
 * @brief Combo chart view implementation (combination of line and column charts)
 */
class SComboChartView : public SAbstractChartView
{
    DEF_SOBJECT(SAbstractChartView, L"comboChart")

public:
    SComboChartView();
    virtual ~SComboChartView();

    // SAbstractChartView interface
    virtual SChartData* GetChartData();
    virtual void CallTouchListener();

    /**
     * @brief Get combo chart data
     * @return Pointer to combo chart data
     */
    SComboChartData* GetComboChartData() const { return m_pData; }

    /**
     * @brief Set combo chart data
     * @param pData Pointer to combo chart data
     */
    void SetComboChartData(SComboChartData* pData);

    /**
     * @brief Set value select listener
     * @param pListener Pointer to value select listener
     */
    void SetOnValueSelectListener(IComboChartOnValueSelectListener* pListener);

    /**
     * @brief Get combo chart type
     * @return Combo chart type (0=columns behind lines, 1=lines behind columns)
     */
    int GetComboType() const { return m_comboType; }

    /**
     * @brief Set combo chart type
     * @param type Combo chart type
     */
    void SetComboType(int type);

    /**
     * @brief Check if using separate Y axes
     * @return TRUE if using separate Y axes
     */
    BOOL IsUsingSeparateYAxes() const { return m_bUseSeparateYAxes; }

    /**
     * @brief Set separate Y axes usage
     * @param bUseSeparate TRUE to use separate Y axes
     */
    void SetUseSeparateYAxes(BOOL bUseSeparate);

    /**
     * @brief Check if secondary Y axis is on right side
     * @return TRUE if secondary Y axis is on right side
     */
    BOOL IsSecondaryAxisOnRight() const { return m_bSecondaryAxisOnRight; }

    /**
     * @brief Set secondary Y axis position
     * @param bOnRight TRUE to place secondary Y axis on right side
     */
    void SetSecondaryAxisOnRight(BOOL bOnRight);

    /**
     * @brief Get line chart alpha
     * @return Line chart alpha (0.0 to 1.0)
     */
    float GetLineChartAlpha() const { return m_lineChartAlpha; }

    /**
     * @brief Set line chart alpha
     * @param alpha Line chart alpha (0.0 to 1.0)
     */
    void SetLineChartAlpha(float alpha);

    /**
     * @brief Get column chart alpha
     * @return Column chart alpha (0.0 to 1.0)
     */
    float GetColumnChartAlpha() const { return m_columnChartAlpha; }

    /**
     * @brief Set column chart alpha
     * @param alpha Column chart alpha (0.0 to 1.0)
     */
    void SetColumnChartAlpha(float alpha);

    /**
     * @brief Get line chart data for sub-renderers
     * @return Line chart data or NULL
     */
    SLineChartData* GetLineChartDataForRenderer() const;

    /**
     * @brief Get column chart data for sub-renderers
     * @return Column chart data or NULL
     */
    SColumnChartData* GetColumnChartDataForRenderer() const;

protected:
    // SAbstractChartView interface
    virtual void InitializeRenderers();

    // Message handlers
    virtual void OnLButtonDown(UINT nFlags, CPoint point);

    /**
     * @brief Handle combo value selection
     * @param chartType Chart type (0=line, 1=column)
     * @param valueIndex Selected value index
     * @param subValueIndex Selected sub-value index
     */
    void OnComboValueSelected(int chartType, int valueIndex, int subValueIndex);

    /**
     * @brief Handle combo value deselection
     */
    void OnComboValueDeselected();

private:
    /**
     * @brief Calculate combined viewport from both chart data
     */
    void CalculateCombinedViewport();

private:
    SComboChartData* m_pData;
    SComboChartRenderer* m_pComboChartRenderer;
    IComboChartOnValueSelectListener* m_pOnValueSelectListener;
    
    // Chart properties
    int m_comboType;
    BOOL m_bUseSeparateYAxes;
    BOOL m_bSecondaryAxisOnRight;
    float m_lineChartAlpha;
    float m_columnChartAlpha;

    // Default values
    static const int DEFAULT_COMBO_TYPE;
    static const float DEFAULT_LINE_ALPHA;
    static const float DEFAULT_COLUMN_ALPHA;

    friend class SComboChartValueSelectListener;
};

/**
 * @class SComboChartValueSelectListener
 * @brief Default implementation of combo chart value select listener
 */
class SComboChartValueSelectListener : public IComboChartOnValueSelectListener
{
public:
    SComboChartValueSelectListener(SComboChartView* pView) : m_pView(pView) {}
    virtual ~SComboChartValueSelectListener() {}

    virtual void OnValueSelected(int chartType, int valueIndex, int subValueIndex)
    {
        if (m_pView)
        {
            m_pView->OnComboValueSelected(chartType, valueIndex, subValueIndex);
        }
    }

    virtual void OnValueDeselected()
    {
        if (m_pView)
        {
            m_pView->OnComboValueDeselected();
        }
    }

private:
    SComboChartView* m_pView;
};

SNSEND
