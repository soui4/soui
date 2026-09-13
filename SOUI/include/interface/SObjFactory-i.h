#ifndef __SOBJFACTORY_I__H__
#define __SOBJFACTORY_I__H__
#include <interface/obj-ref-i.h>

#define MAX_OBJNAME 100

typedef struct _SObjectInfo
{
    wchar_t szName[MAX_OBJNAME];
    LPCWSTR szAlise;
    int nType;
} SObjectInfo;

SNSBEGIN
#undef INTERFACE
#define INTERFACE IObjectFactory
DECLARE_INTERFACE_(IObjectFactory, IObjRef)
{
    /**
     * @brief Increment reference count
     * @return long - new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement reference count
     * @return long - new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Create new object
     * @return IObject* - pointer to newly created object
     */
    STDMETHOD_(IObject *, NewObject)(CTHIS) SCONST PURE;

    /**
     * @brief Get base class name
     * @return LPCWSTR - base class name
     */
    STDMETHOD_(LPCWSTR, BaseClassName)(CTHIS) SCONST PURE;

    /**
     * @brief Get object info
     * @return SObjectInfo - object info structure
     */
    STDMETHOD_(void, GetObjectInfo)(CTHIS_ SObjectInfo * info) SCONST PURE;

    /**
     * @brief Clone object factory
     * @return IObjectFactory* - pointer to cloned object factory
     */
    STDMETHOD_(IObjectFactory *, Clone)(CTHIS) SCONST PURE;
};
SNSEND

EXTERN_C SOUI_EXP void ObjInfo_New(SObjectInfo *pInfo, LPCWSTR name, int type, LPCWSTR alise);
EXTERN_C SOUI_EXP BOOL ObjInfo_IsValid(const SObjectInfo *pObjInfo);
#endif /**< __SOBJFACTORY_I__H__ */
