#ifndef __SLOG_CAPI_H__
#define __SLOG_CAPI_H__

#include "../slog-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IObjRef base interface macros */
#define IOutputFileBuilder_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IOutputFileBuilder_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IOutputFileBuilder_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IOutputFileBuilder specific interface macros */
#define IOutputFileBuilder_CreateFile(This, pszFileName, lMaxFileSize) \
    ((This)->lpVtbl->CreateFile(This, pszFileName, lMaxFileSize))

#ifdef __cplusplus
}
#endif

#endif /* __SLOG_CAPI_H__ */