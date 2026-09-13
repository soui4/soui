#ifndef __SATTRSTORAGE_I__H__
#define __SATTRSTORAGE_I__H__
#include <interface/obj-ref-i.h>
#include <interface/sstring-i.h>

SNSBEGIN

typedef struct IWindow IWindow;

#undef INTERFACE
#define INTERFACE IAttrStorage
DECLARE_INTERFACE_(IAttrStorage, IObjRef)
{
    /**
     * @brief Add reference
     * @return long -- reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Release reference
     * @return long -- reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Method to save attribute values when responding to IObject::SetAttribute
     * @param strName const IStringW* -- attribute name
     * @param strValue const IStringW* -- attribute value
     * @param bHandled BOOL -- whether this attribute has already been handled
     * @return void
     */
    STDMETHOD_(void, OnSetAttribute)
    (THIS_ const IStringW *strName, const IStringW *strValue, BOOL bHandled) PURE;

    /**
     * @brief Respond to IObject::GetAttribute calls
     * @param strName const IStringW* -- attribute name
     * @param[out] strValue IStringW* -- returned attribute value
     * @return BOOL -- TRUE: success
     */
    STDMETHOD_(BOOL, OnGetAttribute)(CTHIS_ const IStringW *strName, IStringW *strValue) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IAttrStorageFactory
DECLARE_INTERFACE_(IAttrStorageFactory, IObjRef)
{
    /**
     * @brief Add reference
     * @return long -- reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Release reference
     * @return long -- reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Create an IAttrStorage object
     * @param owner IWindow* -- the host of IAttrStorage
     * @param[out] ppAttrStorage IAttrStorage** -- return value
     * @return HRESULT -- S_OK: success
     */
    STDMETHOD_(HRESULT, CreateAttrStorage)
    (CTHIS_ IWindow * owner, IAttrStorage * *ppAttrStorage) SCONST PURE;
};

SNSEND
#endif /**< __SATTRSTORAGE_I__H__ */
