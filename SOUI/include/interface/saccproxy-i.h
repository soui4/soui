#ifndef __SACCPROXY_I__H__
#define __SACCPROXY_I__H__

#include <interface/obj-ref-i.h>

SNSBEGIN

/** Forward declaration of IWindow interface */
interface IWindow;

/** Forward declaration of IAccProxy interface */
#undef INTERFACE
#define INTERFACE IAccProxy
DECLARE_INTERFACE_(IAccProxy, IObjRef)
{
    /** Increments the reference count for an interface on an object */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /** Decrements the reference count for an interface on an object */
    STDMETHOD_(long, Release)(THIS) PURE;

    /** Called when the final release of the object occurs */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /** Retrieves the name of the accessible object */
    STDMETHOD(get_accName)(THIS_ BSTR * pszName) PURE;

    /** Retrieves the value of the accessible object */
    STDMETHOD(get_accValue)(THIS_ BSTR * pszValue) PURE;

    /** Retrieves the description of the accessible object */
    STDMETHOD(get_accDescription)(THIS_ BSTR * pszDescription) PURE;

    /** Retrieves the role of the accessible object */
    STDMETHOD(get_accRole)(THIS_ VARIANT * pvarRole) PURE;

    /** Retrieves the state of the accessible object */
    STDMETHOD(get_accState)(THIS_ VARIANT * pvarState) PURE;

    /** Retrieves the help string for the accessible object */
    STDMETHOD(get_accHelp)(THIS_ BSTR * pszHelp) PURE;

    /** Retrieves the help topic for the accessible object */
    STDMETHOD(get_accHelpTopic)(THIS_ BSTR * pszHelpFile, long *pidTopic) PURE;

    /** Retrieves the keyboard shortcut for the accessible object */
    STDMETHOD(get_accKeyboardShortcut)(THIS_ BSTR * pszKeyboardShortcut) PURE;

    /** Retrieves the default action for the accessible object */
    STDMETHOD(get_accDefaultAction)(THIS_ BSTR * pszDefaultAction) PURE;

    /** Selects the accessible object */
    STDMETHOD(accSelect)(THIS_ long flagsSelect) PURE;

    /** Performs the default action for the accessible object */
    STDMETHOD(accDoDefaultAction)(THIS) PURE;

    /** Sets the name of the accessible object */
    STDMETHOD(put_accName)(THIS_ BSTR szName) PURE;

    /** Sets the value of the accessible object */
    STDMETHOD(put_accValue)(THIS_ BSTR szValue) PURE;

    //----------------------------------------------------------------------
    /** AccChild (real child window) support. */
    //
    /** A child window is an SWindow that has its own IAccessible. It is exposed */
    /** to MSAA as a real child object and addressed via get_accChild() (which */
    /** returns an IDispatch). These are the *default* accessible children of a */
    /** window (its regular child windows). MVC virtual-list controls override */
    /** them to expose their currently-visible item panels. All indices are */
    /** 1-based; SViewBase/STreeView expose the underlying visible items via */
    /** GetVisibleAccChildCount()/GetVisibleAccChild()/GetVisibleAccSelIndex(). */
    //----------------------------------------------------------------------

    /** Number of AccChild (real child window) objects. */
    STDMETHOD_(int, GetAccChildCount)(THIS) PURE;

    /** The i-th AccChild window. */
    STDMETHOD_(IWindow *, GetAccChild)(THIS_ int iChild) PURE;

    /** Currently selected child's 1-based index, 0 = none. */
    STDMETHOD_(int, GetAccSelIndex)(THIS) PURE;

    //----------------------------------------------------------------------
    /** AccSubItem (self-drawn sub-item) support. */
    //
    /** An AccSubItem is a drawn sub-item that is NOT an SWindow (e.g. a tab */
    /** header of STabCtrl, a column header of SHeaderCtrl). In MSAA it is */
    /** addressed as a child id on the parent object itself (CHILDID_SELF + N) */
    /** instead of having its own IAccessible. All indices are 1-based. */
    //----------------------------------------------------------------------

    /** Number of AccSubItems (0 = none). */
    STDMETHOD_(int, GetAccSubItemCount)(THIS) PURE;

    /** Name / Role (ROLE_SYSTEM_*) / State (STATE_SYSTEM_*) of the i-th AccSubItem. */
    /** Name returns a SysAllocString'd BSTR or NULL. */
    STDMETHOD_(BSTR, GetAccSubItemName)(THIS_ int iChild) PURE;
    STDMETHOD_(long, GetAccSubItemRole)(THIS_ int iChild) PURE;
    STDMETHOD_(DWORD, GetAccSubItemState)(THIS_ int iChild) PURE;

    /** Host-client rect of the i-th AccSubItem. */
    STDMETHOD_(void, GetAccSubItemRect)(THIS_ int iChild, RECT *prc) PURE;

    /** Currently selected AccSubItem (1-based), -1 = none. */
    STDMETHOD_(int, GetAccSubItemSel)(THIS) PURE;

    /** Select the i-th AccSubItem. Return S_OK / E_INVALIDARG / E_NOTIMPL. */
    STDMETHOD(SetAccSubItemSel)(THIS_ int iChild) PURE;

    /** Hit-test in host-client coords; return 1-based AccSubItem index or -1. */
    STDMETHOD_(int, HitTestAccSubItem)(THIS_ long x, long y) PURE;
};

SNSEND
#endif /**< __SACCPROXY_I__H__ */