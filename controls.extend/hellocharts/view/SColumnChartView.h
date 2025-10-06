#pragma once
#include "SAbstractChartView.h"
#include "../model/SColumnChartData.h"

SNSBEGIN

// Forward declarations
class SColumnChartRenderer;
struct IColumnChartOnValueSelectListener;

/**
 * @interface IColumnChartOnValueSelectListener
 * @brief Interface for column chart value selection events
 */
struct IColumnChartOnValueSelectListener
{
    /**
     * @brief Called when a column value is selected
     * @param columnIndex Selected column index
     * @param subcolumnIndex Selected subcolumn index
     * @param pValue Selected value
     */
    virtual void OnValueSelected(int columnIndex, int subcolumnIndex, SSubcolumnValue* pValue) = 0;
    
    /**
     * @brief Called when selection is cleared
     */
    virtual void OnValueDeselected() = 0;
};

/**
 * @class SColumnChartView
 * @brief Column chart view implementation
 */
class SColumnChartView : public SAbstractChartView
{
    DEF_SOBJECT(SAbstractChartView, L"columnChart")

public:
    /**
     * @brief Constructor
     */
    SColumnChartView();
    
    /**
     * @brief Destructor
     */
    virtual ~SColumnChartView();

    /**
     * @brief Get column chart data
     * @return Pointer to column chart data
     */
    SColumnChartData* GetColumnChartData() const { return m_pData; }
    
    /**
     * @brief Set column chart data
     * @param pData Pointer to column chart data
     */
    void SetColumnChartData(SColumnChartData* pData);
    
    /**
     * @brief Get value selection listener
     * @return Pointer to listener
     */
    IColumnChartOnValueSelectListener* GetOnValueSelectListener() const { return m_pOnValueSelectListener; }
    
    /**
     * @brief Set value selection listener
     * @param pListener Pointer to listener
     */
    void SetOnValueSelectListener(IColumnChartOnValueSelectListener* pListener) { m_pOnValueSelectListener = pListener; }

    // SAbstractChartView interface
    virtual SChartData* GetChartData() OVERRIDE { return m_pData; }
    virtual void CallTouchListener() OVERRIDE;
    virtual void InitializeRenderers() OVERRIDE;
    virtual void WINAPI OnInitFinished(IXmlNode* xmlNode) OVERRIDE;

    // SOUI attributes
    SOUI_ATTRS_BEGIN()
        ATTR_BOOL(L"stacked", m_bStacked, FALSE)
        ATTR_FLOAT(L"fillRatio", m_fillRatio, 0.75f)
    SOUI_ATTRS_END()

protected:
    /**
     * @brief Handle value selection
     * @param columnIndex Selected column index
     * @param subcolumnIndex Selected subcolumn index
     */
    void OnValueSelected(int columnIndex, int subcolumnIndex);
    
    /**
     * @brief Handle value deselection
     */
    void OnValueDeselected();

private:
    SColumnChartData* m_pData;
    IColumnChartOnValueSelectListener* m_pOnValueSelectListener;
    
    // Attributes
    BOOL m_bStacked;
    float m_fillRatio;
};

SNSEND
