#ifndef __SOUI_CAPI_H__
#define __SOUI_CAPI_H__

/*
 * SOUI C API Helper Macros
 * 
 * This header provides C-style function call macros for all SOUI C++ interfaces.
 * It enables C code to call SOUI interface methods using familiar C syntax.
 * 
 * Usage:
 *   1. Include this header in your C code
 *   2. Use the provided macros instead of direct vtable calls
 *   3. All macros follow the pattern: InterfaceName_MethodName(This, ...)
 * 
 * Example:
 *   ITimer* timer = GetTimer();
 *   ITimer_StartTimer(timer, 1000, TRUE, 0);  // Start 1-second repeating timer
 *   ITimer_KillTimer(timer);                  // Stop the timer
 *   ITimer_Release(timer);                    // Release reference
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Include all individual C API headers */
#include "SAccelerator-capi.h"
#include "STimer-capi.h"
#include "SRender-capi.h"
#include "SWindow-capi.h"
#include "SAdapter-capi.h"
#include "SAnimation-capi.h"
#include "SFactory-capi.h"
#include "SRunnable-capi.h"
#include "SResProvider-capi.h"
#include "SEvtArgs-capi.h"
#include "SWndContainer-capi.h"
#include "SHostPresenter-capi.h"
#include "SScriptModule-capi.h"
#include "SResProviderMgr-capi.h"
#include "SImgDecoder-capi.h"
#include "SMessageBox-capi.h"
#include "SMatrix-capi.h"
#include "STransform-capi.h"
#include "SValueAnimator-capi.h"
#include "SPathEffect-capi.h"
#include "SGradient-capi.h"
#include "SAttrStorage-capi.h"
#include "SCtrl-capi.h"
#include "SHttpClient-capi.h"
#include "SMsgLoop-capi.h"
#include "SAppCfg-capi.h"
#include "SNativeWnd-capi.h"
#include "SNotifyCenter-capi.h"
#include "SObjFactory-capi.h"
#include "SSkinPool-capi.h"
#include "SSkinobj-capi.h"
#include "STaskLoop-capi.h"
#include "STranslator-capi.h"
#include "SListViewItemLocator-capi.h"
#include "SNcPainter-capi.h"
#include "SRealWndHandler-capi.h"
#include "STileViewItemLocator-capi.h"
#include "STreeViewItemLocator-capi.h"
#include "STimelineHandler-capi.h"
#include "sacchelper-capi.h"
#include "saccproxy-capi.h"
#include "sapp-capi.h"
#include "scaret-capi.h"
#include "shostwnd-capi.h"
#include "sinterpolator-capi.h"
#include "sipcobj-capi.h"

/* All C API headers have been included - SOUI C API is now complete! */

/*
 * Common C API Macros for all IObjRef-derived interfaces
 * These macros work with any interface that inherits from IObjRef
 */

/* Generic reference counting macros */
#define IObjRef_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IObjRef_Release(This) \
    ((This)->lpVtbl->Release(This))


/*
 * Safe reference management macros
 * These macros include NULL pointer checks for safer usage
 */
#define IObjRef_SafeAddRef(This) \
    ((This) ? IObjRef_AddRef(This) : 0)

#define IObjRef_SafeRelease(This) \
    do { \
        if (This) { \
            IObjRef_Release(This); \
            (This) = NULL; \
        } \
    } while(0)

/*
 * Utility macros for interface casting and validation
 */
#define IObjRef_IsValidInterface(This) \
    ((This) != NULL && (This)->lpVtbl != NULL)

/*
 * Error handling macros
 */
#define SOUI_SUCCEEDED(hr) \
    ((HRESULT)(hr) >= 0)

#define SOUI_FAILED(hr) \
    ((HRESULT)(hr) < 0)

/*
 * Common return values
 */
#ifndef S_OK
#define S_OK                    ((HRESULT)0L)
#endif

#ifndef E_FAIL
#define E_FAIL                  ((HRESULT)0x80004005L)
#endif

#ifndef E_NOINTERFACE
#define E_NOINTERFACE           ((HRESULT)0x80004002L)
#endif

#ifndef E_INVALIDARG
#define E_INVALIDARG            ((HRESULT)0x80070057L)
#endif

#ifndef E_OUTOFMEMORY
#define E_OUTOFMEMORY           ((HRESULT)0x8007000EL)
#endif

/*
 * Convenience macros for common patterns
 */

/* Auto-release pattern */
#define SOUI_AUTO_RELEASE(var) \
    __attribute__((cleanup(SOUI_AutoReleaseCleanup))) var

/* Cleanup function for auto-release */
static inline void SOUI_AutoReleaseCleanup(void** ppInterface)
{
    if (ppInterface && *ppInterface) {
        IObjRef_Release((IUnknown*)*ppInterface);
        *ppInterface = NULL;
    }
}

/* Scoped reference pattern */
#define SOUI_SCOPED_REF(Type, var, ptr) \
    Type* var = ptr; \
    if (var) SOUI_AddRef(var); \
    __attribute__((cleanup(SOUI_ScopedRefCleanup_##Type))) Type** var##_cleanup = &var

/* Template for scoped reference cleanup functions */
#define SOUI_DEFINE_SCOPED_CLEANUP(Type) \
    static inline void SOUI_ScopedRefCleanup_##Type(Type*** ppInterface) { \
        if (ppInterface && *ppInterface && **ppInterface) { \
            IObjRef_Release((IUnknown*)**ppInterface); \
            **ppInterface = NULL; \
        } \
    }

#ifdef __cplusplus
}
#endif

#endif /* __SOUI_CAPI_H__ */
