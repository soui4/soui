/** SOUI window layout interface */
#ifndef __SLAYOUT_I__H__
#define __SLAYOUT_I__H__

#include <interface/sobject-i.h>
#include <interface/SPropertyAnimator-i.h>
SNSBEGIN

typedef struct IWindow IWindow;

typedef enum ORIENTATION
{
    Horz,
    Vert,
    Any,
    Both,
} ORIENTATION;

enum
{
    SIZE_UNDEF = -3,
    SIZE_WRAP_CONTENT = -1,
    SIZE_MATCH_PARENT = -2,
    SIZE_SPEC = 0,
};

/**
 * @enum Unit
 * @brief Enumeration of layout size units
 */
typedef enum _Unit
{
    unknow = -1, /**< Unknown unit */
    px = 0,      /**< Pixel */
    dp,          /**< Device-independent pixel */
    dip = dp,    /**< Device-independent pixel (alias) */
    sp           /**< Scaled pixel */
} Unit;

typedef struct _LAYOUTSIZE
{
    float fSize; /**< Size value */
    Unit unit;   /**< Size unit */
} LAYOUTSIZE;

/**
 * @brief Animation state enumeration
 */
typedef enum _ANI_STATE
{
    ANI_START = 0, /**< Animation start state */
    ANI_PROGRESS,  /**< Animation in-progress state */
    ANI_END,       /**< Animation end state */
} ANI_STATE;

#undef INTERFACE
#define INTERFACE ILayoutParam
DECLARE_INTERFACE_(ILayoutParam, IObject)
{
    DEF_OBJ_BASE(ILayoutParam, LayoutParam)
#include <interface/SobjectApi.h>

    //----------------------------------------------------------

    /**
     * @brief Clear data
     * @return
     */
    STDMETHOD_(void, Clear)(THIS) PURE;

    /**
     * @brief Layout fills parent window flag
     * @param orientation ORIENTATION--layout orientation
     * @return TRUE--layout fills parent window
     */
    STDMETHOD_(BOOL, IsMatchParent)(CTHIS_ ORIENTATION orientation) SCONST PURE;

    /**
     * @brief Layout fits window content flag
     * @param orientation ORIENTATION--layout orientation
     * @return TRUE--fits window content flag
     */
    STDMETHOD_(BOOL, IsWrapContent)(CTHIS_ ORIENTATION orientation) SCONST PURE;

    /**
     * @brief Layout specified size flag
     * @param orientation ORIENTATION--layout orientation
     * @return TRUE--specified size
     */
    STDMETHOD_(BOOL, IsSpecifiedSize)(CTHIS_ ORIENTATION orientation) SCONST PURE;

    /**
     * @brief Get specified layout size
     * @param orientation ORIENTATION--layout orientation
     * @return SLayoutSize--layout size
     */
    STDMETHOD_(BOOL, GetSpecifiedSize)(CTHIS_ ORIENTATION orientation, LAYOUTSIZE * pLayoutSize) SCONST PURE;

    /**
     * @brief Set layout size
     * @param orientation ORIENTATION--layout orientation
     * @param layoutSize SLayoutSize--layout size
     * @return
     */
    STDMETHOD_(void, SetSpecifiedSize)(THIS_ ORIENTATION orientation, const LAYOUTSIZE *layoutSize) PURE;

    /**
     * @brief Set layout to fit parent window size
     * @param orientation ORIENTATION--layout orientation
     * @return
     */
    STDMETHOD_(void, SetMatchParent)(THIS_ ORIENTATION orientation) PURE;

    /**
     * @brief Set layout to fit content
     * @param orientation ORIENTATION--layout orientation
     * @return
     */
    STDMETHOD_(void, SetWrapContent)(THIS_ ORIENTATION orientation) PURE;

    /**
     * @brief Update property animator state
     * @param pHolder IPropertyValuesHolder*--property value holder
     * @param fraction float--animation progress (0.0-1.0)
     * @param state ANI_STATE--animation state (ANI_START/ANI_PROGRESS/ANI_END)
     * @note This method is called during animation to handle animation state changes and trigger layout updates
     */
    STDMETHOD_(BOOL, SetAnimatorValue)(THIS_ IPropertyValuesHolder * pHolder, float fraction, ANI_STATE state) PURE;

    /**
     * @brief Get layout structure data
     * @return void* layout structure
     */
    STDMETHOD_(void *, GetRawData)(THIS) PURE;

    /**
     * @brief Clone layout parameters
     * @return ILayoutParam *--layout parameter object
     */
    STDMETHOD_(ILayoutParam *, Clone)(CTHIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE ILayout
DECLARE_INTERFACE_(ILayout, IObject)
{
    DEF_OBJ_BASE(ILayout, Layout)
#include <interface/SobjectApi.h>
    //------------------------------------------------------------------------

    /**
     * @brief Check whether the current layout type matches the layout parameters
     * @param pLayoutParam const ILayoutParam *--layout parameter
     * @return TRUE--matched
     */
    STDMETHOD_(BOOL, IsParamAcceptable)(CTHIS_ const ILayoutParam *pLayoutParam) SCONST PURE;

    /**
     * @brief Layout child windows of the specified control
     * @param pParent IWindow *--layout control
     * @return
     */
    STDMETHOD_(void, LayoutChildren)(THIS_ IWindow * pParent) PURE;

    /**
     * @brief Create a layout parameter object matching the current layout
     * @return ILayoutParam *--layout parameter object
     */
    STDMETHOD_(ILayoutParam *, CreateLayoutParam)(CTHIS) SCONST PURE;

    /**
     * @brief Calculate window size
     * @param pParent const IWindow*--target window
     * @param nWidth int--parent window width
     * @param nHeight int--parent window height
     * @return SIZE--target window size
     */
    STDMETHOD_(SIZE, MeasureChildren)
    (CTHIS_ const IWindow *pParent, int nWidth, int nHeight) SCONST PURE;
};

SNSEND
#endif /**< __SLAYOUT_I__H__ */