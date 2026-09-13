#ifndef __STIMERGENERATOR__H__
#define __STIMERGENERATOR__H__

#include "core/SSingletonMap.h"

SNSBEGIN

/**
 * @struct TIMERINFO
 * @brief Timer information structure
 */
struct TIMERINFO
{
    SAutoRefPtr<IEvtSlot> pEvtSlot; /**< Pointer to event slot object */
    BOOL bRepeat;                   /**< Whether the timer repeats */
    LPARAM uData;                   /**< User data */
};

/**
 * @class STimerGenerator
 * @brief Timer generator class, derived from the singleton map class
 */
class STimerGenerator : public SSingletonMap<STimerGenerator, TIMERINFO, UINT_PTR> {
    SINGLETON2_TYPE(SINGLETON_TIMERGENERATOR)

  public:
    /**
     * @brief Destructor
     */
    ~STimerGenerator();

    /**
     * @brief Set the timer
     * @param pEvtSlot Pointer to event slot object
     * @param nElapse Timer interval (milliseconds)
     * @param bRepeat Whether the timer repeats
     * @param uData User data
     * @return Timer ID
     */
    UINT SetTimer(IEvtSlot *pEvtSlot, UINT nElapse, BOOL bRepeat, LPARAM uData);

    /**
     * @brief Clear the timer
     * @param uID Timer ID
     */
    void ClearTimer(UINT_PTR uID);

    /**
     * @brief Timer callback function
     * @param hwnd Window handle
     * @param uMsg Message ID
     * @param idEvent Timer ID
     * @param dwTime Timestamp
     */
    static VOID CALLBACK _TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
};

SNSEND

#endif /**< __STIMERGENERATOR__H__ */