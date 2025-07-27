#ifndef __STIMELINEHANDLER_CAPI_H__
#define __STIMELINEHANDLER_CAPI_H__

#include "../STimelineHandler-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for STimelineHandler Interface
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

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* ITimelineHandler Helper Functions */
static inline void ITimelineHandler_OnNextFrame_C(ITimelineHandler* pThis)
{
    ITimelineHandler_OnNextFrame(pThis);
}

/* ITimelineHandlersMgr Helper Functions */
static inline BOOL ITimelineHandlersMgr_RegisterTimelineHandler_C(ITimelineHandlersMgr* pThis, ITimelineHandler* pHandler)
{
    return ITimelineHandlersMgr_RegisterTimelineHandler(pThis, pHandler);
}

static inline BOOL ITimelineHandlersMgr_UnregisterTimelineHandler_C(ITimelineHandlersMgr* pThis, ITimelineHandler* pHandler)
{
    return ITimelineHandlersMgr_UnregisterTimelineHandler(pThis, pHandler);
}

/*
 * Convenience macros for common timeline handler operations
 */

/* Timeline handler shortcuts */
#define ITimelineHandler_ProcessFrame(This) \
    ITimelineHandler_OnNextFrame(This)

#define ITimelineHandler_HandleFrame(This) \
    ITimelineHandler_OnNextFrame(This)

#define ITimelineHandler_NextFrame(This) \
    ITimelineHandler_OnNextFrame(This)

/* Timeline manager shortcuts */
#define ITimelineHandlersMgr_RegisterHandler(This, handler) \
    ITimelineHandlersMgr_RegisterTimelineHandler(This, handler)

#define ITimelineHandlersMgr_UnregisterHandler(This, handler) \
    ITimelineHandlersMgr_UnregisterTimelineHandler(This, handler)

#define ITimelineHandlersMgr_AddHandler(This, handler) \
    ITimelineHandlersMgr_RegisterTimelineHandler(This, handler)

#define ITimelineHandlersMgr_RemoveHandler(This, handler) \
    ITimelineHandlersMgr_UnregisterTimelineHandler(This, handler)

/* Safe timeline operations */
#define ITimelineHandler_SafeOnNextFrame(This) \
    do { \
        if (This) { \
            ITimelineHandler_OnNextFrame(This); \
        } \
    } while(0)

#define ITimelineHandlersMgr_SafeRegisterHandler(This, handler) \
    ((This) && (handler) ? ITimelineHandlersMgr_RegisterTimelineHandler(This, handler) : FALSE)

#define ITimelineHandlersMgr_SafeUnregisterHandler(This, handler) \
    ((This) && (handler) ? ITimelineHandlersMgr_UnregisterTimelineHandler(This, handler) : FALSE)

/*
 * Timeline handler helper functions
 */
static inline BOOL ITimelineHandler_IsValid_C(ITimelineHandler* pThis)
{
    return (pThis != NULL);
}

static inline BOOL ITimelineHandlersMgr_IsValid_C(ITimelineHandlersMgr* pThis)
{
    return (pThis != NULL);
}

/*
 * Timeline handler state management
 */
typedef struct TimelineHandlerState {
    ITimelineHandler* handler;
    ITimelineHandlersMgr* manager;
    BOOL isRegistered;
    BOOL isValid;
} TimelineHandlerState;

static inline void TimelineHandlerState_Init(TimelineHandlerState* state, ITimelineHandler* handler, ITimelineHandlersMgr* manager)
{
    if (state) {
        state->handler = handler;
        state->manager = manager;
        state->isRegistered = FALSE;
        state->isValid = (handler != NULL && manager != NULL);
    }
}

static inline BOOL TimelineHandlerState_Register(TimelineHandlerState* state)
{
    if (!state || !state->isValid || state->isRegistered) return FALSE;
    
    if (ITimelineHandlersMgr_RegisterTimelineHandler(state->manager, state->handler)) {
        state->isRegistered = TRUE;
        return TRUE;
    }
    return FALSE;
}

static inline BOOL TimelineHandlerState_Unregister(TimelineHandlerState* state)
{
    if (!state || !state->isValid || !state->isRegistered) return FALSE;
    
    if (ITimelineHandlersMgr_UnregisterTimelineHandler(state->manager, state->handler)) {
        state->isRegistered = FALSE;
        return TRUE;
    }
    return FALSE;
}

static inline void TimelineHandlerState_ProcessFrame(TimelineHandlerState* state)
{
    if (state && state->isValid && state->handler) {
        ITimelineHandler_OnNextFrame(state->handler);
    }
}

/*
 * Timeline handler collection management
 */
typedef struct TimelineHandlerCollection {
    TimelineHandlerState* handlers;
    int count;
    int capacity;
} TimelineHandlerCollection;

static inline void TimelineHandlerCollection_Init(TimelineHandlerCollection* collection, int initialCapacity)
{
    if (collection) {
        collection->handlers = initialCapacity > 0 ? 
            (TimelineHandlerState*)malloc(sizeof(TimelineHandlerState) * initialCapacity) : NULL;
        collection->count = 0;
        collection->capacity = initialCapacity;
    }
}

