#ifndef __SPATHEFFECT_CAPI_H__
#define __SPATHEFFECT_CAPI_H__

#include "../SPathEffect-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SPathEffect Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IPathEffect C API Macros */
#define IPathEffect_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IPathEffect_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IPathEffect_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IPathEffect_GetRealPathEffect(This) \
    ((This)->lpVtbl->GetRealPathEffect(This))

/* ICornerPathEffect C API Macros */
#define ICornerPathEffect_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ICornerPathEffect_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ICornerPathEffect_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ICornerPathEffect_GetRealPathEffect(This) \
    ((This)->lpVtbl->GetRealPathEffect(This))

#define ICornerPathEffect_Init(This, radius) \
    ((This)->lpVtbl->Init(This, radius))

/* IDashPathEffect C API Macros */
#define IDashPathEffect_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IDashPathEffect_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IDashPathEffect_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IDashPathEffect_GetRealPathEffect(This) \
    ((This)->lpVtbl->GetRealPathEffect(This))

#define IDashPathEffect_Init(This, intervals, count, phase) \
    ((This)->lpVtbl->Init(This, intervals, count, phase))

/* IDiscretePathEffect C API Macros */
#define IDiscretePathEffect_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IDiscretePathEffect_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IDiscretePathEffect_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IDiscretePathEffect_GetRealPathEffect(This) \
    ((This)->lpVtbl->GetRealPathEffect(This))

#define IDiscretePathEffect_Init(This, segmentLength, deviation) \
    ((This)->lpVtbl->Init(This, segmentLength, deviation))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IPathEffect Helper Functions */
static inline long IPathEffect_AddRef_C(IPathEffect* pThis)
{
    return IPathEffect_AddRef(pThis);
}

static inline long IPathEffect_Release_C(IPathEffect* pThis)
{
    return IPathEffect_Release(pThis);
}

static inline void* IPathEffect_GetRealPathEffect_C(IPathEffect* pThis)
{
    return IPathEffect_GetRealPathEffect(pThis);
}

/* ICornerPathEffect Helper Functions */
static inline long ICornerPathEffect_AddRef_C(ICornerPathEffect* pThis)
{
    return ICornerPathEffect_AddRef(pThis);
}

static inline long ICornerPathEffect_Release_C(ICornerPathEffect* pThis)
{
    return ICornerPathEffect_Release(pThis);
}

static inline void ICornerPathEffect_Init_C(ICornerPathEffect* pThis, float radius)
{
    ICornerPathEffect_Init(pThis, radius);
}

static inline void* ICornerPathEffect_GetRealPathEffect_C(ICornerPathEffect* pThis)
{
    return ICornerPathEffect_GetRealPathEffect(pThis);
}

/* IDashPathEffect Helper Functions */
static inline long IDashPathEffect_AddRef_C(IDashPathEffect* pThis)
{
    return IDashPathEffect_AddRef(pThis);
}

static inline long IDashPathEffect_Release_C(IDashPathEffect* pThis)
{
    return IDashPathEffect_Release(pThis);
}

static inline void IDashPathEffect_Init_C(IDashPathEffect* pThis, float intervals[], int count, float phase)
{
    IDashPathEffect_Init(pThis, intervals, count, phase);
}

static inline void* IDashPathEffect_GetRealPathEffect_C(IDashPathEffect* pThis)
{
    return IDashPathEffect_GetRealPathEffect(pThis);
}

/* IDiscretePathEffect Helper Functions */
static inline long IDiscretePathEffect_AddRef_C(IDiscretePathEffect* pThis)
{
    return IDiscretePathEffect_AddRef(pThis);
}

static inline long IDiscretePathEffect_Release_C(IDiscretePathEffect* pThis)
{
    return IDiscretePathEffect_Release(pThis);
}

static inline void IDiscretePathEffect_Init_C(IDiscretePathEffect* pThis, float segmentLength, float deviation)
{
    IDiscretePathEffect_Init(pThis, segmentLength, deviation);
}

static inline void* IDiscretePathEffect_GetRealPathEffect_C(IDiscretePathEffect* pThis)
{
    return IDiscretePathEffect_GetRealPathEffect(pThis);
}

/*
 * Convenience macros for common path effect operations
 */

/* Corner path effect shortcuts */
#define ICornerPathEffect_SetRadius(This, radius) \
    ICornerPathEffect_Init(This, radius)

