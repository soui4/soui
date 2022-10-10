#pragma once

#include <core/SSingleton2.h>
#include <helper/SCriticalSection.h>
#include <interface/SNotifyCenter-i.h>
#if _MSC_VER >= 1700 // VS2012
#define ENABLE_RUNONUI
#endif
#ifdef ENABLE_RUNONUI
#include <functional>
// 将 闭包 传递到了 UI线程
// 所以 这里 尽量 将 相同类型的 处理 放到一起 执行  而不是分开调用。

// SendMessage [&] 中的 & 是指 fn里调用的变量 都是 引用拷贝的
#define SRUNONUISYNC(fn) SNotifyCenter::getSingletonPtr()->RunOnUISync([&]() { fn })

// PostMessage [=] 中的 等号 是指 fn里调用的变量 都是 值拷贝的
#define SRUNONUI(fn) SNotifyCenter::getSingletonPtr()->RunOnUIAsync([=]() { fn })

#endif

SNSBEGIN

template <class T>
class TAutoEventMapReg {
    typedef TAutoEventMapReg<T> _thisClass;

  public:
    TAutoEventMapReg()
    {
        registerNotifyCenter();
    }

    ~TAutoEventMapReg()
    {
        unregisterNotifyCenter();
    }

    void registerNotifyCenter();
    void unregisterNotifyCenter();

  protected:
    BOOL OnEvent(IEvtArgs *e)
    {
        T *pThis = static_cast<T *>(this);
        return pThis->_HandleEvent(e);
    }
};

struct INotifyCallback
{
    virtual void OnFireEvent(IEvtArgs *e) = 0;
    virtual void OnFireEvts() = 0;
};

class SNotifyReceiver;

class SOUI_EXP SNotifyCenter
    : public INotifyCenter
	, public SSingleton2<SNotifyCenter>
    , public SEventSet
    , protected INotifyCallback {
	SINGLETON2_TYPE(SINGLETON_NOTIFYCENTER)
	friend SApplication;
private:
    SNotifyCenter(int nIntervel = 20);
    ~SNotifyCenter(void);

public:
   /**
     * FireEventSync
     * @brief    触发一个同步通知事件
     * @param    EventArgs *e -- 事件对象
     * @return
     *
     * Describe  只能在UI线程中调用
     */
    STDMETHOD_(void,FireEventSync)(THIS_ IEvtArgs *e) OVERRIDE;

    /**
     * FireEventAsync
     * @brief    触发一个异步通知事件
     * @param    EventArgs *e -- 事件对象
     * @return
     *
     * Describe  可以在非UI线程中调用，EventArgs
     * *e必须是从堆上分配的内存，调用后使用Release释放引用计数
     */
    STDMETHOD_(void,FireEventAsync)(THIS_ IEvtArgs *e) OVERRIDE;

    /**
     * RegisterEventMap
     * @brief    注册一个处理通知的对象
     * @param    const ISlotFunctor *slot -- 事件处理对象
     * @return
     *
     * Describe
     */
    STDMETHOD_(BOOL, RegisterEventMap)(THIS_ const IEvtSlot *slot) OVERRIDE;

    /**
     * RegisterEventMap
     * @brief    注销一个处理通知的对象
     * @param    const ISlotFunctor *slot -- 事件处理对象
     * @return
     *
     * Describe
     */
    STDMETHOD_(BOOL, UnregisterEventMap)(THIS_ const IEvtSlot *slot) OVERRIDE;

    /**
     * RunOnUI
     * @brief    
     * @param    IRunnable * pRunnable -- runnable执行体
	 * @param    BOOL bSync -- 同步执行标志
     * @return
     *
     * Describe
     */
	STDMETHOD_(void,RunOnUI)(THIS_ IRunnable * pRunnable,BOOL bSync) OVERRIDE;

	STDMETHOD_(void,RunOnUI2)(THIS_ FunRunOnUI fun, WPARAM wp, LPARAM lp,BOOL bSync) OVERRIDE;
public:

#ifdef ENABLE_RUNONUI
	void RunOnUISync(std::function<void(void)> fn);
	void RunOnUIAsync(std::function<void(void)> fn);
#endif

  protected:
    virtual void OnFireEvent(IEvtArgs *e);
    virtual void OnFireEvts();

    DWORD m_dwMainTrdID; //主线程ID

    SList<IEvtSlot *> m_evtHandlerMap;

    SNotifyReceiver *m_pReceiver;

    SCriticalSection m_cs;
    SList<IEvtArgs *> m_ayncEvent;
    BOOL m_bRunning;
    int m_nInterval;

	SList<SAutoRefPtr<IRunnable> > m_asyncRunnable;

};

template <class T>
inline void TAutoEventMapReg<T>::registerNotifyCenter()
{
    SNotifyCenter::getSingleton().RegisterEventMap(&Subscriber(&_thisClass::OnEvent, this));
}

template <class T>
inline void TAutoEventMapReg<T>::unregisterNotifyCenter()
{
    SNotifyCenter::getSingleton().UnregisterEventMap(&Subscriber(&_thisClass::OnEvent, this));
}

SNSEND
