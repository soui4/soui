#ifndef __SOBJDEFATTR__H__
#define __SOBJDEFATTR__H__

#include "core/SSingletonMap.h"

SNSBEGIN

/**
 * @class SObjDefAttr
 * @brief Class for managing default attributes of objects.
 *
 * @details This class manages default attributes for objects using an XML document. It allows initialization from an XML node,
 *          checking if the attribute list is empty, and retrieving default attributes for specific classes.
 */
class SOUI_EXP SObjDefAttr
    : public SCmnMap<SXmlNode, SStringW>
    , public TObjRefImpl<IObjRef> {
  public:
    /**
     * @brief Constructor.
     */
    SObjDefAttr()
    {
    }

    /**
     * @brief Destructor.
     */
    virtual ~SObjDefAttr()
    {
    }

    /**
     * @brief Initializes the default attributes from an XML node.
     * @param xmlNode XML node containing the default attributes.
     * @return TRUE if initialization is successful, FALSE otherwise.
     */
    BOOL Init(SXmlNode xmlNode);

    /**
     * @brief Checks if the attribute list is empty.
     * @return TRUE if the attribute list is empty, FALSE otherwise.
     */
    bool IsEmpty()
    {
        return !m_xmlRoot.root();
    }

    /**
     * @brief Retrieves the default attributes for a specific class.
     * @param pszClassName Name of the class.
     * @return XML node containing the default attributes for the class, or an invalid node if not found.
     */
    SXmlNode GetDefAttribute(LPCWSTR pszClassName);

  protected:
    /**
     * @brief Builds the class attributes from an XML node.
     * @param xmlNode XML node containing the class attributes.
     * @param pszClassName Name of the class.
     */
    void BuildClassAttribute(SXmlNode &xmlNode, LPCWSTR pszClassName);

    SXmlDoc m_xmlRoot; // XML document containing the default attributes
};

SNSEND

#endif // __SOBJDEFATTR__H__