#ifndef __SGRADIENT_CAPI_H__
#define __SGRADIENT_CAPI_H__

#include "../SGradient-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SGradient Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IGradient C API Macros */
#define IGradient_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IGradient_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IGradient_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IGradient_GetGradientData(This) \
    ((This)->lpVtbl->GetGradientData(This))

#define IGradient_GetGradientLength(This) \
    ((This)->lpVtbl->GetGradientLength(This))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IGradient Helper Functions */
static inline long IGradient_AddRef_C(IGradient* pThis)
{
    return IGradient_AddRef(pThis);
}

static inline long IGradient_Release_C(IGradient* pThis)
{
    return IGradient_Release(pThis);
}

static inline const GradientItem* IGradient_GetGradientData_C(IGradient* pThis)
{
    return IGradient_GetGradientData(pThis);
}

static inline int IGradient_GetGradientLength_C(IGradient* pThis)
{
    return IGradient_GetGradientLength(pThis);
}

/*
 * Convenience macros for common gradient operations
 */

/* Gradient data access shortcuts */
#define IGradient_GetData(This) \
    IGradient_GetGradientData(This)

#define IGradient_GetLength(This) \
    IGradient_GetGradientLength(This)

#define IGradient_GetCount(This) \
    IGradient_GetGradientLength(This)

#define IGradient_IsEmpty(This) \
    (IGradient_GetGradientLength(This) == 0)

#define IGradient_HasData(This) \
    (IGradient_GetGradientLength(This) > 0)

/* Safe gradient operations */
#define IGradient_SafeGetData(This, ppData) \
    do { \
        if ((This) && (ppData)) { \
            *(ppData) = IGradient_GetGradientData(This); \
        } else if (ppData) { \
            *(ppData) = NULL; \
        } \
    } while(0)

#define IGradient_SafeGetLength(This, pLength) \
    do { \
        if ((This) && (pLength)) { \
            *(pLength) = IGradient_GetGradientLength(This); \
        } else if (pLength) { \
            *(pLength) = 0; \
        } \
    } while(0)

/*
 * Gradient helper functions
 */
static inline BOOL IGradient_IsValid_C(IGradient* pThis)
{
    if (!pThis) return FALSE;
    
    int length = IGradient_GetGradientLength(pThis);
    const GradientItem* data = IGradient_GetGradientData(pThis);
    
    return (length > 0 && data != NULL);
}

static inline GradientItem IGradient_GetItemAt_C(IGradient* pThis, int index)
{
    GradientItem item = {0, 0.0f};
    
    if (!pThis) return item;
    
    int length = IGradient_GetGradientLength(pThis);
    const GradientItem* data = IGradient_GetGradientData(pThis);
    
    if (data && index >= 0 && index < length) {
        item = data[index];
    }
    
    return item;
}

static inline COLORREF IGradient_GetColorAt_C(IGradient* pThis, int index)
{
    if (!pThis) return 0;
    
    int length = IGradient_GetGradientLength(pThis);
    const GradientItem* data = IGradient_GetGradientData(pThis);
    
    if (data && index >= 0 && index < length) {
        return data[index].cr;
    }
    
    return 0;
}

static inline float IGradient_GetPositionAt_C(IGradient* pThis, int index)
{
    if (!pThis) return 0.0f;
    
    int length = IGradient_GetGradientLength(pThis);
    const GradientItem* data = IGradient_GetGradientData(pThis);
    
    if (data && index >= 0 && index < length) {
        return data[index].pos;
    }
    
    return 0.0f;
}

static inline GradientItem IGradient_GetFirstItem_C(IGradient* pThis)
{
    return IGradient_GetItemAt_C(pThis, 0);
}

static inline GradientItem IGradient_GetLastItem_C(IGradient* pThis)
{
    if (!pThis) {
        GradientItem item = {0, 0.0f};
        return item;
    }
    
    int length = IGradient_GetGradientLength(pThis);
    return IGradient_GetItemAt_C(pThis, length - 1);
}

static inline COLORREF IGradient_GetStartColor_C(IGradient* pThis)
{
    return IGradient_GetColorAt_C(pThis, 0);
}

