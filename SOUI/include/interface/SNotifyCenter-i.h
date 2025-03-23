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
     * @brief 触发一个同步通知事件
     * @param e IEvtArgs* -- 事件对象
     * @return void
     * @remark 只能在UI线程中调用
     */
    STDMETHOD_(void, FireEventSync)(THIS_ IEvtArgs * e) PURE;

    /**
     * @brief 触发一个异步通知事件
     * @param e IEvtArgs* -- 事件对象
     * @return void
     * @remark 可以在非UI线程中调用，IEvtArgs *e 必须是从堆上分配的内存，调用后使用 Release 释放引用计数
     */
    STDMETHOD_(void, FireEventAsync)(THIS_ IEvtArgs * e) PURE;

    /**
     * @brief 注册一个处理通知的对象
     * @param slot const IEvtSlot* -- 事件处理对象
     * @return BOOL
     */
    STDMETHOD_(BOOL, RegisterEventMap)(THIS_ const IEvtSlot *slot) PURE;

    /**
     * @brief 注销一个处理通知的对象
     * @param slot const IEvtSlot* -- 事件处理对象
     * @return BOOL
     */
    STDMETHOD_(BOOL, UnregisterEventMap)(THIS_ const IEvtSlot *slot) PURE;

    /**
     * @brief 在UI线程中执行一个任务
     * @param pRunnable IRunnable* -- runnable执行体
     * @param bSync BOOL -- 同步执行标志
     * @return void
     */
    STDMETHOD_(void, RunOnUI)(THIS_ IRunnable * pRunnable, BOOL bSync) PURE;

    /**
     * @brief 在UI线程中执行一个任务
     * @param fun FunRunOnUI -- callback function
     * @param wp WPARAM -- param 1
     * @param lp LPARAM -- param 2
     * @param bSync BOOL -- 同步执行标志
     * @return void
     */
    STDMETHOD_(void, RunOnUI2)(THIS_ FunRunOnUI fun, WPARAM wp, LPARAM lp, BOOL bSync) PURE;
};

SNSEND
#endif // __SNOTIFYCENTER_I__H__