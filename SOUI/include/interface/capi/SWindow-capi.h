#ifndef __SWINDOW_CAPI_H__
#define __SWINDOW_CAPI_H__

#include "../SWindow-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for IWindow Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IObject base interface macros */
#define IWindow_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IWindow_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IWindow_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IWindow_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define IWindow_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IWindow_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define IWindow_GetID(This) \
    ((This)->lpVtbl->GetID(This))

#define IWindow_SetID(This, uID) \
    ((This)->lpVtbl->SetID(This, uID))

#define IWindow_InitFromXml(This, pNode) \
    ((This)->lpVtbl->InitFromXml(This, pNode))

#define IWindow_OnInitFinished(This, pNode) \
    ((This)->lpVtbl->OnInitFinished(This, pNode))

/* IWindow specific interface macros */
#define IWindow_GetSwnd(This) \
    ((This)->lpVtbl->GetSwnd(This))

#define IWindow_GetContainer(This) \
    ((This)->lpVtbl->GetContainer(This))

#define IWindow_SetContainer(This, pContainer) \
    ((This)->lpVtbl->SetContainer(This, pContainer))

#define IWindow_GetParent(This) \
    ((This)->lpVtbl->GetParent(This))

#define IWindow_SetParent(This, pParent) \
    ((This)->lpVtbl->SetParent(This, pParent))

#define IWindow_GetTopLevelParent(This) \
    ((This)->lpVtbl->GetTopLevelParent(This))

#define IWindow_GetWindow(This, uCode) \
    ((This)->lpVtbl->GetWindow(This, uCode))

#define IWindow_GetChild(This, uCode) \
    ((This)->lpVtbl->GetChild(This, uCode))

#define IWindow_CreateChildren(This, pNode) \
    ((This)->lpVtbl->CreateChildren(This, pNode))

#define IWindow_DestroyChild(This, pChild) \
    ((This)->lpVtbl->DestroyChild(This, pChild))

#define IWindow_GetChildrenCount(This) \
    ((This)->lpVtbl->GetChildrenCount(This))

#define IWindow_FindChildByID(This, uID, nDeep) \
    ((This)->lpVtbl->FindChildByID(This, uID, nDeep))

#define IWindow_FindChildByName(This, pszName, nDeep) \
    ((This)->lpVtbl->FindChildByName(This, pszName, nDeep))

#define IWindow_FindChildByNameA(This, pszName, nDeep) \
    ((This)->lpVtbl->FindChildByNameA(This, pszName, nDeep))

#define IWindow_CreateChildrenFromString(This, pszXml) \
    ((This)->lpVtbl->CreateChildrenFromString(This, pszXml))

#define IWindow_GetWindowRect(This, prect) \
    ((This)->lpVtbl->GetWindowRect(This, prect))

#define IWindow_GetClientRect(This, prect) \
    ((This)->lpVtbl->GetClientRect(This, prect))

#define IWindow_IsContainPoint(This, pt, bClientOnly) \
    ((This)->lpVtbl->IsContainPoint(This, pt, bClientOnly))

#define IWindow_SwndFromPoint(This, pt, bIncludeMsgTransparent) \
    ((This)->lpVtbl->SwndFromPoint(This, pt, bIncludeMsgTransparent))

#define IWindow_Move(This, prect) \
    ((This)->lpVtbl->Move(This, prect))

#define IWindow_Move2(This, x, y, cx, cy) \
    ((This)->lpVtbl->Move2(This, x, y, cx, cy))

#define IWindow_SetWindowPos(This, hWndInsertAfter, x, y, cx, cy, uFlags) \
    ((This)->lpVtbl->SetWindowPos(This, hWndInsertAfter, x, y, cx, cy, uFlags))

#define IWindow_GetDesiredSize(This, pRet, nParentWid, nParentHei) \
    ((This)->lpVtbl->GetDesiredSize(This, pRet, nParentWid, nParentHei))

#define IWindow_GetDesiredSize2(This, pRet, szParent) \
    ((This)->lpVtbl->GetDesiredSize2(This, pRet, szParent))

#define IWindow_AdjustIWindowSize(This, szParent) \
    ((This)->lpVtbl->AdjustIWindowSize(This, szParent))

#define IWindow_SetWindowText(This, lpszText) \
    ((This)->lpVtbl->SetWindowText(This, lpszText))

