#pragma once

#include <string>
#include <helper/SCriticalSection.h>
#include <helper/SNoCopyable.hpp>
#include <interface/STaskLoop-i.h>
#include <helper/SSemaphore.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

class ThreadPrivate;
struct IRunnable;

/**
 * Thread class for creating a thread.
 */
class Thread : public SNoCopyable
{
public:
    enum ThreadPriority
    {
        High = 1,
        Normal = 0,
        Low = -1,
    };

    /**
     * Set stopping flag to thread. The thread may not be stopped immediately.
     */
    void stop();

    /**
     * Wait for thread stopped synchronized.
     */
    void waitForStop();

    /**
     * Get stopping flag.
     * @return true if stopping flag is set.
     */
    bool isStopping() ;
    /**
      * Get stopped flag.
      * @return true if stopping flag is set or start flag is false.
      */
    bool isStopped() ;

    /**
     * Get thread ID.
     * @return thread ID.
     */
    long getThreadID() const;

    /**
     * Get thread name.
     * @return thread name.
     */
    std::string getName() ;

    /**
     * Identify whether thread is started.
     * @return true if started.
     */
    bool isStart()
    {
        SAutoLock autoLock(_lock);
        return _start;
    }

    /**
     * Default constructor.
     */
    Thread();

    /**
     * Destructor.
     */
    virtual ~Thread();

	bool start(IRunnable *runnable, const std::string &name, ThreadPriority priority = Normal);

private:
   static unsigned int CALLBACK threadProc(void *args);

    void clear();
    void setThreadName(const std::string &name);
    bool setPriority(ThreadPriority priority);

    // Thread object itself must be thread-safe
    SCriticalSection _lock;
    bool _start;
    bool _stopping;
    SAutoRefPtr<IRunnable> _runnable;
    std::string _name;
    SSemaphore _startSem;
    ThreadPrivate &_private;
    ThreadPriority _priorityLevel;
};

SNSEND