#define ICornerPathEffect_SetSmallRadius(This) \
    ICornerPathEffect_Init(This, 2.0f)

#define ICornerPathEffect_SetMediumRadius(This) \
    ICornerPathEffect_Init(This, 5.0f)

#define ICornerPathEffect_SetLargeRadius(This) \
    ICornerPathEffect_Init(This, 10.0f)

/* Dash path effect shortcuts */
#define IDashPathEffect_SetSimpleDash(This, dashLength, gapLength) \
    do { \
        float intervals[] = {dashLength, gapLength}; \
        IDashPathEffect_Init(This, intervals, 2, 0.0f); \
    } while(0)

#define IDashPathEffect_SetDotDash(This, dotLength, dashLength, gapLength) \
    do { \
        float intervals[] = {dotLength, gapLength, dashLength, gapLength}; \
        IDashPathEffect_Init(This, intervals, 4, 0.0f); \
    } while(0)

#define IDashPathEffect_SetDottedLine(This, dotLength, gapLength) \
    do { \
        float intervals[] = {dotLength, gapLength}; \
        IDashPathEffect_Init(This, intervals, 2, 0.0f); \
    } while(0)

#define IDashPathEffect_SetDashedLine(This, dashLength, gapLength) \
    do { \
        float intervals[] = {dashLength, gapLength}; \
        IDashPathEffect_Init(This, intervals, 2, 0.0f); \
    } while(0)

/* Discrete path effect shortcuts */
#define IDiscretePathEffect_SetLowNoise(This, segmentLength) \
    IDiscretePathEffect_Init(This, segmentLength, 1.0f)

#define IDiscretePathEffect_SetMediumNoise(This, segmentLength) \
    IDiscretePathEffect_Init(This, segmentLength, 3.0f)

#define IDiscretePathEffect_SetHighNoise(This, segmentLength) \
    IDiscretePathEffect_Init(This, segmentLength, 5.0f)

#define IDiscretePathEffect_SetCustomNoise(This, segmentLength, deviation) \
    IDiscretePathEffect_Init(This, segmentLength, deviation)

/* Safe path effect operations */
#define IPathEffect_SafeGetRealPathEffect(This, ppReal) \
    do { \
        if ((This) && (ppReal)) { \
            *(ppReal) = IPathEffect_GetRealPathEffect(This); \
        } else if (ppReal) { \
            *(ppReal) = NULL; \
        } \
    } while(0)

#define ICornerPathEffect_SafeInit(This, radius) \
    do { \
        if ((This) && (radius) >= 0.0f) { \
            ICornerPathEffect_Init(This, radius); \
        } \
    } while(0)

#define IDashPathEffect_SafeInit(This, intervals, count, phase) \
    do { \
        if ((This) && (intervals) && (count) > 0) { \
            IDashPathEffect_Init(This, intervals, count, phase); \
        } \
    } while(0)

#define IDiscretePathEffect_SafeInit(This, segmentLength, deviation) \
    do { \
        if ((This) && (segmentLength) > 0.0f && (deviation) >= 0.0f) { \
            IDiscretePathEffect_Init(This, segmentLength, deviation); \
        } \
    } while(0)

/*
 * Path effect helper functions
 */
static inline void IDashPathEffect_InitSimple_C(IDashPathEffect* pThis, float dashLength, float gapLength, float phase)
{
    if (!pThis || dashLength <= 0.0f || gapLength <= 0.0f) return;
    
    float intervals[] = {dashLength, gapLength};
    IDashPathEffect_Init(pThis, intervals, 2, phase);
}

static inline void IDashPathEffect_InitComplex_C(IDashPathEffect* pThis, float* intervals, int count, float phase)
{
    if (!pThis || !intervals || count <= 0) return;
    
    // Validate intervals - all should be positive
    for (int i = 0; i < count; i++) {
        if (intervals[i] <= 0.0f) return;
    }
    
    IDashPathEffect_Init(pThis, intervals, count, phase);
}

static inline void IDashPathEffect_InitDotDash_C(IDashPathEffect* pThis, float dotSize, float dashSize, float gapSize)
{
    if (!pThis || dotSize <= 0.0f || dashSize <= 0.0f || gapSize <= 0.0f) return;
    
    float intervals[] = {dotSize, gapSize, dashSize, gapSize};
    IDashPathEffect_Init(pThis, intervals, 4, 0.0f);
}

