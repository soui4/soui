#ifndef __SREALWNDHANDLER_CAPI_H__
#define __SREALWNDHANDLER_CAPI_H__

#include "../SRealWndHandler-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Real window handler constants
 */
#define REALWND_SYSTEM_HANDLE   FALSE
#define REALWND_USER_HANDLE     TRUE

/*
 * C API Helper Macros for SRealWndHandler Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IRealWndHandler C API Macros */
#define IRealWndHandler_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IRealWndHandler_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IRealWndHandler_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IRealWndHandler_OnRealWndCreate(This, pRealWnd) \
    ((This)->lpVtbl->OnRealWndCreate(This, pRealWnd))

#define IRealWndHandler_OnRealWndDestroy(This, pRealWnd) \
    ((This)->lpVtbl->OnRealWndDestroy(This, pRealWnd))

#define IRealWndHandler_OnRealWndInit(This, pRealWnd) \
    ((This)->lpVtbl->OnRealWndInit(This, pRealWnd))

#define IRealWndHandler_OnRealWndPosition(This, pRealWnd, rcWnd) \
    ((This)->lpVtbl->OnRealWndPosition(This, pRealWnd, rcWnd))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IRealWndHandler Helper Functions */
static inline long IRealWndHandler_AddRef_C(IRealWndHandler* pThis)
{
    return IRealWndHandler_AddRef(pThis);
}

static inline long IRealWndHandler_Release_C(IRealWndHandler* pThis)
{
    return IRealWndHandler_Release(pThis);
}

static inline HWND IRealWndHandler_OnRealWndCreate_C(IRealWndHandler* pThis, IWindow* pRealWnd)
{
    return IRealWndHandler_OnRealWndCreate(pThis, pRealWnd);
}

static inline void IRealWndHandler_OnRealWndDestroy_C(IRealWndHandler* pThis, IWindow* pRealWnd)
{
    IRealWndHandler_OnRealWndDestroy(pThis, pRealWnd);
}

static inline BOOL IRealWndHandler_OnRealWndInit_C(IRealWndHandler* pThis, IWindow* pRealWnd)
{
    return IRealWndHandler_OnRealWndInit(pThis, pRealWnd);
}

static inline BOOL IRealWndHandler_OnRealWndPosition_C(IRealWndHandler* pThis, IWindow* pRealWnd, const RECT* rcWnd)
{
    return IRealWndHandler_OnRealWndPosition(pThis, pRealWnd, rcWnd);
}

/*
 * Convenience macros for common real window handler operations
 */

/* Window lifecycle shortcuts */
#define IRealWndHandler_CreateWindow(This, window) \
    IRealWndHandler_OnRealWndCreate(This, window)

#define IRealWndHandler_DestroyWindow(This, window) \
    IRealWndHandler_OnRealWndDestroy(This, window)

#define IRealWndHandler_InitWindow(This, window) \
    IRealWndHandler_OnRealWndInit(This, window)

#define IRealWndHandler_PositionWindow(This, window, rect) \
    IRealWndHandler_OnRealWndPosition(This, window, rect)

/* Window management shortcuts */
#define IRealWndHandler_HandleCreate(This, window) \
    IRealWndHandler_OnRealWndCreate(This, window)

#define IRealWndHandler_HandleDestroy(This, window) \
    IRealWndHandler_OnRealWndDestroy(This, window)

#define IRealWndHandler_HandleInit(This, window) \
    IRealWndHandler_OnRealWndInit(This, window)

#define IRealWndHandler_HandlePosition(This, window, rect) \
    IRealWndHandler_OnRealWndPosition(This, window, rect)

/* Return value interpretation shortcuts */
#define IRealWndHandler_SystemShouldHandleInit(This, window) \
    (!IRealWndHandler_OnRealWndInit(This, window))

#define IRealWndHandler_UserShouldHandleInit(This, window) \
    IRealWndHandler_OnRealWndInit(This, window)

#define IRealWndHandler_SystemShouldHandlePosition(This, window, rect) \
    (!IRealWndHandler_OnRealWndPosition(This, window, rect))

#define IRealWndHandler_UserShouldHandlePosition(This, window, rect) \
    IRealWndHandler_OnRealWndPosition(This, window, rect)

/* Safe real window handler operations */
#define IRealWndHandler_SafeCreateWindow(This, window, phWnd) \
    do { \
        if ((This) && (window) && (phWnd)) { \
            *(phWnd) = IRealWndHandler_OnRealWndCreate(This, window); \
        } else if (phWnd) { \
            *(phWnd) = NULL; \
        } \
    } while(0)

