#ifndef __SACCPROXY_I__H__
#define __SACCPROXY_I__H__

#include <interface/obj-ref-i.h>

SNSBEGIN

#ifdef _WIN32

// Forward declaration of IAccProxy interface
#undef INTERFACE
#define INTERFACE IAccProxy
DECLARE_INTERFACE_(IAccProxy, IObjRef)
{
    // Increments the reference count for an interface on an object
    STDMETHOD_(long, AddRef)(THIS) PURE;

    // Decrements the reference count for an interface on an object
    STDMETHOD_(long, Release)(THIS) PURE;

    // Called when the final release of the object occurs
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    // Retrieves the name of the accessible object
    STDMETHOD(get_accName)(THIS_ BSTR * pszName) PURE;

    // Retrieves the value of the accessible object
    STDMETHOD(get_accValue)(THIS_ BSTR * pszValue) PURE;

    // Retrieves the description of the accessible object
    STDMETHOD(get_accDescription)(THIS_ BSTR * pszDescription) PURE;

    // Retrieves the role of the accessible object
    STDMETHOD(get_accRole)(THIS_ VARIANT * pvarRole) PURE;

    // Retrieves the state of the accessible object
    STDMETHOD(get_accState)(THIS_ VARIANT * pvarState) PURE;

    // Retrieves the help string for the accessible object
    STDMETHOD(get_accHelp)(THIS_ BSTR * pszHelp) PURE;

    // Retrieves the help topic for the accessible object
    STDMETHOD(get_accHelpTopic)(THIS_ BSTR * pszHelpFile, long *pidTopic) PURE;

    // Retrieves the keyboard shortcut for the accessible object
    STDMETHOD(get_accKeyboardShortcut)(THIS_ BSTR * pszKeyboardShortcut) PURE;

    // Retrieves the default action for the accessible object
    STDMETHOD(get_accDefaultAction)(THIS_ BSTR * pszDefaultAction) PURE;

    // Selects the accessible object
    STDMETHOD(accSelect)(THIS_ long flagsSelect) PURE;

    // Performs the default action for the accessible object
    STDMETHOD(accDoDefaultAction)(THIS) PURE;

    // Sets the name of the accessible object
    STDMETHOD(put_accName)(THIS_ BSTR szName) PURE;

    // Sets the value of the accessible object
    STDMETHOD(put_accValue)(THIS_ BSTR szValue) PURE;
};

#endif //_WIN32

SNSEND
#endif // __SACCPROXY_I__H__