#ifndef __SCHARTDATA_H__
#define __SCHARTDATA_H__ 

SNSBEGIN

// Forward declarations
class SAxis;

/**
 * @class SChartData
 * @brief Base class for all chart data types
 */
class SChartData
{
public:
    SChartData();
    SChartData(const SChartData& other);
    virtual ~SChartData();

    /**
     * @brief Get chart background color
     * @return Background color
     */
    COLORREF GetBackgroundColor() const { return m_backgroundColor; }

    /**
     * @brief Set chart background color
     * @param color Background color
     */
    void SetBackgroundColor(COLORREF color) { m_backgroundColor = color; }

    /**
     * @brief Check if axes are enabled
     * @return TRUE if axes are enabled
     */
    BOOL IsAxesEnabled() const { return m_bAxesEnabled; }

    /**
     * @brief Set axes enabled state
     * @param bEnabled Axes enabled state
     */
    void SetAxesEnabled(BOOL bEnabled) { m_bAxesEnabled = bEnabled; }

    /**
     * @brief Get X axis
     * @return Pointer to X axis
     */
    SAxis* GetAxisX() { return m_pAxisX; }

    /**
     * @brief Set X axis
     * @param pAxis Pointer to X axis
     */
    void SetAxisX(SAxis* pAxis);

    /**
     * @brief Get Y axis
     * @return Pointer to Y axis
     */
    SAxis* GetAxisY() { return m_pAxisY; }

    /**
     * @brief Set Y axis
     * @param pAxis Pointer to Y axis
     */
    void SetAxisY(SAxis* pAxis);

    /**
     * @brief Check if value selection is enabled
     * @return TRUE if value selection is enabled
     */
    BOOL IsValueSelectionEnabled() const { return m_bValueSelectionEnabled; }

    /**
     * @brief Set value selection enabled state
     * @param bEnabled Value selection enabled state
     */
    void SetValueSelectionEnabled(BOOL bEnabled) { m_bValueSelectionEnabled = bEnabled; }

    /**
     * @brief Check if value labels are enabled
     * @return TRUE if value labels are enabled
     */
    BOOL IsValueLabelsEnabled() const { return m_bValueLabelsEnabled; }

    /**
     * @brief Set value labels enabled state
     * @param bEnabled Value labels enabled state
     */
    void SetValueLabelsEnabled(BOOL bEnabled) { m_bValueLabelsEnabled = bEnabled; }

    /**
     * @brief Initialize animation data (set diff values)
     */
    virtual void InitializeAnimation() {}

    /**
     * @brief Update chart data for animation
     * @param scale Animation scale factor
     */
    virtual void Update(float scale) {}

    /**
     * @brief Finish chart data animation
     */
    virtual void Finish(){}

protected:
    COLORREF m_backgroundColor;
    BOOL m_bAxesEnabled;
    BOOL m_bValueSelectionEnabled;
    BOOL m_bValueLabelsEnabled;
    SAxis* m_pAxisX;
    SAxis* m_pAxisY;
};

/**
 * @class SSelectedValue
 * @brief Represents a selected value in chart
 */
class  SSelectedValue
{
public:
    SSelectedValue();
    SSelectedValue(int firstIndex, int secondIndex);
    SSelectedValue(const SSelectedValue& other);

    /**
     * @brief Check if value is set
     * @return TRUE if value is set
     */
    BOOL IsSet() const { return m_bIsSet; }

    /**
     * @brief Set value
     * @param firstIndex First index
     * @param secondIndex Second index
     */
    void Set(int firstIndex, int secondIndex);

    /**
     * @brief Clear value
     */
    void Clear();

    /**
     * @brief Get first index
     * @return First index
     */
    int GetFirstIndex() const { return m_firstIndex; }

    /**
     * @brief Get second index
     * @return Second index
     */
    int GetSecondIndex() const { return m_secondIndex; }

private:
    BOOL m_bIsSet;
    int m_firstIndex;
    int m_secondIndex;
};

/**
 * @class SViewport
 * @brief Represents chart viewport
 */
class  SViewport
{
public:
    SViewport();
    SViewport(float left, float top, float right, float bottom);
    SViewport(const SViewport& other);

    /**
     * @brief Get left boundary
     * @return Left boundary
     */
    float GetLeft() const { return m_left; }

    /**
     * @brief Set left boundary
     * @param left Left boundary
     */
    void SetLeft(float left) { m_left = left; }

    /**
     * @brief Get top boundary
     * @return Top boundary
     */
    float GetTop() const { return m_top; }

    /**
     * @brief Set top boundary
     * @param top Top boundary
     */
    void SetTop(float top) { m_top = top; }

    /**
     * @brief Get right boundary
     * @return Right boundary
     */
    float GetRight() const { return m_right; }

