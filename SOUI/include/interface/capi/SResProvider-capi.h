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

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IResProvider Helper Functions */
static inline long IResProvider_AddRef_C(IResProvider* pThis)
{
    return IResProvider_AddRef(pThis);
}

static inline long IResProvider_Release_C(IResProvider* pThis)
{
    return IResProvider_Release(pThis);
}

static inline void IResProvider_OnFinalRelease_C(IResProvider* pThis)
{
    IResProvider_OnFinalRelease(pThis);
}

static inline BOOL IResProvider_Init_C(IResProvider* pThis, WPARAM wParam, LPARAM lParam)
{
    return IResProvider_Init(pThis, wParam, lParam);
}

static inline BOOL IResProvider_HasResource_C(IResProvider* pThis, LPCTSTR pszType, LPCTSTR pszResName)
{
    return IResProvider_HasResource(pThis, pszType, pszResName);
}

static inline HICON IResProvider_LoadIcon_C(IResProvider* pThis, LPCTSTR pszResName, int cx, int cy)
{
    return IResProvider_LoadIcon(pThis, pszResName, cx, cy);
}

static inline HBITMAP IResProvider_LoadBitmap_C(IResProvider* pThis, LPCTSTR pszResName)
{
    return IResProvider_LoadBitmap(pThis, pszResName);
}

static inline HCURSOR IResProvider_LoadCursor_C(IResProvider* pThis, LPCTSTR pszResName)
{
    return IResProvider_LoadCursor(pThis, pszResName);
}

static inline IBitmapS* IResProvider_LoadImage_C(IResProvider* pThis, LPCTSTR pszType, LPCTSTR pszResName)
{
    return IResProvider_LoadImage(pThis, pszType, pszResName);
}

static inline IImgX* IResProvider_LoadImgX_C(IResProvider* pThis, LPCTSTR pszType, LPCTSTR pszResName)
{
    return IResProvider_LoadImgX(pThis, pszType, pszResName);
}

static inline size_t IResProvider_GetRawBufferSize_C(IResProvider* pThis, LPCTSTR pszType, LPCTSTR pszResName)
{
    return IResProvider_GetRawBufferSize(pThis, pszType, pszResName);
}

static inline BOOL IResProvider_GetRawBuffer_C(IResProvider* pThis, LPCTSTR pszType, LPCTSTR pszResName, LPVOID pBuf, size_t size)
{
    return IResProvider_GetRawBuffer(pThis, pszType, pszResName, pBuf, size);
}

static inline void IResProvider_EnumResource_C(IResProvider* pThis, EnumResCallback funEnumCB, LPARAM lp)
{
    IResProvider_EnumResource(pThis, funEnumCB, lp);
}

static inline void IResProvider_EnumFile_C(IResProvider* pThis, EnumFileCallback funEnumCB, LPARAM lp)
{
    IResProvider_EnumFile(pThis, funEnumCB, lp);
}

/*
 * Convenience macros for common resource operations
 */

/* Icon loading shortcuts */
#define IResProvider_LoadIcon16(This, pszResName) \
    IResProvider_LoadIcon(This, pszResName, 16, 16)

#define IResProvider_LoadIcon32(This, pszResName) \
    IResProvider_LoadIcon(This, pszResName, 32, 32)

#define IResProvider_LoadIcon48(This, pszResName) \
    IResProvider_LoadIcon(This, pszResName, 48, 48)

#define IResProvider_LoadIconDefault(This, pszResName) \
    IResProvider_LoadIcon(This, pszResName, 0, 0)

/* Image loading shortcuts */
#define IResProvider_LoadPNG(This, pszResName) \
    IResProvider_LoadImage(This, _T("PNG"), pszResName)

#define IResProvider_LoadJPG(This, pszResName) \
    IResProvider_LoadImage(This, _T("JPG"), pszResName)

#define IResProvider_LoadBMP(This, pszResName) \
    IResProvider_LoadImage(This, _T("BMP"), pszResName)

#define IResProvider_LoadGIF(This, pszResName) \
    IResProvider_LoadImage(This, _T("GIF"), pszResName)

#define IResProvider_LoadImageAuto(This, pszResName) \
    IResProvider_LoadImage(This, NULL, pszResName)

/* Resource existence checks */
#define IResProvider_HasIcon(This, pszResName) \
    IResProvider_HasResource(This, _T("ICON"), pszResName)

#define IResProvider_HasBitmap(This, pszResName) \
    IResProvider_HasResource(This, _T("BITMAP"), pszResName)

#define IResProvider_HasCursor(This, pszResName) \
    IResProvider_HasResource(This, _T("CURSOR"), pszResName)

#define IResProvider_HasPNG(This, pszResName) \
    IResProvider_HasResource(This, _T("PNG"), pszResName)

/* Safe resource loading with error checking */
#define IResProvider_SafeLoadImage(This, pszType, pszResName, ppImage) \
    do { \
        if (SOUI_IsValidInterface(This) && (ppImage)) { \
            if (IResProvider_HasResource(This, pszType, pszResName)) { \
                *(ppImage) = IResProvider_LoadImage(This, pszType, pszResName); \
            } else { \
                *(ppImage) = NULL; \
            } \
        } else if (ppImage) { \
            *(ppImage) = NULL; \
        } \
    } while(0)

/* Raw buffer helpers */
static inline LPVOID IResProvider_AllocAndGetRawBuffer_C(IResProvider* pThis, LPCTSTR pszType, LPCTSTR pszResName, size_t* pSize)
{
    if (!SOUI_IsValidInterface(pThis) || !pSize) {
        if (pSize) *pSize = 0;
        return NULL;
    }
    
    size_t size = IResProvider_GetRawBufferSize(pThis, pszType, pszResName);
    if (size == 0) {
        *pSize = 0;
        return NULL;
    }
    
    LPVOID pBuf = malloc(size);
    if (!pBuf) {
        *pSize = 0;
        return NULL;
    }
    
    if (IResProvider_GetRawBuffer(pThis, pszType, pszResName, pBuf, size)) {
        *pSize = size;
        return pBuf;
    } else {
        free(pBuf);
        *pSize = 0;
        return NULL;
    }
}

#define IResProvider_FreeRawBuffer(pBuf) \
    do { \
        if (pBuf) { \
            free(pBuf); \
            (pBuf) = NULL; \
        } \
    } while(0)

/*
 * Reference counting helpers
 */
#define IResProvider_SafeAddRef(This) \
    SOUI_SafeAddRef((IUnknown*)(This))

#define IResProvider_SafeRelease(This) \
    SOUI_SafeRelease((IUnknown**)(This))

#ifdef __cplusplus
}
#endif

#endif /* __SRESPROVIDER_CAPI_H__ */
