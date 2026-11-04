// wasm 版 ITaskLoop：两条后端，按构建能力选定，对上保持同一套 ITaskLoop 语义。
//
//   泵后端（默认）：不建线程，队列由宿主主循环按帧预算驱动（STaskLoopPump）。
//     浏览器主线程只有一个，任务再慢也只能拖长一帧，不会把页面冻死。
//   worker 后端（-pthread）：start() 建一条 wasm worker 线程，队列由它消费。
//     UI 线程只负责入队 + Atomics 唤醒，不等待 worker —— 浏览器主线程没有
//     Atomics.wait，Emscripten 只能用忙等模拟（见 emscripten_futex_wait.c 的注释），
//     忙等期间 DOM 事件不再派发，所以"UI 等 worker"在这个平台上不是不划算，是不可用。
//
// worker 后端的同步不用 SSemaphore：那份实现落在 swinx posix 的
// CreateSemaphore/WaitForSingleObject 上，即 pipe + select，而阻塞 poll/select
// 在 Emscripten worker 里永不返回（实测）。这里改用线性内存上的一个 32 位待办计数
// 做 futex 门：worker 睡在 emscripten_futex_wait（= Atomics.wait），UI 入队后
// emscripten_futex_wake（= Atomics.notify）叫醒它。
//
// 门只能按"待办条数"计数，不能做成布尔标志：两次入队都发生在 worker 睡着之前时，
// 布尔门会把两个任务合并成一个（实测复现过：seq=3 而只跑了 2 个任务）。
//
// 结果回抛不在本模块：应用侧用 IMessageLoop::PostTask 切回 UI 线程
// （SOUI/src/core/SMsgLoop.cpp 里那条队列本来就有 SCriticalSection 保护），
// 宿主每帧调一次 ExecutePendingTask。见 games/cnchess/client/wasm/cnchess_main.cpp。

#include <windows.h>
#include "TaskLoopWasm.h"
#include <helper/obj-ref-impl.hpp>
#include <helper/SCriticalSection.h>
#include <emscripten/emscripten.h>
#include <emscripten/atomic.h>
#include <emscripten/threading_primitives.h>
#include <algorithm>
#include <climits>
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

// 两条后端共用这两个计数，worker 线程与 UI 线程会同时递增（worker 消费队列，内联的
// waitUntilDone=TRUE 由调用线程跑掉），所以一律走 tlAtomicAdd，不用裸 ++：烟测要拿
// 精确的执行数做判据，丢一次递增就是一条红。
// g_workerRunCount 只在 workerLoop 里加，它是"任务真的离开 UI 线程"的直接证据；
// 不带 -pthread 的构建永远读到 0。
volatile unsigned int g_runCount = 0;
volatile unsigned int g_workerRunCount = 0;

// worker 栈按 4 MB 单独给，不跟 -sSTACK_SIZE 走。STACK_SIZE 是浏览器主线程的栈，
// wasm 端口把它开到 32 MB 是因为布局/测量递归深；任务线程跑的是解压、AI 这类
// 迭代型工作，栈需求低得多。pthread 的栈从堆里分配，每条 worker 都按 STACK_SIZE
// 取的话多开几个 taskloop 就把线性内存吃光了。
const size_t kWasmTaskLoopStack = 4 * 1024 * 1024;

// 门的三个 32 位量只经这组包装读写。emscripten_atomic_* 收 void*，而它们是
// volatile 成员，把转型集中在一处比满文件撒 (void*) 好读；带 -pthread 时
// 这些调用编成 i32.atomic.*，futex 判的就是同一份内存。
unsigned int tlAtomicLoad(volatile unsigned int *p)
{
    return emscripten_atomic_load_u32((const void *)p);
}

void tlAtomicStore(volatile unsigned int *p, unsigned int nVal)
{
    emscripten_atomic_store_u32((void *)p, nVal);
}

unsigned int tlAtomicAdd(volatile unsigned int *p, unsigned int nVal)
{
    return emscripten_atomic_add_u32((void *)p, nVal);
}

