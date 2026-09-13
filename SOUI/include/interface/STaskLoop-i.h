#ifndef __STASKLOOP_I__H__
#define __STASKLOOP_I__H__
#include <interface/obj-ref-i.h>
#include <interface/SRunnable-i.h>

SNSBEGIN

typedef enum Priority
{
    High = 1,
    Normal = 0,
    Low = -1,
} Priority;

typedef struct ITaskLoop ITaskLoop;
#undef INTERFACE
#define INTERFACE ITaskLoopListener
DECLARE_INTERFACE(ITaskLoopListener)
{
    /**
     * @brief Callback before the task loop starts
     * @param taskLoop ITaskLoop* -- Task loop object
     */
    STDMETHOD_(void, onStart)(THIS_ ITaskLoop * taskLoop) PURE;

    /**
     * @brief Callback before the task loop exits
     * @param taskLoop ITaskLoop* -- Task loop object
     */
    STDMETHOD_(void, onStop)(THIS_ ITaskLoop * taskLoop) PURE;
};

#undef INTERFACE
#define INTERFACE ITaskLoop
DECLARE_INTERFACE_(ITaskLoop, IObjRef)
{
    /**
     * @brief Add reference
     * @return long -- reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Release reference
     * @return long -- reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Get the name of the task queue
     * @param pszBuf char* -- Buffer
     * @param nBufLen int -- Buffer length
     * @return BOOL -- TRUE: success, FALSE: failure
     */
    STDMETHOD_(BOOL, getName)(THIS_ char *pszBuf, int nBufLen) PURE;

    /**
     * @brief Start a thread to run the task queue
     * @param pszName const char* -- Thread name
     * @param priority Priority -- Thread priority
     * @return void
     */
    STDMETHOD_(void, start)(THIS_ const char *pszName, Priority priority) PURE;

    /**
     * @brief Stop thread synchronization
     * @return void
     */
    STDMETHOD_(void, stop)(THIS) PURE;

    /**
     * @brief Post or send a task to the task manager
     * @param runnable const IRunnable* -- Task object to run
     * @param waitUntilDone BOOL -- TRUE: send task, FALSE: post task
     * @param priority int -- Task priority
     * @return long -- Task ID, can be used to cancel the task
     */
    STDMETHOD_(long, postTask)
    (THIS_ const IRunnable *runnable, BOOL waitUntilDone, int priority) PURE;

    /**
     * @brief Remove tasks of a specific object from the task loop's pending task list
     * @param object void* -- Specific object whose tasks are to be removed
     * @return void
     */
    STDMETHOD_(void, cancelTasksForObject)(THIS_ void *object) PURE;

    /**
     * @brief Cancel the task with the specified task ID
     * @param taskId long -- Task ID to cancel
     * @return BOOL -- TRUE: success, FALSE: failure
     */
    STDMETHOD_(BOOL, cancelTask)(THIS_ long taskId) PURE;

    /**
     * @brief Get the total number of tasks in the task loop queue
     * @return int -- Total number of tasks in the task loop queue
     */
    STDMETHOD_(int, getTaskCount)(CTHIS) SCONST PURE;

    /**
     * @brief Get the state of the running loop
     * @return BOOL -- TRUE: running, FALSE: not running
     */
    STDMETHOD_(BOOL, isRunning)(THIS) PURE;

    /**
     * @brief Get information of the running task
     * @param buf char* -- Buffer receiving task information
     * @param bufLen int -- Buffer length
     * @return BOOL -- TRUE: success, FALSE: no task running
     */
    STDMETHOD_(BOOL, getRunningTaskInfo)(THIS_ char *buf, int bufLen) PURE;

    /**
     * @brief Set a task to run repeatedly
     * @param pTask IRunnable* -- Task object to run
     * @param intervel int -- Heartbeat interval
     * @return void
     * @remark The task loop holds a clone of pTask. Set pTask to null to stop the heartbeat.
     */
    STDMETHOD_(void, setHeartBeatTask)(THIS_ IRunnable * pTask, int intervel) PURE;

    /**
     * @brief Set the task loop listener
     * @param listener ITaskLoopListener* -- Listener object
     * @return void
     * @remark The task loop does not own the listener; the caller must manage its lifetime.
     */
    STDMETHOD_(void, setListener)(THIS_ ITaskLoopListener * listener) PURE;

    /**
     * @brief Get the task loop listener
     * @return ITaskLoopListener* -- Listener object
     */
    STDMETHOD_(ITaskLoopListener *, getListener)(THIS) PURE;
};

SNSEND
#endif /**< __STASKLOOP_I__H__ */
