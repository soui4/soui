#ifndef __STIMELINEHANDLER_I__H__
#define __STIMELINEHANDLER_I__H__
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
    #ifdef __cplusplus
    enum
    {
        kPulseInterval = 10  // 10ms
    };
    #endif
    /**
     * @brief 下一个动画帧处理接口
     * @return
     */
    STDMETHOD_(void, OnNextFrame)(THIS) PURE;
};

SNSEND
#endif // __STIMELINEHANDLER_I__H__