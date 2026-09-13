#ifndef __SATTRALIAS_I__H__
#define __SATTRALIAS_I__H__
#include <interface/obj-ref-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IAttrAlias
DECLARE_INTERFACE_(IAttrAlias, IObjRef)
{
    /**
     * @brief Increases the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decreases the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Called when the reference count reaches zero and the object is about to be released.
     * @details This method is intended to perform any necessary cleanup before the object is deleted.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;
    //-----------------------------------------------------------------------------------

    /**
     * @brief Get the attribute alias
     * @param pszAttr -- attribute name
     * @param pszClsName -- class name
     * @param objType -- object type
     * @return LPCWSTR attribute alias
     */
    STDMETHOD_(LPCWSTR, GetAttrAlias)(CTHIS_ LPCWSTR pszAttr, LPCWSTR pszClsName, int objType) SCONST PURE;
};

SNSEND
#endif /**< __SATTRALIAS_I__H__ */