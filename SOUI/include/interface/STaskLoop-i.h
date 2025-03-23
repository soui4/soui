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

#undef INTERFACE
#define INTERFACE ITaskLoop
DECLARE_INTERFACE_(ITaskLoop, IObjRef)
{
    /**
     * @brief 添加引用
     * @return long -- 引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 释放引用
     * @return long -- 引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief 获取任务队列的名称
     * @param pszBuf char* -- 缓冲区
     * @param nBufLen int -- 缓冲区长度
     * @return BOOL -- TRUE: 成功，FALSE: 失败
     */
    STDMETHOD_(BOOL, getName)(THIS_ char *pszBuf, int nBufLen) PURE;

    /**
     * @brief 启动一个线程运行任务队列
     * @param pszName const char* -- 线程名称
     * @param priority Priority -- 线程优先级
     * @return void
     */
    STDMETHOD_(void, start)(THIS_ const char *pszName, Priority priority) PURE;

    /**
     * @brief 停止线程同步
     * @return void
     */
    STDMETHOD_(void, stop)(THIS) PURE;

    /**
     * @brief 向任务管理器发布或发送任务
     * @param runnable const IRunnable* -- 要运行的任务对象
     * @param waitUntilDone BOOL -- TRUE: 发送任务，FALSE: 发布任务
     * @param priority int -- 任务优先级
     * @return long -- 任务ID，可用于取消任务
     */
    STDMETHOD_(long, postTask)
    (THIS_ const IRunnable *runnable, BOOL waitUntilDone, int priority) PURE;

    /**
     * @brief 从任务循环待处理任务列表中移除特定对象的任务
     * @param object void* -- 要移除任务的特定对象
     * @return void
     */
    STDMETHOD_(void, cancelTasksForObject)(THIS_ void *object) PURE;

    /**
     * @brief 取消特定任务ID列表中的任务
     * @param taskId long -- 要取消的任务ID
     * @return BOOL -- TRUE: 成功，FALSE: 失败
     */
    STDMETHOD_(BOOL, cancelTask)(THIS_ long taskId) PURE;

    /**
     * @brief 获取任务循环队列中的总任务数
     * @return int -- 任务循环队列中的总任务数
     */
    STDMETHOD_(int, getTaskCount)(CTHIS) SCONST PURE;

    /**
     * @brief 获取运行循环的状态
     * @return BOOL -- TRUE: 运行中，FALSE: 未运行
     */
    STDMETHOD_(BOOL, isRunning)(THIS) PURE;

    /**
     * @brief 获取正在运行的任务信息
     * @param buf char* -- 接收任务信息的缓冲区
     * @param bufLen int -- 缓冲区长度
     * @return BOOL -- TRUE: 成功，FALSE: 无任务运行
     */
    STDMETHOD_(BOOL, getRunningTaskInfo)(THIS_ char *buf, int bufLen) PURE;

    /**
     * @brief 设置一个任务重复运行
     * @param pTask IRunnable* -- 要运行的任务对象
     * @param intervel int -- 心跳间隔
     * @return void
     * @remark 任务循环将持有pTask的克隆。将pTask设置为null以停止心跳。
     */
    STDMETHOD_(void, setHeartBeatTask)(THIS_ IRunnable * pTask, int intervel) PURE;
};

SNSEND
#endif // __STASKLOOP_I__H__


