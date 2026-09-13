#ifndef __SANILAYOUT__H__
#define __SANILAYOUT__H__

#include <interface/slayout-i.h>
#include <layout/SAnchorLayoutParamStruct.h>
#include <sobject/Sobject.hpp>

SNSBEGIN

/**
 * @class SAnchorLayoutParam
 * @brief 9-anchor layout parameter class
 */
class SOUI_EXP SAnchorLayoutParam
    : public TObjRefImpl<SObjectImpl<ILayoutParam>>
    , public SAnchorLayoutParamStruct {
    DEF_SOBJECT(SObjectImpl<ILayoutParam>, L"AnchorLayoutParam")

    friend class SAnchorLayout;

    SAutoRefPtr<IPropertyValuesHolder> m_aniPosHolder;
    float m_fAniFraction;

  public:
    /**
     * @brief Constructor
     */
    SAnchorLayoutParam();
    SAnchorLayoutParam(const SAnchorLayoutParam &other);
    ~SAnchorLayoutParam();

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

  protected:
    /**
     * @brief Process size attribute
     * @param strValue attribute value string
     * @param bLoading whether loading
     * @return processing result
     */
    HRESULT OnAttrSize(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief Process position attribute
     * @param strValue attribute value string
     * @param bLoading whether loading
     * @return processing result
     */
    HRESULT OnAttrPos(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief Process offset attribute
     * @param strValue attribute value string
     * @param bLoading whether loading
     * @return processing result
     */
    HRESULT OnAttrOffset(const SStringW &strValue, BOOL bLoading);

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"size", OnAttrSize)         /**< Size */
        ATTR_LAYOUTSIZE(L"width", width, TRUE)   /**< Width */
        ATTR_LAYOUTSIZE(L"height", height, TRUE) /**< Height */
        ATTR_CUSTOM(L"pos", OnAttrPos)           /**< Position */
        ATTR_CUSTOM(L"offset", OnAttrOffset)     /**< Offset */
        ATTR_FLOAT(L"offsetX", pos.fOffsetX, TRUE)
        ATTR_FLOAT(L"offsetY", pos.fOffsetY, TRUE)
    SOUI_ATTRS_BREAK()

  public:
    static BOOL ParsePosition(const SStringW &pos, AnchorPos &AniPos);
};

/**
 * @class SouiLayout
 * @brief Soui layout class
 */
class SOUI_EXP SAnchorLayout : public TObjRefImpl<SObjectImpl<ILayout>> {
    DEF_SOBJECT(SObjectImpl<ILayout>, L"Anchor")

  public:
    typedef POINT(CALLBACK *PFN_Position2Point)(const AnchorPos &pos, const CRect &rcParent, const CSize &szChild, int nScale, void *userData);

    /**
     * @brief Constructor
     */
    SAnchorLayout(void);

    /**
     * @brief Destructor
     */
    ~SAnchorLayout(void);

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

    void SetPosition2PointCallback(PFN_Position2Point pfnAnchor2Pos, void *userData)
    {
        m_pfnPosition2Point = pfnAnchor2Pos;
        m_pUserData = userData;
    }
    PFN_Position2Point GetPosition2PointCallback() const
    {
        return m_pfnPosition2Point;
    }

    static POINT CALLBACK DefaultPosition2Point(const AnchorPos &pos, const CRect &rcParent, const CSize &szChild, int nScale, void *userData);

  protected:
    static CPoint Anchor2Pos(const CRect &rcParent, int type);
    POINT CalcPoint4Animator(const AnchorPos &start, const AnchorPos &end, float fraction, const CRect &rcParent, const CSize &szChild, int nScale) const;

    PFN_Position2Point m_pfnPosition2Point;
    void *m_pUserData; /**< for callback param; */
};

SNSEND

#endif /**< __SANILAYOUT__H__ */