static inline void TimelineHandlerCollection_Cleanup(TimelineHandlerCollection* collection)
{
    if (collection) {
        // Unregister all handlers
        for (int i = 0; i < collection->count; i++) {
            TimelineHandlerState_Unregister(&collection->handlers[i]);
        }
        
        if (collection->handlers) {
            free(collection->handlers);
        }
        memset(collection, 0, sizeof(TimelineHandlerCollection));
    }
}

static inline BOOL TimelineHandlerCollection_Add(TimelineHandlerCollection* collection, ITimelineHandler* handler, ITimelineHandlersMgr* manager)
{
    if (!collection || !handler || !manager || collection->count >= collection->capacity) return FALSE;
    
    TimelineHandlerState_Init(&collection->handlers[collection->count], handler, manager);
    if (TimelineHandlerState_Register(&collection->handlers[collection->count])) {
        collection->count++;
        return TRUE;
    }
    return FALSE;
}

static inline void TimelineHandlerCollection_ProcessAllFrames(TimelineHandlerCollection* collection)
{
    if (!collection) return;
    
    for (int i = 0; i < collection->count; i++) {
        TimelineHandlerState_ProcessFrame(&collection->handlers[i]);
    }
}

static inline void TimelineHandlerCollection_UnregisterAll(TimelineHandlerCollection* collection)
{
    if (!collection) return;
    
    for (int i = 0; i < collection->count; i++) {
        TimelineHandlerState_Unregister(&collection->handlers[i]);
    }
}

static inline void TimelineHandlerCollection_RegisterAll(TimelineHandlerCollection* collection)
{
    if (!collection) return;
    
    for (int i = 0; i < collection->count; i++) {
        TimelineHandlerState_Register(&collection->handlers[i]);
    }
}

/*
 * Timeline frame processing helpers
 */
typedef struct FrameProcessor {
    ITimelineHandler** handlers;
    int handlerCount;
    int frameCount;
    BOOL isProcessing;
} FrameProcessor;

static inline void FrameProcessor_Init(FrameProcessor* processor, ITimelineHandler** handlers, int handlerCount)
{
    if (processor) {
        processor->handlers = handlers;
        processor->handlerCount = handlerCount;
        processor->frameCount = 0;
        processor->isProcessing = FALSE;
    }
}

static inline void FrameProcessor_ProcessFrame(FrameProcessor* processor)
{
    if (!processor || processor->isProcessing) return;
    
    processor->isProcessing = TRUE;
    
    for (int i = 0; i < processor->handlerCount; i++) {
        if (processor->handlers[i]) {
            ITimelineHandler_OnNextFrame(processor->handlers[i]);
        }
    }
    
    processor->frameCount++;
    processor->isProcessing = FALSE;
}

static inline int FrameProcessor_GetFrameCount(const FrameProcessor* processor)
{
    return processor ? processor->frameCount : 0;
}

static inline void FrameProcessor_Reset(FrameProcessor* processor)
{
    if (processor) {
        processor->frameCount = 0;
        processor->isProcessing = FALSE;
    }
}

/*
 * Timeline callback function type
 */
typedef void (*TimelineFrameCallback)(void* userData);

typedef struct TimelineCallbackHandler {
    TimelineFrameCallback callback;
    void* userData;
} TimelineCallbackHandler;

static inline void TimelineCallbackHandler_Init(TimelineCallbackHandler* handler, TimelineFrameCallback callback, void* userData)
{
    if (handler) {
        handler->callback = callback;
        handler->userData = userData;
    }
}

static inline void TimelineCallbackHandler_ProcessFrame(TimelineCallbackHandler* handler)
{
    if (handler && handler->callback) {
        handler->callback(handler->userData);
    }
}

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void ITimelineHandler_DebugInfo_C(ITimelineHandler* pThis)
{
    if (!pThis) {
        printf("TimelineHandler: NULL\n");
        return;
    }
    
    printf("TimelineHandler: %p\n", pThis);
}

static inline void ITimelineHandlersMgr_DebugInfo_C(ITimelineHandlersMgr* pThis)
{
    if (!pThis) {
        printf("TimelineHandlersMgr: NULL\n");
        return;
    }
    
    printf("TimelineHandlersMgr: %p\n", pThis);
}

static inline void TimelineHandlerState_DebugInfo(const TimelineHandlerState* state)
{
    if (!state) {
        printf("TimelineHandlerState: NULL\n");
        return;
    }
    
    printf("TimelineHandlerState: %p\n", state);
    printf("  Handler: %p\n", state->handler);
    printf("  Manager: %p\n", state->manager);
    printf("  Registered: %s\n", state->isRegistered ? "Yes" : "No");
    printf("  Valid: %s\n", state->isValid ? "Yes" : "No");
}
#else
#define ITimelineHandler_DebugInfo_C(This) ((void)0)
#define ITimelineHandlersMgr_DebugInfo_C(This) ((void)0)
#define TimelineHandlerState_DebugInfo(state) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STIMELINEHANDLER_CAPI_H__ */
