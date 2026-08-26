#ifndef __SATTRSTORAGE_CAPI_H__
#define __SATTRSTORAGE_CAPI_H__

#include "../SAttrStorage-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SAttrStorage Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IAttrStorage C API Macros */
#define IAttrStorage_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IAttrStorage_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IAttrStorage_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IAttrStorage_OnSetAttribute(This, strName, strValue, bHandled) \
    ((This)->lpVtbl->OnSetAttribute(This, strName, strValue, bHandled))

#define IAttrStorage_OnGetAttribute(This, strName, strValue) \
    ((This)->lpVtbl->OnGetAttribute(This, strName, strValue))

/* IAttrStorageFactory C API Macros */
#define IAttrStorageFactory_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IAttrStorageFactory_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IAttrStorageFactory_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IAttrStorageFactory_CreateAttrStorage(This, owner, ppAttrStorage) \
    ((This)->lpVtbl->CreateAttrStorage(This, owner, ppAttrStorage))

#ifdef __cplusplus
}
#endif

#endif /* __SATTRSTORAGE_CAPI_H__ */
