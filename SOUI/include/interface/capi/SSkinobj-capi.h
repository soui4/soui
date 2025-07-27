#ifndef __SSKINOBJ_CAPI_H__
#define __SSKINOBJ_CAPI_H__

#include "../SSkinobj-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SSkinobj Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ISkinObj C API Macros */
#define ISkinObj_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ISkinObj_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ISkinObj_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ISkinObj_DrawByState2(This, pRT, rcDraw, dwState, byAlpha) \
    ((This)->lpVtbl->DrawByState2(This, pRT, rcDraw, dwState, byAlpha))

#define ISkinObj_DrawByState(This, pRT, rcDraw, dwState) \
    ((This)->lpVtbl->DrawByState(This, pRT, rcDraw, dwState))

#define ISkinObj_DrawByIndex2(This, pRT, rcDraw, iState, byAlpha) \
    ((This)->lpVtbl->DrawByIndex2(This, pRT, rcDraw, iState, byAlpha))

#define ISkinObj_DrawByIndex(This, pRT, rcDraw, iState) \
    ((This)->lpVtbl->DrawByIndex(This, pRT, rcDraw, iState))

#define ISkinObj_GetSkinSize(This) \
    ((This)->lpVtbl->GetSkinSize(This))

#define ISkinObj_GetStates(This) \
    ((This)->lpVtbl->GetStates(This))

#define ISkinObj_GetAlpha(This) \
    ((This)->lpVtbl->GetAlpha(This))

#define ISkinObj_SetAlpha(This, byAlpha) \
    ((This)->lpVtbl->SetAlpha(This, byAlpha))

#define ISkinObj_OnColorize(This, cr) \
    ((This)->lpVtbl->OnColorize(This, cr))

#define ISkinObj_GetScale(This) \
    ((This)->lpVtbl->GetScale(This))

#define ISkinObj_SetScale(This, scale) \
    ((This)->lpVtbl->SetScale(This, scale))

#define ISkinObj_Scale(This, nScale) \
    ((This)->lpVtbl->Scale(This, nScale))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* ISkinObj Helper Functions */
static inline long ISkinObj_AddRef_C(ISkinObj* pThis)
{
    return ISkinObj_AddRef(pThis);
}

static inline long ISkinObj_Release_C(ISkinObj* pThis)
{
    return ISkinObj_Release(pThis);
}

static inline void ISkinObj_DrawByState2_C(const ISkinObj* pThis, IRenderTarget* pRT, LPCRECT rcDraw, DWORD dwState, BYTE byAlpha)
{
    ISkinObj_DrawByState2(pThis, pRT, rcDraw, dwState, byAlpha);
}

static inline void ISkinObj_DrawByState_C(const ISkinObj* pThis, IRenderTarget* pRT, LPCRECT rcDraw, DWORD dwState)
{
    ISkinObj_DrawByState(pThis, pRT, rcDraw, dwState);
}

static inline void ISkinObj_DrawByIndex2_C(const ISkinObj* pThis, IRenderTarget* pRT, LPCRECT rcDraw, int iState, BYTE byAlpha)
{
    ISkinObj_DrawByIndex2(pThis, pRT, rcDraw, iState, byAlpha);
}

static inline void ISkinObj_DrawByIndex_C(const ISkinObj* pThis, IRenderTarget* pRT, LPCRECT rcDraw, int iState)
{
    ISkinObj_DrawByIndex(pThis, pRT, rcDraw, iState);
}

static inline SIZE ISkinObj_GetSkinSize_C(const ISkinObj* pThis)
{
    return ISkinObj_GetSkinSize(pThis);
}

static inline int ISkinObj_GetStates_C(const ISkinObj* pThis)
{
    return ISkinObj_GetStates(pThis);
}

static inline BYTE ISkinObj_GetAlpha_C(const ISkinObj* pThis)
{
    return ISkinObj_GetAlpha(pThis);
}

static inline void ISkinObj_SetAlpha_C(ISkinObj* pThis, BYTE byAlpha)
{
    ISkinObj_SetAlpha(pThis, byAlpha);
}

static inline void ISkinObj_OnColorize_C(ISkinObj* pThis, COLORREF cr)
{
    ISkinObj_OnColorize(pThis, cr);
}

static inline int ISkinObj_GetScale_C(const ISkinObj* pThis)
{
    return ISkinObj_GetScale(pThis);
}

static inline void ISkinObj_SetScale_C(ISkinObj* pThis, int scale)
{
    ISkinObj_SetScale(pThis, scale);
}

static inline ISkinObj* ISkinObj_Scale_C(ISkinObj* pThis, int nScale)
{
    return ISkinObj_Scale(pThis, nScale);
}

/*
 * Convenience macros for common skin object operations
 */

/* Drawing shortcuts */
#define ISkinObj_Draw(This, pRT, rcDraw) \
    ISkinObj_DrawByState(This, pRT, rcDraw, 0)

