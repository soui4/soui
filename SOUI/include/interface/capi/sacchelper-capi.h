#ifndef __SACCHELPER_CAPI_H__
#define __SACCHELPER_CAPI_H__

#include "../sacchelper-i.h"

#ifdef _WIN32
#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for sacchelper Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IAccHelper C API Macros */
#define IAccHelper_QueryInterface(This, riid, ppvObject) \
    ((This)->lpVtbl->QueryInterface(This, riid, ppvObject))

#define IAccHelper_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IAccHelper_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IAccHelper_SetOwner(This, pOwner) \
    ((This)->lpVtbl->SetOwner(This, pOwner))

#define IAccHelper_GetOwner(This) \
    ((This)->lpVtbl->GetOwner(This))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IAccHelper Helper Functions */
static inline HRESULT IAccHelper_QueryInterface_C(IAccHelper* pThis, REFIID riid, void** ppvObject)
{
    return IAccHelper_QueryInterface(pThis, riid, ppvObject);
}

static inline ULONG IAccHelper_AddRef_C(IAccHelper* pThis)
{
    return IAccHelper_AddRef(pThis);
}

static inline ULONG IAccHelper_Release_C(IAccHelper* pThis)
{
    return IAccHelper_Release(pThis);
}

static inline void IAccHelper_SetOwner_C(IAccHelper* pThis, IWindow* pOwner)
{
    IAccHelper_SetOwner(pThis, pOwner);
}

static inline IWindow* IAccHelper_GetOwner_C(const IAccHelper* pThis)
{
    return IAccHelper_GetOwner(pThis);
}

/*
 * Convenience macros for common accessibility helper operations
 */

/* Owner management shortcuts */
#define IAccHelper_SetOwnerWindow(This, owner) \
    IAccHelper_SetOwner(This, owner)

#define IAccHelper_GetOwnerWindow(This) \
    IAccHelper_GetOwner(This)

#define IAccHelper_SetWindow(This, window) \
    IAccHelper_SetOwner(This, window)

#define IAccHelper_GetWindow(This) \
    IAccHelper_GetOwner(This)

/* Safe accessibility helper operations */
#define IAccHelper_SafeSetOwner(This, owner) \
    do { \
        if ((This) && (owner)) { \
            IAccHelper_SetOwner(This, owner); \
        } \
    } while(0)

#define IAccHelper_SafeGetOwner(This, ppOwner) \
    do { \
        if ((This) && (ppOwner)) { \
            *(ppOwner) = IAccHelper_GetOwner(This); \
        } else if (ppOwner) { \
            *(ppOwner) = NULL; \
        } \
    } while(0)

#define IAccHelper_SafeQueryInterface(This, riid, ppvObject) \
    ((This) && (riid) && (ppvObject) ? IAccHelper_QueryInterface(This, riid, ppvObject) : E_INVALIDARG)

/*
 * Accessibility helper helper functions
 */
static inline BOOL IAccHelper_IsValid_C(IAccHelper* pThis)
{
    return (pThis != NULL);
}

static inline BOOL IAccHelper_HasOwner_C(IAccHelper* pThis)
{
    if (!pThis) return FALSE;
    
    return (IAccHelper_GetOwner(pThis) != NULL);
}

static inline BOOL IAccHelper_ChangeOwner_C(IAccHelper* pThis, IWindow* newOwner)
{
    if (!pThis) return FALSE;
    
    IAccHelper_SetOwner(pThis, newOwner);
    return (IAccHelper_GetOwner(pThis) == newOwner);
}

/*
 * Accessibility helper state management
 */
typedef struct AccHelperState {
    IAccHelper* accHelper;
    IWindow* ownerWindow;
    BOOL isValid;
    BOOL hasOwner;
} AccHelperState;

