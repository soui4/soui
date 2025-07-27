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

#define IResProviderMgr_AddResProvider_Default(This, pResProvider) \
    ((This)->lpVtbl->AddResProvider(This, pResProvider, _T("uidef:xml_init")))

#define IResProviderMgr_AddResProviderA(This, pResProvider, pszUidef) \
    ((This)->lpVtbl->AddResProviderA(This, pResProvider, pszUidef))

#define IResProviderMgr_AddResProviderA_Default(This, pResProvider) \
    ((This)->lpVtbl->AddResProviderA(This, pResProvider, "uidef:xml_init"))

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

#define IResProviderMgr_HasResource(This, pszType, pszResName) \
    ((This)->lpVtbl->HasResource(This, pszType, pszResName))

#define IResProviderMgr_LoadIcon(This, pszResName, cx, cy, bFromFile) \
    ((This)->lpVtbl->LoadIcon(This, pszResName, cx, cy, bFromFile))

#define IResProviderMgr_LoadIcon_Default(This, pszResName) \
    ((This)->lpVtbl->LoadIcon(This, pszResName, 0, 0, FALSE))

#define IResProviderMgr_LoadCursor(This, pszResName, bFromFile) \
    ((This)->lpVtbl->LoadCursor(This, pszResName, bFromFile))

#define IResProviderMgr_LoadCursor_Default(This, pszResName) \
    ((This)->lpVtbl->LoadCursor(This, pszResName, FALSE))

#define IResProviderMgr_LoadBitmap(This, pszResName, bFromFile) \
    ((This)->lpVtbl->LoadBitmap(This, pszResName, bFromFile))

#define IResProviderMgr_LoadBitmap_Default(This, pszResName) \
    ((This)->lpVtbl->LoadBitmap(This, pszResName, FALSE))

#define IResProviderMgr_LoadImage(This, pszType, pszResName) \
    ((This)->lpVtbl->LoadImage(This, pszType, pszResName))

#define IResProviderMgr_LoadImgX(This, pszType, pszResName) \
    ((This)->lpVtbl->LoadImgX(This, pszType, pszResName))

#define IResProviderMgr_GetRawBufferSize(This, pszType, pszResName) \
    ((This)->lpVtbl->GetRawBufferSize(This, pszType, pszResName))

#define IResProviderMgr_GetRawBuffer(This, pszType, pszResName, pBuf, size) \
    ((This)->lpVtbl->GetRawBuffer(This, pszType, pszResName, pBuf, size))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IResProviderMgr Helper Functions */
static inline void IResProviderMgr_AddResProvider_C(IResProviderMgr* pThis, IResProvider* pResProvider, LPCTSTR pszUidef)
{
    IResProviderMgr_AddResProvider(pThis, pResProvider, pszUidef);
}

static inline void IResProviderMgr_AddResProviderA_C(IResProviderMgr* pThis, IResProvider* pResProvider, LPCSTR pszUidef)
{
    IResProviderMgr_AddResProviderA(pThis, pResProvider, pszUidef);
}

static inline void IResProviderMgr_RemoveResProvider_C(IResProviderMgr* pThis, IResProvider* pResProvider)
{
    IResProviderMgr_RemoveResProvider(pThis, pResProvider);
}

static inline void IResProviderMgr_RemoveAll_C(IResProviderMgr* pThis)
{
    IResProviderMgr_RemoveAll(pThis);
}

static inline IResProvider* IResProviderMgr_GetHeadResProvider_C(IResProviderMgr* pThis)
{
    return IResProviderMgr_GetHeadResProvider(pThis);
}

static inline IResProvider* IResProviderMgr_GetTailResProvider_C(IResProviderMgr* pThis)
{
    return IResProviderMgr_GetTailResProvider(pThis);
}

static inline void IResProviderMgr_SetFilePrefix_C(IResProviderMgr* pThis, LPCTSTR pszFilePrefix)
{
    IResProviderMgr_SetFilePrefix(pThis, pszFilePrefix);
}

static inline BOOL IResProviderMgr_HasResource_C(IResProviderMgr* pThis, LPCTSTR pszType, LPCTSTR pszResName)
{
    return IResProviderMgr_HasResource(pThis, pszType, pszResName);
}

static inline HICON IResProviderMgr_LoadIcon_C(IResProviderMgr* pThis, LPCTSTR pszResName, int cx, int cy, BOOL bFromFile)
{
    return IResProviderMgr_LoadIcon(pThis, pszResName, cx, cy, bFromFile);
}

