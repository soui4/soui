#ifndef __SAPP_CAPI_H__
#define __SAPP_CAPI_H__

#include "../sapp-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for IApplication Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IObjRef base interface macros */
#define IApplication_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IApplication_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IApplication_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IApplication specific interface macros */
#define IApplication_GetModule(This) \
    ((This)->lpVtbl->GetModule(This))

#define IApplication_LoadSystemNamedResource(This, pResProvider) \
    ((This)->lpVtbl->LoadSystemNamedResource(This, pResProvider))

#define IApplication_GetTranslator(This) \
    ((This)->lpVtbl->GetTranslator(This))

#define IApplication_SetTranslator(This, pTrans) \
    ((This)->lpVtbl->SetTranslator(This, pTrans))

#define IApplication_GetToolTipFactory(This) \
    ((This)->lpVtbl->GetToolTipFactory(This))

#define IApplication_SetToolTipFactory(This, pToolTipFac) \
    ((This)->lpVtbl->SetToolTipFactory(This, pToolTipFac))

#define IApplication_SetMsgLoopFactory(This, pMsgLoopFac) \
    ((This)->lpVtbl->SetMsgLoopFactory(This, pMsgLoopFac))

#define IApplication_GetMsgLoopFactory(This) \
    ((This)->lpVtbl->GetMsgLoopFactory(This))

#define IApplication_SetLogManager(This, pLogMgr) \
    ((This)->lpVtbl->SetLogManager(This, pLogMgr))

#define IApplication_GetLogManager(This) \
    ((This)->lpVtbl->GetLogManager(This))

#define IApplication_SetAttrStorageFactory(This, pAttrStorageFactory) \
    ((This)->lpVtbl->SetAttrStorageFactory(This, pAttrStorageFactory))

#define IApplication_GetAttrStorageFactory(This) \
    ((This)->lpVtbl->GetAttrStorageFactory(This))

#define IApplication_Run(This, hMainWnd) \
    ((This)->lpVtbl->Run(This, hMainWnd))

#define IApplication_Quit(This, nCode) \
    ((This)->lpVtbl->Quit(This, nCode))

#define IApplication_GetMainWnd(This) \
    ((This)->lpVtbl->GetMainWnd(This))

#define IApplication_AddMsgLoop(This, pMsgLoop, bReplace) \
    ((This)->lpVtbl->AddMsgLoop(This, pMsgLoop, bReplace))

#define IApplication_AddMsgLoop_Default(This, pMsgLoop) \
    ((This)->lpVtbl->AddMsgLoop(This, pMsgLoop, FALSE))

#define IApplication_RemoveMsgLoop(This) \
    ((This)->lpVtbl->RemoveMsgLoop(This))

#define IApplication_GetMsgLoop(This, dwThreadID) \
    ((This)->lpVtbl->GetMsgLoop(This, dwThreadID))

#define IApplication_GetMsgLoop_Current(This) \
    ((This)->lpVtbl->GetMsgLoop(This, GetCurrentThreadId()))

#define IApplication_GetResProviderMgr(This) \
    ((This)->lpVtbl->GetResProviderMgr(This))

#define IApplication_GetRealWndHander(This) \
    ((This)->lpVtbl->GetRealWndHander(This))

#define IApplication_SetRealWndHandler(This, pRealHandler) \
    ((This)->lpVtbl->SetRealWndHandler(This, pRealHandler))

#define IApplication_GetRenderFactory(This) \
    ((This)->lpVtbl->GetRenderFactory(This))

#define IApplication_SetRenderFactory(This, renderFac) \
    ((This)->lpVtbl->SetRenderFactory(This, renderFac))

#define IApplication_SetScriptFactory(This, pScriptModule) \
    ((This)->lpVtbl->SetScriptFactory(This, pScriptModule))

#define IApplication_InitXmlNamedID(This, pNames, nIds, nCount) \
    ((This)->lpVtbl->InitXmlNamedID(This, pNames, nIds, nCount))

#define IApplication_LoadXmlDocment(This, strResId) \
    ((This)->lpVtbl->LoadXmlDocment(This, strResId))

#define IApplication_LoadXmlDocmentU8(This, strResId) \
    ((This)->lpVtbl->LoadXmlDocmentU8(This, strResId))

#define IApplication_LoadAnimation(This, strResId) \
    ((This)->lpVtbl->LoadAnimation(This, strResId))

#define IApplication_LoadAnimationU8(This, strResId) \
    ((This)->lpVtbl->LoadAnimationU8(This, strResId))

#define IApplication_LoadValueAnimator(This, strResId) \
    ((This)->lpVtbl->LoadValueAnimator(This, strResId))

