#ifndef __STIMELINEHANDLER_I__H__
#define __STIMELINEHANDLER_I__H__
#include <utilities-def.h>

SNSBEGIN
/**
 * @struct     ITimelineHandler
 * @brief      Timeline handler interface
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
        kPulseInterval = 10 /**< 10ms */
    };
#endif
    /**
     * @brief Next animation frame handler interface
     * @return
     */
    STDMETHOD_(void, OnNextFrame)(THIS) PURE;
};

SNSEND
#endif /**< __STIMELINEHANDLER_I__H__ */