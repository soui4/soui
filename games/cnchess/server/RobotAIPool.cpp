#include "stdafx.h"
#include "RobotAIPool.h"
#include <ChessAI.h>
#include <TaskLoop/TaskLoop.h>
#include <helper/SFunctor.hpp>
#include <helper/slog.h>
#include <stdio.h>
#define kLogTag "RobotAIPool"
SNSBEGIN

template<> CRobotAIPool * SSingleton<CRobotAIPool>::ms_Singleton = NULL;

CRobotAIPool::CRobotAIPool()
{
}

CRobotAIPool::~CRobotAIPool()
{
    Shutdown();
}

bool CRobotAIPool::Init(int nThreads)
{
    if (nThreads < 1) nThreads = 1;
    if (nThreads > 16) nThreads = 16;
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_taskLoops.empty())
        return true; // already initialized
    for (int i = 0; i < nThreads; ++i)
    {
        SAutoRefPtr<ITaskLoop> taskLoop;
        if (!TASKLOOP::SCreateInstance((IObjRef **)&taskLoop))
        {
            SLOGE() << "create ITaskLoop failed, index=" << i;
            return false;
        }
        char szName[32];
        sprintf(szName, "RobotAI-%d", i);
        taskLoop->start(szName, Priority::Normal);
        m_taskLoops.push_back(taskLoop);
    }
    SLOGI() << "robot AI pool initialized, threads=" << (int)m_taskLoops.size();
    return !m_taskLoops.empty();
}

void CRobotAIPool::Shutdown()
{
    std::vector<SAutoRefPtr<ITaskLoop> > loops;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        loops.swap(m_taskLoops);
        m_serviceSink = ServiceSink();
        m_applySink = ApplySink();
    }
    for (size_t i = 0; i < loops.size(); ++i)
        loops[i]->stop();
    SLOGI() << "robot AI pool shutdown";
}

void CRobotAIPool::SetSink(const ServiceSink &service, const ApplySink &apply)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_serviceSink = service;
    m_applySink = apply;
}

bool CRobotAIPool::Dispatch(const SRobotTask &task)
{
    // pick the task loop with the fewest queued(waiting) tasks for balancing
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_taskLoops.empty())
        return false;
    int nBest = 0, nMin = m_taskLoops[0]->getTaskCount();
    for (size_t i = 1; i < m_taskLoops.size(); ++i)
    {
        int nCount = m_taskLoops[i]->getTaskCount();
        if (nCount < nMin)
        {
            nMin = nCount;
            nBest = (int)i;
        }
    }
    // post a copy to the pool: search only, never touches live game state
    SRobotTask cloneTask = task;
    ITaskLoop *pLoop = m_taskLoops[nBest];
    StdRunnable runnable([this, cloneTask] { RunTask(cloneTask); });
    return pLoop->postTask(&runnable, false, 0) >= 0;
}

void CRobotAIPool::RunTask(SRobotTask task)
{
    // run the best-move search on a pool thread against the board snapshot
    MOVESTEP best = CChessAI::SearchBestMove(task.layout, task.depth);
    ApplySink apply;
    ServiceSink service;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        service = m_serviceSink;
        apply = m_applySink;
    }
    if (!service)
        return;
    // post the result back to the game main thread to apply the move
    service([apply, task, best] {
        if (apply)
            apply(task.tableId, task.seatId, task.generation, best);
    });
}

SNSEND