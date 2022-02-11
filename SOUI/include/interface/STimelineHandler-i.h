#pragma once
#include <utilities-def.h>

SNSBEGIN
/**
 * @struct     ITimelineHandler
 * @brief      时间轴处理接口
 *
 * Describe
 */
#undef INTERFACE
#define INTERFACE ITimelineHandler
DECLARE_INTERFACE(ITimelineHandler)
{
    /**
     * @brief 下一个动画帧处理接口
     * @return
     */
    STDMETHOD_(void, OnNextFrame)(THIS) PURE;
};

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
};
SNSEND