#define IWindow_GetWindowText(This, bRawText) \
    ((This)->lpVtbl->GetWindowText(This, bRawText))

#define IWindow_GetWindowTextLength(This) \
    ((This)->lpVtbl->GetWindowTextLength(This))

#define IWindow_SendSwndMessage(This, uMsg, wParam, lParam, pbMsgHandled) \
    ((This)->lpVtbl->SendSwndMessage(This, uMsg, wParam, lParam, pbMsgHandled))

#define IWindow_PostSwndMessage(This, uMsg, wParam, lParam) \
    ((This)->lpVtbl->PostSwndMessage(This, uMsg, wParam, lParam))

#define IWindow_SetSwndProc(This, pfnWndProc, bAfter) \
    ((This)->lpVtbl->SetSwndProc(This, pfnWndProc, bAfter))

#define IWindow_SetTimer(This, uElapse, bRepeat, id) \
    ((This)->lpVtbl->SetTimer(This, uElapse, bRepeat, id))

#define IWindow_SetTimer2(This, uElapse, bRepeat) \
    ((This)->lpVtbl->SetTimer2(This, uElapse, bRepeat))

#define IWindow_KillTimer(This, id) \
    ((This)->lpVtbl->KillTimer(This, id))

#define IWindow_GetScale(This) \
    ((This)->lpVtbl->GetScale(This))

#define IWindow_GetScaleX(This) \
    ((This)->lpVtbl->GetScaleX(This))

#define IWindow_GetScaleY(This) \
    ((This)->lpVtbl->GetScaleY(This))

#define IWindow_IsVisible(This, bCheckParent) \
    ((This)->lpVtbl->IsVisible(This, bCheckParent))

#define IWindow_SetVisible(This, bVisible, bUpdate) \
    ((This)->lpVtbl->SetVisible(This, bVisible, bUpdate))

#define IWindow_EnableWindow(This, bEnable, bUpdate) \
    ((This)->lpVtbl->EnableWindow(This, bEnable, bUpdate))

#define IWindow_IsDisabled(This, bCheckParent) \
    ((This)->lpVtbl->IsDisabled(This, bCheckParent))

#define IWindow_IsFocusable(This) \
    ((This)->lpVtbl->IsFocusable(This))

#define IWindow_IsFocused(This) \
    ((This)->lpVtbl->IsFocused(This))

#define IWindow_SetFocus(This) \
    ((This)->lpVtbl->SetFocus(This))

#define IWindow_KillFocus(This) \
    ((This)->lpVtbl->KillFocus(This))

#define IWindow_Invalidate(This) \
    ((This)->lpVtbl->Invalidate(This))

#define IWindow_InvalidateRect(This, lprect) \
    ((This)->lpVtbl->InvalidateRect(This, lprect))

#define IWindow_LockUpdate(This) \
    ((This)->lpVtbl->LockUpdate(This))

#define IWindow_UnlockUpdate(This) \
    ((This)->lpVtbl->UnlockUpdate(This))

#define IWindow_IsUpdateLocked(This) \
    ((This)->lpVtbl->IsUpdateLocked(This))

#define IWindow_Update(This) \
    ((This)->lpVtbl->Update(This))

#define IWindow_SetAttribute(This, strAttribName, strValue, bLoading) \
    ((This)->lpVtbl->SetAttribute(This, strAttribName, strValue, bLoading))

#define IWindow_GetAttribute(This, strAttribName, strValue) \
    ((This)->lpVtbl->GetAttribute(This, strAttribName, strValue))

#define IWindow_ModifyState(This, dwStateAdd, dwStateRemove, bUpdate) \
    ((This)->lpVtbl->ModifyState(This, dwStateAdd, dwStateRemove, bUpdate))

#define IWindow_GetState(This) \
    ((This)->lpVtbl->GetState(This))

#define IWindow_ModifyStyleEx(This, dwStyleExAdd, dwStyleExRemove, bUpdate) \
    ((This)->lpVtbl->ModifyStyleEx(This, dwStyleExAdd, dwStyleExRemove, bUpdate))

#define IWindow_GetStyleEx(This) \
    ((This)->lpVtbl->GetStyleEx(This))

#define IWindow_SetOwner(This, pOwner) \
    ((This)->lpVtbl->SetOwner(This, pOwner))

