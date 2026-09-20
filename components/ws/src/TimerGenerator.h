/**
 * @file TimerGenerator.h
 * @brief Timer generator: a single dedicated thread schedules all timers by ID
 *
 * Modeled after swinx's window timer implementation (timer_sched_loop in src/sysobjs.cpp): one
 * thread owns the due time of every timer and waits on a condition variable until the earliest
 * one is due, then processes all that are due.
 *
 * Unlike swinx, this generator is a pure producer: when a timer becomes due it invokes
 * ITimerListener::onTimer directly on the timer thread and performs no thread switching for the
 * business layer. The business layer decides, inside onTimer, whether to move the real work to
 * its own thread (e.g. via IWsServer::postServiceTask). A timer may be marked non-repeating
 * (one-shot): it fires once and is then removed from the schedule automatically.
 */

#ifndef TIMER_GENERATOR_H
#define TIMER_GENERATOR_H

#include <interface/ws-i.h>
#include <helper/obj-ref-impl.hpp>
#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

SNSBEGIN

/**
 * @brief Timer generator
 *
 * Internally only one timer thread exists; it stores each timer's due time keyed by ID. When a
 * timer is due the generator invokes ITimerListener::onTimer on the timer thread and does NOT run
 * any business logic inside the generator. The business layer decides in onTimer whether to move
 * the work to its own thread (e.g. postServiceTask).
 */
class CTimerGenerator : public TObjRefImpl<ITimerGenerator>
{
  public:
    CTimerGenerator();
    virtual ~CTimerGenerator();
  public:
    STDMETHODIMP_(BOOL) start(THIS_ ITimerListener *pListener) OVERRIDE;
    STDMETHODIMP_(void) stop(THIS) OVERRIDE;
    STDMETHODIMP_(UINT_PTR) setTimer(THIS_ UINT_PTR uTimerID, uint32_t uIntervalMs, BOOL bRepeat = true) OVERRIDE;
    STDMETHODIMP_(BOOL) killTimer(THIS_ UINT_PTR uTimerID) OVERRIDE;

  private:
    // One timer: next due time (monotonic clock), period, and whether it repeats.
    struct TimerEntry
    {
        std::chrono::steady_clock::time_point due;
        uint32_t uPeriodMs;
        bool bRepeat; // TRUE = periodic (re-fires every period); FALSE = one-shot (auto-removed after firing)
    };

    // Timer thread body: owns the schedule and invokes onTimer directly on this thread.
    void ThreadProc();
    ITimerListener *m_pListener; // business layer impl; caller-owned, must outlive the generator

    std::map<UINT_PTR, TimerEntry> m_timers;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::thread m_thread;
    bool m_bStop;       // request the timer thread to exit; also rejects new timers
    UINT_PTR m_uNextID; // cursor for auto-allocating IDs when setTimer(0, ...) is used
};

SNSEND
#endif // TIMER_GENERATOR_H
