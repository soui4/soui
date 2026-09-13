//********************************************************************
/** Control operation interfaces are defined here; all interfaces inherit from IObjRef and specify their own interface ID */
//********************************************************************
#ifndef __SCTRL_I__H__
#define __SCTRL_I__H__
#include <interface/SWindow-i.h>
#include <SGuidDef.h>
SNSBEGIN

/** {BCB520F9-3C81-4AD7-AD E2-FE72BD761032} */
DEFINE_SGUID(IID_ICtrl, 0xBCB520F9, 0x3C81, 0x4AD7, 0xAD, 0xe2, 0xFE, 0x72, 0xBD, 0x76, 0x10, 0x32);
#undef INTERFACE
#define INTERFACE ICtrl
DECLARE_INTERFACE_(ICtrl, IObjRef)
{
    DECLARE_CLASS_SIID(IID_ICtrl)

    /**
     * @brief Increment the reference count
     * @return new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count
     * @return new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Cast to the IWindow* interface
     * @return IWindow* interface
     */
    STDMETHOD_(IWindow *, ToIWindow)(THIS) PURE;
};

SNSEND
#endif /**< __SCTRL_I__H__ */