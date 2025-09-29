#ifndef __SNOTIFYCENTER__H__
#define __SNOTIFYCENTER__H__

#include <core/SSingleton2.h>
#include <helper/SCriticalSection.h>
#include <interface/SNotifyCenter-i.h>
#include <event/SEventSet.h>

#if !defined(_WIN32) || _MSC_VER >= 1700 // VS2012
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

/**
 * @class TAutoEventMapReg
 * @brief 自动注册和注销事件映射的模板类
 * @tparam T 类型
 */
template <class T>
class TAutoEventMapReg {
    typedef TAutoEventMapReg<T> _thisClass;

  public:
    /**
     * @brief 构造函数，自动注册事件映射
     */
    TAutoEventMapReg()
    {
        registerNotifyCenter();
    }

    /**
     * @brief 析构函数，自动注销事件映射
     */
    ~TAutoEventMapReg()
    {
        unregisterNotifyCenter();
    }

    /**
     * @brief 注册事件映射
     */
    void registerNotifyCenter();

    /**
     * @brief 注销事件映射
     */
    void unregisterNotifyCenter();

  protected:
    /**
     * @brief 处理事件
     * @param e 事件参数对象
     * @return 成功返回TRUE，失败返回FALSE
     */
    BOOL OnEvent(IEvtArgs *e)
    {
        T *pThis = static_cast<T *>(this);
        return pThis->_HandleEvent(e);
    }
};

/**
 * @struct INotifyCallback
 * @brief 通知回调接口
 */
struct INotifyCallback
{
    /**
     * @brief 触发事件
     * @param e 事件参数对象
     */
    virtual void OnFireEvent(IEvtArgs *e) = 0;

    /**
     * @brief 触发多个事件
     */
    virtual void OnFireEvts() = 0;
};

class SNotifyReceiver;

/**
 * @class SNotifyCenter
 * @brief 通知中心类，管理事件的注册、注销和触发
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
     * @brief 构造函数
     * @param nIntervel 事件处理间隔时间（毫秒）
     */
    SNotifyCenter(int nIntervel = 20);

    /**
     * @brief 析构函数
     */
    ~SNotifyCenter(void);

  public:
    /**
     * @brief 触发一个同步通知事件
     * @param e 事件参数对象
     *
     * @details 只能在UI线程中调用
     */
    STDMETHOD_(void, FireEventSync)(THIS_ IEvtArgs *e) OVERRIDE;

    /**
     * @brief 触发一个异步通知事件
     * @param e 事件参数对象
     *
     * @details 可以在非UI线程中调用，EventArgs *e必须是从堆上分配的内存，调用后使用Release释放引用计数
     */
    STDMETHOD_(void, FireEventAsync)(THIS_ IEvtArgs *e) OVERRIDE;

    /**
     * @brief 注册一个处理通知的对象
     * @param slot 事件处理对象
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, RegisterEventMap)(THIS_ const IEvtSlot *slot) OVERRIDE;

    /**
     * @brief 注销一个处理通知的对象
     * @param slot 事件处理对象
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, UnregisterEventMap)(THIS_ const IEvtSlot *slot) OVERRIDE;

    /**
     * @brief 在UI线程中运行一个可运行对象
     * @param pRunnable 可运行对象
     * @param bSync 同步执行标志
     */
    STDMETHOD_(void, RunOnUI)(THIS_ IRunnable *pRunnable, BOOL bSync) OVERRIDE;

    /**
     * @brief 在UI线程中运行一个函数
     * @param fun 函数指针
     * @param wp WPARAM参数
     * @param lp LPARAM参数
     * @param bSync 同步执行标志
     */
    STDMETHOD_(void, RunOnUI2)(THIS_ FunRunOnUI fun, WPARAM wp, LPARAM lp, BOOL bSync) OVERRIDE;

  public:
#ifdef ENABLE_RUNONUI
    /**
     * @brief 在UI线程中同步运行一个闭包
     * @param fn 闭包函数
     */
    void RunOnUISync(std::function<void(void)> fn);

    /**
     * @brief 在UI线程中异步运行一个闭包
     * @param fn 闭包函数
     */
    void RunOnUIAsync(std::function<void(void)> fn);
#endif

  protected:
    /**
     * @brief 触发事件
     * @param e 事件参数对象
     */
    virtual void OnFireEvent(IEvtArgs *e);

    /**
     * @brief 触发多个事件
     */
    virtual void OnFireEvts();

    tid_t m_dwMainTrdID; // 主线程ID

    SList<IEvtSlot *> m_evtHandlerMap; // 事件处理对象列表

    SNotifyReceiver *m_pReceiver; // 通知接收器

    SCriticalSection m_cs;         // 临界区对象
    SList<IEvtArgs *> m_ayncEvent; // 异步事件列表
    BOOL m_bRunning;               // 运行状态标志
    int m_nInterval;               // 事件处理间隔时间（毫秒）

    SList<SAutoRefPtr<IRunnable>> m_asyncRunnable; // 异步可运行对象列表
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

#endif // __SNOTIFYCENTER__H__