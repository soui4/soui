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

#define ISouiFactory_CreateApp_Simple(This, pRenderFac, hInst) \
    ((This)->lpVtbl->CreateApp(This, pRenderFac, hInst, _T("SOUIHOST"), FALSE))

#define ISouiFactory_CreateNativeWnd(This) \
    ((This)->lpVtbl->CreateNativeWnd(This))

#define ISouiFactory_CreateHostWnd(This, pszResID) \
    ((This)->lpVtbl->CreateHostWnd(This, pszResID))

#define ISouiFactory_CreateHostWnd_Default(This) \
    ((This)->lpVtbl->CreateHostWnd(This, NULL))

#define ISouiFactory_CreateHostDialog(This, pszResID) \
    ((This)->lpVtbl->CreateHostDialog(This, pszResID))

#define ISouiFactory_CreateHostDialog_Default(This) \
    ((This)->lpVtbl->CreateHostDialog(This, NULL))

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

#define ISouiFactory_CreateMenu_Default(This) \
    ((This)->lpVtbl->CreateMenu(This, 0))

#define ISouiFactory_CreateMenuEx(This) \
    ((This)->lpVtbl->CreateMenuEx(This))

#define ISouiFactory_CreateTimer(This, pEvtSlot) \
    ((This)->lpVtbl->CreateTimer(This, pEvtSlot))

#define ISouiFactory_CreateAnimatorGroup(This) \
    ((This)->lpVtbl->CreateAnimatorGroup(This))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* ISouiFactory Helper Functions */
static inline long ISouiFactory_AddRef_C(ISouiFactory* pThis)
{
    return ISouiFactory_AddRef(pThis);
}

static inline long ISouiFactory_Release_C(ISouiFactory* pThis)
{
    return ISouiFactory_Release(pThis);
}

static inline IApplication* ISouiFactory_CreateApp_C(ISouiFactory* pThis, IRenderFactory* pRenderFac, HMODULE hInst, LPCTSTR pszHostClassName, BOOL bImeApp)
{
    return ISouiFactory_CreateApp(pThis, pRenderFac, hInst, pszHostClassName, bImeApp);
}

static inline IApplication* ISouiFactory_CreateApp_Simple_C(ISouiFactory* pThis, IRenderFactory* pRenderFac, HMODULE hInst)
{
    return ISouiFactory_CreateApp_Simple(pThis, pRenderFac, hInst);
}

static inline INativeWnd* ISouiFactory_CreateNativeWnd_C(ISouiFactory* pThis)
{
    return ISouiFactory_CreateNativeWnd(pThis);
}

static inline IHostWnd* ISouiFactory_CreateHostWnd_C(ISouiFactory* pThis, LPCTSTR pszResID)
{
    return ISouiFactory_CreateHostWnd(pThis, pszResID);
}

static inline IHostWnd* ISouiFactory_CreateHostWnd_Default_C(ISouiFactory* pThis)
{
    return ISouiFactory_CreateHostWnd_Default(pThis);
}

static inline IHostDialog* ISouiFactory_CreateHostDialog_C(ISouiFactory* pThis, LPCTSTR pszResID)
{
    return ISouiFactory_CreateHostDialog(pThis, pszResID);
}

static inline IHostDialog* ISouiFactory_CreateHostDialog_Default_C(ISouiFactory* pThis)
{
    return ISouiFactory_CreateHostDialog_Default(pThis);
}

static inline IStringA* ISouiFactory_CreateStringA_C(ISouiFactory* pThis, LPCSTR pszSrc)
{
    return ISouiFactory_CreateStringA(pThis, pszSrc);
}

static inline IStringW* ISouiFactory_CreateStringW_C(ISouiFactory* pThis, LPCWSTR pszSrc)
{
    return ISouiFactory_CreateStringW(pThis, pszSrc);
}

static inline IXmlDoc* ISouiFactory_CreateXmlDoc_C(ISouiFactory* pThis)
{
    return ISouiFactory_CreateXmlDoc(pThis);
}

