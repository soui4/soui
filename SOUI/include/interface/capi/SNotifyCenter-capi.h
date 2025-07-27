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

#define INotifyCenter_RunOnUI_Async(This, pRunnable) \
    ((This)->lpVtbl->RunOnUI(This, pRunnable, FALSE))

#define INotifyCenter_RunOnUI_Sync(This, pRunnable) \
    ((This)->lpVtbl->RunOnUI(This, pRunnable, TRUE))

#define INotifyCenter_RunOnUI2(This, fun, wp, lp, bSync) \
    ((This)->lpVtbl->RunOnUI2(This, fun, wp, lp, bSync))

#define INotifyCenter_RunOnUI2_Async(This, fun, wp, lp) \
    ((This)->lpVtbl->RunOnUI2(This, fun, wp, lp, FALSE))

#define INotifyCenter_RunOnUI2_Sync(This, fun, wp, lp) \
    ((This)->lpVtbl->RunOnUI2(This, fun, wp, lp, TRUE))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* INotifyCenter Helper Functions */
static inline void INotifyCenter_FireEventSync_C(INotifyCenter* pThis, IEvtArgs* e)
{
    INotifyCenter_FireEventSync(pThis, e);
}

static inline void INotifyCenter_FireEventAsync_C(INotifyCenter* pThis, IEvtArgs* e)
{
    INotifyCenter_FireEventAsync(pThis, e);
}

static inline BOOL INotifyCenter_RegisterEventMap_C(INotifyCenter* pThis, const IEvtSlot* slot)
{
    return INotifyCenter_RegisterEventMap(pThis, slot);
}

static inline BOOL INotifyCenter_UnregisterEventMap_C(INotifyCenter* pThis, const IEvtSlot* slot)
{
    return INotifyCenter_UnregisterEventMap(pThis, slot);
}

static inline void INotifyCenter_RunOnUI_C(INotifyCenter* pThis, IRunnable* pRunnable, BOOL bSync)
{
    INotifyCenter_RunOnUI(pThis, pRunnable, bSync);
}

static inline void INotifyCenter_RunOnUI2_C(INotifyCenter* pThis, FunRunOnUI fun, WPARAM wp, LPARAM lp, BOOL bSync)
{
    INotifyCenter_RunOnUI2(pThis, fun, wp, lp, bSync);
}

/*
 * Convenience macros for common notification operations
 */

/* Event firing shortcuts */
#define INotifyCenter_FireSync(This, event) \
    INotifyCenter_FireEventSync(This, event)

#define INotifyCenter_FireAsync(This, event) \
    INotifyCenter_FireEventAsync(This, event)

#define INotifyCenter_PostEvent(This, event) \
    INotifyCenter_FireEventAsync(This, event)

#define INotifyCenter_SendEvent(This, event) \
    INotifyCenter_FireEventSync(This, event)

/* Event map management shortcuts */
#define INotifyCenter_RegisterSlot(This, slot) \
    INotifyCenter_RegisterEventMap(This, slot)

#define INotifyCenter_UnregisterSlot(This, slot) \
    INotifyCenter_UnregisterEventMap(This, slot)

#define INotifyCenter_AddEventHandler(This, slot) \
    INotifyCenter_RegisterEventMap(This, slot)

#define INotifyCenter_RemoveEventHandler(This, slot) \
    INotifyCenter_UnregisterEventMap(This, slot)

/* UI thread execution shortcuts */
#define INotifyCenter_PostToUI(This, runnable) \
    INotifyCenter_RunOnUI_Async(This, runnable)

#define INotifyCenter_SendToUI(This, runnable) \
    INotifyCenter_RunOnUI_Sync(This, runnable)

#define INotifyCenter_InvokeOnUI(This, runnable) \
    INotifyCenter_RunOnUI_Sync(This, runnable)

#define INotifyCenter_PostCallbackToUI(This, callback, wp, lp) \
    INotifyCenter_RunOnUI2_Async(This, callback, wp, lp)