static inline HCURSOR IResProviderMgr_LoadCursor_C(IResProviderMgr* pThis, LPCTSTR pszResName, BOOL bFromFile)
{
    return IResProviderMgr_LoadCursor(pThis, pszResName, bFromFile);
}

static inline HBITMAP IResProviderMgr_LoadBitmap_C(IResProviderMgr* pThis, LPCTSTR pszResName, BOOL bFromFile)
{
    return IResProviderMgr_LoadBitmap(pThis, pszResName, bFromFile);
}

static inline IBitmapS* IResProviderMgr_LoadImage_C(IResProviderMgr* pThis, LPCTSTR pszType, LPCTSTR pszResName)
{
    return IResProviderMgr_LoadImage(pThis, pszType, pszResName);
}

static inline IImgX* IResProviderMgr_LoadImgX_C(IResProviderMgr* pThis, LPCTSTR pszType, LPCTSTR pszResName)
{
    return IResProviderMgr_LoadImgX(pThis, pszType, pszResName);
}

static inline size_t IResProviderMgr_GetRawBufferSize_C(IResProviderMgr* pThis, LPCTSTR pszType, LPCTSTR pszResName)
{
    return IResProviderMgr_GetRawBufferSize(pThis, pszType, pszResName);
}

static inline BOOL IResProviderMgr_GetRawBuffer_C(IResProviderMgr* pThis, LPCTSTR pszType, LPCTSTR pszResName, LPVOID pBuf, size_t size)
{
    return IResProviderMgr_GetRawBuffer(pThis, pszType, pszResName, pBuf, size);
}

/*
 * Convenience macros for common resource manager operations
 */

/* Provider management shortcuts */
#define IResProviderMgr_AddProvider(This, provider) \
    IResProviderMgr_AddResProvider_Default(This, provider)

#define IResProviderMgr_AddProviderA(This, provider) \
    IResProviderMgr_AddResProviderA_Default(This, provider)

#define IResProviderMgr_RemoveProvider(This, provider) \
    IResProviderMgr_RemoveResProvider(This, provider)

#define IResProviderMgr_ClearAll(This) \
    IResProviderMgr_RemoveAll(This)

/* Resource loading shortcuts */
#define IResProviderMgr_LoadIcon16(This, name) \
    IResProviderMgr_LoadIcon(This, name, 16, 16, FALSE)

#define IResProviderMgr_LoadIcon32(This, name) \
    IResProviderMgr_LoadIcon(This, name, 32, 32, FALSE)

#define IResProviderMgr_LoadIcon48(This, name) \
    IResProviderMgr_LoadIcon(This, name, 48, 48, FALSE)

#define IResProviderMgr_LoadIconFromFile(This, name, cx, cy) \
    IResProviderMgr_LoadIcon(This, name, cx, cy, TRUE)

#define IResProviderMgr_LoadCursorFromFile(This, name) \
    IResProviderMgr_LoadCursor(This, name, TRUE)

#define IResProviderMgr_LoadBitmapFromFile(This, name) \
    IResProviderMgr_LoadBitmap(This, name, TRUE)

/* Image type shortcuts */
#define IResProviderMgr_LoadPNG(This, name) \
    IResProviderMgr_LoadImage(This, _T("PNG"), name)

#define IResProviderMgr_LoadJPG(This, name) \
    IResProviderMgr_LoadImage(This, _T("JPG"), name)

#define IResProviderMgr_LoadBMP(This, name) \
    IResProviderMgr_LoadImage(This, _T("BMP"), name)

#define IResProviderMgr_LoadGIF(This, name) \
    IResProviderMgr_LoadImage(This, _T("GIF"), name)

#define IResProviderMgr_LoadPNGX(This, name) \
    IResProviderMgr_LoadImgX(This, _T("PNG"), name)

#define IResProviderMgr_LoadJPGX(This, name) \
    IResProviderMgr_LoadImgX(This, _T("JPG"), name)

/* Resource existence checks */
#define IResProviderMgr_HasIcon(This, name) \
    IResProviderMgr_HasResource(This, _T("ICON"), name)

#define IResProviderMgr_HasBitmap(This, name) \
    IResProviderMgr_HasResource(This, _T("BITMAP"), name)

#define IResProviderMgr_HasCursor(This, name) \
    IResProviderMgr_HasResource(This, _T("CURSOR"), name)

#define IResProviderMgr_HasPNG(This, name) \
    IResProviderMgr_HasResource(This, _T("PNG"), name)

#define IResProviderMgr_HasJPG(This, name) \
    IResProviderMgr_HasResource(This, _T("JPG"), name)

