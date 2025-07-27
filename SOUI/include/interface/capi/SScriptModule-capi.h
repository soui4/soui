#ifndef __SSCRIPTMODULE_CAPI_H__
#define __SSCRIPTMODULE_CAPI_H__

#include "../SScriptModule-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SScriptModule Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IScriptModule C API Macros */
#define IScriptModule_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IScriptModule_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IScriptModule_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IScriptModule_GetScriptEngine(This) \
    ((This)->lpVtbl->GetScriptEngine(This))

#define IScriptModule_getIdentifierString(This) \
    ((This)->lpVtbl->getIdentifierString(This))

#define IScriptModule_executeScriptFile(This, pszScriptFile) \
    ((This)->lpVtbl->executeScriptFile(This, pszScriptFile))

#define IScriptModule_executeScriptBuffer(This, buff, sz) \
    ((This)->lpVtbl->executeScriptBuffer(This, buff, sz))

#define IScriptModule_executeScriptedEventHandler(This, handler_name, pEvt) \
    ((This)->lpVtbl->executeScriptedEventHandler(This, handler_name, pEvt))

#define IScriptModule_executeMain(This, hInst, pszWorkDir, pszArgs) \
    ((This)->lpVtbl->executeMain(This, hInst, pszWorkDir, pszArgs))

#define IScriptModule_getIdleHandler(This) \
    ((This)->lpVtbl->getIdleHandler(This))

/* IScriptFactory C API Macros */
#define IScriptFactory_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IScriptFactory_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IScriptFactory_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IScriptFactory_CreateScriptModule(This, ppScriptModule) \
    ((This)->lpVtbl->CreateScriptModule(This, ppScriptModule))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IScriptModule Helper Functions */
static inline long IScriptModule_AddRef_C(IScriptModule* pThis)
{
    return IScriptModule_AddRef(pThis);
}

static inline long IScriptModule_Release_C(IScriptModule* pThis)
{
    return IScriptModule_Release(pThis);
}

static inline void* IScriptModule_GetScriptEngine_C(IScriptModule* pThis)
{
    return IScriptModule_GetScriptEngine(pThis);
}

static inline LPCSTR IScriptModule_getIdentifierString_C(IScriptModule* pThis)
{
    return IScriptModule_getIdentifierString(pThis);
}

static inline BOOL IScriptModule_executeScriptFile_C(IScriptModule* pThis, LPCSTR pszScriptFile)
{
    return IScriptModule_executeScriptFile(pThis, pszScriptFile);
}

static inline void IScriptModule_executeScriptBuffer_C(IScriptModule* pThis, LPCSTR buff, size_t sz)
{
    IScriptModule_executeScriptBuffer(pThis, buff, sz);
}

static inline BOOL IScriptModule_executeScriptedEventHandler_C(IScriptModule* pThis, LPCSTR handler_name, IEvtArgs* pEvt)
{
    return IScriptModule_executeScriptedEventHandler(pThis, handler_name, pEvt);
}

static inline int IScriptModule_executeMain_C(IScriptModule* pThis, HINSTANCE hInst, LPCSTR pszWorkDir, LPCSTR pszArgs)
{
    return IScriptModule_executeMain(pThis, hInst, pszWorkDir, pszArgs);
}

static inline IIdleHandler* IScriptModule_getIdleHandler_C(IScriptModule* pThis)
{
    return IScriptModule_getIdleHandler(pThis);
}

/* IScriptFactory Helper Functions */
static inline long IScriptFactory_AddRef_C(IScriptFactory* pThis)
{
    return IScriptFactory_AddRef(pThis);
}

static inline long IScriptFactory_Release_C(IScriptFactory* pThis)
{
    return IScriptFactory_Release(pThis);
}

static inline HRESULT IScriptFactory_CreateScriptModule_C(IScriptFactory* pThis, IScriptModule** ppScriptModule)
{
    return IScriptFactory_CreateScriptModule(pThis, ppScriptModule);
}

/*
 * Convenience macros for common script operations
 */

/* Script execution shortcuts */
#define IScriptModule_ExecuteFile(This, filename) \
    IScriptModule_executeScriptFile(This, filename)

#define IScriptModule_ExecuteString(This, script) \
    IScriptModule_executeScriptBuffer(This, script, strlen(script))

#define IScriptModule_ExecuteBuffer(This, buffer, size) \
    IScriptModule_executeScriptBuffer(This, buffer, size)

/* Event handler execution */
#define IScriptModule_HandleEvent(This, handler, event) \
    IScriptModule_executeScriptedEventHandler(This, handler, event)

/* Script engine access */
#define IScriptModule_GetEngine(This) \
    IScriptModule_GetScriptEngine(This)

#define IScriptModule_GetEngineAs(This, Type) \
    ((Type*)IScriptModule_GetScriptEngine(This))

#define IScriptModule_GetIdentifier(This) \
    IScriptModule_getIdentifierString(This)

/* Main execution shortcuts */
#define IScriptModule_RunMain(This, hInst, workDir, args) \
    IScriptModule_executeMain(This, hInst, workDir, args)

