#ifndef __STASKLOOP_CAPI_H__
#define __STASKLOOP_CAPI_H__

#include "../STaskLoop-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Task priority constants
 */
#define TASK_PRIORITY_HIGH      High
#define TASK_PRIORITY_NORMAL    Normal
#define TASK_PRIORITY_LOW       Low

/*
 * C API Helper Macros for STaskLoop Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

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

#define ITaskLoop_start_Normal(This, pszName) \
    ((This)->lpVtbl->start(This, pszName, TASK_PRIORITY_NORMAL))

#define ITaskLoop_stop(This) \
    ((This)->lpVtbl->stop(This))

#define ITaskLoop_postTask(This, runnable, waitUntilDone, priority) \
    ((This)->lpVtbl->postTask(This, runnable, waitUntilDone, priority))

#define ITaskLoop_postTask_Normal(This, runnable, waitUntilDone) \
    ((This)->lpVtbl->postTask(This, runnable, waitUntilDone, TASK_PRIORITY_NORMAL))

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

#define ITaskLoop_clearHeartBeatTask(This) \
    ((This)->lpVtbl->setHeartBeatTask(This, NULL, 0))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* ITaskLoop Helper Functions */
static inline long ITaskLoop_AddRef_C(ITaskLoop* pThis)
{
    return ITaskLoop_AddRef(pThis);
}

static inline long ITaskLoop_Release_C(ITaskLoop* pThis)
{
    return ITaskLoop_Release(pThis);
}

static inline BOOL ITaskLoop_getName_C(ITaskLoop* pThis, char* pszBuf, int nBufLen)
{
    return ITaskLoop_getName(pThis, pszBuf, nBufLen);
}

static inline void ITaskLoop_start_C(ITaskLoop* pThis, const char* pszName, Priority priority)
{
    ITaskLoop_start(pThis, pszName, priority);
}

static inline void ITaskLoop_stop_C(ITaskLoop* pThis)
{
    ITaskLoop_stop(pThis);
}

static inline long ITaskLoop_postTask_C(ITaskLoop* pThis, const IRunnable* runnable, BOOL waitUntilDone, int priority)
{
    return ITaskLoop_postTask(pThis, runnable, waitUntilDone, priority);
}

static inline void ITaskLoop_cancelTasksForObject_C(ITaskLoop* pThis, void* object)
{
    ITaskLoop_cancelTasksForObject(pThis, object);
}

static inline BOOL ITaskLoop_cancelTask_C(ITaskLoop* pThis, long taskId)
{
    return ITaskLoop_cancelTask(pThis, taskId);
}

static inline int ITaskLoop_getTaskCount_C(ITaskLoop* pThis)
{
    return ITaskLoop_getTaskCount(pThis);
}

static inline BOOL ITaskLoop_isRunning_C(ITaskLoop* pThis)
{
    return ITaskLoop_isRunning(pThis);
}

static inline BOOL ITaskLoop_getRunningTaskInfo_C(ITaskLoop* pThis, char* buf, int bufLen)
{
    return ITaskLoop_getRunningTaskInfo(pThis, buf, bufLen);
}

static inline void ITaskLoop_setHeartBeatTask_C(ITaskLoop* pThis, IRunnable* pTask, int intervel)
{
    ITaskLoop_setHeartBeatTask(pThis, pTask, intervel);
}

/*
 * Convenience macros for common task loop operations
 */

/* Task posting shortcuts */
#define ITaskLoop_PostTask(This, runnable) \
    ITaskLoop_postTask_Normal(This, runnable, FALSE)

#define ITaskLoop_SendTask(This, runnable) \
    ITaskLoop_postTask_Normal(This, runnable, TRUE)

#define ITaskLoop_PostHighPriorityTask(This, runnable) \
    ITaskLoop_postTask(This, runnable, FALSE, TASK_PRIORITY_HIGH)

#define ITaskLoop_PostLowPriorityTask(This, runnable) \
    ITaskLoop_postTask(This, runnable, FALSE, TASK_PRIORITY_LOW)

