#ifndef __STRANSLATOR_CAPI_H__
#define __STRANSLATOR_CAPI_H__

#include "../STranslator-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for STranslator Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ITranslator C API Macros */
#define ITranslator_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ITranslator_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ITranslator_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ITranslator_Load(This, pData, uType) \
    ((This)->lpVtbl->Load(This, pData, uType))

#define ITranslator_GetName(This, szBuf) \
    ((This)->lpVtbl->GetName(This, szBuf))

#define ITranslator_GetNameA(This, str) \
    ((This)->lpVtbl->GetNameA(This, str))

#define ITranslator_NameEqual(This, pszName) \
    ((This)->lpVtbl->NameEqual(This, pszName))

#define ITranslator_NameEqualA(This, str) \
    ((This)->lpVtbl->NameEqualA(This, str))

#define ITranslator_guid(This) \
    ((This)->lpVtbl->guid(This))

#define ITranslator_tr(This, strSrc, strCtx, pszOut, nLen) \
    ((This)->lpVtbl->tr(This, strSrc, strCtx, pszOut, nLen))

#define ITranslator_getFontInfo(This, strFont) \
    ((This)->lpVtbl->getFontInfo(This, strFont))

/* ITranslatorMgr C API Macros */
#define ITranslatorMgr_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ITranslatorMgr_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ITranslatorMgr_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ITranslatorMgr_IsValid(This) \
    ((This)->lpVtbl->IsValid(This))

#define ITranslatorMgr_SetLanguage(This, pszLang) \
    ((This)->lpVtbl->SetLanguage(This, pszLang))

#define ITranslatorMgr_SetLanguageA(This, pszLang) \
    ((This)->lpVtbl->SetLanguageA(This, pszLang))

#define ITranslatorMgr_GetLanguage(This, szOut) \
    ((This)->lpVtbl->GetLanguage(This, szOut))

#define ITranslatorMgr_GetLanguageA(This, out) \
    ((This)->lpVtbl->GetLanguageA(This, out))

#define ITranslatorMgr_CreateTranslator(This, ppTranslator) \
    ((This)->lpVtbl->CreateTranslator(This, ppTranslator))

#define ITranslatorMgr_InstallTranslator(This, ppTranslator) \
    ((This)->lpVtbl->InstallTranslator(This, ppTranslator))

#define ITranslatorMgr_UninstallTranslator(This, id) \
    ((This)->lpVtbl->UninstallTranslator(This, id))

#define ITranslatorMgr_tr(This, strSrc, strCtx, pszOut, nLen) \
    ((This)->lpVtbl->tr(This, strSrc, strCtx, pszOut, nLen))

#ifdef __cplusplus
}
#endif

#endif /* __STRANSLATOR_CAPI_H__ */
