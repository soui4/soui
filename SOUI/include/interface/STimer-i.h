#ifndef __STIMER_I__H__
#define __STIMER_I__H__
#include <interface/obj-ref-i.h>
#include <interface/SEvtArgs-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ITimer
DECLARE_INTERFACE_(ITimer, IObjRef)
{
    /** Add reference */
    //
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /** Release reference */
    //
    STDMETHOD_(long, Release)(THIS) PURE;

    /** Release object */
    //
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * StartTimer
     * @brief    Start the timer
     * @param    int nElapse --  Delay (ms)
     * @param    BOOL bRepeat -- Repeat flag
     * @param    LPARAM uData -- Timer custom parameter
     * @return   BOOL, TRUE--success
     * Describe
     */
    STDMETHOD_(BOOL, StartTimer)(THIS_ int nElapse, BOOL bRepeat, LPARAM uData DEF_VAL(0)) PURE;

    /**
     * KillTimer
     * @brief    Stop the timer
     * @return   void
     * Describe
     */
    STDMETHOD_(void, KillTimer)(THIS) PURE;
};

SNSEND
#endif /**< __STIMER_I__H__ */