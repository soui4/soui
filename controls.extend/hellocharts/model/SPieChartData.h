#pragma once

#include "SChartData.h"
#include <vector>

SNSBEGIN

// Forward declarations
class SSliceValue;

/**
 * @class SPieChartData
 * @brief Data model for pie charts
 */
class SPieChartData : public SChartData
{
public:
    SPieChartData();
    SPieChartData(const SPieChartData& other);
    virtual ~SPieChartData();

    /**
     * @brief Get slice values
     * @return Vector of slice values
     */
    const std::vector<SSliceValue*>& GetValues() const { return m_values; }

    /**
     * @brief Set slice values
     * @param values Vector of slice values
     */
    void SetValues(const std::vector<SSliceValue*>& values);

    /**
     * @brief Add slice value
     * @param pValue Pointer to slice value
     */
    void AddValue(SSliceValue* pValue);

    /**
     * @brief Get slice value by index
     * @param index Value index
     * @return Pointer to slice value or NULL if index is invalid
     */
    SSliceValue* GetValue(size_t index) const;

    /**
     * @brief Get number of slice values
     * @return Number of slice values
     */
    size_t GetValueCount() const { return m_values.size(); }

    /**
     * @brief Clear all slice values
     */
    void ClearValues();

    /**
     * @brief Check if pie chart has labels
     * @return TRUE if has labels
     */
    BOOL HasLabels() const { return m_bHasLabels; }

    /**
     * @brief Set labels state
     * @param bHasLabels Labels state
     */
    void SetHasLabels(BOOL bHasLabels) { m_bHasLabels = bHasLabels; }

    /**
     * @brief Check if pie chart has label lines
     * @return TRUE if has label lines
     */
    BOOL HasLabelLines() const { return m_bHasLabelLines; }

    /**
     * @brief Set label lines state
     * @param bHasLabelLines Label lines state
     */
    void SetHasLabelLines(BOOL bHasLabelLines) { m_bHasLabelLines = bHasLabelLines; }

    /**
     * @brief Get center circle ratio (for donut charts)
     * @return Center circle ratio (0.0 = pie, >0.0 = donut)
     */
    float GetCenterCircleRatio() const { return m_centerCircleRatio; }

    /**
     * @brief Set center circle ratio (for donut charts)
     * @param ratio Center circle ratio (0.0 = pie, >0.0 = donut)
     */
    void SetCenterCircleRatio(float ratio) { m_centerCircleRatio = ratio; }

    /**
     * @brief Get center circle color
     * @return Center circle color
     */
    COLORREF GetCenterCircleColor() const { return m_centerCircleColor; }

    /**
     * @brief Set center circle color
     * @param color Center circle color
     */
    void SetCenterCircleColor(COLORREF color) { m_centerCircleColor = color; }

    /**
     * @brief Get slice spacing
     * @return Slice spacing in degrees
     */
    float GetSliceSpacing() const { return m_sliceSpacing; }

    /**
     * @brief Set slice spacing
     * @param spacing Slice spacing in degrees
     */
    void SetSliceSpacing(float spacing) { m_sliceSpacing = spacing; }

    void InitializeAnimation() override;
    void Update(float scale) override;
    void Finish() override;

private:
    std::vector<SSliceValue*> m_values;
    BOOL m_bHasLabels;
    BOOL m_bHasLabelLines;
    float m_centerCircleRatio;
    COLORREF m_centerCircleColor;
    float m_sliceSpacing;
};

/**
 * @class SSliceValue
 * @brief Represents a slice in pie chart
 */
class SSliceValue
{
public:
    SSliceValue();
    SSliceValue(float value);
    SSliceValue(float value, COLORREF color);
    SSliceValue(float value, COLORREF color, const SStringT& label);
    SSliceValue(const SSliceValue& other);
    virtual ~SSliceValue();

    /**
     * @brief Get slice value
     * @return Slice value
     */
    float GetValue() const { return m_value; }

    /**
     * @brief Set slice value
     * @param value Slice value
     */
    void SetValue(float value) { m_value = value; }

    /**
     * @brief Get slice color
     * @return Slice color
     */
    COLORREF GetColor() const { return m_color; }

    /**
     * @brief Set slice color
     * @param color Slice color
     */
    void SetColor(COLORREF color) { m_color = color; }

    /**
     * @brief Get slice label
     * @return Slice label
     */
    SStringT GetLabel() const { return m_label; }

    /**
     * @brief Set slice label
     * @param label Slice label
     */
    void SetLabel(const SStringT& label) { m_label = label; }

    /**
     * @brief Get slice origin color (for animation)
     * @return Origin color
     */
    COLORREF GetOriginColor() const { return m_originColor; }

    /**
     * @brief Set slice origin color (for animation)
     * @param color Origin color
     */
    void SetOriginColor(COLORREF color) { m_originColor = color; }

    /**
     * @brief Get slice diff color (for animation)
     * @return Diff color
     */
    COLORREF GetDiffColor() const { return m_diffColor; }

    /**
     * @brief Set slice diff color (for animation)
     * @param color Diff color
     */
    void SetDiffColor(COLORREF color) { m_diffColor = color; }

    /**
     * @brief Initialize slice animation
     */
    void InitializeAnimation();

    /**
     * @brief Update slice for animation
     * @param scale Animation scale factor
     */
    void Update(float scale);

    /**
     * @brief Finish slice animation
     */
    void Finish();

private:
    float m_value;
    COLORREF m_color;
    SStringT m_label;
    COLORREF m_originColor;
    COLORREF m_diffColor;
};

SNSEND