static inline void AccHelperState_Init(AccHelperState* state, IAccHelper* accHelper)
{
    if (state) {
        state->accHelper = accHelper;
        if (accHelper) {
            state->ownerWindow = IAccHelper_GetOwner(accHelper);
            state->isValid = TRUE;
            state->hasOwner = (state->ownerWindow != NULL);
        } else {
            memset(state, 0, sizeof(AccHelperState));
        }
    }
}

static inline void AccHelperState_Update(AccHelperState* state)
{
    if (state && state->accHelper) {
        AccHelperState_Init(state, state->accHelper);
    }
}

static inline void AccHelperState_SetOwner(AccHelperState* state, IWindow* owner)
{
    if (state && state->accHelper) {
        IAccHelper_SetOwner(state->accHelper, owner);
        state->ownerWindow = owner;
        state->hasOwner = (owner != NULL);
    }
}

/*
 * COM interface helpers
 */
static inline HRESULT IAccHelper_QueryForInterface_C(IAccHelper* pThis, REFIID riid, void** ppInterface)
{
    if (!pThis || !ppInterface) return E_INVALIDARG;
    
    *ppInterface = NULL;
    return IAccHelper_QueryInterface(pThis, riid, ppInterface);
}

/*
 * Reference counting helpers
 */
#define IAccHelper_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IAccHelper_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Accessibility helper collection management
 */
typedef struct AccHelperCollection {
    AccHelperState* helpers;
    int count;
    int capacity;
} AccHelperCollection;

static inline void AccHelperCollection_Init(AccHelperCollection* collection, int initialCapacity)
{
    if (collection) {
        collection->helpers = initialCapacity > 0 ? 
            (AccHelperState*)malloc(sizeof(AccHelperState) * initialCapacity) : NULL;
        collection->count = 0;
        collection->capacity = initialCapacity;
    }
}

static inline void AccHelperCollection_Cleanup(AccHelperCollection* collection)
{
    if (collection) {
        if (collection->helpers) {
            free(collection->helpers);
        }
        memset(collection, 0, sizeof(AccHelperCollection));
    }
}

static inline BOOL AccHelperCollection_Add(AccHelperCollection* collection, IAccHelper* accHelper)
{
    if (!collection || !accHelper || collection->count >= collection->capacity) return FALSE;
    
    AccHelperState_Init(&collection->helpers[collection->count], accHelper);
    collection->count++;
    return TRUE;
}

static inline IAccHelper* AccHelperCollection_FindByOwner(AccHelperCollection* collection, IWindow* owner)
{
    if (!collection || !owner) return NULL;
    
    for (int i = 0; i < collection->count; i++) {
        if (collection->helpers[i].ownerWindow == owner) {
            return collection->helpers[i].accHelper;
        }
    }
    return NULL;
}

static inline void AccHelperCollection_UpdateAll(AccHelperCollection* collection)
{
    if (!collection) return;
    
    for (int i = 0; i < collection->count; i++) {
        AccHelperState_Update(&collection->helpers[i]);
    }
}

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void IAccHelper_DebugInfo_C(IAccHelper* pThis)
{
    if (!pThis) {
        printf("AccHelper: NULL\n");
        return;
    }
    
    printf("AccHelper: %p\n", pThis);
    printf("  Owner Window: %p\n", IAccHelper_GetOwner(pThis));
    printf("  Has Owner: %s\n", IAccHelper_HasOwner_C(pThis) ? "Yes" : "No");
}

static inline void AccHelperState_DebugInfo(const AccHelperState* state)
{
    if (!state) {
        printf("AccHelperState: NULL\n");
        return;
    }
    
    printf("AccHelperState: %p\n", state);
    printf("  AccHelper: %p\n", state->accHelper);
    printf("  Owner Window: %p\n", state->ownerWindow);
    printf("  Valid: %s\n", state->isValid ? "Yes" : "No");
    printf("  Has Owner: %s\n", state->hasOwner ? "Yes" : "No");
}
#else
#define IAccHelper_DebugInfo_C(This) ((void)0)
#define AccHelperState_DebugInfo(state) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif //_WIN32
#endif /* __SACCHELPER_CAPI_H__ */
