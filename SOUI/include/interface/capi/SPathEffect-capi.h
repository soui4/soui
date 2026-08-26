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

/*
 * ICornerPathEffect interface macros
 * ICornerPathEffect inherits from IPathEffect; use IPathEffect_ macros for inherited methods.
 */
#define ICornerPathEffect_Init(This, radius) \
    ((This)->lpVtbl->Init(This, radius))

/*
 * IDashPathEffect interface macros
 * IDashPathEffect inherits from IPathEffect; use IPathEffect_ macros for inherited methods.
 */
#define IDashPathEffect_Init(This, intervals, count, phase) \
    ((This)->lpVtbl->Init(This, intervals, count, phase))

/*
 * IDiscretePathEffect interface macros
 * IDiscretePathEffect inherits from IPathEffect; use IPathEffect_ macros for inherited methods.
 */
#define IDiscretePathEffect_Init(This, segmentLength, deviation) \
    ((This)->lpVtbl->Init(This, segmentLength, deviation))

#ifdef __cplusplus
}
#endif

#endif /* __SPATHEFFECT_CAPI_H__ */
