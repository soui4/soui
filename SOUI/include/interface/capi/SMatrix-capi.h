#ifndef __SMATRIX_CAPI_H__
#define __SMATRIX_CAPI_H__

#include "../SMatrix-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SMatrix Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IMatrix C API Macros */
#define IMatrix_Data(This) \
    ((This)->lpVtbl->Data(This))

#define IMatrix_reset(This) \
    ((This)->lpVtbl->reset(This))

#define IMatrix_setIdentity(This) \
    ((This)->lpVtbl->setIdentity(This))

#define IMatrix_isIdentity(This) \
    ((This)->lpVtbl->isIdentity(This))

#define IMatrix_setTranslate(This, dx, dy) \
    ((This)->lpVtbl->setTranslate(This, dx, dy))

#define IMatrix_setScale(This, sx, sy) \
    ((This)->lpVtbl->setScale(This, sx, sy))

#define IMatrix_setScale2(This, sx, sy, px, py) \
    ((This)->lpVtbl->setScale2(This, sx, sy, px, py))

#define IMatrix_setRotate(This, degrees) \
    ((This)->lpVtbl->setRotate(This, degrees))

#define IMatrix_setRotate2(This, degrees, px, py) \
    ((This)->lpVtbl->setRotate2(This, degrees, px, py))

#define IMatrix_setSkew(This, kx, ky) \
    ((This)->lpVtbl->setSkew(This, kx, ky))

#define IMatrix_setSkew2(This, kx, ky, px, py) \
    ((This)->lpVtbl->setSkew2(This, kx, ky, px, py))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IMatrix Helper Functions */
static inline IxForm* IMatrix_Data_C(IMatrix* pThis)
{
    return IMatrix_Data(pThis);
}

static inline void IMatrix_reset_C(IMatrix* pThis)
{
    IMatrix_reset(pThis);
}

static inline void IMatrix_setIdentity_C(IMatrix* pThis)
{
    IMatrix_setIdentity(pThis);
}

static inline BOOL IMatrix_isIdentity_C(IMatrix* pThis)
{
    return IMatrix_isIdentity(pThis);
}

static inline void IMatrix_setTranslate_C(IMatrix* pThis, float dx, float dy)
{
    IMatrix_setTranslate(pThis, dx, dy);
}

static inline void IMatrix_setScale_C(IMatrix* pThis, float sx, float sy)
{
    IMatrix_setScale(pThis, sx, sy);
}

static inline void IMatrix_setScale2_C(IMatrix* pThis, float sx, float sy, float px, float py)
{
    IMatrix_setScale2(pThis, sx, sy, px, py);
}

static inline void IMatrix_setRotate_C(IMatrix* pThis, float degrees)
{
    IMatrix_setRotate(pThis, degrees);
}

static inline void IMatrix_setRotate2_C(IMatrix* pThis, float degrees, float px, float py)
{
    IMatrix_setRotate2(pThis, degrees, px, py);
}

static inline void IMatrix_setSkew_C(IMatrix* pThis, float kx, float ky)
{
    IMatrix_setSkew(pThis, kx, ky);
}

static inline void IMatrix_setSkew2_C(IMatrix* pThis, float kx, float ky, float px, float py)
{
    IMatrix_setSkew2(pThis, kx, ky, px, py);
}

/*
 * Convenience macros for common matrix operations
 */

/* Matrix initialization shortcuts */
#define IMatrix_Reset(This) \
    IMatrix_reset(This)

#define IMatrix_Identity(This) \
    IMatrix_setIdentity(This)

#define IMatrix_Clear(This) \
    IMatrix_reset(This)

/* Translation shortcuts */
#define IMatrix_Translate(This, dx, dy) \
    IMatrix_setTranslate(This, dx, dy)

#define IMatrix_TranslateX(This, dx) \
    IMatrix_setTranslate(This, dx, 0.0f)

#define IMatrix_TranslateY(This, dy) \
    IMatrix_setTranslate(This, 0.0f, dy)

/* Scaling shortcuts */
#define IMatrix_Scale(This, sx, sy) \
    IMatrix_setScale(This, sx, sy)

#define IMatrix_ScaleUniform(This, scale) \
    IMatrix_setScale(This, scale, scale)

