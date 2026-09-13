#ifndef __SGRIDLAYOUT__H__
#define __SGRIDLAYOUT__H__

#include <interface/slayout-i.h>
#include <layout/SGridLayoutParamStruct.h>
#include <sobject/Sobject.hpp>

SNSBEGIN

/** Int = %d StringA */
#define ATTR_GRIDGRAVITY(attribname, varname, allredraw)        \
    if (0 == strAttribName.CompareNoCase(attribname))           \
    {                                                           \
        varname = SGridLayoutParam::parseGridGravity(strValue); \
        hRet = allredraw ? S_OK : S_FALSE;                      \
    }                                                           \
    else

/**
 * @class SGridLayoutParam
 * @brief Grid layout parameter class
 */
class SGridLayoutParam
    : public TObjRefImpl<SObjectImpl<ILayoutParam>>
    , public SGridLayoutParamStruct {
    DEF_SOBJECT(TObjRefImpl<SObjectImpl<ILayoutParam>>, L"GridLayoutParam")

    friend class SGridLayout;

  public:
    /**
     * @brief Parse grid alignment
     * @param strValue alignment string
     * @return parsed alignment enum value
     */
    static GridGravity parseGridGravity(const SStringW &strValue);

    /**
     * @brief Constructor
     */
    SGridLayoutParam();

    /**
     * @brief Clear layout parameters
     */
    STDMETHOD_(void, Clear)(THIS) OVERRIDE;

    /**
     * @brief Check whether it matches parent container size
     * @param orientation direction (horizontal or vertical)
     * @return Returns TRUE if it matches parent container size, otherwise FALSE
     */
    STDMETHOD_(BOOL, IsMatchParent)(THIS_ ORIENTATION orientation) SCONST OVERRIDE;

    /**
     * @brief Check whether it wraps content size
     * @param orientation direction (horizontal or vertical)
     * @return Returns TRUE if it wraps content size, otherwise FALSE
     */
    STDMETHOD_(BOOL, IsWrapContent)(THIS_ ORIENTATION orientation) SCONST OVERRIDE;

    /**
     * @brief Check whether a specific size is specified
     * @param orientation direction (horizontal or vertical)
     * @return Returns TRUE if a specific size is specified, otherwise FALSE
     */
    STDMETHOD_(BOOL, IsSpecifiedSize)(THIS_ ORIENTATION orientation) SCONST OVERRIDE;

    /**
     * @brief Get the specified size
     * @param orientation direction (horizontal or vertical)
     * @return specified size
     */
    STDMETHOD_(BOOL, GetSpecifiedSize)(THIS_ ORIENTATION orientation, LAYOUTSIZE *pLayoutSize) SCONST OVERRIDE;

    /**
     * @brief Set to match parent container size
     * @param orientation direction (horizontal or vertical)
     */
    STDMETHOD_(void, SetMatchParent)(THIS_ ORIENTATION orientation) OVERRIDE;

    /**
     * @brief Set to wrap content size
     * @param orientation direction (horizontal or vertical)
     */
    STDMETHOD_(void, SetWrapContent)(THIS_ ORIENTATION orientation) OVERRIDE;

    /**
     * @brief Set the specified size
     * @param orientation direction (horizontal or vertical)
     * @param layoutSize specified size
     */
    STDMETHOD_(void, SetSpecifiedSize)(THIS_ ORIENTATION orientation, const LAYOUTSIZE *pLayoutSize) OVERRIDE;

    /**
     * @brief Get raw data pointer
     * @return raw data pointer
     */
    STDMETHOD_(void *, GetRawData)(THIS) OVERRIDE;

    /**
     * @brief Clone layout parameters
     * @return pointer to the cloned layout parameter object
     */
    STDMETHOD_(ILayoutParam *, Clone)(THIS) SCONST OVERRIDE;

    /**
     * @brief Update the property animator state
     * @param pHolder IPropertyValuesHolder*--property value holder
     * @param fraction float--animation progress (0.0-1.0)
     * @param state ANI_STATE--animation state (ANI_START/ANI_PROGRESS/ANI_END)
     */
    STDMETHOD_(BOOL, SetAnimatorValue)(THIS_ IPropertyValuesHolder *pHolder, float fraction, ANI_STATE state) OVERRIDE;

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"rowSpan", nRowSpan, TRUE)                                       /**< Row span count */
        ATTR_INT(L"columnSpan", nColSpan, TRUE)                                    /**< Column span count */
        ATTR_LAYOUTSIZE(L"width", width, TRUE)                                     /**< Width */
        ATTR_LAYOUTSIZE(L"height", height, TRUE)                                   /**< Height */
        ATTR_CUSTOM(L"size", OnAttrSize)                                           /**< Size */
        ATTR_GRIDGRAVITY(L"layout_xGravity", layoutGravityX, TRUE)                 /**< Horizontal alignment */
        ATTR_GRIDGRAVITY(L"layout_yGravity", layoutGravityY, TRUE)                 /**< Vertical alignment */
        ATTR_GRIDGRAVITY(L"layout_gravity", layoutGravityX = layoutGravityY, TRUE) /**< Alignment (same for horizontal and vertical) */
        ATTR_FLOAT(L"columnWeight", fColWeight, TRUE)                              /**< Column weight */
        ATTR_FLOAT(L"rowWeight", fRowWeight, TRUE)                                 /**< Row weight */
    SOUI_ATTRS_BREAK()

  protected:
    /**
     * @brief Process size attribute
     * @param strValue attribute value string
     * @param bLoading whether loading
     * @return processing result
     */
    HRESULT OnAttrSize(const SStringW &strValue, BOOL bLoading);
};

