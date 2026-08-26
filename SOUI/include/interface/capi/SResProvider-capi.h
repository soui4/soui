#ifndef __SRESPROVIDER_CAPI_H__
#define __SRESPROVIDER_CAPI_H__

#include "../SResProvider-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SResProvider Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IResProvider C API Macros */
#define IResProvider_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IResProvider_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IResProvider_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IResProvider_Init(This, wParam, lParam) \
    ((This)->lpVtbl->Init(This, wParam, lParam))

#define IResProvider_HasResource(This, pszType, pszResName) \
    ((This)->lpVtbl->HasResource(This, pszType, pszResName))

#define IResProvider_LoadIcon(This, pszResName, cx, cy) \
    ((This)->lpVtbl->LoadIcon(This, pszResName, cx, cy))

#define IResProvider_LoadBitmap(This, pszResName) \
    ((This)->lpVtbl->LoadBitmap(This, pszResName))

#define IResProvider_LoadCursor(This, pszResName) \
    ((This)->lpVtbl->LoadCursor(This, pszResName))

#define IResProvider_LoadImage(This, pszType, pszResName) \
    ((This)->lpVtbl->LoadImage(This, pszType, pszResName))

#define IResProvider_LoadImgX(This, pszType, pszResName) \
    ((This)->lpVtbl->LoadImgX(This, pszType, pszResName))

#define IResProvider_GetRawBufferSize(This, pszType, pszResName) \
    ((This)->lpVtbl->GetRawBufferSize(This, pszType, pszResName))

#define IResProvider_GetRawBuffer(This, pszType, pszResName, pBuf, size) \
    ((This)->lpVtbl->GetRawBuffer(This, pszType, pszResName, pBuf, size))

#define IResProvider_EnumResource(This, funEnumCB, lp) \
    ((This)->lpVtbl->EnumResource(This, funEnumCB, lp))

#define IResProvider_EnumFile(This, funEnumCB, lp) \
    ((This)->lpVtbl->EnumFile(This, funEnumCB, lp))

#ifdef __cplusplus
}
#endif

#endif /* __SRESPROVIDER_CAPI_H__ */
