#ifndef __STIMER_CAPI_H__
#define __STIMER_CAPI_H__

#include "../STimer-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for ITimer Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ITimer C API Macros (IObjRef base interface methods) */
#define ITimer_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ITimer_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ITimer_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* ITimer C API Macros (ITimer specific methods) */
#define ITimer_StartTimer(This, nElapse, bRepeat, uData) \
    ((This)->lpVtbl->StartTimer(This, nElapse, bRepeat, uData))

#define ITimer_KillTimer(This) \
    ((This)->lpVtbl->KillTimer(This))

#ifdef __cplusplus
}
#endif

#endif /* __STIMER_CAPI_H__ */
