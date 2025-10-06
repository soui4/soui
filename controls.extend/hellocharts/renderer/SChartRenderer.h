#pragma once

#include <souistd.h>
#include <interface/SRender-i.h>
#include "../model/SChartData.h"

SNSBEGIN

// Forward declarations
class ISChart;

/**
 * @class SChartRenderer
 * @brief Base class for all chart renderers
 */
class SChartRenderer
{
public:
    SChartRenderer(ISChart* pChart);
    virtual ~SChartRenderer();

    /**
     * @brief Draw chart
     * @param pRT Render target
     */
    virtual void Draw(IRenderTarget* pRT) = 0;

    /**
     * @brief Draw unclipped elements (like tooltips)
     * @param pRT Render target
     */
    virtual void DrawUnclipped(IRenderTarget* pRT) = 0;

    /**
     * @brief Check if value is touched
     * @param touchX Touch X coordinate
     * @param touchY Touch Y coordinate
     * @return TRUE if value is touched
     */
    virtual BOOL IsTouched(float touchX, float touchY) = 0;

    /**
     * @brief Get selected value
     * @return Selected value
     */
    virtual SSelectedValue GetSelectedValue() const { return m_selectedValue; }

    /**
     * @brief Set selected value
     * @param selectedValue Selected value
     */
    virtual void SetSelectedValue(const SSelectedValue& selectedValue) { m_selectedValue = selectedValue; }

    /**
     * @brief Clear selection
     */
    virtual void ClearSelection() { m_selectedValue.Clear(); }

    /**
     * @brief Called when chart size changes
     */
    virtual void OnChartSizeChanged() {}

    /**
     * @brief Called when chart data changes
     */
    virtual void OnChartDataChanged() {}

    virtual void SelectValue(const SSelectedValue& selectedValue) {}
    virtual void DeselectValue() {}
    /**
     * @brief Get chart reference
     * @return Chart reference
     */
    ISChart* GetChart() const { return m_pChart; }

protected:
    ISChart* m_pChart;
    SSelectedValue m_selectedValue;
};

/**
 * @class SAxesRenderer
 * @brief Renderer for chart axes
 */
class  SAxesRenderer
{
public:
    SAxesRenderer(ISChart* pChart);
    virtual ~SAxesRenderer();

    /**
     * @brief Draw axes in background
     * @param pRT Render target
     */
    virtual void DrawInBackground(IRenderTarget* pRT);

    /**
     * @brief Draw axes in foreground
     * @param pRT Render target
     */
    virtual void DrawInForeground(IRenderTarget* pRT);

    /**
     * @brief Called when chart size changes
     */
    virtual void OnChartSizeChanged(IRenderTarget* pRT);

    /**
     * @brief Get chart reference
     * @return Chart reference
     */
    ISChart* GetChart() const { return m_pChart; }

    /**
     * @brief Set axes margin
     * @param left Left margin
     * @param top Top margin
     * @param right Right margin
     * @param bottom Bottom margin
     */
    void SetAxesMargin(int left, int top, int right, int bottom);

    /**
     * @brief Get axes margin
     * @param pLeft Pointer to left margin
     * @param pTop Pointer to top margin
     * @param pRight Pointer to right margin
     * @param pBottom Pointer to bottom margin
     */
    void GetAxesMargin(int* pLeft, int* pTop, int* pRight, int* pBottom) const;

protected:
    /**
     * @brief Draw X axis
     * @param pRT Render target
     */
    virtual void DrawXAxis(IRenderTarget* pRT);

    /**
     * @brief Draw Y axis
     * @param pRT Render target
     */
    virtual void DrawYAxis(IRenderTarget* pRT);

    /**
     * @brief Draw axis lines
     * @param pRT Render target
     */
    virtual void DrawAxisLines(IRenderTarget* pRT);

    /**
     * @brief Draw axis labels
     * @param pRT Render target
     */
    virtual void DrawAxisLabels(IRenderTarget* pRT);

    /**
     * @brief Calculate axes margins based on labels
     */
    virtual void CalculateAxesMargins(IRenderTarget* pRT);

protected:
    ISChart* m_pChart;
    int m_axesMarginLeft;
    int m_axesMarginTop;
    int m_axesMarginRight;
    int m_axesMarginBottom;
    
    // Default colors and sizes
    static const COLORREF DEFAULT_AXIS_LINE_COLOR = RGBA(200, 200, 200, 255);
    static const COLORREF DEFAULT_AXIS_TEXT_COLOR = RGBA(100, 100, 100, 255);
    static const int DEFAULT_AXIS_TEXT_SIZE = 12;
    static const int DEFAULT_AXIS_MARGIN = 60;  // Increased from 20 to 60 for better label spacing
};

SNSEND
