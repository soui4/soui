#ifndef __SHOSTWND_CAPI_H__
#define __SHOSTWND_CAPI_H__

#include "../shostwnd-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for shostwnd Interface
 * These macros provide C-style function call syntax for C++ interface methods
 * Note: This covers the main methods from shostwnd-i.h (364 lines total)
 */

/* IHostWnd C API Macros (inherits from INativeWnd) */
#define IHostWnd_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IHostWnd_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IHostWnd_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IHostWnd_CreateNative(This, lpWindowName, dwStyle, dwExStyle, x, y, nWidth, nHeight, hWndParent, nID, lpParam) \
    ((This)->lpVtbl->CreateNative(This, lpWindowName, dwStyle, dwExStyle, x, y, nWidth, nHeight, hWndParent, nID, lpParam))

#define IHostWnd_GetHwnd(This) \
    ((This)->lpVtbl->GetHwnd(This))

#define IHostWnd_SubclassWindow(This, hWnd) \
    ((This)->lpVtbl->SubclassWindow(This, hWnd))

#define IHostWnd_UnsubclassWindow(This, bForce) \
    ((This)->lpVtbl->UnsubclassWindow(This, bForce))

#define IHostWnd_CreateEx(This, hWndParent, dwStyle, dwExStyle, x, y, nWidth, nHeight, xmlInit) \
    ((This)->lpVtbl->CreateEx(This, hWndParent, dwStyle, dwExStyle, x, y, nWidth, nHeight, xmlInit))

#define IHostWnd_CreateEx_Default(This, hWndParent, dwStyle, dwExStyle, x, y, nWidth, nHeight) \
    ((This)->lpVtbl->CreateEx(This, hWndParent, dwStyle, dwExStyle, x, y, nWidth, nHeight, NULL))

#define IHostWnd_Create(This, hWndParent, x, y, nWidth, nHeight) \
    ((This)->lpVtbl->Create(This, hWndParent, x, y, nWidth, nHeight))

#define IHostWnd_Create_Default(This, hWndParent) \
    ((This)->lpVtbl->Create(This, hWndParent, 0, 0, 0, 0))

#define IHostWnd_SetLayoutId(This, pszLayoutId) \
    ((This)->lpVtbl->SetLayoutId(This, pszLayoutId))

#define IHostWnd_InitFromXml(This, pNode) \
    ((This)->lpVtbl->InitFromXml(This, pNode))

#define IHostWnd_GetIRoot(This) \
    ((This)->lpVtbl->GetIRoot(This))

#define IHostWnd_IsTranslucent(This) \
    ((This)->lpVtbl->IsTranslucent(This))

#define IHostWnd_GetPresenter(This) \
    ((This)->lpVtbl->GetPresenter(This))

#define IHostWnd_SetPresenter(This, pPresenter) \
    ((This)->lpVtbl->SetPresenter(This, pPresenter))

#define IHostWnd_GetMsgLoop(This) \
    ((This)->lpVtbl->GetMsgLoop(This))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IHostWnd Helper Functions */
static inline long IHostWnd_AddRef_C(IHostWnd* pThis)
{
    return IHostWnd_AddRef(pThis);
}

static inline long IHostWnd_Release_C(IHostWnd* pThis)
{
    return IHostWnd_Release(pThis);
}

static inline HWND IHostWnd_CreateEx_C(IHostWnd* pThis, HWND hWndParent, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, IXmlNode* xmlInit)
{
    return IHostWnd_CreateEx(pThis, hWndParent, dwStyle, dwExStyle, x, y, nWidth, nHeight, xmlInit);
}

static inline HWND IHostWnd_Create_C(IHostWnd* pThis, HWND hWndParent, int x, int y, int nWidth, int nHeight)
{
    return IHostWnd_Create(pThis, hWndParent, x, y, nWidth, nHeight);
}

static inline void IHostWnd_SetLayoutId_C(IHostWnd* pThis, LPCTSTR pszLayoutId)
{
    IHostWnd_SetLayoutId(pThis, pszLayoutId);
}

static inline BOOL IHostWnd_InitFromXml_C(IHostWnd* pThis, IXmlNode* pNode)
{
    return IHostWnd_InitFromXml(pThis, pNode);
}

static inline IWindow* IHostWnd_GetIRoot_C(IHostWnd* pThis)
{
    return IHostWnd_GetIRoot(pThis);
}

static inline BOOL IHostWnd_IsTranslucent_C(const IHostWnd* pThis)
{
    return IHostWnd_IsTranslucent(pThis);
}

static inline IHostPresenter* IHostWnd_GetPresenter_C(IHostWnd* pThis)
{
    return IHostWnd_GetPresenter(pThis);
}

static inline void IHostWnd_SetPresenter_C(IHostWnd* pThis, IHostPresenter* pPresenter)
{
    IHostWnd_SetPresenter(pThis, pPresenter);
}

