/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SSkin.h
 * @brief      Skin Classes for SOUI
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Description: Provides various skin classes for rendering different UI elements in SOUI.
 */

#ifndef __SSKIN__H__
#define __SSKIN__H__

#include <core/SSkinObjBase.h>
#include <layout/SLayoutSize.h>
#include <helper/SplitString.h>
#include <matrix/SPoint.h>
#include <sobject/Sobject.hpp>

SNSBEGIN

/**
 * @class      SSkinImgList
 * @brief      Image List Skin
 *
 * Description: Represents a skin that uses an image list to display different states.
 */
class SOUI_EXP SSkinImgList : public SSkinObjBase {
    DEF_SOBJECT(SSkinObjBase, L"imglist")

  public:
    /**
     * @brief Constructor for SSkinImgList.
     */
    SSkinImgList();

    /**
     * @brief Destructor for SSkinImgList.
     */
    virtual ~SSkinImgList();

    /**
     * @brief Gets the size of the skin.
     * @return Size of the skin.
     */
    STDMETHOD_(SIZE, GetSkinSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the number of states in the skin.
     * @return Number of states.
     */
    STDMETHOD_(int, GetStates)(THIS) SCONST OVERRIDE;

    /**
     * @brief Handles colorization of the skin.
     * @param cr Color reference for colorization.
     */
    STDMETHOD_(void, OnColorize)(THIS_ COLORREF cr) OVERRIDE;

    /**
     * @brief Sets the number of states in the skin.
     * @param nStates Number of states.
     */
    virtual void SetStates(int nStates)
    {
        m_nStates = nStates;
    }

    /**
     * @brief Sets the image for the skin.
     * @param pImg Pointer to the bitmap source.
     * @return TRUE if successful, FALSE otherwise.
     */
    virtual bool SetImage(IBitmapS *pImg);

    /**
     * @brief Gets the image used by the skin.
     * @return Pointer to the bitmap source.
     */
    virtual IBitmapS *GetImage() const;

    /**
     * @brief Sets whether the image should be tiled.
     * @param bTile TRUE to tile the image, FALSE otherwise.
     */
    virtual void SetTile(BOOL bTile)
    {
        m_bTile = bTile;
    }

    /**
     * @brief Checks if the image is tiled.
     * @return TRUE if tiled, FALSE otherwise.
     */
    virtual BOOL IsTile() const
    {
        return m_bTile;
    }

    /**
     * @brief Sets whether the images are arranged vertically.
     * @param bVertical TRUE for vertical arrangement, FALSE for horizontal.
     */
    virtual void SetVertical(BOOL bVertical)
    {
        m_bVertical = bVertical;
    }

    /**
     * @brief Checks if the images are arranged vertically.
     * @return TRUE if vertical, FALSE otherwise.
     */
    virtual BOOL IsVertical() const
    {
        return m_bVertical;
    }

  protected:
    /**
     * @brief Gets the expand mode for the skin.
     * @return Expand mode.
     */
    virtual UINT GetExpandMode() const;

  protected:
    /**
     * @brief Called when initialization is finished.
     * @param pNode Pointer to the XML node.
     */
    STDMETHOD_(void, OnInitFinished)(THIS_ IXmlNode *pNode) OVERRIDE;

    /**
     * @brief Draws the skin by index.
     * @param pRT Pointer to the render target.
     * @param rcDraw Rectangle to draw in.
     * @param iState State index.
     * @param byAlpha Alpha value for transparency.
     */
    void _DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const override;

    /**
     * @brief Scales the skin.
     * @param skinObj Pointer to the skin object.
     * @param nScale Scale factor.
     */
    void _Scale(ISkinObj *skinObj, int nScale) override;

  protected:
    int m_nStates;                     // Number of skin states
    BOOL m_bTile;                      // Flag to indicate if the image is tiled
    BOOL m_bAutoFit;                   // Flag to indicate if the image should auto-fit
    BOOL m_bVertical;                  // Flag to indicate if images are arranged vertically
    SAutoRefPtr<IBitmapS> m_imgBackup; // Backup of the image before colorization
    FilterLevel m_filterLevel;         // Filter level for image scaling

  protected:
    mutable SAutoRefPtr<IBitmapS> m_pImg; // Pointer to the bitmap source
    mutable SStringW m_strSrc;            // Source string for the image
    BOOL m_bLazyLoad;                     // Flag to indicate lazy loading

  protected:
    /**
     * @brief Handles the 'src' attribute.
     * @param value Source string.
     * @param bLoading TRUE if loading, FALSE otherwise.
     * @return Result of the attribute handling.
     */
    HRESULT OnAttrSrc(const SStringW &value, BOOL bLoading);

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"src", OnAttrSrc)
        ATTR_BOOL(L"tile", m_bTile, FALSE)         // Draw whether to tile, 0--stretch (default), other--tile
        ATTR_BOOL(L"autoFit", m_bAutoFit, FALSE)   // autoFit is 0, do not automatically fit to drawing area
        ATTR_BOOL(L"vertical", m_bVertical, FALSE) // Sub-images are vertically arranged, 0--horizontal (default), other--vertical
        ATTR_INT(L"states", m_nStates, FALSE)      // Number of sub-images, default is 1
        ATTR_BOOL(L"lazyLoad", m_bLazyLoad, FALSE)
        ATTR_ENUM_BEGIN(L"filterLevel", FilterLevel, FALSE)
            ATTR_ENUM_VALUE(L"none", kNone_FilterLevel)
            ATTR_ENUM_VALUE(L"low", kLow_FilterLevel)
            ATTR_ENUM_VALUE(L"medium", kMedium_FilterLevel)
            ATTR_ENUM_VALUE(L"high", kHigh_FilterLevel)
        ATTR_ENUM_END(m_filterLevel)
    SOUI_ATTRS_END()
};

