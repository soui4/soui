#ifndef __STIMELINEHANDLER_CAPI_H__
#define __STIMELINEHANDLER_CAPI_H__

#include "../STimelineHandler-i.h"
#include "../STimelineHandlersMgr-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for STimelineHandler Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ITimelineHandler C API Macros */
#define ITimelineHandler_OnNextFrame(This) \
    ((This)->lpVtbl->OnNextFrame(This))

/* ITimelineHandlersMgr C API Macros */
#define ITimelineHandlersMgr_RegisterTimelineHandler(This, pHandler) \
    ((This)->lpVtbl->RegisterTimelineHandler(This, pHandler))

#define ITimelineHandlersMgr_UnregisterTimelineHandler(This, pHandler) \
    ((This)->lpVtbl->UnregisterTimelineHandler(This, pHandler))

#define ITimelineHandlersMgr_RegisterValueAnimator(This, pAnimator) \
    ((This)->lpVtbl->RegisterValueAnimator(This, pAnimator))

#define ITimelineHandlersMgr_UnregisterValueAnimator(This, pAnimator) \
    ((This)->lpVtbl->UnregisterValueAnimator(This, pAnimator))

#ifdef __cplusplus
}
#endif

#endif /* __STIMELINEHANDLER_CAPI_H__ */
