#ifndef __SAPP_CAPI_H__
#define __SAPP_CAPI_H__

#include "../sapp-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Application singleton type constants
 */
#define APP_SINGLETON_UIDEF             SINGLETON_UIDEF
#define APP_SINGLETON_SWNDMGR           SINGLETON_SWNDMGR
#define APP_SINGLETON_TIMERGENERATOR    SINGLETON_TIMERGENERATOR
#define APP_SINGLETON_WINDOWFINDER      SINGLETON_WINDOWFINDER
#define APP_SINGLETON_RICHEDITMENUDEF   SINGLETON_RICHEDITMENUDEF
#define APP_SINGLETON_HOSTMGR           SINGLETON_HOSTMGR
#define APP_SINGLETON_NOTIFYCENTER      SINGLETON_NOTIFYCENTER

/*
 * C API Helper Macros for sapp Interface
 * These macros provide C-style function call syntax for C++ interface methods
 * Note: This covers the main methods from sapp-i.h (387 lines total)
 */

/* IApplication C API Macros */
#define IApplication_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IApplication_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IApplication_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

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

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IApplication Helper Functions */
static inline long IApplication_AddRef_C(IApplication* pThis)
{
    return IApplication_AddRef(pThis);
}

static inline long IApplication_Release_C(IApplication* pThis)
{
    return IApplication_Release(pThis);
}

static inline HMODULE IApplication_GetModule_C(const IApplication* pThis)
{
    return IApplication_GetModule(pThis);
}

static inline UINT IApplication_LoadSystemNamedResource_C(IApplication* pThis, IResProvider* pResProvider)
{
    return IApplication_LoadSystemNamedResource(pThis, pResProvider);
}

static inline ITranslatorMgr* IApplication_GetTranslator_C(IApplication* pThis)
{
    return IApplication_GetTranslator(pThis);
}

static inline void IApplication_SetTranslator_C(IApplication* pThis, ITranslatorMgr* pTrans)
{
    IApplication_SetTranslator(pThis, pTrans);
}

static inline IToolTipFactory* IApplication_GetToolTipFactory_C(IApplication* pThis)
{
    return IApplication_GetToolTipFactory(pThis);
}

static inline void IApplication_SetToolTipFactory_C(IApplication* pThis, IToolTipFactory* pToolTipFac)
{
    IApplication_SetToolTipFactory(pThis, pToolTipFac);
}

static inline BOOL IApplication_SetMsgLoopFactory_C(IApplication* pThis, IMsgLoopFactory* pMsgLoopFac)
{
    return IApplication_SetMsgLoopFactory(pThis, pMsgLoopFac);
}

static inline IMsgLoopFactory* IApplication_GetMsgLoopFactory_C(IApplication* pThis)
{
    return IApplication_GetMsgLoopFactory(pThis);
}

static inline void IApplication_SetLogManager_C(IApplication* pThis, ILogMgr* pLogMgr)
{
    IApplication_SetLogManager(pThis, pLogMgr);
}

static inline ILogMgr* IApplication_GetLogManager_C(IApplication* pThis)
{
    return IApplication_GetLogManager(pThis);
}

static inline void IApplication_SetAttrStorageFactory_C(IApplication* pThis, IAttrStorageFactory* pAttrStorageFactory)
{
    IApplication_SetAttrStorageFactory(pThis, pAttrStorageFactory);
}

static inline IAttrStorageFactory* IApplication_GetAttrStorageFactory_C(IApplication* pThis)
{
    return IApplication_GetAttrStorageFactory(pThis);
}

static inline int IApplication_Run_C(IApplication* pThis, HWND hMainWnd)
{
    return IApplication_Run(pThis, hMainWnd);
}

static inline void IApplication_Quit_C(IApplication* pThis, int nCode)
{
    IApplication_Quit(pThis, nCode);
}

static inline HWND IApplication_GetMainWnd_C(IApplication* pThis)
{
    return IApplication_GetMainWnd(pThis);
}

static inline BOOL IApplication_AddMsgLoop_C(IApplication* pThis, IMessageLoop* pMsgLoop, BOOL bReplace)
{
    return IApplication_AddMsgLoop(pThis, pMsgLoop, bReplace);
}

static inline BOOL IApplication_RemoveMsgLoop_C(IApplication* pThis)
{
    return IApplication_RemoveMsgLoop(pThis);
}

static inline IMessageLoop* IApplication_GetMsgLoop_C(const IApplication* pThis, tid_t dwThreadID)
{
    return IApplication_GetMsgLoop(pThis, dwThreadID);
}

static inline IResProviderMgr* IApplication_GetResProviderMgr_C(IApplication* pThis)
{
    return IApplication_GetResProviderMgr(pThis);
}

static inline IRealWndHandler* IApplication_GetRealWndHander_C(IApplication* pThis)
{
    return IApplication_GetRealWndHander(pThis);
}

/*
 * Convenience macros for common application operations
 */

/* Application lifecycle shortcuts */
#define IApplication_Start(This, mainWnd) \
    IApplication_Run(This, mainWnd)

#define IApplication_Exit(This, code) \
    IApplication_Quit(This, code)

#define IApplication_ExitSuccess(This) \
    IApplication_Quit(This, 0)

