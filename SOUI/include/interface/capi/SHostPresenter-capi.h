#ifndef __SHOSTPRESENTER_CAPI_H__
#define __SHOSTPRESENTER_CAPI_H__

#include "../SHostPresenter-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SHostPresenter Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IHostPresenter C API Macros */
#define IHostPresenter_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IHostPresenter_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IHostPresenter_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IHostPresenter_OnHostCreate(This) \
    ((This)->lpVtbl->OnHostCreate(This))

#define IHostPresenter_OnHostDestroy(This) \
    ((This)->lpVtbl->OnHostDestroy(This))

#define IHostPresenter_OnHostResize(This, szHost) \
    ((This)->lpVtbl->OnHostResize(This, szHost))

#define IHostPresenter_OnHostPresent(This, hdc, pMemRT, rcDirty, byAlpha) \
    ((This)->lpVtbl->OnHostPresent(This, hdc, pMemRT, rcDirty, byAlpha))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IHostPresenter Helper Functions */
static inline long IHostPresenter_AddRef_C(IHostPresenter* pThis)
{
    return IHostPresenter_AddRef(pThis);
}

static inline long IHostPresenter_Release_C(IHostPresenter* pThis)
{
    return IHostPresenter_Release(pThis);
}

static inline void IHostPresenter_OnFinalRelease_C(IHostPresenter* pThis)
{
    IHostPresenter_OnFinalRelease(pThis);
}

static inline void IHostPresenter_OnHostCreate_C(IHostPresenter* pThis)
{
    IHostPresenter_OnHostCreate(pThis);
}

static inline void IHostPresenter_OnHostDestroy_C(IHostPresenter* pThis)
{
    IHostPresenter_OnHostDestroy(pThis);
}

static inline void IHostPresenter_OnHostResize_C(IHostPresenter* pThis, SIZE szHost)
{
    IHostPresenter_OnHostResize(pThis, szHost);
}

static inline void IHostPresenter_OnHostPresent_C(IHostPresenter* pThis, HDC hdc, IRenderTarget* pMemRT, LPCRECT rcDirty, BYTE byAlpha)
{
    IHostPresenter_OnHostPresent(pThis, hdc, pMemRT, rcDirty, byAlpha);
}

/*
 * Convenience macros for common host presenter operations
 */

/* Host lifecycle shortcuts */
#define IHostPresenter_Create(This) \
    IHostPresenter_OnHostCreate(This)

#define IHostPresenter_Destroy(This) \
    IHostPresenter_OnHostDestroy(This)

/* Host resize shortcuts */
#define IHostPresenter_ResizeToSize(This, width, height) \
    do { \
        SIZE sz = {width, height}; \
        IHostPresenter_OnHostResize(This, sz); \
    } while(0)

#define IHostPresenter_ResizeToRect(This, pRect) \
    do { \
        if (pRect) { \
            SIZE sz = {(pRect)->right - (pRect)->left, (pRect)->bottom - (pRect)->top}; \
            IHostPresenter_OnHostResize(This, sz); \
        } \
    } while(0)

/* Host presentation shortcuts */
#define IHostPresenter_PresentFull(This, hdc, pMemRT, byAlpha) \
    IHostPresenter_OnHostPresent(This, hdc, pMemRT, NULL, byAlpha)

#define IHostPresenter_PresentOpaque(This, hdc, pMemRT, rcDirty) \
    IHostPresenter_OnHostPresent(This, hdc, pMemRT, rcDirty, 255)

#define IHostPresenter_PresentFullOpaque(This, hdc, pMemRT) \
    IHostPresenter_OnHostPresent(This, hdc, pMemRT, NULL, 255)

/* Safe operations with validation */
#define IHostPresenter_SafeCreate(This) \
    do { \
        if (IObjRef_IsValidInterface(This)) { \
            IHostPresenter_OnHostCreate(This); \
        } \
    } while(0)

#define IHostPresenter_SafeDestroy(This) \
    do { \
        if (IObjRef_IsValidInterface(This)) { \
            IHostPresenter_OnHostDestroy(This); \
        } \
    } while(0)

#define IHostPresenter_SafeResize(This, szHost) \
    do { \
        if (IObjRef_IsValidInterface(This)) { \
            IHostPresenter_OnHostResize(This, szHost); \
        } \
    } while(0)