#define ITaskLoop_SendHighPriorityTask(This, runnable) \
    ITaskLoop_postTask(This, runnable, TRUE, TASK_PRIORITY_HIGH)

#define ITaskLoop_SendLowPriorityTask(This, runnable) \
    ITaskLoop_postTask(This, runnable, TRUE, TASK_PRIORITY_LOW)

/* Task management shortcuts */
#define ITaskLoop_CancelTasks(This, object) \
    ITaskLoop_cancelTasksForObject(This, object)

#define ITaskLoop_CancelTaskById(This, taskId) \
    ITaskLoop_cancelTask(This, taskId)

#define ITaskLoop_GetTaskCount(This) \
    ITaskLoop_getTaskCount(This)

#define ITaskLoop_HasTasks(This) \
    (ITaskLoop_getTaskCount(This) > 0)

#define ITaskLoop_IsEmpty(This) \
    (ITaskLoop_getTaskCount(This) == 0)

/* Thread management shortcuts */
#define ITaskLoop_StartNormal(This, name) \
    ITaskLoop_start_Normal(This, name)

#define ITaskLoop_StartHigh(This, name) \
    ITaskLoop_start(This, name, TASK_PRIORITY_HIGH)

#define ITaskLoop_StartLow(This, name) \
    ITaskLoop_start(This, name, TASK_PRIORITY_LOW)

#define ITaskLoop_Stop(This) \
    ITaskLoop_stop(This)

#define ITaskLoop_IsActive(This) \
    ITaskLoop_isRunning(This)

#define ITaskLoop_IsStopped(This) \
    (!ITaskLoop_isRunning(This))

/* Heartbeat shortcuts */
#define ITaskLoop_SetHeartBeat(This, task, interval) \
    ITaskLoop_setHeartBeatTask(This, task, interval)

#define ITaskLoop_ClearHeartBeat(This) \
    ITaskLoop_clearHeartBeatTask(This)

#define ITaskLoop_SetHeartBeat1Second(This, task) \
    ITaskLoop_setHeartBeatTask(This, task, 1000)

#define ITaskLoop_SetHeartBeat5Seconds(This, task) \
    ITaskLoop_setHeartBeatTask(This, task, 5000)

#define ITaskLoop_SetHeartBeat10Seconds(This, task) \
    ITaskLoop_setHeartBeatTask(This, task, 10000)

/* Safe task loop operations */
#define ITaskLoop_SafeStart(This, name, priority) \
    do { \
        if ((This) && (name) && !ITaskLoop_isRunning(This)) { \
            ITaskLoop_start(This, name, priority); \
        } \
    } while(0)

#define ITaskLoop_SafeStop(This) \
    do { \
        if ((This) && ITaskLoop_isRunning(This)) { \
            ITaskLoop_stop(This); \
        } \
    } while(0)

#define ITaskLoop_SafePostTask(This, runnable, wait, priority) \
    ((This) && (runnable) ? ITaskLoop_postTask(This, runnable, wait, priority) : -1)

#define ITaskLoop_SafeCancelTask(This, taskId) \
    ((This) && (taskId) >= 0 ? ITaskLoop_cancelTask(This, taskId) : FALSE)

#define ITaskLoop_SafeCancelTasks(This, object) \
    do { \
        if ((This) && (object)) { \
            ITaskLoop_cancelTasksForObject(This, object); \
        } \
    } while(0)

#define ITaskLoop_SafeSetHeartBeat(This, task, interval) \
    do { \
        if ((This) && (task) && (interval) > 0) { \
            ITaskLoop_setHeartBeatTask(This, task, interval); \
        } \
    } while(0)

/*
 * Task loop helper functions
 */
static inline BOOL ITaskLoop_IsValid_C(ITaskLoop* pThis)
{
    return (pThis != NULL);
}

static inline char* ITaskLoop_GetName_C(ITaskLoop* pThis, char* buffer, int bufferSize)
{
    if (!pThis || !buffer || bufferSize <= 0) return NULL;
    
    if (ITaskLoop_getName(pThis, buffer, bufferSize)) {
        return buffer;
    }
    return NULL;
}