#define IMatrix_ScaleX(This, sx) \
    IMatrix_setScale(This, sx, 1.0f)

#define IMatrix_ScaleY(This, sy) \
    IMatrix_setScale(This, 1.0f, sy)

#define IMatrix_ScaleAt(This, sx, sy, px, py) \
    IMatrix_setScale2(This, sx, sy, px, py)

#define IMatrix_ScaleUniformAt(This, scale, px, py) \
    IMatrix_setScale2(This, scale, scale, px, py)

/* Rotation shortcuts */
#define IMatrix_Rotate(This, degrees) \
    IMatrix_setRotate(This, degrees)

#define IMatrix_RotateAt(This, degrees, px, py) \
    IMatrix_setRotate2(This, degrees, px, py)

#define IMatrix_RotateRadians(This, radians) \
    IMatrix_setRotate(This, (radians) * 180.0f / 3.14159265f)

#define IMatrix_RotateRadiansAt(This, radians, px, py) \
    IMatrix_setRotate2(This, (radians) * 180.0f / 3.14159265f, px, py)

/* Skew shortcuts */
#define IMatrix_Skew(This, kx, ky) \
    IMatrix_setSkew(This, kx, ky)

#define IMatrix_SkewX(This, kx) \
    IMatrix_setSkew(This, kx, 0.0f)

#define IMatrix_SkewY(This, ky) \
    IMatrix_setSkew(This, 0.0f, ky)

#define IMatrix_SkewAt(This, kx, ky, px, py) \
    IMatrix_setSkew2(This, kx, ky, px, py)

/* Matrix state checking */
#define IMatrix_IsIdentity(This) \
    IMatrix_isIdentity(This)

#define IMatrix_IsTransformed(This) \
    (!IMatrix_isIdentity(This))

/* Safe matrix operations */
#define IMatrix_SafeReset(This) \
    do { \
        if (This) { \
            IMatrix_reset(This); \
        } \
    } while(0)

#define IMatrix_SafeSetIdentity(This) \
    do { \
        if (This) { \
            IMatrix_setIdentity(This); \
        } \
    } while(0)

#define IMatrix_SafeTranslate(This, dx, dy) \
    do { \
        if (This) { \
            IMatrix_setTranslate(This, dx, dy); \
        } \
    } while(0)

#define IMatrix_SafeScale(This, sx, sy) \
    do { \
        if (This) { \
            IMatrix_setScale(This, sx, sy); \
        } \
    } while(0)

#define IMatrix_SafeRotate(This, degrees) \
    do { \
        if (This) { \
            IMatrix_setRotate(This, degrees); \
        } \
    } while(0)

/*
 * Matrix transformation helper functions
 */
static inline void IMatrix_SetTransform_C(IMatrix* pThis, float tx, float ty, float sx, float sy, float rotation)
{
    if (!pThis) return;
    
    IMatrix_setIdentity(pThis);
    if (tx != 0.0f || ty != 0.0f) {
        IMatrix_setTranslate(pThis, tx, ty);
    }
    if (sx != 1.0f || sy != 1.0f) {
        IMatrix_setScale(pThis, sx, sy);
    }
    if (rotation != 0.0f) {
        IMatrix_setRotate(pThis, rotation);
    }
}

static inline void IMatrix_SetTransformAt_C(IMatrix* pThis, float tx, float ty, float sx, float sy, float rotation, float px, float py)
{
    if (!pThis) return;
    
    IMatrix_setIdentity(pThis);
    if (tx != 0.0f || ty != 0.0f) {
        IMatrix_setTranslate(pThis, tx, ty);
    }
    if (sx != 1.0f || sy != 1.0f) {
        IMatrix_setScale2(pThis, sx, sy, px, py);
    }
    if (rotation != 0.0f) {
        IMatrix_setRotate2(pThis, rotation, px, py);
    }
}

static inline BOOL IMatrix_IsScaled_C(IMatrix* pThis)
{
    if (!pThis) return FALSE;
    
    IxForm* data = IMatrix_Data(pThis);
    if (!data) return FALSE;
    
    // Check if scale factors are not 1.0 (assuming IxForm has scale components)
    // Note: This is a simplified check - actual implementation depends on IxForm structure
    return !IMatrix_isIdentity(pThis);
}

