#ifndef __SMSGLOOP_CAPI_H__
#define __SMSGLOOP_CAPI_H__

#include "../SMsgLoop-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SMsgLoop Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IMsgFilter C API Macros */
#define IMsgFilter_PreTranslateMessage(This, pMsg) \
    ((This)->lpVtbl->PreTranslateMessage(This, pMsg))

/* IIdleHandler C API Macros */
#define IIdleHandler_OnIdle(This, iRun) \
    ((This)->lpVtbl->OnIdle(This, iRun))

/* IMessageLoop C API Macros */
#define IMessageLoop_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IMessageLoop_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IMessageLoop_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IMessageLoop_AddMessageFilter(This, pMessageFilter) \
    ((This)->lpVtbl->AddMessageFilter(This, pMessageFilter))

#define IMessageLoop_RemoveMessageFilter(This, pMessageFilter) \
    ((This)->lpVtbl->RemoveMessageFilter(This, pMessageFilter))

#define IMessageLoop_AddIdleHandler(This, pIdleHandler) \
    ((This)->lpVtbl->AddIdleHandler(This, pIdleHandler))

#define IMessageLoop_RemoveIdleHandler(This, pIdleHandler) \
    ((This)->lpVtbl->RemoveIdleHandler(This, pIdleHandler))

#define IMessageLoop_PreTranslateMessage(This, pMsg) \
    ((This)->lpVtbl->PreTranslateMessage(This, pMsg))

#define IMessageLoop_OnIdle(This, nIdleCount) \
    ((This)->lpVtbl->OnIdle(This, nIdleCount))

#define IMessageLoop_Run(This) \
    ((This)->lpVtbl->Run(This))

#define IMessageLoop_IsRunning(This) \
    ((This)->lpVtbl->IsRunning(This))

#define IMessageLoop_OnMsg(This, pMsg) \
    ((This)->lpVtbl->OnMsg(This, pMsg))

#define IMessageLoop_Quit(This, exitCode) \
    ((This)->lpVtbl->Quit(This, exitCode))

#define IMessageLoop_PostTask(This, runable) \
    ((This)->lpVtbl->PostTask(This, runable))

#define IMessageLoop_RemoveTasksForObject(This, pObj) \
    ((This)->lpVtbl->RemoveTasksForObject(This, pObj))

#define IMessageLoop_ExecutePendingTask(This) \
    ((This)->lpVtbl->ExecutePendingTask(This))

#define IMessageLoop_PeekMsg(This, pMsg, wMsgFilterMin, wMsgFilterMax, bRemove) \
    ((This)->lpVtbl->PeekMsg(This, pMsg, wMsgFilterMin, wMsgFilterMax, bRemove))

#define IMessageLoop_WaitMsg(This) \
    ((This)->lpVtbl->WaitMsg(This))

#define IMessageLoop_HandleMsg(This) \
    ((This)->lpVtbl->HandleMsg(This))

#define IMessageLoop_OnStart(This) \
    ((This)->lpVtbl->OnStart(This))

#define IMessageLoop_OnStop(This) \
    ((This)->lpVtbl->OnStop(This))

/* IMsgLoopFactory C API Macros */
#define IMsgLoopFactory_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IMsgLoopFactory_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IMsgLoopFactory_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IMsgLoopFactory_CreateMsgLoop(This, ppMsgLoop, pParentLoop) \
    ((This)->lpVtbl->CreateMsgLoop(This, ppMsgLoop, pParentLoop))

#ifdef __cplusplus
}
#endif

#endif /* __SMSGLOOP_CAPI_H__ */