static inline IResProvider* ISouiFactory_CreateResProvider_C(ISouiFactory* pThis, BUILTIN_RESTYPE resType)
{
    return ISouiFactory_CreateResProvider(pThis, resType);
}

static inline IEvtSlot* ISouiFactory_CreateFuncSlot_C(ISouiFactory* pThis, FunCallback fun, void* ctx)
{
    return ISouiFactory_CreateFuncSlot(pThis, fun, ctx);
}

static inline IMenu* ISouiFactory_CreateMenu_C(ISouiFactory* pThis, HMENU hMenu)
{
    return ISouiFactory_CreateMenu(pThis, hMenu);
}

static inline IMenu* ISouiFactory_CreateMenu_Default_C(ISouiFactory* pThis)
{
    return ISouiFactory_CreateMenu_Default(pThis);
}

static inline IMenuEx* ISouiFactory_CreateMenuEx_C(ISouiFactory* pThis)
{
    return ISouiFactory_CreateMenuEx(pThis);
}

static inline ITimer* ISouiFactory_CreateTimer_C(ISouiFactory* pThis, IEvtSlot* pEvtSlot)
{
    return ISouiFactory_CreateTimer(pThis, pEvtSlot);
}

static inline IAnimatorGroup* ISouiFactory_CreateAnimatorGroup_C(ISouiFactory* pThis)
{
    return ISouiFactory_CreateAnimatorGroup(pThis);
}

/*
 * Convenience macros for common factory operations
 */

/* String creation shortcuts */
#define ISouiFactory_CreateEmptyStringA(This) \
    ISouiFactory_CreateStringA(This, "")

#define ISouiFactory_CreateEmptyStringW(This) \
    ISouiFactory_CreateStringW(This, L"")

/* Unicode/ANSI string creation based on UNICODE define */
#ifdef UNICODE
#define ISouiFactory_CreateString(This, pszSrc) \
    ISouiFactory_CreateStringW(This, pszSrc)
#define ISouiFactory_CreateEmptyString(This) \
    ISouiFactory_CreateEmptyStringW(This)
#else
#define ISouiFactory_CreateString(This, pszSrc) \
    ISouiFactory_CreateStringA(This, pszSrc)
#define ISouiFactory_CreateEmptyString(This) \
    ISouiFactory_CreateEmptyStringA(This)
#endif

/* Resource provider shortcuts */
#define ISouiFactory_CreateFileResProvider(This) \
    ISouiFactory_CreateResProvider(This, BUILTIN_RESTYPE_FILE)

#define ISouiFactory_CreateZipResProvider(This) \
    ISouiFactory_CreateResProvider(This, BUILTIN_RESTYPE_ZIP)

#define ISouiFactory_CreatePEResProvider(This) \
    ISouiFactory_CreateResProvider(This, BUILTIN_RESTYPE_PE)

/* Global factory function wrapper */
static inline ISouiFactory* CreateSouiFactory_C(void)
{
    return CreateSouiFactory();
}

/*
 * Safe factory pattern - automatically manages factory lifetime
 */
#define SOUI_FACTORY_SCOPE_BEGIN() \
    do { \
        ISouiFactory* factory = CreateSouiFactory_C(); \
        if (factory) { \
            ISouiFactory_AddRef(factory);

#define SOUI_FACTORY_SCOPE_END() \
            ISouiFactory_Release(factory); \
        } \
    } while(0)

/*
 * Example usage of scoped factory:
 * 
 * SOUI_FACTORY_SCOPE_BEGIN()
 *     IStringW* str = ISouiFactory_CreateStringW(factory, L"Hello");
 *     // Use str...
 *     IStringW_Release(str);
 * SOUI_FACTORY_SCOPE_END()
 */

#ifdef __cplusplus
}
#endif

#endif /* __SFACTORY_CAPI_H__ */
