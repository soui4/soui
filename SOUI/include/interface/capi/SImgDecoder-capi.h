#ifndef __SIMGDECODER_CAPI_H__
#define __SIMGDECODER_CAPI_H__

#include "../SImgDecoder-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SImgDecoder Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IImgFrame C API Macros */
#define IImgFrame_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IImgFrame_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IImgFrame_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IImgFrame_GetSize(This, pWid, pHei) \
    ((This)->lpVtbl->GetSize(This, pWid, pHei))

#define IImgFrame_GetPixels(This) \
    ((This)->lpVtbl->GetPixels(This))

#define IImgFrame_GetDelay(This) \
    ((This)->lpVtbl->GetDelay(This))

/* IImgX C API Macros */
#define IImgX_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IImgX_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IImgX_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IImgX_LoadFromMemory(This, pBuf, bufLen) \
    ((This)->lpVtbl->LoadFromMemory(This, pBuf, bufLen))

#define IImgX_LoadFromFileW(This, pszFileName) \
    ((This)->lpVtbl->LoadFromFileW(This, pszFileName))

#define IImgX_LoadFromFileA(This, pszFileName) \
    ((This)->lpVtbl->LoadFromFileA(This, pszFileName))

#define IImgX_GetFrameCount(This) \
    ((This)->lpVtbl->GetFrameCount(This))

#define IImgX_GetFrame(This, iFrame) \
    ((This)->lpVtbl->GetFrame(This, iFrame))

/* IImgDecoderFactory C API Macros */
#define IImgDecoderFactory_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IImgDecoderFactory_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IImgDecoderFactory_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IImgDecoderFactory_CreateImgX(This, ppImgDecoder) \
    ((This)->lpVtbl->CreateImgX(This, ppImgDecoder))

#define IImgDecoderFactory_SaveImage(This, pBits, nWid, nHei, pszFileName, pFormat) \
    ((This)->lpVtbl->SaveImage(This, pBits, nWid, nHei, pszFileName, pFormat))

#define IImgDecoderFactory_SaveImage2(This, pBits, nWid, nHei, pszFileName, imgFmt) \
    ((This)->lpVtbl->SaveImage2(This, pBits, nWid, nHei, pszFileName, imgFmt))

#define IImgDecoderFactory_GetDescription(This) \
    ((This)->lpVtbl->GetDescription(This))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IImgFrame Helper Functions */
static inline long IImgFrame_AddRef_C(IImgFrame* pThis)
{
    return IImgFrame_AddRef(pThis);
}

static inline long IImgFrame_Release_C(IImgFrame* pThis)
{
    return IImgFrame_Release(pThis);
}

static inline BOOL IImgFrame_GetSize_C(IImgFrame* pThis, UINT* pWid, UINT* pHei)
{
    return IImgFrame_GetSize(pThis, pWid, pHei);
}

static inline const VOID* IImgFrame_GetPixels_C(IImgFrame* pThis)
{
    return IImgFrame_GetPixels(pThis);
}

static inline int IImgFrame_GetDelay_C(IImgFrame* pThis)
{
    return IImgFrame_GetDelay(pThis);
}

/* IImgX Helper Functions */
static inline long IImgX_AddRef_C(IImgX* pThis)
{
    return IImgX_AddRef(pThis);
}

static inline long IImgX_Release_C(IImgX* pThis)
{
    return IImgX_Release(pThis);
}

static inline int IImgX_LoadFromMemory_C(IImgX* pThis, void* pBuf, size_t bufLen)
{
    return IImgX_LoadFromMemory(pThis, pBuf, bufLen);
}

static inline int IImgX_LoadFromFileW_C(IImgX* pThis, LPCWSTR pszFileName)
{
    return IImgX_LoadFromFileW(pThis, pszFileName);
}

static inline int IImgX_LoadFromFileA_C(IImgX* pThis, LPCSTR pszFileName)
{
    return IImgX_LoadFromFileA(pThis, pszFileName);
}

static inline UINT IImgX_GetFrameCount_C(IImgX* pThis)
{
    return IImgX_GetFrameCount(pThis);
}

static inline IImgFrame* IImgX_GetFrame_C(IImgX* pThis, UINT iFrame)
{
    return IImgX_GetFrame(pThis, iFrame);
}

/* IImgDecoderFactory Helper Functions */
static inline long IImgDecoderFactory_AddRef_C(IImgDecoderFactory* pThis)
{
    return IImgDecoderFactory_AddRef(pThis);
}