#define INotifyCenter_SendCallbackToUI(This, callback, wp, lp) \
    INotifyCenter_RunOnUI2_Sync(This, callback, wp, lp)

#define INotifyCenter_InvokeCallbackOnUI(This, callback, wp, lp) \
    INotifyCenter_RunOnUI2_Sync(This, callback, wp, lp)

/* Safe notification operations */
#define INotifyCenter_SafeFireSync(This, event) \
    do { \
        if ((This) && (event)) { \
            INotifyCenter_FireEventSync(This, event); \
        } \
    } while(0)

#define INotifyCenter_SafeFireAsync(This, event) \
    do { \
        if ((This) && (event)) { \
            INotifyCenter_FireEventAsync(This, event); \
        } \
    } while(0)

#define INotifyCenter_SafeRegisterSlot(This, slot) \
    ((This) && (slot) ? INotifyCenter_RegisterEventMap(This, slot) : FALSE)

#define INotifyCenter_SafeUnregisterSlot(This, slot) \
    ((This) && (slot) ? INotifyCenter_UnregisterEventMap(This, slot) : FALSE)

#define INotifyCenter_SafeRunOnUI(This, runnable, sync) \
    do { \
        if ((This) && (runnable)) { \
            INotifyCenter_RunOnUI(This, runnable, sync); \
        } \
    } while(0)

#define INotifyCenter_SafeRunOnUI2(This, callback, wp, lp, sync) \
    do { \
        if ((This) && (callback)) { \
            INotifyCenter_RunOnUI2(This, callback, wp, lp, sync); \
        } \
    } while(0)

/*
 * Notification center helper functions
 */
static inline void INotifyCenter_PostSimpleCallback_C(INotifyCenter* pThis, FunRunOnUI callback, WPARAM param)
{
    if (pThis && callback) {
        INotifyCenter_RunOnUI2_Async(pThis, callback, param, 0);
    }
}

static inline void INotifyCenter_SendSimpleCallback_C(INotifyCenter* pThis, FunRunOnUI callback, WPARAM param)
{
    if (pThis && callback) {
        INotifyCenter_RunOnUI2_Sync(pThis, callback, param, 0);
    }
}

static inline void INotifyCenter_PostNotification_C(INotifyCenter* pThis, FunRunOnUI callback, void* data)
{
    if (pThis && callback) {
        INotifyCenter_RunOnUI2_Async(pThis, callback, (WPARAM)data, 0);
    }
}

static inline void INotifyCenter_SendNotification_C(INotifyCenter* pThis, FunRunOnUI callback, void* data)
{
    if (pThis && callback) {
        INotifyCenter_RunOnUI2_Sync(pThis, callback, (WPARAM)data, 0);
    }
}

/*
 * Event slot management helpers
 */
typedef struct NotifySlotManager {
    INotifyCenter* notifyCenter;
    const IEvtSlot** slots;
    int slotCount;
    int maxSlots;
} NotifySlotManager;

static inline void NotifySlotManager_Init(NotifySlotManager* mgr, INotifyCenter* notifyCenter, int maxSlots)
{
    if (mgr) {
        mgr->notifyCenter = notifyCenter;
        mgr->slots = maxSlots > 0 ? (const IEvtSlot**)malloc(sizeof(IEvtSlot*) * maxSlots) : NULL;
        mgr->slotCount = 0;
        mgr->maxSlots = maxSlots;
    }
}

static inline void NotifySlotManager_Cleanup(NotifySlotManager* mgr)
{
    if (mgr) {
        // Unregister all slots
        for (int i = 0; i < mgr->slotCount; i++) {
            if (mgr->notifyCenter && mgr->slots[i]) {
                INotifyCenter_UnregisterEventMap(mgr->notifyCenter, mgr->slots[i]);
            }
        }
        
        if (mgr->slots) {
            free((void*)mgr->slots);
        }
        memset(mgr, 0, sizeof(NotifySlotManager));
    }
}

