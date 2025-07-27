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
#define IIdleHandler_OnIdle(This) \
    ((This)->lpVtbl->OnIdle(This))

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

#define IMessageLoop_Quit_Default(This) \
    ((This)->lpVtbl->Quit(This, 0))

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

/* IMsgLoopFactory C API Macros */
#define IMsgLoopFactory_CreateMsgLoop(This, ppMsgLoop, pParentLoop) \
    ((This)->lpVtbl->CreateMsgLoop(This, ppMsgLoop, pParentLoop))

#define IMsgLoopFactory_CreateMsgLoop_Default(This, ppMsgLoop) \
    ((This)->lpVtbl->CreateMsgLoop(This, ppMsgLoop, NULL))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IMsgFilter Helper Functions */
static inline BOOL IMsgFilter_PreTranslateMessage_C(IMsgFilter* pThis, MSG* pMsg)
{
    return IMsgFilter_PreTranslateMessage(pThis, pMsg);
}

/* IIdleHandler Helper Functions */
static inline BOOL IIdleHandler_OnIdle_C(IIdleHandler* pThis)
{
    return IIdleHandler_OnIdle(pThis);
}

/* IMessageLoop Helper Functions */
static inline long IMessageLoop_AddRef_C(IMessageLoop* pThis)
{
    return IMessageLoop_AddRef(pThis);
}

static inline long IMessageLoop_Release_C(IMessageLoop* pThis)
{
    return IMessageLoop_Release(pThis);
}

static inline BOOL IMessageLoop_AddMessageFilter_C(IMessageLoop* pThis, IMsgFilter* pMessageFilter)
{
    return IMessageLoop_AddMessageFilter(pThis, pMessageFilter);
}

static inline BOOL IMessageLoop_RemoveMessageFilter_C(IMessageLoop* pThis, IMsgFilter* pMessageFilter)
{
    return IMessageLoop_RemoveMessageFilter(pThis, pMessageFilter);
}

static inline BOOL IMessageLoop_AddIdleHandler_C(IMessageLoop* pThis, IIdleHandler* pIdleHandler)
{
    return IMessageLoop_AddIdleHandler(pThis, pIdleHandler);
}

static inline BOOL IMessageLoop_RemoveIdleHandler_C(IMessageLoop* pThis, IIdleHandler* pIdleHandler)
{
    return IMessageLoop_RemoveIdleHandler(pThis, pIdleHandler);
}

static inline int IMessageLoop_Run_C(IMessageLoop* pThis)
{
    return IMessageLoop_Run(pThis);
}

static inline BOOL IMessageLoop_IsRunning_C(IMessageLoop* pThis)
{
    return IMessageLoop_IsRunning(pThis);
}

static inline void IMessageLoop_OnMsg_C(IMessageLoop* pThis, LPMSG pMsg)
{
    IMessageLoop_OnMsg(pThis, pMsg);
}

static inline void IMessageLoop_Quit_C(IMessageLoop* pThis, int exitCode)
{
    IMessageLoop_Quit(pThis, exitCode);
}

static inline BOOL IMessageLoop_PostTask_C(IMessageLoop* pThis, IRunnable* runable)
{
    return IMessageLoop_PostTask(pThis, runable);
}

static inline int IMessageLoop_RemoveTasksForObject_C(IMessageLoop* pThis, void* pObj)
{
    return IMessageLoop_RemoveTasksForObject(pThis, pObj);
}

static inline BOOL IMessageLoop_PeekMsg_C(IMessageLoop* pThis, LPMSG pMsg, UINT wMsgFilterMin, UINT wMsgFilterMax, BOOL bRemove)
{
    return IMessageLoop_PeekMsg(pThis, pMsg, wMsgFilterMin, wMsgFilterMax, bRemove);
}

static inline BOOL IMessageLoop_WaitMsg_C(IMessageLoop* pThis)
{
    return IMessageLoop_WaitMsg(pThis);
}

static inline int IMessageLoop_HandleMsg_C(IMessageLoop* pThis)
{
    return IMessageLoop_HandleMsg(pThis);
}

/* IMsgLoopFactory Helper Functions */
static inline HRESULT IMsgLoopFactory_CreateMsgLoop_C(IMsgLoopFactory* pThis, IMessageLoop** ppMsgLoop, IMessageLoop* pParentLoop)
{
    return IMsgLoopFactory_CreateMsgLoop(pThis, ppMsgLoop, pParentLoop);
}

/*
 * Convenience macros for common message loop operations
 */

/* Message loop lifecycle shortcuts */
#define IMessageLoop_Start(This) \
    IMessageLoop_Run(This)

#define IMessageLoop_Stop(This) \
    IMessageLoop_Quit_Default(This)

#define IMessageLoop_StopWithCode(This, code) \
    IMessageLoop_Quit(This, code)

/* Message processing shortcuts */
#define IMessageLoop_PeekAllMessages(This, pMsg) \
    IMessageLoop_PeekMsg(This, pMsg, 0, 0, TRUE)

