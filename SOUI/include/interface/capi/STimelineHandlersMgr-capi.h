#ifndef __STIMELINEHANDLERSMGR_CAPI_H__
#define __STIMELINEHANDLERSMGR_CAPI_H__

#include "../STimelineHandlersMgr-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ITimelineHandlersMgr interface macros */
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

#endif /* __STIMELINEHANDLERSMGR_CAPI_H__ */