static inline BOOL NotifySlotManager_AddSlot(NotifySlotManager* mgr, const IEvtSlot* slot)
{
    if (!mgr || !slot || mgr->slotCount >= mgr->maxSlots) return FALSE;
    
    if (mgr->notifyCenter && INotifyCenter_RegisterEventMap(mgr->notifyCenter, slot)) {
        mgr->slots[mgr->slotCount++] = slot;
        return TRUE;
    }
    return FALSE;
}

static inline BOOL NotifySlotManager_RemoveSlot(NotifySlotManager* mgr, const IEvtSlot* slot)
{
    if (!mgr || !slot) return FALSE;
    
    for (int i = 0; i < mgr->slotCount; i++) {
        if (mgr->slots[i] == slot) {
            if (mgr->notifyCenter) {
                INotifyCenter_UnregisterEventMap(mgr->notifyCenter, slot);
            }
            
            // Shift remaining slots
            for (int j = i; j < mgr->slotCount - 1; j++) {
                mgr->slots[j] = mgr->slots[j + 1];
            }
            mgr->slotCount--;
            return TRUE;
        }
    }
    return FALSE;
}

/*
 * UI thread callback helpers
 */
typedef struct UICallbackData {
    void* userData;
    void (*callback)(void* userData);
    BOOL completed;
    HANDLE completionEvent;
} UICallbackData;

static void CALLBACK UICallback_Wrapper(WPARAM wp, LPARAM lp)
{
    UICallbackData* data = (UICallbackData*)wp;
    if (data && data->callback) {
        data->callback(data->userData);
        data->completed = TRUE;
        if (data->completionEvent) {
            SetEvent(data->completionEvent);
        }
    }
}

static inline void INotifyCenter_RunCallbackOnUI_C(INotifyCenter* pThis, void (*callback)(void*), void* userData, BOOL bSync)
{
    if (!pThis || !callback) return;
    
    UICallbackData data;
    data.userData = userData;
    data.callback = callback;
    data.completed = FALSE;
    data.completionEvent = bSync ? CreateEvent(NULL, TRUE, FALSE, NULL) : NULL;
    
    INotifyCenter_RunOnUI2(pThis, UICallback_Wrapper, (WPARAM)&data, 0, bSync);
    
    if (bSync && data.completionEvent) {
        WaitForSingleObject(data.completionEvent, INFINITE);
        CloseHandle(data.completionEvent);
    }
}

#define INotifyCenter_PostCallbackOnUI(This, callback, userData) \
    INotifyCenter_RunCallbackOnUI_C(This, callback, userData, FALSE)

#define INotifyCenter_SendCallbackOnUI(This, callback, userData) \
    INotifyCenter_RunCallbackOnUI_C(This, callback, userData, TRUE)

/*
 * Event creation and firing helpers
 */
static inline void INotifyCenter_FireCustomEvent_C(INotifyCenter* pThis, UINT eventId, void* eventData, BOOL bAsync)
{
    if (!pThis) return;
    
    // Note: This would require creating a custom event args object
    // The actual implementation would depend on the event system design
    // For now, this is a placeholder showing the pattern
}

/*
 * Thread safety helpers
 */
static inline BOOL INotifyCenter_IsUIThread_C(void)
{
    // This would need to be implemented based on the UI framework's thread detection
    // For now, return TRUE as a placeholder
    return TRUE;
}

static inline void INotifyCenter_EnsureUIThread_C(INotifyCenter* pThis, IRunnable* runnable)
{
    if (!pThis || !runnable) return;
    
    if (INotifyCenter_IsUIThread_C()) {
        // Already on UI thread, run directly
        IRunnable_Run(runnable);
    } else {
        // Post to UI thread
        INotifyCenter_PostToUI(pThis, runnable);
    }
}

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void INotifyCenter_DebugInfo_C(INotifyCenter* pThis)
{
    if (!pThis) {
        printf("NotifyCenter: NULL\n");
        return;
    }
    
    printf("NotifyCenter: %p\n", pThis);
    // Additional debug information would require more interface methods
}
#else
#define INotifyCenter_DebugInfo_C(This) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SNOTIFYCENTER_CAPI_H__ */
