#ifndef __SACCHELPER_I__H__
#define __SACCHELPER_I__H__

#ifdef _WIN32
SNSBEGIN

// Forward declaration of IWindow interface
typedef struct IWindow IWindow;

DEFINE_SGUID(IID_IAccHelper,
    0x9FAF3067, 0x6723, 0x40DA, 0xA5, 0x6B, 0xCD, 0xCB, 0x11, 0xB8, 0x90, 0x2B);
// IAccHelper interface declaration with IID
#undef INTERFACE
#define INTERFACE IAccHelper
DECLARE_INTERFACE_(IAccHelper, IUnknown)
{
    DECLARE_CLASS_SIID(IID_IAccHelper)
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