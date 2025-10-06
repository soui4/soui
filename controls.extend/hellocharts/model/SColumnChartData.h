#ifndef __SCOLUMNCHARTDATA_H__
#define __SCOLUMNCHARTDATA_H__
#include "SChartData.h"
#include "SLineChartData.h"  // For SPointValue
#include <vector>

SNSBEGIN

// Forward declarations
class SSubcolumnValue;

/**
 * @class SColumn
 * @brief Represents a column in column chart
 */
class SColumn
{
public:
    static const COLORREF DEFAULT_COLUMN_COLOR = RGBA(51, 181, 229, 255);
    static const int DEFAULT_COLUMN_STROKE_WIDTH = 1;

public:
    /**
     * @brief Constructor
     * @param values Column values
     */
    SColumn(const std::vector<SSubcolumnValue*>& values);
    
    /**
     * @brief Destructor
     */
    ~SColumn();

    /**
     * @brief Get column values
     * @return Column values
     */
    const std::vector<SSubcolumnValue*>& GetValues() const { return m_values; }
    
    /**
     * @brief Get value count
     * @return Value count
     */
    size_t GetValueCount() const { return m_values.size(); }
    
    /**
     * @brief Get value at index
     * @param index Value index
     * @return Pointer to value or NULL
     */
    SSubcolumnValue* GetValue(size_t index) const;
    
    /**
     * @brief Add value
     * @param pValue Pointer to value
     */
    void AddValue(SSubcolumnValue* pValue);
    
    /**
     * @brief Get column color
     * @return Column color
     */
    COLORREF GetColor() const { return m_color; }
    
    /**
     * @brief Set column color
     * @param color Column color
     */
    void SetColor(COLORREF color) { m_color = color; }
    
    /**
     * @brief Get stroke width
     * @return Stroke width
     */
    int GetStrokeWidth() const { return m_strokeWidth; }
    
    /**
     * @brief Set stroke width
     * @param width Stroke width
     */
    void SetStrokeWidth(int width) { m_strokeWidth = width; }
    
    /**
     * @brief Check if column has labels
     * @return TRUE if has labels
     */
    BOOL HasLabels() const { return m_bHasLabels; }
    
    /**
     * @brief Set has labels flag
     * @param bHasLabels Has labels flag
     */
    void SetHasLabels(BOOL bHasLabels) { m_bHasLabels = bHasLabels; }

private:
    std::vector<SSubcolumnValue*> m_values;
    COLORREF m_color;
    int m_strokeWidth;
    BOOL m_bHasLabels;
};

/**
 * @class SSubcolumnValue
 * @brief Represents a subcolumn value in column chart
 */
class SSubcolumnValue : public SPointValue
{
public:
    /**
     * @brief Constructor
     * @param value Value
     */
    SSubcolumnValue(float value);
    
    /**
     * @brief Constructor with color
     * @param value Value
     * @param color Color
     */
    SSubcolumnValue(float value, COLORREF color);
    
    /**
     * @brief Get difference value
     * @return Difference value
     */
    float GetDiff() const { return m_diff; }
    
    /**
     * @brief Set difference value
     * @param diff Difference value
     */
    void SetDiff(float diff) { m_diff = diff; }

    /**
     * @brief Get target Y value (for animation)
     * @return Target Y value
     */
    float GetTargetY() const { return m_targetY; }

    /**
     * @brief Set target Y value (for animation)
     * @param targetY Target Y value
     */
    void SetTargetY(float targetY) { m_targetY = targetY; }

    /**
     * @brief Initialize animation
     */
    void InitializeAnimation();

private:
    float m_diff;
    float m_targetY;  // Target value for animation
};

/**
 * @class SColumnChartData
 * @brief Column chart data container
 */
class SColumnChartData : public SChartData
{
public:
    /**
     * @brief Constructor
     */
    SColumnChartData();
    
    /**
     * @brief Destructor
     */
    virtual ~SColumnChartData();

    /**
     * @brief Get columns
     * @return Columns
     */
    const std::vector<SColumn*>& GetColumns() const { return m_columns; }
    
    /**
     * @brief Get column count
     * @return Column count
     */
    size_t GetColumnCount() const { return m_columns.size(); }
    
    /**
     * @brief Get column at index
     * @param index Column index
     * @return Pointer to column or NULL
     */
    SColumn* GetColumn(size_t index) const;
    
    /**
     * @brief Add column
     * @param pColumn Pointer to column
     */
    void AddColumn(SColumn* pColumn);
    
    /**
     * @brief Check if chart is stacked
     * @return TRUE if stacked
     */
    BOOL IsStacked() const { return m_bStacked; }
    
    /**
     * @brief Set stacked flag
     * @param bStacked Stacked flag
     */
    void SetStacked(BOOL bStacked) { m_bStacked = bStacked; }
    
    /**
     * @brief Get fill ratio
     * @return Fill ratio (0.0 - 1.0)
     */
    float GetFillRatio() const { return m_fillRatio; }
    
    /**
     * @brief Set fill ratio
     * @param ratio Fill ratio (0.0 - 1.0)
     */
    void SetFillRatio(float ratio) { m_fillRatio = ratio; }

    /**
     * @brief Generate dummy data for testing
     * @return Pointer to dummy data
     */
    static SColumnChartData* GenerateDummyData();

    // SChartData interface
    virtual void InitializeAnimation() override;
    virtual void Update(float scale) override;
    virtual void Finish() override;

private:
    std::vector<SColumn*> m_columns;
    BOOL m_bStacked;
    float m_fillRatio;
};

SNSEND

#endif // __SCOLUMNCHARTDATA_H__
