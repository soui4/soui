#ifndef __SREALWNDHANDLER_CAPI_H__
#define __SREALWNDHANDLER_CAPI_H__

#include "../SRealWndHandler-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SRealWndHandler Interface
 * These macros provide C-style function call syntax for C++ interface methods
 * IRealWndHandler inherits from IObjRef.
 */

/* IObjRef base interface macros */
#define IRealWndHandler_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IRealWndHandler_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IRealWndHandler_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IRealWndHandler specific interface macros */
#define IRealWndHandler_OnRealWndCreate(This, pRealWnd) \
    ((This)->lpVtbl->OnRealWndCreate(This, pRealWnd))

#define IRealWndHandler_OnRealWndDestroy(This, pRealWnd) \
    ((This)->lpVtbl->OnRealWndDestroy(This, pRealWnd))

#define IRealWndHandler_OnRealWndInit(This, pRealWnd) \
    ((This)->lpVtbl->OnRealWndInit(This, pRealWnd))

#define IRealWndHandler_OnRealWndPosition(This, pRealWnd, rcWnd) \
    ((This)->lpVtbl->OnRealWndPosition(This, pRealWnd, rcWnd))

#ifdef __cplusplus
}
#endif

#endif /* __SREALWNDHANDLER_CAPI_H__ */
