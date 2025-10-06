#ifndef __SCHART_H__
#define __SCHART_H__

#include <core/SWnd.h>
#include <interface/SRender-i.h>

SNSBEGIN

// Forward declarations
class SChartData;
class SChartRenderer;
class SAxesRenderer;
class SChartComputator;
class SChartTouchHandler;
class SSelectedValue;
class SViewport;

/**
 * @class ISChart
 * @brief Interface for all chart types
 * @details Base interface that all chart implementations must implement
 */
struct ISChart
{
    /**
     * @brief Get chart data
     * @return Pointer to chart data
     */
    virtual SChartData* GetChartData() = 0;

    /**
     * @brief Get chart renderer
     * @return Pointer to chart renderer
     */
    virtual SChartRenderer* GetChartRenderer() = 0;

    /**
     * @brief Set chart renderer
     * @param pRenderer Pointer to chart renderer
     */
    virtual void SetChartRenderer(SChartRenderer* pRenderer) = 0;

    /**
     * @brief Get axes renderer
     * @return Pointer to axes renderer
     */
    virtual SAxesRenderer* GetAxesRenderer() = 0;

    /**
     * @brief Get chart computator
     * @return Pointer to chart computator
     */
    virtual SChartComputator* GetChartComputator() = 0;

    /**
     * @brief Get touch handler
     * @return Pointer to touch handler
     */
    virtual SChartTouchHandler* GetTouchHandler() = 0;

    /**
     * @brief Update chart data during animation
     * @param scale Animation scale factor
     */
    virtual void AnimationDataUpdate(float scale) = 0;

    /**
     * @brief Called when data animation finished
     */
    virtual void AnimationDataFinished() = 0;

    /**
     * @brief Start chart data animation
     * @param duration Animation duration in milliseconds
     */
    virtual void StartDataAnimation(DWORD duration) = 0;

    /**
     * @brief Cancel data animation
     */
    virtual void CancelDataAnimation() = 0;

    /**
     * @brief Check if data animation is running
     * @return TRUE if animation is running
     */
    virtual BOOL IsDataAnimationRunning() = 0;

    /**
     * @brief Set viewport change listener
     * @param pListener Pointer to viewport change listener
     */
    virtual void SetViewportChangeListener(void* pListener) = 0;

    /**
     * @brief Get current viewport
     * @return Current viewport
     */
    virtual SViewport GetCurrentViewport() = 0;

    /**
     * @brief Set current viewport
     * @param viewport New viewport
     */
    virtual void SetCurrentViewport(const SViewport& viewport) = 0;

    /**
     * @brief Get maximum viewport
     * @return Maximum viewport
     */
    virtual SViewport GetMaximumViewport() = 0;

    /**
     * @brief Set maximum viewport
     * @param viewport Maximum viewport
     */
    virtual void SetMaximumViewport(const SViewport& viewport) = 0;

    /**
     * @brief Check if chart is interactive
     * @return TRUE if interactive
     */
    virtual BOOL IsInteractive() = 0;

    /**
     * @brief Set chart interactive state
     * @param bInteractive Interactive state
     */
    virtual void SetInteractive(BOOL bInteractive) = 0;

    /**
     * @brief Check if zoom is enabled
     * @return TRUE if zoom is enabled
     */
    virtual BOOL IsZoomEnabled() = 0;

    /**
     * @brief Set zoom enabled state
     * @param bEnabled Zoom enabled state
     */
    virtual void SetZoomEnabled(BOOL bEnabled) = 0;

    /**
     * @brief Check if scroll is enabled
     * @return TRUE if scroll is enabled
     */
    virtual BOOL IsScrollEnabled() = 0;

    /**
     * @brief Set scroll enabled state
     * @param bEnabled Scroll enabled state
     */
    virtual void SetScrollEnabled(BOOL bEnabled) = 0;

    /**
     * @brief Get selected value
     * @return Selected value
     */
    virtual SSelectedValue GetSelectedValue() = 0;

    /**
     * @brief Select value at given coordinates
     * @param x X coordinate
     * @param y Y coordinate
     * @return TRUE if value was selected
     */
    virtual BOOL SelectValue(float x, float y) = 0;

    /**
     * @brief Clear selection
     */
    virtual void ClearSelection() = 0;

    /**
     * @brief Called when chart data changes
     */
    virtual void OnChartDataChange() = 0;

    /**
     * @brief Called when chart size changes
     */
    virtual void OnChartSizeChanged() = 0;

    /**
     * @brief Called when touch event occurs
     */
    virtual void CallTouchListener() = 0;

    virtual void GetRect(LPRECT pRect) = 0;
};

SNSEND

#endif // __SCHART_H__