static inline IMessageLoop* IHostWnd_GetMsgLoop_C(IHostWnd* pThis)
{
    return IHostWnd_GetMsgLoop(pThis);
}

static inline HWND IHostWnd_GetHwnd_C(IHostWnd* pThis)
{
    return IHostWnd_GetHwnd(pThis);
}

/*
 * Convenience macros for common host window operations
 */

/* Window creation shortcuts */
#define IHostWnd_CreateSimple(This, parent, x, y, width, height) \
    IHostWnd_Create(This, parent, x, y, width, height)

#define IHostWnd_CreateWithStyle(This, parent, style, exStyle, x, y, width, height) \
    IHostWnd_CreateEx_Default(This, parent, style, exStyle, x, y, width, height)

#define IHostWnd_CreateWithXml(This, parent, style, exStyle, x, y, width, height, xml) \
    IHostWnd_CreateEx(This, parent, style, exStyle, x, y, width, height, xml)

#define IHostWnd_CreateChild(This, parent) \
    IHostWnd_Create_Default(This, parent)

/* Layout and initialization shortcuts */
#define IHostWnd_SetLayout(This, layoutId) \
    IHostWnd_SetLayoutId(This, layoutId)

#define IHostWnd_LoadFromXml(This, xmlNode) \
    IHostWnd_InitFromXml(This, xmlNode)

#define IHostWnd_Initialize(This, xmlNode) \
    IHostWnd_InitFromXml(This, xmlNode)

/* Root window shortcuts */
#define IHostWnd_GetRoot(This) \
    IHostWnd_GetIRoot(This)

#define IHostWnd_GetRootWindow(This) \
    IHostWnd_GetIRoot(This)

#define IHostWnd_GetDuiRoot(This) \
    IHostWnd_GetIRoot(This)

/* Presenter shortcuts */
#define IHostWnd_GetHostPresenter(This) \
    IHostWnd_GetPresenter(This)

#define IHostWnd_SetHostPresenter(This, presenter) \
    IHostWnd_SetPresenter(This, presenter)

/* Message loop shortcuts */
#define IHostWnd_GetMessageLoop(This) \
    IHostWnd_GetMsgLoop(This)

/* Transparency shortcuts */
#define IHostWnd_IsTransparent(This) \
    IHostWnd_IsTranslucent(This)

#define IHostWnd_HasTransparency(This) \
    IHostWnd_IsTranslucent(This)

/* Safe host window operations */
#define IHostWnd_SafeCreateEx(This, parent, style, exStyle, x, y, width, height, xml, phWnd) \
    do { \
        if ((This) && (phWnd)) { \
            *(phWnd) = IHostWnd_CreateEx(This, parent, style, exStyle, x, y, width, height, xml); \
        } else if (phWnd) { \
            *(phWnd) = NULL; \
        } \
    } while(0)

#define IHostWnd_SafeCreate(This, parent, x, y, width, height, phWnd) \
    do { \
        if ((This) && (phWnd)) { \
            *(phWnd) = IHostWnd_Create(This, parent, x, y, width, height); \
        } else if (phWnd) { \
            *(phWnd) = NULL; \
        } \
    } while(0)

#define IHostWnd_SafeSetLayoutId(This, layoutId) \
    do { \
        if ((This) && (layoutId)) { \
            IHostWnd_SetLayoutId(This, layoutId); \
        } \
    } while(0)

#define IHostWnd_SafeInitFromXml(This, xmlNode) \
    ((This) && (xmlNode) ? IHostWnd_InitFromXml(This, xmlNode) : FALSE)

#define IHostWnd_SafeGetRoot(This, ppRoot) \
    do { \
        if ((This) && (ppRoot)) { \
            *(ppRoot) = IHostWnd_GetIRoot(This); \
        } else if (ppRoot) { \
            *(ppRoot) = NULL; \
        } \
    } while(0)

#define IHostWnd_SafeSetPresenter(This, presenter) \
    do { \
        if ((This) && (presenter)) { \
            IHostWnd_SetPresenter(This, presenter); \
        } \
    } while(0)

/*
 * Host window helper functions
 */
static inline BOOL IHostWnd_IsValid_C(IHostWnd* pThis)
{
    return (pThis != NULL);
}

static inline BOOL IHostWnd_HasValidHwnd_C(IHostWnd* pThis)
{
    if (!pThis) return FALSE;
    
    HWND hwnd = IHostWnd_GetHwnd(pThis);
    return (hwnd != NULL && IsWindow(hwnd));
}

static inline BOOL IHostWnd_HasRoot_C(IHostWnd* pThis)
{
    if (!pThis) return FALSE;
    
    return (IHostWnd_GetIRoot(pThis) != NULL);
}

static inline BOOL IHostWnd_HasPresenter_C(IHostWnd* pThis)
{
    if (!pThis) return FALSE;
    
    return (IHostWnd_GetPresenter(pThis) != NULL);
}

