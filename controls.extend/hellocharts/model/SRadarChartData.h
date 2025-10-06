#pragma once

#include "SChartData.h"
#include <vector>

SNSBEGIN

// Forward declarations
class SRadarValue;

/**
 * @class SRadarChartData
 * @brief Data model for radar chart
 */
class SRadarChartData : public SChartData
{
public:
    SRadarChartData();
    SRadarChartData(const SRadarChartData& other);
    virtual ~SRadarChartData();

    /**
     * @brief Get radar values
     * @return Vector of radar values
     */
    const std::vector<SRadarValue*>& GetValues() const { return m_values; }

    /**
     * @brief Set radar values
     * @param values Vector of radar values
     */
    void SetValues(const std::vector<SRadarValue*>& values);

    /**
     * @brief Add radar value
     * @param pValue Radar value to add
     */
    void AddValue(SRadarValue* pValue);

    /**
     * @brief Get radar value by index
     * @param index Value index
     * @return Radar value or NULL if index is invalid
     */
    SRadarValue* GetValue(size_t index) const;

    /**
     * @brief Get number of radar values
     * @return Number of values
     */
    size_t GetValueCount() const { return m_values.size(); }

    /**
     * @brief Clear all radar values
     */
    void ClearValues();

    /**
     * @brief Get radar axes labels
     * @return Vector of axis labels
     */
    const std::vector<SStringT>& GetAxesLabels() const { return m_axesLabels; }

    /**
     * @brief Set radar axes labels
     * @param labels Vector of axis labels
     */
    void SetAxesLabels(const std::vector<SStringT>& labels);

    /**
     * @brief Add axis label
     * @param label Axis label
     */
    void AddAxisLabel(const SStringT& label);

    /**
     * @brief Get number of axes
     * @return Number of axes
     */
    size_t GetAxesCount() const { return m_axesLabels.size(); }

    /**
     * @brief Check if radar has labels
     * @return TRUE if radar has labels
     */
    BOOL HasRadarLabels() const { return m_bHasRadarLabels; }

    /**
     * @brief Set radar labels enabled state
     * @param bEnabled Labels enabled state
     */
    void SetHasRadarLabels(BOOL bEnabled) { m_bHasRadarLabels = bEnabled; }

    /**
     * @brief Check if radar has web lines
     * @return TRUE if radar has web lines
     */
    BOOL HasWebLines() const { return m_bHasWebLines; }

    /**
     * @brief Set web lines enabled state
     * @param bEnabled Web lines enabled state
     */
    void SetHasWebLines(BOOL bEnabled) { m_bHasWebLines = bEnabled; }

    /**
     * @brief Get number of web rings
     * @return Number of web rings
     */
    int GetWebRings() const { return m_webRings; }

    /**
     * @brief Set number of web rings
     * @param rings Number of web rings
     */
    void SetWebRings(int rings) { m_webRings = rings; }

    /**
     * @brief Get maximum value for scaling
     * @return Maximum value
     */
    float GetMaxValue() const { return m_maxValue; }

    /**
     * @brief Set maximum value for scaling
     * @param maxValue Maximum value
     */
    void SetMaxValue(float maxValue) { m_maxValue = maxValue; }

    /**
     * @brief Initialize animation data
     */
    virtual void InitializeAnimation() override;

    /**
     * @brief Update chart data for animation
     * @param scale Animation scale factor
     */
    virtual void Update(float scale) override;

    /**
     * @brief Finish chart data animation
     */
    virtual void Finish() override;

private:
    std::vector<SRadarValue*> m_values;
    std::vector<SStringT> m_axesLabels;
    BOOL m_bHasRadarLabels;
    BOOL m_bHasWebLines;
    int m_webRings;
    float m_maxValue;

    // Default values
    static const int DEFAULT_WEB_RINGS;
    static const float DEFAULT_MAX_VALUE;
};

/**
 * @class SRadarValue
 * @brief Represents a radar data series
 */
class SRadarValue
{
public:
    SRadarValue();
    SRadarValue(const SStringT& label);
    SRadarValue(const SStringT& label, COLORREF color);
    SRadarValue(const SRadarValue& other);
    virtual ~SRadarValue();

    /**
     * @brief Get radar values for each axis
     * @return Vector of axis values
     */
    const std::vector<float>& GetAxisValues() const { return m_axisValues; }

    /**
     * @brief Set radar values for each axis
     * @param values Vector of axis values
     */
    void SetAxisValues(const std::vector<float>& values);

    /**
     * @brief Add axis value
     * @param value Axis value
     */
    void AddAxisValue(float value);

    /**
     * @brief Get axis value by index
     * @param index Axis index
     * @return Axis value or 0.0f if index is invalid
     */
    float GetAxisValue(size_t index) const;

    /**
     * @brief Set axis value by index
     * @param index Axis index
     * @param value Axis value
     */
    void SetAxisValue(size_t index, float value);

    /**
     * @brief Get number of axis values
     * @return Number of axis values
     */
    size_t GetAxisValueCount() const { return m_axisValues.size(); }

    /**
     * @brief Get radar series label
     * @return Series label
     */
    SStringT GetLabel() const { return m_label; }

    /**
     * @brief Set radar series label
     * @param label Series label
     */
    void SetLabel(const SStringT& label) { m_label = label; }

    /**
     * @brief Get radar series color
     * @return Series color
     */
    COLORREF GetColor() const { return m_color; }

    /**
     * @brief Set radar series color
     * @param color Series color
     */
    void SetColor(COLORREF color) { m_color = color; }

    /**
     * @brief Get original color (for animation)
     * @return Original color
     */
    COLORREF GetOriginColor() const { return m_originColor; }

    /**
     * @brief Set original color (for animation)
     * @param color Original color
     */
    void SetOriginColor(COLORREF color) { m_originColor = color; }

    /**
     * @brief Get difference color (for animation)
     * @return Difference color
     */
    COLORREF GetDiffColor() const { return m_diffColor; }

    /**
     * @brief Set difference color (for animation)
     * @param color Difference color
     */
    void SetDiffColor(COLORREF color) { m_diffColor = color; }

    /**
     * @brief Initialize animation
     */
    void InitializeAnimation();

    /**
     * @brief Update animation
     * @param scale Animation scale (0.0 to 1.0)
     */
    void Update(float scale);

    /**
     * @brief Finish animation
     */
    void Finish();

private:
    std::vector<float> m_axisValues;
    SStringT m_label;
    COLORREF m_color;
    
    // Animation support
    COLORREF m_originColor;
    COLORREF m_diffColor;
};

SNSEND
