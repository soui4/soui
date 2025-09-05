#ifndef __SSKINOBJBASE__H__
#define __SSKINOBJBASE__H__

#include <interface/SSkinobj-i.h>
#include <interface/sxml-i.h>
#include <helper/obj-ref-impl.hpp>
#include <sobject/Sobject.hpp>
#include <souicoll.h>

SNSBEGIN

/**
 * @class SState2Index
 * @brief Maps skin states to indices.
 *
 * This class provides functionality to map skin states to their corresponding indices.
 * It can be initialized from an XML node and supports default state-to-index mappings.
 */
class SOUI_EXP SState2Index {
  public:
    /**
     * @brief Default constructor.
     */
    SState2Index()
    {
    }

    /**
     * @brief Copy constructor.
     * @param src Source SState2Index object to copy from.
     */
    SState2Index(const SState2Index &src);

    /**
     * @brief Gets the index for a given state.
     * @param dwState State to get the index for.
     * @param checkAsPushdown Flag to check if the state should be treated as pushdown.
     * @return Index corresponding to the state.
     */
    int GetIndex(DWORD dwState, bool checkAsPushdown) const;

    /**
     * @brief Initializes the state-to-index mapping from an XML node.
     * @param pNode Pointer to the XML node containing the state mappings.
     * @return TRUE if initialization is successful, FALSE otherwise.
     */
    BOOL Init(IXmlNode *pNode);

    /**
     * @brief Gets the default index for a given state.
     * @param dwState State to get the default index for.
     * @param checkAsPushdown Flag to check if the state should be treated as pushdown.
     * @return Default index corresponding to the state.
     */
    static int GetDefIndex(DWORD dwState, bool checkAsPushdown = false);

    /**
     * @brief Converts a string representation of a state to a DWORD value.
     * @param strState String representation of the state.
     * @return DWORD value representing the state.
     */
    static DWORD String2State(const SStringW &strState);

  protected:
    SMap<DWORD, int> m_mapOfStates; // Map of states to indices.
};

/**
 * @class SSkinObjBase
 * @brief Base class for skin objects.
 *
 * This class provides a base implementation for skin objects, which are used to define
 * the visual appearance of UI elements. It includes methods for drawing, scaling, and
 * colorizing skins, as well as managing state-to-index mappings.
 */
class SOUI_EXP SSkinObjBase : public TObjRefImpl<SObjectImpl<ISkinObj> > {
    DEF_SOBJECT(SObjectImpl<ISkinObj>, L"skinObjBase")

  public:
    /**
     * @brief Constructor.
     */
    SSkinObjBase();

    /**
     * @brief Gets the name of the skin object.
     * @return Name of the skin object.
     */
    STDMETHOD_(LPCWSTR, GetName)(THIS) SCONST OVERRIDE;

    /**
     * @brief Draws the skin by state with alpha blending.
     * @param pRT Pointer to the render target.
     * @param rcDraw Rectangle to draw in.
     * @param dwState State to draw.
     * @param byAlpha Alpha value for transparency.
     */
    STDMETHOD_(void, DrawByState2)
    (THIS_ IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState, BYTE byAlpha) SCONST OVERRIDE;

    /**
     * @brief Draws the skin by state without alpha blending.
     * @param pRT Pointer to the render target.
     * @param rcDraw Rectangle to draw in.
     * @param dwState State to draw.
     */
    STDMETHOD_(void, DrawByState)
    (THIS_ IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState) SCONST OVERRIDE;

    /**
     * @brief Draws the skin by index with alpha blending.
     * @param pRT Pointer to the render target.
     * @param rcDraw Rectangle to draw in.
     * @param iState Index of the state to draw.
     * @param byAlpha Alpha value for transparency.
     */
    STDMETHOD_(void, DrawByIndex2)
    (THIS_ IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) SCONST OVERRIDE;

    /**
     * @brief Draws the skin by index without alpha blending.
     * @param pRT Pointer to the render target.
     * @param rcDraw Rectangle to draw in.
     * @param iState Index of the state to draw.
     */
    STDMETHOD_(void, DrawByIndex)
    (THIS_ IRenderTarget *pRT, LPCRECT rcDraw, int iState) SCONST OVERRIDE;

    /**
     * @brief Gets the size of the skin.
     * @return Size of the skin.
     */
    STDMETHOD_(SIZE, GetSkinSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the number of states supported by the skin.
     * @return Number of states.
     */
    STDMETHOD_(int, GetStates)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the alpha value of the skin.
     * @return Alpha value.
     */
    STDMETHOD_(BYTE, GetAlpha)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the alpha value of the skin.
     * @param byAlpha Alpha value to set.
     */
    STDMETHOD_(void, SetAlpha)(THIS_ BYTE byAlpha) OVERRIDE;

    /**
     * @brief Gets the scale factor of the skin.
     * @return Scale factor.
     */
    STDMETHOD_(int, GetScale)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the scale factor of the skin.
     * @param scale Scale factor to set.
     */
    STDMETHOD_(void, SetScale)(THIS_ int scale) OVERRIDE;

    /**
     * @brief Scales the skin.
     * @param nScale Scale factor.
     * @return Pointer to the scaled skin object.
     */
    STDMETHOD_(ISkinObj *, Scale)(THIS_ int nScale) OVERRIDE;

    /**
     * @brief Applies colorization to the skin.
     * @param cr Color to apply for colorization.
     */
    STDMETHOD_(void, OnColorize)(THIS_ COLORREF cr) OVERRIDE;

    /**
     * @brief Converts a state to its corresponding index.
     * @param dwState State to convert.
     * @return Index corresponding to the state.
     */
    int State2Index(DWORD dwState) const;

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"alpha", m_byAlpha, TRUE) // Skin transparency
        ATTR_BOOL(L"enableColorize", m_bEnableColorize, TRUE)
        ATTR_BOOL(L"checkAsPushdown", m_checkAsPushdown, TRUE)
        ATTR_INT(L"scale", m_nScale, FALSE)
        ATTR_BOOL(L"enableScale", m_bEnableScale, TRUE)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Called when initialization is finished.
     * @param pNode Pointer to the XML node.
     */
    STDMETHOD_(void, OnInitFinished)(THIS_ IXmlNode *pNode) OVERRIDE;

    /**
     * @brief Scales the skin object.
     * @param pObj Pointer to the skin object.
     * @param nScale Scale factor.
     */
    virtual void _Scale(ISkinObj *pObj, int nScale);

    /**
     * @brief Draws the skin by state with alpha blending.
     * @param pRT Pointer to the render target.
     * @param rcDraw Rectangle to draw in.
     * @param dwState State to draw.
     * @param byAlpha Alpha value for transparency.
     */
    virtual void _DrawByState(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState, BYTE byAlpha) const;

    /**
     * @brief Draws the skin by index with alpha blending.
     * @param pRT Pointer to the render target.
     * @param rcDraw Rectangle to draw in.
     * @param iState Index of the state to draw.
     * @param byAlpha Alpha value for transparency.
     */
    virtual void _DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const = 0;

  protected:
    BYTE m_byAlpha;             // Alpha value for transparency.
    COLORREF m_crColorize;      // Color for colorization.
    bool m_bEnableColorize;     // Flag to enable colorization.
    int m_nScale;               // Scale factor.
    bool m_bEnableScale;        // Flag to enable scaling.
    bool m_checkAsPushdown;     // Flag to check if the state should be treated as pushdown.
    SState2Index m_state2Index; // State-to-index mapping.
};

SNSEND
#endif // __SSKINOBJBASE__H__