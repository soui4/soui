#ifndef __STIMELINEHANDLERSMGR_I__H__
#define __STIMELINEHANDLERSMGR_I__H__
#include <utilities-def.h>
#include <interface/STimelineHandler-i.h>
#include <interface/SValueAnimator-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ITimelineHandlersMgr
DECLARE_INTERFACE(ITimelineHandlersMgr)
{
    /**
     * @brief Register the animation frame handler interface
     * @param pHandler ITimelineHandler *--Animation frame handler interface
     * @return TRUE--success
     */
    STDMETHOD_(BOOL, RegisterTimelineHandler)(THIS_ ITimelineHandler * pHandler) PURE;

    /**
     * @brief Unregister the animation frame handler interface
     * @param pHandler ITimelineHandler *--Animation frame handler interface
     * @return TRUE--success
     */
    STDMETHOD_(BOOL, UnregisterTimelineHandler)(THIS_ ITimelineHandler * pHandler) PURE;

    /**
     * @brief Register the property animator
     * @param pAnimator IValueAnimator *--Property animator
     * @return TRUE--success
     */
    STDMETHOD_(BOOL, RegisterValueAnimator)(THIS_ IValueAnimator * pAnimator) PURE;

    /**
     * @brief Unregister the property animator
     * @param pAnimator IValueAnimator *--Property animator
     * @return TRUE--success
     */
    STDMETHOD_(BOOL, UnregisterValueAnimator)(THIS_ IValueAnimator * pAnimator) PURE;
};
SNSEND
#endif /**< __STIMELINEHANDLERSMGR_I__H__ */