#define IRealWndHandler_SafeDestroyWindow(This, window) \
    do { \
        if ((This) && (window)) { \
            IRealWndHandler_OnRealWndDestroy(This, window); \
        } \
    } while(0)

#define IRealWndHandler_SafeInitWindow(This, window) \
    ((This) && (window) ? IRealWndHandler_OnRealWndInit(This, window) : REALWND_SYSTEM_HANDLE)

#define IRealWndHandler_SafePositionWindow(This, window, rect) \
    ((This) && (window) && (rect) ? IRealWndHandler_OnRealWndPosition(This, window, rect) : REALWND_SYSTEM_HANDLE)

/*
 * Real window handler helper functions
 */
static inline BOOL IRealWndHandler_IsValid_C(IRealWndHandler* pThis)
{
    return (pThis != NULL);
}

static inline HWND IRealWndHandler_CreateWindowSafe_C(IRealWndHandler* pThis, IWindow* pRealWnd)
{
    if (!pThis || !pRealWnd) return NULL;
    
    return IRealWndHandler_OnRealWndCreate(pThis, pRealWnd);
}

static inline BOOL IRealWndHandler_InitWindowWithDefault_C(IRealWndHandler* pThis, IWindow* pRealWnd, BOOL defaultValue)
{
    if (!pThis || !pRealWnd) return defaultValue;
    
    return IRealWndHandler_OnRealWndInit(pThis, pRealWnd);
}

static inline BOOL IRealWndHandler_PositionWindowWithDefault_C(IRealWndHandler* pThis, IWindow* pRealWnd, const RECT* rcWnd, BOOL defaultValue)
{
    if (!pThis || !pRealWnd || !rcWnd) return defaultValue;
    
    return IRealWndHandler_OnRealWndPosition(pThis, pRealWnd, rcWnd);
}

static inline BOOL IRealWndHandler_MoveWindow_C(IRealWndHandler* pThis, IWindow* pRealWnd, int x, int y, int width, int height)
{
    if (!pThis || !pRealWnd) return REALWND_SYSTEM_HANDLE;
    
    RECT rect = {x, y, x + width, y + height};
    return IRealWndHandler_OnRealWndPosition(pThis, pRealWnd, &rect);
}

static inline BOOL IRealWndHandler_ResizeWindow_C(IRealWndHandler* pThis, IWindow* pRealWnd, int width, int height)
{
    if (!pThis || !pRealWnd) return REALWND_SYSTEM_HANDLE;
    
    // For resize, we assume current position (0,0) - the handler should get actual position
    RECT rect = {0, 0, width, height};
    return IRealWndHandler_OnRealWndPosition(pThis, pRealWnd, &rect);
}

/*
 * Real window handler state management
 */
typedef struct RealWndHandlerState {
    IRealWndHandler* handler;
    IWindow* currentWindow;
    HWND currentHwnd;
    RECT currentRect;
    BOOL isValid;
    BOOL hasCurrentWindow;
} RealWndHandlerState;

static inline void RealWndHandlerState_Init(RealWndHandlerState* state, IRealWndHandler* handler)
{
    if (state) {
        state->handler = handler;
        state->currentWindow = NULL;
        state->currentHwnd = NULL;
        memset(&state->currentRect, 0, sizeof(RECT));
        state->isValid = (handler != NULL);
        state->hasCurrentWindow = FALSE;
    }
}

static inline HWND RealWndHandlerState_CreateWindow(RealWndHandlerState* state, IWindow* window)
{
    if (!state || !state->handler || !window) return NULL;
    
    HWND hwnd = IRealWndHandler_OnRealWndCreate(state->handler, window);
    if (hwnd) {
        state->currentWindow = window;
        state->currentHwnd = hwnd;
        state->hasCurrentWindow = TRUE;
    }
    return hwnd;
}

static inline void RealWndHandlerState_DestroyWindow(RealWndHandlerState* state, IWindow* window)
{
    if (!state || !state->handler || !window) return;
    
    IRealWndHandler_OnRealWndDestroy(state->handler, window);
    
    if (state->currentWindow == window) {
        state->currentWindow = NULL;
        state->currentHwnd = NULL;
        state->hasCurrentWindow = FALSE;
        memset(&state->currentRect, 0, sizeof(RECT));
    }
}

static inline BOOL RealWndHandlerState_InitWindow(RealWndHandlerState* state, IWindow* window)
{
    if (!state || !state->handler || !window) return REALWND_SYSTEM_HANDLE;
    
    return IRealWndHandler_OnRealWndInit(state->handler, window);
}