static inline COLORREF IGradient_GetEndColor_C(IGradient* pThis)
{
    if (!pThis) return 0;
    
    int length = IGradient_GetGradientLength(pThis);
    return IGradient_GetColorAt_C(pThis, length - 1);
}

/*
 * Gradient interpolation helpers
 */
static inline COLORREF IGradient_InterpolateColor_C(IGradient* pThis, float position)
{
    if (!pThis || position < 0.0f || position > 1.0f) return 0;
    
    int length = IGradient_GetGradientLength(pThis);
    const GradientItem* data = IGradient_GetGradientData(pThis);
    
    if (!data || length == 0) return 0;
    if (length == 1) return data[0].cr;
    
    // Find the two gradient items to interpolate between
    for (int i = 0; i < length - 1; i++) {
        if (position >= data[i].pos && position <= data[i + 1].pos) {
            float t = (position - data[i].pos) / (data[i + 1].pos - data[i].pos);
            
            COLORREF c1 = data[i].cr;
            COLORREF c2 = data[i + 1].cr;
            
            BYTE r1 = GetRValue(c1), g1 = GetGValue(c1), b1 = GetBValue(c1);
            BYTE r2 = GetRValue(c2), g2 = GetGValue(c2), b2 = GetBValue(c2);
            
            BYTE r = (BYTE)(r1 + (r2 - r1) * t);
            BYTE g = (BYTE)(g1 + (g2 - g1) * t);
            BYTE b = (BYTE)(b1 + (b2 - b1) * t);
            
            return RGB(r, g, b);
        }
    }
    
    // If position is beyond the last item, return the last color
    return data[length - 1].cr;
}

static inline int IGradient_FindNearestIndex_C(IGradient* pThis, float position)
{
    if (!pThis || position < 0.0f || position > 1.0f) return -1;
    
    int length = IGradient_GetGradientLength(pThis);
    const GradientItem* data = IGradient_GetGradientData(pThis);
    
    if (!data || length == 0) return -1;
    
    int nearestIndex = 0;
    float minDistance = fabsf(data[0].pos - position);
    
    for (int i = 1; i < length; i++) {
        float distance = fabsf(data[i].pos - position);
        if (distance < minDistance) {
            minDistance = distance;
            nearestIndex = i;
        }
    }
    
    return nearestIndex;
}

/*
 * Gradient analysis helpers
 */
static inline BOOL IGradient_IsMonochrome_C(IGradient* pThis)
{
    if (!pThis) return TRUE;
    
    int length = IGradient_GetGradientLength(pThis);
    const GradientItem* data = IGradient_GetGradientData(pThis);
    
    if (!data || length <= 1) return TRUE;
    
    COLORREF firstColor = data[0].cr;
    for (int i = 1; i < length; i++) {
        if (data[i].cr != firstColor) {
            return FALSE;
        }
    }
    
    return TRUE;
}

static inline BOOL IGradient_HasTransparency_C(IGradient* pThis)
{
    if (!pThis) return FALSE;
    
    int length = IGradient_GetGradientLength(pThis);
    const GradientItem* data = IGradient_GetGradientData(pThis);
    
    if (!data) return FALSE;
    
    for (int i = 0; i < length; i++) {
        // Check if any color has alpha channel (assuming COLORREF includes alpha)
        if ((data[i].cr & 0xFF000000) != 0xFF000000) {
            return TRUE;
        }
    }
    
    return FALSE;
}

static inline float IGradient_GetTotalRange_C(IGradient* pThis)
{
    if (!pThis) return 0.0f;
    
    int length = IGradient_GetGradientLength(pThis);
    const GradientItem* data = IGradient_GetGradientData(pThis);
    
    if (!data || length == 0) return 0.0f;
    if (length == 1) return 0.0f;
    
    return data[length - 1].pos - data[0].pos;
}

/*
 * Gradient copying and conversion helpers
 */
static inline BOOL IGradient_CopyToArray_C(IGradient* pThis, GradientItem* buffer, int bufferSize)
{
    if (!pThis || !buffer || bufferSize <= 0) return FALSE;
    
    int length = IGradient_GetGradientLength(pThis);
    const GradientItem* data = IGradient_GetGradientData(pThis);
    
    if (!data || length == 0) return FALSE;
    
    int copyCount = (length < bufferSize) ? length : bufferSize;
    for (int i = 0; i < copyCount; i++) {
        buffer[i] = data[i];
    }
    
    return TRUE;
}