/**
 * @class      SSkinImgCenter
 * @brief      Centered Image Skin
 *
 * Description: Represents a skin that centers an image within the drawing area.
 */
class SOUI_EXP SSkinImgCenter : public SSkinImgList {
    DEF_SOBJECT(SSkinImgList, L"imgCenter")

  public:
    /**
     * @brief Constructor for SSkinImgCenter.
     */
    SSkinImgCenter()
    {
    }

  protected:
    /**
     * @brief Draws the skin by index.
     * @param pRT Pointer to the render target.
     * @param rcDraw Rectangle to draw in.
     * @param iState State index.
     * @param byAlpha Alpha value for transparency.
     */
    void _DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const override;
};

/**
 * @class      SSkinImgFrame
 * @brief      Image Frame Skin
 *
 * Description: Represents a skin that uses an image frame with margins.
 */
class SOUI_EXP SSkinImgFrame : public SSkinImgList {
    DEF_SOBJECT(SSkinImgList, L"imgframe")

  public:
    /**
     * @brief Constructor for SSkinImgFrame.
     */
    SSkinImgFrame();

    /**
     * @brief Sets the margins for the image frame.
     * @param rcMargin Margin rectangle.
     */
    void SetMargin(const CRect rcMargin)
    {
        m_rcMargin = rcMargin;
    }

    /**
     * @brief Gets the margins for the image frame.
     * @return Margin rectangle.
     */
    CRect GetMargin()
    {
        return m_rcMargin;
    }

  protected:
    /**
     * @brief Gets the expand mode for the skin.
     * @return Expand mode.
     */
    UINT GetExpandMode() const override;

    /**
     * @brief Scales the skin.
     * @param skinObj Pointer to the skin object.
     * @param nScale Scale factor.
     */
    void _Scale(ISkinObj *skinObj, int nScale) override;

    /**
     * @brief Draws the skin by index.
     * @param pRT Pointer to the render target.
     * @param rcDraw Rectangle to draw in.
     * @param iState State index.
     * @param byAlpha Alpha value for transparency.
     */
    void _DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const override;

  protected:
    CRect m_rcMargin; // Margin rectangle for the image frame

    SOUI_ATTRS_BEGIN()
        ATTR_RECT(L"margin", m_rcMargin, FALSE)                          // Nine-grid margins
        ATTR_INT(L"left", m_rcMargin.left, FALSE)                        // Left margin of the nine-grid
        ATTR_INT(L"top", m_rcMargin.top, FALSE)                          // Top margin of the nine-grid
        ATTR_INT(L"right", m_rcMargin.right, FALSE)                      // Right margin of the nine-grid
        ATTR_INT(L"bottom", m_rcMargin.bottom, FALSE)                    // Bottom margin of the nine-grid
        ATTR_INT(L"margin-x", m_rcMargin.left = m_rcMargin.right, FALSE) // Left and right margins of the nine-grid
        ATTR_INT(L"margin-y", m_rcMargin.top = m_rcMargin.bottom, FALSE) // Top and bottom margins of the nine-grid
        ATTR_MARGIN(L"margin2", m_rcMargin, FALSE)
    SOUI_ATTRS_END()
};

/**
 * @class      SSkinImgFrame2
 * @brief      Enhanced Image Frame Skin
 *
 * Description: Represents an enhanced image frame skin that supports loading Android .9 patches.
 */
class SOUI_EXP SSkinImgFrame2 : public SSkinImgFrame {
    DEF_SOBJECT(SSkinImgFrame, L"imgframe2")

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"src", OnAttrSrc)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Handles the 'src' attribute.
     * @param strValue Source string.
     * @param bLoading TRUE if loading, FALSE otherwise.
     * @return Result of the attribute handling.
     */
    HRESULT OnAttrSrc(const SStringW &strValue, BOOL bLoading);
};

/**
 * @class      SSkinButton
 * @brief      Button Skin
 *
 * Description: Represents a skin for buttons with different states and colors.
 */
class SOUI_EXP SSkinButton : public SSkinObjBase {
    DEF_SOBJECT(SSkinObjBase, L"button")

    enum
    {
        ST_NORMAL = 0,
        ST_HOVER,
        ST_PUSHDOWN,
        ST_DISABLE,
    };

    struct BTNCOLORS
    {
        COLORREF m_crBorder[4];
        COLORREF m_crUp[4];
        COLORREF m_crDown[4];
    };

  public:
    /**
     * @brief Constructor for SSkinButton.
     */
    SSkinButton();

    /**
     * @brief Gets the number of states in the skin.
     * @return Number of states.
     */
    STDMETHOD_(int, GetStates)(THIS) SCONST OVERRIDE;

    /**
     * @brief Scales the skin.
     * @param nScale Scale factor.
     * @return Pointer to the scaled skin object.
     */
    STDMETHOD_(ISkinObj *, Scale)(THIS_ int nScale) OVERRIDE;

    /**
     * @brief Handles colorization of the skin.
     * @param cr Color reference for colorization.
     */
    STDMETHOD_(void, OnColorize)(THIS_ COLORREF cr) OVERRIDE;

