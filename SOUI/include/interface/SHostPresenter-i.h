#pragma once
#include <interface/shostwnd-i.h>
SNSBEGIN

#undef INTERFACE
#define INTERFACE IHostPresenter
DECLARE_INTERFACE_(IHostPresenter, IObjRef)
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

	STDMETHOD_(void,OnHostCreate)(THIS) PURE;
	STDMETHOD_(void,OnHostDestroy)(THIS) PURE;
	STDMETHOD_(void,OnHostResize)(THIS_ SIZE szHost) PURE;
	STDMETHOD_(void,OnHostPresent)(THIS_ HDC hdc,IRenderTarget *pMemRT,LPCRECT rcDirty,BYTE byAlpha) PURE;
	STDMETHOD_(void,OnHostAlpha)(THIS_ BYTE byAlpha) PURE;
};

SNSEND