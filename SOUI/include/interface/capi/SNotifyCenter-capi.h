#ifndef __SNOTIFYCENTER_CAPI_H__
#define __SNOTIFYCENTER_CAPI_H__

#include "../SNotifyCenter-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SNotifyCenter Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* INotifyCenter C API Macros */
#define INotifyCenter_FireEventSync(This, e) \
    ((This)->lpVtbl->FireEventSync(This, e))

#define INotifyCenter_FireEventAsync(This, e) \
    ((This)->lpVtbl->FireEventAsync(This, e))

#define INotifyCenter_RegisterEventMap(This, slot) \
    ((This)->lpVtbl->RegisterEventMap(This, slot))

#define INotifyCenter_UnregisterEventMap(This, slot) \
    ((This)->lpVtbl->UnregisterEventMap(This, slot))

#define INotifyCenter_RunOnUI(This, pRunnable, bSync) \
    ((This)->lpVtbl->RunOnUI(This, pRunnable, bSync))

#define INotifyCenter_RunOnUI2(This, fun, wp, lp, bSync) \
    ((This)->lpVtbl->RunOnUI2(This, fun, wp, lp, bSync))

#ifdef __cplusplus
}
#endif

#endif /* __SNOTIFYCENTER_CAPI_H__ */
