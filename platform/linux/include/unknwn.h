#ifndef __UNKNWN_H_
#define __UNKNWN_H_
#include "ctypes.h"
#include "objbase.h"
#include "guid.h"

#if defined(__cplusplus)
    
#undef IUnknown
#define INTERFACE IUnknown
DECLARE_INTERFACE(IUnknown)
{
    DECLARE_CLASS_SIID(0,0,0,0,0,0)
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID riid,void **ppvObject) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;
};
    
#else 	/* C style interface */

    typedef struct IUnknownVtbl
    {
        HRESULT ( WINAPI *QueryInterface )( 
            IUnknown * This,
            /* [in] */ REFGUID riid,
            /* [iid_is][out] */ 
            void **ppvObject);
        
        ULONG ( WINAPI *AddRef )( 
            IUnknown * This);
        
        ULONG ( WINAPI *Release )( 
            IUnknown * This);
        
    } IUnknownVtbl;

    interface IUnknown
    {
        struct IUnknownVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUnknown_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUnknown_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUnknown_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */

#endif//__UNKNWN_H_