#pragma once

#include "SChartRenderer.h"
#include "../model/SLineChartData.h"

SNSBEGIN

// Forward declarations
class SLineChartView;

/**
 * @class SLineChartRenderer
 * @brief Renderer for line charts
 */
class  SLineChartRenderer : public SChartRenderer
{
public:
    SLineChartRenderer(ISChart* pChart, SLineChartView* pLineChartView);
    virtual ~SLineChartRenderer();

    // SChartRenderer interface implementation
    virtual void Draw(IRenderTarget* pRT) OVERRIDE;
    virtual void DrawUnclipped(IRenderTarget* pRT) OVERRIDE;
    virtual BOOL IsTouched(float touchX, float touchY) OVERRIDE;

    /**
     * @brief Get line chart view
     * @return Pointer to line chart view
     */
    SLineChartView* GetLineChartView() const { return m_pLineChartView; }

    /**
     * @brief Get touch tolerance radius
     * @return Touch tolerance radius in pixels
     */
    float GetTouchToleranceRadius() const { return m_touchToleranceRadius; }

    /**
     * @brief Set touch tolerance radius
     * @param radius Touch tolerance radius in pixels
     */
    void SetTouchToleranceRadius(float radius) { m_touchToleranceRadius = radius; }

protected:
    /**
     * @brief Draw all lines
     * @param pRT Render target
     */
    virtual void DrawLines(IRenderTarget* pRT);

    /**
     * @brief Draw single line
     * @param pRT Render target
     * @param pLine Pointer to line
     * @param lineIndex Line index
     */
    virtual void DrawLine(IRenderTarget* pRT, SLine* pLine, int lineIndex);

    /**
     * @brief Draw line path
     * @param pRT Render target
     * @param pLine Pointer to line
     */
    virtual void DrawLinePath(IRenderTarget* pRT, SLine* pLine);

    /**
     * @brief Draw cubic line path
     * @param pRT Render target
     * @param pLine Pointer to line
     */
    virtual void DrawCubicLinePath(IRenderTarget* pRT, SLine* pLine);

    /**
     * @brief Draw line area (fill)
     * @param pRT Render target
     * @param pLine Pointer to line
     */
    virtual void DrawLineArea(IRenderTarget* pRT, SLine* pLine);

    /**
     * @brief Draw line points
     * @param pRT Render target
     * @param pLine Pointer to line
     * @param lineIndex Line index
     */
    virtual void DrawLinePoints(IRenderTarget* pRT, SLine* pLine, int lineIndex);

    /**
     * @brief Draw single point
     * @param pRT Render target
     * @param pValue Pointer to point value
     * @param pointRadius Point radius
     * @param color Point color
     * @param bSelected Whether point is selected
     */
    virtual void DrawPoint(IRenderTarget* pRT, SPointValue* pValue, int pointRadius, 
                          COLORREF color, BOOL bSelected);

    /**
     * @brief Draw point labels
     * @param pRT Render target
     * @param pLine Pointer to line
     */
    virtual void DrawPointLabels(IRenderTarget* pRT, SLine* pLine);

    /**
     * @brief Draw single point label
     * @param pRT Render target
     * @param pValue Pointer to point value
     */
    virtual void DrawPointLabel(IRenderTarget* pRT, SPointValue* pValue);

    /**
     * @brief Check if point is touched
     * @param pValue Pointer to point value
     * @param touchX Touch X coordinate
     * @param touchY Touch Y coordinate
     * @param tolerance Touch tolerance
     * @return TRUE if point is touched
     */
    virtual BOOL IsPointTouched(SPointValue* pValue, float touchX, float touchY, float tolerance);

    /**
     * @brief Calculate distance between two points
     * @param x1 First point X
     * @param y1 First point Y
     * @param x2 Second point X
     * @param y2 Second point Y
     * @return Distance between points
     */
    virtual float CalculateDistance(float x1, float y1, float x2, float y2);

    /**
     * @brief Get line chart data
     * @return Pointer to line chart data
     */
    SLineChartData* GetLineChartData() const;

private:
    SLineChartView* m_pLineChartView;
    float m_touchToleranceRadius;
    
    // Default values
    static const float DEFAULT_TOUCH_TOLERANCE_RADIUS;
    static const int DEFAULT_SELECTED_POINT_RADIUS_INCREASE;
    static const COLORREF DEFAULT_SELECTED_POINT_COLOR;
};

SNSEND