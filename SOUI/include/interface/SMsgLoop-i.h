#ifndef __SMSGLOOP_I__H__
#define __SMSGLOOP_I__H__

#include <windows.h>
#include <interface/obj-ref-i.h>
#include <interface/SRunnable-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IMsgFilter
/// @interface IMsgFilter
/// @brief Interface for message filtering.
DECLARE_INTERFACE(IMsgFilter)
{
    /**
     * @brief Preprocesses a message.
     * @param pMsg Pointer to the message.
     * @return TRUE if the message is handled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, PreTranslateMessage)(THIS_ MSG * pMsg) PURE;
};

#undef INTERFACE
#define INTERFACE IIdleHandler
/// @interface IIdleHandler
/// @brief Interface for handling idle time.
DECLARE_INTERFACE(IIdleHandler)
{
    /**
     * @brief Handles idle time.
     * @return TRUE if further idle processing is needed, FALSE otherwise.
     */
    STDMETHOD_(BOOL, OnIdle)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IMessageLoop
/// @interface IMessageLoop
/// @brief Interface for message loops.
DECLARE_INTERFACE_(IMessageLoop, IObjRef)
{
    /**
     * @brief Adds a reference to the object.
     * @return Current reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Releases a reference to the object.
     * @return Current reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Finalizes the release of the object.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    // Message filter operations

    /**
     * @brief Adds a message filter.
     * @param pMessageFilter Pointer to the message filter.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, AddMessageFilter)(THIS_ IMsgFilter * pMessageFilter) PURE;

    /**
     * @brief Removes a message filter.
     * @param pMessageFilter Pointer to the message filter.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RemoveMessageFilter)(THIS_ IMsgFilter * pMessageFilter) PURE;

    // Idle handler operations

    /**
     * @brief Adds an idle handler.
     * @param pIdleHandler Pointer to the idle handler.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, AddIdleHandler)(THIS_ IIdleHandler * pIdleHandler) PURE;

    /**
     * @brief Removes an idle handler.
     * @param pIdleHandler Pointer to the idle handler.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RemoveIdleHandler)(THIS_ IIdleHandler * pIdleHandler) PURE;

    // Override to change message filtering

    /**
     * @brief Preprocesses a message.
     * @param pMsg Pointer to the message.
     * @return TRUE if the message is handled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, PreTranslateMessage)(THIS_ MSG * pMsg) PURE;

    // Override to change idle processing

    /**
     * @brief Handles idle time.
     * @param nIdleCount Idle count.
     * @return TRUE if further idle processing is needed, FALSE otherwise.
     */
    STDMETHOD_(BOOL, OnIdle)(THIS_ int nIdleCount) PURE;

    /**
     * @brief Runs the message loop.
     * @return Return value of the message loop.
     */
    STDMETHOD_(int, Run)(THIS) PURE;

    /**
     * @brief Checks if the message loop is running.
     * @return TRUE if running, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsRunning)(CTHIS) SCONST PURE;

    /**
     * @brief Processes a message.
     * @param pMsg Pointer to the message.
     */
    STDMETHOD_(void, OnMsg)(THIS_ LPMSG pMsg) PURE;

    /**
     * @brief Quits the message loop.
     * @param exitCode Exit code.
     */
    STDMETHOD_(void, Quit)(THIS_ int exitCode DEF_VAL(0)) PURE;

    /**
     * @brief Posts a task to the message loop.
     * @param runable Pointer to the runnable object.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, PostTask)(THIS_ IRunnable * runable) PURE;

    /**
     * @brief Removes tasks associated with an object.
     * @param pObj Pointer to the object.
     * @return Number of tasks removed.
     */
    STDMETHOD_(int, RemoveTasksForObject)(THIS_ void *pObj) PURE;

    /**
     * @brief Executes pending tasks.
     */
    STDMETHOD_(void, ExecutePendingTask)(THIS) PURE;

    /**
     * @brief Peeks at messages in the message queue.
     * @param pMsg Pointer to the message.
     * @param wMsgFilterMin Minimum message filter value.
     * @param wMsgFilterMax Maximum message filter value.
     * @param bRemove Whether to remove the message.
     * @return TRUE if a message is available, FALSE otherwise.
     */
    STDMETHOD_(BOOL, PeekMsg)(THIS_ LPMSG pMsg, UINT wMsgFilterMin, UINT wMsgFilterMax, BOOL bRemove) PURE;

    /**
     * @brief Waits for a message.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, WaitMsg)(THIS) PURE;

    /**
     * @brief Handles a message.
     * @return Return value of message handling.
     */
    STDMETHOD_(int, HandleMsg)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IMsgLoopFactory
/// @interface IMsgLoopFactory
/// @brief Interface for creating message loops.
DECLARE_INTERFACE_(IMsgLoopFactory, IObjRef)
{
    /**
     * @brief Creates a message loop.
     * @param ppMsgLoop Pointer to the message loop pointer.
     * @param pParentLoop Pointer to the parent message loop (optional).
     * @return HRESULT
     */
    STDMETHOD_(HRESULT, CreateMsgLoop)(THIS_ IMessageLoop * *ppMsgLoop, IMessageLoop * pParentLoop DEF_VAL(NULL)) PURE;
};

SNSEND
#endif // __SMSGLOOP_I__H__