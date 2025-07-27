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

/* IObjRef base interface macros */
#define ITimer_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ITimer_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ITimer_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* ITimer specific interface macros */
#define ITimer_StartTimer(This, nElapse, bRepeat, uData) \
    ((This)->lpVtbl->StartTimer(This, nElapse, bRepeat, uData))

#define ITimer_StartTimer_Simple(This, nElapse, bRepeat) \
    ((This)->lpVtbl->StartTimer(This, nElapse, bRepeat, 0))

#define ITimer_KillTimer(This) \
    ((This)->lpVtbl->KillTimer(This))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 * These functions provide an alternative C-style API
 */

/* IObjRef Helper Functions */
static inline long ITimer_AddRef_C(ITimer* pThis)
{
    return ITimer_AddRef(pThis);
}

static inline long ITimer_Release_C(ITimer* pThis)
{
    return ITimer_Release(pThis);
}

static inline void ITimer_OnFinalRelease_C(ITimer* pThis)
{
    ITimer_OnFinalRelease(pThis);
}

/* ITimer Helper Functions */
static inline BOOL ITimer_StartTimer_C(ITimer* pThis, int nElapse, BOOL bRepeat, LPARAM uData)
{
    return ITimer_StartTimer(pThis, nElapse, bRepeat, uData);
}

static inline BOOL ITimer_StartTimer_Simple_C(ITimer* pThis, int nElapse, BOOL bRepeat)
{
    return ITimer_StartTimer_Simple(pThis, nElapse, bRepeat);
}

static inline void ITimer_KillTimer_C(ITimer* pThis)
{
    ITimer_KillTimer(pThis);
}

/*
 * Convenience macros for common timer operations
 */
#define ITimer_StartOneShotTimer(This, nElapse) \
    ITimer_StartTimer_Simple(This, nElapse, FALSE)

#define ITimer_StartRepeatingTimer(This, nElapse) \
    ITimer_StartTimer_Simple(This, nElapse, TRUE)

#define ITimer_StartOneShotTimerWithData(This, nElapse, uData) \
    ITimer_StartTimer(This, nElapse, FALSE, uData)

#define ITimer_StartRepeatingTimerWithData(This, nElapse, uData) \
    ITimer_StartTimer(This, nElapse, TRUE, uData)

#ifdef __cplusplus
}
#endif

#endif /* __STIMER_CAPI_H__ */
