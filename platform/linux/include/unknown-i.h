#ifndef __UNKNWN_H_
#define __UNKNWN_H_
#include "ctypes.h"
#include "Objbase.h"
#include "guid.h"

#define __RPC__in
#define __RPC__deref_out_opt
#define __RPC__in_opt
#define __RPC__deref_opt_in_opt
#define __RPC__out


#undef INTERFACE
#define INTERFACE IUnknown
DECLARE_INTERFACE(IUnknown)
{
    DECLARE_CLASS_SIID(0,0,0,0,0,1)
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID riid,void **ppvObject) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;
};
    
#endif//__UNKNWN_H_