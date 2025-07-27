#ifndef __SACCPROXY_CAPI_H__
#define __SACCPROXY_CAPI_H__

#include "../saccproxy-i.h"

#ifdef _WIN32
#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for saccproxy Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IAccProxy C API Macros */
#define IAccProxy_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IAccProxy_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IAccProxy_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IAccProxy_get_accName(This, pszName) \
    ((This)->lpVtbl->get_accName(This, pszName))

#define IAccProxy_get_accValue(This, pszValue) \
    ((This)->lpVtbl->get_accValue(This, pszValue))

#define IAccProxy_get_accDescription(This, pszDescription) \
    ((This)->lpVtbl->get_accDescription(This, pszDescription))

#define IAccProxy_get_accRole(This, pvarRole) \
    ((This)->lpVtbl->get_accRole(This, pvarRole))

#define IAccProxy_get_accState(This, pvarState) \
    ((This)->lpVtbl->get_accState(This, pvarState))

#define IAccProxy_get_accHelp(This, pszHelp) \
    ((This)->lpVtbl->get_accHelp(This, pszHelp))

#define IAccProxy_get_accHelpTopic(This, pszHelpFile, pidTopic) \
    ((This)->lpVtbl->get_accHelpTopic(This, pszHelpFile, pidTopic))

#define IAccProxy_get_accKeyboardShortcut(This, pszKeyboardShortcut) \
    ((This)->lpVtbl->get_accKeyboardShortcut(This, pszKeyboardShortcut))

#define IAccProxy_get_accDefaultAction(This, pszDefaultAction) \
    ((This)->lpVtbl->get_accDefaultAction(This, pszDefaultAction))

#define IAccProxy_accSelect(This, flagsSelect) \
    ((This)->lpVtbl->accSelect(This, flagsSelect))

#define IAccProxy_accDoDefaultAction(This) \
    ((This)->lpVtbl->accDoDefaultAction(This))

#define IAccProxy_put_accName(This, szName) \
    ((This)->lpVtbl->put_accName(This, szName))

#define IAccProxy_put_accValue(This, szValue) \
    ((This)->lpVtbl->put_accValue(This, szValue))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IAccProxy Helper Functions */
static inline long IAccProxy_AddRef_C(IAccProxy* pThis)
{
    return IAccProxy_AddRef(pThis);
}

static inline long IAccProxy_Release_C(IAccProxy* pThis)
{
    return IAccProxy_Release(pThis);
}

static inline HRESULT IAccProxy_get_accName_C(IAccProxy* pThis, BSTR* pszName)
{
    return IAccProxy_get_accName(pThis, pszName);
}

static inline HRESULT IAccProxy_get_accValue_C(IAccProxy* pThis, BSTR* pszValue)
{
    return IAccProxy_get_accValue(pThis, pszValue);
}

static inline HRESULT IAccProxy_get_accDescription_C(IAccProxy* pThis, BSTR* pszDescription)
{
    return IAccProxy_get_accDescription(pThis, pszDescription);
}

static inline HRESULT IAccProxy_get_accRole_C(IAccProxy* pThis, VARIANT* pvarRole)
{
    return IAccProxy_get_accRole(pThis, pvarRole);
}

static inline HRESULT IAccProxy_get_accState_C(IAccProxy* pThis, VARIANT* pvarState)
{
    return IAccProxy_get_accState(pThis, pvarState);
}

static inline HRESULT IAccProxy_get_accHelp_C(IAccProxy* pThis, BSTR* pszHelp)
{
    return IAccProxy_get_accHelp(pThis, pszHelp);
}

static inline HRESULT IAccProxy_get_accHelpTopic_C(IAccProxy* pThis, BSTR* pszHelpFile, long* pidTopic)
{
    return IAccProxy_get_accHelpTopic(pThis, pszHelpFile, pidTopic);
}

static inline HRESULT IAccProxy_get_accKeyboardShortcut_C(IAccProxy* pThis, BSTR* pszKeyboardShortcut)
{
    return IAccProxy_get_accKeyboardShortcut(pThis, pszKeyboardShortcut);
}