unsigned int tlAtomicSub(volatile unsigned int *p, unsigned int nVal)
{
    return emscripten_atomic_sub_u32((void *)p, nVal);
}

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
        , m_pending(0)
        , m_stopping(0)
        , m_workerLive(0)
        , m_workerHandle(0)
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
        (void)priority; // 线程优先级在 wasm 上没有对应物，只有名字能用
        {
            SAutoLock lock(m_cs);
            m_items.clear();
            tlAtomicStore(&m_pending, 0);
            tlAtomicStore(&m_stopping, 0);
            if (pszName)
                m_strName = pszName;
        }
        if (m_running)
            return;
        m_running = true;
        // 起不来 worker 就留在泵后端，任务照跑，只是仍在 UI 线程上。
        startWorker();
        if (m_listener)
            m_listener->onStart(this);
    }

    STDMETHOD_(void, stop)(THIS) OVERRIDE
    {
        {
            SAutoLock lock(m_cs);
            if (!m_running)
            {
                m_items.clear();
                tlAtomicStore(&m_pending, 0);
                return;
            }
            m_running = false;
            // 与桌面版一致：stop 之后队列里的任务不会再执行
            m_items.clear();
            tlAtomicStore(&m_pending, 0);
            tlAtomicStore(&m_stopping, 1);
            m_hasRunningItem = false;
            m_runningItemInfo.clear();
            m_heartBeatTask = NULL;
            m_nHeartBeatInterval = -1;
            m_tsTick = -1;
        }
        // 回收 worker 必须在这里做完：析构之后还在跑的线程会摸到已释放的 this。
        joinWorker();
        if (m_listener)
            m_listener->onStop(this);
    }

    STDMETHOD_(long, postTask)(THIS_ const IRunnable *runnable, BOOL waitUntilDone, int priority) OVERRIDE
    {
        if (!runnable)
            return -1;

        SAutoRefPtr<IRunnable> pClone;
        pClone.Attach(runnable->clone());
        if (!pClone)
            return -1;

        if (waitUntilDone)
        {
            // send 语义要等执行完，而浏览器主线程等 worker 等于冻住整个标签页。
            // 与桌面版"从任务线程 send"的分支一致：本线程就是执行线程，直接内联跑掉。
            if (!m_running)
                return -1;
            runItem(pClone, priority);
            tlAtomicAdd(&g_runCount, 1);
            return -1;
        }

        unsigned int nOldPending = 0;
        long nTaskID = 0;
        {
            SAutoLock lock(m_cs);
            if (!m_running)
                return -1;

            SWasmTaskItem item(pClone, priority);
            item.taskID = m_nextTaskID;
            nTaskID = m_nextTaskID;
            m_nextTaskID = (m_nextTaskID + 1) & 0x7fffffff;

            // 与桌面版排序口径一致：按优先级从高到低，同优先级保持 FIFO
            std::list<SWasmTaskItem>::iterator it = m_items.begin();
            while (it != m_items.end() && it->nPriority >= priority)
                ++it;
            m_items.insert(it, item);
            // 入队和待办计数在同一个锁里变更：worker 判空与睡下去之间不会插入。
            nOldPending = tlAtomicAdd(&m_pending, 1);
        }
        // 锁外再叫醒：只在队列由空变非空时唤醒，其余情况 worker 本来就在跑。
        if (nOldPending == 0)
            wakeWorker();
        return nTaskID;
    }

    STDMETHOD_(void, cancelTasksForObject)(THIS_ void *object) OVERRIDE
    {
        if (!object)
            return;
        SAutoLock lock(m_cs);
        std::list<SWasmTaskItem>::iterator it = m_items.begin();
        while (it != m_items.end())
        {
            if (it->runnable->getObject() == object)
            {
                it = m_items.erase(it);
                tlAtomicSub(&m_pending, 1);
            }
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
        SAutoLock lock(m_cs);
        std::list<SWasmTaskItem>::iterator it = m_items.begin();
        while (it != m_items.end())
        {
            if (it->taskID == taskId)
            {
                m_items.erase(it);
                tlAtomicSub(&m_pending, 1);
                return TRUE;
            }
            ++it;
        }
        return FALSE;
    }

    STDMETHOD_(int, getTaskCount)(CTHIS) SCONST OVERRIDE
    {
        SAutoLock lock(m_cs);
        return (int)m_items.size();
    }

    STDMETHOD_(BOOL, isRunning)(THIS) OVERRIDE
    {
        return m_running ? TRUE : FALSE;
    }

    STDMETHOD_(BOOL, getRunningTaskInfo)(THIS_ char *buf, int bufLen) OVERRIDE
    {
        if (!buf || bufLen <= 0)
            return FALSE;
        std::string info;
        {
            SAutoLock lock(m_cs);
            if (!m_hasRunningItem)
                return FALSE;
            info = m_runningItemInfo;
        }
        size_t nCopy = info.length();
        if (nCopy >= (size_t)bufLen)
            nCopy = (size_t)bufLen - 1;
        memcpy(buf, info.c_str(), nCopy);
        buf[nCopy] = '\0';
        return TRUE;
    }

    STDMETHOD_(void, setHeartBeatTask)(THIS_ IRunnable *pTask, int intervel) OVERRIDE
    {
        SAutoLock lock(m_cs);
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

    /** 泵后端专用：把队列跑一段。deadline<=0 表示不设预算。返回本次执行的任务数。 */
    long pump(double deadline)
    {
        // worker 后端时队列归那条线程，这里再跑一遍就是重复执行。
        if (!m_running || tlAtomicLoad(&m_workerLive))
            return 0;

        long nRan = 0;
        while (m_running)
        {
            SAutoRefPtr<IRunnable> pHeartBeat;
            bool bBeatDue = false;
            {
                SAutoLock lock(m_cs);
                pHeartBeat = m_heartBeatTask;
                if (pHeartBeat && m_nHeartBeatInterval >= 0)
                {
                    unsigned int tsNow = GetTickCount();
                    if (tsNow - m_tsTick >= (unsigned int)m_nHeartBeatInterval)
                    {
                        m_tsTick = tsNow;
                        bBeatDue = true;
                    }
                }
            }
            if (bBeatDue)
            {
                // 跑在锁外：心跳任务里 postTask 回本循环是常见写法。
                pHeartBeat->run();
                nRan++;
                tlAtomicAdd(&g_runCount, 1);
            }

            SWasmTaskItem item(NULL, 0);
            {
                SAutoLock lock(m_cs);
                if (m_items.empty())
                    break;
                item = m_items.front();
                m_items.pop_front();
                tlAtomicSub(&m_pending, 1);
            }
            runItem(item.runnable, item.nPriority);
            nRan++;
            tlAtomicAdd(&g_runCount, 1);

            if (deadline > 0 && emscripten_get_now() >= deadline)
                break;
        }
        return nRan;
    }

    /** worker 后端是否真的在跑（诊断用：泵后端为 0） */
    bool hasWorker()
    {
        return tlAtomicLoad(&m_workerLive) != 0;
    }

  private:
    void runItem(IRunnable *pRunnable, int nPriority)
    {
        (void)nPriority;
        {
            SAutoLock lock(m_cs);
            m_hasRunningItem = true;
            const char *pszInfo = pRunnable->getClassInfo();
            m_runningItemInfo = pszInfo ? pszInfo : "";
        }
        pRunnable->run();
        {
            SAutoLock lock(m_cs);
            m_hasRunningItem = false;
            m_runningItemInfo.clear();
        }
    }

    // ---- 后端收口点：非 pthread 构建里这三个入口全部退化成空操作，
    //      上面的 start/stop/postTask/pump 正文因此不带任何平台分支。
    bool startWorker();
    void wakeWorker();
    void joinWorker();
    void workerLoop();
    static void *workerThreadProc(void *pParam);

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

    // m_pending/m_stopping/m_workerLive 是跨线程读的，只经 emscripten_atomic_* 存取；
    // m_pending 同时是 futex 的等待地址，必须是 4 字节对齐的 32 位量。
    volatile unsigned int m_pending;
    volatile unsigned int m_stopping;
    volatile unsigned int m_workerLive;
    unsigned long m_workerHandle;

    mutable SCriticalSection m_cs;
};

//-----------------------------------------------------------------------------
// worker 后端。整块收在一个 #ifdef 里：没有 -pthread 时这三个入口是空操作，
// 实例自动留在泵后端，行为与改动前一致。
//-----------------------------------------------------------------------------
#ifdef __EMSCRIPTEN_PTHREADS__

#include <pthread.h>

void *SWasmTaskLoop::workerThreadProc(void *pParam)
{
    static_cast<SWasmTaskLoop *>(pParam)->workerLoop();
    return NULL;
}

bool SWasmTaskLoop::startWorker()
{
    if (m_workerHandle)
        return true;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, kWasmTaskLoopStack);
    pthread_t handle;
    int nErr = pthread_create(&handle, &attr, workerThreadProc, this);
    pthread_attr_destroy(&attr);
    if (nErr != 0)
        return false;

    m_workerHandle = (unsigned long)handle;
    tlAtomicStore(&m_workerLive, 1);
    return true;
}