static inline long ITaskLoop_PostTaskAndWait_C(ITaskLoop* pThis, const IRunnable* runnable, int priority)
{
    if (!pThis || !runnable) return -1;
    
    return ITaskLoop_postTask(pThis, runnable, TRUE, priority);
}

static inline long ITaskLoop_PostTaskAsync_C(ITaskLoop* pThis, const IRunnable* runnable, int priority)
{
    if (!pThis || !runnable) return -1;
    
    return ITaskLoop_postTask(pThis, runnable, FALSE, priority);
}

static inline BOOL ITaskLoop_WaitForCompletion_C(ITaskLoop* pThis, int timeoutMs)
{
    if (!pThis) return FALSE;
    
    // Simple polling implementation
    int elapsed = 0;
    const int pollInterval = 10; // 10ms
    
    while (ITaskLoop_isRunning(pThis) && ITaskLoop_getTaskCount(pThis) > 0) {
        if (timeoutMs > 0 && elapsed >= timeoutMs) {
            return FALSE; // Timeout
        }
        
        Sleep(pollInterval);
        elapsed += pollInterval;
    }
    
    return TRUE;
}

/*
 * Task loop state management
 */
typedef struct TaskLoopState {
    ITaskLoop* taskLoop;
    char name[64];
    int taskCount;
    BOOL isRunning;
    BOOL isValid;
    BOOL hasHeartBeat;
} TaskLoopState;

static inline void TaskLoopState_Init(TaskLoopState* state, ITaskLoop* taskLoop)
{
    if (state) {
        state->taskLoop = taskLoop;
        if (taskLoop) {
            ITaskLoop_getName(taskLoop, state->name, sizeof(state->name));
            state->taskCount = ITaskLoop_getTaskCount(taskLoop);
            state->isRunning = ITaskLoop_isRunning(taskLoop);
            state->isValid = TRUE;
            state->hasHeartBeat = FALSE; // Would need additional tracking
        } else {
            memset(state, 0, sizeof(TaskLoopState));
        }
    }
}

static inline void TaskLoopState_Update(TaskLoopState* state)
{
    if (state && state->taskLoop) {
        state->taskCount = ITaskLoop_getTaskCount(state->taskLoop);
        state->isRunning = ITaskLoop_isRunning(state->taskLoop);
    }
}

static inline long TaskLoopState_PostTask(TaskLoopState* state, const IRunnable* runnable, BOOL waitUntilDone, int priority)
{
    if (!state || !state->isValid || !state->taskLoop || !runnable) return -1;
    
    long taskId = ITaskLoop_postTask(state->taskLoop, runnable, waitUntilDone, priority);
    if (taskId >= 0) {
        state->taskCount++;
    }
    return taskId;
}

static inline BOOL TaskLoopState_CancelTask(TaskLoopState* state, long taskId)
{
    if (!state || !state->isValid || !state->taskLoop) return FALSE;
    
    if (ITaskLoop_cancelTask(state->taskLoop, taskId)) {
        if (state->taskCount > 0) {
            state->taskCount--;
        }
        return TRUE;
    }
    return FALSE;
}

/*
 * Reference counting helpers
 */
#define ITaskLoop_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define ITaskLoop_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void ITaskLoop_DebugInfo_C(ITaskLoop* pThis)
{
    if (!pThis) {
        printf("TaskLoop: NULL\n");
        return;
    }
    
    char name[64];
    char taskInfo[256];
    
    ITaskLoop_getName(pThis, name, sizeof(name));
    BOOL hasRunningTask = ITaskLoop_getRunningTaskInfo(pThis, taskInfo, sizeof(taskInfo));
    
    printf("TaskLoop: %p\n", pThis);
    printf("  Name: %s\n", name);
    printf("  Running: %s\n", ITaskLoop_isRunning(pThis) ? "Yes" : "No");
    printf("  Task Count: %d\n", ITaskLoop_getTaskCount(pThis));
    if (hasRunningTask) {
        printf("  Running Task: %s\n", taskInfo);
    }
}
#else
#define ITaskLoop_DebugInfo_C(This) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STASKLOOP_CAPI_H__ */
