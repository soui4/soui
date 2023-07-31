#ifndef __STIMER_I__H__
#define __STIMER_I__H__
#include <interface/obj-ref-i.h>
#include <interface/SEvtArgs-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ITimer
DECLARE_INTERFACE_(ITimer, IObjRef)
{
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //////////////////////////////////////////////////////////////////////////

    /**
     * StartTimer
     * @brief    启动定时器
     * @param    int nElapse --  延时(ms)
     * @param    BOOL bRepeat -- 重复标志
     * @param    LPARAM uData -- Timer 自定义参数
     * @return   BOOL, TRUE--成功
     * Describe
     */
    STDMETHOD_(BOOL, StartTimer)(THIS_ int nElapse, BOOL bRepeat, LPARAM uData DEF_VAL(0)) PURE;

    /**
     * KillTimer
     * @brief    停止定时器
     * @return   void
     * Describe
     */
    STDMETHOD_(void, KillTimer)(THIS) PURE;
};

SNSEND
#endif // __STIMER_I__H__