    /**
     * @brief Sets the colors for different button states.
     * @param crUp Colors for the up state.
     * @param crDown Colors for the down state.
     * @param crBorder Colors for the border.
     */
    void SetColors(COLORREF crUp[4], COLORREF crDown[4], COLORREF crBorder[4]);

  protected:
    /**
     * @brief Draws the skin by index.
     * @param pRT Pointer to the render target.
     * @param rcDraw Rectangle to draw in.
     * @param iState State index.
     * @param byAlpha Alpha value for transparency.
     */
    void _DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const override;

  protected:
    BTNCOLORS m_colors;       // Colors for different states
    BTNCOLORS m_colorsBackup; // Backup of colors before colorization

    int m_nCornerRadius;    // Corner radius
    float m_fCornerPercent; // Corner percentage (0.5 for semi-circle)

    SOUI_ATTRS_BEGIN()
        ATTR_COLOR(L"colorBorder", m_colors.m_crBorder[0], TRUE)             // Normal border color
        ATTR_COLOR(L"colorBorderHover", m_colors.m_crBorder[1], TRUE)        // Hover border color
        ATTR_COLOR(L"colorBorderPush", m_colors.m_crBorder[2], TRUE)         // Pushed border color
        ATTR_COLOR(L"colorBorderDisable", m_colors.m_crBorder[3], TRUE)      // Disabled border color
        ATTR_COLOR(L"colorUp", m_colors.m_crUp[ST_NORMAL], TRUE)             // Normal up color
        ATTR_COLOR(L"colorDown", m_colors.m_crDown[ST_NORMAL], TRUE)         // Normal down color
        ATTR_COLOR(L"colorUpHover", m_colors.m_crUp[ST_HOVER], TRUE)         // Hover up color
        ATTR_COLOR(L"colorDownHover", m_colors.m_crDown[ST_HOVER], TRUE)     // Hover down color
        ATTR_COLOR(L"colorUpPush", m_colors.m_crUp[ST_PUSHDOWN], TRUE)       // Pushed up color
        ATTR_COLOR(L"colorDownPush", m_colors.m_crDown[ST_PUSHDOWN], TRUE)   // Pushed down color
        ATTR_COLOR(L"colorUpDisable", m_colors.m_crUp[ST_DISABLE], TRUE)     // Disabled up color
        ATTR_COLOR(L"colorDownDisable", m_colors.m_crDown[ST_DISABLE], TRUE) // Disabled down color
        ATTR_INT(L"cornerRadius", m_nCornerRadius, TRUE)                     // Corner radius
        ATTR_FLOAT(L"cornerPercent", m_fCornerPercent, TRUE)                 // Corner percentage (0.5 for semi-circle)
    SOUI_ATTRS_END()
};

/**
 * @class      SSkinGradation
 * @brief      Gradient Skin
 *
 * Description: Represents a skin with a gradient fill.
 */
class SOUI_EXP SSkinGradation : public SSkinObjBase {
    DEF_SOBJECT(SSkinObjBase, L"gradation")

  public:
    /**
     * @brief Constructor for SSkinGradation.
     */
    SSkinGradation();

    /**
     * @brief Sets the starting color of the gradient.
     * @param crFrom Starting color.
     */
    void SetColorFrom(COLORREF crFrom)
    {
        m_crFrom = crFrom;
    }

    /**
     * @brief Sets the ending color of the gradient.
     * @param crTo Ending color.
     */
    void SetColorTo(COLORREF crTo)
    {
        m_crTo = crTo;
    }

    /**
     * @brief Sets whether the gradient is vertical.
     * @param bVertical TRUE for vertical, FALSE for horizontal.
     */
    void SetVertical(BOOL bVertical)
    {
        m_bVert = bVertical;
    }

  public:
    /**
     * @brief Scales the skin.
     * @param nScale Scale factor.
     * @return Pointer to the scaled skin object.
     */
    STDMETHOD_(ISkinObj *, Scale)(THIS_ int nScale) OVERRIDE;

  protected:
    /**
     * @brief Draws the skin by index.
     * @param pRT Pointer to the render target.
     * @param prcDraw Rectangle to draw in.
     * @param iState State index.
     * @param byAlpha Alpha value for transparency.
     */
    void _DrawByIndex(IRenderTarget *pRT, LPCRECT prcDraw, int iState, BYTE byAlpha) const override;

  protected:
    COLORREF m_crFrom; // Starting color of the gradient
    COLORREF m_crTo;   // Ending color of the gradient
    BOOL m_bVert;      // Flag to indicate if the gradient is vertical

    SOUI_ATTRS_BEGIN()
        ATTR_COLOR(L"colorFrom", m_crFrom, TRUE) // Starting color of the gradient
        ATTR_COLOR(L"colorTo", m_crTo, TRUE)     // Ending color of the gradient
        ATTR_BOOL(L"vertical", m_bVert, TRUE)    // Gradient direction, 0--horizontal, 1--vertical (default)
    SOUI_ATTRS_END()
};

/**
 * @class      SGradientDesc
 * @brief      Gradient Descriptor
 *
 * Description: Describes a gradient for use in skins.
 */
class SOUI_EXP SGradientDesc {
  public:
    /**
     * @brief Constructor for SGradientDesc.
     */
    SGradientDesc();