static inline void ICornerPathEffect_InitFromPixels_C(ICornerPathEffect* pThis, int radiusPixels, float dpi)
{
    if (!pThis || radiusPixels < 0 || dpi <= 0.0f) return;
    
    float radius = (float)radiusPixels * 96.0f / dpi; // Convert to device-independent units
    ICornerPathEffect_Init(pThis, radius);
}

static inline void IDiscretePathEffect_InitFromPixels_C(IDiscretePathEffect* pThis, int segmentPixels, int deviationPixels, float dpi)
{
    if (!pThis || segmentPixels <= 0 || deviationPixels < 0 || dpi <= 0.0f) return;
    
    float segmentLength = (float)segmentPixels * 96.0f / dpi;
    float deviation = (float)deviationPixels * 96.0f / dpi;
    IDiscretePathEffect_Init(pThis, segmentLength, deviation);
}

/*
 * Path effect type checking helpers
 */
static inline BOOL IPathEffect_IsCornerEffect_C(IPathEffect* pThis)
{
    if (!pThis) return FALSE;
    
    // This would require QueryInterface or similar mechanism
    // For now, return FALSE as we can't determine type without additional info
    return FALSE;
}

static inline BOOL IPathEffect_IsDashEffect_C(IPathEffect* pThis)
{
    if (!pThis) return FALSE;
    
    // This would require QueryInterface or similar mechanism
    return FALSE;
}

static inline BOOL IPathEffect_IsDiscreteEffect_C(IPathEffect* pThis)
{
    if (!pThis) return FALSE;
    
    // This would require QueryInterface or similar mechanism
    return FALSE;
}

/*
 * Reference counting helpers
 */
#define IPathEffect_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IPathEffect_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define ICornerPathEffect_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define ICornerPathEffect_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define IDashPathEffect_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IDashPathEffect_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define IDiscretePathEffect_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IDiscretePathEffect_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Common dash patterns
 */
typedef struct DashPattern {
    float* intervals;
    int count;
    float phase;
} DashPattern;

static inline DashPattern DashPattern_CreateSimple(float dashLength, float gapLength)
{
    static float intervals[2];
    intervals[0] = dashLength;
    intervals[1] = gapLength;
    
    DashPattern pattern = {intervals, 2, 0.0f};
    return pattern;
}

static inline DashPattern DashPattern_CreateDotted(float dotSize, float gapSize)
{
    static float intervals[2];
    intervals[0] = dotSize;
    intervals[1] = gapSize;
    
    DashPattern pattern = {intervals, 2, 0.0f};
    return pattern;
}

static inline DashPattern DashPattern_CreateDotDash(float dotSize, float dashSize, float gapSize)
{
    static float intervals[4];
    intervals[0] = dotSize;
    intervals[1] = gapSize;
    intervals[2] = dashSize;
    intervals[3] = gapSize;
    
    DashPattern pattern = {intervals, 4, 0.0f};
    return pattern;
}

static inline void IDashPathEffect_ApplyPattern_C(IDashPathEffect* pThis, const DashPattern* pattern)
{
    if (pThis && pattern && pattern->intervals && pattern->count > 0) {
        IDashPathEffect_Init(pThis, pattern->intervals, pattern->count, pattern->phase);
    }
}

/*
 * Predefined common patterns
 */
#define DASH_PATTERN_DOTTED     DashPattern_CreateDotted(1.0f, 2.0f)
#define DASH_PATTERN_DASHED     DashPattern_CreateSimple(5.0f, 3.0f)
#define DASH_PATTERN_DOT_DASH   DashPattern_CreateDotDash(1.0f, 5.0f, 3.0f)

#define IDashPathEffect_SetDottedPattern(This) \
    do { \
        DashPattern pattern = DASH_PATTERN_DOTTED; \
        IDashPathEffect_ApplyPattern_C(This, &pattern); \
    } while(0)

#define IDashPathEffect_SetDashedPattern(This) \
    do { \
        DashPattern pattern = DASH_PATTERN_DASHED; \
        IDashPathEffect_ApplyPattern_C(This, &pattern); \
    } while(0)

#define IDashPathEffect_SetDotDashPattern(This) \
    do { \
        DashPattern pattern = DASH_PATTERN_DOT_DASH; \
        IDashPathEffect_ApplyPattern_C(This, &pattern); \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif /* __SPATHEFFECT_CAPI_H__ */
