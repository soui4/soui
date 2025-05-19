#ifndef __SACCHELPER_I__H__
#define __SACCHELPER_I__H__

#ifdef _WIN32
SNSBEGIN

// Forward declaration of IWindow interface
typedef struct IWindow IWindow;

// IAccHelper interface declaration with IID
#undef INTERFACE
#define INTERFACE IAccHelper
DECLARE_INTERFACE_IID_(IAccHelper, IUnknown, "9FAF3067-6723-40DA-A56B-CDCB11B8902B")
{
    // Standard COM method to query for a pointer to a specified interface
    STDMETHOD_(HRESULT, QueryInterface)
    (THIS_ REFIID riid, void **ppvObject) PURE;

    // Standard COM method to increment the reference count for an interface on an object
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;

    // Standard COM method to decrement the reference count for an interface on an object
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // Method to set the owner window for the accessibility helper
    STDMETHOD_(void, SetOwner)(THIS_ IWindow * pOwner) PURE;

    // Method to get the owner window of the accessibility helper
    STDMETHOD_(IWindow *, GetOwner)(CTHIS) SCONST PURE;
};

SNSEND

#endif //_WIN32
#endif // __SACCHELPER_I__H__