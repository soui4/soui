#ifndef __SACCHELPER_I__H__
#define __SACCHELPER_I__H__
#include <interface/SWindow-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IAccHelper
DECLARE_INTERFACE_(IAccHelper, IUnknown)
{
    DECLARE_CLASS_SIID(0x9FAF3067,0x6723,0x40DA,0xA56B,0xDCB1,0x1B8902B)
   
    STDMETHOD_(HRESULT, QueryInterface)
    (THIS_ REFIID riid, void  **ppvObject) PURE;

    STDMETHOD_(ULONG, AddRef)(THIS) PURE;

    STDMETHOD_(ULONG, Release)(THIS) PURE;

    STDMETHOD_(void, SetOwner)(THIS_ IWindow * pOwner) PURE;
    STDMETHOD_(IWindow *, GetOwner)(CTHIS) SCONST PURE;
};

SNSEND
#endif // __SACCHELPER_I__H__