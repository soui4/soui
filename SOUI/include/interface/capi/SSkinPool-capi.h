#ifndef __SSKINPOOL_CAPI_H__
#define __SSKINPOOL_CAPI_H__

#include "../SSkinPool-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SSkinPool Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ISkinPool C API Macros (IObjRef base interface methods) */
#define ISkinPool_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ISkinPool_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ISkinPool_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* ISkinPool C API Macros (ISkinPool specific methods) */
#define ISkinPool_GetSkin(This, strSkinName, nScale) \
    ((This)->lpVtbl->GetSkin(This, strSkinName, nScale))

#define ISkinPool_LoadSkins(This, xmlNode) \
    ((This)->lpVtbl->LoadSkins(This, xmlNode))

#define ISkinPool_AddSkin(This, skin) \
    ((This)->lpVtbl->AddSkin(This, skin))

#define ISkinPool_RemoveSkin(This, skin) \
    ((This)->lpVtbl->RemoveSkin(This, skin))

#define ISkinPool_RemoveAll(This) \
    ((This)->lpVtbl->RemoveAll(This))

#ifdef __cplusplus
}
#endif

#endif /* __SSKINPOOL_CAPI_H__ */