static inline BOOL IHostWnd_HasMsgLoop_C(IHostWnd* pThis)
{
    if (!pThis) return FALSE;
    
    return (IHostWnd_GetMsgLoop(pThis) != NULL);
}

/*
 * Host window state management
 */
typedef struct HostWndState {
    IHostWnd* hostWnd;
    HWND hwnd;
    IWindow* rootWindow;
    IHostPresenter* presenter;
    IMessageLoop* msgLoop;
    BOOL isTranslucent;
    BOOL isValid;
    BOOL hasValidHwnd;
    BOOL hasRoot;
    BOOL hasPresenter;
    BOOL hasMsgLoop;
} HostWndState;

static inline void HostWndState_Init(HostWndState* state, IHostWnd* hostWnd)
{
    if (state) {
        state->hostWnd = hostWnd;
        if (hostWnd) {
            state->hwnd = IHostWnd_GetHwnd(hostWnd);
            state->rootWindow = IHostWnd_GetIRoot(hostWnd);
            state->presenter = IHostWnd_GetPresenter(hostWnd);
            state->msgLoop = IHostWnd_GetMsgLoop(hostWnd);
            state->isTranslucent = IHostWnd_IsTranslucent(hostWnd);
            state->isValid = TRUE;
            state->hasValidHwnd = IHostWnd_HasValidHwnd_C(hostWnd);
            state->hasRoot = (state->rootWindow != NULL);
            state->hasPresenter = (state->presenter != NULL);
            state->hasMsgLoop = (state->msgLoop != NULL);
        } else {
            memset(state, 0, sizeof(HostWndState));
        }
    }
}

static inline void HostWndState_Update(HostWndState* state)
{
    if (state && state->hostWnd) {
        HostWndState_Init(state, state->hostWnd);
    }
}

static inline HWND HostWndState_CreateWindow(HostWndState* state, HWND parent, int x, int y, int width, int height)
{
    if (!state || !state->hostWnd) return NULL;
    
    HWND hwnd = IHostWnd_Create(state->hostWnd, parent, x, y, width, height);
    if (hwnd) {
        HostWndState_Update(state);
    }
    return hwnd;
}

static inline BOOL HostWndState_InitFromXml(HostWndState* state, IXmlNode* xmlNode)
{
    if (!state || !state->hostWnd || !xmlNode) return FALSE;
    
    BOOL result = IHostWnd_InitFromXml(state->hostWnd, xmlNode);
    if (result) {
        HostWndState_Update(state);
    }
    return result;
}

/*
 * Reference counting helpers
 */
#define IHostWnd_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IHostWnd_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void IHostWnd_DebugInfo_C(IHostWnd* pThis)
{
    if (!pThis) {
        printf("HostWnd: NULL\n");
        return;
    }
    
    printf("HostWnd: %p\n", pThis);
    printf("  HWND: %p\n", IHostWnd_GetHwnd(pThis));
    printf("  Has Valid HWND: %s\n", IHostWnd_HasValidHwnd_C(pThis) ? "Yes" : "No");
    printf("  Root Window: %p\n", IHostWnd_GetIRoot(pThis));
    printf("  Has Root: %s\n", IHostWnd_HasRoot_C(pThis) ? "Yes" : "No");
    printf("  Presenter: %p\n", IHostWnd_GetPresenter(pThis));
    printf("  Has Presenter: %s\n", IHostWnd_HasPresenter_C(pThis) ? "Yes" : "No");
    printf("  Message Loop: %p\n", IHostWnd_GetMsgLoop(pThis));
    printf("  Has MsgLoop: %s\n", IHostWnd_HasMsgLoop_C(pThis) ? "Yes" : "No");
    printf("  Is Translucent: %s\n", IHostWnd_IsTranslucent(pThis) ? "Yes" : "No");
}

static inline void HostWndState_DebugInfo(const HostWndState* state)
{
    if (!state) {
        printf("HostWndState: NULL\n");
        return;
    }
    
    printf("HostWndState: %p\n", state);
    printf("  HostWnd: %p\n", state->hostWnd);
    printf("  HWND: %p\n", state->hwnd);
    printf("  Valid: %s\n", state->isValid ? "Yes" : "No");
    printf("  Has Valid HWND: %s\n", state->hasValidHwnd ? "Yes" : "No");
    printf("  Has Root: %s\n", state->hasRoot ? "Yes" : "No");
    printf("  Has Presenter: %s\n", state->hasPresenter ? "Yes" : "No");
    printf("  Has MsgLoop: %s\n", state->hasMsgLoop ? "Yes" : "No");
    printf("  Is Translucent: %s\n", state->isTranslucent ? "Yes" : "No");
}
#else
#define IHostWnd_DebugInfo_C(This) ((void)0)
#define HostWndState_DebugInfo(state) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SHOSTWND_CAPI_H__ */