static inline long IImgDecoderFactory_Release_C(IImgDecoderFactory* pThis)
{
    return IImgDecoderFactory_Release(pThis);
}

static inline BOOL IImgDecoderFactory_CreateImgX_C(IImgDecoderFactory* pThis, IImgX** ppImgDecoder)
{
    return IImgDecoderFactory_CreateImgX(pThis, ppImgDecoder);
}

static inline HRESULT IImgDecoderFactory_SaveImage_C(IImgDecoderFactory* pThis, BYTE* pBits, int nWid, int nHei, LPCWSTR pszFileName, const void* pFormat)
{
    return IImgDecoderFactory_SaveImage(pThis, pBits, nWid, nHei, pszFileName, pFormat);
}

static inline HRESULT IImgDecoderFactory_SaveImage2_C(IImgDecoderFactory* pThis, BYTE* pBits, int nWid, int nHei, LPCWSTR pszFileName, ImgFmt imgFmt)
{
    return IImgDecoderFactory_SaveImage2(pThis, pBits, nWid, nHei, pszFileName, imgFmt);
}

static inline LPCWSTR IImgDecoderFactory_GetDescription_C(IImgDecoderFactory* pThis)
{
    return IImgDecoderFactory_GetDescription(pThis);
}

/*
 * Convenience macros for common image operations
 */

/* Unicode/ANSI file loading shortcuts */
#ifdef UNICODE
#define IImgX_LoadFromFile(This, fileName) \
    IImgX_LoadFromFileW(This, fileName)
#else
#define IImgX_LoadFromFile(This, fileName) \
    IImgX_LoadFromFileA(This, fileName)
#endif

/* Frame access shortcuts */
#define IImgX_GetFirstFrame(This) \
    IImgX_GetFrame(This, 0)

#define IImgX_GetLastFrame(This) \
    IImgX_GetFrame(This, IImgX_GetFrameCount(This) - 1)

#define IImgX_IsAnimated(This) \
    (IImgX_GetFrameCount(This) > 1)

#define IImgX_IsSingleFrame(This) \
    (IImgX_GetFrameCount(This) == 1)

/* Frame size helpers */
#define IImgFrame_GetWidth(This, pWidth) \
    IImgFrame_GetSize(This, pWidth, NULL)

#define IImgFrame_GetHeight(This, pHeight) \
    IImgFrame_GetSize(This, NULL, pHeight)

/* Safe operations */
#define IImgX_SafeLoadFromMemory(This, pBuf, bufLen) \
    (IObjRef_IsValidInterface(This) && (pBuf) && (bufLen) > 0 ? \
     IImgX_LoadFromMemory(This, pBuf, bufLen) : -1)

#define IImgX_SafeLoadFromFile(This, fileName) \
    (IObjRef_IsValidInterface(This) && (fileName) ? \
     IImgX_LoadFromFile(This, fileName) : -1)

#define IImgX_SafeGetFrame(This, iFrame, ppFrame) \
    do { \
        if (IObjRef_IsValidInterface(This) && (ppFrame) && \
            (iFrame) < IImgX_GetFrameCount(This)) { \
            *(ppFrame) = IImgX_GetFrame(This, iFrame); \
        } else if (ppFrame) { \
            *(ppFrame) = NULL; \
        } \
    } while(0)

/* Image format shortcuts for SaveImage2 */
#define IImgDecoderFactory_SavePNG(This, pBits, nWid, nHei, fileName) \
    IImgDecoderFactory_SaveImage2(This, pBits, nWid, nHei, fileName, Img_PNG)

#define IImgDecoderFactory_SaveJPG(This, pBits, nWid, nHei, fileName) \
    IImgDecoderFactory_SaveImage2(This, pBits, nWid, nHei, fileName, Img_JPG)

#define IImgDecoderFactory_SaveBMP(This, pBits, nWid, nHei, fileName) \
    IImgDecoderFactory_SaveImage2(This, pBits, nWid, nHei, fileName, Img_BMP)

#define IImgDecoderFactory_SaveTIFF(This, pBits, nWid, nHei, fileName) \
    IImgDecoderFactory_SaveImage2(This, pBits, nWid, nHei, fileName, Img_TIFF)

#define IImgDecoderFactory_SaveWebP(This, pBits, nWid, nHei, fileName) \
    IImgDecoderFactory_SaveImage2(This, pBits, nWid, nHei, fileName, Img_Webp)

