#ifndef __STIMER__H__
#define __STIMER__H__

#include <interface/STimer-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

/**
 * @class STimer
 * @brief Timer class
 */
class SOUI_EXP STimer : public TObjRefImpl<ITimer> {
  public:
    /**
     * @brief Constructor
     * @param pSlot Pointer to event slot object
     */
    STimer(IEvtSlot *pSlot);

    /**
     * @brief Destructor
     */
    ~STimer(void);

  public:
    /**
     * @brief Start the timer
     * @param nElapse Timer interval (milliseconds)
     * @param bRepeat Whether the timer repeats
     * @param uData User data (default is 0)
     * @return Returns TRUE if the timer started successfully, otherwise FALSE
     */
    STDMETHOD_(BOOL, StartTimer)(THIS_ int nElapse, BOOL bRepeat, LPARAM uData DEF_VAL(0)) OVERRIDE;

    /**
     * @brief Stop the timer
     */
    STDMETHOD_(void, KillTimer)(THIS) OVERRIDE;

  private:
    UINT m_uTimerId;                 /**< Timer ID */
    SAutoRefPtr<IEvtSlot> m_evtSlot; /**< Pointer to event slot object */
};

SNSEND

#endif /**< __STIMER__H__ */