  protected:
    SAutoRefPtr<IGradient> m_gradient; // Pointer to the gradient object
    SLayoutSize m_radius;              // Radius for radial gradients
    float m_ratio_radius;              // Ratio radius for radial gradients
    GradientType m_type;               // Type of gradient (linear, radial, sweep)
    float m_angle;                     // Angle for linear gradients
    float m_centerX;                   // Center X for radial and sweep gradients
    float m_centerY;                   // Center Y for radial and sweep gradients
    BOOL m_bFullArc;                   // Flag to indicate full arc for sweep gradients

  public:
    /**
     * @brief Gets the gradient information.
     * @param nScale Scale factor.
     * @param wid Width of the drawing area.
     * @param hei Height of the drawing area.
     * @return Gradient information.
     */
    GradientInfo GetGradientInfo(int nScale, int wid, int hei) const;

    /**
     * @brief Gets the gradient object.
     * @return Pointer to the gradient object.
     */
    IGradient *GetGradient()
    {
        return m_gradient;
    }

  protected:
    SOUI_ATTRS_BEGIN()
        ATTR_ENUM_BEGIN(L"type", GradientType, TRUE)
            ATTR_ENUM_VALUE(L"linear", linear)
            ATTR_ENUM_VALUE(L"radial", radial)
            ATTR_ENUM_VALUE(L"sweep", sweep)
        ATTR_ENUM_END(m_type)
        ATTR_LAYOUTSIZE(L"radius", m_radius, TRUE)
        ATTR_FLOAT(L"ratio_radius", m_ratio_radius, TRUE)
        ATTR_FLOAT(L"angle", m_angle, TRUE)
        ATTR_FLOAT(L"centerX", m_centerX, TRUE)
        ATTR_FLOAT(L"centerY", m_centerY, TRUE)
        ATTR_BOOL(L"fullArc", m_bFullArc, TRUE)
        ATTR_GRADIENT(L"gradient", m_gradient, TRUE)
        ATTR_CHAIN_PTR(m_gradient, 0)
    SOUI_ATTRS_BREAK()
};

/**
 * @class      SSkinGradation2
 * @brief      Enhanced Gradient Skin
 *
 * Description: Represents an enhanced gradient skin that includes additional corner attributes.
 */
class SOUI_EXP SSkinGradation2
    : public SSkinObjBase
    , public SGradientDesc {
    DEF_SOBJECT(SSkinObjBase, L"gradation2")

  public:
    /**
     * @brief Constructor for SSkinGradation2.
     */
    SSkinGradation2();

  public:
    /**
     * @brief Scales the skin.
     * @param nScale Scale factor.
     * @return Pointer to the scaled skin object.
     */
    STDMETHOD_(ISkinObj *, Scale)(THIS_ int nScale) OVERRIDE;

    /**
     * @brief Called when initialization is finished.
     * @param xmlNode Pointer to the XML node.
     */
    STDMETHOD_(void, OnInitFinished)(THIS_ IXmlNode *xmlNode) OVERRIDE;

  protected:
    /**
     * @brief Draws the skin by index.
     * @param pRT Pointer to the render target.
     * @param prcDraw Rectangle to draw in.
     * @param iState State index.
     * @param byAlpha Alpha value for transparency.
     */
    void _DrawByIndex(IRenderTarget *pRT, LPCRECT prcDraw, int iState, BYTE byAlpha) const override;

  protected:
    SPoint m_ptCorner;         // Corner point for gradient
    SLayoutSize m_szCorner[2]; // Corner sizes

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_SPOINT(L"ratio_corners", m_ptCorner, TRUE)
        ATTR_LAYOUTSIZE2(L"corners", m_szCorner, TRUE)
        ATTR_CHAIN_CLASS(SGradientDesc)
    SOUI_ATTRS_END()
};

/**
 * @enum       SBSTATE
 * @brief      Scrollbar State Enum
 *
 * Description: Defines the states for a scrollbar.
 */
enum SBSTATE
{
    SBST_NORMAL = 0, // Normal state
    SBST_HOVER,      // Hover state
    SBST_PUSHDOWN,   // Pushed down state
    SBST_DISABLE,    // Disabled state
    SBST_INACTIVE,   // Inactive state, mainly for arrow heads
};

/**
 * @def        MAKESBSTATE(sbCode, nState1, bVertical)
 * @brief      Macro to create a scrollbar state code.
 * @param      sbCode Scrollbar code.
 * @param      nState1 State identifier.
 * @param      bVertical TRUE for vertical, FALSE for horizontal.
 */
#define MAKESBSTATE(sbCode, nState1, bVertical) MAKELONG((sbCode), MAKEWORD((nState1), (bVertical)))

/**
 * @def        SB_CORNOR
 * @brief      Constant for scrollbar corner.
 */
#define SB_CORNOR 10

/**
 * @def        SB_THUMBGRIPPER
 * @brief      Constant for scrollbar thumb gripper.
 */
#define SB_THUMBGRIPPER 11 // Scrollbar thumb gripper

/**
 * @def        THUMB_MINSIZE
 * @brief      Minimum size for the scrollbar thumb.
 */
#define THUMB_MINSIZE 18

/**
 * @class      SSkinScrollbar
 * @brief      Scrollbar Skin
 *
 * Description: Represents a skin for scrollbars.
 */
class SOUI_EXP SSkinScrollbar : public SSkinImgList {
    DEF_SOBJECT(SSkinImgList, L"scrollbar")

