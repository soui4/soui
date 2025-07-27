#ifndef __STRANSFORM_CAPI_H__
#define __STRANSFORM_CAPI_H__

#include "../STransform-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Transformation type constants
 */
#define TRANSFORM_TYPE_IDENTITY  TYPE_IDENTITY
#define TRANSFORM_TYPE_ALPHA     TYPE_ALPHA
#define TRANSFORM_TYPE_MATRIX    TYPE_MATRIX
#define TRANSFORM_TYPE_BOTH      TYPE_BOTH

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

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* ITransformation Helper Functions */
static inline IMatrix* ITransformation_GetMatrix_C(ITransformation* pThis)
{
    return ITransformation_GetMatrix(pThis);
}

static inline BYTE ITransformation_GetAlpha_C(ITransformation* pThis)
{
    return ITransformation_GetAlpha(pThis);
}

static inline void ITransformation_SetAlpha_C(ITransformation* pThis, BYTE alpha)
{
    ITransformation_SetAlpha(pThis, alpha);
}

static inline void ITransformation_Clear_C(ITransformation* pThis)
{
    ITransformation_Clear(pThis);
}

static inline void ITransformation_SetTransformationType_C(ITransformation* pThis, int type)
{
    ITransformation_SetTransformationType(pThis, type);
}

static inline void ITransformation_Compose_C(ITransformation* pThis, const ITransformation* t)
{
    ITransformation_Compose(pThis, t);
}

/*
 * Convenience macros for common transformation operations
 */

/* Alpha operations */
#define ITransformation_SetOpaque(This) \
    ITransformation_SetAlpha(This, 255)

#define ITransformation_SetTransparent(This) \
    ITransformation_SetAlpha(This, 0)

#define ITransformation_SetHalfTransparent(This) \
    ITransformation_SetAlpha(This, 128)

#define ITransformation_IsOpaque(This) \
    (ITransformation_GetAlpha(This) == 255)

#define ITransformation_IsTransparent(This) \
    (ITransformation_GetAlpha(This) == 0)

#define ITransformation_IsVisible(This) \
    (ITransformation_GetAlpha(This) > 0)

/* Alpha percentage operations */
#define ITransformation_SetAlphaPercent(This, percent) \
    ITransformation_SetAlpha(This, (BYTE)((percent) * 255 / 100))

#define ITransformation_GetAlphaPercent(This) \
    ((int)(ITransformation_GetAlpha(This) * 100 / 255))

#define ITransformation_SetAlphaFloat(This, alpha) \
    ITransformation_SetAlpha(This, (BYTE)((alpha) * 255))

#define ITransformation_GetAlphaFloat(This) \
    ((float)(ITransformation_GetAlpha(This) / 255.0f))

/* Transformation type operations */
#define ITransformation_SetIdentity(This) \
    ITransformation_SetTransformationType(This, TRANSFORM_TYPE_IDENTITY)

#define ITransformation_SetAlphaOnly(This) \
    ITransformation_SetTransformationType(This, TRANSFORM_TYPE_ALPHA)

#define ITransformation_SetMatrixOnly(This) \
    ITransformation_SetTransformationType(This, TRANSFORM_TYPE_MATRIX)

#define ITransformation_SetBoth(This) \
    ITransformation_SetTransformationType(This, TRANSFORM_TYPE_BOTH)

/* Matrix access shortcuts */
#define ITransformation_GetMatrixData(This) \
    (ITransformation_GetMatrix(This) ? IMatrix_Data(ITransformation_GetMatrix(This)) : NULL)

#define ITransformation_ResetMatrix(This) \
    do { \
        IMatrix* matrix = ITransformation_GetMatrix(This); \
        if (matrix) { \
            IMatrix_reset(matrix); \
        } \
    } while(0)

#define ITransformation_SetMatrixIdentity(This) \
    do { \
        IMatrix* matrix = ITransformation_GetMatrix(This); \
        if (matrix) { \
            IMatrix_setIdentity(matrix); \
        } \
    } while(0)

/* Combined matrix operations */
#define ITransformation_Translate(This, dx, dy) \
    do { \
        IMatrix* matrix = ITransformation_GetMatrix(This); \
        if (matrix) { \
            IMatrix_setTranslate(matrix, dx, dy); \
        } \
    } while(0)

#define ITransformation_Scale(This, sx, sy) \
    do { \
        IMatrix* matrix = ITransformation_GetMatrix(This); \
        if (matrix) { \
            IMatrix_setScale(matrix, sx, sy); \
        } \
    } while(0)

