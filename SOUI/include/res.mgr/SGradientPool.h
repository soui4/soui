/**
 * @file SGradientPool.h
 * @brief Gradient Pool Management
 * @version v1.0
 * @author SOUI团队
 * @date 2014-05-28
 *
 * @details Manages a pool of gradients identified by names.
 */

#ifndef __SGRADIENTPOOL__H__
#define __SGRADIENTPOOL__H__

#include <core/SCmnMap.h>
#include <interface/obj-ref-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

/**
 * @class SGradientPool
 * @brief Manages the mapping of gradient names to IGradient objects.
 *
 * @details This class provides functionality to store and retrieve gradient objects by their names.
 *          It inherits from `SCmnMap` to manage the mapping and from `TObjRefImpl<IObjRef>` to handle reference counting.
 */
class SOUI_EXP SGradientPool
    : public SCmnMap<SAutoRefPtr<IGradient>, SStringW>
    , public TObjRefImpl<IObjRef> {
  public:
    /**
     * @brief Retrieves the gradient object by its name.
     * @param strName Name of the gradient.
     * @return Pointer to the IGradient object, or nullptr if not found.
     */
    IGradient *GetGradient(const SStringW &strName);

    /**
     * @brief Initializes the gradient pool from an XML node.
     * @param xmlNode XML node containing the gradient definitions.
     * @return TRUE if initialization is successful, FALSE otherwise.
     */
    BOOL Init(SXmlNode xmlNode);
};

SNSEND

#endif // __SGRADIENTPOOL__H__