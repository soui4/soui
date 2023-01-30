#pragma once
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
     * @return   BOOL, TRUE--成功
     * Describe
     */
	STDMETHOD_(BOOL,StartTimer)(THIS_ int nElapse,BOOL bRepeat) PURE;

	/**
     * KillTimer
     * @brief    停止定时器
     * @return   void
     * Describe
     */
	STDMETHOD_(void,KillTimer)(THIS) PURE;
};

SNSEND