#define IScriptModule_RunMainSimple(This, hInst) \
    IScriptModule_executeMain(This, hInst, NULL, NULL)

/* Safe script operations */
#define IScriptModule_SafeExecuteFile(This, filename) \
    (IObjRef_IsValidInterface(This) && (filename) ? IScriptModule_executeScriptFile(This, filename) : FALSE)

#define IScriptModule_SafeExecuteString(This, script) \
    do { \
        if (IObjRef_IsValidInterface(This) && (script)) { \
            IScriptModule_executeScriptBuffer(This, script, strlen(script)); \
        } \
    } while(0)

#define IScriptModule_SafeHandleEvent(This, handler, event) \
    (IObjRef_IsValidInterface(This) && (handler) && (event) ? \
     IScriptModule_executeScriptedEventHandler(This, handler, event) : FALSE)

/* Script factory shortcuts */
#define IScriptFactory_CreateModule(This, ppModule) \
    IScriptFactory_CreateScriptModule(This, ppModule)

#define IScriptFactory_SafeCreateModule(This, ppModule) \
    do { \
        if (IObjRef_IsValidInterface(This) && (ppModule)) { \
            if (FAILED(IScriptFactory_CreateScriptModule(This, ppModule))) { \
                *(ppModule) = NULL; \
            } \
        } else if (ppModule) { \
            *(ppModule) = NULL; \
        } \
    } while(0)

/*
 * Script execution helper functions
 */
static inline BOOL IScriptModule_ExecuteFileWithCheck_C(IScriptModule* pThis, LPCSTR filename)
{
    if (!IObjRef_IsValidInterface(pThis) || !filename) {
        return FALSE;
    }
    
    // Check if file exists (basic check)
    FILE* file = fopen(filename, "r");
    if (!file) {
        return FALSE;
    }
    fclose(file);
    
    return IScriptModule_executeScriptFile(pThis, filename);
}

static inline BOOL IScriptModule_ExecuteStringWithLength_C(IScriptModule* pThis, LPCSTR script, size_t length)
{
    if (!IObjRef_IsValidInterface(pThis) || !script || length == 0) {
        return FALSE;
    }
    
    IScriptModule_executeScriptBuffer(pThis, script, length);
    return TRUE;
}

static inline BOOL IScriptModule_ExecuteEventHandlerSafe_C(IScriptModule* pThis, LPCSTR handler_name, IEvtArgs* pEvt)
{
    if (!IObjRef_IsValidInterface(pThis) || !handler_name || !pEvt) {
        return FALSE;
    }
    
    return IScriptModule_executeScriptedEventHandler(pThis, handler_name, pEvt);
}

/*
 * Reference counting helpers
 */
#define IScriptModule_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IScriptModule_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define IScriptFactory_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IScriptFactory_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Script module management helpers
 */
typedef struct ScriptModuleManager {
    IScriptFactory* factory;
    IScriptModule* module;
    LPCSTR identifier;
    BOOL isInitialized;
} ScriptModuleManager;

static inline void ScriptModuleManager_Init(ScriptModuleManager* mgr, IScriptFactory* factory)
{
    if (mgr) {
        mgr->factory = factory;
        mgr->module = NULL;
        mgr->identifier = NULL;
        mgr->isInitialized = FALSE;
        if (factory) {
            IScriptFactory_SafeAddRef(factory);
        }
    }
}

static inline void ScriptModuleManager_Cleanup(ScriptModuleManager* mgr)
{
    if (mgr) {
        if (mgr->module) {
            IScriptModule_SafeRelease(&mgr->module);
        }
        if (mgr->factory) {
            IScriptFactory_SafeRelease(&mgr->factory);
        }
        memset(mgr, 0, sizeof(ScriptModuleManager));
    }
}

static inline BOOL ScriptModuleManager_CreateModule(ScriptModuleManager* mgr)
{
    if (!mgr || !mgr->factory) {
        return FALSE;
    }
    
    if (mgr->module) {
        return TRUE; // Already created
    }
    
    HRESULT hr = IScriptFactory_CreateScriptModule(mgr->factory, &mgr->module);
    if (SUCCEEDED(hr) && mgr->module) {
        mgr->identifier = IScriptModule_getIdentifierString(mgr->module);
        mgr->isInitialized = TRUE;
        return TRUE;
    }
    
    return FALSE;
}

static inline BOOL ScriptModuleManager_ExecuteFile(ScriptModuleManager* mgr, LPCSTR filename)
{
    if (!mgr || !mgr->module || !mgr->isInitialized) {
        return FALSE;
    }
    
    return IScriptModule_executeScriptFile(mgr->module, filename);
}

static inline BOOL ScriptModuleManager_HandleEvent(ScriptModuleManager* mgr, LPCSTR handler, IEvtArgs* evt)
{
    if (!mgr || !mgr->module || !mgr->isInitialized) {
        return FALSE;
    }
    
    return IScriptModule_executeScriptedEventHandler(mgr->module, handler, evt);
}

#ifdef __cplusplus
}
#endif

#endif /* __SSCRIPTMODULE_CAPI_H__ */
