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

#ifdef __cplusplus
}
#endif

#endif /* __SSCRIPTMODULE_CAPI_H__ */
