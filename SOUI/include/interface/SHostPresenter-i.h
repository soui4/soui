#ifndef __SHOSTPRESENTER_I__H__
#define __SHOSTPRESENTER_I__H__
#include <interface/obj-ref-i.h>
#include <interface/SRender-i.h>
SNSBEGIN

#undef INTERFACE
#define INTERFACE IHostPresenter
DECLARE_INTERFACE_(IHostPresenter, IObjRef)
{
    /** Add reference */
    /**
     * @return long - current reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /** Release reference */
    /**
     * @return long - current reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /** Release object */
    /**
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Called when the host is created
     * @return void
     */
    STDMETHOD_(void, OnHostCreate)(THIS) PURE;

    /**
     * @brief Called when the host is destroyed
     * @return void
     */
    STDMETHOD_(void, OnHostDestroy)(THIS) PURE;

    /**
     * @brief Called when the host is resized
     * @param szHost - new host size
     * @return void
     */
    STDMETHOD_(void, OnHostResize)(THIS_ SIZE szHost) PURE;

    /**
     * @brief Called when the host renders
     * @param hdc - device context handle
     * @param pMemRT - memory render target
     * @param rcDirty - dirty rectangle region
     * @param byAlpha - alpha
     * @return void
     */
    STDMETHOD_(void, OnHostPresent)(THIS_ HDC hdc, IRenderTarget * pMemRT, LPCRECT rcDirty, BYTE byAlpha) PURE;
};

SNSEND
#endif /**< __SHOSTPRESENTER_I__H__ */
