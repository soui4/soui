#pragma once
#include "SChartRenderer.h"
#include "../model/SColumnChartData.h"

SNSBEGIN

// Forward declarations
class SColumnChartView;

/**
 * @class SColumnChartRenderer
 * @brief Renderer for column charts
 */
class SColumnChartRenderer : public SChartRenderer
{
public:
    static const COLORREF DEFAULT_SELECTED_COLUMN_COLOR = RGBA(255, 255, 255, 255);
    static const int DEFAULT_SELECTED_COLUMN_STROKE_WIDTH_INCREASE = 2;

public:
    /**
     * @brief Constructor
     * @param pChart Pointer to chart view
     * @param pColumnChartView Pointer to column chart view
     */
    SColumnChartRenderer(ISChart* pChart, SColumnChartView* pColumnChartView);
    
    /**
     * @brief Destructor
     */
    virtual ~SColumnChartRenderer();

    // SChartRenderer interface
    virtual void Draw(IRenderTarget* pRT) OVERRIDE;
    virtual void DrawUnclipped(IRenderTarget* pRT) OVERRIDE;
    virtual BOOL IsTouched(float touchX, float touchY) OVERRIDE;
    virtual void SelectValue(const SSelectedValue& selectedValue) OVERRIDE;
    virtual void DeselectValue() OVERRIDE;

    /**
     * @brief Get column chart data
     * @return Pointer to column chart data
     */
    SColumnChartData* GetColumnChartData() const;

protected:
    /**
     * @brief Draw single column
     * @param pRT Render target
     * @param pColumn Column to draw
     * @param columnIndex Column index
     */
    void DrawColumn(IRenderTarget* pRT, SColumn* pColumn, int columnIndex);
    
    /**
     * @brief Draw single subcolumn
     * @param pRT Render target
     * @param pValue Subcolumn value
     * @param columnIndex Column index
     * @param subcolumnIndex Subcolumn index
     * @param columnRect Column rectangle
     */
    void DrawSubcolumn(IRenderTarget* pRT, SSubcolumnValue* pValue, 
                      int columnIndex, int subcolumnIndex, const CRect& columnRect);
    
    /**
     * @brief Draw column labels
     * @param pRT Render target
     * @param pColumn Column to draw labels for
     * @param columnIndex Column index
     */
    void DrawColumnLabels(IRenderTarget* pRT, SColumn* pColumn, int columnIndex);
    
    /**
     * @brief Calculate column rectangle
     * @param columnIndex Column index
     * @param subcolumnIndex Subcolumn index
     * @return Column rectangle
     */
    CRect CalculateColumnRect(int columnIndex, int subcolumnIndex);
    
    /**
     * @brief Calculate column width
     * @return Column width in pixels
     */
    float CalculateColumnWidth();

private:
    SColumnChartView* m_pColumnChartView;
};

SNSEND