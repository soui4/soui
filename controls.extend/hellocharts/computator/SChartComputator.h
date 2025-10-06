#ifndef __SCHARTCOMPUTATOR_H__
#define __SCHARTCOMPUTATOR_H__

#include <souistd.h>
#include "../model/SChartData.h"

SNSBEGIN

/**
 * @class SChartComputator
 * @brief Handles coordinate transformations and calculations for charts
 */
class SChartComputator
{
public:
    SChartComputator();
    virtual ~SChartComputator();

    /**
     * @brief Set content rectangle
     * @param width Chart width
     * @param height Chart height
     * @param paddingLeft Left padding
     * @param paddingTop Top padding
     * @param paddingRight Right padding
     * @param paddingBottom Bottom padding
     */
    void SetContentRect(int width, int height, int paddingLeft, int paddingTop, 
                       int paddingRight, int paddingBottom);

    /**
     * @brief Get content rectangle
     * @return Content rectangle
     */
    CRect GetContentRect() const { return m_contentRect; }

    /**
     * @brief Get content rectangle minus all margins
     * @return Content rectangle minus margins
     */
    CRect GetContentRectMinusAllMargins() const;

    /**
     * @brief Set chart area margins
     * @param left Left margin
     * @param top Top margin
     * @param right Right margin
     * @param bottom Bottom margin
     */
    void SetChartAreaMargins(int left, int top, int right, int bottom);

    /**
     * @brief Get chart area margins
     * @param pLeft Pointer to left margin
     * @param pTop Pointer to top margin
     * @param pRight Pointer to right margin
     * @param pBottom Pointer to bottom margin
     */
    void GetChartAreaMargins(int* pLeft, int* pTop, int* pRight, int* pBottom) const;

    /**
     * @brief Set maximum viewport
     * @param viewport Maximum viewport
     */
    void SetMaxViewport(const SViewport& viewport) { m_maximumViewport = viewport; }

    /**
     * @brief Set current viewport
     * @param viewport Current viewport
     */
    void SetCurrentViewport(const SViewport& viewport) { m_currentViewport = viewport; }

    /**
     * @brief Get current viewport
     * @return Current viewport
     */
    SViewport GetCurrentViewport() const { return m_currentViewport; }

    /**
     * @brief Set maximum viewport
     * @param viewport Maximum viewport
     */
    void SetMaximumViewport(const SViewport& viewport) { m_maximumViewport = viewport; }

    /**
     * @brief Get maximum viewport
     * @return Maximum viewport
     */
    SViewport GetMaximumViewport() const { return m_maximumViewport; }

    /**
     * @brief Set visible viewport
     * @param viewport Visible viewport
     */
    void SetVisibleViewport(const SViewport& viewport) { m_visibleViewport = viewport; }

    /**
     * @brief Get visible viewport
     * @return Visible viewport
     */
    SViewport GetVisibleViewport() const { return m_visibleViewport; }

    /**
     * @brief Convert raw X value to pixel X coordinate
     * @param rawX Raw X value
     * @return Pixel X coordinate
     */
    float ComputeRawX(float rawX) const;

    /**
     * @brief Convert raw Y value to pixel Y coordinate
     * @param rawY Raw Y value
     * @return Pixel Y coordinate
     */
    float ComputeRawY(float rawY) const;

    /**
     * @brief Convert pixel X coordinate to raw X value
     * @param pixelX Pixel X coordinate
     * @return Raw X value
     */
    float ComputeRawDistanceX(float pixelX) const;

    /**
     * @brief Convert pixel Y coordinate to raw Y value
     * @param pixelY Pixel Y coordinate
     * @return Raw Y value
     */
    float ComputeRawDistanceY(float pixelY) const;

    /**
     * @brief Check if point is in chart area
     * @param x X coordinate
     * @param y Y coordinate
     * @return TRUE if point is in chart area
     */
    BOOL IsInContentArea(float x, float y) const;

    /**
     * @brief Get chart width
     * @return Chart width in pixels
     */
    int GetChartWidth() const;

    /**
     * @brief Get chart height
     * @return Chart height in pixels
     */
    int GetChartHeight() const;

    /**
     * @brief Get minimum viewport zoom
     * @return Minimum zoom level
     */
    float GetMinimumViewportZoom() const { return m_minViewportZoom; }

    /**
     * @brief Set minimum viewport zoom
     * @param zoom Minimum zoom level
     */
    void SetMinimumViewportZoom(float zoom) { m_minViewportZoom = zoom; }

    /**
     * @brief Get maximum viewport zoom
     * @return Maximum zoom level
     */
    float GetMaximumViewportZoom() const { return m_maxViewportZoom; }

    /**
     * @brief Set maximum viewport zoom
     * @param zoom Maximum zoom level
     */
    void SetMaximumViewportZoom(float zoom) { m_maxViewportZoom = zoom; }

    /**
     * @brief Constrain viewport to valid bounds
     * @param viewport Viewport to constrain
     * @return Constrained viewport
     */
    SViewport ConstrainViewport(const SViewport& viewport) const;

private:
    CRect m_contentRect;
    int m_chartMarginLeft;
    int m_chartMarginTop;
    int m_chartMarginRight;
    int m_chartMarginBottom;
    
    SViewport m_currentViewport;
    SViewport m_maximumViewport;
    SViewport m_visibleViewport;
    
    float m_minViewportZoom;
    float m_maxViewportZoom;
};

SNSEND

#endif // __SCHARTCOMPUTATOR_H__