#define IMessageLoop_PeekNoRemove(This, pMsg) \
    IMessageLoop_PeekMsg(This, pMsg, 0, 0, FALSE)

#define IMessageLoop_ProcessMessage(This, pMsg) \
    IMessageLoop_OnMsg(This, pMsg)

/* Task management shortcuts */
#define IMessageLoop_PostRunnable(This, runnable) \
    IMessageLoop_PostTask(This, runnable)

#define IMessageLoop_ClearTasks(This, obj) \
    IMessageLoop_RemoveTasksForObject(This, obj)

#define IMessageLoop_ProcessTasks(This) \
    IMessageLoop_ExecutePendingTask(This)

/* Handler management shortcuts */
#define IMessageLoop_AddFilter(This, filter) \
    IMessageLoop_AddMessageFilter(This, filter)

#define IMessageLoop_RemoveFilter(This, filter) \
    IMessageLoop_RemoveMessageFilter(This, filter)

#define IMessageLoop_AddIdle(This, handler) \
    IMessageLoop_AddIdleHandler(This, handler)

#define IMessageLoop_RemoveIdle(This, handler) \
    IMessageLoop_RemoveIdleHandler(This, handler)

/* Safe message loop operations */
#define IMessageLoop_SafeRun(This) \
    ((This) ? IMessageLoop_Run(This) : -1)

#define IMessageLoop_SafeQuit(This, code) \
    do { \
        if ((This) && IMessageLoop_IsRunning(This)) { \
            IMessageLoop_Quit(This, code); \
        } \
    } while(0)

#define IMessageLoop_SafePostTask(This, runnable) \
    ((This) && (runnable) ? IMessageLoop_PostTask(This, runnable) : FALSE)

#define IMessageLoop_SafeAddFilter(This, filter) \
    ((This) && (filter) ? IMessageLoop_AddMessageFilter(This, filter) : FALSE)

#define IMessageLoop_SafeRemoveFilter(This, filter) \
    ((This) && (filter) ? IMessageLoop_RemoveMessageFilter(This, filter) : FALSE)

#define IMessageLoop_SafeAddIdleHandler(This, handler) \
    ((This) && (handler) ? IMessageLoop_AddIdleHandler(This, handler) : FALSE)

#define IMessageLoop_SafeRemoveIdleHandler(This, handler) \
    ((This) && (handler) ? IMessageLoop_RemoveIdleHandler(This, handler) : FALSE)

/* Factory shortcuts */
#define IMsgLoopFactory_CreateLoop(This, ppLoop) \
    IMsgLoopFactory_CreateMsgLoop_Default(This, ppLoop)

#define IMsgLoopFactory_CreateChildLoop(This, ppLoop, parent) \
    IMsgLoopFactory_CreateMsgLoop(This, ppLoop, parent)

#define IMsgLoopFactory_SafeCreateLoop(This, ppLoop) \
    do { \
        if ((This) && (ppLoop)) { \
            if (FAILED(IMsgLoopFactory_CreateMsgLoop_Default(This, ppLoop))) { \
                *(ppLoop) = NULL; \
            } \
        } else if (ppLoop) { \
            *(ppLoop) = NULL; \
        } \
    } while(0)

/*
 * Message loop helper functions
 */
static inline BOOL IMessageLoop_ProcessSingleMessage_C(IMessageLoop* pThis)
{
    if (!pThis) return FALSE;
    
    MSG msg;
    if (IMessageLoop_PeekMsg(pThis, &msg, 0, 0, TRUE)) {
        IMessageLoop_OnMsg(pThis, &msg);
        return TRUE;
    }
    return FALSE;
}

static inline int IMessageLoop_ProcessAllMessages_C(IMessageLoop* pThis)
{
    if (!pThis) return 0;
    
    int count = 0;
    MSG msg;
    while (IMessageLoop_PeekMsg(pThis, &msg, 0, 0, TRUE)) {
        IMessageLoop_OnMsg(pThis, &msg);
        count++;
    }
    return count;
}

static inline BOOL IMessageLoop_RunWithTimeout_C(IMessageLoop* pThis, DWORD timeoutMs)
{
    if (!pThis) return FALSE;
    
    DWORD startTime = GetTickCount();
    while (IMessageLoop_IsRunning(pThis)) {
        if (GetTickCount() - startTime >= timeoutMs) {
            return FALSE; // Timeout
        }
        
        MSG msg;
        if (IMessageLoop_PeekMsg(pThis, &msg, 0, 0, TRUE)) {
            IMessageLoop_OnMsg(pThis, &msg);
        } else {
            Sleep(1); // Brief sleep to avoid busy waiting
        }
    }
    return TRUE; // Loop exited normally
}

/*
 * Reference counting helpers
 */
#define IMessageLoop_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IMessageLoop_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define IMsgLoopFactory_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IMsgLoopFactory_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#ifdef __cplusplus
}
#endif

#endif /* __SMSGLOOP_CAPI_H__ */