#define ITransformation_Rotate(This, degrees) \
    do { \
        IMatrix* matrix = ITransformation_GetMatrix(This); \
        if (matrix) { \
            IMatrix_setRotate(matrix, degrees); \
        } \
    } while(0)

#define ITransformation_Skew(This, kx, ky) \
    do { \
        IMatrix* matrix = ITransformation_GetMatrix(This); \
        if (matrix) { \
            IMatrix_setSkew(matrix, kx, ky); \
        } \
    } while(0)

/* Safe transformation operations */
#define ITransformation_SafeClear(This) \
    do { \
        if (This) { \
            ITransformation_Clear(This); \
        } \
    } while(0)

#define ITransformation_SafeSetAlpha(This, alpha) \
    do { \
        if (This) { \
            ITransformation_SetAlpha(This, alpha); \
        } \
    } while(0)

#define ITransformation_SafeCompose(This, t) \
    do { \
        if ((This) && (t)) { \
            ITransformation_Compose(This, t); \
        } \
    } while(0)

/*
 * Transformation helper functions
 */
static inline void ITransformation_SetTransform_C(ITransformation* pThis, float tx, float ty, float sx, float sy, float rotation, BYTE alpha)
{
    if (!pThis) return;
    
    ITransformation_Clear(pThis);
    ITransformation_SetAlpha(pThis, alpha);
    
    IMatrix* matrix = ITransformation_GetMatrix(pThis);
    if (matrix) {
        IMatrix_setIdentity(matrix);
        if (tx != 0.0f || ty != 0.0f) {
            IMatrix_setTranslate(matrix, tx, ty);
        }
        if (sx != 1.0f || sy != 1.0f) {
            IMatrix_setScale(matrix, sx, sy);
        }
        if (rotation != 0.0f) {
            IMatrix_setRotate(matrix, rotation);
        }
    }
    
    if (alpha != 255) {
        if (tx != 0.0f || ty != 0.0f || sx != 1.0f || sy != 1.0f || rotation != 0.0f) {
            ITransformation_SetTransformationType(pThis, TRANSFORM_TYPE_BOTH);
        } else {
            ITransformation_SetTransformationType(pThis, TRANSFORM_TYPE_ALPHA);
        }
    } else {
        if (tx != 0.0f || ty != 0.0f || sx != 1.0f || sy != 1.0f || rotation != 0.0f) {
            ITransformation_SetTransformationType(pThis, TRANSFORM_TYPE_MATRIX);
        } else {
            ITransformation_SetTransformationType(pThis, TRANSFORM_TYPE_IDENTITY);
        }
    }
}

static inline void ITransformation_SetAlphaTransform_C(ITransformation* pThis, BYTE alpha)
{
    if (!pThis) return;
    
    ITransformation_Clear(pThis);
    ITransformation_SetAlpha(pThis, alpha);
    ITransformation_SetTransformationType(pThis, alpha == 255 ? TRANSFORM_TYPE_IDENTITY : TRANSFORM_TYPE_ALPHA);
}

static inline void ITransformation_SetMatrixTransform_C(ITransformation* pThis, float tx, float ty, float sx, float sy, float rotation)
{
    if (!pThis) return;
    
    ITransformation_Clear(pThis);
    ITransformation_SetAlpha(pThis, 255);
    
    IMatrix* matrix = ITransformation_GetMatrix(pThis);
    if (matrix) {
        IMatrix_setIdentity(matrix);
        if (tx != 0.0f || ty != 0.0f) {
            IMatrix_setTranslate(matrix, tx, ty);
        }
        if (sx != 1.0f || sy != 1.0f) {
            IMatrix_setScale(matrix, sx, sy);
        }
        if (rotation != 0.0f) {
            IMatrix_setRotate(matrix, rotation);
        }
    }
    
    if (tx != 0.0f || ty != 0.0f || sx != 1.0f || sy != 1.0f || rotation != 0.0f) {
        ITransformation_SetTransformationType(pThis, TRANSFORM_TYPE_MATRIX);
    } else {
        ITransformation_SetTransformationType(pThis, TRANSFORM_TYPE_IDENTITY);
    }
}

static inline BOOL ITransformation_IsIdentity_C(ITransformation* pThis)
{
    if (!pThis) return TRUE;
    
    if (ITransformation_GetAlpha(pThis) != 255) {
        return FALSE;
    }
    
    IMatrix* matrix = ITransformation_GetMatrix(pThis);
    return matrix ? IMatrix_isIdentity(matrix) : TRUE;
}

static inline BOOL ITransformation_HasMatrix_C(ITransformation* pThis)
{
    if (!pThis) return FALSE;
    
    IMatrix* matrix = ITransformation_GetMatrix(pThis);
    return matrix ? !IMatrix_isIdentity(matrix) : FALSE;
}

