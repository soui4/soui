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

#ifdef __cplusplus
}
#endif

#endif /* __SIMGDECODER_CAPI_H__ */
