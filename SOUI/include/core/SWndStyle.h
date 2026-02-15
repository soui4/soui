/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SwndStyle.h
 * @brief      SOUI窗口风格管理
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Description: This file defines the SwndStyle class, which manages the style attributes of SOUI windows.
 */

#ifndef __SWNDSTYLE__H__
#define __SWNDSTYLE__H__

#include "res.mgr/SDpiAwareFont.h"

SNSBEGIN

/**
 * @class SwndStyle
 * @brief Manages the style attributes of SOUI windows.
 *
 * This class provides functionality to manage various style attributes of SOUI windows, including
 * text alignment, background and border colors, cursor, text colors, fonts, margins, and padding.
 */
class SOUI_EXP SwndStyle {
  public:
    /**
     * @brief Alignment constants for horizontal alignment.
     */
    enum
    {
        Align_Left = 0x000U,   // valign = top
        Align_Center = 0x100U, // valign = middle
        Align_Right = 0x200U,  // valign = bottom
        Align_MaskX = 0x300U,
    };

    /**
     * @brief Alignment constants for vertical alignment.
     */
    enum
    {
        VAlign_Top = 0x0000U,    // valign = top
        VAlign_Middle = 0x1000U, // valign = middle
        VAlign_Bottom = 0x2000U, // valign = bottom
        Align_MaskY = 0x3000U,
    };

  public:
    /**
     * @brief Constructor.
     */
    SwndStyle();

    COLORREF m_crBg;     /**< Background color */
    COLORREF m_crBorder; /**< Border color */

    SStringT m_strCursor; /**< Cursor name */

    DWORD m_bDotted : 1;          /**< Support ellipsis for text */
    DWORD m_bTrackMouseEvent : 1; /**< Monitor mouse enter and leave messages */
    DWORD m_bVideoCanvas : 1;     /**< Video rendering canvas */

    /**
     * @brief Retrieves the text alignment.
     * @return Text alignment flags.
     */
    UINT GetTextAlign() const;

    /**
     * @brief Retrieves the alignment (horizontal and vertical).
     * @return Alignment flags.
     */
    UINT GetAlign() const;

    /**
     * @brief Retrieves the number of states.
     * @return Number of states.
     */
    int GetStates() const;

    /**
     * @brief Retrieves the text color for a specific state.
     * @param iState State index.
     * @return Text color for the specified state.
     */
    COLORREF GetTextColor(int iState) const;

    /**
     * @brief Retrieves the text font for a specific state.
     * @param iState State index.
     * @return Text font for the specified state.
     */
    IFontPtr GetTextFont(int iState) const;

    /**
     * @brief Retrieves the margin rectangle.
     * @return Margin rectangle.
     */
    CRect GetMargin() const;

    /**
     * @brief Retrieves the padding rectangle.
     * @return Padding rectangle.
     */
    CRect GetPadding() const;

    /**
     * @brief Sets the text color for a specific state.
     * @param iState State index.
     * @param cr Text color to set.
     */
    void SetTextColor(int iState, COLORREF cr)
    {
        m_crText[iState] = cr;
    }

    /**
     * @brief Sets the scale factor.
     * @param nScale Scale factor to set.
     */
    void SetScale(int nScale);

    /**
     * @brief Retrieves the scale factor.
     * @return Scale factor.
     */
    int GetScale() const;

    /**
     * @brief Sets the horizontal alignment.
     * @param uAlign Horizontal alignment flags.
     */
    void SetAlign(UINT uAlign);

    /**
     * @brief Sets the vertical alignment.
     * @param uAlign Vertical alignment flags.
     */
    void SetVAlign(UINT uAlign);

    /**
     * @brief Retrieves the multi-line flag.
     * @return TRUE if multi-lines are enabled, FALSE otherwise.
     */
    BOOL GetMultiLines() const
    {
        return m_bMultiLines;
    }

    /**
     * @brief Sets the multi-line flag.
     * @param bMultiLines TRUE to enable multi-lines, FALSE to disable.
     */
    void SetMultiLines(BOOL bMultiLines)
    {
        m_bMultiLines = bMultiLines;
    }

    /**
     * @brief Updates the font based on the current scale.
     */
    void UpdateFont();

