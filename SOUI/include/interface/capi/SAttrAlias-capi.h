#ifndef __SATTRALIAS_CAPI_H__
#define __SATTRALIAS_CAPI_H__

#include "../SAttrAlias-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SAttrAlias Interface
 * These macros provide C-style function call syntax for C++ interface methods
 * IAttrAlias inherits from IObjRef.
 */

/* IObjRef base interface macros */
#define IAttrAlias_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IAttrAlias_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IAttrAlias_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IAttrAlias specific interface macros */
#define IAttrAlias_GetAttrAlias(This, pszAttr, pszClsName, objType) \
    ((This)->lpVtbl->GetAttrAlias(This, pszAttr, pszClsName, objType))

#ifdef __cplusplus
}
#endif

#endif /* __SATTRALIAS_CAPI_H__ */
