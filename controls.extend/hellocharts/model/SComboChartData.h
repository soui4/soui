#ifndef __SCOMBOCHARTDATA_H__
#define __SCOMBOCHARTDATA_H__

#include "SChartData.h"
#include "SLineChartData.h"
#include "SColumnChartData.h"

SNSBEGIN

/**
 * @class SComboChartData
 * @brief Data model for combo chart (combination of line and column charts)
 */
class SComboChartData : public SChartData
{
public:
    SComboChartData();
    SComboChartData(const SComboChartData& other);
    virtual ~SComboChartData();

    /**
     * @brief Get line chart data
     * @return Pointer to line chart data
     */
    SLineChartData* GetLineChartData() const { return m_pLineChartData; }

    /**
     * @brief Set line chart data
     * @param pData Pointer to line chart data
     */
    void SetLineChartData(SLineChartData* pData);

    /**
     * @brief Get column chart data
     * @return Pointer to column chart data
     */
    SColumnChartData* GetColumnChartData() const { return m_pColumnChartData; }

    /**
     * @brief Set column chart data
     * @param pData Pointer to column chart data
     */
    void SetColumnChartData(SColumnChartData* pData);

    /**
     * @brief Check if combo chart has line data
     * @return TRUE if has line data
     */
    BOOL HasLineData() const { return m_pLineChartData != NULL; }

    /**
     * @brief Check if combo chart has column data
     * @return TRUE if has column data
     */
    BOOL HasColumnData() const { return m_pColumnChartData != NULL; }

    /**
     * @brief Get combo chart type
     * @return Combo chart type
     */
    int GetComboType() const { return m_comboType; }

    /**
     * @brief Set combo chart type
     * @param type Combo chart type (0=columns behind lines, 1=lines behind columns)
     */
    void SetComboType(int type) { m_comboType = type; }

    /**
     * @brief Check if using separate Y axes
     * @return TRUE if using separate Y axes
     */
    BOOL IsUsingSeparateYAxes() const { return m_bUseSeparateYAxes; }

    /**
     * @brief Set separate Y axes usage
     * @param bUseSeparate TRUE to use separate Y axes
     */
    void SetUseSeparateYAxes(BOOL bUseSeparate) { m_bUseSeparateYAxes = bUseSeparate; }

    /**
     * @brief Get secondary Y axis
     * @return Pointer to secondary Y axis
     */
    SAxis* GetSecondaryAxisY() const { return m_pSecondaryAxisY; }

    /**
     * @brief Set secondary Y axis
     * @param pAxis Pointer to secondary Y axis
     */
    void SetSecondaryAxisY(SAxis* pAxis);

    /**
     * @brief Check if secondary Y axis is on right side
     * @return TRUE if secondary Y axis is on right side
     */
    BOOL IsSecondaryAxisOnRight() const { return m_bSecondaryAxisOnRight; }

    /**
     * @brief Set secondary Y axis position
     * @param bOnRight TRUE to place secondary Y axis on right side
     */
    void SetSecondaryAxisOnRight(BOOL bOnRight) { m_bSecondaryAxisOnRight = bOnRight; }

    /**
     * @brief Get line chart alpha
     * @return Line chart alpha (0.0 to 1.0)
     */
    float GetLineChartAlpha() const { return m_lineChartAlpha; }

    /**
     * @brief Set line chart alpha
     * @param alpha Line chart alpha (0.0 to 1.0)
     */
    void SetLineChartAlpha(float alpha);

    /**
     * @brief Get column chart alpha
     * @return Column chart alpha (0.0 to 1.0)
     */
    float GetColumnChartAlpha() const { return m_columnChartAlpha; }

    /**
     * @brief Set column chart alpha
     * @param alpha Column chart alpha (0.0 to 1.0)
     */
    void SetColumnChartAlpha(float alpha);

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
    SLineChartData* m_pLineChartData;
    SColumnChartData* m_pColumnChartData;
    SAxis* m_pSecondaryAxisY;
    
    int m_comboType;
    BOOL m_bUseSeparateYAxes;
    BOOL m_bSecondaryAxisOnRight;
    float m_lineChartAlpha;
    float m_columnChartAlpha;

    // Default values
    static const int DEFAULT_COMBO_TYPE;
    static const float DEFAULT_LINE_ALPHA;
    static const float DEFAULT_COLUMN_ALPHA;
};

SNSEND

#endif // __SCOMBOCHARTDATA_H__