    /**
     * @brief Set right boundary
     * @param right Right boundary
     */
    void SetRight(float right) { m_right = right; }

    /**
     * @brief Get bottom boundary
     * @return Bottom boundary
     */
    float GetBottom() const { return m_bottom; }

    /**
     * @brief Set bottom boundary
     * @param bottom Bottom boundary
     */
    void SetBottom(float bottom) { m_bottom = bottom; }

    /**
     * @brief Get viewport width
     * @return Viewport width
     */
    float GetWidth() const { return m_right - m_left; }

    /**
     * @brief Get viewport height
     * @return Viewport height
     */
    float GetHeight() const { return m_bottom - m_top; }

    /**
     * @brief Set viewport bounds
     * @param left Left boundary
     * @param top Top boundary
     * @param right Right boundary
     * @param bottom Bottom boundary
     */
    void Set(float left, float top, float right, float bottom);

    /**
     * @brief Check if viewport contains point
     * @param x X coordinate
     * @param y Y coordinate
     * @return TRUE if viewport contains point
     */
    BOOL Contains(float x, float y) const;

    /**
     * @brief Check if viewport is valid
     * @return TRUE if viewport is valid
     */
    BOOL IsValid() const;


    float GetCenterX() const
    {
        return (m_left + m_right) / 2;
    }

    float GetCenterY() const
    {
        return (m_top + m_bottom) / 2;
    }

  private:
    float m_left;
    float m_top;
    float m_right;
    float m_bottom;
};

/**
 * @class SPointValue
 * @brief Represents a point value in line chart
 */
class  SPointValue
{
public:
    SPointValue();
    SPointValue(float x, float y);
    SPointValue(float x, float y, const SStringT& label);
    SPointValue(const SPointValue& other);
    virtual ~SPointValue();

    /**
     * @brief Get X value
     * @return X value
     */
    float GetX() const { return m_x; }

    /**
     * @brief Set X value
     * @param x X value
     */
    void SetX(float x) { m_x = x; }

    /**
     * @brief Get Y value
     * @return Y value
     */
    float GetY() const { return m_y; }

    /**
     * @brief Set Y value
     * @param y Y value
     */
    void SetY(float y) { m_y = y; }

    /**
     * @brief Get point label
     * @return Point label
     */
    SStringT GetLabel() const { return m_label; }

    /**
     * @brief Set point label
     * @param label Point label
     */
    void SetLabel(const SStringT& label) { m_label = label; }

    /**
     * @brief Get point color
     * @return Point color
     */
    COLORREF GetColor() const { return m_color; }

    /**
     * @brief Set point color
     * @param color Point color
     */
    void SetColor(COLORREF color) { m_color = color; }

    /**
     * @brief Check if point has custom color
     * @return TRUE if point has custom color
     */
    BOOL HasCustomColor() const { return m_bHasCustomColor; }

    /**
     * @brief Set custom color state
     * @param bHasCustomColor Custom color state
     */
    void SetHasCustomColor(BOOL bHasCustomColor) { m_bHasCustomColor = bHasCustomColor; }

    /**
     * @brief Get origin X value (for animation)
     * @return Origin X value
     */
    float GetOriginX() const { return m_originX; }

    /**
     * @brief Set origin X value (for animation)
     * @param originX Origin X value
     */
    void SetOriginX(float originX) { m_originX = originX; }

    /**
     * @brief Get origin Y value (for animation)
     * @return Origin Y value
     */
    float GetOriginY() const { return m_originY; }

    /**
     * @brief Set origin Y value (for animation)
     * @param originY Origin Y value
     */
    void SetOriginY(float originY) { m_originY = originY; }

    /**
     * @brief Get difference X value (for animation)
     * @return Difference X value
     */
    float GetDiffX() const { return m_diffX; }

    /**
     * @brief Set difference X value (for animation)
     * @param diffX Difference X value
     */
    void SetDiffX(float diffX) { m_diffX = diffX; }

    /**
     * @brief Get difference Y value (for animation)
     * @return Difference Y value
     */
    float GetDiffY() const { return m_diffY; }

    /**
     * @brief Set difference Y value (for animation)
     * @param diffY Difference Y value
     */
    void SetDiffY(float diffY) { m_diffY = diffY; }

    /**
     * @brief Initialize point animation
     */
    void InitializeAnimation();

    /**
     * @brief Update point for animation
     * @param scale Animation scale factor
     */
    void Update(float scale);

    /**
     * @brief Finish point animation
     */
    void Finish();

private:
    float m_x;
    float m_y;
    SStringT m_label;
    COLORREF m_color;
    BOOL m_bHasCustomColor;

    // Animation support
    float m_originX;
    float m_originY;
    float m_diffX;
    float m_diffY;
};

SNSEND

#endif // __SCHARTDATA_H__
