#ifndef __SRENDER_I__H__
#define __SRENDER_I__H__

#include <interface/obj-ref-i.h>
#include <interface/sobject-i.h>
#include <interface/SGradient-i.h>
#include <interface/SPathEffect-i.h>
#include <interface/SImgDecoder-i.h>
#include <interface/sxml-i.h>

SNSBEGIN

typedef struct IRenderFactory IRenderFactory;

// Enumerations
typedef enum EXPEND_MODE
{
    EM_NULL = 0, /*< 不变 */
    EM_STRETCH,  /*< 拉伸 */
    EM_TILE,     /*< 平铺 */
} EXPEND_MODE;

typedef enum FilterLevel
{
    kUndef_FilterLevel = 100, // don't use -1 as filter level will pass to render in word and will get 65535
    kNone_FilterLevel = 0,
    kLow_FilterLevel,
    kMedium_FilterLevel,
    kHigh_FilterLevel
} FilterLevel;

typedef enum GradientType
{
    linear = 0, /*< 线性渐变 */
    radial,     /*< 辐射渐变 */
    sweep       /*< 扫描渐变 */
} GradientType;

typedef enum RopMode
{
    kClear_Mode,    //!< [0, 0]
    kSrc_Mode,      //!< [Sa, Sc]
    kDst_Mode,      //!< [Da, Dc]
    kSrcOver_Mode,  //!< [Sa + Da - Sa*Da, Rc = Sc + (1 - Sa)*Dc]
    kDstOver_Mode,  //!< [Sa + Da - Sa*Da, Rc = Dc + (1 - Da)*Sc]
    kSrcIn_Mode,    //!< [Sa * Da, Sc * Da]
    kDstIn_Mode,    //!< [Sa * Da, Sa * Dc]
    kSrcOut_Mode,   //!< [Sa * (1 - Da), Sc * (1 - Da)]
    kDstOut_Mode,   //!< [Da * (1 - Sa), Dc * (1 - Sa)]
    kSrcATop_Mode,  //!< [Da, Sc * Da + (1 - Sa) * Dc]
    kDstATop_Mode,  //!< [Sa, Sa * Dc + Sc * (1 - Da)]
    kXor_Mode,      //!< [Sa + Da - 2 * Sa * Da, Sc * (1 - Da) + (1 - Sa) * Dc]
    kPlus_Mode,     //!< [Sa + Da, Sc + Dc]
    kModulate_Mode, // multiplies all components (= alpha and color)

    // Following blend modes are defined in the CSS Compositing standard:
    // https://dvcs.w3.org/hg/FXTF/rawfile/tip/compositing/index.html#blending
    kScreen_Mode,
    kLastCoeffMode = kScreen_Mode,

    kOverlay_Mode,
    kDarken_Mode,
    kLighten_Mode,
    kColorDodge_Mode,
    kColorBurn_Mode,
    kHardLight_Mode,
    kSoftLight_Mode,
    kDifference_Mode,
    kExclusion_Mode,
    kMultiply_Mode,
    kLastSeparableMode = kMultiply_Mode,

    kHue_Mode,
    kSaturation_Mode,
    kColor_Mode,
    kLuminosity_Mode,

    // extend xfermode
    kSrcCopy = SRCCOPY,
    kDstInvert = DSTINVERT,
    kSrcInvert = SRCINVERT,
    kSrcAnd = SRCAND,
} RopMode;

typedef struct fPoint
{
    float fX, fY;
} fPoint;

typedef struct fRect
{
    float fLeft, fTop, fRight, fBottom;
} fRect;

typedef enum _BlurStyle
{
    kNone_BLurStyle = -1,
    kNormal_BlurStyle = 0, //!< 模糊内部和外部
    kSolid_BlurStyle,      //!< 固定内部，模糊外部
    kOuter_BlurStyle,      //!< 无内部，模糊外部
    kInner_BlurStyle,      //!< 模糊内部，无外部

    kLastEnum_BlurStyle = kInner_BlurStyle
} BlurStyle;

typedef enum _BlurFlags
{
    kNone_BlurFlag = 0x00,
    /** The blur layer's radius is not affected by transforms */
    kIgnoreTransform_BlurFlag = 0x01,
    /** Use a smoother, higher quality blur algorithm */
    kHighQuality_BlurFlag = 0x02,
    /** mask for all blur flags */
    kAll_BlurFlag = 0x03
} BlurFlags;

// Interfaces
#undef INTERFACE
#define INTERFACE IMaskFilter
DECLARE_INTERFACE_(IMaskFilter, IObjRef)
{
    // Retrieves a pointer to the mask filter
    STDMETHOD_(void *, GetPtr)(THIS) PURE;
};

typedef enum _OBJTYPE
{
    OT_NULL = 0,
    OT_PEN,
    OT_BRUSH,
    OT_BITMAP,
    OT_FONT,
    OT_RGN,
    OT_PATH,
} OBJTYPE;
/**
 * @struct IRenderObj
 * @brief Base class for all renderable objects.
 *
 * @details All renderable objects use reference counting to manage their lifecycle.
 */
#undef INTERFACE
#define INTERFACE IRenderObj
DECLARE_INTERFACE_(IRenderObj, IObjRef)
{
    /**
     * @brief Increments the reference count for the object.
     *
     * @details This method is used to increase the reference count of the object,
     * ensuring it remains valid as long as references exist.
     *
     * @return long The new reference count after incrementing.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrements the reference count for the object.
     *
     * @details This method decreases the reference count of the object.
     * When the count reaches zero, the object may be released.
     *
     * @return long The new reference count after decrementing.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Called when the final release of the object occurs.
     *
     * @details This method is invoked when the reference count reaches zero,
     * allowing for any necessary cleanup before the object is destroyed.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Obtains the render factory that created this rendering object.
     *
     * @details Retrieves a pointer to the render factory responsible for creating this object.
     *
     * @return IRenderFactory* Pointer to the render factory.
     */
    STDMETHOD_(IRenderFactory *, GetRenderFactory)(CTHIS) SCONST PURE;

    /**
     * @brief Queries the type of the rendering object.
     *
     * @details Returns an enumeration value representing the type of the object (e.g., brush, pen, bitmap).
     *
     * @return OBJTYPE The type of the rendering object.
     */
    STDMETHOD_(OBJTYPE, ObjectType)(CTHIS) SCONST PURE;
};

/**
 * @enum _BrushType
 * @brief Enumeration defining types of brushes.
 *
 * @details Specifies the different types of brushes available for rendering purposes.
 */
typedef enum _BrushType
{
    Brush_Color = 0, //!< Color brush.
    Brush_Bitmap,    //!< Bitmap brush.
    Brush_Shader     //!< Shader brush.
} BrushType;

typedef enum _TileMode
{
    /** replicate the edge color if the shader draws outside of its
     *  original bounds
     */
    kClamp_TileMode = 0,

    /** repeat the shader's image horizontally and vertically */
    kRepeat_TileMode,

    /** repeat the shader's image horizontally and vertically, alternating
     *  mirror images so that adjacent images always seam
     */
    kMirror_TileMode,
} TileMode;

/**
 * @struct     IBrushS
 * @brief      Brush object interface.
 *
 * @describe
 */
#undef INTERFACE
#define INTERFACE IBrushS
/**
 * @struct IBrushS
 * @brief Brush object interface.
 *
 * @details This interface represents a brush used for rendering operations.
 * It inherits from the IRenderObj interface and provides methods to query
 * the brush type and manage the brush's lifecycle.
 */
DECLARE_INTERFACE_(IBrushS, IRenderObj)
{
    /**
     * @brief Increments the reference count for the object.
     *
     * @details This method is used to increase the reference count of the object,
     * ensuring it remains valid as long as references exist.
     *
     * @return long The new reference count after incrementing.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrements the reference count for the object.
     *
     * @details This method decreases the reference count of the object.
     * When the count reaches zero, the object may be released.
     *
     * @return long The new reference count after decrementing.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Called when the final release of the object occurs.
     *
     * @details This method is invoked when the reference count reaches zero,
     * allowing for any necessary cleanup before the object is destroyed.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Obtains the render factory that created this rendering object.
     *
     * @details Retrieves a pointer to the render factory responsible for creating this object.
     *
     * @return IRenderFactory* Pointer to the render factory.
     */
    STDMETHOD_(IRenderFactory *, GetRenderFactory)(CTHIS) SCONST PURE;

    /**
     * @brief Queries the type of the rendering object.
     *
     * @details Returns an enumeration value representing the type of the object (e.g., brush, pen, bitmap).
     *
     * @return OBJTYPE The type of the rendering object.
     */
    STDMETHOD_(OBJTYPE, ObjectType)(CTHIS) SCONST PURE;

    /**
     * @brief Queries the type of the brush.
     *
     * @details Returns an enumeration value representing the type of the brush (e.g., color, bitmap, shader).
     *
     * @return BrushType The type of the brush.
     */
    STDMETHOD_(BrushType, GetBrushType)(CTHIS) SCONST PURE;
};