static inline HRESULT IAccProxy_get_accDefaultAction_C(IAccProxy* pThis, BSTR* pszDefaultAction)
{
    return IAccProxy_get_accDefaultAction(pThis, pszDefaultAction);
}

static inline HRESULT IAccProxy_accSelect_C(IAccProxy* pThis, long flagsSelect)
{
    return IAccProxy_accSelect(pThis, flagsSelect);
}

static inline HRESULT IAccProxy_accDoDefaultAction_C(IAccProxy* pThis)
{
    return IAccProxy_accDoDefaultAction(pThis);
}

static inline HRESULT IAccProxy_put_accName_C(IAccProxy* pThis, BSTR szName)
{
    return IAccProxy_put_accName(pThis, szName);
}

static inline HRESULT IAccProxy_put_accValue_C(IAccProxy* pThis, BSTR szValue)
{
    return IAccProxy_put_accValue(pThis, szValue);
}

/*
 * Convenience macros for common accessibility proxy operations
 */

/* Property access shortcuts */
#define IAccProxy_GetName(This, name) \
    IAccProxy_get_accName(This, name)

#define IAccProxy_GetValue(This, value) \
    IAccProxy_get_accValue(This, value)

#define IAccProxy_GetDescription(This, desc) \
    IAccProxy_get_accDescription(This, desc)

#define IAccProxy_GetRole(This, role) \
    IAccProxy_get_accRole(This, role)

#define IAccProxy_GetState(This, state) \
    IAccProxy_get_accState(This, state)

#define IAccProxy_GetHelp(This, help) \
    IAccProxy_get_accHelp(This, help)

#define IAccProxy_GetHelpTopic(This, helpFile, topic) \
    IAccProxy_get_accHelpTopic(This, helpFile, topic)

#define IAccProxy_GetKeyboardShortcut(This, shortcut) \
    IAccProxy_get_accKeyboardShortcut(This, shortcut)

#define IAccProxy_GetDefaultAction(This, action) \
    IAccProxy_get_accDefaultAction(This, action)

/* Property modification shortcuts */
#define IAccProxy_SetName(This, name) \
    IAccProxy_put_accName(This, name)

#define IAccProxy_SetValue(This, value) \
    IAccProxy_put_accValue(This, value)

/* Action shortcuts */
#define IAccProxy_Select(This, flags) \
    IAccProxy_accSelect(This, flags)

#define IAccProxy_DoDefaultAction(This) \
    IAccProxy_accDoDefaultAction(This)

#define IAccProxy_PerformDefaultAction(This) \
    IAccProxy_accDoDefaultAction(This)

/* Safe accessibility proxy operations */
#define IAccProxy_SafeGetName(This, name) \
    ((This) && (name) ? IAccProxy_get_accName(This, name) : E_INVALIDARG)

#define IAccProxy_SafeGetValue(This, value) \
    ((This) && (value) ? IAccProxy_get_accValue(This, value) : E_INVALIDARG)

#define IAccProxy_SafeSetName(This, name) \
    ((This) && (name) ? IAccProxy_put_accName(This, name) : E_INVALIDARG)

#define IAccProxy_SafeSetValue(This, value) \
    ((This) && (value) ? IAccProxy_put_accValue(This, value) : E_INVALIDARG)

#define IAccProxy_SafeSelect(This, flags) \
    ((This) ? IAccProxy_accSelect(This, flags) : E_INVALIDARG)

#define IAccProxy_SafeDoDefaultAction(This) \
    ((This) ? IAccProxy_accDoDefaultAction(This) : E_INVALIDARG)

/*
 * Accessibility proxy helper functions
 */
static inline BOOL IAccProxy_IsValid_C(IAccProxy* pThis)
{
    return (pThis != NULL);
}

static inline BOOL IAccProxy_HasName_C(IAccProxy* pThis)
{
    if (!pThis) return FALSE;
    
    BSTR name = NULL;
    HRESULT hr = IAccProxy_get_accName(pThis, &name);
    if (SUCCEEDED(hr) && name) {
        SysFreeString(name);
        return TRUE;
    }
    return FALSE;
}