  public:
    /**
     * @brief Constructor for SSkinScrollbar.
     */
    SSkinScrollbar();

    /**
     * @brief Checks if the scrollbar supports displaying arrow heads.
     * @return TRUE if supported, FALSE otherwise.
     */
    virtual BOOL HasArrow() const
    {
        return TRUE;
    }

    /**
     * @brief Gets the ideal size of the scrollbar.
     * @return Ideal size of the scrollbar.
     */
    virtual int GetIdealSize() const;

  protected:
    /**
     * @brief Gets the rectangle of a specified part in the original bitmap.
     * @param nSbCode Scrollbar code.
     * @param nState State identifier.
     * @param bVertical TRUE for vertical, FALSE for horizontal.
     * @return Rectangle of the part.
     */
    virtual CRect GetPartRect(int nSbCode, int nState, BOOL bVertical) const;

  protected:
    /**
     * @brief Draws the skin by index.
     * @param pRT Pointer to the render target.
     * @param prcDraw Rectangle to draw in.
     * @param iState State index.
     * @param byAlpha Alpha value for transparency.
     */
    void _DrawByIndex(IRenderTarget *pRT, LPCRECT prcDraw, int iState, BYTE byAlpha) const override
    {
    }

    /**
     * @brief Draws the skin by state.
     * @param pRT Pointer to the render target.
     * @param prcDraw Rectangle to draw in.
     * @param dwState State identifier.
     * @param byAlpha Alpha value for transparency.
     */
    void _DrawByState(IRenderTarget *pRT, LPCRECT prcDraw, DWORD dwState, BYTE byAlpha) const override;

    /**
     * @brief Scales the skin.
     * @param skinObj Pointer to the skin object.
     * @param nScale Scale factor.
     */
    void _Scale(ISkinObj *skinObj, int nScale) override;

  protected:
    int m_nMargin;       // Margin size
    BOOL m_bHasGripper;  // Flag to indicate if the thumb has a gripper
    BOOL m_bHasInactive; // Flag to indicate if there is an inactive state for arrow heads

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"margin", m_nMargin, FALSE)           // Edge stretch size
        ATTR_INT(L"hasGripper", m_bHasGripper, FALSE)   // Thumb has gripper
        ATTR_INT(L"hasInactive", m_bHasInactive, FALSE) // Has inactive state
    SOUI_ATTRS_END()
};

//////////////////////////////////////////////////////////////////////////
/**
 * @class      SSkinColorRect
 * @brief      Color Rectangle Skin
 *
 * Description: Represents a skin for a colored rectangle with optional borders.
 */
class SOUI_EXP SSkinColorRect : public SSkinObjBase {
    DEF_SOBJECT(SSkinObjBase, L"colorrect")

  public:
    /**
     * @brief Constructor for SSkinColorRect.
     */
    SSkinColorRect();

    /**
     * @brief Destructor for SSkinColorRect.
     */
    virtual ~SSkinColorRect();

    /**
     * @brief Gets the number of states in the skin.
     * @return Number of states.
     */
    STDMETHOD_(int, GetStates)(THIS) SCONST OVERRIDE;

    /**
     * @brief Scales the skin.
     * @param nScale Scale factor.
     * @return Pointer to the scaled skin object.
     */
    STDMETHOD_(ISkinObj *, Scale)(THIS_ int nScale) OVERRIDE;

  protected:
    /**
     * @brief Draws the skin by index.
     * @param pRT Pointer to the render target.
     * @param prcDraw Rectangle to draw in.
     * @param iState State index.
     * @param byAlpha Alpha value for transparency.
     */
    void _DrawByIndex(IRenderTarget *pRT, LPCRECT prcDraw, int iState, BYTE byAlpha) const override;

  protected:
    int m_nRadius;          // Corner radius
    float m_fCornerPercent; // Corner percentage (0.5 for semi-circle)

    COLORREF m_crStates[4];  // Colors for different states
    COLORREF m_crBorders[4]; // Border colors for different states
    int m_nBorderWidth;      // Border width

    SOUI_ATTRS_BEGIN()
        ATTR_COLOR(L"normal", m_crStates[0], FALSE)           // Normal state color
        ATTR_COLOR(L"hover", m_crStates[1], FALSE)            // Hover state color
        ATTR_COLOR(L"pushdown", m_crStates[2], FALSE)         // Pushed down state color
        ATTR_COLOR(L"disable", m_crStates[3], FALSE)          // Disabled state color
        ATTR_COLOR(L"normalBorder", m_crBorders[0], FALSE)    // Normal state border color
        ATTR_COLOR(L"hoverBorder", m_crBorders[1], FALSE)     // Hover state border color
        ATTR_COLOR(L"pushdownBorder", m_crBorders[2], FALSE)  // Pushed down state border color
        ATTR_COLOR(L"disableBorder", m_crBorders[3], FALSE)   // Disabled state border color
        ATTR_INT(L"borderWidth", m_nBorderWidth, FALSE)       // Border width
        ATTR_INT(L"cornerRadius", m_nRadius, FALSE)           // Corner radius
        ATTR_FLOAT(L"cornerPercent", m_fCornerPercent, FALSE) // Corner percentage (0.5 for semi-circle)
    SOUI_ATTRS_END()
};
//////////////////////////////////////////////////////////////////////////

class SOUI_EXP SSkinShape : public SSkinObjBase {
    DEF_SOBJECT(SSkinObjBase, L"shape")
    enum Shape
    {
        rectangle,
        oval,
        ring
    };

