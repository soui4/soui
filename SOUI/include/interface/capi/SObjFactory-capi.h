#ifndef __SOBJFACTORY_CAPI_H__
#define __SOBJFACTORY_CAPI_H__

#include "../SObjFactory-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for IObjectFactory Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IObjRef base interface macros */
#define IObjectFactory_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IObjectFactory_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IObjectFactory_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IObjectFactory specific interface macros */
#define IObjectFactory_NewObject(This) \
    ((This)->lpVtbl->NewObject(This))

#define IObjectFactory_BaseClassName(This) \
    ((This)->lpVtbl->BaseClassName(This))

#define IObjectFactory_GetObjectInfo(This, info) \
    ((This)->lpVtbl->GetObjectInfo(This, info))

#define IObjectFactory_Clone(This) \
    ((This)->lpVtbl->Clone(This))

#ifdef __cplusplus
}
#endif

#endif /* __SOBJFACTORY_CAPI_H__ */
