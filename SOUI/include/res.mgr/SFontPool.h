/**
 * @file SFontPool.h
 * @brief SOUI Font Management Module
 * @version v1.0
 * @author SOUI group
 * @date 2014/08/02
 *
 * @details This module manages and retrieves font resources.
 */

#ifndef __SFONTPOOL__H__
#define __SFONTPOOL__H__

#include <core/SSingletonMap.h>
#include <interface/SRender-i.h>
#include <res.mgr/SFontInfo.h>
#include <helper/obj-ref-impl.hpp>

#define FF_DEFAULTFONT L""

SNSBEGIN

/**
 * @class FontKey
 * @brief Key for a FONT
 *
 * @details Used to implement a font map, uniquely identifying a font.
 */
typedef FontInfo FontKey;

/**
 * @class CElementTraits<FontInfo>
 * @brief Hash and comparison template for FontInfo
 *
 * @details Used to implement a font map, providing hash and comparison functions.
 */
template <>
class CElementTraits<FontInfo> : public CElementTraitsBase<FontInfo> {
  public:
    /**
     * @brief Calculate the hash value of a FontInfo object
     * @param fontKey FontInfo object
     * @return Hash value
     */
    static ULONG Hash(INARGTYPE fontKey)
    {
        ULONG uRet = SNS::CElementTraits<SStringW>::Hash(fontKey.strFaceName);
        uRet = (uRet << 5) + SNS::CElementTraits<SStringW>::Hash(fontKey.strPropEx);
        uRet = (uRet << 5) + (UINT)fontKey.style.syle + 1 + fontKey.scale;
        return uRet;
    }

    /**
     * @brief Compare two FontInfo objects for equality
     * @param element1 First FontInfo object
     * @param element2 Second FontInfo object
     * @return True if equal, false otherwise
     */
    static bool CompareElements(INARGTYPE element1, INARGTYPE element2)
    {
        return element1.strFaceName == element2.strFaceName && element1.strPropEx == element2.strPropEx && element1.style.syle == element2.style.syle && element1.scale == element2.scale;
    }

    /**
     * @brief Compare two FontInfo objects in order
     * @param element1 First FontInfo object
     * @param element2 Second FontInfo object
     * @return Comparison result, 0 if equal, negative if element1 < element2, positive if element1 > element2
     */
    static int CompareElementsOrdered(INARGTYPE element1, INARGTYPE element2)
    {
        int nRet = element1.strFaceName.Compare(element2.strFaceName);
        if (nRet == 0)
            nRet = element1.strPropEx.Compare(element2.strPropEx);
        if (nRet == 0)
            nRet = (int)(element1.style.syle - element2.style.syle);
        if (nRet == 0)
            nRet = (int)(element1.scale - element2.scale);

        return nRet;
    }
};

typedef IFontS *IFontPtr;
typedef BOOL (*FunFontCheck)(const SStringW &strFontName);

/**
 * @class SFontPool
 * @brief Font pool management class
 *
 * @details Manages and retrieves font resources, supporting font caching and reuse.
 */
class SOUI_EXP SFontPool : public SCmnMap<IFontPtr, FontInfo> {
  public:
    /**
     * @brief Set the callback function for checking fonts
     * @param fontCheck Callback function for checking fonts
     */
    static void SetFontChecker(FunFontCheck fontCheck);

    /**
     * @brief Check if a font is valid
     * @param strFontName Font name
     * @return TRUE if valid, FALSE if invalid
     */
    static BOOL CheckFont(const SStringW &strFontName);

    /**
     * @brief Convert font description to FontInfo
     * @param strFontInfo Font description string
     * @param defFontInfo Default FontInfo
     * @return FontInfo object
     */
    static FontInfo FontInfoFromString(const SStringW &strFontInfo, const FontInfo &defFontInfo);

    /**
     * @brief Convert FontInfo to font description
     * @param fi FontInfo object
     * @return Font description string
     */
    static SStringW FontInfoToString(const FontInfo &fi);

    /**
     * @brief Get IFontPtr corresponding to the specified description string
     * @param strFont Font description string
     * @param scale Scale factor
     * @return IFontPtr Font object pointer
     *
     * @details Description string format: face:宋体,bold:0,italic:1,underline:1,strike:1,adding:10
     */
    static IFontPtr GetFont(const SStringW &strFont, int scale);

    /**
     * @brief Set default font
     * @param strFontInfo Default font description string
     */
    static void SetDefFontInfo(const SStringW &strFontInfo);

    /**
     * @brief Get default font information
     * @return FontInfo Default font information
     */
    static FontInfo GetDefFontInfo();

  protected:
    /**
     * @brief Callback function when FontInfo is removed
     * @param obj Removed IFontPtr object
     */
    static void OnKeyRemoved(const IFontPtr &obj);

  protected:
    /**
     * @brief Constructor
     * @param fac Render factory object pointer
     */
    SFontPool(IRenderFactory *fac);

  protected:
    /**
     * @brief Get constant reference to default font information
     * @return FontInfo Constant reference to default font information
     */
    const FontInfo &_GetDefFontInfo() const;

    /**
     * @brief Set render factory object
     * @param fac Render factory object pointer
     */
    void SetRenderFactory(IRenderFactory *fac);

    /**
     * @brief Get IFontPtr corresponding to the specified description string
     * @param strFont Font description string
     * @param scale Scale factor
     * @return IFontPtr Font object pointer
     */
    IFontPtr _GetFont(const SStringW &strFont, int scale);

    /**
     * @brief Set default font description string
     * @param strFontInfo Default font description string
     */
    void _SetDefFontInfo(const SStringW &strFontInfo);

    /**
     * @brief Create font object corresponding to FontInfo
     * @param fontInfo FontInfo object
     * @return IFontPtr Font object pointer
     */
    IFontPtr _CreateFont(const FontInfo &fontInfo);

    /**
     * @brief Set default font information
     * @param fontInfo FontInfo object
     */
    void _SetDefFontInfo(const FontInfo &fontInfo);

    SAutoRefPtr<IRenderFactory> m_RenderFactory; /**< Render factory object pointer */
    FontInfo m_defFontInfo;                      /**< Default font information */

    static FunFontCheck s_funFontCheck; /**< Font check callback function */
};

SNSEND

#endif // __SFONTPOOL__H__