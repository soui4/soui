#ifndef __ROBOTAIPOOL_H__
#define __ROBOTAIPOOL_H__

#include <core/SSingleton.h>
#include <interface/STaskLoop-i.h>
#include <helper/obj-ref-impl.hpp>
#include <functional>
#include <vector>
#include <mutex>
#include "RobotDispatch.h"

SNSBEGIN

// Robot AI thread pool based on SOUI ITaskLoop (one task queue per thread).
// Each thinking task is dispatched to the thread with the fewest queued tasks
// for load balancing. Search runs on the pool threads (snapshot only), and the
// result is posted back through the service sink to the game main thread.
class CRobotAIPool : public SSingleton<CRobotAIPool>
{
public:
    CRobotAIPool();
    virtual ~CRobotAIPool();

    // Create nThreads task-loop threads (clamped to [1,16]). No-op if already initialized.
    bool Init(int nThreads);

    // Stop and reclaim all threads, clear callbacks.
    void Shutdown();

    // Dispatch one robot thinking task; returns true if it entered the pool queue.
    bool Dispatch(const SRobotTask &task);

    // serviceSink: post a callback to the game main thread (LWS thread).
    typedef std::function<void(std::function<void()>)> ServiceSink;
    // applySink: apply a robot move on the game main thread (tableId, seatId, generation, best).
    typedef std::function<void(int, int, int, const MOVESTEP &)> ApplySink;
    void SetSink(const ServiceSink &service, const ApplySink &apply);

protected:
    void RunTask(SRobotTask task); // runs the search on a pool thread

private:
    std::mutex m_mutex;
    std::vector<SAutoRefPtr<ITaskLoop> > m_taskLoops;
    ServiceSink m_serviceSink;
    ApplySink m_applySink;
};

SNSEND

#endif // !__ROBOTAIPOOL_H__