void SWasmTaskLoop::wakeWorker()
{
    if (!tlAtomicLoad(&m_workerLive))
        return;
    emscripten_futex_wake((volatile void *)&m_pending, INT_MAX);
}

void SWasmTaskLoop::joinWorker()
{
    if (!m_workerHandle)
        return;

    emscripten_futex_wake((volatile void *)&m_pending, INT_MAX);

    // 任务里 Release 掉最后一条引用的话，这里就是"自己 join 自己"。
    // pthread_join 会返回 EDEADLK，处理办法是 detach：线程还在收尾，但对象
    // 已经不由它负责了 —— 真出现这种写法说明调用方的引用模型有问题，
    // 至少不要让它变成静默死锁。
    pthread_t handle = (pthread_t)m_workerHandle;
    if (pthread_equal(pthread_self(), handle))
        pthread_detach(handle);
    else
        pthread_join(handle, NULL);

    m_workerHandle = 0;
    tlAtomicStore(&m_workerLive, 0);
}

void SWasmTaskLoop::workerLoop()
{
    for (;;)
    {
        SWasmTaskItem item(NULL, 0);
        bool bGot = false;
        {
            SAutoLock lock(m_cs);
            if (!m_items.empty())
            {
                item = m_items.front();
                m_items.pop_front();
                tlAtomicSub(&m_pending, 1);
                bGot = true;
            }
        }
        if (bGot)
        {
            runItem(item.runnable, item.nPriority);
            tlAtomicAdd(&g_runCount, 1);
            // 队列由 worker 线程消费：这两条计数同时增长，差值就是"跑在 UI 线程上"的任务数。
            tlAtomicAdd(&g_workerRunCount, 1);
            continue;
        }

        if (tlAtomicLoad(&m_stopping) || !m_running)
            break;

        // 心跳：桌面版跑在任务线程上，这里保持同一口径。
        double waitMs = -1; // -1 = 无限等
        SAutoRefPtr<IRunnable> pHeartBeat;
        {
            SAutoLock lock(m_cs);
            pHeartBeat = m_heartBeatTask;
            if (pHeartBeat && m_nHeartBeatInterval >= 0)
            {
                unsigned int tsNow = GetTickCount();
                unsigned int tsDelta = tsNow - m_tsTick;
                if ((int)tsDelta >= m_nHeartBeatInterval)
                {
                    m_tsTick = tsNow;
                    waitMs = 0;
                }
                else
                    waitMs = (double)(m_nHeartBeatInterval - (int)tsDelta);
            }
        }
        if (waitMs == 0)
        {
            pHeartBeat->run();
            tlAtomicAdd(&g_runCount, 1);
            tlAtomicAdd(&g_workerRunCount, 1);
            continue;
        }

        // 睡下去前重新读一次待办数：入队方只在 0->1 那一次叫醒，
        // 若此处已非 0 说明有新任务在判空之后插进来了，futex_wait 会立刻
        // 返回 -EWOULDBLOCK，不会漏任务。
        unsigned int nExpect = tlAtomicLoad(&m_pending);
        if (nExpect != 0)
            continue;
        emscripten_futex_wait((volatile void *)&m_pending, 0, waitMs);
    }
}

