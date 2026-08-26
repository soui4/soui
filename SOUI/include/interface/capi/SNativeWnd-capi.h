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
 * Interfaces covered:
 *   INativeWnd (inherits IObjRef)
 */

/* INativeWnd C API Macros (IObjRef base interface) */
#define INativeWnd_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define INativeWnd_Release(This) \
    ((This)->lpVtbl->Release(This))

#define INativeWnd_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* INativeWnd specific interface macros */
#define INativeWnd_CreateNative(This, lpWindowName, dwStyle, dwExStyle, x, y, nWidth, nHeight, hWndParent, nID, lpParam) \
    ((This)->lpVtbl->CreateNative(This, lpWindowName, dwStyle, dwExStyle, x, y, nWidth, nHeight, hWndParent, nID, lpParam))

#define INativeWnd_GetHwnd(This) \
    ((This)->lpVtbl->GetHwnd(This))

#define INativeWnd_SubclassWindow(This, hWnd) \
    ((This)->lpVtbl->SubclassWindow(This, hWnd))

#define INativeWnd_UnsubclassWindow(This, bForce) \
    ((This)->lpVtbl->UnsubclassWindow(This, bForce))

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

#define INativeWnd_ModifyStyleEx(This, dwRemove, dwAdd, nFlags) \
    ((This)->lpVtbl->ModifyStyleEx(This, dwRemove, dwAdd, nFlags))

#define INativeWnd_SetWindowPos(This, hWndInsertAfter, x, y, cx, cy, nFlags) \
    ((This)->lpVtbl->SetWindowPos(This, hWndInsertAfter, x, y, cx, cy, nFlags))

#define INativeWnd_CenterWindow(This, hWndCenter) \
    ((This)->lpVtbl->CenterWindow(This, hWndCenter))

#define INativeWnd_DestroyWindow(This) \
    ((This)->lpVtbl->DestroyWindow(This))

#define INativeWnd_IsWindow(This) \
    ((This)->lpVtbl->IsWindow(This))

#define INativeWnd_Invalidate(This, bErase) \
    ((This)->lpVtbl->Invalidate(This, bErase))

#define INativeWnd_InvalidateRect(This, lpRect, bErase) \
    ((This)->lpVtbl->InvalidateRect(This, lpRect, bErase))

#define INativeWnd_GetWindowRect(This, lpRect) \
    ((This)->lpVtbl->GetWindowRect(This, lpRect))

#define INativeWnd_GetClientRect(This, lpRect) \
    ((This)->lpVtbl->GetClientRect(This, lpRect))

#define INativeWnd_ClientToScreen(This, lpPoint) \
    ((This)->lpVtbl->ClientToScreen(This, lpPoint))

#define INativeWnd_ClientToScreen2(This, lpRect) \
    ((This)->lpVtbl->ClientToScreen2(This, lpRect))

#define INativeWnd_ScreenToClient(This, lpPoint) \
    ((This)->lpVtbl->ScreenToClient(This, lpPoint))

#define INativeWnd_ScreenToClient2(This, lpRect) \
    ((This)->lpVtbl->ScreenToClient2(This, lpRect))

#define INativeWnd_MapWindowPoints(This, hWndTo, lpPoint, nCount) \
    ((This)->lpVtbl->MapWindowPoints(This, hWndTo, lpPoint, nCount))

#define INativeWnd_MapWindowRect(This, hWndTo, lpRect) \
    ((This)->lpVtbl->MapWindowRect(This, hWndTo, lpRect))

#define INativeWnd_SetTimer(This, nIDEvent, nElapse, lpfnTimer) \
    ((This)->lpVtbl->SetTimer(This, nIDEvent, nElapse, lpfnTimer))

#define INativeWnd_KillTimer(This, nIDEvent) \
    ((This)->lpVtbl->KillTimer(This, nIDEvent))

#define INativeWnd_GetDC(This) \
    ((This)->lpVtbl->GetDC(This))

