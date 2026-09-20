#include "TimerGenerator.h"
#include <helper/SFunctor.hpp>
#include <helper/slog.h>

#define kLogTag "TimerGenerator"

SNSBEGIN

CTimerGenerator::CTimerGenerator()
    : m_pListener(NULL)
    , m_bStop(false) // 允许在 start() 之前登记定时器
    , m_uNextID(1)
{
}

CTimerGenerator::~CTimerGenerator()
{
    stop();
}

void CTimerGenerator::stop()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_bStop = true;
        // 摘掉响应器: 队列里可能还残留已收集但未分派的到期事件, 它们不应再回调业务层
        m_pListener = NULL;
    }
    m_cv.notify_all();
    // 不在定时器线程自身里 join (否则自连接死锁); 其它线程调用时正常 join
    if (m_thread.joinable() && std::this_thread::get_id() != m_thread.get_id())
        m_thread.join();
}

BOOL CTimerGenerator::start(ITimerListener *pListener)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_thread.joinable())
        return FALSE; // 已在运行, 不允许重复启动
    m_pListener = pListener;
    m_bStop = false; // 允许 stop() 之后再次 start(): 重置退出标志, 否则新线程会立刻退出
    m_thread = std::thread(&CTimerGenerator::ThreadProc, this);
    return TRUE;
}

UINT_PTR CTimerGenerator::setTimer(UINT_PTR uTimerID, uint32_t uIntervalMs, BOOL bRepeat)
{
    if (uIntervalMs == 0)
    {
        // 周期为0语义上等于"移除该定时器"(与 SetTimer(id,0) 一致); 自动分配ID(uTimerID==0)无对象可移除
        if (uTimerID)
            killTimer(uTimerID);
        return 0;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_bStop)
        return 0; // 发生器已停止: 不再接受新定时器
    if (uTimerID == 0)
    {
        // 自动分配一个未被占用的ID(与Win32 SetTimer(id=0)语义一致)
        while (m_timers.find(m_uNextID) != m_timers.end())
            ++m_uNextID;
        uTimerID = m_uNextID;
        ++m_uNextID;
    }
    TimerEntry &timer = m_timers[uTimerID];
    timer.due = std::chrono::steady_clock::now() + std::chrono::milliseconds(uIntervalMs);
    timer.uPeriodMs = uIntervalMs;
    timer.bRepeat = bRepeat;
    // 新增/改期可能比当前最早到期项更早, 唤醒定时器线程重算等待时刻
    m_cv.notify_all();
    return uTimerID;
}

BOOL CTimerGenerator::killTimer(UINT_PTR uTimerID)
{
    if (!uTimerID)
        return FALSE;
    std::lock_guard<std::mutex> lock(m_mutex);
    BOOL bRemoved = m_timers.erase(uTimerID) > 0;
    if (bRemoved)
        m_cv.notify_all();
    return bRemoved;
}

void CTimerGenerator::ThreadProc()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    // 线程启动瞬间: 把所有已登记定时器的首个到期时刻重基到 "now + period",
    // 这样在 start() 之前登记的定时器, 其首次触发也从服务器真正起来时算起。
    {
        auto now = std::chrono::steady_clock::now();
        for (auto &it : m_timers)
            it.second.due = now + std::chrono::milliseconds(it.second.uPeriodMs);
    }

    for (;;)
    {
        if (m_bStop)
            break;
        if (m_timers.empty())
        {
            m_cv.wait(lock);
            continue;
        }

        auto now = std::chrono::steady_clock::now();
        auto earliest = m_timers.begin()->second.due;
        for (const auto &it : m_timers)
        {
            if (it.second.due < earliest)
                earliest = it.second.due;
        }
        if (earliest > now)
        {
            m_cv.wait_until(lock, earliest);
            continue; // 醒来后重新判断: 可能是新增/删除定时器, 也可能只是到期
        }

        // 收集所有已到期定时器并推进其下次到期, 全部在锁内完成 —— 包括捕获响应器指针,
        // 以避免与 stop() 并发写 m_pListener 形成数据竞争(锁外再读会有 race)。
        // 一次性定时器在收集阶段就摘除, 故 onTimer 期间它已不在调度表里, 不会被重复触发。
        ITimerListener *pListener = m_pListener;
        std::vector<UINT_PTR> fired;
        for (auto it = m_timers.begin(); it != m_timers.end(); )
        {
            if (it->second.due > now)
                continue;

            if (it->second.bRepeat)
            {
                // 周期性: 推进下次到期。若已错过一个以上周期(宿主挂起/线程被长时间抢占),
                // 不做"补课" —— 否则会在极短时间内连续触发, 对"周期性广播"这类用途不可接受。
                std::chrono::milliseconds period(it->second.uPeriodMs);
                auto next = it->second.due + period;
                if (next <= now)
                {
                    SLOGW() << "service timer " << it->first << " overdue, resync to now + "
                            << it->second.uPeriodMs << "ms";
                    next = now + period;
                }
                it->second.due = next;
                ++it;
            }
            else
            {
                // 一次性: 记录ID后立刻摘除, 触发一次即结束
                fired.push_back(it->first);
                it = m_timers.erase(it);
            }
        }

        lock.unlock();
        if (pListener) {
            for (size_t i = 0; i < fired.size(); ++i)
            {
                pListener->onTimer(fired[i]);
            }
        }
        lock.lock();
    }
}

SNSEND
