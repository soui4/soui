#ifndef __SLAYOUTSIZE__H__
#define __SLAYOUTSIZE__H__

#include <interface/slayout-i.h>
#include <string/tstring.h>
SNSBEGIN

namespace LayoutSize_Style
{
static const LPCWSTR kSize_WrapContent = L"wrapContent";
static const LPCWSTR kSize_MatchParent = L"matchParent";
static const LPCWSTR kSize_FillParent = L"full";
} // namespace LayoutSize_Style

namespace LayoutSize_Unit
{
static const LPCWSTR kUnit_Px = L"px";
static const LPCWSTR kUnit_Dp = L"dp";
static const LPCWSTR kUnit_Dip = L"dip";
static const LPCWSTR kUnit_Sp = L"sp";
} // namespace LayoutSize_Unit

/**
 * @class SLayoutSize
 * @brief Layout size class
 */
class SOUI_EXP SLayoutSize : public LAYOUTSIZE {
  public:
    /**
     * @brief Default constructor
     * @param fSize Size value (default 0.0f)
     */
    SLayoutSize(float fSize = 0.0f);

    /**
     * @brief Constructor
     * @param fSize Size value
     * @param unit Size unit
     */
    SLayoutSize(float fSize, Unit unit);

    SLayoutSize(const SLayoutSize &src)
    {
        *this = src;
    }
    SLayoutSize(const LAYOUTSIZE &src)
    {
        *this = src;
    }

    /**
     * @brief Set to wrap content size
     */
    void setWrapContent();

    /**
     * @brief Check whether it is wrap content size
     * @return Returns TRUE if wrap content size, otherwise returns FALSE
     */
    bool isWrapContent() const;

    /**
     * @brief Set to match parent container size
     */
    void setMatchParent();

    /**
     * @brief Check whether it matches parent container size
     * @return Returns TRUE if matching parent container size, otherwise returns FALSE
     */
    bool isMatchParent() const;

    /**
     * @brief Set the specified size
     * @param fSize Size value
     * @param unit Size unit
     */
    void setSize(float fSize, Unit unit);

    /**
     * @brief Check whether it is the specified size
     * @return Returns TRUE if the specified size, otherwise returns FALSE
     */
    bool isSpecifiedSize() const;

    /**
     * @brief Set to invalid size
     */
    void setInvalid();

    /**
     * @brief Check whether it is valid size
     * @return Returns TRUE if valid size, otherwise returns FALSE
     */
    bool isValid() const;

    /**
     * @brief Convert the size to a pixel value
     * @param scale Scale factor
     * @return Pixel value
     */
    int toPixelSize(int scale) const;

    /**
     * @brief Convert the size to string representation
     * @return String representation
     */
    SStringW toString() const;

    /**
     * @brief Check whether the size is zero
     * @return Returns TRUE if the size is zero, otherwise returns FALSE
     */
    bool isZero() const;

    /**
     * @brief Parse the size from a string
     * @param strSize Size string
     */
    void parseString(const SStringW &strSize);

    /**
     * @brief Assignment operator overload
     * @param src Source size object
     * @return Reference to the current size object
     */
    SLayoutSize &operator=(const SLayoutSize &src);
    SLayoutSize &operator=(const LAYOUTSIZE &src);

    /**
     * @brief Compare whether size values are equal
     * @param value Size value
     * @return Returns TRUE if size values are equal, otherwise returns FALSE
     */
    bool valueEqual(float value);

    /**
     * @brief Create a size object from a string
     * @param strSize Size string
     * @return Size object
     */
    static SLayoutSize fromString(const SStringW &strSize);

    /**
     * @brief Compare whether two floating-point numbers are equal
     * @param a First floating-point number
     * @param b Second floating-point number
     * @return Returns TRUE if equal, otherwise returns FALSE
     */
    static bool fequal(float a, float b);

    /**
     * @brief Parse the unit from a string
     * @param strUnit Unit string
     * @return Unit enum value
     */
    static Unit unitFromString(const SStringW &strUnit);

    static Unit defUnit; /**< Default unit */
};

SNSEND

#endif /**< __SLAYOUTSIZE__H__ */