#define IHostPresenter_SafePresent(This, hdc, pMemRT, rcDirty, byAlpha) \
    do { \
        if (IObjRef_IsValidInterface(This) && (hdc) && (pMemRT)) { \
            IHostPresenter_OnHostPresent(This, hdc, pMemRT, rcDirty, byAlpha); \
        } \
    } while(0)

/*
 * Host presenter lifecycle management helpers
 */
static inline void IHostPresenter_FullLifecycle_C(IHostPresenter* pThis, SIZE szHost)
{
    if (IObjRef_IsValidInterface(pThis)) {
        IHostPresenter_OnHostCreate(pThis);
        IHostPresenter_OnHostResize(pThis, szHost);
    }
}

static inline void IHostPresenter_CleanupLifecycle_C(IHostPresenter* pThis)
{
    if (IObjRef_IsValidInterface(pThis)) {
        IHostPresenter_OnHostDestroy(pThis);
    }
}

/*
 * Presentation helper functions
 */
static inline void IHostPresenter_PresentWithRect_C(IHostPresenter* pThis, HDC hdc, IRenderTarget* pMemRT, 
                                                   int left, int top, int right, int bottom, BYTE byAlpha)
{
    if (IObjRef_IsValidInterface(pThis) && hdc && pMemRT) {
        RECT rcDirty = {left, top, right, bottom};
        IHostPresenter_OnHostPresent(pThis, hdc, pMemRT, &rcDirty, byAlpha);
    }
}

static inline void IHostPresenter_PresentWithSize_C(IHostPresenter* pThis, HDC hdc, IRenderTarget* pMemRT, 
                                                   int width, int height, BYTE byAlpha)
{
    if (IObjRef_IsValidInterface(pThis) && hdc && pMemRT) {
        RECT rcDirty = {0, 0, width, height};
        IHostPresenter_OnHostPresent(pThis, hdc, pMemRT, &rcDirty, byAlpha);
    }
}

/*
 * Reference counting helpers
 */
#define IHostPresenter_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IHostPresenter_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Host presenter state management
 */
typedef struct HostPresenterState {
    IHostPresenter* presenter;
    SIZE currentSize;
    BOOL isCreated;
    BOOL isDestroyed;
} HostPresenterState;

static inline void HostPresenterState_Init(HostPresenterState* state, IHostPresenter* presenter)
{
    if (state) {
        state->presenter = presenter;
        state->currentSize.cx = 0;
        state->currentSize.cy = 0;
        state->isCreated = FALSE;
        state->isDestroyed = FALSE;
        if (presenter) {
            IHostPresenter_SafeAddRef(presenter);
        }
    }
}

static inline void HostPresenterState_Cleanup(HostPresenterState* state)
{
    if (state) {
        if (state->presenter) {
            if (state->isCreated && !state->isDestroyed) {
                IHostPresenter_SafeDestroy(state->presenter);
            }
            IHostPresenter_SafeRelease(&state->presenter);
        }
        memset(state, 0, sizeof(HostPresenterState));
    }
}

static inline BOOL HostPresenterState_Create(HostPresenterState* state, SIZE szHost)
{
    if (state && state->presenter && !state->isCreated) {
        IHostPresenter_SafeCreate(state->presenter);
        IHostPresenter_SafeResize(state->presenter, szHost);
        state->currentSize = szHost;
        state->isCreated = TRUE;
        return TRUE;
    }
    return FALSE;
}

static inline BOOL HostPresenterState_Destroy(HostPresenterState* state)
{
    if (state && state->presenter && state->isCreated && !state->isDestroyed) {
        IHostPresenter_SafeDestroy(state->presenter);
        state->isDestroyed = TRUE;
        return TRUE;
    }
    return FALSE;
}

static inline BOOL HostPresenterState_Resize(HostPresenterState* state, SIZE szHost)
{
    if (state && state->presenter && state->isCreated && !state->isDestroyed) {
        IHostPresenter_SafeResize(state->presenter, szHost);
        state->currentSize = szHost;
        return TRUE;
    }
    return FALSE;
}

static inline BOOL HostPresenterState_Present(HostPresenterState* state, HDC hdc, IRenderTarget* pMemRT, 
                                             LPCRECT rcDirty, BYTE byAlpha)
{
    if (state && state->presenter && state->isCreated && !state->isDestroyed && hdc && pMemRT) {
        IHostPresenter_SafePresent(state->presenter, hdc, pMemRT, rcDirty, byAlpha);
        return TRUE;
    }
    return FALSE;
}

#ifdef __cplusplus
}
#endif

#endif /* __SHOSTPRESENTER_CAPI_H__ */
