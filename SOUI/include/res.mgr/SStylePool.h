#ifndef __SSTYLEPOOL__H__
#define __SSTYLEPOOL__H__

#include <core/SCmnMap.h>
#include <interface/obj-ref-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

/**
 * @file SStylePool.h
 * @brief Style Pool Management
 * @version v1.0
 * @author SOUI团队
 * @date 2014-05-28
 *
 * @details Manages a pool of styles and templates, providing functionality to load, retrieve, and manage styles and templates from XML.
 */

/**
 * @class SStylePool
 * @brief Manages the mapping of style names to XML nodes.
 *
 * @details This class provides functionality to load styles from an XML node and retrieve them by name.
 */
class SOUI_EXP SStylePool
    : public SCmnMap<SXmlNode, SStringW>
    , public TObjRefImpl<IObjRef> {
  public:
    /**
     * @brief Retrieves a style XML node by name.
     * @param strName Name of the style.
     * @return XML node containing the style, or an invalid node if not found.
     */
    SXmlNode GetStyle(const SStringW &strName);

    /**
     * @brief Initializes the style pool from an XML node.
     * @param xmlNode XML node containing the style definitions.
     * @return TRUE if initialization is successful, FALSE otherwise.
     */
    BOOL Init(SXmlNode xmlNode);

  protected:
    SXmlDoc m_xmlDoc; // XML document containing the styles
};

/**
 * @class STemplatePool
 * @brief Manages the mapping of template names to template strings.
 *
 * @details This class provides functionality to load templates from an XML node and retrieve them by name.
 */
class SOUI_EXP STemplatePool
    : public SCmnMap<SStringW, SStringW>
    , public TObjRefImpl<IObjRef> {
  public:
    /**
     * @brief Initializes the template pool from an XML node.
     * @param xmlNode XML node containing the template definitions.
     * @return TRUE if initialization is successful, FALSE otherwise.
     */
    BOOL Init(SXmlNode xmlNode);

    /**
     * @brief Retrieves a template string by name.
     * @param strName Name of the template.
     * @return Template string, or an empty string if not found.
     */
    SStringW GetTemplateString(const SStringW &strName) const;
};

SNSEND

#endif // __SSTYLEPOOL__H__