static inline BOOL RealWndHandlerState_PositionWindow(RealWndHandlerState* state, IWindow* window, const RECT* rect)
{
    if (!state || !state->handler || !window || !rect) return REALWND_SYSTEM_HANDLE;
    
    BOOL result = IRealWndHandler_OnRealWndPosition(state->handler, window, rect);
    
    if (state->currentWindow == window) {
        state->currentRect = *rect;
    }
    
    return result;
}

/*
 * Window lifecycle management
 */
typedef struct WindowLifecycle {
    IRealWndHandler* handler;
    IWindow* window;
    HWND hwnd;
    RECT rect;
    BOOL isCreated;
    BOOL isInitialized;
    BOOL isPositioned;
} WindowLifecycle;

static inline void WindowLifecycle_Init(WindowLifecycle* lifecycle, IRealWndHandler* handler, IWindow* window)
{
    if (lifecycle) {
        lifecycle->handler = handler;
        lifecycle->window = window;
        lifecycle->hwnd = NULL;
        memset(&lifecycle->rect, 0, sizeof(RECT));
        lifecycle->isCreated = FALSE;
        lifecycle->isInitialized = FALSE;
        lifecycle->isPositioned = FALSE;
    }
}

static inline BOOL WindowLifecycle_Create(WindowLifecycle* lifecycle)
{
    if (!lifecycle || !lifecycle->handler || !lifecycle->window || lifecycle->isCreated) return FALSE;
    
    lifecycle->hwnd = IRealWndHandler_OnRealWndCreate(lifecycle->handler, lifecycle->window);
    lifecycle->isCreated = (lifecycle->hwnd != NULL);
    return lifecycle->isCreated;
}

static inline BOOL WindowLifecycle_Initialize(WindowLifecycle* lifecycle)
{
    if (!lifecycle || !lifecycle->handler || !lifecycle->window || !lifecycle->isCreated) return FALSE;
    
    BOOL userHandles = IRealWndHandler_OnRealWndInit(lifecycle->handler, lifecycle->window);
    lifecycle->isInitialized = TRUE;
    return userHandles;
}

static inline BOOL WindowLifecycle_Position(WindowLifecycle* lifecycle, const RECT* rect)
{
    if (!lifecycle || !lifecycle->handler || !lifecycle->window || !lifecycle->isCreated || !rect) return FALSE;
    
    BOOL userHandles = IRealWndHandler_OnRealWndPosition(lifecycle->handler, lifecycle->window, rect);
    lifecycle->rect = *rect;
    lifecycle->isPositioned = TRUE;
    return userHandles;
}

static inline void WindowLifecycle_Destroy(WindowLifecycle* lifecycle)
{
    if (!lifecycle || !lifecycle->handler || !lifecycle->window || !lifecycle->isCreated) return;
    
    IRealWndHandler_OnRealWndDestroy(lifecycle->handler, lifecycle->window);
    lifecycle->hwnd = NULL;
    lifecycle->isCreated = FALSE;
    lifecycle->isInitialized = FALSE;
    lifecycle->isPositioned = FALSE;
    memset(&lifecycle->rect, 0, sizeof(RECT));
}

/*
 * Reference counting helpers
 */
#define IRealWndHandler_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IRealWndHandler_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void IRealWndHandler_DebugInfo_C(IRealWndHandler* pThis)
{
    if (!pThis) {
        printf("RealWndHandler: NULL\n");
        return;
    }
    
    printf("RealWndHandler: %p\n", pThis);
}

static inline void WindowLifecycle_DebugInfo(const WindowLifecycle* lifecycle)
{
    if (!lifecycle) {
        printf("WindowLifecycle: NULL\n");
        return;
    }
    
    printf("WindowLifecycle: %p\n", lifecycle);
    printf("  Handler: %p\n", lifecycle->handler);
    printf("  Window: %p\n", lifecycle->window);
    printf("  HWND: %p\n", lifecycle->hwnd);
    printf("  Created: %s\n", lifecycle->isCreated ? "Yes" : "No");
    printf("  Initialized: %s\n", lifecycle->isInitialized ? "Yes" : "No");
    printf("  Positioned: %s\n", lifecycle->isPositioned ? "Yes" : "No");
    printf("  Rect: (%d,%d,%d,%d)\n", lifecycle->rect.left, lifecycle->rect.top, lifecycle->rect.right, lifecycle->rect.bottom);
}
#else
#define IRealWndHandler_DebugInfo_C(This) ((void)0)
#define WindowLifecycle_DebugInfo(lifecycle) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SREALWNDHANDLER_CAPI_H__ */
