#ifndef __SNATIVEWND_CAPI_H__
#define __SNATIVEWND_CAPI_H__

#include "../SNativeWnd-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SNativeWnd Interface
 * These macros provide C-style function call syntax for C++ interface methods
 * 
 * Note: This covers the main methods from SNativeWndApi.h (517 lines total)
 * Additional methods can be added following the same pattern.
 */

/* INativeWnd C API Macros */
#define INativeWnd_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define INativeWnd_Release(This) \
    ((This)->lpVtbl->Release(This))

#define INativeWnd_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define INativeWnd_CreateNative(This, lpWindowName, dwStyle, dwExStyle, x, y, nWidth, nHeight, hWndParent, nID, lpParam) \
    ((This)->lpVtbl->CreateNative(This, lpWindowName, dwStyle, dwExStyle, x, y, nWidth, nHeight, hWndParent, nID, lpParam))

#define INativeWnd_GetHwnd(This) \
    ((This)->lpVtbl->GetHwnd(This))

#define INativeWnd_SubclassWindow(This, hWnd) \
    ((This)->lpVtbl->SubclassWindow(This, hWnd))

#define INativeWnd_UnsubclassWindow(This, bForce) \
    ((This)->lpVtbl->UnsubclassWindow(This, bForce))

#define INativeWnd_UnsubclassWindow_Default(This) \
    ((This)->lpVtbl->UnsubclassWindow(This, FALSE))

#define INativeWnd_GetCurrentMessage(This) \
    ((This)->lpVtbl->GetCurrentMessage(This))

#define INativeWnd_GetDlgCtrlID(This) \
    ((This)->lpVtbl->GetDlgCtrlID(This))

#define INativeWnd_GetStyle(This) \
    ((This)->lpVtbl->GetStyle(This))

#define INativeWnd_GetExStyle(This) \
    ((This)->lpVtbl->GetExStyle(This))

#define INativeWnd_GetWindowLongPtr(This, nIndex) \
    ((This)->lpVtbl->GetWindowLongPtr(This, nIndex))

#define INativeWnd_SetWindowLongPtr(This, nIndex, dwNewLong) \
    ((This)->lpVtbl->SetWindowLongPtr(This, nIndex, dwNewLong))

#define INativeWnd_GetParent(This) \
    ((This)->lpVtbl->GetParent(This))

#define INativeWnd_SetParent(This, hWndNewParent) \
    ((This)->lpVtbl->SetParent(This, hWndNewParent))

#define INativeWnd_IsWindowEnabled(This) \
    ((This)->lpVtbl->IsWindowEnabled(This))

#define INativeWnd_ModifyStyle(This, dwRemove, dwAdd, nFlags) \
    ((This)->lpVtbl->ModifyStyle(This, dwRemove, dwAdd, nFlags))

#define INativeWnd_ModifyStyle_Default(This, dwRemove, dwAdd) \
    ((This)->lpVtbl->ModifyStyle(This, dwRemove, dwAdd, 0))

#define INativeWnd_ModifyStyleEx(This, dwRemove, dwAdd, nFlags) \
    ((This)->lpVtbl->ModifyStyleEx(This, dwRemove, dwAdd, nFlags))

#define INativeWnd_ModifyStyleEx_Default(This, dwRemove, dwAdd) \
    ((This)->lpVtbl->ModifyStyleEx(This, dwRemove, dwAdd, 0))

#define INativeWnd_SetWindowPos(This, hWndInsertAfter, x, y, cx, cy, nFlags) \
    ((This)->lpVtbl->SetWindowPos(This, hWndInsertAfter, x, y, cx, cy, nFlags))

#define INativeWnd_CenterWindow(This, hWndCenter) \
    ((This)->lpVtbl->CenterWindow(This, hWndCenter))

#define INativeWnd_CenterWindow_Default(This) \
    ((This)->lpVtbl->CenterWindow(This, NULL))

#define INativeWnd_DestroyWindow(This) \
    ((This)->lpVtbl->DestroyWindow(This))

#define INativeWnd_IsWindow(This) \
    ((This)->lpVtbl->IsWindow(This))

#define INativeWnd_Invalidate(This, bErase) \
    ((This)->lpVtbl->Invalidate(This, bErase))

