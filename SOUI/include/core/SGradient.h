#ifndef _SGRADIENT_H_
#define _SGRADIENT_H_

#include <interface/SGradient-i.h>
#include <sobject/Sobject.hpp>
#include <helper/obj-ref-impl.hpp>
#include <souicoll.h>

SNSBEGIN

/**
 * @class      SGradient
 * @brief      Gradient management class
 *
 * @details    This class manages gradient data, including loading and retrieving gradient items.
 *             It implements the `IGradient` interface and provides methods to handle gradient attributes.
 */
class SOUI_EXP SGradient : public TObjRefImpl<SObjectImpl<IGradient>> {
    DEF_SOBJECT(SObjectImpl<IGradient>, L"gradient")

  public:
    /**
     * @brief    Constructor
     *
     * @details  Initializes the gradient object.
     */
    SGradient();

  public:
    /**
     * @brief    Gets the gradient data
     * @return   Pointer to the gradient data
     *
     * @details  Returns a pointer to the array of gradient items.
     */
    STDMETHOD_(const GradientItem *, GetGradientData)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief    Gets the length of the gradient
     * @return   Number of gradient items
     *
     * @details  Returns the number of gradient items in the gradient data.
     */
    STDMETHOD_(int, GetGradientLength)(CTHIS) SCONST OVERRIDE;

  protected:
    /**
     * @brief    Initializes the object after loading from XML
     * @param    pNode  Pointer to the XML node
     * @return   HRESULT indicating success or failure
     *
     * @details  Initializes the gradient object after it has been loaded from an XML node.
     */
    STDMETHOD_(void, OnInitFinished)(THIS_ IXmlNode *pNode) OVERRIDE;

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"colors", OnAttrColors)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief    Handles the "colors" attribute
     * @param    value     Attribute value
     * @param    bLoading  TRUE if loading, FALSE otherwise
     * @return   HRESULT indicating success or failure
     *
     * @details  Processes the "colors" attribute to load gradient colors from the provided value.
     */
    HRESULT OnAttrColors(const SStringW &value, BOOL bLoading);

    /**
     * @brief    Loads the color table from an XML node
     * @param    xmlNode  Pointer to the XML node
     * @return   Number of colors loaded
     *
     * @details  Loads the color table from the specified XML node and returns the number of colors loaded.
     */
    int LoadColorTable(IXmlNode *xmlNode);

  protected:
    SArray<GradientItem> m_arrGradient; /**< Array of gradient items. */
};

SNSEND

#endif // _SGRADIENT_H_