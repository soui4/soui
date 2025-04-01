#ifndef __SMSGLOOP_I__H__
#define __SMSGLOOP_I__H__

#include <windows.h>
#include <interface/obj-ref-i.h>
#include <interface/SRunnable-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IMsgFilter
DECLARE_INTERFACE(IMsgFilter)
{
    /**
     * @brief 预处理消息
     * @param pMsg - 消息指针
     * @return BOOL - 如果消息已处理返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, PreTranslateMessage)(THIS_ MSG * pMsg) PURE;
};

#undef INTERFACE
#define INTERFACE IIdleHandler
DECLARE_INTERFACE(IIdleHandler)
{
    /**
     * @brief 空闲时处理
     * @return BOOL - 如果需要继续空闲处理返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, OnIdle)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IMessageLoop
DECLARE_INTERFACE_(IMessageLoop, IObjRef)
{
    //! 添加引用
    /*!
     * @return long - 当前引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //! 释放引用
    /*!
     * @return long - 当前引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //! 释放对象
    /*!
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    // Message filter operations

    /**
     * @brief 添加消息过滤器
     * @param pMessageFilter - 消息过滤器指针
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, AddMessageFilter)(THIS_ IMsgFilter * pMessageFilter) PURE;

    /**
     * @brief 移除消息过滤器
     * @param pMessageFilter - 消息过滤器指针
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, RemoveMessageFilter)(THIS_ IMsgFilter * pMessageFilter) PURE;

    // Idle handler operations

    /**
     * @brief 添加空闲处理器
     * @param pIdleHandler - 空闲处理器指针
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, AddIdleHandler)(THIS_ IIdleHandler * pIdleHandler) PURE;

    /**
     * @brief 移除空闲处理器
     * @param pIdleHandler - 空闲处理器指针
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, RemoveIdleHandler)(THIS_ IIdleHandler * pIdleHandler) PURE;

    // Override to change message filtering

    /**
     * @brief 预处理消息
     * @param pMsg - 消息指针
     * @return BOOL - 如果消息已处理返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, PreTranslateMessage)(THIS_ MSG * pMsg) PURE;

    // Override to change idle processing

    /**
     * @brief 空闲时处理
     * @param nIdleCount - 空闲计数
     * @return BOOL - 如果需要继续空闲处理返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, OnIdle)(THIS_ int nIdleCount) PURE;

    /**
     * @brief 运行消息循环
     * @return int - 消息循环的返回值
     */
    STDMETHOD_(int, Run)(THIS) PURE;

    /**
     * @brief 检查消息循环是否正在运行
     * @return BOOL - 正在运行返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, IsRunning)(CTHIS) SCONST PURE;

    /**
     * @brief 处理消息
     * @param pMsg - 消息指针
     * @return void
     */
    STDMETHOD_(void, OnMsg)(THIS_ LPMSG pMsg) PURE;

    /**
     * @brief 退出消息循环
     * @param exitCode - 退出代码
     * @return void
     */
    STDMETHOD_(void, Quit)(THIS_ int exitCode DEF_VAL(0)) PURE;

    /**
     * @brief 发布任务
     * @param runable - 可运行对象指针
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, PostTask)(THIS_ IRunnable * runable) PURE;

    /**
     * @brief 移除与对象关联的任务
     * @param pObj - 对象指针
     * @return int - 移除的任务数量
     */
    STDMETHOD_(int, RemoveTasksForObject)(THIS_ void *pObj) PURE;

    /**
     * @brief 执行挂起的任务
     * @return void
     */
    STDMETHOD_(void, ExecutePendingTask)(THIS) PURE;

    /**
     * @brief 检查消息队列中是否有消息
     * @param pMsg - 消息指针
     * @param wMsgFilterMin - 最小消息过滤值
     * @param wMsgFilterMax - 最大消息过滤值
     * @param bRemove - 是否移除消息
     * @return BOOL - 有消息返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, PeekMsg)(THIS_ LPMSG pMsg, UINT wMsgFilterMin, UINT wMsgFilterMax, BOOL bRemove) PURE;

    /**
     * @brief 等待消息
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, WaitMsg)(THIS) PURE;

    /**
     * @brief 处理消息
     * @return int - 消息处理的返回值
     */
    STDMETHOD_(int, HandleMsg)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IMsgLoopFactory
DECLARE_INTERFACE_(IMsgLoopFactory, IObjRef)
{
    /**
     * @brief 创建消息循环
     * @param ppMsgLoop - 消息循环指针的指针
     * @param pParentLoop - 父消息循环指针
     * @return HRESULT
     */
    STDMETHOD_(HRESULT, CreateMsgLoop)(THIS_ IMessageLoop * *ppMsgLoop, IMessageLoop * pParentLoop DEF_VAL(NULL)) PURE;
};

SNSEND
#endif // __SMSGLOOP_I__H__