#define INativeWnd_Invalidate_Default(This) \
    ((This)->lpVtbl->Invalidate(This, TRUE))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* INativeWnd Helper Functions */
static inline long INativeWnd_AddRef_C(INativeWnd* pThis)
{
    return INativeWnd_AddRef(pThis);
}

static inline long INativeWnd_Release_C(INativeWnd* pThis)
{
    return INativeWnd_Release(pThis);
}

static inline HWND INativeWnd_CreateNative_C(INativeWnd* pThis, LPCTSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle, 
                                             int x, int y, int nWidth, int nHeight, HWND hWndParent, int nID, LPVOID lpParam)
{
    return INativeWnd_CreateNative(pThis, lpWindowName, dwStyle, dwExStyle, x, y, nWidth, nHeight, hWndParent, nID, lpParam);
}

static inline HWND INativeWnd_GetHwnd_C(INativeWnd* pThis)
{
    return INativeWnd_GetHwnd(pThis);
}

static inline BOOL INativeWnd_SubclassWindow_C(INativeWnd* pThis, HWND hWnd)
{
    return INativeWnd_SubclassWindow(pThis, hWnd);
}

static inline HWND INativeWnd_UnsubclassWindow_C(INativeWnd* pThis, BOOL bForce)
{
    return INativeWnd_UnsubclassWindow(pThis, bForce);
}

static inline const MSG* INativeWnd_GetCurrentMessage_C(INativeWnd* pThis)
{
    return INativeWnd_GetCurrentMessage(pThis);
}

static inline int INativeWnd_GetDlgCtrlID_C(INativeWnd* pThis)
{
    return INativeWnd_GetDlgCtrlID(pThis);
}

static inline DWORD INativeWnd_GetStyle_C(INativeWnd* pThis)
{
    return INativeWnd_GetStyle(pThis);
}

static inline DWORD INativeWnd_GetExStyle_C(INativeWnd* pThis)
{
    return INativeWnd_GetExStyle(pThis);
}

static inline BOOL INativeWnd_SetWindowPos_C(INativeWnd* pThis, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags)
{
    return INativeWnd_SetWindowPos(pThis, hWndInsertAfter, x, y, cx, cy, nFlags);
}

static inline BOOL INativeWnd_DestroyWindow_C(INativeWnd* pThis)
{
    return INativeWnd_DestroyWindow(pThis);
}

/*
 * Convenience macros for common native window operations
 */

/* Window creation shortcuts */
#define INativeWnd_CreateSimple(This, name, style, x, y, width, height, parent) \
    INativeWnd_CreateNative(This, name, style, 0, x, y, width, height, parent, 0, NULL)

#define INativeWnd_CreateChild(This, name, style, x, y, width, height, parent, id) \
    INativeWnd_CreateNative(This, name, style | WS_CHILD, 0, x, y, width, height, parent, id, NULL)

#define INativeWnd_CreatePopup(This, name, style, x, y, width, height) \
    INativeWnd_CreateNative(This, name, style | WS_POPUP, 0, x, y, width, height, NULL, 0, NULL)

/* Window positioning shortcuts */
#define INativeWnd_MoveWindow(This, x, y, width, height) \
    INativeWnd_SetWindowPos(This, NULL, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE)

#define INativeWnd_ResizeWindow(This, width, height) \
    INativeWnd_SetWindowPos(This, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE)

#define INativeWnd_MoveToTop(This) \
    INativeWnd_SetWindowPos(This, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE)

#define INativeWnd_MoveToBottom(This) \
    INativeWnd_SetWindowPos(This, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE)

#define INativeWnd_BringToFront(This) \
    INativeWnd_SetWindowPos(This, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE)

/* Style modification shortcuts */
#define INativeWnd_AddStyle(This, style) \
    INativeWnd_ModifyStyle_Default(This, 0, style)

#define INativeWnd_RemoveStyle(This, style) \
    INativeWnd_ModifyStyle_Default(This, style, 0)

#define INativeWnd_AddExStyle(This, style) \
    INativeWnd_ModifyStyleEx_Default(This, 0, style)

#define INativeWnd_RemoveExStyle(This, style) \
    INativeWnd_ModifyStyleEx_Default(This, style, 0)