/**
 * @class SGridLayout
 * @brief Grid layout class
 */
class SOUI_EXP SGridLayout : public TObjRefImpl<SObjectImpl<ILayout>> {
    DEF_SOBJECT_EX(SObjectImpl<ILayout>, L"gridLayout", L"grid")

  public:
    /**
     * @brief Constructor
     */
    SGridLayout(void);

    /**
     * @brief Destructor
     */
    ~SGridLayout(void);

    /**
     * @brief Check whether the layout parameter is acceptable
     * @param pLayoutParam layout parameter object pointer
     * @return Returns TRUE if acceptable, otherwise FALSE
     */
    STDMETHOD_(BOOL, IsParamAcceptable)(THIS_ const ILayoutParam *pLayoutParam) SCONST OVERRIDE;

    /**
     * @brief Layout child windows
     * @param pParent parent window pointer
     */
    STDMETHOD_(void, LayoutChildren)(THIS_ IWindow *pParent) OVERRIDE;

    /**
     * @brief Create layout parameter object
     * @param pOwner IWindow*--owner window object of the layout parameter
     * @return layout parameter object pointer
     */
    STDMETHOD_(ILayoutParam *, CreateLayoutParam)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Measure child window size
     * @param pParent parent window pointer
     * @param nWidth width
     * @param nHeight height
     * @return measured size
     */
    STDMETHOD_(SIZE, MeasureChildren)
    (THIS_ const IWindow *pParent, int nWidth, int nHeight) SCONST OVERRIDE;

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"columnCount", m_nCols, TRUE)                       /**< Column count */
        ATTR_INT(L"rowCount", m_nRows, TRUE)                          /**< Row count */
        ATTR_LAYOUTSIZE(L"xInterval", m_xInterval, TRUE)              /**< Horizontal spacing */
        ATTR_LAYOUTSIZE(L"yInterval", m_yInterval, TRUE)              /**< Vertical spacing */
        ATTR_LAYOUTSIZE(L"interval", m_xInterval = m_yInterval, TRUE) /**< Spacing (same for horizontal and vertical) */
        ATTR_GRIDGRAVITY(L"xGravity", m_GravityX, TRUE)               /**< Horizontal alignment */
        ATTR_GRIDGRAVITY(L"yGravity", m_GravityY, TRUE)               /**< Vertical alignment */
        ATTR_GRIDGRAVITY(L"gravity", m_GravityX = m_GravityY, TRUE)   /**< Alignment (same for horizontal and vertical) */
        ATTR_ENUM_BEGIN(L"orientation", ORIENTATION, TRUE)            /**< Arrange direction */
            ATTR_ENUM_VALUE(L"horizontal", Horz)                      /**< Horizontal arrangement */
            ATTR_ENUM_VALUE(L"vertical", Vert)                        /**< Vertical arrangement */
        ATTR_ENUM_END(m_orientation)
    SOUI_ATTRS_BREAK()

  protected:
    /**
     * @brief Calculate the number of grid cells
     * @param pParent parent window pointer
     * @return number of grid cells
     */
    int CalcCells(const IWindow *pParent) const;

    int m_nCols;             /**< Column count */
    int m_nRows;             /**< Row count */
    SLayoutSize m_xInterval; /**< Horizontal spacing */
    SLayoutSize m_yInterval; /**< Vertical spacing */

    GridGravity m_GravityX;    /**< Horizontal alignment */
    GridGravity m_GravityY;    /**< Vertical alignment */
    ORIENTATION m_orientation; /**< Arrange direction */
};

SNSEND

#endif /**< __SGRIDLAYOUT__H__ */