static inline BOOL ITransformation_HasAlpha_C(ITransformation* pThis)
{
    return pThis ? (ITransformation_GetAlpha(pThis) != 255) : FALSE;
}

/*
 * Transformation state management
 */
typedef struct TransformationState {
    ITransformation* transform;
    BYTE originalAlpha;
    BOOL hasOriginalMatrix;
    int transformationType;
} TransformationState;

static inline void TransformationState_Init(TransformationState* state, ITransformation* transform)
{
    if (state) {
        state->transform = transform;
        state->originalAlpha = transform ? ITransformation_GetAlpha(transform) : 255;
        state->hasOriginalMatrix = transform ? ITransformation_HasMatrix_C(transform) : FALSE;
        state->transformationType = TRANSFORM_TYPE_IDENTITY;
    }
}

static inline void TransformationState_Save(TransformationState* state)
{
    if (state && state->transform) {
        state->originalAlpha = ITransformation_GetAlpha(state->transform);
        state->hasOriginalMatrix = ITransformation_HasMatrix_C(state->transform);
    }
}

static inline void TransformationState_Restore(TransformationState* state)
{
    if (state && state->transform) {
        ITransformation_Clear(state->transform);
        ITransformation_SetAlpha(state->transform, state->originalAlpha);
        
        if (state->hasOriginalMatrix) {
            // Note: Restoring matrix would require saving matrix state
            // This is a simplified implementation
            ITransformation_SetTransformationType(state->transform, TRANSFORM_TYPE_MATRIX);
        } else if (state->originalAlpha != 255) {
            ITransformation_SetTransformationType(state->transform, TRANSFORM_TYPE_ALPHA);
        } else {
            ITransformation_SetTransformationType(state->transform, TRANSFORM_TYPE_IDENTITY);
        }
    }
}

static inline void TransformationState_Apply(TransformationState* state, float tx, float ty, float sx, float sy, float rotation, BYTE alpha)
{
    if (state && state->transform) {
        ITransformation_SetTransform_C(state->transform, tx, ty, sx, sy, rotation, alpha);
    }
}

/*
 * Common transformation presets
 */
#define ITransformation_FadeIn(This, alpha) \
    ITransformation_SetAlphaTransform_C(This, alpha)

#define ITransformation_FadeOut(This, alpha) \
    ITransformation_SetAlphaTransform_C(This, 255 - alpha)

#define ITransformation_ScaleUniform(This, scale) \
    ITransformation_SetMatrixTransform_C(This, 0.0f, 0.0f, scale, scale, 0.0f)

#define ITransformation_FlipX(This) \
    ITransformation_SetMatrixTransform_C(This, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f)

#define ITransformation_FlipY(This) \
    ITransformation_SetMatrixTransform_C(This, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f)

#define ITransformation_Rotate90(This) \
    ITransformation_SetMatrixTransform_C(This, 0.0f, 0.0f, 1.0f, 1.0f, 90.0f)

#define ITransformation_Rotate180(This) \
    ITransformation_SetMatrixTransform_C(This, 0.0f, 0.0f, 1.0f, 1.0f, 180.0f)

#define ITransformation_Rotate270(This) \
    ITransformation_SetMatrixTransform_C(This, 0.0f, 0.0f, 1.0f, 1.0f, 270.0f)

/*
 * Animation-friendly helpers
 */
static inline void ITransformation_InterpolateAlpha_C(ITransformation* pThis, BYTE startAlpha, BYTE endAlpha, float progress)
{
    if (!pThis || progress < 0.0f || progress > 1.0f) return;
    
    BYTE alpha = (BYTE)(startAlpha + (endAlpha - startAlpha) * progress);
    ITransformation_SetAlpha(pThis, alpha);
}

static inline void ITransformation_InterpolateScale_C(ITransformation* pThis, float startScale, float endScale, float progress)
{
    if (!pThis || progress < 0.0f || progress > 1.0f) return;
    
    float scale = startScale + (endScale - startScale) * progress;
    ITransformation_SetMatrixTransform_C(pThis, 0.0f, 0.0f, scale, scale, 0.0f);
}

static inline void ITransformation_InterpolateRotation_C(ITransformation* pThis, float startAngle, float endAngle, float progress)
{
    if (!pThis || progress < 0.0f || progress > 1.0f) return;
    
    float angle = startAngle + (endAngle - startAngle) * progress;
    ITransformation_SetMatrixTransform_C(pThis, 0.0f, 0.0f, 1.0f, 1.0f, angle);
}

#ifdef __cplusplus
}
#endif

#endif /* __STRANSFORM_CAPI_H__ */
