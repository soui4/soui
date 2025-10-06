#ifndef __SBUBBLECHARTDATA_H__
#define __SBUBBLECHARTDATA_H__

#include "SChartData.h"
#include <vector>

SNSBEGIN

// Forward declarations
class SBubbleValue;

/**
 * @class SBubbleChartData
 * @brief Data model for bubble chart
 */
class SBubbleChartData : public SChartData
{
public:
    SBubbleChartData();
    SBubbleChartData(const SBubbleChartData& other);
    virtual ~SBubbleChartData();

    /**
     * @brief Get bubble values
     * @return Vector of bubble values
     */
    const std::vector<SBubbleValue*>& GetValues() const { return m_values; }

    /**
     * @brief Set bubble values
     * @param values Vector of bubble values
     */
    void SetValues(const std::vector<SBubbleValue*>& values);

    /**
     * @brief Add bubble value
     * @param pValue Bubble value to add
     */
    void AddValue(SBubbleValue* pValue);

    /**
     * @brief Get bubble value by index
     * @param index Value index
     * @return Bubble value or NULL if index is invalid
     */
    SBubbleValue* GetValue(size_t index) const;

    /**
     * @brief Get number of bubble values
     * @return Number of values
     */
    size_t GetValueCount() const { return m_values.size(); }

    /**
     * @brief Clear all bubble values
     */
    void ClearValues();

    /**
     * @brief Check if bubbles have labels
     * @return TRUE if bubbles have labels
     */
    BOOL HasBubbleLabels() const { return m_bHasBubbleLabels; }

    /**
     * @brief Set bubble labels enabled state
     * @param bEnabled Labels enabled state
     */
    void SetHasBubbleLabels(BOOL bEnabled) { m_bHasBubbleLabels = bEnabled; }

    /**
     * @brief Get minimum bubble radius
     * @return Minimum bubble radius in pixels
     */
    int GetMinBubbleRadius() const { return m_minBubbleRadius; }

    /**
     * @brief Set minimum bubble radius
     * @param radius Minimum bubble radius in pixels
     */
    void SetMinBubbleRadius(int radius) { m_minBubbleRadius = radius; }

    /**
     * @brief Get maximum bubble radius
     * @return Maximum bubble radius in pixels
     */
    int GetMaxBubbleRadius() const { return m_maxBubbleRadius; }

    /**
     * @brief Set maximum bubble radius
     * @param radius Maximum bubble radius in pixels
     */
    void SetMaxBubbleRadius(int radius) { m_maxBubbleRadius = radius; }

    /**
     * @brief Get bubble scale type
     * @return Bubble scale type
     */
    int GetBubbleScale() const { return m_bubbleScale; }

    /**
     * @brief Set bubble scale type
     * @param scale Bubble scale type (0=linear, 1=area)
     */
    void SetBubbleScale(int scale) { m_bubbleScale = scale; }

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
    std::vector<SBubbleValue*> m_values;
    BOOL m_bHasBubbleLabels;
    int m_minBubbleRadius;
    int m_maxBubbleRadius;
    int m_bubbleScale; // 0=linear, 1=area

    // Default values
    static const int DEFAULT_MIN_BUBBLE_RADIUS;
    static const int DEFAULT_MAX_BUBBLE_RADIUS;
    static const int DEFAULT_BUBBLE_SCALE;
};

/**
 * @class SBubbleValue
 * @brief Represents a bubble in bubble chart
 */
class SBubbleValue
{
public:
    SBubbleValue();
    SBubbleValue(float x, float y, float z);
    SBubbleValue(float x, float y, float z, COLORREF color);
    SBubbleValue(float x, float y, float z, COLORREF color, const SStringT& label);
    SBubbleValue(const SBubbleValue& other);
    virtual ~SBubbleValue();

    /**
     * @brief Get X coordinate
     * @return X coordinate
     */
    float GetX() const { return m_x; }

    /**
     * @brief Set X coordinate
     * @param x X coordinate
     */
    void SetX(float x) { m_x = x; }

    /**
     * @brief Get Y coordinate
     * @return Y coordinate
     */
    float GetY() const { return m_y; }

    /**
     * @brief Set Y coordinate
     * @param y Y coordinate
     */
    void SetY(float y) { m_y = y; }

    /**
     * @brief Get Z value (bubble size)
     * @return Z value
     */
    float GetZ() const { return m_z; }

    /**
     * @brief Set Z value (bubble size)
     * @param z Z value
     */
    void SetZ(float z) { m_z = z; }

    /**
     * @brief Get bubble color
     * @return Bubble color
     */
    COLORREF GetColor() const { return m_color; }

    /**
     * @brief Set bubble color
     * @param color Bubble color
     */
    void SetColor(COLORREF color) { m_color = color; }

    /**
     * @brief Get bubble label
     * @return Bubble label
     */
    SStringT GetLabel() const { return m_label; }

    /**
     * @brief Set bubble label
     * @param label Bubble label
     */
    void SetLabel(const SStringT& label) { m_label = label; }

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
    float m_x;
    float m_y;
    float m_z;
    COLORREF m_color;
    SStringT m_label;
    
    // Animation support
    COLORREF m_originColor;
    COLORREF m_diffColor;
};

SNSEND

#endif // __SBUBBLECHARTDATA_H__
