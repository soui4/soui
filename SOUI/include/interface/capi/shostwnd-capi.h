#ifndef __SHOSTWND_CAPI_H__
#define __SHOSTWND_CAPI_H__

#include "../shostwnd-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for IHostWnd / IHostDialog Interface
 * These macros provide C-style function call syntax for C++ interface methods
 * IHostWnd inherits from INativeWnd; IHostDialog inherits from IHostWnd.
 */

/* IObjRef base interface macros */
#define IHostWnd_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IHostWnd_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IHostWnd_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* INativeWnd base interface macros */
#define IHostWnd_CreateNative(This, lpWindowName, dwStyle, dwExStyle, x, y, nWidth, nHeight, hWndParent, nID, lpParam) \
    ((This)->lpVtbl->CreateNative(This, lpWindowName, dwStyle, dwExStyle, x, y, nWidth, nHeight, hWndParent, nID, lpParam))

#define IHostWnd_GetHwnd(This) \
    ((This)->lpVtbl->GetHwnd(This))

#define IHostWnd_SubclassWindow(This, hWnd) \
    ((This)->lpVtbl->SubclassWindow(This, hWnd))

#define IHostWnd_UnsubclassWindow(This, bForce) \
    ((This)->lpVtbl->UnsubclassWindow(This, bForce))

#define IHostWnd_GetCurrentMessage(This) \
    ((This)->lpVtbl->GetCurrentMessage(This))

#define IHostWnd_GetDlgCtrlID(This) \
    ((This)->lpVtbl->GetDlgCtrlID(This))

#define IHostWnd_GetStyle(This) \
    ((This)->lpVtbl->GetStyle(This))

#define IHostWnd_GetExStyle(This) \
    ((This)->lpVtbl->GetExStyle(This))

#define IHostWnd_GetWindowLongPtr(This, nIndex) \
    ((This)->lpVtbl->GetWindowLongPtr(This, nIndex))

#define IHostWnd_SetWindowLongPtr(This, nIndex, dwNewLong) \
    ((This)->lpVtbl->SetWindowLongPtr(This, nIndex, dwNewLong))

#define IHostWnd_GetParent(This) \
    ((This)->lpVtbl->GetParent(This))

#define IHostWnd_SetParent(This, hWndNewParent) \
    ((This)->lpVtbl->SetParent(This, hWndNewParent))

#define IHostWnd_IsWindowEnabled(This) \
    ((This)->lpVtbl->IsWindowEnabled(This))

#define IHostWnd_ModifyStyle(This, dwRemove, dwAdd, nFlags) \
    ((This)->lpVtbl->ModifyStyle(This, dwRemove, dwAdd, nFlags))

#define IHostWnd_ModifyStyleEx(This, dwRemove, dwAdd, nFlags) \
    ((This)->lpVtbl->ModifyStyleEx(This, dwRemove, dwAdd, nFlags))

#define IHostWnd_SetWindowPos(This, hWndInsertAfter, x, y, cx, cy, nFlags) \
    ((This)->lpVtbl->SetWindowPos(This, hWndInsertAfter, x, y, cx, cy, nFlags))

#define IHostWnd_CenterWindow(This, hWndCenter) \
    ((This)->lpVtbl->CenterWindow(This, hWndCenter))

#define IHostWnd_DestroyWindow(This) \
    ((This)->lpVtbl->DestroyWindow(This))

#define IHostWnd_IsWindow(This) \
    ((This)->lpVtbl->IsWindow(This))

#define IHostWnd_Invalidate(This, bErase) \
    ((This)->lpVtbl->Invalidate(This, bErase))

#define IHostWnd_InvalidateRect(This, lpRect, bErase) \
    ((This)->lpVtbl->InvalidateRect(This, lpRect, bErase))

#define IHostWnd_GetWindowRect(This, lpRect) \
    ((This)->lpVtbl->GetWindowRect(This, lpRect))

#define IHostWnd_GetClientRect(This, lpRect) \
    ((This)->lpVtbl->GetClientRect(This, lpRect))

#define IHostWnd_ClientToScreen(This, lpPoint) \
    ((This)->lpVtbl->ClientToScreen(This, lpPoint))

#define IHostWnd_ClientToScreen2(This, lpRect) \
    ((This)->lpVtbl->ClientToScreen2(This, lpRect))