/* Window state shortcuts */
#define INativeWnd_Show(This) \
    ShowWindow(INativeWnd_GetHwnd(This), SW_SHOW)

#define INativeWnd_Hide(This) \
    ShowWindow(INativeWnd_GetHwnd(This), SW_HIDE)

#define INativeWnd_Minimize(This) \
    ShowWindow(INativeWnd_GetHwnd(This), SW_MINIMIZE)

#define INativeWnd_Maximize(This) \
    ShowWindow(INativeWnd_GetHwnd(This), SW_MAXIMIZE)

#define INativeWnd_Restore(This) \
    ShowWindow(INativeWnd_GetHwnd(This), SW_RESTORE)

/* Invalidation shortcuts */
#define INativeWnd_Refresh(This) \
    INativeWnd_Invalidate_Default(This)

#define INativeWnd_RefreshNoErase(This) \
    INativeWnd_Invalidate(This, FALSE)

#define INativeWnd_Update(This) \
    UpdateWindow(INativeWnd_GetHwnd(This))

/* Safe native window operations */
#define INativeWnd_SafeGetHwnd(This) \
    ((This) ? INativeWnd_GetHwnd(This) : NULL)

#define INativeWnd_SafeDestroy(This) \
    ((This) && INativeWnd_IsWindow(This) ? INativeWnd_DestroyWindow(This) : FALSE)

#define INativeWnd_SafeSubclass(This, hWnd) \
    ((This) && IsWindow(hWnd) ? INativeWnd_SubclassWindow(This, hWnd) : FALSE)

#define INativeWnd_SafeUnsubclass(This, bForce) \
    ((This) && INativeWnd_IsWindow(This) ? INativeWnd_UnsubclassWindow(This, bForce) : NULL)

#define INativeWnd_SafeSetParent(This, parent) \
    ((This) && INativeWnd_IsWindow(This) ? INativeWnd_SetParent(This, parent) : NULL)

#define INativeWnd_SafeCenterWindow(This, center) \
    ((This) && INativeWnd_IsWindow(This) ? INativeWnd_CenterWindow(This, center) : FALSE)

/*
 * Message handler function type and helpers
 */
typedef BOOL (*FunMsgHandler_C)(const LPMSG pMsg, LRESULT* pRes, void* ctx);

static inline MsgHandlerInfo MsgHandlerInfo_Create(FunMsgHandler_C handler, void* context)
{
    MsgHandlerInfo info;
    info.fun = (FunMsgHandler)handler;
    info.ctx = context;
    return info;
}

/*
 * Reference counting helpers
 */
#define INativeWnd_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define INativeWnd_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Native window state management
 */
typedef struct NativeWndState {
    INativeWnd* nativeWnd;
    HWND hwnd;
    DWORD style;
    DWORD exStyle;
    HWND parent;
    int ctrlId;
    BOOL isValid;
    BOOL isEnabled;
} NativeWndState;

static inline void NativeWndState_Init(NativeWndState* state, INativeWnd* nativeWnd)
{
    if (state) {
        state->nativeWnd = nativeWnd;
        if (nativeWnd) {
            state->hwnd = INativeWnd_GetHwnd(nativeWnd);
            state->style = INativeWnd_GetStyle(nativeWnd);
            state->exStyle = INativeWnd_GetExStyle(nativeWnd);
            state->parent = INativeWnd_GetParent(nativeWnd);
            state->ctrlId = INativeWnd_GetDlgCtrlID(nativeWnd);
            state->isValid = INativeWnd_IsWindow(nativeWnd);
            state->isEnabled = INativeWnd_IsWindowEnabled(nativeWnd);
        } else {
            memset(state, 0, sizeof(NativeWndState));
        }
    }
}

static inline void NativeWndState_Update(NativeWndState* state)
{
    if (state && state->nativeWnd) {
        NativeWndState_Init(state, state->nativeWnd);
    }
}

static inline BOOL NativeWndState_IsValid(const NativeWndState* state)
{
    return state && state->isValid && state->hwnd && IsWindow(state->hwnd);
}

/*
 * Note: Additional methods from SNativeWndApi.h can be added here
 * The file contains 517 lines with many more window management methods
 * including message handling, drawing, focus management, etc.
 */

#ifdef __cplusplus
}
#endif

#endif /* __SNATIVEWND_CAPI_H__ */
