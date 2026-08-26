#ifndef __SFACTORY_CAPI_H__
#define __SFACTORY_CAPI_H__

#include "../SFactory-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SFactory Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ISouiFactory C API Macros */
#define ISouiFactory_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ISouiFactory_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ISouiFactory_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ISouiFactory_CreateApp(This, pRenderFac, hInst, pszHostClassName, bImeApp) \
    ((This)->lpVtbl->CreateApp(This, pRenderFac, hInst, pszHostClassName, bImeApp))

#define ISouiFactory_CreateNativeWnd(This) \
    ((This)->lpVtbl->CreateNativeWnd(This))

#define ISouiFactory_CreateHostWnd(This, pszResID) \
    ((This)->lpVtbl->CreateHostWnd(This, pszResID))

#define ISouiFactory_CreateHostDialog(This, pszResID) \
    ((This)->lpVtbl->CreateHostDialog(This, pszResID))

#define ISouiFactory_CreateStringA(This, pszSrc) \
    ((This)->lpVtbl->CreateStringA(This, pszSrc))

#define ISouiFactory_CreateStringW(This, pszSrc) \
    ((This)->lpVtbl->CreateStringW(This, pszSrc))

#define ISouiFactory_CreateXmlDoc(This) \
    ((This)->lpVtbl->CreateXmlDoc(This))

#define ISouiFactory_CreateResProvider(This, resType) \
    ((This)->lpVtbl->CreateResProvider(This, resType))

#define ISouiFactory_CreateFuncSlot(This, fun, ctx) \
    ((This)->lpVtbl->CreateFuncSlot(This, fun, ctx))

#define ISouiFactory_CreateMenu(This, hMenu) \
    ((This)->lpVtbl->CreateMenu(This, hMenu))

#define ISouiFactory_CreateMenuEx(This) \
    ((This)->lpVtbl->CreateMenuEx(This))

#define ISouiFactory_CreateTimer(This, pEvtSlot) \
    ((This)->lpVtbl->CreateTimer(This, pEvtSlot))

#define ISouiFactory_CreateAnimatorGroup(This) \
    ((This)->lpVtbl->CreateAnimatorGroup(This))

#ifdef __cplusplus
}
#endif

#endif /* __SFACTORY_CAPI_H__ */