#define IHostWnd_ScreenToClient(This, lpPoint) \
    ((This)->lpVtbl->ScreenToClient(This, lpPoint))

#define IHostWnd_ScreenToClient2(This, lpRect) \
    ((This)->lpVtbl->ScreenToClient2(This, lpRect))

#define IHostWnd_MapWindowPoints(This, hWndTo, lpPoint, nCount) \
    ((This)->lpVtbl->MapWindowPoints(This, hWndTo, lpPoint, nCount))

#define IHostWnd_MapWindowRect(This, hWndTo, lpRect) \
    ((This)->lpVtbl->MapWindowRect(This, hWndTo, lpRect))

#define IHostWnd_SetTimer(This, nIDEvent, nElapse, lpfnTimer) \
    ((This)->lpVtbl->SetTimer(This, nIDEvent, nElapse, lpfnTimer))

#define IHostWnd_KillTimer(This, nIDEvent) \
    ((This)->lpVtbl->KillTimer(This, nIDEvent))

#define IHostWnd_GetDC(This) \
    ((This)->lpVtbl->GetDC(This))

#define IHostWnd_GetWindowDC(This) \
    ((This)->lpVtbl->GetWindowDC(This))

#define IHostWnd_ReleaseDC(This, hDC) \
    ((This)->lpVtbl->ReleaseDC(This, hDC))

#define IHostWnd_CreateCaret(This, hBitmap, nWidth, nHeight) \
    ((This)->lpVtbl->CreateCaret(This, hBitmap, nWidth, nHeight))

#define IHostWnd_HideCaret(This) \
    ((This)->lpVtbl->HideCaret(This))

#define IHostWnd_ShowCaret(This) \
    ((This)->lpVtbl->ShowCaret(This))

#define IHostWnd_GetCapture(This) \
    ((This)->lpVtbl->GetCapture(This))

#define IHostWnd_SetCapture(This) \
    ((This)->lpVtbl->SetCapture(This))

#define IHostWnd_ReleaseCapture(This) \
    ((This)->lpVtbl->ReleaseCapture(This))

#define IHostWnd_SetFocus(This) \
    ((This)->lpVtbl->SetFocus(This))

#define IHostWnd_SendMessage(This, message, wParam, lParam) \
    ((This)->lpVtbl->SendMessage(This, message, wParam, lParam))

#define IHostWnd_PostMessage(This, message, wParam, lParam) \
    ((This)->lpVtbl->PostMessage(This, message, wParam, lParam))

#define IHostWnd_SendNotifyMessage(This, message, wParam, lParam) \
    ((This)->lpVtbl->SendNotifyMessage(This, message, wParam, lParam))

#define IHostWnd_SetWindowText(This, lpszString) \
    ((This)->lpVtbl->SetWindowText(This, lpszString))

#define IHostWnd_GetWindowText(This, lpszStringBuf, nMaxCount) \
    ((This)->lpVtbl->GetWindowText(This, lpszStringBuf, nMaxCount))

#define IHostWnd_IsIconic(This) \
    ((This)->lpVtbl->IsIconic(This))

#define IHostWnd_IsZoomed(This) \
    ((This)->lpVtbl->IsZoomed(This))

#define IHostWnd_IsWindowVisible(This) \
    ((This)->lpVtbl->IsWindowVisible(This))

#define IHostWnd_MoveWindow(This, x, y, nWidth, nHeight, bRepaint) \
    ((This)->lpVtbl->MoveWindow(This, x, y, nWidth, nHeight, bRepaint))

#define IHostWnd_MoveWindow2(This, lpRect, bRepaint) \
    ((This)->lpVtbl->MoveWindow2(This, lpRect, bRepaint))

#define IHostWnd_ShowWindow(This, nCmdShow) \
    ((This)->lpVtbl->ShowWindow(This, nCmdShow))

#define IHostWnd_UpdateWindow(This) \
    ((This)->lpVtbl->UpdateWindow(This))

#define IHostWnd_SetWindowRgn(This, hRgn, bRedraw) \
    ((This)->lpVtbl->SetWindowRgn(This, hRgn, bRedraw))

#define IHostWnd_SetLayeredWindowAttributes(This, crKey, bAlpha, dwFlags) \
    ((This)->lpVtbl->SetLayeredWindowAttributes(This, crKey, bAlpha, dwFlags))

