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

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IRunnable Helper Functions */
static inline long IRunnable_AddRef_C(IRunnable* pThis)
{
    return IRunnable_AddRef(pThis);
}

static inline long IRunnable_Release_C(IRunnable* pThis)
{
    return IRunnable_Release(pThis);
}

static inline void IRunnable_OnFinalRelease_C(IRunnable* pThis)
{
    IRunnable_OnFinalRelease(pThis);
}

static inline IRunnable* IRunnable_clone_C(IRunnable* pThis)
{
    return IRunnable_clone(pThis);
}

static inline void IRunnable_run_C(IRunnable* pThis)
{
    IRunnable_run(pThis);
}

static inline void* IRunnable_getObject_C(IRunnable* pThis)
{
    return IRunnable_getObject(pThis);
}

static inline const char* IRunnable_getClassInfo_C(IRunnable* pThis)
{
    return IRunnable_getClassInfo(pThis);
}

/*
 * Convenience macros for common runnable operations
 */
#define IRunnable_Execute(This) \
    IRunnable_run(This)

#define IRunnable_Clone(This) \
    IRunnable_clone(This)

#define IRunnable_GetHost(This) \
    IRunnable_getObject(This)

#define IRunnable_GetDescription(This) \
    IRunnable_getClassInfo(This)

/*
 * Safe execution pattern
 */
#define IRunnable_SafeExecute(This) \
    do { \
        if (SOUI_IsValidInterface(This)) { \
            IRunnable_run(This); \
        } \
    } while(0)

#define IRunnable_SafeClone(This, ppClone) \
    do { \
        if (SOUI_IsValidInterface(This) && (ppClone)) { \
            *(ppClone) = IRunnable_clone(This); \
        } else if (ppClone) { \
            *(ppClone) = NULL; \
        } \
    } while(0)

/*
 * Reference counting helpers
 */
#define IRunnable_SafeAddRef(This) \
    SOUI_SafeAddRef((IUnknown*)(This))

#define IRunnable_SafeRelease(This) \
    SOUI_SafeRelease((IUnknown**)(This))

#ifdef __cplusplus
}
#endif

#endif /* __SRUNNABLE_CAPI_H__ */
