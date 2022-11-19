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
	STDMETHOD_(UINT,StartTimer)(THIS_ int nElapse,BOOL bRepeat) PURE;

	STDMETHOD_(void,KillTimer)(THIS) PURE;
};

SNSEND