#else // !__EMSCRIPTEN_PTHREADS__

bool SWasmTaskLoop::startWorker()
{
    return false;
}

void SWasmTaskLoop::wakeWorker()
{
}

void SWasmTaskLoop::joinWorker()
{
}

void SWasmTaskLoop::workerLoop()
{
}

#endif // __EMSCRIPTEN_PTHREADS__

namespace
{

// 实例注册表：worker 后端下 stop()/Release 可能发生在任务线程里，
// 泵又在 UI 线程遍历，所以这张表必须加锁（泵先整表摘下来再逐个驱动，
// 避免带着表的锁去跑任务）。
std::vector<SWasmTaskLoop *> &loopRegistry()
{
    static std::vector<SWasmTaskLoop *> s_loops;
    return s_loops;
}

SCriticalSection &registryLock()
{
    static SCriticalSection s_cs;
    return s_cs;
}

} // namespace

SWasmTaskLoop::~SWasmTaskLoop()
{
    stop();
    SAutoLock lock(registryLock());
    std::vector<SWasmTaskLoop *> &loops = loopRegistry();
    loops.erase(std::remove(loops.begin(), loops.end(), this), loops.end());
}

int STaskLoopPump(double budgetMs)
{
    std::vector<SAutoRefPtr<SWasmTaskLoop> > loops;
    {
        SAutoLock lock(registryLock());
        loops.reserve(loopRegistry().size());
        for (size_t i = 0; i < loopRegistry().size(); ++i)
            loops.push_back(loopRegistry()[i]); // AddRef：泵的过程中有任务可能 stop/Release
    }

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
    return (long)tlAtomicLoad(&g_runCount);
}

long STaskLoopWorkerRunCount()
{
    return (long)tlAtomicLoad(&g_workerRunCount);
}

int STaskLoopPendingCount()
{
    int nPending = 0;
    SAutoLock lock(registryLock());
    std::vector<SWasmTaskLoop *> &loops = loopRegistry();
    for (size_t i = 0; i < loops.size(); ++i)
        nPending += loops[i]->getTaskCount();
    return nPending;
}

int STaskLoopLiveCount()
{
    SAutoLock lock(registryLock());
    return (int)loopRegistry().size();
}

int STaskLoopWorkerCount()
{
    int nWorkers = 0;
    SAutoLock lock(registryLock());
    std::vector<SWasmTaskLoop *> &loops = loopRegistry();
    for (size_t i = 0; i < loops.size(); ++i)
    {
        if (loops[i]->hasWorker())
            nWorkers++;
    }
    return nWorkers;
}

namespace TASKLOOP
{
SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef **ppTaskLoop)
{
    SWasmTaskLoop *pLoop = new SWasmTaskLoop();
    {
        SAutoLock lock(registryLock());
        loopRegistry().push_back(pLoop);
    }
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
