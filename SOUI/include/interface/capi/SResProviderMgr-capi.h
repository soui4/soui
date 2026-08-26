#ifndef __SRESPROVIDERMGR_CAPI_H__
#define __SRESPROVIDERMGR_CAPI_H__

#include "../SResProviderMgr-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SResProviderMgr Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IResProviderMgr C API Macros */
#define IResProviderMgr_AddResProvider(This, pResProvider, pszUidef) \
    ((This)->lpVtbl->AddResProvider(This, pResProvider, pszUidef))

#define IResProviderMgr_AddResProviderA(This, pResProvider, pszUidef) \
    ((This)->lpVtbl->AddResProviderA(This, pResProvider, pszUidef))

#define IResProviderMgr_RemoveResProvider(This, pResProvider) \
    ((This)->lpVtbl->RemoveResProvider(This, pResProvider))

#define IResProviderMgr_RemoveAll(This) \
    ((This)->lpVtbl->RemoveAll(This))

#define IResProviderMgr_GetHeadResProvider(This) \
    ((This)->lpVtbl->GetHeadResProvider(This))

#define IResProviderMgr_GetTailResProvider(This) \
    ((This)->lpVtbl->GetTailResProvider(This))

#define IResProviderMgr_SetFilePrefix(This, pszFilePrefix) \
    ((This)->lpVtbl->SetFilePrefix(This, pszFilePrefix))

#define IResProviderMgr_GetFilePrefix(This, pszFilePrefix, cchMax) \
    ((This)->lpVtbl->GetFilePrefix(This, pszFilePrefix, cchMax))

#define IResProviderMgr_HasResource(This, pszType, pszResName) \
    ((This)->lpVtbl->HasResource(This, pszType, pszResName))

#define IResProviderMgr_LoadIcon(This, pszResName, cx, cy, bFromFile) \
    ((This)->lpVtbl->LoadIcon(This, pszResName, cx, cy, bFromFile))

#define IResProviderMgr_LoadCursor(This, pszResName, bFromFile) \
    ((This)->lpVtbl->LoadCursor(This, pszResName, bFromFile))

#define IResProviderMgr_LoadBitmap(This, pszResName, bFromFile) \
    ((This)->lpVtbl->LoadBitmap(This, pszResName, bFromFile))

#define IResProviderMgr_LoadImage(This, pszType, pszResName) \
    ((This)->lpVtbl->LoadImage(This, pszType, pszResName))

#define IResProviderMgr_LoadImgX(This, pszType, pszResName) \
    ((This)->lpVtbl->LoadImgX(This, pszType, pszResName))

#define IResProviderMgr_GetRawBufferSize(This, pszType, pszResName) \
    ((This)->lpVtbl->GetRawBufferSize(This, pszType, pszResName))

#define IResProviderMgr_GetRawBuffer(This, pszType, pszResName, pBuf, size) \
    ((This)->lpVtbl->GetRawBuffer(This, pszType, pszResName, pBuf, size))

#ifdef __cplusplus
}
#endif

#endif /* __SRESPROVIDERMGR_CAPI_H__ */