    /**
     * @class      SShapeSolid
     * @brief      Solid Shape
     *
     * Description: Represents a solid-colored shape.
     */
    class SShapeSolid : public TObjRefImpl<SObject> {
        DEF_SOBJECT(TObjRefImpl<SObject>, L"solid")

      public:
        /**
         * @brief Constructor for SShapeSolid.
         */
        SShapeSolid()
            : m_crSolid(CR_INVALID)
        {
        }

        SOUI_ATTRS_BEGIN()
            ATTR_COLOR(L"color", m_crSolid, TRUE) // Solid color
        SOUI_ATTRS_END()

        /**
         * @brief Creates a brush for rendering.
         * @param pRT Pointer to the render target.
         * @param byAlpha Alpha value for transparency.
         * @return Pointer to the brush.
         */
        IBrushS *CreateBrush(IRenderTarget *pRT, BYTE byAlpha);

      protected:
        COLORREF m_crSolid; // Solid color
    };

    /**
     * @class      SShapeBitmap
     * @brief      Bitmap Shape
     *
     * Description: Represents a shape filled with a bitmap.
     */
    class SShapeBitmap : public TObjRefImpl<SObject> {
        DEF_SOBJECT(TObjRefImpl<SObject>, L"bitmap")

      public:
        /**
         * @brief Constructor for SShapeBitmap.
         */
        SShapeBitmap()
            : m_tileX(kRepeat_TileMode)
            , m_tileY(kRepeat_TileMode)
        {
        }

        SOUI_ATTRS_BEGIN()
            ATTR_IMAGEAUTOREF(L"src", m_pImg, TRUE) // Bitmap source
            ATTR_ENUM_BEGIN(L"tileX", TileMode, TRUE)
                ATTR_ENUM_VALUE(L"clamp", kClamp_TileMode)
                ATTR_ENUM_VALUE(L"repeat", kRepeat_TileMode)
                ATTR_ENUM_VALUE(L"mirror", kMirror_TileMode)
            ATTR_ENUM_END(m_tileX)
            ATTR_ENUM_BEGIN(L"tileY", TileMode, TRUE)
                ATTR_ENUM_VALUE(L"clamp", kClamp_TileMode)
                ATTR_ENUM_VALUE(L"repeat", kRepeat_TileMode)
                ATTR_ENUM_VALUE(L"mirror", kMirror_TileMode)
            ATTR_ENUM_END(m_tileY)
        SOUI_ATTRS_END()

        /**
         * @brief Creates a brush for rendering.
         * @param pRT Pointer to the render target.
         * @param byAlpha Alpha value for transparency.
         * @return Pointer to the brush.
         */
        IBrushS *CreateBrush(IRenderTarget *pRT, BYTE byAlpha);

      protected:
        SAutoRefPtr<IBitmapS> m_pImg; // Bitmap source
        TileMode m_tileX, m_tileY;    // Tiling modes for X and Y
    };

    /**
     * @class      SGradientBrush
     * @brief      Gradient Brush
     *
     * Description: Represents a brush with a gradient fill.
     */
    class SGradientBrush
        : public TObjRefImpl<SObject>
        , public SGradientDesc {
        DEF_SOBJECT(TObjRefImpl<SObject>, L"gradient")

      public:
        /**
         * @brief Constructor for SGradientBrush.
         */
        SGradientBrush()
        {
        }

        /**
         * @brief Creates a brush for rendering.
         * @param pRT Pointer to the render target.
         * @param nScale Scale factor.
         * @param byAlpha Alpha value for transparency.
         * @param wid Width of the drawing area.
         * @param hei Height of the drawing area.
         * @return Pointer to the brush.
         */
        IBrushS *CreateBrush(IRenderTarget *pRT, int nScale, BYTE byAlpha, int wid, int hei) const;

        /**
         * @brief Called when initialization is finished.
         * @param xmlNode Pointer to the XML node.
         */
        STDMETHOD_(void, OnInitFinished)(THIS_ IXmlNode *xmlNode) override;

      public:
        SOUI_ATTRS_BEGIN()
            ATTR_CHAIN_CLASS(SGradientDesc)
        SOUI_ATTRS_END()
    };

    /**
     * @class      SStroke
     * @brief      Stroke Shape
     *
     * Description: Represents a stroke (outline) for shapes.
     */
    class SStroke : public TObjRefImpl<SObject> {
        DEF_SOBJECT(TObjRefImpl<SObject>, L"stroke")

      public:
        /**
         * @brief Constructor for SStroke.
         */
        SStroke();

        SOUI_ATTRS_BEGIN()
            ATTR_LAYOUTSIZE(L"width", m_width, TRUE) // Stroke width
            ATTR_COLOR(L"color", m_color, TRUE)      // Stroke color
            ATTR_ENUM_BEGIN(L"style", int, TRUE)
                ATTR_ENUM_VALUE(L"solid", PS_SOLID)
                ATTR_ENUM_VALUE(L"dash", PS_DASH)
                ATTR_ENUM_VALUE(L"dashDot", PS_DASHDOT)
                ATTR_ENUM_VALUE(L"dashDotDot", PS_DASHDOTDOT)
            ATTR_ENUM_END(m_style)
            ATTR_ENUM_BEGIN(L"endStyle", int, TRUE)
                ATTR_ENUM_VALUE(L"flat", PS_ENDCAP_FLAT)
                ATTR_ENUM_VALUE(L"round", PS_ENDCAP_ROUND)
                ATTR_ENUM_VALUE(L"square", PS_ENDCAP_SQUARE)
            ATTR_ENUM_END(m_endStyle)
            ATTR_ENUM_BEGIN(L"joinStyle", int, TRUE)
                ATTR_ENUM_VALUE(L"round", PS_JOIN_ROUND)
                ATTR_ENUM_VALUE(L"bevel", PS_JOIN_BEVEL)
                ATTR_ENUM_VALUE(L"miter", PS_JOIN_MITER)
            ATTR_ENUM_END(m_joinStyle)
        SOUI_ATTRS_END()

