#ifndef __STOOLTIP_CAPI_H__
#define __STOOLTIP_CAPI_H__

#include "../stooltip-i.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
 * C API Helper Macros for IToolTip Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IObjRef base interface macros */
#define IToolTip_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IToolTip_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IToolTip_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IToolTip specific interface macros */
#define IToolTip_Create(This) \
    ((This)->lpVtbl->Create(This))

#define IToolTip_Destroy(This) \
    ((This)->lpVtbl->Destroy(This))

#define IToolTip_GetMsgFilter(This) \
    ((This)->lpVtbl->GetMsgFilter(This))

#define IToolTip_PreTranslateMessage(This, pMsg) \
    ((This)->lpVtbl->PreTranslateMessage(This, pMsg))

#define IToolTip_UpdateTip(This, id, rc, pszTip, nScale) \
    ((This)->lpVtbl->UpdateTip(This, id, rc, pszTip, nScale))

#define IToolTip_SetToolTip(This, rc, tipAlign, pszTip, nScale) \
    ((This)->lpVtbl->SetToolTip(This, rc, tipAlign, pszTip, nScale))

#define IToolTip_ClearTip(This) \
    ((This)->lpVtbl->ClearTip(This))

#define IToolTip_RelayEvent(This, pMsg) \
    ((This)->lpVtbl->RelayEvent(This, pMsg))

/* IToolTipFactory interface macros */
#define IToolTipFactory_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IToolTipFactory_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IToolTipFactory_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IToolTipFactory_CreateToolTip(This, hHost) \
    ((This)->lpVtbl->CreateToolTip(This, hHost))

#define IToolTipFactory_DestroyToolTip(This, pToolTip) \
    ((This)->lpVtbl->DestroyToolTip(This, pToolTip))

#ifdef __cplusplus
}
#endif

#endif /* __STOOLTIP_CAPI_H__ */