#define IApplication_ExitFailure(This) \
    IApplication_Quit(This, -1)

/* Module and window shortcuts */
#define IApplication_GetAppModule(This) \
    IApplication_GetModule(This)

#define IApplication_GetMainWindow(This) \
    IApplication_GetMainWnd(This)

/* Factory management shortcuts */
#define IApplication_GetTooltipFactory(This) \
    IApplication_GetToolTipFactory(This)

#define IApplication_SetTooltipFactory(This, factory) \
    IApplication_SetToolTipFactory(This, factory)

#define IApplication_GetMsgFactory(This) \
    IApplication_GetMsgLoopFactory(This)

#define IApplication_SetMsgFactory(This, factory) \
    IApplication_SetMsgLoopFactory(This, factory)

/* Resource and provider shortcuts */
#define IApplication_LoadSystemResource(This, provider) \
    IApplication_LoadSystemNamedResource(This, provider)

#define IApplication_GetResourceMgr(This) \
    IApplication_GetResProviderMgr(This)

#define IApplication_GetRealWndHandler(This) \
    IApplication_GetRealWndHander(This)

/* Message loop shortcuts */
#define IApplication_AddCurrentMsgLoop(This, msgLoop) \
    IApplication_AddMsgLoop_Default(This, msgLoop)

#define IApplication_GetCurrentMsgLoop(This) \
    IApplication_GetMsgLoop_Current(This)

/* Safe application operations */
#define IApplication_SafeRun(This, mainWnd) \
    ((This) ? IApplication_Run(This, mainWnd) : -1)

#define IApplication_SafeQuit(This, code) \
    do { \
        if (This) { \
            IApplication_Quit(This, code); \
        } \
    } while(0)

#define IApplication_SafeLoadSystemResource(This, provider) \
    ((This) && (provider) ? IApplication_LoadSystemNamedResource(This, provider) : 1)

#define IApplication_SafeSetTranslator(This, translator) \
    do { \
        if ((This) && (translator)) { \
            IApplication_SetTranslator(This, translator); \
        } \
    } while(0)

#define IApplication_SafeAddMsgLoop(This, msgLoop, replace) \
    ((This) && (msgLoop) ? IApplication_AddMsgLoop(This, msgLoop, replace) : FALSE)

/*
 * Application helper functions
 */
static inline BOOL IApplication_IsValid_C(IApplication* pThis)
{
    return (pThis != NULL);
}

static inline BOOL IApplication_HasMainWindow_C(IApplication* pThis)
{
    if (!pThis) return FALSE;
    
    HWND mainWnd = IApplication_GetMainWnd(pThis);
    return (mainWnd != NULL && IsWindow(mainWnd));
}

static inline BOOL IApplication_IsRunning_C(IApplication* pThis)
{
    if (!pThis) return FALSE;
    
    // Check if we have a main window and current message loop
    HWND mainWnd = IApplication_GetMainWnd(pThis);
    IMessageLoop* msgLoop = IApplication_GetMsgLoop_Current(pThis);
    
    return (mainWnd != NULL && msgLoop != NULL);
}

/*
 * Application state management
 */
typedef struct ApplicationState {
    IApplication* application;
    HMODULE appModule;
    HWND mainWindow;
    IMessageLoop* currentMsgLoop;
    ITranslatorMgr* translator;
    IResProviderMgr* resProviderMgr;
    IRealWndHandler* realWndHandler;
    BOOL isValid;
    BOOL isRunning;
} ApplicationState;

static inline void ApplicationState_Init(ApplicationState* state, IApplication* application)
{
    if (state) {
        state->application = application;
        if (application) {
            state->appModule = IApplication_GetModule(application);
            state->mainWindow = IApplication_GetMainWnd(application);
            state->currentMsgLoop = IApplication_GetMsgLoop_Current(application);
            state->translator = IApplication_GetTranslator(application);
            state->resProviderMgr = IApplication_GetResProviderMgr(application);
            state->realWndHandler = IApplication_GetRealWndHander(application);
            state->isValid = TRUE;
            state->isRunning = IApplication_IsRunning_C(application);
        } else {
            memset(state, 0, sizeof(ApplicationState));
        }
    }
}

static inline void ApplicationState_Update(ApplicationState* state)
{
    if (state && state->application) {
        ApplicationState_Init(state, state->application);
    }
}

/*
 * Reference counting helpers
 */
#define IApplication_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IApplication_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void IApplication_DebugInfo_C(IApplication* pThis)
{
    if (!pThis) {
        printf("Application: NULL\n");
        return;
    }
    
    printf("Application: %p\n", pThis);
    printf("  Module: %p\n", IApplication_GetModule(pThis));
    printf("  Main Window: %p\n", IApplication_GetMainWnd(pThis));
    printf("  Has Main Window: %s\n", IApplication_HasMainWindow_C(pThis) ? "Yes" : "No");
    printf("  Is Running: %s\n", IApplication_IsRunning_C(pThis) ? "Yes" : "No");
    printf("  Translator: %p\n", IApplication_GetTranslator(pThis));
    printf("  Resource Manager: %p\n", IApplication_GetResProviderMgr(pThis));
}
#else
#define IApplication_DebugInfo_C(This) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SAPP_CAPI_H__ */
