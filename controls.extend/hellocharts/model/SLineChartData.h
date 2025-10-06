#pragma once

#include "SChartData.h"
#include <vector>

SNSBEGIN

// Forward declarations
class SLine;

/**
 * @class SLineChartData
 * @brief Data model for line chart
 */
class  SLineChartData : public SChartData
{
public:
    SLineChartData();
    SLineChartData(const std::vector<SLine*>& lines);
    SLineChartData(const SLineChartData& other);
    virtual ~SLineChartData();

    /**
     * @brief Get lines
     * @return Vector of lines
     */
    const std::vector<SLine*>& GetLines() const { return m_lines; }

    /**
     * @brief Set lines
     * @param lines Vector of lines
     */
    void SetLines(const std::vector<SLine*>& lines);

    /**
     * @brief Add line
     * @param pLine Pointer to line
     */
    void AddLine(SLine* pLine);

    /**
     * @brief Clear all lines
     */
    void ClearLines();

    /**
     * @brief Get line count
     * @return Number of lines
     */
    size_t GetLineCount() const { return m_lines.size(); }

    /**
     * @brief Get line at index
     * @param index Line index
     * @return Pointer to line or NULL if index is invalid
     */
    SLine* GetLine(size_t index) const;

    /**
     * @brief Get base value
     * @return Base value for filled lines
     */
    float GetBaseValue() const { return m_baseValue; }

    /**
     * @brief Set base value
     * @param baseValue Base value for filled lines
     */
    void SetBaseValue(float baseValue) { m_baseValue = baseValue; }

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

    /**
     * @brief Generate dummy data for testing
     * @return Dummy line chart data
     */
    static SLineChartData* GenerateDummyData();

private:
    std::vector<SLine*> m_lines;
    float m_baseValue;
    
    static const float DEFAULT_BASE_VALUE;
};

/**
 * @class SLine
 * @brief Represents a line in line chart
 */
class  SLine
{
public:
    SLine();
    SLine(const std::vector<SPointValue*>& values);
    SLine(const SLine& other);
    virtual ~SLine();

    /**
     * @brief Get line values
     * @return Vector of point values
     */
    const std::vector<SPointValue*>& GetValues() const { return m_values; }

    /**
     * @brief Set line values
     * @param values Vector of point values
     */
    void SetValues(const std::vector<SPointValue*>& values);

    /**
     * @brief Add point value
     * @param pValue Pointer to point value
     */
    void AddValue(SPointValue* pValue);

    /**
     * @brief Clear all values
     */
    void ClearValues();

    /**
     * @brief Get value count
     * @return Number of values
     */
    size_t GetValueCount() const { return m_values.size(); }

    /**
     * @brief Get value at index
     * @param index Value index
     * @return Pointer to point value or NULL if index is invalid
     */
    SPointValue* GetValue(size_t index) const;

    /**
     * @brief Get line color
     * @return Line color
     */
    COLORREF GetColor() const { return m_color; }

    /**
     * @brief Set line color
     * @param color Line color
     */
    void SetColor(COLORREF color) { m_color = color; }

    /**
     * @brief Get line stroke width
     * @return Stroke width in pixels
     */
    int GetStrokeWidth() const { return m_strokeWidth; }

    /**
     * @brief Set line stroke width
     * @param strokeWidth Stroke width in pixels
     */
    void SetStrokeWidth(int strokeWidth) { m_strokeWidth = strokeWidth; }

    /**
     * @brief Check if line is cubic
     * @return TRUE if line is cubic
     */
    BOOL IsCubic() const { return m_bCubic; }

    /**
     * @brief Set line cubic state
     * @param bCubic Cubic state
     */
    void SetCubic(BOOL bCubic) { m_bCubic = bCubic; }

    /**
     * @brief Check if line is filled
     * @return TRUE if line is filled
     */
    BOOL IsFilled() const { return m_bFilled; }

    /**
     * @brief Set line filled state
     * @param bFilled Filled state
     */
    void SetFilled(BOOL bFilled) { m_bFilled = bFilled; }

    /**
     * @brief Check if line has points
     * @return TRUE if line has points
     */
    BOOL HasPoints() const { return m_bHasPoints; }

    /**
     * @brief Set line points state
     * @param bHasPoints Points state
     */
    void SetHasPoints(BOOL bHasPoints) { m_bHasPoints = bHasPoints; }

    /**
     * @brief Check if line has labels
     * @return TRUE if line has labels
     */
    BOOL HasLabels() const { return m_bHasLabels; }

    /**
     * @brief Set line labels state
     * @param bHasLabels Labels state
     */
    void SetHasLabels(BOOL bHasLabels) { m_bHasLabels = bHasLabels; }

    /**
     * @brief Get point radius
     * @return Point radius in pixels
     */
    int GetPointRadius() const { return m_pointRadius; }

    /**
     * @brief Set point radius
     * @param pointRadius Point radius in pixels
     */
    void SetPointRadius(int pointRadius) { m_pointRadius = pointRadius; }

    /**
     * @brief Get area transparency
     * @return Area transparency (0-255)
     */
    int GetAreaTransparency() const { return m_areaTransparency; }

    /**
     * @brief Set area transparency
     * @param transparency Area transparency (0-255)
     */
    void SetAreaTransparency(int transparency) { m_areaTransparency = transparency; }

    /**
     * @brief Initialize line animation
     */
    void InitializeAnimation();

    /**
     * @brief Update line for animation
     * @param scale Animation scale factor
     */
    void Update(float scale);

    /**
     * @brief Finish line animation
     */
    void Finish();

private:
    std::vector<SPointValue*> m_values;
    COLORREF m_color;
    int m_strokeWidth;
    BOOL m_bCubic;
    BOOL m_bFilled;
    BOOL m_bHasPoints;
    BOOL m_bHasLabels;
    int m_pointRadius;
    int m_areaTransparency;
    
public:
    // Default values
    static const COLORREF DEFAULT_LINE_COLOR;
    static const int DEFAULT_STROKE_WIDTH;
    static const int DEFAULT_POINT_RADIUS;
    static const int DEFAULT_AREA_TRANSPARENCY;
};

SNSEND