#define IImgDecoderFactory_SaveTGA(This, pBits, nWid, nHei, fileName) \
    IImgDecoderFactory_SaveImage2(This, pBits, nWid, nHei, fileName, Img_TGA)

/* Safe factory operations */
#define IImgDecoderFactory_SafeCreateImgX(This, ppImgX) \
    do { \
        if (IObjRef_IsValidInterface(This) && (ppImgX)) { \
            if (!IImgDecoderFactory_CreateImgX(This, ppImgX)) { \
                *(ppImgX) = NULL; \
            } \
        } else if (ppImgX) { \
            *(ppImgX) = NULL; \
        } \
    } while(0)

#define IImgDecoderFactory_SafeSaveImage(This, pBits, nWid, nHei, fileName, fmt) \
    (IObjRef_IsValidInterface(This) && (pBits) && (nWid) > 0 && (nHei) > 0 && (fileName) ? \
     IImgDecoderFactory_SaveImage2(This, pBits, nWid, nHei, fileName, fmt) : E_INVALIDARG)

/*
 * Image loading helper functions
 */
static inline IImgX* IImgDecoderFactory_LoadImageFromFile_C(IImgDecoderFactory* pThis, LPCWSTR fileName)
{
    if (!IObjRef_IsValidInterface(pThis) || !fileName) {
        return NULL;
    }
    
    IImgX* pImgX = NULL;
    if (IImgDecoderFactory_CreateImgX(pThis, &pImgX) && pImgX) {
        if (IImgX_LoadFromFileW(pImgX, fileName) == 0) {
            return pImgX;
        } else {
            IImgX_Release(pImgX);
            return NULL;
        }
    }
    return NULL;
}

static inline IImgX* IImgDecoderFactory_LoadImageFromMemory_C(IImgDecoderFactory* pThis, void* pBuf, size_t bufLen)
{
    if (!IObjRef_IsValidInterface(pThis) || !pBuf || bufLen == 0) {
        return NULL;
    }
    
    IImgX* pImgX = NULL;
    if (IImgDecoderFactory_CreateImgX(pThis, &pImgX) && pImgX) {
        if (IImgX_LoadFromMemory(pImgX, pBuf, bufLen) == 0) {
            return pImgX;
        } else {
            IImgX_Release(pImgX);
            return NULL;
        }
    }
    return NULL;
}

/*
 * Frame information helper functions
 */
static inline BOOL IImgFrame_GetDimensions_C(IImgFrame* pThis, UINT* pWidth, UINT* pHeight)
{
    if (!IObjRef_IsValidInterface(pThis)) {
        if (pWidth) *pWidth = 0;
        if (pHeight) *pHeight = 0;
        return FALSE;
    }
    return IImgFrame_GetSize(pThis, pWidth, pHeight);
}

static inline BOOL IImgFrame_IsValidDelay_C(IImgFrame* pThis)
{
    return IObjRef_IsValidInterface(pThis) && (IImgFrame_GetDelay(pThis) > 0);
}

/*
 * Reference counting helpers
 */
#define IImgFrame_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IImgFrame_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define IImgX_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IImgX_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define IImgDecoderFactory_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IImgDecoderFactory_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Image format string helpers
 */
static inline LPCWSTR ImgFmt_ToString(ImgFmt fmt)
{
    switch (fmt) {
        case Img_PNG:  return L"PNG";
        case Img_BMP:  return L"BMP";
        case Img_TIFF: return L"TIFF";
        case Img_JPG:  return L"JPG";
        case Img_Webp: return L"WebP";
        case Img_TGA:  return L"TGA";
        default:       return L"Unknown";
    }
}

static inline ImgFmt ImgFmt_FromExtension(LPCWSTR ext)
{
    if (!ext) return Img_PNG; // Default
    
    if (_wcsicmp(ext, L".png") == 0) return Img_PNG;
    if (_wcsicmp(ext, L".bmp") == 0) return Img_BMP;
    if (_wcsicmp(ext, L".tiff") == 0 || _wcsicmp(ext, L".tif") == 0) return Img_TIFF;
    if (_wcsicmp(ext, L".jpg") == 0 || _wcsicmp(ext, L".jpeg") == 0) return Img_JPG;
    if (_wcsicmp(ext, L".webp") == 0) return Img_Webp;
    if (_wcsicmp(ext, L".tga") == 0) return Img_TGA;
    
    return Img_PNG; // Default fallback
}

#ifdef __cplusplus
}
#endif

#endif /* __SIMGDECODER_CAPI_H__ */