#define ISkinObj_DrawWithAlpha(This, pRT, rcDraw, alpha) \
    ISkinObj_DrawByState2(This, pRT, rcDraw, 0, alpha)

#define ISkinObj_DrawState(This, pRT, rcDraw, state) \
    ISkinObj_DrawByState(This, pRT, rcDraw, state)

#define ISkinObj_DrawStateWithAlpha(This, pRT, rcDraw, state, alpha) \
    ISkinObj_DrawByState2(This, pRT, rcDraw, state, alpha)

#define ISkinObj_DrawIndex(This, pRT, rcDraw, index) \
    ISkinObj_DrawByIndex(This, pRT, rcDraw, index)

#define ISkinObj_DrawIndexWithAlpha(This, pRT, rcDraw, index, alpha) \
    ISkinObj_DrawByIndex2(This, pRT, rcDraw, index, alpha)

/* Size and state shortcuts */
#define ISkinObj_GetWidth(This) \
    (ISkinObj_GetSkinSize(This).cx)

#define ISkinObj_GetHeight(This) \
    (ISkinObj_GetSkinSize(This).cy)

#define ISkinObj_GetStateCount(This) \
    ISkinObj_GetStates(This)

#define ISkinObj_HasMultipleStates(This) \
    (ISkinObj_GetStates(This) > 1)

#define ISkinObj_IsValidState(This, state) \
    ((state) >= 0 && (state) < ISkinObj_GetStates(This))

/* Alpha shortcuts */
#define ISkinObj_SetOpaque(This) \
    ISkinObj_SetAlpha(This, 255)

#define ISkinObj_SetTransparent(This) \
    ISkinObj_SetAlpha(This, 0)

#define ISkinObj_SetHalfTransparent(This) \
    ISkinObj_SetAlpha(This, 128)

#define ISkinObj_IsOpaque(This) \
    (ISkinObj_GetAlpha(This) == 255)

#define ISkinObj_IsTransparent(This) \
    (ISkinObj_GetAlpha(This) == 0)

#define ISkinObj_IsVisible(This) \
    (ISkinObj_GetAlpha(This) > 0)

/* Scale shortcuts */
#define ISkinObj_SetScale100(This) \
    ISkinObj_SetScale(This, 100)

#define ISkinObj_SetScale125(This) \
    ISkinObj_SetScale(This, 125)

#define ISkinObj_SetScale150(This) \
    ISkinObj_SetScale(This, 150)

#define ISkinObj_SetScale200(This) \
    ISkinObj_SetScale(This, 200)

#define ISkinObj_IsNormalScale(This) \
    (ISkinObj_GetScale(This) == 100)

/* Safe skin object operations */
#define ISkinObj_SafeDraw(This, pRT, rcDraw, state) \
    do { \
        if ((This) && (pRT) && (rcDraw)) { \
            ISkinObj_DrawByState(This, pRT, rcDraw, state); \
        } \
    } while(0)

#define ISkinObj_SafeDrawWithAlpha(This, pRT, rcDraw, state, alpha) \
    do { \
        if ((This) && (pRT) && (rcDraw)) { \
            ISkinObj_DrawByState2(This, pRT, rcDraw, state, alpha); \
        } \
    } while(0)

#define ISkinObj_SafeSetAlpha(This, alpha) \
    do { \
        if (This) { \
            ISkinObj_SetAlpha(This, alpha); \
        } \
    } while(0)

#define ISkinObj_SafeSetScale(This, scale) \
    do { \
        if ((This) && (scale) > 0) { \
            ISkinObj_SetScale(This, scale); \
        } \
    } while(0)

#define ISkinObj_SafeColorize(This, color) \
    do { \
        if (This) { \
            ISkinObj_OnColorize(This, color); \
        } \
    } while(0)

#define ISkinObj_SafeScale(This, scale, ppScaled) \
    do { \
        if ((This) && (scale) > 0 && (ppScaled)) { \
            *(ppScaled) = ISkinObj_Scale(This, scale); \
        } else if (ppScaled) { \
            *(ppScaled) = NULL; \
        } \
    } while(0)

/*
 * Skin object helper functions
 */
static inline BOOL ISkinObj_IsValid_C(const ISkinObj* pThis)
{
    return (pThis != NULL);
}

static inline BOOL ISkinObj_CanDraw_C(const ISkinObj* pThis)
{
    if (!pThis) return FALSE;
    
    SIZE size = ISkinObj_GetSkinSize(pThis);
    return (size.cx > 0 && size.cy > 0 && ISkinObj_GetAlpha(pThis) > 0);
}

