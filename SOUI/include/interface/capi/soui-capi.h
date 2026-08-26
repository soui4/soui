#ifndef __SOUI_CAPI_H__
#define __SOUI_CAPI_H__

/*
 * SOUI C API Helper Macros
 *
 * This header includes all individual C API headers for SOUI interfaces.
 * It enables C code to call SOUI interface methods using familiar C syntax.
 *
 * Usage:
 *   1. Include this header in your C code
 *   2. Use the provided macros instead of direct vtable calls
 *   3. All macros follow the pattern: InterfaceName_MethodName(This, ...)
 *
 * Example:
 *   ITimer* timer = GetTimer();
 *   ITimer_StartTimer(timer, 1000, TRUE, 0);
 *   ITimer_KillTimer(timer);
 *   ITimer_Release(timer);
 */

/* Include all individual C API headers
 * Note: includes are outside extern "C" because interface headers (-i.h) may
 * contain C++ constructs (templates, namespaces). The macro definitions in each
 * capi header are preprocessor-only and do not require C linkage. */
#include "SAccelerator-capi.h"
#include "SAdapter-capi.h"
#include "SAnimation-capi.h"
#include "SAttrAlias-capi.h"
#include "SAttrStorage-capi.h"
#include "SCtrl-capi.h"
#include "SEvtArgs-capi.h"
#include "SFactory-capi.h"
#include "SGradient-capi.h"
#include "SHttpClient-capi.h"
#include "SImgDecoder-capi.h"
#include "SListViewItemLocator-capi.h"
#include "SLog-capi.h"
#include "SMatrix-capi.h"
#include "SMenu-capi.h"
#include "SMenuEx-capi.h"
#include "SMessageBox-capi.h"
#include "SMsgLoop-capi.h"
#include "SNativeWnd-capi.h"
#include "SNcPainter-capi.h"
#include "SNotifyCenter-capi.h"
#include "SObjFactory-capi.h"
#include "SPathEffect-capi.h"
#include "SPropertyValuesHolder-capi.h"
#include "SRealWndHandler-capi.h"
#include "SRender-capi.h"
#include "SResProvider-capi.h"
#include "SResProviderMgr-capi.h"
#include "SRunnable-capi.h"
#include "SScriptModule-capi.h"
#include "SSkinPool-capi.h"
#include "SSkinobj-capi.h"
#include "STaskLoop-capi.h"
#include "STileViewItemLocator-capi.h"
#include "STimelineHandler-capi.h"
#include "STimelineHandlersMgr-capi.h"
#include "STimer-capi.h"
#include "STransform-capi.h"
#include "STranslator-capi.h"
#include "STreeViewItemLocator-capi.h"
#include "SValueAnimator-capi.h"
#include "SWindow-capi.h"
#include "SWndContainer-capi.h"
#include "SHostPresenter-capi.h"
#include "sacchelper-capi.h"
#include "saccproxy-capi.h"
#include "sapp-capi.h"
#include "scaret-capi.h"
#include "shostwnd-capi.h"
#include "sinterpolator-capi.h"
#include "sipcobj-capi.h"
#include "stooltip-capi.h"

/*
 * Common reference counting macros for IObjRef-derived interfaces
 */
#define IObjRef_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IObjRef_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IObjRef_SafeAddRef(This) \
    ((This) ? IObjRef_AddRef(This) : 0)

#define IObjRef_SafeRelease(This) \
    do { \
        if (This) { \
            IObjRef_Release(This); \
            (This) = NULL; \
        } \
    } while(0)

#define IObjRef_IsValidInterface(This) \
    ((This) != NULL && (This)->lpVtbl != NULL)

/*
 * HRESULT helpers
 */
#ifndef SOUI_SUCCEEDED
#define SOUI_SUCCEEDED(hr) \
    ((HRESULT)(hr) >= 0)
#endif

#ifndef SOUI_FAILED
#define SOUI_FAILED(hr) \
    ((HRESULT)(hr) < 0)
#endif

#endif /* __SOUI_CAPI_H__ */