      public:
        SLayoutSize m_width; // Stroke width
        COLORREF m_color;    // Stroke color

        /**
         * @brief Gets the style of the stroke.
         * @return Stroke style.
         */
        int GetStyle() const;

      private:
        int m_style;     // Line style
        int m_endStyle;  // End cap style
        int m_joinStyle; // Join style
    };

    /**
     * @class SCornerSize
     * @brief Represents the corner size for shapes.
     *
     * This class defines the corner radius for shapes, allowing customization of
     * the X and Y dimensions independently. It supports scaling based on layout size.
     */
    class SCornerSize : public TObjRefImpl<SObject> {
        DEF_SOBJECT(TObjRefImpl<SObject>, L"corners")

      public:
        /**
         * @brief Handles the 'radius' attribute.
         * @param strValue The value of the 'radius' attribute.
         * @param bLoading TRUE if loading, FALSE otherwise.
         * @return HRESULT indicating success or failure.
         */
        HRESULT OnAttrRadius(const SStringW strValue, BOOL bLoading);

        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"radius", OnAttrRadius)         // Custom handling for 'radius' attribute.
            ATTR_LAYOUTSIZE(L"radiusX", m_radiusX, TRUE) // X-axis radius.
            ATTR_LAYOUTSIZE(L"radiusY", m_radiusY, TRUE) // Y-axis radius.
        SOUI_ATTRS_END()

        /**
         * @brief Gets the corner point in pixel coordinates.
         * @param nScale Scale factor for layout size conversion.
         * @return POINT representing the corner size in pixels.
         */
        POINT GetConner(int nScale) const
        {
            return CPoint(m_radiusX.toPixelSize(nScale), m_radiusY.toPixelSize(nScale));
        }

      protected:
        SLayoutSize m_radiusX, m_radiusY; // Layout sizes for X and Y radii.
    };

    /**
     * @class SRatioCornerSize
     * @brief Represents the ratio-based corner size for shapes.
     *
     * This class defines the corner radius as a ratio of the shape's dimensions,
     * allowing dynamic sizing based on the shape's width and height.
     */
    class SRatioCornerSize : public TObjRefImpl<SObject> {
        DEF_SOBJECT(TObjRefImpl<SObject>, L"ratio_corners")

      public:
        /**
         * @brief Constructor initializes the ratio values to 0.
         */
        SRatioCornerSize()
        {
            m_radius.fX = m_radius.fY = 0.0f;
        }

        /**
         * @brief Handles the 'radius' attribute.
         * @param strValue The value of the 'radius' attribute.
         * @param bLoading TRUE if loading, FALSE otherwise.
         * @return HRESULT indicating success or failure.
         */
        HRESULT OnAttrRadius(const SStringW strValue, BOOL bLoading);

        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"radius", OnAttrRadius)      // Custom handling for 'radius' attribute.
            ATTR_FLOAT(L"radiusX", m_radius.fX, TRUE) // Ratio for X-axis radius.
            ATTR_FLOAT(L"radiusY", m_radius.fY, TRUE) // Ratio for Y-axis radius.
        SOUI_ATTRS_END()

        /**
         * @brief Gets the corner point based on the shape's dimensions.
         * @param rc The rectangle defining the shape's dimensions.
         * @return POINT representing the corner size based on ratios.
         */
        POINT GetConner(const CRect &rc) const
        {
            return CPoint((int)(rc.Width() / 2 * m_radius.fX), (int)(rc.Height() / 2 * m_radius.fY));
        }

      protected:
        SPoint m_radius; // Ratios for X and Y radii, ranging from [0,1].
    };
    /**
     * @class SShapeSize
     * @brief Represents the size of a shape.
     *
     * This class defines the width and height of a shape, supporting layout-based sizing.
     */
    class SShapeSize : public TObjRefImpl<SObject> {
        DEF_SOBJECT(TObjRefImpl<SObject>, L"size")
        friend class SSkinShape;

      public:
        SOUI_ATTRS_BEGIN()
            ATTR_LAYOUTSIZE(L"width", m_width, TRUE)   // Width of the shape.
            ATTR_LAYOUTSIZE(L"height", m_height, TRUE) // Height of the shape.
        SOUI_ATTRS_END()
      protected:
        SLayoutSize m_width, m_height; // Layout sizes for width and height.
    };

    /**
     * @class SShapeRing
     * @brief Represents a ring shape.
     *
     * This class defines a ring shape with start and sweep angles for drawing arcs.
     */
    class SShapeRing : public TObjRefImpl<SObject> {
        DEF_SOBJECT(TObjRefImpl<SObject>, L"ring")
        friend class SSkinShape;

      public:
        /**
         * @brief Constructor initializes the start and sweep angles.
         */
        SShapeRing()
            : m_startAngle(0.0f)
            , m_sweepAngle(360.0f)
        {
        }

        SOUI_ATTRS_BEGIN()
            ATTR_FLOAT(L"startAngle", m_startAngle, TRUE) // Start angle of the ring.
            ATTR_FLOAT(L"sweepAngle", m_sweepAngle, TRUE) // Sweep angle of the ring.
        SOUI_ATTRS_END()

      protected:
        float m_startAngle; // Start angle for the ring.
        float m_sweepAngle; // Sweep angle for the ring.
    };

  public:
    SSkinShape();

    STDMETHOD_(SIZE, GetSkinSize)(THIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetStates)(THIS) SCONST OVERRIDE;

    SOUI_ATTRS_BEGIN()
        ATTR_ENUM_BEGIN(L"shape", Shape, TRUE)
            ATTR_ENUM_VALUE(L"rectangle", rectangle)
            ATTR_ENUM_VALUE(L"oval", oval)
            ATTR_ENUM_VALUE(L"ring", ring)
        ATTR_ENUM_END(m_shape)
    SOUI_ATTRS_END()
  protected:
    STDMETHOD_(void, OnInitFinished)(THIS_ IXmlNode *pNode) OVERRIDE;

    void _DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const override;

    void _Scale(ISkinObj *pObj, int nScale) override;
    POINT GetCornerSize(const CRect &rc) const;

    Shape m_shape;

    SAutoRefPtr<SShapeSolid> m_solid;
    SAutoRefPtr<SShapeBitmap> m_bitmap;
    SAutoRefPtr<SGradientBrush> m_gradient;
    SAutoRefPtr<SShapeSize> m_shapeSize;
    SAutoRefPtr<SCornerSize> m_cornerSize;
    SAutoRefPtr<SRatioCornerSize> m_ratioCornerSize;
    SAutoRefPtr<SStroke> m_stroke;
    SAutoRefPtr<SShapeRing> m_ringParam;
};

