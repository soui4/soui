#pragma once
#include <utilities-def.h>
#include <interface/SEvtArgs-i.h>
#include <interface/STaskLoop-i.h>

SNSBEGIN

typedef void (*FunRunOnUI)(WPARAM wp,LPARAM lp);

#undef INTERFACE
#define INTERFACE INotifyCenter
DECLARE_INTERFACE(INotifyCenter)
{
    /**
     * FireEventSync
     * @brief    触发一个同步通知事件
     * @param    EventArgs *e -- 事件对象
     * @return
     *
     * Describe  只能在UI线程中调用
     */
    STDMETHOD_(void,FireEventSync)(THIS_ IEvtArgs *e) PURE;

    /**
     * FireEventAsync
     * @brief    触发一个异步通知事件
     * @param    EventArgs *e -- 事件对象
     * @return
     *
     * Describe  可以在非UI线程中调用，EventArgs
     * *e必须是从堆上分配的内存，调用后使用Release释放引用计数
     */
    STDMETHOD_(void,FireEventAsync)(THIS_ IEvtArgs *e) PURE;

    /**
     * RegisterEventMap
     * @brief    注册一个处理通知的对象
     * @param    const ISlotFunctor *slot -- 事件处理对象
     * @return
     *
     * Describe
     */
    STDMETHOD_(BOOL, RegisterEventMap)(THIS_ const IEvtSlot *slot) PURE;

    /**
     * RegisterEventMap
     * @brief    注销一个处理通知的对象
     * @param    const ISlotFunctor *slot -- 事件处理对象
     * @return
     *
     * Describe
     */
    STDMETHOD_(BOOL, UnregisterEventMap)(THIS_ const IEvtSlot *slot) PURE;

    /**
     * RunOnUI
     * @brief    
     * @param    IRunnable * pRunnable -- runnable执行体
	 * @param    BOOL bSync -- 同步执行标志
     * @return
     *
     * Describe
     */
	STDMETHOD_(void,RunOnUI)(THIS_ IRunnable * pRunnable,BOOL bSync) PURE;

    /**
     * RunOnUI2
     * @brief    
     * @param    FunRunOnUI fun -- callback function
	 * @param    WPARAM wp -- param 1
	 * @param    LPARAM wp -- param 2
	 * @param    BOOL bSync -- 同步执行标志
     * @return
     *
     * Describe
     */
	STDMETHOD_(void,RunOnUI2)(THIS_ FunRunOnUI fun, WPARAM wp, LPARAM lp,BOOL bSync) PURE;

};

SNSEND