#define INativeWnd_GetWindowDC(This) \
    ((This)->lpVtbl->GetWindowDC(This))

#define INativeWnd_ReleaseDC(This, hDC) \
    ((This)->lpVtbl->ReleaseDC(This, hDC))

#define INativeWnd_CreateCaret(This, hBitmap, nWidth, nHeight) \
    ((This)->lpVtbl->CreateCaret(This, hBitmap, nWidth, nHeight))

#define INativeWnd_HideCaret(This) \
    ((This)->lpVtbl->HideCaret(This))

#define INativeWnd_ShowCaret(This) \
    ((This)->lpVtbl->ShowCaret(This))

#define INativeWnd_GetCapture(This) \
    ((This)->lpVtbl->GetCapture(This))

#define INativeWnd_SetCapture(This) \
    ((This)->lpVtbl->SetCapture(This))

#define INativeWnd_ReleaseCapture(This) \
    ((This)->lpVtbl->ReleaseCapture(This))

#define INativeWnd_SetFocus(This) \
    ((This)->lpVtbl->SetFocus(This))

#define INativeWnd_SendMessage(This, message, wParam, lParam) \
    ((This)->lpVtbl->SendMessage(This, message, wParam, lParam))

#define INativeWnd_PostMessage(This, message, wParam, lParam) \
    ((This)->lpVtbl->PostMessage(This, message, wParam, lParam))

#define INativeWnd_SendNotifyMessage(This, message, wParam, lParam) \
    ((This)->lpVtbl->SendNotifyMessage(This, message, wParam, lParam))

#define INativeWnd_SetWindowText(This, lpszString) \
    ((This)->lpVtbl->SetWindowText(This, lpszString))

#define INativeWnd_GetWindowText(This, lpszStringBuf, nMaxCount) \
    ((This)->lpVtbl->GetWindowText(This, lpszStringBuf, nMaxCount))

#define INativeWnd_IsIconic(This) \
    ((This)->lpVtbl->IsIconic(This))

#define INativeWnd_IsZoomed(This) \
    ((This)->lpVtbl->IsZoomed(This))

#define INativeWnd_IsWindowVisible(This) \
    ((This)->lpVtbl->IsWindowVisible(This))

#define INativeWnd_MoveWindow(This, x, y, nWidth, nHeight, bRepaint) \
    ((This)->lpVtbl->MoveWindow(This, x, y, nWidth, nHeight, bRepaint))

#define INativeWnd_MoveWindow2(This, lpRect, bRepaint) \
    ((This)->lpVtbl->MoveWindow2(This, lpRect, bRepaint))

#define INativeWnd_ShowWindow(This, nCmdShow) \
    ((This)->lpVtbl->ShowWindow(This, nCmdShow))

#define INativeWnd_UpdateWindow(This) \
    ((This)->lpVtbl->UpdateWindow(This))

#define INativeWnd_SetWindowRgn(This, hRgn, bRedraw) \
    ((This)->lpVtbl->SetWindowRgn(This, hRgn, bRedraw))

#define INativeWnd_SetLayeredWindowAttributes(This, crKey, bAlpha, dwFlags) \
    ((This)->lpVtbl->SetLayeredWindowAttributes(This, crKey, bAlpha, dwFlags))

#define INativeWnd_UpdateLayeredWindow(This, hdcDst, pptDst, psize, hdcSrc, pptSrc, crKey, pblend, dwFlags) \
    ((This)->lpVtbl->UpdateLayeredWindow(This, hdcDst, pptDst, psize, hdcSrc, pptSrc, crKey, pblend, dwFlags))

#define INativeWnd_SetMsgHandler(This, fun, ctx) \
    ((This)->lpVtbl->SetMsgHandler(This, fun, ctx))

#define INativeWnd_GetMsgHandler(This) \
    ((This)->lpVtbl->GetMsgHandler(This))

#ifdef __cplusplus
}
#endif

#endif /* __SNATIVEWND_CAPI_H__ */
