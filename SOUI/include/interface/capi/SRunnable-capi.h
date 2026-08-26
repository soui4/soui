#ifndef __SRUNNABLE_CAPI_H__
#define __SRUNNABLE_CAPI_H__

#include "../SRunnable-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SRunnable Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IRunnable C API Macros */
#define IRunnable_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IRunnable_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IRunnable_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IRunnable_clone(This) \
    ((This)->lpVtbl->clone(This))

#define IRunnable_run(This) \
    ((This)->lpVtbl->run(This))

#define IRunnable_getObject(This) \
    ((This)->lpVtbl->getObject(This))

#define IRunnable_getClassInfo(This) \
    ((This)->lpVtbl->getClassInfo(This))

#ifdef __cplusplus
}
#endif

#endif /* __SRUNNABLE_CAPI_H__ */