#define IHostWnd_UpdateLayeredWindow(This, hdcDst, pptDst, psize, hdcSrc, pptSrc, crKey, pblend, dwFlags) \
    ((This)->lpVtbl->UpdateLayeredWindow(This, hdcDst, pptDst, psize, hdcSrc, pptSrc, crKey, pblend, dwFlags))

#define IHostWnd_SetMsgHandler(This, fun, ctx) \
    ((This)->lpVtbl->SetMsgHandler(This, fun, ctx))

#define IHostWnd_GetMsgHandler(This) \
    ((This)->lpVtbl->GetMsgHandler(This))

/* IHostWnd specific interface macros */
#define IHostWnd_CreateEx(This, hWndParent, dwStyle, dwExStyle, x, y, nWidth, nHeight, xmlInit) \
    ((This)->lpVtbl->CreateEx(This, hWndParent, dwStyle, dwExStyle, x, y, nWidth, nHeight, xmlInit))

#define IHostWnd_CreateEx_Default(This, hWndParent, dwStyle, dwExStyle, x, y, nWidth, nHeight) \
    ((This)->lpVtbl->CreateEx(This, hWndParent, dwStyle, dwExStyle, x, y, nWidth, nHeight, NULL))

#define IHostWnd_Create(This, hWndParent, x, y, nWidth, nHeight) \
    ((This)->lpVtbl->Create(This, hWndParent, x, y, nWidth, nHeight))

#define IHostWnd_Create_Default(This, hWndParent) \
    ((This)->lpVtbl->Create(This, hWndParent, 0, 0, 0, 0))

#define IHostWnd_Attach(This, hWnd, xmlInit) \
    ((This)->lpVtbl->Attach(This, hWnd, xmlInit))

#define IHostWnd_Attach_Default(This, hWnd) \
    ((This)->lpVtbl->Attach(This, hWnd, NULL))

#define IHostWnd_Detach(This) \
    ((This)->lpVtbl->Detach(This))

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

#define IHostWnd_FindIChildByID(This, nId) \
    ((This)->lpVtbl->FindIChildByID(This, nId))

#define IHostWnd_FindIChildByName(This, pszName) \
    ((This)->lpVtbl->FindIChildByName(This, pszName))

#define IHostWnd_FindIChildByNameA(This, pszName) \
    ((This)->lpVtbl->FindIChildByNameA(This, pszName))

#define IHostWnd_GetNcPainter(This) \
    ((This)->lpVtbl->GetNcPainter(This))

#define IHostWnd_SetEventHandler(This, fun, ctx) \
    ((This)->lpVtbl->SetEventHandler(This, fun, ctx))

#define IHostWnd_GetEventHandler(This) \
    ((This)->lpVtbl->GetEventHandler(This))

#define IHostWnd_AnimateHostWindow(This, dwTime, dwFlags) \
    ((This)->lpVtbl->AnimateHostWindow(This, dwTime, dwFlags))

#define IHostWnd_EnableDragDrop(This) \
    ((This)->lpVtbl->EnableDragDrop(This))

#define IHostWnd_ShowHostWnd(This, uShowCmd, bWaitAniDone) \
    ((This)->lpVtbl->ShowHostWnd(This, uShowCmd, bWaitAniDone))

#define IHostWnd_EnablePrivateUiDef(This, bEnable) \
    ((This)->lpVtbl->EnablePrivateUiDef(This, bEnable))

#define IHostWnd_SetScale(This, nScale, pDestRect) \
    ((This)->lpVtbl->SetScale(This, nScale, pDestRect))

/*
 * IHostDialog interface macros
 * IHostDialog inherits from IHostWnd; use IHostWnd_ macros for inherited methods.
 */

/* IHostDialog specific interface macros */
#define IHostDialog_DoModal(This, hParent, dwStyle, dwExStyle) \
    ((This)->lpVtbl->DoModal(This, hParent, dwStyle, dwExStyle))

#define IHostDialog_DoModal_Default(This) \
    ((This)->lpVtbl->DoModal(This, NULL, WS_POPUP | WS_CLIPCHILDREN, 0))

#define IHostDialog_EndDialog(This, nResult) \
    ((This)->lpVtbl->EndDialog(This, nResult))

#ifdef __cplusplus
}
#endif

#endif /* __SHOSTWND_CAPI_H__ */
