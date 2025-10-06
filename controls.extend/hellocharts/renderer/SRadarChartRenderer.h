#pragma once

#include "SChartRenderer.h"
#include "../model/SRadarChartData.h"

SNSBEGIN

// Forward declarations
class SRadarChartView;

/**
 * @class SRadarChartRenderer
 * @brief Renderer for radar charts
 */
class SRadarChartRenderer : public SChartRenderer
{
public:
    SRadarChartRenderer(ISChart* pChart);
    virtual ~SRadarChartRenderer();

    // SChartRenderer interface
    virtual void Draw(IRenderTarget* pRT);
    virtual void DrawUnclipped(IRenderTarget* pRT);
    virtual BOOL IsTouched(float touchX, float touchY);

    /**
     * @brief Select radar value
     * @param selectedValue Selected value
     */
    virtual void SelectValue(const SSelectedValue& selectedValue);

    /**
     * @brief Deselect radar value
     */
    virtual void DeselectValue();

    /**
     * @brief Get radar chart view
     * @return Radar chart view
     */
    SRadarChartView* GetRadarChartView() const { return m_pRadarChartView; }

    /**
     * @brief Set radar chart view
     * @param pView Radar chart view
     */
    void SetRadarChartView(SRadarChartView* pView) { m_pRadarChartView = pView; }

private:
    /**
     * @brief Draw radar web (background grid)
     * @param pRT Render target
     */
    void DrawRadarWeb(IRenderTarget* pRT);

    /**
     * @brief Draw radar axes
     * @param pRT Render target
     */
    void DrawRadarAxes(IRenderTarget* pRT);

    /**
     * @brief Draw radar data series
     * @param pRT Render target
     */
    void DrawRadarSeries(IRenderTarget* pRT);

    /**
     * @brief Draw radar labels
     * @param pRT Render target
     */
    void DrawRadarLabels(IRenderTarget* pRT);

    /**
     * @brief Draw single radar series
     * @param pRT Render target
     * @param pValue Radar value
     * @param seriesIndex Series index
     * @param isSelected Is series selected
     */
    void DrawSingleSeries(IRenderTarget* pRT, SRadarValue* pValue, int seriesIndex, BOOL isSelected);

    /**
     * @brief Calculate radar center point
     * @return Center point
     */
    CPoint CalculateRadarCenter();

    /**
     * @brief Calculate radar radius
     * @return Radar radius
     */
    float CalculateRadarRadius();

    /**
     * @brief Calculate axis angle
     * @param axisIndex Axis index
     * @param totalAxes Total number of axes
     * @return Angle in radians
     */
    float CalculateAxisAngle(int axisIndex, int totalAxes);

    /**
     * @brief Calculate point on radar axis
     * @param center Radar center
     * @param angle Axis angle in radians
     * @param radius Distance from center
     * @return Point coordinates
     */
    CPoint CalculateAxisPoint(CPoint center, float angle, float radius);

    /**
     * @brief Get radar chart data
     * @return Radar chart data
     */
    SRadarChartData* GetRadarChartData() const;

    /**
     * @brief Check if point is near radar series
     * @param touchX Touch X coordinate
     * @param touchY Touch Y coordinate
     * @param pValue Radar value
     * @param threshold Distance threshold
     * @return TRUE if point is near series
     */
    BOOL IsPointNearSeries(float touchX, float touchY, SRadarValue* pValue, float threshold);

    /**
     * @brief Calculate distance from point to line segment
     * @param px Point X
     * @param py Point Y
     * @param x1 Line start X
     * @param y1 Line start Y
     * @param x2 Line end X
     * @param y2 Line end Y
     * @return Distance
     */
    float CalculateDistanceToLine(float px, float py, float x1, float y1, float x2, float y2);

private:
    SRadarChartView* m_pRadarChartView;

    // Default values
    static const float DEFAULT_TOUCH_THRESHOLD;
    static const float DEFAULT_SERIES_ALPHA;
    static const float SELECTED_SERIES_ALPHA;
};

/**
 * @interface IRadarChartOnValueSelectListener
 * @brief Interface for radar chart value selection events
 */
struct IRadarChartOnValueSelectListener
{
    /**
     * @brief Called when a radar series is selected
     * @param seriesIndex Selected series index
     * @param pValue Selected radar value
     */
    virtual void OnValueSelected(int seriesIndex, SRadarValue* pValue) = 0;

    /**
     * @brief Called when radar series is deselected
     */
    virtual void OnValueDeselected() = 0;
};

SNSEND
