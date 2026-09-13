#ifndef __SOUILAYOUT__H__
#define __SOUILAYOUT__H__

#include <interface/slayout-i.h>
#include <layout/SouiLayoutParamStruct.h>
#include <sobject/Sobject.hpp>

SNSBEGIN

/**
 * @class SouiLayoutParam
 * @brief Soui layout parameter class
 */
class SouiLayoutParam
    : public TObjRefImpl<SObjectImpl<ILayoutParam>>
    , public SouiLayoutParamStruct {
    DEF_SOBJECT(SObjectImpl<ILayoutParam>, L"SouiLayoutParam")

    friend class SouiLayout;

  public:
    /**
     * @brief Constructor
     */
    SouiLayoutParam();

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
    STDMETHOD_(void, SetSpecifiedSize)
    (THIS_ ORIENTATION orientation, const LAYOUTSIZE *layoutSize) OVERRIDE;

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
    /**
     * @brief Check whether offset is needed
     * @param orientation direction (horizontal or vertical)
     * @return Returns TRUE if offset is needed, otherwise FALSE
     */
    bool IsOffsetRequired(ORIENTATION orientation) const;

    /**
     * @brief Get extra size
     * @param orientation direction (horizontal or vertical)
     * @param nScale scale factor
     * @return extra size
     */
    int GetExtraSize(ORIENTATION orientation, int nScale) const;

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
        ATTR_LAYOUTSIZE(L"width", width, TRUE)   /**< Width */
        ATTR_LAYOUTSIZE(L"height", height, TRUE) /**< Height */
        ATTR_CUSTOM(L"pos", OnAttrPos)           /**< Position */
        ATTR_CUSTOM(L"size", OnAttrSize)         /**< Size */
        ATTR_CUSTOM(L"offset", OnAttrOffset)     /**< Offset */
        ATTR_FLOAT(L"offsetX", fOffsetX, TRUE)
        ATTR_FLOAT(L"offsetY", fOffsetY, TRUE)
    SOUI_ATTRS_BREAK()

  protected:
    /**
     * @brief Convert coordinate described by string into POSITION_ITEM
     * @param strPos coordinate string
     * @param posItem reference to position info struct
     * @return Returns TRUE if conversion succeeds, otherwise FALSE
     */
    BOOL StrPos2ItemPos(const SStringW &strPos, POS_INFO &posItem);

    /**
     * @brief Parse the first two positions defined in pos
     * @param pos1 first position string
     * @param pos2 second position string
     * @return Returns TRUE if parsing succeeds, otherwise FALSE
     */
    BOOL ParsePosition12(const SStringW &pos1, const SStringW &pos2);

    /**
     * @brief Parse the last two positions defined in pos
     * @param pos3 third position string
     * @param pos4 fourth position string
     * @return Returns TRUE if parsing succeeds, otherwise FALSE
     */
    BOOL ParsePosition34(const SStringW &pos3, const SStringW &pos4);
};

/**
 * @class SouiLayout
 * @brief Soui layout class
 */
class SOUI_EXP SouiLayout : public TObjRefImpl<SObjectImpl<ILayout>> {
    DEF_SOBJECT(SObjectImpl<ILayout>, L"SouiLayout")

  public:
    /**
     * @brief Constructor
     */
    SouiLayout(void);

    /**
     * @brief Destructor
     */
    ~SouiLayout(void);

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

  protected:
    /**
     * @struct WndPos
     * @brief Window position struct
     */
    struct WndPos
    {
        SWindow *pWnd;     /**< Window pointer */
        CRect rc;          /**< Window rect region */
        bool bWaitOffsetX; /**< Whether waiting for X offset */
        bool bWaitOffsetY; /**< Whether waiting for Y offset */
    };

    /**
     * @brief Calculate expanded position
     * @param pListChildren child window list pointer
     * @param nWidth width
     * @param nHeight height
     */
    void CalcPositionEx(SList<WndPos> *pListChildren, int nWidth, int nHeight) const;

    /**
     * @brief Calculate position
     * @param pListChildren child window list pointer
     * @param nWidth width
     * @param nHeight height
     * @return calculation result
     */
    int CalcPostion(SList<WndPos> *pListChildren, int nWidth, int nHeight) const;

    /**
     * @brief Convert position item to value
     * @param pLstChilds child window list pointer
     * @param position position pointer
     * @param pos position info struct
     * @param nMax max value
     * @param bX whether X direction
     * @param nScale scale factor
     * @return calculated value
     */
    int PositionItem2Value(SList<WndPos> *pLstChilds, SPOSITION position, const POS_INFO &pos, int nMax, BOOL bX, int nScale) const;

    /**
     * @brief Calculate the left position of child window
     * @param pWindow window pointer
     * @param pParam layout parameter object pointer
     * @return calculated left position
     */
    int CalcChildLeft(SWindow *pWindow, SouiLayoutParam *pParam);

    /**
     * @brief Calculate the right position of child window
     * @param pWindow window pointer
     * @param pParam layout parameter object pointer
     * @return calculated right position
     */
    int CalcChildRight(SWindow *pWindow, SouiLayoutParam *pParam);

    /**
     * @brief Calculate the top position of child window
     * @param pWindow window pointer
     * @param pParam layout parameter object pointer
     * @return calculated top position
     */
    int CalcChildTop(SWindow *pWindow, SouiLayoutParam *pParam);

    /**
     * @brief Calculate the bottom position of child window
     * @param pWindow window pointer
     * @param pParam layout parameter object pointer
     * @return calculated bottom position
     */
    int CalcChildBottom(SWindow *pWindow, SouiLayoutParam *pParam);

    /**
     * @brief Check whether the position is waiting
     * @param nPos position value
     * @return Returns TRUE if waiting, otherwise FALSE
     */
    BOOL IsWaitingPos(int nPos) const;

    /**
     * @brief Get the reference sibling window
     * @param pCurWnd current window pointer
     * @param uCode code
     * @return reference sibling window pointer
     */
    SWindow *GetRefSibling(SWindow *pCurWnd, int uCode);

    /**
     * @brief Get the window layout rect region
     * @param pWindow window pointer
     * @return window layout rect region
     */
    CRect GetWindowLayoutRect(SWindow *pWindow);
};

SNSEND

#endif /**< __SOUILAYOUT__H__ */