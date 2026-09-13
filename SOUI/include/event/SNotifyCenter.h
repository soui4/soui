#ifndef __SNOTIFYCENTER__H__
#define __SNOTIFYCENTER__H__

#include <core/SSingleton2.h>
#include <helper/SCriticalSection.h>
#include <interface/SNotifyCenter-i.h>
#include <event/SEventSet.h>

#if !defined(_WIN32) || _MSC_VER >= 1700 /**< VS2012 */
#define ENABLE_RUNONUI
#endif
#ifdef ENABLE_RUNONUI
#include <functional>
/** Pass the closure to the UI thread */
/** Therefore, here we try to group handlers of the same type together for execution, rather than calling them separately. */

/** In SendMessage [&], the & means the variables used in fn are captured by reference. */
#define SRUNONUISYNC(fn) SNotifyCenter::getSingletonPtr()->RunOnUISync([&]() { fn })

/** In PostMessage [=], the = means the variables used in fn are captured by value. */
#define SRUNONUI(fn) SNotifyCenter::getSingletonPtr()->RunOnUIAsync([=]() { fn })

#endif

SNSBEGIN

/**
 * @class TAutoEventMapReg
 * @brief Template class that automatically registers and unregisters the event map
 * @tparam T type
 */
template <class T>
class TAutoEventMapReg {
    typedef TAutoEventMapReg<T> _thisClass;

  public:
    /**
     * @brief Constructor; automatically registers the event map
     */
    TAutoEventMapReg()
    {
        registerNotifyCenter();
    }

    /**
     * @brief Destructor; automatically unregisters the event map
     */
    ~TAutoEventMapReg()
    {
        unregisterNotifyCenter();
    }

    /**
     * @brief Register the event map
     */
    void registerNotifyCenter();

    /**
     * @brief Unregister the event map
     */
    void unregisterNotifyCenter();

  protected:
    /**
     * @brief Handle the event
     * @param e event argument object
     * @return Returns TRUE on success, FALSE on failure
     */
    BOOL OnEvent(IEvtArgs *e)
    {
        T *pThis = static_cast<T *>(this);
        return pThis->_HandleEvent(e);
    }
};

/**
 * @struct INotifyCallback
 * @brief Notification callback interface
 */
struct INotifyCallback
{
    /**
     * @brief Fire the event
     * @param e event argument object
     */
    virtual void OnFireEvent(IEvtArgs *e) = 0;

    /**
     * @brief Fire multiple events
     */
    virtual void OnFireEvts() = 0;
};

class SNotifyReceiver;

/**
 * @class SNotifyCenter
 * @brief Notification center class; manages event registration, unregistration, and firing
 */
class SOUI_EXP SNotifyCenter
    : public INotifyCenter
    , public SSingleton2<SNotifyCenter>
    , public SEventSet
    , protected INotifyCallback {
    SINGLETON2_TYPE(SINGLETON_NOTIFYCENTER)
    friend SApplication;

  private:
    /**
     * @brief Constructor
     * @param nIntervel event handling interval (in milliseconds)
     */
    SNotifyCenter(int nIntervel = 20);

    /**
     * @brief Destructor
     */
    ~SNotifyCenter(void);

  public:
    /**
     * @brief Fire a synchronous notification event
     * @param e event argument object
     *
     * @details Can only be called in the UI thread
     */
    STDMETHOD_(void, FireEventSync)(THIS_ IEvtArgs *e) OVERRIDE;

    /**
     * @brief Fire an asynchronous notification event
     * @param e event argument object
     *
     * @details Can be called from a non-UI thread. EventArgs *e must be memory allocated on the heap; call Release to decrement the reference count after use.
     */
    STDMETHOD_(void, FireEventAsync)(THIS_ IEvtArgs *e) OVERRIDE;

    /**
     * @brief Register a notification handler object
     * @param slot event handler object
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, RegisterEventMap)(THIS_ const IEvtSlot *slot) OVERRIDE;

    /**
     * @brief Unregister a notification handler object
     * @param slot event handler object
     * @return Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, UnregisterEventMap)(THIS_ const IEvtSlot *slot) OVERRIDE;

    /**
     * @brief Run a runnable object in the UI thread
     * @param pRunnable runnable object
     * @param bSync synchronous execution flag
     */
    STDMETHOD_(void, RunOnUI)(THIS_ IRunnable *pRunnable, BOOL bSync) OVERRIDE;

    /**
     * @brief Run a function in the UI thread
     * @param fun function pointer
     * @param wp WPARAM parameter
     * @param lp LPARAM parameter
     * @param bSync synchronous execution flag
     */
    STDMETHOD_(void, RunOnUI2)(THIS_ FunRunOnUI fun, WPARAM wp, LPARAM lp, BOOL bSync) OVERRIDE;

  public:
#ifdef ENABLE_RUNONUI
    /**
     * @brief Run a closure synchronously in the UI thread
     * @param fn closure function
     */
    void RunOnUISync(std::function<void(void)> fn);

    /**
     * @brief Run a closure asynchronously in the UI thread
     * @param fn closure function
     */
    void RunOnUIAsync(std::function<void(void)> fn);
#endif

  protected:
    /**
     * @brief Fire the event
     * @param e event argument object
     */
    virtual void OnFireEvent(IEvtArgs *e);

    /**
     * @brief Fire multiple events
     */
    virtual void OnFireEvts();

    tid_t m_dwMainTrdID; /**< Main thread ID */

    SList<IEvtSlot *> m_evtHandlerMap; /**< Event handler object list */

    SNotifyReceiver *m_pReceiver; /**< Notification receiver */

    SCriticalSection m_cs;         /**< Critical section object */
    SList<IEvtArgs *> m_ayncEvent; /**< Asynchronous event list */
    BOOL m_bRunning;               /**< Running state flag */
    int m_nInterval;               /**< Event handling interval (in milliseconds) */

    SList<SAutoRefPtr<IRunnable>> m_asyncRunnable; /**< Asynchronous runnable object list */
};

template <class T>
inline void TAutoEventMapReg<T>::registerNotifyCenter()
{
    MemberFunctionSlot<TAutoEventMapReg<T>, IEvtArgs> slot = Subscriber(&_thisClass::OnEvent, this);
    SNotifyCenter::getSingleton().RegisterEventMap(&slot);
}

template <class T>
inline void TAutoEventMapReg<T>::unregisterNotifyCenter()
{
    MemberFunctionSlot<TAutoEventMapReg<T>, IEvtArgs> slot = Subscriber(&_thisClass::OnEvent, this);
    SNotifyCenter::getSingleton().UnregisterEventMap(&slot);
}

SNSEND

#endif /**< __SNOTIFYCENTER__H__ */