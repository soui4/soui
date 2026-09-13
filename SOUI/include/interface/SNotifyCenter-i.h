#ifndef __SNOTIFYCENTER_I__H__
#define __SNOTIFYCENTER_I__H__
#include <utilities-def.h>
#include <interface/SEvtArgs-i.h>
#include <interface/STaskLoop-i.h>

SNSBEGIN

typedef void (*FunRunOnUI)(WPARAM wp, LPARAM lp);

#undef INTERFACE
#define INTERFACE INotifyCenter
DECLARE_INTERFACE(INotifyCenter)
{
    /**
     * @brief Trigger a synchronous notification event
     * @param e IEvtArgs* -- event object
     * @return void
     * @remark Can only be called in the UI thread
     */
    STDMETHOD_(void, FireEventSync)(THIS_ IEvtArgs * e) PURE;

    /**
     * @brief Trigger an asynchronous notification event
     * @param e IEvtArgs* -- event object
     * @return void
     * @remark Can be called in non-UI threads. IEvtArgs *e must be memory allocated on the heap; call Release to release the reference count after use.
     */
    STDMETHOD_(void, FireEventAsync)(THIS_ IEvtArgs * e) PURE;

    /**
     * @brief Register an object that handles notifications
     * @param slot const IEvtSlot* -- event handler object
     * @return BOOL
     */
    STDMETHOD_(BOOL, RegisterEventMap)(THIS_ const IEvtSlot *slot) PURE;

    /**
     * @brief Unregister an object that handles notifications
     * @param slot const IEvtSlot* -- event handler object
     * @return BOOL
     */
    STDMETHOD_(BOOL, UnregisterEventMap)(THIS_ const IEvtSlot *slot) PURE;

    /**
     * @brief Execute a task in the UI thread
     * @param pRunnable IRunnable* -- runnable body
     * @param bSync BOOL -- synchronous execution flag
     * @return void
     */
    STDMETHOD_(void, RunOnUI)(THIS_ IRunnable * pRunnable, BOOL bSync) PURE;

    /**
     * @brief Execute a task in the UI thread
     * @param fun FunRunOnUI -- callback function
     * @param wp WPARAM -- param 1
     * @param lp LPARAM -- param 2
     * @param bSync BOOL -- synchronous execution flag
     * @return void
     */
    STDMETHOD_(void, RunOnUI2)(THIS_ FunRunOnUI fun, WPARAM wp, LPARAM lp, BOOL bSync) PURE;
};

SNSEND
#endif /**< __SNOTIFYCENTER_I__H__ */