static inline BOOL IGradient_ExtractColors_C(IGradient* pThis, COLORREF* colors, int bufferSize)
{
    if (!pThis || !colors || bufferSize <= 0) return FALSE;
    
    int length = IGradient_GetGradientLength(pThis);
    const GradientItem* data = IGradient_GetGradientData(pThis);
    
    if (!data || length == 0) return FALSE;
    
    int copyCount = (length < bufferSize) ? length : bufferSize;
    for (int i = 0; i < copyCount; i++) {
        colors[i] = data[i].cr;
    }
    
    return TRUE;
}

static inline BOOL IGradient_ExtractPositions_C(IGradient* pThis, float* positions, int bufferSize)
{
    if (!pThis || !positions || bufferSize <= 0) return FALSE;
    
    int length = IGradient_GetGradientLength(pThis);
    const GradientItem* data = IGradient_GetGradientData(pThis);
    
    if (!data || length == 0) return FALSE;
    
    int copyCount = (length < bufferSize) ? length : bufferSize;
    for (int i = 0; i < copyCount; i++) {
        positions[i] = data[i].pos;
    }
    
    return TRUE;
}

/*
 * Reference counting helpers
 */
#define IGradient_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IGradient_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Gradient state management
 */
typedef struct GradientState {
    IGradient* gradient;
    int length;
    const GradientItem* data;
    BOOL isValid;
    BOOL isMonochrome;
    COLORREF startColor;
    COLORREF endColor;
} GradientState;

static inline void GradientState_Init(GradientState* state, IGradient* gradient)
{
    if (state) {
        state->gradient = gradient;
        if (gradient) {
            state->length = IGradient_GetGradientLength(gradient);
            state->data = IGradient_GetGradientData(gradient);
            state->isValid = (state->length > 0 && state->data != NULL);
            state->isMonochrome = IGradient_IsMonochrome_C(gradient);
            state->startColor = IGradient_GetStartColor_C(gradient);
            state->endColor = IGradient_GetEndColor_C(gradient);
        } else {
            state->length = 0;
            state->data = NULL;
            state->isValid = FALSE;
            state->isMonochrome = TRUE;
            state->startColor = 0;
            state->endColor = 0;
        }
    }
}

static inline void GradientState_Update(GradientState* state)
{
    if (state && state->gradient) {
        GradientState_Init(state, state->gradient);
    }
}

static inline COLORREF GradientState_InterpolateColor(const GradientState* state, float position)
{
    if (!state || !state->isValid) return 0;
    
    return IGradient_InterpolateColor_C(state->gradient, position);
}

static inline BOOL GradientState_IsValidPosition(const GradientState* state, float position)
{
    return state && state->isValid && position >= 0.0f && position <= 1.0f;
}

/*
 * Common gradient patterns (helper functions for creating gradient data)
 */
static inline GradientItem GradientItem_Create(COLORREF color, float position)
{
    GradientItem item;
    item.cr = color;
    item.pos = position;
    return item;
}

static inline GradientItem GradientItem_CreateRGB(BYTE r, BYTE g, BYTE b, float position)
{
    return GradientItem_Create(RGB(r, g, b), position);
}

/*
 * Gradient debugging helpers
 */
#ifdef _DEBUG
static inline void IGradient_DebugPrint_C(IGradient* pThis)
{
    if (!pThis) {
        printf("Gradient: NULL\n");
        return;
    }
    
    int length = IGradient_GetGradientLength(pThis);
    const GradientItem* data = IGradient_GetGradientData(pThis);
    
    printf("Gradient: %d items\n", length);
    if (data) {
        for (int i = 0; i < length; i++) {
            printf("  [%d] Color: 0x%08X, Position: %.3f\n", 
                   i, data[i].cr, data[i].pos);
        }
    } else {
        printf("  Data: NULL\n");
    }
}
#else
#define IGradient_DebugPrint_C(This) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SGRADIENT_CAPI_H__ */