/* Safe resource loading with error checking */
#define IResProviderMgr_SafeLoadImage(This, pszType, pszResName, ppImage) \
    do { \
        if ((This) && (ppImage)) { \
            if (IResProviderMgr_HasResource(This, pszType, pszResName)) { \
                *(ppImage) = IResProviderMgr_LoadImage(This, pszType, pszResName); \
            } else { \
                *(ppImage) = NULL; \
            } \
        } else if (ppImage) { \
            *(ppImage) = NULL; \
        } \
    } while(0)

#define IResProviderMgr_SafeLoadImgX(This, pszType, pszResName, ppImgX) \
    do { \
        if ((This) && (ppImgX)) { \
            if (IResProviderMgr_HasResource(This, pszType, pszResName)) { \
                *(ppImgX) = IResProviderMgr_LoadImgX(This, pszType, pszResName); \
            } else { \
                *(ppImgX) = NULL; \
            } \
        } else if (ppImgX) { \
            *(ppImgX) = NULL; \
        } \
    } while(0)

/* Raw buffer helpers */
static inline LPVOID IResProviderMgr_AllocAndGetRawBuffer_C(IResProviderMgr* pThis, LPCTSTR pszType, LPCTSTR pszResName, size_t* pSize)
{
    if (!pThis || !pSize) {
        if (pSize) *pSize = 0;
        return NULL;
    }
    
    size_t size = IResProviderMgr_GetRawBufferSize(pThis, pszType, pszResName);
    if (size == 0) {
        *pSize = 0;
        return NULL;
    }
    
    LPVOID pBuf = malloc(size);
    if (!pBuf) {
        *pSize = 0;
        return NULL;
    }
    
    if (IResProviderMgr_GetRawBuffer(pThis, pszType, pszResName, pBuf, size)) {
        *pSize = size;
        return pBuf;
    } else {
        free(pBuf);
        *pSize = 0;
        return NULL;
    }
}

#define IResProviderMgr_FreeRawBuffer(pBuf) \
    do { \
        if (pBuf) { \
            free(pBuf); \
            (pBuf) = NULL; \
        } \
    } while(0)

/*
 * Resource provider chain management
 */
static inline int IResProviderMgr_GetProviderCount_C(IResProviderMgr* pThis)
{
    if (!pThis) return 0;
    
    int count = 0;
    IResProvider* pProvider = IResProviderMgr_GetHeadResProvider(pThis);
    while (pProvider) {
        count++;
        // Note: This assumes providers are linked in some way
        // The actual implementation would depend on the internal structure
        break; // For now, just count head provider
    }
    return count;
}

static inline BOOL IResProviderMgr_HasAnyProvider_C(IResProviderMgr* pThis)
{
    return pThis && (IResProviderMgr_GetHeadResProvider(pThis) != NULL);
}

/*
 * Resource manager state helpers
 */
typedef struct ResProviderMgrState {
    IResProviderMgr* manager;
    LPCTSTR filePrefix;
    int providerCount;
    BOOL isInitialized;
} ResProviderMgrState;

static inline void ResProviderMgrState_Init(ResProviderMgrState* state, IResProviderMgr* manager)
{
    if (state) {
        state->manager = manager;
        state->filePrefix = NULL;
        state->providerCount = 0;
        state->isInitialized = (manager != NULL);
    }
}

static inline void ResProviderMgrState_SetFilePrefix(ResProviderMgrState* state, LPCTSTR prefix)
{
    if (state && state->manager) {
        IResProviderMgr_SetFilePrefix(state->manager, prefix);
        state->filePrefix = prefix;
    }
}

static inline BOOL ResProviderMgrState_AddProvider(ResProviderMgrState* state, IResProvider* provider)
{
    if (state && state->manager && provider) {
        IResProviderMgr_AddProvider(state->manager, provider);
        state->providerCount++;
        return TRUE;
    }
    return FALSE;
}

static inline BOOL ResProviderMgrState_RemoveProvider(ResProviderMgrState* state, IResProvider* provider)
{
    if (state && state->manager && provider) {
        IResProviderMgr_RemoveProvider(state->manager, provider);
        if (state->providerCount > 0) {
            state->providerCount--;
        }
        return TRUE;
    }
    return FALSE;
}

static inline void ResProviderMgrState_ClearAll(ResProviderMgrState* state)
{
    if (state && state->manager) {
        IResProviderMgr_ClearAll(state->manager);
        state->providerCount = 0;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* __SRESPROVIDERMGR_CAPI_H__ */
