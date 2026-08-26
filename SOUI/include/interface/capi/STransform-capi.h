#ifndef __STRANSFORM_CAPI_H__
#define __STRANSFORM_CAPI_H__

#include "../STransform-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for STransform Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ITransformation C API Macros */
#define ITransformation_GetMatrix(This) \
    ((This)->lpVtbl->GetMatrix(This))

#define ITransformation_GetAlpha(This) \
    ((This)->lpVtbl->GetAlpha(This))

#define ITransformation_SetAlpha(This, alpha) \
    ((This)->lpVtbl->SetAlpha(This, alpha))

#define ITransformation_Clear(This) \
    ((This)->lpVtbl->Clear(This))

#define ITransformation_SetTransformationType(This, type) \
    ((This)->lpVtbl->SetTransformationType(This, type))

#define ITransformation_Compose(This, t) \
    ((This)->lpVtbl->Compose(This, t))

#ifdef __cplusplus
}
#endif

#endif /* __STRANSFORM_CAPI_H__ */
