#ifndef __STASKLOOP_CAPI_H__
#define __STASKLOOP_CAPI_H__

#include "../STaskLoop-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for STaskLoop Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ITaskLoopListener C API Macros */
#define ITaskLoopListener_onStart(This, taskLoop) \
    ((This)->lpVtbl->onStart(This, taskLoop))

#define ITaskLoopListener_onStop(This, taskLoop) \
    ((This)->lpVtbl->onStop(This, taskLoop))

/* ITaskLoop C API Macros */
#define ITaskLoop_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ITaskLoop_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ITaskLoop_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ITaskLoop_getName(This, pszBuf, nBufLen) \
    ((This)->lpVtbl->getName(This, pszBuf, nBufLen))

#define ITaskLoop_start(This, pszName, priority) \
    ((This)->lpVtbl->start(This, pszName, priority))

#define ITaskLoop_stop(This) \
    ((This)->lpVtbl->stop(This))

#define ITaskLoop_postTask(This, runnable, waitUntilDone, priority) \
    ((This)->lpVtbl->postTask(This, runnable, waitUntilDone, priority))

#define ITaskLoop_cancelTasksForObject(This, object) \
    ((This)->lpVtbl->cancelTasksForObject(This, object))

#define ITaskLoop_cancelTask(This, taskId) \
    ((This)->lpVtbl->cancelTask(This, taskId))

#define ITaskLoop_getTaskCount(This) \
    ((This)->lpVtbl->getTaskCount(This))

#define ITaskLoop_isRunning(This) \
    ((This)->lpVtbl->isRunning(This))

#define ITaskLoop_getRunningTaskInfo(This, buf, bufLen) \
    ((This)->lpVtbl->getRunningTaskInfo(This, buf, bufLen))

#define ITaskLoop_setHeartBeatTask(This, pTask, intervel) \
    ((This)->lpVtbl->setHeartBeatTask(This, pTask, intervel))

#define ITaskLoop_setListener(This, listener) \
    ((This)->lpVtbl->setListener(This, listener))

#define ITaskLoop_getListener(This) \
    ((This)->lpVtbl->getListener(This))

#ifdef __cplusplus
}
#endif

#endif /* __STASKLOOP_CAPI_H__ */