/**
 * @class SSKinGroup
 * @brief Represents a group of skins for different states.
 *
 * This class allows grouping multiple skins for normal, hover, pushdown, and disabled states.
 */
class SOUI_EXP SSKinGroup : public SSkinObjBase {
    DEF_SOBJECT(SSkinObjBase, L"group")

  public:
    /**
     * @brief Gets the size of the skin group.
     * @return SIZE representing the skin group size.
     */
    STDMETHOD_(SIZE, GetSkinSize)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Gets the number of states supported by the skin group.
     * @return int representing the number of states.
     */
    STDMETHOD_(int, GetStates)(CTHIS) SCONST OVERRIDE;

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"normal", m_skins[0], FALSE)   // Normal state skin.
        ATTR_SKIN(L"hover", m_skins[1], FALSE)    // Hover state skin.
        ATTR_SKIN(L"pushDown", m_skins[2], FALSE) // Pushdown state skin.
        ATTR_SKIN(L"disable", m_skins[3], FALSE)  // Disabled state skin.
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Draws the skin group by index.
     * @param pRT Pointer to the render target.
     * @param rcDraw Rectangle to draw in.
     * @param iState State index.
     * @param byAlpha Alpha value for transparency.
     */
    void _DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const override;

    /**
     * @brief Scales the skin group.
     * @param skinObj Pointer to the skin object.
     * @param nScale Scale factor.
     */
    void _Scale(ISkinObj *skinObj, int nScale) override;

  protected:
    SAutoRefPtr<ISkinObj> m_skins[4]; // Array of skins for different states.
};

/**
 * @class      SSkinTreeLines
 * @brief      Tree Lines Skin
 *
 * Description: Represents a skin that draws tree control lines using IRenderTarget API,
 * supporting 10 states and adapting to any row height.
 */
class SOUI_EXP SSkinTreeLines : public SSkinObjBase {
    DEF_SOBJECT(SSkinObjBase, L"treelines")

  public:
    /**
     * @brief Constructor for SSkinTreeLines.
     */
    SSkinTreeLines();

    /**
     * @brief Gets the size of the skin.
     * @return Size of the skin.
     */
    STDMETHOD_(SIZE, GetSkinSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the number of states in the skin.
     * @return Number of states.
     */
    STDMETHOD_(int, GetStates)(THIS) SCONST OVERRIDE;

  protected:
    /**
     * @brief Draws the skin by index.
     * @param pRT Pointer to the render target.
     * @param rcDraw Rectangle to draw in.
     * @param iState State index.
     * @param byAlpha Alpha value for transparency.
     */
    void _DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const override;

    /**
     * @brief Scales the skin.
     * @param skinObj Pointer to the skin object.
     * @param nScale Scale factor.
     */
    void _Scale(ISkinObj *skinObj, int nScale) override;

  protected:
    COLORREF m_crLine; // Line color
    COLORREF m_crCross; // Cross color for expand/collapse indicators
    int m_nLineWidth;  // Line width
    int m_nBoxSize;     // Size of the expand/collapse box
  public:
    SOUI_ATTRS_BEGIN()
        ATTR_COLOR(L"colorLine", m_crLine, TRUE)        // Line color
        ATTR_COLOR(L"colorCross", m_crCross, TRUE)
        ATTR_INT(L"lineWidth", m_nLineWidth, TRUE)  // Line width
        ATTR_INT(L"boxSize", m_nBoxSize, TRUE)     // Expand/collapse box size)
    SOUI_ATTRS_END()
};

SNSEND
#endif // __SSKIN__H__