#define IWindow_GetOwner(This) \
    ((This)->lpVtbl->GetOwner(This))

#define IWindow_SetUserData(This, uData) \
    ((This)->lpVtbl->SetUserData(This, uData))

#define IWindow_GetUserData(This) \
    ((This)->lpVtbl->GetUserData(This))

#define IWindow_DoColorize(This, cr) \
    ((This)->lpVtbl->DoColorize(This, cr))

#define IWindow_GetColorizeColor(This) \
    ((This)->lpVtbl->GetColorizeColor(This))

#define IWindow_SetColorizeColor(This, cr) \
    ((This)->lpVtbl->SetColorizeColor(This, cr))

#define IWindow_RequestRelayout(This, pSource, bSourceResizable) \
    ((This)->lpVtbl->RequestRelayout(This, pSource, bSourceResizable))

#define IWindow_UpdateLayout(This) \
    ((This)->lpVtbl->UpdateLayout(This))

#define IWindow_GetLayoutParam(This) \
    ((This)->lpVtbl->GetLayoutParam(This))

#define IWindow_IsFloat(This) \
    ((This)->lpVtbl->IsFloat(This))

#define IWindow_IsDisplay(This) \
    ((This)->lpVtbl->IsDisplay(This))

#define IWindow_GetName(This) \
    ((This)->lpVtbl->GetName(This))

#define IWindow_SetName(This, pszName) \
    ((This)->lpVtbl->SetName(This, pszName))

#define IWindow_GetToolTipText(This) \
    ((This)->lpVtbl->GetToolTipText(This))

#define IWindow_SetToolTipText(This, pszText) \
    ((This)->lpVtbl->SetToolTipText(This, pszText))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* Common helper functions */
static inline SWND IWindow_GetSwnd_C(IWindow* pThis)
{
    return IWindow_GetSwnd(pThis);
}

static inline IWindow* IWindow_GetParent_C(IWindow* pThis)
{
    return IWindow_GetParent(pThis);
}

static inline IWindow* IWindow_FindChildByID_C(IWindow* pThis, int uID, int nDeep)
{
    return IWindow_FindChildByID(pThis, uID, nDeep);
}

static inline BOOL IWindow_SetWindowText_C(IWindow* pThis, LPCWSTR lpszText)
{
    return IWindow_SetWindowText(pThis, lpszText);
}

static inline LPCWSTR IWindow_GetWindowText_C(IWindow* pThis, BOOL bRawText)
{
    return IWindow_GetWindowText(pThis, bRawText);
}

static inline void IWindow_SetVisible_C(IWindow* pThis, BOOL bVisible, BOOL bUpdate)
{
    IWindow_SetVisible(pThis, bVisible, bUpdate);
}

static inline BOOL IWindow_IsVisible_C(IWindow* pThis, BOOL bCheckParent)
{
    return IWindow_IsVisible(pThis, bCheckParent);
}

static inline void IWindow_Move_C(IWindow* pThis, LPCRECT prect)
{
    IWindow_Move(pThis, prect);
}

static inline void IWindow_Move2_C(IWindow* pThis, int x, int y, int cx, int cy)
{
    IWindow_Move2(pThis, x, y, cx, cy);
}

static inline void IWindow_Invalidate_C(IWindow* pThis)
{
    IWindow_Invalidate(pThis);
}

static inline void IWindow_Update_C(IWindow* pThis)
{
    IWindow_Update(pThis);
}

/*
 * Convenience macros for common window operations
 */
#define IWindow_Show(This) \
    IWindow_SetVisible(This, TRUE, TRUE)

#define IWindow_Hide(This) \
    IWindow_SetVisible(This, FALSE, TRUE)

#define IWindow_Enable(This) \
    IWindow_EnableWindow(This, TRUE, TRUE)

#define IWindow_Disable(This) \
    IWindow_EnableWindow(This, FALSE, TRUE)

#define IWindow_SetText(This, text) \
    IWindow_SetWindowText(This, text)

#define IWindow_GetText(This) \
    IWindow_GetWindowText(This, FALSE)

#define IWindow_GetRawText(This) \
    IWindow_GetWindowText(This, TRUE)

#ifdef __cplusplus
}
#endif

#endif /* __SWINDOW_CAPI_H__ */