static inline BOOL IMatrix_IsRotated_C(IMatrix* pThis)
{
    if (!pThis) return FALSE;
    
    // Similar to IsScaled, this would need access to IxForm internals
    return !IMatrix_isIdentity(pThis);
}

static inline BOOL IMatrix_IsTranslated_C(IMatrix* pThis)
{
    if (!pThis) return FALSE;
    
    // Similar to above, this would need access to IxForm internals
    return !IMatrix_isIdentity(pThis);
}

/*
 * Common transformation presets
 */
#define IMatrix_SetFlipX(This) \
    IMatrix_setScale(This, -1.0f, 1.0f)

#define IMatrix_SetFlipY(This) \
    IMatrix_setScale(This, 1.0f, -1.0f)

#define IMatrix_SetFlipXY(This) \
    IMatrix_setScale(This, -1.0f, -1.0f)

#define IMatrix_SetFlipXAt(This, px, py) \
    IMatrix_setScale2(This, -1.0f, 1.0f, px, py)

#define IMatrix_SetFlipYAt(This, px, py) \
    IMatrix_setScale2(This, 1.0f, -1.0f, px, py)

#define IMatrix_SetFlipXYAt(This, px, py) \
    IMatrix_setScale2(This, -1.0f, -1.0f, px, py)

/* Common rotation angles */
#define IMatrix_Rotate90(This) \
    IMatrix_setRotate(This, 90.0f)

#define IMatrix_Rotate180(This) \
    IMatrix_setRotate(This, 180.0f)

#define IMatrix_Rotate270(This) \
    IMatrix_setRotate(This, 270.0f)

#define IMatrix_Rotate90At(This, px, py) \
    IMatrix_setRotate2(This, 90.0f, px, py)

#define IMatrix_Rotate180At(This, px, py) \
    IMatrix_setRotate2(This, 180.0f, px, py)

#define IMatrix_Rotate270At(This, px, py) \
    IMatrix_setRotate2(This, 270.0f, px, py)

/*
 * Matrix state management
 */
typedef struct MatrixState {
    IMatrix* matrix;
    BOOL isIdentity;
    BOOL hasTranslation;
    BOOL hasScale;
    BOOL hasRotation;
    BOOL hasSkew;
} MatrixState;

static inline void MatrixState_Init(MatrixState* state, IMatrix* matrix)
{
    if (state) {
        state->matrix = matrix;
        state->isIdentity = matrix ? IMatrix_isIdentity(matrix) : TRUE;
        state->hasTranslation = FALSE;
        state->hasScale = FALSE;
        state->hasRotation = FALSE;
        state->hasSkew = FALSE;
    }
}

static inline void MatrixState_Reset(MatrixState* state)
{
    if (state && state->matrix) {
        IMatrix_setIdentity(state->matrix);
        state->isIdentity = TRUE;
        state->hasTranslation = FALSE;
        state->hasScale = FALSE;
        state->hasRotation = FALSE;
        state->hasSkew = FALSE;
    }
}

static inline void MatrixState_Translate(MatrixState* state, float dx, float dy)
{
    if (state && state->matrix) {
        IMatrix_setTranslate(state->matrix, dx, dy);
        state->isIdentity = FALSE;
        state->hasTranslation = TRUE;
    }
}

static inline void MatrixState_Scale(MatrixState* state, float sx, float sy)
{
    if (state && state->matrix) {
        IMatrix_setScale(state->matrix, sx, sy);
        state->isIdentity = FALSE;
        state->hasScale = TRUE;
    }
}

static inline void MatrixState_Rotate(MatrixState* state, float degrees)
{
    if (state && state->matrix) {
        IMatrix_setRotate(state->matrix, degrees);
        state->isIdentity = FALSE;
        state->hasRotation = TRUE;
    }
}

static inline void MatrixState_Skew(MatrixState* state, float kx, float ky)
{
    if (state && state->matrix) {
        IMatrix_setSkew(state->matrix, kx, ky);
        state->isIdentity = FALSE;
        state->hasSkew = TRUE;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* __SMATRIX_CAPI_H__ */
