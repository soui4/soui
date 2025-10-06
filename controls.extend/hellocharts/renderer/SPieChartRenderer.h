#pragma once

#include "SChartRenderer.h"
#include "../model/SPieChartData.h"

SNSBEGIN

/**
 * @class SPieChartRenderer
 * @brief Renderer for pie charts
 */
class SPieChartRenderer : public SChartRenderer
{
public:
    SPieChartRenderer(ISChart* pChart);
    virtual ~SPieChartRenderer();

    // SChartRenderer interface
    virtual void Draw(IRenderTarget* pRT) OVERRIDE;
    virtual void DrawUnclipped(IRenderTarget* pRT) OVERRIDE;
    virtual BOOL IsTouched(float touchX, float touchY) OVERRIDE;
    virtual void SelectValue(const SSelectedValue& selectedValue) OVERRIDE;
    virtual void DeselectValue() OVERRIDE;

    /**
     * @brief Get selected value
     * @return Selected value
     */
    SSelectedValue GetSelectedValue() const { return m_selectedValue; }

    /**
     * @brief Set maximum pie radius
     * @param radius Maximum radius
     */
    void SetMaxRadius(float radius) { m_maxRadius = radius; }

    /**
     * @brief Get maximum pie radius
     * @return Maximum radius
     */
    float GetMaxRadius() const { return m_maxRadius; }

    /**
     * @brief Set label text size
     * @param textSize Label text size
     */
    void SetLabelTextSize(int textSize) { m_labelTextSize = textSize; }

    /**
     * @brief Get label text size
     * @return Label text size
     */
    int GetLabelTextSize() const { return m_labelTextSize; }

private:
    /**
     * @brief Draw pie slices
     * @param pRT Render target
     */
    void DrawSlices(IRenderTarget* pRT);

    /**
     * @brief Draw slice labels
     * @param pRT Render target
     */
    void DrawLabels(IRenderTarget* pRT);

    /**
     * @brief Draw center circle (for donut charts)
     * @param pRT Render target
     */
    void DrawCenterCircle(IRenderTarget* pRT);

    /**
     * @brief Calculate pie center point
     * @return Center point
     */
    CPoint CalculatePieCenter();

    /**
     * @brief Calculate pie radius
     * @return Pie radius
     */
    float CalculatePieRadius();

    /**
     * @brief Calculate slice angles
     * @param startAngles Output array of start angles
     * @param sweepAngles Output array of sweep angles
     */
    void CalculateSliceAngles(std::vector<float>& startAngles, std::vector<float>& sweepAngles);

    /**
     * @brief Draw single slice
     * @param pRT Render target
     * @param center Pie center
     * @param radius Pie radius
     * @param innerRadius Inner radius (for donut)
     * @param startAngle Start angle in degrees
     * @param sweepAngle Sweep angle in degrees
     * @param color Slice color
     */
    void DrawSlice(IRenderTarget* pRT, CPoint center, float radius, float innerRadius,
                   float startAngle, float sweepAngle, COLORREF color);

    /**
     * @brief Draw slice label
     * @param pRT Render target
     * @param center Pie center
     * @param radius Pie radius
     * @param angle Label angle in degrees
     * @param pValue Slice value
     */
    void DrawSliceLabel(IRenderTarget* pRT, CPoint center, float radius,
                        float angle, SSliceValue* pValue);

    /**
     * @brief Draw label for large slice (inside pie)
     * @param pRT Render target
     * @param center Pie center
     * @param radius Pie radius
     * @param angle Label angle in degrees
     * @param pValue Slice value
     * @param sweepAngle Slice sweep angle
     */
    void DrawLargeSliceLabel(IRenderTarget* pRT, CPoint center, float radius,
                            float angle, SSliceValue* pValue, float sweepAngle);

    /**
     * @brief Draw labels for small slices (outside with leader lines)
     * @param pRT Render target
     * @param center Pie center
     * @param radius Pie radius
     * @param startAngles Start angles array
     * @param sweepAngles Sweep angles array
     * @param smallSlices Indices of small slices
     * @param pData Pie chart data
     */
    void DrawSmallSliceLabels(IRenderTarget* pRT, CPoint center, float radius,
                             const std::vector<float>& startAngles,
                             const std::vector<float>& sweepAngles,
                             const std::vector<size_t>& smallSlices,
                             SPieChartData* pData);

    /**
     * @brief Draw legend for small slices
     * @param pRT Render target
     * @param smallSlices Indices of small slices
     * @param pData Pie chart data
     */
    void DrawLegend(IRenderTarget* pRT, const std::vector<size_t>& smallSlices,
                   SPieChartData* pData);

    /**
     * @brief Calculate total value of all slices
     * @param pData Pie chart data
     * @return Total value
     */
    float CalculateTotalValue(SPieChartData* pData);

    /**
     * @brief Check if point is inside slice
     * @param point Point to check
     * @param center Pie center
     * @param radius Pie radius
     * @param startAngle Start angle in degrees
     * @param sweepAngle Sweep angle in degrees
     * @return TRUE if point is inside slice
     */
    BOOL IsPointInSlice(CPoint point, CPoint center, float radius,
                        float startAngle, float sweepAngle);

    /**
     * @brief Calculate angle from center to point
     * @param center Center point
     * @param point Target point
     * @return Angle in degrees
     */
    float CalculateAngle(CPoint center, CPoint point);

    /**
     * @brief Normalize angle to 0-360 range
     * @param angle Angle in degrees
     * @return Normalized angle
     */
    float NormalizeAngle(float angle);

private:
    float m_maxRadius;
    int m_labelTextSize;
    SSelectedValue m_selectedValue;
    
    // Constants
    static const float DEFAULT_MAX_RADIUS;
    static const int DEFAULT_LABEL_TEXT_SIZE;
    static const float LABEL_MARGIN_RATIO;
};

SNSEND
