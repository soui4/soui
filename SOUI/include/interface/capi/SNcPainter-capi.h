#ifndef __SNCPAINTER_CAPI_H__
#define __SNCPAINTER_CAPI_H__

#include "../SNcPainter-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SNcPainter Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* INcPainter C API Macros */
#define INcPainter_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define INcPainter_Release(This) \
    ((This)->lpVtbl->Release(This))

#define INcPainter_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define INcPainter_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define INcPainter_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define INcPainter_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define INcPainter_GetClassInfo(This) \
    ((This)->lpVtbl->GetClassInfo(This))

#define INcPainter_SetAttribute(This, strName, strValue, bLoading) \
    ((This)->lpVtbl->SetAttribute(This, strName, strValue, bLoading))

#define INcPainter_SetAttributeA(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttributeA(This, pszAttr, pszValue, bLoading))

#define INcPainter_GetAttribute(This, strName, strValue) \
    ((This)->lpVtbl->GetAttribute(This, strName, strValue))

#define INcPainter_GetAttributeA(This, pszAttr, strValue) \
    ((This)->lpVtbl->GetAttributeA(This, pszAttr, strValue))

#define INcPainter_AfterAttribute(This) \
    ((This)->lpVtbl->AfterAttribute(This))

#define INcPainter_InitFromXml(This, xmlNode) \
    ((This)->lpVtbl->InitFromXml(This, xmlNode))

#define INcPainter_OnInitFinished(This, xmlNode) \
    ((This)->lpVtbl->OnInitFinished(This, xmlNode))

#define INcPainter_GetRoot(This) \
    ((This)->lpVtbl->GetRoot(This))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* INcPainter Helper Functions */
static inline long INcPainter_AddRef_C(INcPainter* pThis)
{
    return INcPainter_AddRef(pThis);
}

static inline long INcPainter_Release_C(INcPainter* pThis)
{
    return INcPainter_Release(pThis);
}

static inline int INcPainter_GetObjectType_C(const INcPainter* pThis)
{
    return INcPainter_GetObjectType(pThis);
}

static inline LPCWSTR INcPainter_GetObjectClass_C(const INcPainter* pThis)
{
    return INcPainter_GetObjectClass(pThis);
}

static inline BOOL INcPainter_IsClass_C(const INcPainter* pThis, LPCWSTR lpszName)
{
    return INcPainter_IsClass(pThis, lpszName);
}

static inline const IObjectInfo* INcPainter_GetClassInfo_C(const INcPainter* pThis)
{
    return INcPainter_GetClassInfo(pThis);
}

static inline HRESULT INcPainter_SetAttribute_C(INcPainter* pThis, const IStringW* strName, const IStringW* strValue, BOOL bLoading)
{
    return INcPainter_SetAttribute(pThis, strName, strValue, bLoading);
}

static inline HRESULT INcPainter_SetAttributeA_C(INcPainter* pThis, LPCSTR pszAttr, LPCSTR pszValue, BOOL bLoading)
{
    return INcPainter_SetAttributeA(pThis, pszAttr, pszValue, bLoading);
}

static inline BOOL INcPainter_GetAttribute_C(const INcPainter* pThis, const IStringW* strName, IStringW* strValue)
{
    return INcPainter_GetAttribute(pThis, strName, strValue);
}

static inline BOOL INcPainter_GetAttributeA_C(const INcPainter* pThis, LPCSTR pszAttr, IStringA* strValue)
{
    return INcPainter_GetAttributeA(pThis, pszAttr, strValue);
}

static inline void INcPainter_AfterAttribute_C(INcPainter* pThis)
{
    INcPainter_AfterAttribute(pThis);
}

static inline BOOL INcPainter_InitFromXml_C(INcPainter* pThis, IXmlNode* xmlNode)
{
    return INcPainter_InitFromXml(pThis, xmlNode);
}

static inline void INcPainter_OnInitFinished_C(INcPainter* pThis, IXmlNode* xmlNode)
{
    INcPainter_OnInitFinished(pThis, xmlNode);
}

static inline IWindow* INcPainter_GetRoot_C(INcPainter* pThis)
{
    return INcPainter_GetRoot(pThis);
}

/*
 * Convenience macros for common non-client painter operations
 */

/* Root window access shortcuts */
#define INcPainter_GetRootWindow(This) \
    INcPainter_GetRoot(This)

#define INcPainter_GetRootWnd(This) \
    INcPainter_GetRoot(This)

/* Object information shortcuts */
#define INcPainter_GetType(This) \
    INcPainter_GetObjectType(This)

#define INcPainter_GetClass(This) \
    INcPainter_GetObjectClass(This)

#define INcPainter_CheckClass(This, className) \
    INcPainter_IsClass(This, className)

#define INcPainter_GetInfo(This) \
    INcPainter_GetClassInfo(This)

/* Attribute management shortcuts */
#define INcPainter_SetAttr(This, name, value, loading) \
    INcPainter_SetAttribute(This, name, value, loading)

#define INcPainter_SetAttrA(This, name, value, loading) \
    INcPainter_SetAttributeA(This, name, value, loading)

