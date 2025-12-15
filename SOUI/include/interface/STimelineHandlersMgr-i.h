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
     * @brief 注册动画帧处理接口
     * @param pHandler ITimelineHandler *--动画帧处理接口
     * @return TRUE--成功
     */
    STDMETHOD_(BOOL, RegisterTimelineHandler)(THIS_ ITimelineHandler * pHandler) PURE;

    /**
     * @brief 注销动画帧处理接口
     * @param pHandler ITimelineHandler *--动画帧处理接口
     * @return TRUE--成功
     */
    STDMETHOD_(BOOL, UnregisterTimelineHandler)(THIS_ ITimelineHandler * pHandler) PURE;

    /**
     * @brief 注册属性动画器
     * @param pAnimator IValueAnimator *--属性动画器
     * @return TRUE--成功
     */
    STDMETHOD_(BOOL, RegisterValueAnimator)(THIS_ IValueAnimator * pAnimator) PURE;

    /**
     * @brief 注销属性动画器
     * @param pAnimator IValueAnimator *--属性动画器
     * @return TRUE--成功
     */
    STDMETHOD_(BOOL, UnregisterValueAnimator)(THIS_ IValueAnimator * pAnimator) PURE;
};
SNSEND
#endif // __STIMELINEHANDLERSMGR_I__H__