#define IApplication_LoadValueAnimatorU8(This, strResId) \
    ((This)->lpVtbl->LoadValueAnimatorU8(This, strResId))

#define IApplication_LoadImage(This, strResId) \
    ((This)->lpVtbl->LoadImage(This, strResId))

#define IApplication_LoadImageU8(This, strResId) \
    ((This)->lpVtbl->LoadImageU8(This, strResId))

#define IApplication_LoadTranslator(This, strResId) \
    ((This)->lpVtbl->LoadTranslator(This, strResId))

#define IApplication_LoadTranslatorU8(This, strResId) \
    ((This)->lpVtbl->LoadTranslatorU8(This, strResId))

#define IApplication_InstallTranslator(This, trModule) \
    ((This)->lpVtbl->InstallTranslator(This, trModule))

#define IApplication_UnnstallTranslator(This, langId) \
    ((This)->lpVtbl->UnnstallTranslator(This, langId))

#define IApplication_EnableNotifyCenter(This, bEnable, interval) \
    ((This)->lpVtbl->EnableNotifyCenter(This, bEnable, interval))

#define IApplication_EnableNotifyCenter_Default(This, bEnable) \
    ((This)->lpVtbl->EnableNotifyCenter(This, bEnable, 20))

#define IApplication_GetInnerSingleton(This, type) \
    ((This)->lpVtbl->GetInnerSingleton(This, type))

#define IApplication_CreateObject(This, pszName, nType) \
    ((This)->lpVtbl->CreateObject(This, pszName, nType))

#define IApplication_SetCreateObjectCallback(This, cbCreateObj) \
    ((This)->lpVtbl->SetCreateObjectCallback(This, cbCreateObj))

#define IApplication_RegisterObjFactory(This, objFac, bReplace) \
    ((This)->lpVtbl->RegisterObjFactory(This, objFac, bReplace))

#define IApplication_RegisterObjFactory_Default(This, objFac) \
    ((This)->lpVtbl->RegisterObjFactory(This, objFac, FALSE))

#define IApplication_UnregisterObjFactory(This, objFac) \
    ((This)->lpVtbl->UnregisterObjFactory(This, objFac))

#define IApplication_SetDefaultFontInfo(This, pszFontInfo) \
    ((This)->lpVtbl->SetDefaultFontInfo(This, pszFontInfo))

#define IApplication_CreateTaskLoop(This, nCount, priority, bAutoStart) \
    ((This)->lpVtbl->CreateTaskLoop(This, nCount, priority, bAutoStart))

#define IApplication_CreateTaskLoop_Default(This, nCount, priority) \
    ((This)->lpVtbl->CreateTaskLoop(This, nCount, priority, TRUE))

#define IApplication_GetTaskLoop(This, iTaskLoop) \
    ((This)->lpVtbl->GetTaskLoop(This, iTaskLoop))

#define IApplication_GetTaskLoop_Default(This) \
    ((This)->lpVtbl->GetTaskLoop(This, 0))

#define IApplication_SetCreateTaskLoopCallback(This, cbCreateTaskLoop) \
    ((This)->lpVtbl->SetCreateTaskLoopCallback(This, cbCreateTaskLoop))

#define IApplication_CreateScriptModule(This, ppScriptModule) \
    ((This)->lpVtbl->CreateScriptModule(This, ppScriptModule))

#define IApplication_SetEditCtxMenuTemplateResId(This, resId, pResProvider) \
    ((This)->lpVtbl->SetEditCtxMenuTemplateResId(This, resId, pResProvider))

#define IApplication_SetEditCtxMenuTemplateResId_Default(This, resId) \
    ((This)->lpVtbl->SetEditCtxMenuTemplateResId(This, resId, NULL))

#define IApplication_SetMessageBoxTemplateResId(This, resId, pResProvider) \
    ((This)->lpVtbl->SetMessageBoxTemplateResId(This, resId, pResProvider))

#define IApplication_SetMessageBoxTemplateResId_Default(This, resId) \
    ((This)->lpVtbl->SetMessageBoxTemplateResId(This, resId, NULL))

#define IApplication_SetAttrAlias(This, pAttrAlias) \
    ((This)->lpVtbl->SetAttrAlias(This, pAttrAlias))

#define IApplication_GetAttrAlias(This) \
    ((This)->lpVtbl->GetAttrAlias(This))

#define IApplication_GetBaseClassName(This, pszClassName, objType, pszBaseClassName) \
    ((This)->lpVtbl->GetBaseClassName(This, pszClassName, objType, pszBaseClassName))

#ifdef __cplusplus
}
#endif

#endif /* __SAPP_CAPI_H__ */