static inline BOOL IAccProxy_HasValue_C(IAccProxy* pThis)
{
    if (!pThis) return FALSE;
    
    BSTR value = NULL;
    HRESULT hr = IAccProxy_get_accValue(pThis, &value);
    if (SUCCEEDED(hr) && value) {
        SysFreeString(value);
        return TRUE;
    }
    return FALSE;
}

/*
 * Accessibility proxy state management
 */
typedef struct AccProxyState {
    IAccProxy* accProxy;
    BSTR name;
    BSTR value;
    BSTR description;
    VARIANT role;
    VARIANT state;
    BOOL isValid;
    BOOL hasName;
    BOOL hasValue;
} AccProxyState;

static inline void AccProxyState_Init(AccProxyState* state, IAccProxy* accProxy)
{
    if (state) {
        memset(state, 0, sizeof(AccProxyState));
        state->accProxy = accProxy;
        if (accProxy) {
            state->isValid = TRUE;
            
            // Try to get basic properties
            IAccProxy_get_accName(accProxy, &state->name);
            IAccProxy_get_accValue(accProxy, &state->value);
            IAccProxy_get_accDescription(accProxy, &state->description);
            IAccProxy_get_accRole(accProxy, &state->role);
            IAccProxy_get_accState(accProxy, &state->state);
            
            state->hasName = (state->name != NULL);
            state->hasValue = (state->value != NULL);
        }
    }
}

static inline void AccProxyState_Cleanup(AccProxyState* state)
{
    if (state) {
        if (state->name) {
            SysFreeString(state->name);
        }
        if (state->value) {
            SysFreeString(state->value);
        }
        if (state->description) {
            SysFreeString(state->description);
        }
        VariantClear(&state->role);
        VariantClear(&state->state);
        memset(state, 0, sizeof(AccProxyState));
    }
}

static inline BOOL AccProxyState_UpdateName(AccProxyState* state, BSTR newName)
{
    if (!state || !state->accProxy) return FALSE;
    
    HRESULT hr = IAccProxy_put_accName(state->accProxy, newName);
    if (SUCCEEDED(hr)) {
        if (state->name) {
            SysFreeString(state->name);
        }
        state->name = SysAllocString(newName);
        state->hasName = (state->name != NULL);
        return TRUE;
    }
    return FALSE;
}

static inline BOOL AccProxyState_UpdateValue(AccProxyState* state, BSTR newValue)
{
    if (!state || !state->accProxy) return FALSE;
    
    HRESULT hr = IAccProxy_put_accValue(state->accProxy, newValue);
    if (SUCCEEDED(hr)) {
        if (state->value) {
            SysFreeString(state->value);
        }
        state->value = SysAllocString(newValue);
        state->hasValue = (state->value != NULL);
        return TRUE;
    }
    return FALSE;
}

/*
 * Reference counting helpers
 */
#define IAccProxy_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IAccProxy_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void IAccProxy_DebugInfo_C(IAccProxy* pThis)
{
    if (!pThis) {
        printf("AccProxy: NULL\n");
        return;
    }
    
    printf("AccProxy: %p\n", pThis);
    printf("  Has Name: %s\n", IAccProxy_HasName_C(pThis) ? "Yes" : "No");
    printf("  Has Value: %s\n", IAccProxy_HasValue_C(pThis) ? "Yes" : "No");
}

static inline void AccProxyState_DebugInfo(const AccProxyState* state)
{
    if (!state) {
        printf("AccProxyState: NULL\n");
        return;
    }
    
    printf("AccProxyState: %p\n", state);
    printf("  AccProxy: %p\n", state->accProxy);
    printf("  Valid: %s\n", state->isValid ? "Yes" : "No");
    printf("  Has Name: %s\n", state->hasName ? "Yes" : "No");
    printf("  Has Value: %s\n", state->hasValue ? "Yes" : "No");
    
    if (state->name) {
        wprintf(L"  Name: %s\n", state->name);
    }
    if (state->value) {
        wprintf(L"  Value: %s\n", state->value);
    }
}
#else
#define IAccProxy_DebugInfo_C(This) ((void)0)
#define AccProxyState_DebugInfo(state) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif //_WIN32
#endif /* __SACCPROXY_CAPI_H__ */
