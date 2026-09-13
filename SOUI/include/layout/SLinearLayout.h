#ifndef __SLINEARLAYOUT__H__
#define __SLINEARLAYOUT__H__

#include <interface/slayout-i.h>
#include <layout/SLinearLayoutParamStruct.h>
#include <sobject/Sobject.hpp>

SNSBEGIN

/** Int = %d StringA */
#define ATTR_GRAVITY(attribname, varname, allredraw)          \
    if (0 == strAttribName.CompareNoCase(attribname))         \
    {                                                         \
        varname = SLinearLayoutParam::parseGravity(strValue); \
        hRet = allredraw ? S_OK : S_FALSE;                    \
    }                                                         \
    else

/**
 * @class SLinearLayoutParam
 * @brief Linear layout parameter class
 */
class SLinearLayoutParam
    : public TObjRefImpl<SObjectImpl<ILayoutParam>>
    , public SLinearLayoutParamStruct {
    DEF_SOBJECT(SObjectImpl<ILayoutParam>, L"LinearLayoutParam")

    friend class SLinearLayout;

  public:
    static Gravity parseGravity(const SStringW &strValue);

  public:
    /**
     * @brief Constructor
     */
    SLinearLayoutParam();

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
        ATTR_LAYOUTSIZE(L"width", width, TRUE)                  /**< Width */
        ATTR_LAYOUTSIZE(L"height", height, TRUE)                /**< Height */
        ATTR_CUSTOM(L"size", OnAttrSize)                        /**< Size */
        ATTR_FLOAT(L"weight", weight, FALSE)                    /**< Weight */
        ATTR_GRAVITY(L"layout_gravity", gravity, FALSE)         /**< Alignment */
        ATTR_CUSTOM(L"extend", OnAttrExtend)                    /**< Expansion attribute */
        ATTR_LAYOUTSIZE(L"extend_left", extend_left, FALSE)     /**< Left expansion */
        ATTR_LAYOUTSIZE(L"extend_top", extend_top, FALSE)       /**< Top expansion */
        ATTR_LAYOUTSIZE(L"extend_right", extend_right, FALSE)   /**< Right expansion */
        ATTR_LAYOUTSIZE(L"extend_bottom", extend_bottom, FALSE) /**< Bottom expansion */
    SOUI_ATTRS_BREAK()

  protected:
    /**
     * @brief Process size attribute
     * @param strValue attribute value string
     * @param bLoading whether loading
     * @return processing result
     */
    HRESULT OnAttrSize(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief Process expansion attribute
     * @param strValue attribute value string
     * @param bLoading whether loading
     * @return processing result
     */
    HRESULT OnAttrExtend(const SStringW &strValue, BOOL bLoading);
};

/**
 * @class SLinearLayout
 * @brief Linear layout class
 */
class SOUI_EXP SLinearLayout : public TObjRefImpl<SObjectImpl<ILayout>> {
    DEF_SOBJECT(SObjectImpl<ILayout>, L"linearLayout")

  public:
    /**
     * @brief Constructor
     */
    SLinearLayout(void);

    /**
     * @brief Destructor
     */
    ~SLinearLayout(void);

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
        ATTR_ENUM_BEGIN(L"orientation", ORIENTATION, FALSE) /**< Direction */
            ATTR_ENUM_VALUE(L"horizontal", Horz)            /**< Horizontal direction */
            ATTR_ENUM_VALUE(L"vertical", Vert)              /**< Vertical direction */
        ATTR_ENUM_END(m_orientation)
        ATTR_GRAVITY(L"gravity", m_gravity, FALSE)      /**< Alignment */
        ATTR_LAYOUTSIZE(L"interval", m_interval, FALSE) /**< Spacing */
    SOUI_ATTRS_BREAK()

  protected:
    ORIENTATION m_orientation; /**< Direction */
    Gravity m_gravity;         /**< Alignment */
    SLayoutSize m_interval;    /**< Spacing */
};

/**
 * @class SVBox
 * @brief Vertical linear layout class
 */
class SVBox : public SLinearLayout {
    DEF_SOBJECT(SLinearLayout, L"vbox")

  public:
    /**
     * @brief Constructor
     */
    SVBox()
    {
        m_orientation = Vert; // Set direction to vertical
    }
};

/**
 * @class SHBox
 * @brief Horizontal linear layout class
 */
class SHBox : public SLinearLayout {
    DEF_SOBJECT(SLinearLayout, L"hbox")

  public:
    /**
     * @brief Constructor
     */
    SHBox()
    {
        m_orientation = Horz; // Set direction to horizontal
    }
};

SNSEND

#endif /**< __SLINEARLAYOUT__H__ */