  protected:
    SLayoutSize m_rcMargin[4]; /**< 4-week non-client area size */
    SLayoutSize m_rcInset[4];  /**< Text area 4-directional inner padding */
    BOOL m_bMultiLines;        /**< Multiple lines flag */
    UINT m_nTextAlign;         /**< Text alignment */
    UINT m_uAlign, m_uVAlign;  /**< Horizontal and vertical alignment */
    COLORREF m_crText[4];      /**< Text color for 4 states */
    SDpiAwareFont m_ftText[4]; /**< Text font for 4 states */

    int m_nScale;

    /**
     * @brief Handles the 'padding' attribute.
     * @param strValue Attribute value.
     * @param bLoading TRUE if loading, FALSE otherwise.
     * @return HRESULT indicating success or failure.
     */
    HRESULT OnAttrPadding(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief Handles the 'margin' attribute.
     * @param strValue Attribute value.
     * @param bLoading TRUE if loading, FALSE otherwise.
     * @return HRESULT indicating success or failure.
     */
    HRESULT OnAttrMargin(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief Handles the 'margin-x' attribute.
     * @param strValue Attribute value.
     * @param bLoading TRUE if loading, FALSE otherwise.
     * @return HRESULT indicating success or failure.
     */
    HRESULT OnAttrMarginX(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief Handles the 'margin-y' attribute.
     * @param strValue Attribute value.
     * @param bLoading TRUE if loading, FALSE otherwise.
     * @return HRESULT indicating success or failure.
     */
    HRESULT OnAttrMarginY(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief Parses a string into an array of layout sizes.
     * @param strValue String containing layout sizes.
     * @param layoutSizes Array to store parsed layout sizes.
     */
    void _ParseLayoutSize4(const SStringW &strValue, SLayoutSize layoutSizes[]);

    SOUI_ATTRS_BEGIN()
        ATTR_HEX(L"textMode", m_nTextAlign, TRUE)
        ATTR_BOOL(L"multiLines", m_bMultiLines, TRUE)
        ATTR_ENUM_BEGIN(L"align", UINT, TRUE)
            ATTR_ENUM_VALUE(L"left", Align_Left)
            ATTR_ENUM_VALUE(L"center", Align_Center)
            ATTR_ENUM_VALUE(L"right", Align_Right)
        ATTR_ENUM_END(m_uAlign)
        ATTR_ENUM_BEGIN(L"valign", UINT, TRUE)
            ATTR_ENUM_VALUE(L"top", VAlign_Top)
            ATTR_ENUM_VALUE(L"middle", VAlign_Middle)
            ATTR_ENUM_VALUE(L"bottom", VAlign_Bottom)
        ATTR_ENUM_END(m_uVAlign)

        ATTR_COLOR(L"colorBkgnd", m_crBg, TRUE)
        ATTR_COLOR(L"colorBorder", m_crBorder, TRUE)

        ATTR_FONT(L"font", m_ftText[0], TRUE)
        ATTR_FONT(L"fontHover", m_ftText[1], TRUE)
        ATTR_FONT(L"fontPush", m_ftText[2], TRUE)
        ATTR_FONT(L"fontDisable", m_ftText[3], TRUE)

        ATTR_COLOR(L"colorText", m_crText[0], TRUE)
        ATTR_COLOR(L"colorTextHover", m_crText[1], TRUE)
        ATTR_COLOR(L"colorTextPush", m_crText[2], TRUE)
        ATTR_COLOR(L"colorTextDisable", m_crText[3], TRUE)

        ATTR_CUSTOM(L"margin-x", OnAttrMarginX)
        ATTR_CUSTOM(L"margin-y", OnAttrMarginY)
        ATTR_CUSTOM(L"margin", OnAttrMargin)
        ATTR_CUSTOM(L"inset", OnAttrPadding)
        ATTR_CUSTOM(L"padding", OnAttrPadding)
        ATTR_LAYOUTSIZE(L"padding_left", m_rcInset[0], TRUE)
        ATTR_LAYOUTSIZE(L"padding_top", m_rcInset[1], TRUE)
        ATTR_LAYOUTSIZE(L"padding_right", m_rcInset[2], TRUE)
        ATTR_LAYOUTSIZE(L"padding_bottom", m_rcInset[3], TRUE)
        ATTR_STRINGT(L"cursor", m_strCursor, FALSE)
        ATTR_INT(L"dotted", m_bDotted, FALSE)
    SOUI_ATTRS_BREAK()
};

SNSEND
#endif // __SWNDSTYLE__H__