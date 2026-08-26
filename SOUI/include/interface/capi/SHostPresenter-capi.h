#ifndef __SHOSTPRESENTER_CAPI_H__
#define __SHOSTPRESENTER_CAPI_H__

#include "../SHostPresenter-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for IHostPresenter Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IObjRef base interface macros */
#define IHostPresenter_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IHostPresenter_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IHostPresenter_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IHostPresenter specific interface macros */
#define IHostPresenter_OnHostCreate(This) \
    ((This)->lpVtbl->OnHostCreate(This))

#define IHostPresenter_OnHostDestroy(This) \
    ((This)->lpVtbl->OnHostDestroy(This))

#define IHostPresenter_OnHostResize(This, szHost) \
    ((This)->lpVtbl->OnHostResize(This, szHost))

#define IHostPresenter_OnHostPresent(This, hdc, pMemRT, rcDirty, byAlpha) \
    ((This)->lpVtbl->OnHostPresent(This, hdc, pMemRT, rcDirty, byAlpha))

#ifdef __cplusplus
}
#endif

#endif /* __SHOSTPRESENTER_CAPI_H__ */