static inline void ISkinObj_DrawCentered_C(const ISkinObj* pThis, IRenderTarget* pRT, LPCRECT rcContainer, DWORD dwState)
{
    if (!pThis || !pRT || !rcContainer) return;
    
    SIZE skinSize = ISkinObj_GetSkinSize(pThis);
    int containerWidth = rcContainer->right - rcContainer->left;
    int containerHeight = rcContainer->bottom - rcContainer->top;
    
    RECT rcDraw;
    rcDraw.left = rcContainer->left + (containerWidth - skinSize.cx) / 2;
    rcDraw.top = rcContainer->top + (containerHeight - skinSize.cy) / 2;
    rcDraw.right = rcDraw.left + skinSize.cx;
    rcDraw.bottom = rcDraw.top + skinSize.cy;
    
    ISkinObj_DrawByState(pThis, pRT, &rcDraw, dwState);
}

static inline void ISkinObj_DrawStretched_C(const ISkinObj* pThis, IRenderTarget* pRT, LPCRECT rcDraw, DWORD dwState)
{
    if (!pThis || !pRT || !rcDraw) return;
    
    ISkinObj_DrawByState(pThis, pRT, rcDraw, dwState);
}

static inline void ISkinObj_DrawTiled_C(const ISkinObj* pThis, IRenderTarget* pRT, LPCRECT rcArea, DWORD dwState)
{
    if (!pThis || !pRT || !rcArea) return;
    
    SIZE skinSize = ISkinObj_GetSkinSize(pThis);
    if (skinSize.cx <= 0 || skinSize.cy <= 0) return;
    
    int areaWidth = rcArea->right - rcArea->left;
    int areaHeight = rcArea->bottom - rcArea->top;
    
    for (int y = rcArea->top; y < rcArea->bottom; y += skinSize.cy) {
        for (int x = rcArea->left; x < rcArea->right; x += skinSize.cx) {
            RECT rcTile;
            rcTile.left = x;
            rcTile.top = y;
            rcTile.right = min(x + skinSize.cx, rcArea->right);
            rcTile.bottom = min(y + skinSize.cy, rcArea->bottom);
            
            ISkinObj_DrawByState(pThis, pRT, &rcTile, dwState);
        }
    }
}

static inline ISkinObj* ISkinObj_CreateScaledCopy_C(ISkinObj* pThis, int nScale)
{
    if (!pThis || nScale <= 0) return NULL;
    
    return ISkinObj_Scale(pThis, nScale);
}

/*
 * Skin object state management
 */
typedef struct SkinObjState {
    ISkinObj* skinObj;
    SIZE skinSize;
    int stateCount;
    BYTE alpha;
    int scale;
    BOOL isValid;
    BOOL canDraw;
} SkinObjState;

static inline void SkinObjState_Init(SkinObjState* state, ISkinObj* skinObj)
{
    if (state) {
        state->skinObj = skinObj;
        if (skinObj) {
            state->skinSize = ISkinObj_GetSkinSize(skinObj);
            state->stateCount = ISkinObj_GetStates(skinObj);
            state->alpha = ISkinObj_GetAlpha(skinObj);
            state->scale = ISkinObj_GetScale(skinObj);
            state->isValid = TRUE;
            state->canDraw = ISkinObj_CanDraw_C(skinObj);
        } else {
            memset(state, 0, sizeof(SkinObjState));
        }
    }
}

static inline void SkinObjState_Update(SkinObjState* state)
{
    if (state && state->skinObj) {
        SkinObjState_Init(state, state->skinObj);
    }
}

static inline void SkinObjState_SetAlpha(SkinObjState* state, BYTE alpha)
{
    if (state && state->skinObj) {
        ISkinObj_SetAlpha(state->skinObj, alpha);
        state->alpha = alpha;
        state->canDraw = ISkinObj_CanDraw_C(state->skinObj);
    }
}

static inline void SkinObjState_SetScale(SkinObjState* state, int scale)
{
    if (state && state->skinObj && scale > 0) {
        ISkinObj_SetScale(state->skinObj, scale);
        state->scale = scale;
    }
}

/*
 * Reference counting helpers
 */
#define ISkinObj_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define ISkinObj_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void ISkinObj_DebugInfo_C(const ISkinObj* pThis)
{
    if (!pThis) {
        printf("SkinObj: NULL\n");
        return;
    }
    
    SIZE size = ISkinObj_GetSkinSize(pThis);
    printf("SkinObj: %p\n", pThis);
    printf("  Size: %dx%d\n", size.cx, size.cy);
    printf("  States: %d\n", ISkinObj_GetStates(pThis));
    printf("  Alpha: %d\n", ISkinObj_GetAlpha(pThis));
    printf("  Scale: %d\n", ISkinObj_GetScale(pThis));
    printf("  Can Draw: %s\n", ISkinObj_CanDraw_C(pThis) ? "Yes" : "No");
}
#else
#define ISkinObj_DebugInfo_C(This) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SSKINOBJ_CAPI_H__ */
