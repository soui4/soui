// wasm 版 ITaskLoop：不建线程，队列由宿主主循环按帧预算驱动。
//
// 浏览器主线程只有一个，pthread 版的 STaskLoop(TaskLoop.cpp) 在 wasm 下既起不了
// 工作线程、也不能在任务里反过来阻塞 UI。这里把语义改成：
//   - postTask               入队（同优先级 FIFO，高优先级插到低优先级之前）；
//   - postTask(waitUntilDone) 本线程就是"任务线程"，直接内联执行并返回 -1，
//                             与桌面版"从任务线程 send"的分支一致，不会死等；
//   - start/stop             切换 running 标志，stop 丢弃未执行的任务并通知监听器；
//   - 心跳                   由 STaskLoopPump 每次调用时按 GetTickCount 判断到期；
//   - 诊断                   三个计数器供烟测断言"任务真跑过 / 队列会清空 / 实例会释放"。
//
// 队列的消费入口是 STaskLoopPump()，宿主每次主循环调用一次（带时间预算），
// 与 soui-wasm 里 PeekMessage 的 100ms 预算同理：慢任务只拖长一帧，不会冻结页面。

#include <windows.h>
#include "TaskLoopWasm.h"
#include <helper/obj-ref-impl.hpp>
#include <emscripten/emscripten.h>
#include <algorithm>
#include <list>
#include <string>
#include <vector>

SNSBEGIN

namespace
{

struct SWasmTaskItem
{
    SWasmTaskItem(IRunnable *pRunnable, int nPriority)
        : taskID(0)
        , runnable(pRunnable)
        , nPriority(nPriority)
    {
    }

    long taskID;
    SAutoRefPtr<IRunnable> runnable;
    int nPriority;
};

long g_runCount = 0;

} // namespace

class SWasmTaskLoop : public TObjRefImpl<ITaskLoop>
{
  public:
    SWasmTaskLoop()
        : m_running(false)
        , m_hasRunningItem(false)
        , m_nextTaskID(0)
        , m_nHeartBeatInterval(-1)
        , m_tsTick(-1)
        , m_listener(NULL)
    {
    }

    ~SWasmTaskLoop();

    STDMETHOD_(BOOL, getName)(THIS_ char *pszBuf, int nBufLen) OVERRIDE
    {
        if (!pszBuf || nBufLen <= 0)
            return FALSE;
        if (m_strName.length() >= (size_t)nBufLen)
            return FALSE;
        memcpy(pszBuf, m_strName.c_str(), m_strName.length());
        pszBuf[m_strName.length()] = '\0';
        return TRUE;
    }

    STDMETHOD_(void, start)(THIS_ const char *pszName, Priority priority) OVERRIDE
    {
        (void)priority; // 单线程：没有线程优先级可设
        m_items.clear();
        if (pszName)
            m_strName = pszName;
        if (m_running)
            return;
        m_running = true;
        if (m_listener)
            m_listener->onStart(this);
    }

    STDMETHOD_(void, stop)(THIS) OVERRIDE
    {
        if (!m_running)
        {
            m_items.clear();
            return;
        }
        m_running = false;
        // 与桌面版一致：stop 之后队列里的任务不会再执行
        m_items.clear();
        m_hasRunningItem = false;
        m_runningItemInfo.clear();
        if (m_listener)
            m_listener->onStop(this);
    }

    STDMETHOD_(long, postTask)(THIS_ const IRunnable *runnable, BOOL waitUntilDone, int priority) OVERRIDE
    {
        if (!runnable || !m_running)
            return -1;

        SAutoRefPtr<IRunnable> pClone;
        pClone.Attach(runnable->clone());
        if (!pClone)
            return -1;

        if (waitUntilDone)
        {
            // 本线程就是唯一的执行线程，等自己执行完等于死锁，直接内联跑掉
            runItem(pClone, priority);
            g_runCount++;
            return -1;
        }

        SWasmTaskItem item(pClone, priority);
        item.taskID = m_nextTaskID;
        m_nextTaskID = (m_nextTaskID + 1) & 0x7fffffff;

        // 与桌面版排序口径一致：按优先级从高到低，同优先级保持 FIFO
        std::list<SWasmTaskItem>::iterator it = m_items.begin();
        while (it != m_items.end() && it->nPriority >= priority)
            ++it;
        m_items.insert(it, item);
        return item.taskID;
    }

    STDMETHOD_(void, cancelTasksForObject)(THIS_ void *object) OVERRIDE
    {
        if (!object)
            return;
        std::list<SWasmTaskItem>::iterator it = m_items.begin();
        while (it != m_items.end())
        {
            if (it->runnable->getObject() == object)
                it = m_items.erase(it);
            else
                ++it;
        }
        if (m_heartBeatTask && m_heartBeatTask->getObject() == object)
        {
            m_heartBeatTask = NULL;
            m_nHeartBeatInterval = -1;
        }
    }

    STDMETHOD_(BOOL, cancelTask)(THIS_ long taskId) OVERRIDE
    {
        std::list<SWasmTaskItem>::iterator it = m_items.begin();
        while (it != m_items.end())
        {
            if (it->taskID == taskId)
            {
                m_items.erase(it);
                return TRUE;
            }
            ++it;
        }
        return FALSE;
    }

    STDMETHOD_(int, getTaskCount)(CTHIS) SCONST OVERRIDE
    {
        return (int)m_items.size();
    }

