#ifndef __SACCHELPER_CAPI_H__
#define __SACCHELPER_CAPI_H__

#include "../sacchelper-i.h"

#ifdef _WIN32

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for IAccHelper Interface
 * These macros provide C-style function call syntax for C++ interface methods
 * IAccHelper inherits from IUnknown.
 */

/* IUnknown base interface macros */
#define IAccHelper_QueryInterface(This, riid, ppvObject) \
    ((This)->lpVtbl->QueryInterface(This, riid, ppvObject))

#define IAccHelper_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IAccHelper_Release(This) \
    ((This)->lpVtbl->Release(This))

/* IAccHelper specific interface macros */
#define IAccHelper_SetOwner(This, pOwner) \
    ((This)->lpVtbl->SetOwner(This, pOwner))

#define IAccHelper_GetOwner(This) \
    ((This)->lpVtbl->GetOwner(This))

#ifdef __cplusplus
}
#endif

#endif //_WIN32

#endif /* __SACCHELPER_CAPI_H__ */
