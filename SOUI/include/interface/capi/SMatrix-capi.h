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

#ifdef __cplusplus
}
#endif

#endif /* __SMATRIX_CAPI_H__ */