#define INcPainter_GetAttr(This, name, value) \
    INcPainter_GetAttribute(This, name, value)

#define INcPainter_GetAttrA(This, name, value) \
    INcPainter_GetAttributeA(This, name, value)

#define INcPainter_FinishAttributes(This) \
    INcPainter_AfterAttribute(This)

/* XML initialization shortcuts */
#define INcPainter_LoadFromXml(This, xmlNode) \
    INcPainter_InitFromXml(This, xmlNode)

#define INcPainter_FinishInit(This, xmlNode) \
    INcPainter_OnInitFinished(This, xmlNode)

/* Safe non-client painter operations */
#define INcPainter_SafeGetRoot(This, ppRoot) \
    do { \
        if ((This) && (ppRoot)) { \
            *(ppRoot) = INcPainter_GetRoot(This); \
        } else if (ppRoot) { \
            *(ppRoot) = NULL; \
        } \
    } while(0)

#define INcPainter_SafeSetAttribute(This, name, value, loading) \
    ((This) && (name) && (value) ? INcPainter_SetAttribute(This, name, value, loading) : E_INVALIDARG)

#define INcPainter_SafeSetAttributeA(This, name, value, loading) \
    ((This) && (name) && (value) ? INcPainter_SetAttributeA(This, name, value, loading) : E_INVALIDARG)

#define INcPainter_SafeGetAttribute(This, name, value) \
    ((This) && (name) && (value) ? INcPainter_GetAttribute(This, name, value) : FALSE)

#define INcPainter_SafeInitFromXml(This, xmlNode) \
    ((This) && (xmlNode) ? INcPainter_InitFromXml(This, xmlNode) : FALSE)

/*
 * Non-client painter helper functions
 */
static inline BOOL INcPainter_IsValid_C(INcPainter* pThis)
{
    return (pThis != NULL);
}

static inline BOOL INcPainter_HasRoot_C(INcPainter* pThis)
{
    if (!pThis) return FALSE;
    
    return (INcPainter_GetRoot(pThis) != NULL);
}

static inline BOOL INcPainter_IsClassType_C(INcPainter* pThis, LPCWSTR className)
{
    if (!pThis || !className) return FALSE;
    
    return INcPainter_IsClass(pThis, className);
}

static inline LPCWSTR INcPainter_GetClassName_C(INcPainter* pThis)
{
    if (!pThis) return NULL;
    
    return INcPainter_GetObjectClass(pThis);
}

static inline int INcPainter_GetType_C(INcPainter* pThis)
{
    if (!pThis) return 0;
    
    return INcPainter_GetObjectType(pThis);
}

/*
 * Non-client painter state management
 */
typedef struct NcPainterState {
    INcPainter* ncPainter;
    IWindow* rootWindow;
    LPCWSTR className;
    int objectType;
    BOOL isValid;
    BOOL hasRoot;
} NcPainterState;

static inline void NcPainterState_Init(NcPainterState* state, INcPainter* ncPainter)
{
    if (state) {
        state->ncPainter = ncPainter;
        if (ncPainter) {
            state->rootWindow = INcPainter_GetRoot(ncPainter);
            state->className = INcPainter_GetObjectClass(ncPainter);
            state->objectType = INcPainter_GetObjectType(ncPainter);
            state->isValid = TRUE;
            state->hasRoot = (state->rootWindow != NULL);
        } else {
            memset(state, 0, sizeof(NcPainterState));
        }
    }
}

static inline void NcPainterState_Update(NcPainterState* state)
{
    if (state && state->ncPainter) {
        NcPainterState_Init(state, state->ncPainter);
    }
}

static inline BOOL NcPainterState_SetAttribute(NcPainterState* state, const IStringW* name, const IStringW* value, BOOL loading)
{
    if (!state || !state->isValid || !state->ncPainter) return FALSE;
    
    HRESULT hr = INcPainter_SetAttribute(state->ncPainter, name, value, loading);
    return SUCCEEDED(hr);
}

static inline BOOL NcPainterState_InitFromXml(NcPainterState* state, IXmlNode* xmlNode)
{
    if (!state || !state->isValid || !state->ncPainter || !xmlNode) return FALSE;
    
    BOOL result = INcPainter_InitFromXml(state->ncPainter, xmlNode);
    if (result) {
        NcPainterState_Update(state);
    }
    return result;
}

/*
 * Reference counting helpers
 */
#define INcPainter_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define INcPainter_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void INcPainter_DebugInfo_C(INcPainter* pThis)
{
    if (!pThis) {
        printf("NcPainter: NULL\n");
        return;
    }
    
    printf("NcPainter: %p\n", pThis);
    printf("  Class Name: %ls\n", INcPainter_GetObjectClass(pThis));
    printf("  Object Type: %d\n", INcPainter_GetObjectType(pThis));
    printf("  Has Root: %s\n", INcPainter_HasRoot_C(pThis) ? "Yes" : "No");
    printf("  Root Window: %p\n", INcPainter_GetRoot(pThis));
}
#else
#define INcPainter_DebugInfo_C(This) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SNCPAINTER_CAPI_H__ */
