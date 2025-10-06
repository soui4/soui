#pragma once

#include "SChartRenderer.h"
#include "../model/SBubbleChartData.h"

SNSBEGIN

// Forward declarations
class SBubbleChartView;

/**
 * @class SBubbleChartRenderer
 * @brief Renderer for bubble charts
 */
class SBubbleChartRenderer : public SChartRenderer
{
public:
    SBubbleChartRenderer(ISChart* pChart);
    virtual ~SBubbleChartRenderer();

    // SChartRenderer interface
    virtual void Draw(IRenderTarget* pRT) override;
    virtual void DrawUnclipped(IRenderTarget* pRT) override;
    virtual BOOL IsTouched(float touchX, float touchY) override;

    /**
     * @brief Select bubble value
     * @param selectedValue Selected value
     */
    virtual void SelectValue(const SSelectedValue& selectedValue) override;

    /**
     * @brief Deselect bubble value
     */
    virtual void DeselectValue() override;

    /**
     * @brief Get bubble chart view
     * @return Bubble chart view
     */
    SBubbleChartView* GetBubbleChartView() const { return m_pBubbleChartView; }

    /**
     * @brief Set bubble chart view
     * @param pView Bubble chart view
     */
    void SetBubbleChartView(SBubbleChartView* pView) { m_pBubbleChartView = pView; }

private:
    /**
     * @brief Draw bubbles
     * @param pRT Render target
     */
    void DrawBubbles(IRenderTarget* pRT);

    /**
     * @brief Draw bubble labels
     * @param pRT Render target
     */
    void DrawBubbleLabels(IRenderTarget* pRT);

    /**
     * @brief Draw single bubble
     * @param pRT Render target
     * @param pValue Bubble value
     * @param radius Bubble radius
     * @param centerX Bubble center X
     * @param centerY Bubble center Y
     * @param isSelected Is bubble selected
     */
    void DrawBubble(IRenderTarget* pRT, SBubbleValue* pValue, float radius,
                   float centerX, float centerY, BOOL isSelected);

    /**
     * @brief Calculate bubble radius
     * @param pValue Bubble value
     * @param pData Bubble chart data
     * @return Bubble radius in pixels
     */
    float CalculateBubbleRadius(SBubbleValue* pValue, SBubbleChartData* pData);

    /**
     * @brief Get bubble chart data
     * @return Bubble chart data
     */
    SBubbleChartData* GetBubbleChartData() const;

    /**
     * @brief Check if point is inside bubble
     * @param touchX Touch X coordinate
     * @param touchY Touch Y coordinate
     * @param centerX Bubble center X
     * @param centerY Bubble center Y
     * @param radius Bubble radius
     * @return TRUE if point is inside bubble
     */
    BOOL IsPointInBubble(float touchX, float touchY, float centerX, float centerY, float radius);

    /**
     * @brief Find minimum and maximum Z values
     * @param pData Bubble chart data
     * @param minZ Output minimum Z value
     * @param maxZ Output maximum Z value
     */
    void FindMinMaxZ(SBubbleChartData* pData, float& minZ, float& maxZ);

private:
    SBubbleChartView* m_pBubbleChartView;

    // Default values
    static const float DEFAULT_BUBBLE_ALPHA;
    static const float SELECTED_BUBBLE_ALPHA;
};

/**
 * @interface IBubbleChartOnValueSelectListener
 * @brief Interface for bubble chart value selection events
 */
struct IBubbleChartOnValueSelectListener
{
    /**
     * @brief Called when a bubble value is selected
     * @param bubbleIndex Selected bubble index
     * @param pValue Selected bubble value
     */
    virtual void OnValueSelected(int bubbleIndex, SBubbleValue* pValue) = 0;

    /**
     * @brief Called when bubble value is deselected
     */
    virtual void OnValueDeselected() = 0;
};

SNSEND