/**
 * @struct IPenS
 * @brief Pen object interface.
 *
 * @details This interface represents a pen used for rendering operations.
 * It inherits from the IRenderObj interface and provides methods to manage
 * the pen's properties and lifecycle.
 */
#undef INTERFACE
#define INTERFACE IPenS
DECLARE_INTERFACE_(IPenS, IRenderObj)
{
    /**
     * @brief Increments the reference count for the object.
     *
     * @details This method is used to increase the reference count of the object,
     * ensuring it remains valid as long as references exist.
     *
     * @return long The new reference count after incrementing.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrements the reference count for the object.
     *
     * @details This method decreases the reference count of the object.
     * When the count reaches zero, the object may be released.
     *
     * @return long The new reference count after decrementing.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Called when the final release of the object occurs.
     *
     * @details This method is invoked when the reference count reaches zero,
     * allowing for any necessary cleanup before the object is destroyed.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Obtains the render factory that created this rendering object.
     *
     * @details Retrieves a pointer to the render factory responsible for creating this object.
     *
     * @return IRenderFactory* Pointer to the render factory.
     */
    STDMETHOD_(IRenderFactory *, GetRenderFactory)(CTHIS) SCONST PURE;

    /**
     * @brief Queries the type of the rendering object.
     *
     * @details Returns an enumeration value representing the type of the object (e.g., brush, pen, bitmap).
     *
     * @return OBJTYPE The type of the rendering object.
     */
    STDMETHOD_(OBJTYPE, ObjectType)(CTHIS) SCONST PURE;

    /**
     * @brief Retrieves the width of the pen.
     *
     * @details Returns the current width of the pen.
     *
     * @return int The width of the pen.
     */
    STDMETHOD_(int, GetWidth)(CTHIS) SCONST PURE;

    /**
     * @brief Sets the width of the pen.
     *
     * @details Sets the width of the pen to the specified value.
     *
     * @param nWid The new width of the pen.
     */
    STDMETHOD_(void, SetWidth)(THIS_ int nWid) PURE;

    /**
     * @brief Retrieves the style of the pen.
     *
     * @details Returns the current style of the pen.
     *
     * @return int The style of the pen.
     */
    STDMETHOD_(int, GetStyle)(CTHIS) SCONST PURE;

    /**
     * @brief Sets the style of the pen.
     *
     * @details Sets the style of the pen to the specified value.
     *
     * @param nStyle The new style of the pen.
     */
    STDMETHOD_(void, SetStyle)(THIS_ int nStyle) PURE;

    /**
     * @brief Retrieves the color of the pen.
     *
     * @details Returns the current color of the pen.
     *
     * @return COLORREF The color of the pen.
     */
    STDMETHOD_(COLORREF, GetColor)(CTHIS) SCONST PURE;

    /**
     * @brief Sets the color of the pen.
     *
     * @details Sets the color of the pen to the specified value.
     *
     * @param cr The new color of the pen.
     */
    STDMETHOD_(void, SetColor)(THIS_ COLORREF cr) PURE;
};
/**
 * @struct IBitmapS
 * @brief Bitmap object interface.
 *
 * @details This interface represents a bitmap used for rendering operations.
 * It inherits from the IRenderObj interface and provides methods to manage
 * the bitmap's properties and lifecycle, including initialization, loading,
 * saving, and scaling operations.
 */
