#ifndef __SMSGLOOP__H__
#define __SMSGLOOP__H__

#include <soui_exp.h>
#include <helper/obj-ref-impl.hpp>
#include <interface/SMsgLoop-i.h>

SNSBEGIN

// Forward declarations
class SMessageLoopPriv;

/**
 * @brief Message loop class for handling window messages and idle processing.
 * This class implements the IMessageLoop interface and manages the message loop for a window.
 */
class SOUI_EXP SMessageLoop : public TObjRefImpl<IMessageLoop> {
  public:
    /**
     * @brief Constructor for SMessageLoop.
     * @param pParentLoop Pointer to the parent message loop.
     */
    SMessageLoop(IMessageLoop *pParentLoop);

    /**
     * @brief Destructor for SMessageLoop.
     */
    virtual ~SMessageLoop();

  public:
    /**
     * @brief Adds a message filter to the message loop.
     * @param pMessageFilter Pointer to the message filter to add.
     * @return TRUE if the filter is added successfully, FALSE otherwise.
     */
    STDMETHOD_(BOOL, AddMessageFilter)(THIS_ IMsgFilter *pMessageFilter) OVERRIDE;

    /**
     * @brief Removes a message filter from the message loop.
     * @param pMessageFilter Pointer to the message filter to remove.
     * @return TRUE if the filter is removed successfully, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RemoveMessageFilter)(THIS_ IMsgFilter *pMessageFilter) OVERRIDE;

    /**
     * @brief Adds an idle handler to the message loop.
     * @param pIdleHandler Pointer to the idle handler to add.
     * @return TRUE if the handler is added successfully, FALSE otherwise.
     */
    STDMETHOD_(BOOL, AddIdleHandler)(THIS_ IIdleHandler *pIdleHandler) OVERRIDE;

    /**
     * @brief Removes an idle handler from the message loop.
     * @param pIdleHandler Pointer to the idle handler to remove.
     * @return TRUE if the handler is removed successfully, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RemoveIdleHandler)(THIS_ IIdleHandler *pIdleHandler) OVERRIDE;

    /**
     * @brief Pre-translates a message before it is dispatched.
     * @param pMsg Pointer to the message to translate.
     * @return TRUE if the message is translated and should not be dispatched further, FALSE otherwise.
     */
    STDMETHOD_(BOOL, PreTranslateMessage)(THIS_ MSG *pMsg) OVERRIDE;

    /**
     * @brief Handles idle processing.
     * @param nIdleCount Idle count.
     * @return TRUE if more idle processing is needed, FALSE otherwise.
     */
    STDMETHOD_(BOOL, OnIdle)(THIS_ int nIdleCount) OVERRIDE;

    /**
     * @brief Runs the message loop.
     * @return Exit code of the message loop.
     */
    STDMETHOD_(int, Run)(THIS) OVERRIDE;

    /**
     * @brief Checks if the message loop is running.
     * @return TRUE if the message loop is running, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsRunning)(THIS) const OVERRIDE;

    /**
     * @brief Processes a single message.
     * @param pMsg Pointer to the message to process.
     */
    STDMETHOD_(void, OnMsg)(THIS_ LPMSG pMsg) OVERRIDE;

    /**
     * @brief Quits the message loop.
     * @param exitCode Exit code for the message loop.
     */
    STDMETHOD_(void, Quit)(THIS_ int exitCode) OVERRIDE;

    /**
     * @brief Posts a task to the message loop.
     * @param runable Pointer to the task to post.
     * @return TRUE if the task is posted successfully, FALSE otherwise.
     */
    STDMETHOD_(BOOL, PostTask)(THIS_ IRunnable *runable) OVERRIDE;

    /**
     * @brief Removes all tasks associated with a specific object.
     * @param pObj Pointer to the object whose tasks should be removed.
     * @return Number of tasks removed.
     */
    STDMETHOD_(int, RemoveTasksForObject)(THIS_ void *pObj) OVERRIDE;

    /**
     * @brief Executes all pending tasks.
     */
    STDMETHOD_(void, ExecutePendingTask)() OVERRIDE;

    /**
     * @brief Peeks at a message in the message queue without removing it.
     * @param pMsg Pointer to the message to receive.
     * @param wMsgFilterMin Minimum message value to peek.
     * @param wMsgFilterMax Maximum message value to peek.
     * @param bRemove TRUE to remove the message from the queue, FALSE otherwise.
     * @return TRUE if a message is available, FALSE otherwise.
     */
    STDMETHOD_(BOOL, PeekMsg)(THIS_ LPMSG pMsg, UINT wMsgFilterMin, UINT wMsgFilterMax, BOOL bRemove) OVERRIDE;

    /**
     * @brief Waits for a message in the message queue.
     * @return TRUE if a message is available, FALSE otherwise.
     */
    STDMETHOD_(BOOL, WaitMsg)(THIS) OVERRIDE;

    /**
     * @brief Handles a message from the message queue.
     * @return Result of the message handling.
     */
    STDMETHOD_(int, HandleMsg)(THIS) OVERRIDE;

  public:
    /**
     * @brief Checks if a message is an idle message.
     * @param pMsg Pointer to the message to check.
     * @return TRUE if the message is an idle message, FALSE otherwise.
     */
    static BOOL IsIdleMessage(MSG *pMsg);

  protected:
    /**
     * @brief Runs idle processing.
     */
    void RunIdle();

  protected:
    // Flag indicating whether the message loop is running
    BOOL m_bRunning;
    // Flag indicating whether the message loop should quit
    BOOL m_bQuit;
    // Flag indicating whether idle processing should be done
    BOOL m_bDoIdle;
    // Idle count
    int m_nIdleCount;

    // Critical section for thread safety
    SCriticalSection m_cs;
    // Critical section for the running queue
    SCriticalSection m_csRunningQueue;
    // Thread ID of the message loop
    tid_t m_tid;
    // Pointer to the private implementation
    SMessageLoopPriv *m_priv;
};

SNSEND

#endif // __SMSGLOOP__H__