    STDMETHOD_(BOOL, isRunning)(THIS) OVERRIDE
    {
        return m_running ? TRUE : FALSE;
    }

    STDMETHOD_(BOOL, getRunningTaskInfo)(THIS_ char *buf, int bufLen) OVERRIDE
    {
        if (!m_hasRunningItem || !buf || bufLen <= 0)
            return FALSE;
        size_t nCopy = m_runningItemInfo.length();
        if (nCopy >= (size_t)bufLen)
            nCopy = (size_t)bufLen - 1;
        memcpy(buf, m_runningItemInfo.c_str(), nCopy);
        buf[nCopy] = '\0';
        return TRUE;
    }

    STDMETHOD_(void, setHeartBeatTask)(THIS_ IRunnable *pTask, int intervel) OVERRIDE
    {
        if (pTask)
        {
            m_nHeartBeatInterval = intervel;
            m_heartBeatTask.Attach(pTask->clone());
            m_tsTick = GetTickCount();
        }
        else
        {
            m_heartBeatTask = NULL;
            m_nHeartBeatInterval = -1;
            m_tsTick = -1;
        }
    }

    STDMETHOD_(void, setListener)(THIS_ ITaskLoopListener *listener) OVERRIDE
    {
        m_listener = listener;
    }

    STDMETHOD_(ITaskLoopListener *, getListener)(THIS) OVERRIDE
    {
        return m_listener;
    }

    /** 把队列跑一段。deadline<=0 表示不设预算。返回本次执行的任务数。 */
    long pump(double deadline)
    {
        if (!m_running)
            return 0;

        long nRan = 0;
        while (m_running)
        {
            if (m_heartBeatTask && m_nHeartBeatInterval >= 0)
            {
                unsigned int tsNow = GetTickCount();
                if (tsNow - m_tsTick >= (unsigned int)m_nHeartBeatInterval)
                {
                    m_tsTick = tsNow;
                    m_heartBeatTask->run();
                    nRan++;
                    g_runCount++;
                }
            }

            if (m_items.empty())
                break;

            SWasmTaskItem item = m_items.front();
            m_items.pop_front();
            runItem(item.runnable, item.nPriority);
            nRan++;
            g_runCount++;

            if (deadline > 0 && emscripten_get_now() >= deadline)
                break;
        }
        return nRan;
    }

  private:
    void runItem(IRunnable *pRunnable, int nPriority)
    {
        (void)nPriority;
        m_hasRunningItem = true;
        const char *pszInfo = pRunnable->getClassInfo();
        m_runningItemInfo = pszInfo ? pszInfo : "";
        pRunnable->run();
        m_hasRunningItem = false;
        m_runningItemInfo.clear();
    }

    std::string m_strName;
    std::list<SWasmTaskItem> m_items;
    bool m_running;

    bool m_hasRunningItem;
    std::string m_runningItemInfo;
    long m_nextTaskID;

    SAutoRefPtr<IRunnable> m_heartBeatTask;
    int m_nHeartBeatInterval;
    unsigned int m_tsTick;

    ITaskLoopListener *m_listener;
};

namespace
{

// 单线程环境：实例注册表只在主线程被读写，不需要锁。
std::vector<SWasmTaskLoop *> &loopRegistry()
{
    static std::vector<SWasmTaskLoop *> s_loops;
    return s_loops;
}

} // namespace

SWasmTaskLoop::~SWasmTaskLoop()
{
    stop();
    std::vector<SWasmTaskLoop *> &loops = loopRegistry();
    loops.erase(std::remove(loops.begin(), loops.end(), this), loops.end());
}

int STaskLoopPump(double budgetMs)
{
    std::vector<SAutoRefPtr<SWasmTaskLoop> > loops;
    loops.reserve(loopRegistry().size());
    for (size_t i = 0; i < loopRegistry().size(); ++i)
        loops.push_back(loopRegistry()[i]); // AddRef：泵的过程中有任务可能 stop/Release

    const double deadline = budgetMs > 0 ? emscripten_get_now() + budgetMs : 0;
    for (size_t i = 0; i < loops.size(); ++i)
    {
        loops[i]->pump(deadline);
        if (deadline > 0 && emscripten_get_now() >= deadline)
            break;
    }

    int nPending = 0;
    for (size_t i = 0; i < loops.size(); ++i)
        nPending += loops[i]->getTaskCount();
    return nPending;
}

long STaskLoopRunCount()
{
    return g_runCount;
}

int STaskLoopPendingCount()
{
    int nPending = 0;
    std::vector<SWasmTaskLoop *> &loops = loopRegistry();
    for (size_t i = 0; i < loops.size(); ++i)
        nPending += loops[i]->getTaskCount();
    return nPending;
}

int STaskLoopLiveCount()
{
    return (int)loopRegistry().size();
}

namespace TASKLOOP
{
SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef **ppTaskLoop)
{
    SWasmTaskLoop *pLoop = new SWasmTaskLoop();
    loopRegistry().push_back(pLoop);
    // 注册表是裸指针：Release 到 0 时必须摘掉，否则泵会摸到已释放的对象。
    *ppTaskLoop = pLoop;
    return TRUE;
}
} // namespace TASKLOOP

SNSEND

EXTERN_C BOOL TaskLoop_SCreateInstance(IObjRef **ppTaskLoop)
{
    return SNS::TASKLOOP::SCreateInstance(ppTaskLoop);
}