#undef INTERFACE
#define INTERFACE IBitmapS
DECLARE_INTERFACE_(IBitmapS, IRenderObj)
{
    /**
     * @brief Increments the reference count for the object.
     *
     * @details This method is used to increase the reference count of the object,
     * ensuring it remains valid as long as references exist.
     *
     * @return long The new reference count after incrementing.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrements the reference count for the object.
     *
     * @details This method decreases the reference count of the object.
     * When the count reaches zero, the object may be released.
     *
     * @return long The new reference count after decrementing.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Called when the final release of the object occurs.
     *
     * @details This method is invoked when the reference count reaches zero,
     * allowing for any necessary cleanup before the object is destroyed.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Obtains the render factory that created this rendering object.
     *
     * @details Retrieves a pointer to the render factory responsible for creating this object.
     *
     * @return IRenderFactory* Pointer to the render factory.
     */
    STDMETHOD_(IRenderFactory *, GetRenderFactory)(CTHIS) SCONST PURE;

    /**
     * @brief Queries the type of the rendering object.
     *
     * @details Returns an enumeration value representing the type of the object (e.g., brush, pen, bitmap).
     *
     * @return OBJTYPE The type of the rendering object.
     */
    STDMETHOD_(OBJTYPE, ObjectType)(CTHIS) SCONST PURE;

    /**
     * @brief Initializes the bitmap from 32-bit bitmap data.
     *
     * @details Initializes the bitmap with the specified width, height, and pixel data.
     *
     * @param nWid The width of the bitmap.
     * @param nHei The height of the bitmap.
     * @param pBits Pointer to the 32-bit bitmap data.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, Init)(THIS_ int nWid, int nHei, const LPVOID pBits) PURE;

    /**
     * @brief Initializes the bitmap from an IImgFrame object.
     *
     * @details Initializes the bitmap using the data from the specified IImgFrame object.
     *
     * @param pImgFrame Pointer to the IImgFrame object.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, Init2)(THIS_ IImgFrame * pImgFrame) PURE;

    /**
     * @brief Loads the bitmap from a file.
     *
     * @details Loads the bitmap data from the specified file.
     *
     * @param pszFileName Path to the file containing the bitmap data.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, LoadFromFile)(THIS_ LPCTSTR pszFileName) PURE;

    /**
     * @brief Loads the bitmap from memory.
     *
     * @details Loads the bitmap data from the specified memory buffer.
     *
     * @param pBuf Pointer to the memory buffer containing the bitmap data.
     * @param szLen Size of the memory buffer.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, LoadFromMemory)(THIS_ LPBYTE pBuf, size_t szLen) PURE;

    /**
     * @brief Retrieves the width of the bitmap.
     *
     * @details Returns the current width of the bitmap.
     *
     * @return UINT The width of the bitmap.
     */
    STDMETHOD_(UINT, Width)(CTHIS) SCONST PURE;

    /**
     * @brief Retrieves the height of the bitmap.
     *
     * @details Returns the current height of the bitmap.
     *
     * @return UINT The height of the bitmap.
     */
    STDMETHOD_(UINT, Height)(CTHIS) SCONST PURE;

    /**
     * @brief Retrieves the size of the bitmap.
     *
     * @details Returns the current size of the bitmap as a SIZE structure.
     *
     * @return SIZE The size of the bitmap.
     */
    STDMETHOD_(SIZE, Size)(CTHIS) SCONST PURE;

    /**
     * @brief Locks the pixel bits of the bitmap for writing.
     *
     * @details Locks the pixel data of the bitmap for writing operations.
     *
     * @return LPVOID Pointer to the locked pixel data.
     */
    STDMETHOD_(LPVOID, LockPixelBits)(THIS) PURE;

    /**
     * @brief Unlocks the pixel bits of the bitmap.
     *
     * @details Unlocks the pixel data of the bitmap after writing operations.
     *
     * @param pBuf Pointer to the locked pixel data.
     */
    STDMETHOD_(void, UnlockPixelBits)(THIS_ LPVOID pBuf) PURE;

    /**
     * @brief Retrieves the pixel bits of the bitmap for reading.
     *
     * @details Retrieves the pixel data of the bitmap for reading operations.
     *
     * @return const LPVOID Pointer to the pixel data.
     */
    STDMETHOD_(const LPVOID, GetPixelBits)(CTHIS) SCONST PURE;

    /**
     * @brief Clones the bitmap.
     *
     * @details Creates a copy of the bitmap.
     *
     * @param ppClone Pointer to receive the cloned bitmap object.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, Clone)(CTHIS_ IBitmapS * *ppClone) SCONST PURE;

    /**
     * @brief Scales the bitmap and clones it.
     *
     * @details Scales the bitmap by the specified factor and creates a copy.
     *
     * @param pOutput Pointer to receive the scaled bitmap object.
     * @param nScale Scaling factor.
     * @param filterLevel Filter level for scaling.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, Scale)
    (CTHIS_ IBitmapS * *pOutput, int nScale, FilterLevel filterLevel) SCONST PURE;

    /**
     * @brief Scales the bitmap to specified dimensions and clones it.
     *
     * @details Scales the bitmap to the specified width and height and creates a copy.
     *
     * @param pOutput Pointer to receive the scaled bitmap object.
     * @param nWid New width of the bitmap.
     * @param nHei New height of the bitmap.
     * @param filterLevel Filter level for scaling.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, Scale2)
    (CTHIS_ IBitmapS * *pOutput, int nWid, int nHei, FilterLevel filterLevel) SCONST PURE;

    /**
     * @brief Saves the bitmap to a file.
     *
     * @details Saves the bitmap data to the specified file with the given format.
     *
     * @param pszFileName Path to the file where the bitmap data will be saved.
     * @param pFormat Format of the saved file.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, Save)(THIS_ LPCWSTR pszFileName, const LPVOID pFormat) SCONST PURE;

    /**
     * @brief Saves the bitmap to a file with a specified format.
     *
     * @details Saves the bitmap data to the specified file with the given image format.
     *
     * @param pszFileName Path to the file where the bitmap data will be saved.
     * @param imgFmt Image format for saving.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, Save2)(THIS_ LPCWSTR pszFileName, ImgFmt imgFmt) SCONST PURE;
};

typedef IBitmapS *IBitmapPtr;

/**
 * @enum _FillStyle
 * @brief Enumeration defining fill styles.
 *
 * @details Specifies the different fill styles available for rendering purposes.
 */
typedef enum _FillStyle
{
    kFill_Style = 0,          //!< Fill only.
    kStroke_Style = 1,        //!< Stroke only.
    kStrokeAndFill_Style = 2, //!< Stroke and fill.
} FillStyle;

/**
 * @struct IFontS
 * @brief Font object interface.
 *
 * @details This interface represents a font used for rendering operations.
 * It inherits from the IRenderObj interface and provides methods to manage
 * the font's properties and lifecycle.
 */
#undef INTERFACE
#define INTERFACE IFontS
DECLARE_INTERFACE_(IFontS, IRenderObj)
{
    /**
     * @brief Increments the reference count for the object.
     *
     * @details This method is used to increase the reference count of the object,
     * ensuring it remains valid as long as references exist.
     *
     * @return long The new reference count after incrementing.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrements the reference count for the object.
     *
     * @details This method decreases the reference count of the object.
     * When the count reaches zero, the object may be released.
     *
     * @return long The new reference count after decrementing.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Called when the final release of the object occurs.
     *
     * @details This method is invoked when the reference count reaches zero,
     * allowing for any necessary cleanup before the object is destroyed.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Obtains the render factory that created this rendering object.
     *
     * @details Retrieves a pointer to the render factory responsible for creating this object.
     *
     * @return IRenderFactory* Pointer to the render factory.
     */
    STDMETHOD_(IRenderFactory *, GetRenderFactory)(CTHIS) SCONST PURE;

    /**
     * @brief Queries the type of the rendering object.
     *
     * @details Returns an enumeration value representing the type of the object (e.g., brush, pen, bitmap).
     *
     * @return OBJTYPE The type of the rendering object.
     */
    STDMETHOD_(OBJTYPE, ObjectType)(CTHIS) SCONST PURE;

    /**
     * @brief Retrieves the LOGFONT structure of the font.
     *
     * @details Returns a pointer to the LOGFONT structure defining the font attributes.
     *
     * @return const LOGFONT* Pointer to the LOGFONT structure.
     */
    STDMETHOD_(const LOGFONT *, LogFont)(CTHIS) SCONST PURE;

    /**
     * @brief Retrieves the family name of the font.
     *
     * @details Returns the family name of the font.
     *
     * @return LPCTSTR The family name of the font.
     */
    STDMETHOD_(LPCTSTR, FamilyName)(CTHIS) SCONST PURE;

    /**
     * @brief Retrieves the text size of the font.
     *
     * @details Returns the text size of the font.
     *
     * @return int The text size of the font.
     */
    STDMETHOD_(int, TextSize)(CTHIS) SCONST PURE;

    /**
     * @brief Checks if the font is bold.
     *
     * @details Returns a boolean indicating whether the font is bold.
     *
     * @return BOOL TRUE if the font is bold, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsBold)(CTHIS) SCONST PURE;

    /**
     * @brief Checks if the font has an underline.
     *
     * @details Returns a boolean indicating whether the font has an underline.
     *
     * @return BOOL TRUE if the font has an underline, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsUnderline)(CTHIS) SCONST PURE;

    /**
     * @brief Checks if the font is italic.
     *
     * @details Returns a boolean indicating whether the font is italic.
     *
     * @return BOOL TRUE if the font is italic, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsItalic)(CTHIS) SCONST PURE;

    /**
     * @brief Checks if the font has a strikeout.
     *
     * @details Returns a boolean indicating whether the font has a strikeout.
     *
     * @return BOOL TRUE if the font has a strikeout, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsStrikeOut)(CTHIS) SCONST PURE;

    /**
     * @brief Updates the font properties.
     *
     * @details Updates the font properties using the specified LOGFONT structure.
     *
     * @param pLogFont Pointer to the LOGFONT structure defining the new font attributes.
     * @return BOOL TRUE if the update was successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, UpdateFont)(THIS_ const LOGFONT *pLogFont) PURE;

    /**
     * @brief Sets properties from an XML node.
     *
     * @details Sets the font properties based on the attributes specified in the XML node.
     *
     * @param pXmlNode Pointer to the XML node containing the font properties.
     */
    STDMETHOD_(void, SetProp)(THIS_ IXmlNode * pXmlNode) PURE;
};

/**
 * @struct IRegionS
 * @brief Region object interface.
 *
 * @details This interface represents a region used for clipping and other rendering operations.
 * It inherits from the IRenderObj interface and provides methods to manage
 * the region's properties and lifecycle, including combining shapes, checking containment,
 * and offsetting the region.
 */
#undef INTERFACE
#define INTERFACE IRegionS
DECLARE_INTERFACE_(IRegionS, IRenderObj)
{
    /**
     * @brief Increments the reference count for the object.
     *
     * @details This method is used to increase the reference count of the object,
     * ensuring it remains valid as long as references exist.
     *
     * @return long The new reference count after incrementing.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrements the reference count for the object.
     *
     * @details This method decreases the reference count of the object.
     * When the count reaches zero, the object may be released.
     *
     * @return long The new reference count after decrementing.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Called when the final release of the object occurs.
     *
     * @details This method is invoked when the reference count reaches zero,
     * allowing for any necessary cleanup before the object is destroyed.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Obtains the render factory that created this rendering object.
     *
     * @details Retrieves a pointer to the render factory responsible for creating this object.
     *
     * @return IRenderFactory* Pointer to the render factory.
     */
    STDMETHOD_(IRenderFactory *, GetRenderFactory)(CTHIS) SCONST PURE;

    /**
     * @brief Queries the type of the rendering object.
     *
     * @details Returns an enumeration value representing the type of the object (e.g., brush, pen, bitmap).
     *
     * @return OBJTYPE The type of the rendering object.
     */
    STDMETHOD_(OBJTYPE, ObjectType)(CTHIS) SCONST PURE;

    /**
     * @brief Combines a rectangle with the region.
     *
     * @details Combines the specified rectangle with the current region using the specified mode.
     *
     * @param lprect Pointer to the rectangle to combine.
     * @param nCombineMode Mode for combining the rectangle with the region.
     */
    STDMETHOD_(void, CombineRect)(THIS_ LPCRECT lprect, int nCombineMode) PURE;

    /**
     * @brief Combines another region with the current region.
     *
     * @details Combines the specified region with the current region using the specified mode.
     *
     * @param pRgnSrc Pointer to the region to combine.
     * @param nCombineMode Mode for combining the regions.
     */
    STDMETHOD_(void, CombineRgn)(THIS_ const IRegionS *pRgnSrc, int nCombineMode) PURE;

    /**
     * @brief Combines a rounded rectangle with the region.
     *
     * @details Combines the specified rounded rectangle with the current region using the specified mode.
     *
     * @param lprect Pointer to the rectangle defining the bounds of the rounded rectangle.
     * @param ptConner Point defining the corner radii of the rounded rectangle.
     * @param nCombineMode Mode for combining the rounded rectangle with the region.
     */
    STDMETHOD_(void, CombineRoundRect)(THIS_ LPCRECT lprect, POINT ptConner, int nCombineMode) PURE;

    /**
     * @brief Combines an ellipse with the region.
     *
     * @details Combines the specified ellipse with the current region using the specified mode.
     *
     * @param lprect Pointer to the rectangle defining the bounds of the ellipse.
     * @param nCombineMode Mode for combining the ellipse with the region.
     */
    STDMETHOD_(void, CombineEllipse)(THIS_ LPCRECT lprect, int nCombineMode) PURE;

    /**
     * @brief Combines a polygon with the region.
     *
     * @details Combines the specified polygon with the current region using the specified mode.
     *
     * @param pts Array of points defining the polygon.
     * @param count Number of points in the polygon.
     * @param nPolygonMode Mode for filling the polygon.
     * @param nCombineMode Mode for combining the polygon with the region.
     */
    STDMETHOD_(void, CombinePolygon)(THIS_ const POINT *pts, int count, int nPolygonMode, int nCombineMode) PURE;

    /**
     * @brief Checks if a point is inside the region.
     *
     * @details Determines whether the specified point lies within the region.
     *
     * @param pt Point to check.
     * @return BOOL TRUE if the point is inside the region, FALSE otherwise.
     */
    STDMETHOD_(BOOL, PtInRegion)(CTHIS_ POINT pt) SCONST PURE;

    /**
     * @brief Checks if a rectangle intersects with the region.
     *
     * @details Determines whether the specified rectangle intersects with the region.
     *
     * @param lprect Pointer to the rectangle to check.
     * @return BOOL TRUE if the rectangle intersects with the region, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RectInRegion)(CTHIS_ LPCRECT lprect) SCONST PURE;

    /**
     * @brief Retrieves the bounding rectangle of the region.
     *
     * @details Gets the smallest rectangle that completely encloses the region.
     *
     * @param lprect Pointer to the rectangle to receive the bounds.
     */
    STDMETHOD_(void, GetRgnBox)(CTHIS_ LPRECT lprect) SCONST PURE;

    /**
     * @brief Checks if the region is empty.
     *
     * @details Determines whether the region contains no area.
     *
     * @return BOOL TRUE if the region is empty, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsEmpty)(CTHIS) SCONST PURE;

    /**
     * @brief Offsets the region by a specified amount.
     *
     * @details Moves the region by the specified offset.
     *
     * @param pt Offset to apply to the region.
     */
    STDMETHOD_(void, Offset)(THIS_ POINT pt) PURE;

    /**
     * @brief Clears the region.
     *
     * @details Empties the region, making it contain no area.
     */
    STDMETHOD_(void, Clear)(THIS) PURE;
};

/**
 * @enum _xFormIndex
 * @brief Enumeration defining indices for transformation matrix elements.
 *
 * @details Specifies the indices for accessing elements of a 3x3 transformation matrix.
 */
typedef enum _xFormIndex
{
    kMScaleX = 0, //!< Index for the X-axis scaling factor.
    kMSkewX,      //!< Index for the X-axis skew factor.
    kMTransX,     //!< Index for the X-axis translation factor.
    kMSkewY,      //!< Index for the Y-axis skew factor.
    kMScaleY,     //!< Index for the Y-axis scaling factor.
    kMTransY,     //!< Index for the Y-axis translation factor.
    kMPersp0,     //!< Index for the first perspective factor.
    kMPersp1,     //!< Index for the second perspective factor.
    kMPersp2,     //!< Index for the third perspective factor.
    kMCount,      //!< Total number of elements in the transformation matrix.
} xFormIndex;

/**
 * @struct _IxForm
 * @brief Structure representing a 3x3 transformation matrix.
 *
 * @details Contains the elements of a 3x3 transformation matrix used for transformations.
 */
typedef struct _IxForm
{
    float fMat[kMCount]; //!< Array of floats representing the matrix elements.
} IxForm;

/**
 * @enum _FillType
 * @brief Enumeration defining fill types for path objects.
 *
 * @details Specifies the different fill types available for determining the "inside" of a path.
 */
typedef enum _FillType
{
    /** Specifies that "inside" is computed by a non-zero sum of signed edge crossings. */
    kWinding_FillType,

    /** Specifies that "inside" is computed by an odd number of edge crossings. */
    kEvenOdd_FillType,

    /** Same as Winding, but draws outside of the path rather than inside. */
    kInverseWinding_FillType,

    /** Same as EvenOdd, but draws outside of the path rather than inside. */
    kInverseEvenOdd_FillType
} FillType;

/**
 * @enum _Direction
 * @brief Enumeration defining contour directions for path objects.
 *
 * @details Specifies the direction in which closed contours are added to a path.
 */
typedef enum _Direction
{
    /** Direction either has not been or could not be computed. */
    kUnknown_Direction,

    /** Clockwise direction for adding closed contours. */
    kCW_Direction,

    /** Counter-clockwise direction for adding closed contours. */
    kCCW_Direction,
} Direction;

/**
 * @struct     IPath
 * @brief      Interface for path objects.
 *
 * @describe
 */
#undef INTERFACE
#define INTERFACE IPathS
DECLARE_INTERFACE_(IPathS, IRenderObj)
{
    /**
     * @brief Increments the reference count for the object.
     *
     * @details This method is used to increase the reference count of the object,
     * ensuring it remains valid as long as references exist.
     *
     * @return long The new reference count after incrementing.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrements the reference count for the object.
     *
     * @details This method decreases the reference count of the object.
     * When the count reaches zero, the object may be released.
     *
     * @return long The new reference count after decrementing.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Called when the final release of the object occurs.
     *
     * @details This method is invoked when the reference count reaches zero,
     * allowing for any necessary cleanup before the object is destroyed.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * GetRenderFactory
     * @brief    Obtain the factory that created this render object.
     * @return   IRenderFactory * -- The factory.
     */
    STDMETHOD_(IRenderFactory *, GetRenderFactory)(CTHIS) SCONST PURE;

    /**
     * ObjectType
     * @brief    Query the type of the object.
     * @return   const UINT -- The object type.
     */
    STDMETHOD_(OBJTYPE, ObjectType)(CTHIS) SCONST PURE;

    /**
     * getFillType
     * @brief    Return the path's fill type, which defines how "inside" is computed.
     * @return   FillType -- The path's fill type.
     */
    STDMETHOD_(FillType, getFillType)(CTHIS) SCONST PURE;

    /**
     * setFillType
     * @brief    Set the path's fill type, which defines how "inside" is computed.
     * @param ft The new fill type for this path.
     */
    STDMETHOD_(void, setFillType)(THIS_ FillType ft) PURE;

    /**
     * reset
     * @brief    Clear any lines and curves from the path, making it empty. This frees up internal storage.
     */
    STDMETHOD_(void, reset)(THIS) PURE;

    /**
     * isEmpty
     * @brief    Check if the path is empty (contains no lines or curves).
     * @return   BOOL -- TRUE if the path is empty.
     */
    STDMETHOD_(BOOL, isEmpty)(CTHIS) SCONST PURE;

    /**
     * getBounds
     * @brief    Return the bounds of the path's points.
     * @param prc The rectangle to receive the bounds.
     */
    STDMETHOD_(void, getBounds)(CTHIS_ LPRECT prc) SCONST PURE;

    //  Construction methods

    /**
     * moveTo
     * @brief    Set the beginning of the next contour to the point (x,y).
     * @param x  The x-coordinate of the start of a new contour.
     * @param y  The y-coordinate of the start of a new contour.
     */
    STDMETHOD_(void, moveTo)(THIS_ float x, float y) PURE;

    /**
     * rMoveTo
     * @brief    Set the beginning of the next contour relative to the last point on the previous contour.
     * @param dx The amount to add to the x-coordinate of the end of the previous contour.
     * @param dy The amount to add to the y-coordinate of the end of the previous contour.
     */
    STDMETHOD_(void, rMoveTo)(THIS_ float dx, float dy) PURE;

    /**
     * lineTo
     * @brief    Add a line from the last point to the specified point (x,y).
     * @param x  The x-coordinate of the end of a line.
     * @param y  The y-coordinate of the end of a line.
     */
    STDMETHOD_(void, lineTo)(THIS_ float x, float y) PURE;

    /**
     * rLineTo
     * @brief    Add a line from the last point, but the coordinates are relative to the last point.
     * @param dx The amount to add to the x-coordinate of the previous point.
     * @param dy The amount to add to the y-coordinate of the previous point.
     */
    STDMETHOD_(void, rLineTo)(THIS_ float dx, float dy) PURE;

    /**
     * quadTo
     * @brief    Add a quadratic Bezier curve from the last point, approaching control point (x1,y1), and ending at (x2,y2).
     * @param x1 The x-coordinate of the control point.
     * @param y1 The y-coordinate of the control point.
     * @param x2 The x-coordinate of the end point.
     * @param y2 The y-coordinate of the end point.
     */
    STDMETHOD_(void, quadTo)(THIS_ float x1, float y1, float x2, float y2) PURE;

    /**
     * rQuadTo
     * @brief    Add a quadratic Bezier curve, but the coordinates are relative to the last point.
     * @param dx1 The amount to add to the x-coordinate of the last point to specify the control point.
     * @param dy1 The amount to add to the y-coordinate of the last point to specify the control point.
     * @param dx2 The amount to add to the x-coordinate of the last point to specify the end point.
     * @param dy2 The amount to add to the y-coordinate of the last point to specify the end point.
     */
    STDMETHOD_(void, rQuadTo)(THIS_ float dx1, float dy1, float dx2, float dy2) PURE;

    /**
     * conicTo
     * @brief    Add a conic Bezier curve from the last point, approaching control point (x1,y1), and ending at (x2,y2).
     * @param x1 The x-coordinate of the control point.
     * @param y1 The y-coordinate of the control point.
     * @param x2 The x-coordinate of the end point.
     * @param y2 The y-coordinate of the end point.
     * @param w  The weight of the control point.
     */
    STDMETHOD_(void, conicTo)(THIS_ float x1, float y1, float x2, float y2, float w) PURE;

    /**
     * rConicTo
     * @brief    Add a conic Bezier curve, but the coordinates are relative to the last point.
     * @param dx1 The amount to add to the x-coordinate of the last point to specify the control point.
     * @param dy1 The amount to add to the y-coordinate of the last point to specify the control point.
     * @param dx2 The amount to add to the x-coordinate of the last point to specify the end point.
     * @param dy2 The amount to add to the y-coordinate of the last point to specify the end point.
     * @param w   The weight of the control point.
     */
    STDMETHOD_(void, rConicTo)(THIS_ float dx1, float dy1, float dx2, float dy2, float w) PURE;

    /**
     * cubicTo
     * @brief    Add a cubic Bezier curve from the last point, approaching control points (x1,y1) and (x2,y2), and ending at (x3,y3).
     * @param x1 The x-coordinate of the 1st control point.
     * @param y1 The y-coordinate of the 1st control point.
     * @param x2 The x-coordinate of the 2nd control point.
     * @param y2 The y-coordinate of the 2nd control point.
     * @param x3 The x-coordinate of the end point.
     * @param y3 The y-coordinate of the end point.
     */
    STDMETHOD_(void, cubicTo)(THIS_ float x1, float y1, float x2, float y2, float x3, float y3) PURE;

    /**
     * rCubicTo
     * @brief    Add a cubic Bezier curve, but the coordinates are relative to the last point.
     * @param x1 The x-coordinate of the 1st control point.
     * @param y1 The y-coordinate of the 1st control point.
     * @param x2 The x-coordinate of the 2nd control point.
     * @param y2 The y-coordinate of the 2nd control point.
     * @param x3 The x-coordinate of the end point.
     * @param y3 The y-coordinate of the end point.
     */
    STDMETHOD_(void, rCubicTo)(THIS_ float x1, float y1, float x2, float y2, float x3, float y3) PURE;

    /**
     * addRect
     * @brief    Add a closed rectangle contour to the path.
     * @param rect The rectangle to add as a closed contour.
     * @param dir  The direction to wind the rectangle's contour. Cannot be kUnknown_Direction.
     */
    STDMETHOD_(void, addRect)(THIS_ const RECT *rect, Direction dir DEF_VAL(kCW_Direction)) PURE;

    /**
     * addRect2
     * @brief    Add a closed rectangle contour to the path.
     * @param left   The left side of the rectangle.
     * @param top    The top of the rectangle.
     * @param right  The right side of the rectangle.
     * @param bottom The bottom of the rectangle.
     * @param dir    The direction to wind the rectangle's contour. Cannot be kUnknown_Direction.
     */
    STDMETHOD_(void, addRect2)(THIS_ float left, float top, float right, float bottom, Direction dir DEF_VAL(kCW_Direction)) PURE;

    /**
     * addOval
     * @brief    Add a closed oval contour to the path.
     * @param oval The bounding oval to add as a closed contour.
     * @param dir  The direction to wind the oval's contour. Cannot be kUnknown_Direction.
     */
    STDMETHOD_(void, addOval)(THIS_ const RECT *oval, Direction dir DEF_VAL(kCW_Direction)) PURE;

    /**
     * addOval2
     * @brief    Add a closed oval contour to the path.
     * @param left   The left side of the oval.
     * @param top    The top of the oval.
     * @param right  The right side of the oval.
     * @param bottom The bottom of the oval.
     * @param dir    The direction to wind the oval's contour. Cannot be kUnknown_Direction.
     */
    STDMETHOD_(void, addOval2)(THIS_ float left, float top, float right, float bottom, Direction dir DEF_VAL(kCW_Direction)) PURE;

    /**
     * addCircle
     * @brief    Add a closed circle contour to the path.
     * @param x      The x-coordinate of the center of the circle.
     * @param y      The y-coordinate of the center of the circle.
     * @param radius The radius of the circle.
     * @param dir    The direction to wind the circle's contour. Cannot be kUnknown_Direction.
     */
    STDMETHOD_(void, addCircle)(THIS_ float x, float y, float radius, Direction dir DEF_VAL(kCW_Direction)) PURE;

    /**
     * addArc
     * @brief    Add the specified arc to the path as a new contour.
     * @param oval       The bounds of the oval used to define the size of the arc.
     * @param startAngle The starting angle (in degrees) where the arc begins.
     * @param sweepAngle The sweep angle (in degrees) measured clockwise.
     */
    STDMETHOD_(void, addArc)(THIS_ const RECT *oval, float startAngle, float sweepAngle) PURE;

    /**
     * addArc2
     * @brief    Add the specified arc to the path as a new contour.
     * @param left       The left side of the oval.
     * @param top        The top of the oval.
     * @param right      The right side of the oval.
     * @param bottom     The bottom of the oval.
     * @param startAngle The starting angle (in degrees) where the arc begins.
     * @param sweepAngle The sweep angle (in degrees) measured clockwise.
     */
    STDMETHOD_(void, addArc2)(THIS_ float left, float top, float right, float bottom, float startAngle, float sweepAngle) PURE;

    /**
     * addRoundRect
     * @brief    Add a closed round-rectangle contour to the path.
     * @param rect The bounds of the round-rectangle.
     * @param rx   The x-radius of the rounded corners.
     * @param ry   The y-radius of the rounded corners.
     * @param dir  The direction to wind the rectangle's contour. Cannot be kUnknown_Direction.
     */
    STDMETHOD_(void, addRoundRect)(THIS_ const RECT *rect, float rx, float ry, Direction dir DEF_VAL(kCW_Direction)) PURE;

    /**
     * addRoundRect2
     * @brief    Add a closed round-rectangle contour to the path.
     * @param left   The left side of the round-rectangle.
     * @param top    The top of the round-rectangle.
     * @param right  The right side of the round-rectangle.
     * @param bottom The bottom of the round-rectangle.
     * @param rx     The x-radius of the rounded corners.
     * @param ry     The y-radius of the rounded corners.
     * @param dir    The direction to wind the rectangle's contour. Cannot be kUnknown_Direction.
     */
    STDMETHOD_(void, addRoundRect2)(THIS_ float left, float top, float right, float bottom, float rx, float ry, Direction dir DEF_VAL(kCW_Direction)) PURE;

    /**
     * addPoly
     * @brief    Add a new contour made of just lines.
     * @param pts  The array of points defining the polygon.
     * @param count The number of points in the array.
     * @param close Whether to close the polygon.
     */
    STDMETHOD_(void, addPoly)(THIS_ const POINT pts[], int count, BOOL close) PURE;

    /**
     * offset
     * @brief    Offset the path by (dx,dy).
     * @param dx  The amount in the X direction to offset the entire path.
     * @param dy  The amount in the Y direction to offset the entire path.
     */
    STDMETHOD_(void, offset)(THIS_ float dx, float dy) PURE;

    /**
     * transform
     * @brief    Transform the points in this path by the given matrix.
     * @param matrix The transformation matrix to apply to the path.
     */
    STDMETHOD_(void, transform)(THIS_ const IxForm *matrix) PURE;

    /**
     * getLastPt
     * @brief    Return the last point on the path.
     * @param lastPt The structure to receive the last point.
     * @return   BOOL -- TRUE if there are points, FALSE otherwise.
     */
    STDMETHOD_(BOOL, getLastPt)(CTHIS_ fPoint * lastPt) SCONST PURE;

    /**
     * addString
     * @brief    Add a string to the path.
     * @param pszText The string to add.
     * @param nLen    The length of the string.
     * @param x       The x-coordinate of the starting position.
     * @param y       The y-coordinate of the starting position.
     * @param pFont   The font to use for the text.
     */
    STDMETHOD_(void, addString)(THIS_ LPCTSTR pszText, int nLen, float x, float y, const IFontS *pFont) PURE;

    /**
     * clone
     * @brief    Clone the current path.
     * @return   IPathS* -- A pointer to the cloned path.
     */
    STDMETHOD_(IPathS *, clone)(CTHIS) SCONST PURE;

    /**
     * beginFigure
     * @brief    Begin a new figure in the path.
     * @param x  The x-coordinate of the starting point.
     * @param y  The y-coordinate of the starting point.
     * @param bFill Whether to fill the figure. Default is TRUE.
     * @return   BOOL -- TRUE if successful.
     */
    STDMETHOD_(BOOL, beginFigure)(THIS_ float x, float y, BOOL bFill DEF_VAL(TRUE)) PURE;

    /**
     * endFigure
     * @brief    End the current figure in the path.
     * @param bClose Whether to close the figure.
     * @return   BOOL -- TRUE if successful.
     */
    STDMETHOD_(BOOL, endFigure)(THIS_ BOOL bClose) PURE;

    /**
     * getLength
     * @brief    Get the length of the path.
     * @return   float -- The length of the path.
     */
    STDMETHOD_(float, getLength)(CTHIS) SCONST PURE;

    /**
     * getPosTan
     * @brief    Get the position and tangent at a given distance along the path.
     * @param distance The distance along the path.
     * @param pos      The structure to receive the position.
     * @param vec      The structure to receive the tangent vector.
     * @return   BOOL -- TRUE if successful.
     */
    STDMETHOD_(BOOL, getPosTan)(CTHIS_ float distance, fPoint *pos, fPoint *vec) SCONST PURE;

    /**
     * close
     * @brief    Close the current path to stop editing.
     */
    STDMETHOD_(void, close)(THIS) PURE;

    /**
     * hitTest
     * @brief    Test if a point hits the path.
     * @param x  The x-coordinate of the point.
     * @param y  The y-coordinate of the point.
     * @return   BOOL -- TRUE if the point hits the path.
     */
    STDMETHOD_(BOOL, hitTest)(CTHIS_ float x, float y) SCONST PURE;

    /**
     * hitTestStroke
     * @brief    Test if a point hits the stroked path.
     * @param x         The x-coordinate of the point.
     * @param y         The y-coordinate of the point.
     * @param strokeSize The stroke size.
     * @return   BOOL -- TRUE if the point hits the stroked path.
     */
    STDMETHOD_(BOOL, hitTestStroke)(CTHIS_ float x, float y, float strokeSize) SCONST PURE;
};

/**
 * @struct _GradientInfo
 * @brief Structure to define gradient information for creating gradient brushes.
 */
typedef struct _GradientInfo
{
    GradientType type;
    union {
        float angle; // for linear
        struct
        {
            float radius;  // Radius for radial gradient.
            float centerX; // X-coordinate of the center (0.0 -> 1.0, 0.5 is center).
            float centerY; // Y-coordinate of the center (0.0 -> 1.0, 0.5 is center).
        } radial;          // Parameters for radial gradient.
        struct
        {
            BOOL bFullArc; // True if the sweep gradient is rendered for a full circle.
            float centerX; // X-coordinate of the center (0.0 -> 1.0, 0.5 is center).
            float centerY; // Y-coordinate of the center (0.0 -> 1.0, 0.5 is center).
        } sweep;           // Parameters for sweep gradient.
    };
} GradientInfo;

/**
 * @struct IRenderTarget
 * @brief Interface for rendering target objects.
 *
 * @details This interface provides methods for drawing, clipping, transforming, and managing rendering resources
 * such as pens, brushes, and bitmaps. It also supports advanced operations like anti-aliasing, blending, and path-based drawing.
 */
#undef INTERFACE
#define INTERFACE IRenderTarget
DECLARE_INTERFACE_(IRenderTarget, IObjRef)
{
    /**
     * @brief Increment the reference count.
     * @return New reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count and destroy the object if the count reaches zero.
     * @return New reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Called when the reference count reaches zero.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Start a drawing operation.
     */
    STDMETHOD_(void, BeginDraw)(THIS) PURE;

    /**
     * @brief End a drawing operation.
     */
    STDMETHOD_(void, EndDraw)(THIS) PURE;

    /**
     * @brief Check if the render target is offscreen.
     * @return TRUE if offscreen, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsOffscreen)(CTHIS) SCONST PURE;

    /**
     * @brief Create a pen object.
     * @param iStyle Pen style.
     * @param cr Pen color.
     * @param cWidth Pen width.
     * @param ppPen Pointer to the created pen object.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, CreatePen)(THIS_ int iStyle, COLORREF cr, int cWidth, IPenS **ppPen) PURE;

    /**
     * @brief Create a solid color brush.
     * @param cr Brush color.
     * @param ppBrush Pointer to the created brush object.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, CreateSolidColorBrush)(THIS_ COLORREF cr, IBrushS * *ppBrush) PURE;

    /**
     * @brief Create a bitmap brush.
     * @param pBmp Bitmap object.
     * @param xtm X-axis tile mode.
     * @param ytm Y-axis tile mode.
     * @param ppBrush Pointer to the created brush object.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, CreateBitmapBrush)(THIS_ IBitmapS * pBmp, TileMode xtm, TileMode ytm, IBrushS * *ppBrush) PURE;

    /**
     * @brief Create a gradient brush.
     * @param pGradients Array of gradient items.
     * @param nCount Number of gradient items.
     * @param info Gradient information.
     * @param byAlpha Alpha value for the gradient.
     * @param tileMode Tile mode for the gradient.
     * @param ppBrush Pointer to the created brush object.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, CreateGradientBrush)(THIS_ const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha, TileMode tileMode, IBrushS **ppBrush) PURE;

    /**
     * @brief Create a region object.
     * @param ppRegion Pointer to the created region object.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, CreateRegion)(THIS_ IRegionS * *ppRegion) PURE;

    /**
     * @brief Resize the render target.
     * @param sz New size.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, Resize)(THIS_ SIZE sz) PURE;

    /**
     * @brief Offset the viewport origin.
     * @param xOff Offset in the X direction.
     * @param yOff Offset in the Y direction.
     * @param lpPoint Optional pointer to receive the previous viewport origin.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, OffsetViewportOrg)(THIS_ int xOff, int yOff, LPPOINT lpPoint DEF_VAL(NULL)) PURE;

    /**
     * @brief Get the current viewport origin.
     * @param lpPoint Pointer to receive the viewport origin.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, GetViewportOrg)(THIS_ LPPOINT lpPoint) PURE;

    /**
     * @brief Set the viewport origin.
     * @param pt New viewport origin.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, SetViewportOrg)(THIS_ POINT pt) PURE;

    /**
     * @brief Push a rectangular clip region.
     * @param pRect Rectangle defining the clip region.
     * @param mode Region operation mode.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, PushClipRect)(THIS_ LPCRECT pRect, UINT mode DEF_VAL(RGN_AND)) PURE;

    /**
     * @brief Push a region-based clip.
     * @param pRegion Region object.
     * @param mode Region operation mode.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, PushClipRegion)(THIS_ IRegionS * pRegion, UINT mode DEF_VAL(RGN_AND)) PURE;

    /**
     * @brief Pop the last clip region from the stack.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, PopClip)(THIS) PURE;

    /**
     * @brief Exclude a rectangle from the current clip region.
     * @param pRc Rectangle to exclude.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, ExcludeClipRect)(THIS_ LPCRECT pRc) PURE;

    /**
     * @brief Save the current clip state.
     * @param pnState Pointer to receive the clip state identifier.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, SaveClip)(THIS_ int *pnState) PURE;

    /**
     * @brief Restore a previously saved clip state.
     * @param nState Clip state identifier. If -1, restores the last saved state.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, RestoreClip)(THIS_ int nState DEF_VAL(-1)) PURE;

    /**
     * @brief Get the current clip region.
     * @param ppRegion Pointer to receive the clip region object.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, GetClipRegion)(THIS_ IRegionS * *ppRegion) PURE;

    /**
     * @brief Get the bounding box of the current clip region.
     * @param prc Pointer to receive the bounding box rectangle.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, GetClipBox)(THIS_ LPRECT prc) PURE;

    /**
     * @brief Draw text within a rectangle.
     * @param pszText Text to draw.
     * @param cchLen Length of the text.
     * @param pRc Rectangle defining the drawing area.
     * @param uFormat Text formatting options.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawText)(THIS_ LPCTSTR pszText, int cchLen, LPRECT pRc, UINT uFormat) PURE;

    /**
     * @brief Measure the size of the text.
     * @param pszText Text to measure.
     * @param cchLen Length of the text.
     * @param psz Pointer to receive the measured size.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, MeasureText)(THIS_ LPCTSTR pszText, int cchLen, SIZE *psz) PURE;

    /**
     * @brief Output text at a specified position.
     * @param x X-coordinate of the text position.
     * @param y Y-coordinate of the text position.
     * @param lpszString Text to output.
     * @param nCount Number of characters to output.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, TextOut)(THIS_ int x, int y, LPCTSTR lpszString, int nCount) PURE;

    /**
     * @brief Draw a rectangle outline.
     * @param pRect Rectangle to draw.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawRectangle)(THIS_ LPCRECT pRect) PURE;

    /**
     * @brief Fill a rectangle with the current brush.
     * @param pRect Rectangle to fill.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, FillRectangle)(THIS_ LPCRECT pRect) PURE;

    /**
     * @brief Fill a rectangle with a solid color.
     * @param pRect Rectangle to fill.
     * @param cr Solid color.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, FillSolidRect)(THIS_ LPCRECT pRect, COLORREF cr) PURE;

    /**
     * @brief Draw a rounded rectangle outline.
     * @param pRect Rectangle defining the bounds of the rounded rectangle.
     * @param pt X and Y radii for the rounded corners.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawRoundRect)(THIS_ LPCRECT pRect, POINT pt) PURE;

    /**
     * @brief Fill a rounded rectangle with the current brush.
     * @param pRect Rectangle defining the bounds of the rounded rectangle.
     * @param pt X and Y radii for the rounded corners.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, FillRoundRect)(THIS_ LPCRECT pRect, POINT pt) PURE;

    /**
     * @brief Fill a rounded rectangle with a solid color.
     * @param pRect Rectangle defining the bounds of the rounded rectangle.
     * @param pt X and Y radii for the rounded corners.
     * @param cr Solid color.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, FillSolidRoundRect)(THIS_ LPCRECT pRect, POINT pt, COLORREF cr) PURE;

    /**
     * @brief Clear a rectangle with a solid color.
     * @param pRect Rectangle to clear.
     * @param cr Clear color.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, ClearRect)(THIS_ LPCRECT pRect, COLORREF cr) PURE;

    /**
     * @brief Invert the colors in a rectangle.
     * @param pRect Rectangle to invert.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, InvertRect)(THIS_ LPCRECT pRect) PURE;

    /**
     * @brief Draw an ellipse outline.
     * @param pRect Rectangle defining the bounds of the ellipse.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawEllipse)(THIS_ LPCRECT pRect) PURE;

    /**
     * @brief Fill an ellipse with the current brush.
     * @param pRect Rectangle defining the bounds of the ellipse.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, FillEllipse)(THIS_ LPCRECT pRect) PURE;

    /**
     * @brief Fill an ellipse with a solid color.
     * @param pRect Rectangle defining the bounds of the ellipse.
     * @param cr Solid color.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, FillSolidEllipse)(THIS_ LPCRECT pRect, COLORREF cr) PURE;

    /**
     * @brief Draw an arc.
     * @param pRect Rectangle defining the bounds of the arc.
     * @param startAngle Starting angle (in degrees) where the arc begins.
     * @param sweepAngle Sweep angle (in degrees) measured clockwise.
     * @param useCenter Whether to use the center point.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawArc)(THIS_ LPCRECT pRect, float startAngle, float sweepAngle, BOOL useCenter) PURE;

    /**
     * @brief Draw an arc with a specified line width.
     * @param pRect Rectangle defining the bounds of the arc.
     * @param startAngle Starting angle (in degrees) where the arc begins.
     * @param sweepAngle Sweep angle (in degrees) measured clockwise.
     * @param width Line width of the arc.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawArc2)(THIS_ LPCRECT pRect, float startAngle, float sweepAngle, int width) PURE;

    /**
     * @brief Fill an arc.
     * @param pRect Rectangle defining the bounds of the arc.
     * @param startAngle Starting angle (in degrees) where the arc begins.
     * @param sweepAngle Sweep angle (in degrees) measured clockwise.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, FillArc)(THIS_ LPCRECT pRect, float startAngle, float sweepAngle) PURE;

    /**
     * @brief Draw a series of connected lines.
     * @param pPt Array of points defining the lines.
     * @param nCount Number of points.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawLines)(THIS_ LPPOINT pPt, size_t nCount) PURE;

    /**
     * @brief Draw a single line.
     * @param pt1 Starting point of the line.
     * @param pt2 Ending point of the line.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawLine)(THIS_ POINT pt1, POINT pt2) PURE;
    
    /**
     * @brief Draw a polygon.
     * @param pPt Array of points that define the polygon.
     * @param nCount Number of points in the array.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawPolygon)(THIS_ LPPOINT pPt, size_t nCount) PURE;

    /**
     * @brief Fill a polygon.
     * @param pPt Array of points that define the polygon.
     * @param nCount Number of points in the array.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, FillPolygon)(THIS_ LPPOINT pPt, size_t nCount) PURE;


    /**
     * @brief Draw a gradient-filled rectangle.
     * @param pRect Rectangle to fill.
     * @param bVert Vertical gradient flag.
     * @param ptRoundCorner X and Y radii for rounded corners.
     * @param pGradients Array of gradient items.
     * @param nCount Number of gradient items.
     * @param byAlpha Alpha value for the gradient.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawGradientRect)(THIS_ LPCRECT pRect, BOOL bVert, POINT ptRoundCorner, const GradientItem *pGradients, int nCount, BYTE byAlpha DEF_VAL(0xFF)) PURE;

    /**
     * @brief Draw an extended gradient-filled rectangle.
     * @param pRect Rectangle to fill.
     * @param ptRoundCorner X and Y radii for rounded corners.
     * @param pGradients Array of gradient items.
     * @param nCount Number of gradient items.
     * @param info Gradient information.
     * @param byAlpha Alpha value for the gradient.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawGradientRectEx)(THIS_ LPCRECT pRect, POINT ptRoundCorner, const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha DEF_VAL(0xFF)) PURE;

    /**
     * @brief Draw a gradient-filled path.
     * @param path Path object.
     * @param pGradients Array of gradient items.
     * @param nCount Number of gradient items.
     * @param info Gradient information.
     * @param byAlpha Alpha value for the gradient.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawGradientPath)(THIS_ const IPathS *path, const GradientItem *pGradients, int nCount, const GradientInfo *info, BYTE byAlpha DEF_VAL(0xFF)) PURE;

    /**
     * @brief Draw an icon.
     * @param xLeft Left position of the icon.
     * @param yTop Top position of the icon.
     * @param hIcon Icon handle.
     * @param cxWidth Width of the icon.
     * @param cyWidth Height of the icon.
     * @param diFlags Drawing flags.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawIconEx)(THIS_ int xLeft, int yTop, HICON hIcon, int cxWidth, int cyWidth, UINT diFlags) PURE;

    /**
     * @brief Draw a bitmap.
     * @param pRcDest Destination rectangle.
     * @param pBitmap Bitmap object.
     * @param xSrc Source X position.
     * @param ySrc Source Y position.
     * @param byAlpha Alpha value for blending.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawBitmap)(THIS_ LPCRECT pRcDest, const IBitmapS *pBitmap, int xSrc, int ySrc, BYTE byAlpha DEF_VAL(0xFF)) PURE;

    /**
     * @brief Draws a bitmap with expansion mode.
     * @param pRcDest Destination rectangle for the bitmap.
     * @param pBitmap Pointer to the bitmap object to draw.
     * @param pRcSrc Source rectangle within the bitmap.
     * @param expendMode Expansion mode (e.g., stretch, tile).
     * @param byAlpha Optional alpha value for blending (default is 0xFF).
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawBitmapEx)
    (THIS_ LPCRECT pRcDest, const IBitmapS *pBitmap, LPCRECT pRcSrc, UINT expendMode, BYTE byAlpha DEF_VAL(0xFF)) PURE;

    /**
     * @brief Draws a 9-patch bitmap.
     * @param pRcDest Destination rectangle for the bitmap.
     * @param pBitmap Pointer to the bitmap object to draw.
     * @param pRcSrc Source rectangle within the bitmap.
     * @param pRcSourMargin Margins for the 9-patch source.
     * @param expendMode Expansion mode (e.g., stretch, tile).
     * @param byAlpha Optional alpha value for blending (default is 0xFF).
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawBitmap9Patch)
    (THIS_ LPCRECT pRcDest, const IBitmapS *pBitmap, LPCRECT pRcSrc, LPCRECT pRcSourMargin, UINT expendMode, BYTE byAlpha DEF_VAL(0xFF)) PURE;

    /**
     * @brief Performs a bit-block transfer (BitBlt) operation.
     * @param pRcDest Destination rectangle for the BitBlt operation.
     * @param pRTSour Pointer to the source render target.
     * @param xSrc X-coordinate of the source point.
     * @param ySrc Y-coordinate of the source point.
     * @param dwRop Raster operation code (default is kSrcCopy).
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, BitBlt)
    (THIS_ LPCRECT pRcDest, IRenderTarget * pRTSour, int xSrc, int ySrc, DWORD dwRop DEF_VAL(kSrcCopy)) PURE;

    /**
     * @brief Performs an alpha-blended transfer from one render target to another.
     * @param pRcDest Destination rectangle for the alpha blend.
     * @param pRTSrc Pointer to the source render target.
     * @param pRcSrc Source rectangle for the alpha blend.
     * @param byAlpha Alpha value for blending.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, AlphaBlend)
    (THIS_ LPCRECT pRcDest, IRenderTarget * pRTSrc, LPCRECT pRcSrc, BYTE byAlpha) PURE;

    /**
     * @brief Retrieves the current rendering object of the specified type.
     * @param uType Type of the rendering object (e.g., pen, brush).
     * @return Pointer to the current rendering object.
     */
    STDMETHOD_(IRenderObj *, GetCurrentObject)(THIS_ OBJTYPE uType) PURE;

    /**
     * @brief Resets the specified rendering object to its default state.
     * @param objType Type of the rendering object to reset.
     * @param pOldObj Optional pointer to receive the previous object (default is NULL).
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, SelectDefaultObject)
    (THIS_ OBJTYPE objType, IRenderObj * *pOldObj DEF_VAL(NULL)) PURE;

    /**
     * @brief Selects a new rendering object and optionally retrieves the previous one.
     * @param pObj Pointer to the new rendering object.
     * @param pOldObj Optional pointer to receive the previous object (default is NULL).
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, SelectObject)
    (THIS_ IRenderObj * pObj, IRenderObj * *pOldObj DEF_VAL(NULL)) PURE;

    /**
     * @brief Retrieves the current text color.
     * @return COLORREF representing the current text color.
     */
    STDMETHOD_(COLORREF, GetTextColor)(THIS) PURE;

    /**
     * @brief Sets the current text color.
     * @param color New text color.
     * @return COLORREF representing the previous text color.
     */
    STDMETHOD_(COLORREF, SetTextColor)(THIS_ COLORREF color) PURE;

    /**
     * @brief Sets the mask filter for anti-aliasing effects.
     * @param pMaskFilter Pointer to the mask filter object.
     */
    STDMETHOD_(void, SetMaskFilter)(THIS_ IMaskFilter * pMaskFilter) PURE;

    /**
     * @brief Retrieves the current mask filter.
     * @return Pointer to the current mask filter object.
     */
    STDMETHOD_(IMaskFilter *, GetMaskFilter)(THIS) PURE;

    /**
     * @brief Retrieves a device context (DC) compatible with the render target.
     * @param uFlag Flags for DC retrieval.
     * @return Handle to the device context (HDC).
     */
    STDMETHOD_(HDC, GetDC)(THIS_ UINT uFlag) PURE;

    /**
     * @brief Releases a previously retrieved device context (DC).
     * @param hdc Handle to the device context to release.
     * @param pRc Optional pointer to a rectangle defining the area to update (default is NULL).
     */
    STDMETHOD_(void, ReleaseDC)(THIS_ HDC hdc, LPCRECT pRc DEF_VAL(NULL)) PURE;

    /**
     * @brief Sets the coordinate transformation matrix.
     * @param matrix Array of 9 floats representing the new 3x3 transformation matrix.
     * @param oldMatrix Optional array of 9 floats to store the previous transformation matrix (default is NULL).
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, SetTransform)
    (THIS_ const float matrix[9], float oldMatrix[9] DEF_VAL(NULL)) PURE;

    /**
     * @brief Retrieves the current coordinate transformation matrix.
     * @param matrix Array of 9 floats to store the current 2x3 transformation matrix.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, GetTransform)(CTHIS_ float matrix[9]) SCONST PURE;

    /**
     * @brief Retrieves the color of a specific pixel.
     * @param x X-coordinate of the pixel.
     * @param y Y-coordinate of the pixel.
     * @return COLORREF representing the color of the pixel.
     */
    STDMETHOD_(COLORREF, GetPixel)(THIS_ int x, int y) PURE;

    /**
     * @brief Sets the color of a specific pixel.
     * @param x X-coordinate of the pixel.
     * @param y Y-coordinate of the pixel.
     * @param cr New color for the pixel.
     * @return COLORREF representing the previous color of the pixel.
     */
    STDMETHOD_(COLORREF, SetPixel)(THIS_ int x, int y, COLORREF cr) PURE;

    /**
     * @brief Modifies the current clipping region using a path.
     * @param path Pointer to the path object to combine with the current clip.
     * @param mode Region operation mode (e.g., RGN_AND, RGN_OR).
     * @param doAntiAlias Boolean indicating whether anti-aliasing should be applied (default is FALSE).
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, PushClipPath)
    (THIS_ const IPathS *path, UINT mode, BOOL doAntiAlias DEF_VAL(FALSE)) PURE;

    /**
     * @brief Draws the outline of a path using the currently selected pen.
     * @param path Pointer to the path object to draw.
     * @param pathEffect Optional pointer to a path effect object (default is NULL).
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, DrawPath)
    (THIS_ const IPathS *path, IPathEffect *pathEffect DEF_VAL(NULL)) PURE;

    /**
     * @brief Fills the interior of a path using the currently selected brush.
     * @param path Pointer to the path object to fill.
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, FillPath)(THIS_ const IPathS *path) PURE;

    /**
     * @brief Sets the transfer mode for drawing operations.
     * @param mode New transfer mode.
     * @param pOldMode Optional pointer to store the previous transfer mode (default is NULL).
     * @return HRESULT indicating success or failure.
     */
    STDMETHOD_(HRESULT, SetXfermode)(THIS_ int mode, int *pOldMode DEF_VAL(NULL)) PURE;

    /**
     * @brief Enables or disables anti-aliasing for drawing operations.
     * @param bAntiAlias Boolean indicating whether anti-aliasing should be enabled.
     * @return BOOL indicating the previous anti-aliasing state.
     */
    STDMETHOD_(BOOL, SetAntiAlias)(THIS_ BOOL bAntiAlias) PURE;

    /**
     * @brief Retrieves the current anti-aliasing state.
     * @return BOOL indicating whether anti-aliasing is enabled.
     */
    STDMETHOD_(BOOL, GetAntiAlias)(CTHIS) SCONST PURE;
};

/**
 * @struct     IRenderFactory
 * @brief      RenderFactory object
 *
 * Describe    This interface defines a factory for creating various rendering objects and managing resources.
 */
#undef INTERFACE
#define INTERFACE IRenderFactory
DECLARE_INTERFACE_(IRenderFactory, IObjRef)
{
    /**
     * @brief Increments the reference count of the object.
     * @return long - The new reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrements the reference count of the object.
     * @return long - The new reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Final release of the object, performing cleanup if necessary.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Retrieves the image decoder factory associated with this render factory.
     * @return IImgDecoderFactory* - Pointer to the image decoder factory.
     */
    STDMETHOD_(IImgDecoderFactory *, GetImgDecoderFactory)(THIS) PURE;

    /**
     * @brief Sets the image decoder factory for this render factory.
     * @param pImgDecoderFac - Pointer to the new image decoder factory.
     */
    STDMETHOD_(void, SetImgDecoderFactory)(THIS_ IImgDecoderFactory * pImgDecoderFac) PURE;

    /**
     * @brief Creates a render target with specified dimensions.
     * @param ppRenderTarget - Pointer to receive the created render target.
     * @param nWid - Width of the render target (default is 0).
     * @param nHei - Height of the render target (default is 0).
     * @return BOOL - TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, CreateRenderTarget)
    (THIS_ IRenderTarget * *ppRenderTarget, int nWid DEF_VAL(0), int nHei DEF_VAL(0)) PURE;

    /**
     * @brief Creates a render target associated with a specific window handle.
     * @param ppRenderTarget - Pointer to receive the created render target.
     * @param hWnd - Handle to the window.
     * @return BOOL - TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, CreateRenderTarget2)
    (THIS_ IRenderTarget * *ppRenderTarget, HWND hWnd) PURE;

    /**
     * @brief Creates a font object with specified attributes.
     * @param ppFont - Pointer to receive the created font object.
     * @param lf - Pointer to the LOGFONT structure defining the font attributes.
     * @return BOOL - TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, CreateFont)(THIS_ IFontS * *ppFont, const LOGFONT *lf) PURE;

    /**
     * @brief Creates a bitmap object.
     * @param ppBitmap - Pointer to receive the created bitmap object.
     * @return BOOL - TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, CreateBitmap)(THIS_ IBitmapS * *ppBitmap) PURE;

    /**
     * @brief Creates a region object.
     * @param ppRgn - Pointer to receive the created region object.
     * @return BOOL - TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, CreateRegion)(THIS_ IRegionS * *ppRgn) PURE;

    /**
     * @brief Creates a path object.
     * @param ppPath - Pointer to receive the created path object.
     * @return BOOL - TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, CreatePath)(THIS_ IPathS * *ppPath) PURE;

    /**
     * @brief Creates a blur mask filter with specified parameters.
     * @param radius - Blur radius.
     * @param style - Blur style (e.g., normal, solid, outer, inner).
     * @param flag - Blur flags (e.g., ignore transform, high quality).
     * @param ppMaskFilter - Pointer to receive the created mask filter.
     * @return HRESULT - S_OK if successful, error code otherwise.
     */
    STDMETHOD_(HRESULT, CreateBlurMaskFilter)
    (THIS_ float radius, BlurStyle style, BlurFlags flag, IMaskFilter **ppMaskFilter) PURE;

    /**
     * @brief Creates an emboss mask filter with specified parameters.
     * @param direction - Array of 3 floats defining the light direction.
     * @param ambient - Ambient light intensity.
     * @param specular - Specular light intensity.
     * @param blurRadius - Blur radius for the emboss effect.
     * @param ppMaskFilter - Pointer to receive the created mask filter.
     * @return HRESULT - S_OK if successful, error code otherwise.
     */
    STDMETHOD_(HRESULT, CreateEmbossMaskFilter)
    (THIS_ float direction[3], float ambient, float specular, float blurRadius, IMaskFilter **ppMaskFilter) PURE;

    /**
     * @brief Creates a path effect with specified GUID.
     * @param guidEffect - GUID identifying the path effect type.
     * @param ppPathEffect - Pointer to receive the created path effect.
     * @return BOOL - TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, CreatePathEffect)(THIS_ REFGUID guidEffect, IPathEffect * *ppPathEffect) PURE;

    /**
     * @brief Retrieves the default font object.
     * @return IFontS* - Pointer to the default font object.
     */
    STDMETHOD_(IFontS *, GetDefFont)(CTHIS) PURE;
};

#ifdef __cplusplus
// Simplify naming conventions for compatibility with SOUI3 without conflicting with system interfaces.
typedef IFontS IFont;
typedef IPenS IPen;
typedef IBrushS IBrush;
typedef IBitmapS IBitmap;
typedef IRegionS IRegion;
typedef IPathS IPath;
#endif //__cplusplus

SNSEND

/**
 * @brief Font fallback callback interface, specific to Skia rendering.
 * @param u8FontName - Current font name in UTF-8 encoding.
 * @param pWord - Character that cannot be rendered.
 * @param wordLen - Length of the character (1 or 2).
 * @param u8FontNameFallback - Output buffer for the fallback font name in UTF-8 encoding.
 * @param charset - Output parameter for the charset attribute of the fallback font.
 * @return TRUE if a fallback font is found, FALSE otherwise.
 */
typedef BOOL (*FontFallback)(LPCSTR u8FontName, const wchar_t *pWord, size_t wordLen, char u8FontNameFallback[100], int *charset);

#endif // __SRENDER_I__H__