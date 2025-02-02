#ifndef __SACCHELPER_I__H__
#define __SACCHELPER_I__H__

#include <SGuidDef.h>
SNSBEGIN

typedef struct IWindow IWindow;
// {727E4EAB-CFFA-4073-8C01-2815053FA44F}
DEFINE_SGUID(IID_IAccHelper, 0x727e4eab, 0xcffa, 0x4073, 0x8c, 0x1, 0x28, 0x15, 0x5, 0x3f, 0xa4, 0x4f);
#undef INTERFACE
#define INTERFACE IAccHelper
DECLARE_INTERFACE_(IAccHelper, IUnknown)
{
    DECLARE_CLASS_SIID(IID_IAccHelper)

    STDMETHOD_(HRESULT, QueryInterface)
    (THIS_ REFIID riid, void **ppvObject) PURE;

    STDMETHOD_(ULONG, AddRef)(THIS) PURE;

    STDMETHOD_(ULONG, Release)(THIS) PURE;

    STDMETHOD_(void, SetOwner)(THIS_ IWindow * pOwner) PURE;
    STDMETHOD_(IWindow *, GetOwner)(CTHIS) SCONST PURE;
};

SNSEND
#endif // __SACCHELPER_I__H__