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

#define IWindow_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define IWindow_GetClassNameList(This, ppClassNameList, nMaxCount) \
    ((This)->lpVtbl->GetClassNameList(This, ppClassNameList, nMaxCount))

#define IWindow_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define IWindow_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IWindow_GetID(This) \
    ((This)->lpVtbl->GetID(This))

#define IWindow_SetID(This, nID) \
    ((This)->lpVtbl->SetID(This, nID))

#define IWindow_GetName(This) \
    ((This)->lpVtbl->GetName(This))

#define IWindow_GetNameA(This) \
    ((This)->lpVtbl->GetNameA(This))

#define IWindow_SetName(This, pszName) \
    ((This)->lpVtbl->SetName(This, pszName))

#define IWindow_InitFromXml(This, xmlNode) \
    ((This)->lpVtbl->InitFromXml(This, xmlNode))

#define IWindow_OnInitFinished(This, xmlNode) \
    ((This)->lpVtbl->OnInitFinished(This, xmlNode))

#define IWindow_ISetAttribute(This, strAttribName, strValue, bLoading) \
    ((This)->lpVtbl->ISetAttribute(This, strAttribName, strValue, bLoading))

#define IWindow_SetAttributeA(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttributeA(This, pszAttr, pszValue, bLoading))

#define IWindow_SetAttribute(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttribute(This, pszAttr, pszValue, bLoading))

#define IWindow_GetAttribute(This, strAttr, pValue) \
    ((This)->lpVtbl->GetAttribute(This, strAttr, pValue))

#define IWindow_AfterAttribute(This, pszAttr, pszValue, bLoading, hr) \
    ((This)->lpVtbl->AfterAttribute(This, pszAttr, pszValue, bLoading, hr))

#define IWindow_SetAttrHandler(This, attrHandler) \
    ((This)->lpVtbl->SetAttrHandler(This, attrHandler))

/* IWindow specific interface macros */
#define IWindow_QueryInterface(This, id, ppRet) \
    ((This)->lpVtbl->QueryInterface(This, id, ppRet))

#define IWindow_GetContainer(This) \
    ((This)->lpVtbl->GetContainer(This))

#define IWindow_SetContainer(This, pContainer) \
    ((This)->lpVtbl->SetContainer(This, pContainer))

#define IWindow_GetSwnd(This) \
    ((This)->lpVtbl->GetSwnd(This))

#define IWindow_GetLayout(This) \
    ((This)->lpVtbl->GetLayout(This))

#define IWindow_SetLayout(This, pLayout) \
    ((This)->lpVtbl->SetLayout(This, pLayout))

#define IWindow_GetLayoutParam(This) \
    ((This)->lpVtbl->GetLayoutParam(This))

#define IWindow_SetLayoutParam(This, pLayoutParam) \
    ((This)->lpVtbl->SetLayoutParam(This, pLayoutParam))

#define IWindow_IsFloat(This) \
    ((This)->lpVtbl->IsFloat(This))

#define IWindow_IsDisplay(This) \
    ((This)->lpVtbl->IsDisplay(This))

#define IWindow_IsMsgTransparent(This) \
    ((This)->lpVtbl->IsMsgTransparent(This))

#define IWindow_IsClipClient(This) \
    ((This)->lpVtbl->IsClipClient(This))

#define IWindow_SetToolTipText(This, pszText) \
    ((This)->lpVtbl->SetToolTipText(This, pszText))

#define IWindow_SetToolTipTextU8(This, pszText) \
    ((This)->lpVtbl->SetToolTipTextU8(This, pszText))

#define IWindow_IsChecked(This) \
    ((This)->lpVtbl->IsChecked(This))

#define IWindow_SetCheck(This, bCheck) \
    ((This)->lpVtbl->SetCheck(This, bCheck))

#define IWindow_IsDisabled(This, bCheckParent) \
    ((This)->lpVtbl->IsDisabled(This, bCheckParent))

#define IWindow_EnableWindow(This, bEnable, bUpdate) \
    ((This)->lpVtbl->EnableWindow(This, bEnable, bUpdate))

#define IWindow_IsVisible(This, bCheckParent) \
    ((This)->lpVtbl->IsVisible(This, bCheckParent))

#define IWindow_SetVisible(This, bVisible, bUpdate) \
    ((This)->lpVtbl->SetVisible(This, bVisible, bUpdate))

#define IWindow_GetUserData(This) \
    ((This)->lpVtbl->GetUserData(This))

#define IWindow_SetUserData(This, uData) \
    ((This)->lpVtbl->SetUserData(This, uData))

#define IWindow_DoColorize(This, cr) \
    ((This)->lpVtbl->DoColorize(This, cr))

#define IWindow_GetColorizeColor(This) \
    ((This)->lpVtbl->GetColorizeColor(This))

#define IWindow_SSendMessage(This, uMsg, wParam, lParam, pbMsgHandled) \
    ((This)->lpVtbl->SSendMessage(This, uMsg, wParam, lParam, pbMsgHandled))

#define IWindow_SDispatchMessage(This, uMsg, wParam, lParam) \
    ((This)->lpVtbl->SDispatchMessage(This, uMsg, wParam, lParam))

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

#define IWindow_IsUpdateLocked(This, bCheckParent) \
    ((This)->lpVtbl->IsUpdateLocked(This, bCheckParent))

#define IWindow_Update(This, bForce) \
    ((This)->lpVtbl->Update(This, bForce))

#define IWindow_SetWindowRgn(This, pRgn, bRedraw) \
    ((This)->lpVtbl->SetWindowRgn(This, pRgn, bRedraw))

#define IWindow_GetWindowRgn(This) \
    ((This)->lpVtbl->GetWindowRgn(This))

#define IWindow_SetWindowPath(This, pPath, bRedraw) \
    ((This)->lpVtbl->SetWindowPath(This, pPath, bRedraw))

#define IWindow_GetWindowPath(This) \
    ((This)->lpVtbl->GetWindowPath(This))

#define IWindow_Move2(This, x, y, cx, cy) \
    ((This)->lpVtbl->Move2(This, x, y, cx, cy))

#define IWindow_Move(This, prect) \
    ((This)->lpVtbl->Move(This, prect))

#define IWindow_GetWindowRect(This, prect) \
    ((This)->lpVtbl->GetWindowRect(This, prect))

#define IWindow_GetClientRect(This, prect) \
    ((This)->lpVtbl->GetClientRect(This, prect))

#define IWindow_GetVisibleRect(This, prect) \
    ((This)->lpVtbl->GetVisibleRect(This, prect))

#define IWindow_IsContainPoint(This, pt, bClientOnly) \
    ((This)->lpVtbl->IsContainPoint(This, pt, bClientOnly))

#define IWindow_SwndFromPoint(This, pt, bIncludeMsgTransparent) \
    ((This)->lpVtbl->SwndFromPoint(This, pt, bIncludeMsgTransparent))

#define IWindow_SetTimer(This, id, uElapse) \
    ((This)->lpVtbl->SetTimer(This, id, uElapse))

#define IWindow_KillTimer(This, id) \
    ((This)->lpVtbl->KillTimer(This, id))

#define IWindow_GetCapture(This) \
    ((This)->lpVtbl->GetCapture(This))

#define IWindow_SetCapture(This) \
    ((This)->lpVtbl->SetCapture(This))

#define IWindow_ReleaseCapture(This) \
    ((This)->lpVtbl->ReleaseCapture(This))

#define IWindow_SetAnimation(This, animation) \
    ((This)->lpVtbl->SetAnimation(This, animation))

#define IWindow_StartAnimation(This, animation, bStartNow) \
    ((This)->lpVtbl->StartAnimation(This, animation, bStartNow))

#define IWindow_GetAnimation(This) \
    ((This)->lpVtbl->GetAnimation(This))

#define IWindow_ClearAnimation(This) \
    ((This)->lpVtbl->ClearAnimation(This))

#define IWindow_SetAlpha(This, byAlpha) \
    ((This)->lpVtbl->SetAlpha(This, byAlpha))

#define IWindow_GetAlpha(This) \
    ((This)->lpVtbl->GetAlpha(This))

#define IWindow_SetMatrix(This, mtx, bInvalidate) \
    ((This)->lpVtbl->SetMatrix(This, mtx, bInvalidate))

#define IWindow_GetMatrix(This, mtx) \
    ((This)->lpVtbl->GetMatrix(This, mtx))

#define IWindow_GetScale(This) \
    ((This)->lpVtbl->GetScale(This))

#define IWindow_RequestRelayout(This) \
    ((This)->lpVtbl->RequestRelayout(This))

#define IWindow_UpdateLayout(This) \
    ((This)->lpVtbl->UpdateLayout(This))

#define IWindow_UpdateChildrenPosition(This) \
    ((This)->lpVtbl->UpdateChildrenPosition(This))

#define IWindow_IsLayoutDirty(This) \
    ((This)->lpVtbl->IsLayoutDirty(This))

#define IWindow_OnGetDlgCode(This) \
    ((This)->lpVtbl->OnGetDlgCode(This))

#define IWindow_GetIRoot(This) \
    ((This)->lpVtbl->GetIRoot(This))

#define IWindow_GetIParent(This) \
    ((This)->lpVtbl->GetIParent(This))

#define IWindow_GetIWindow(This, uCode) \
    ((This)->lpVtbl->GetIWindow(This, uCode))

#define IWindow_GetIChild(This, iChild) \
    ((This)->lpVtbl->GetIChild(This, iChild))

#define IWindow_GetChildrenCount(This) \
    ((This)->lpVtbl->GetChildrenCount(This))

#define IWindow_IsDescendant(This, pTest) \
    ((This)->lpVtbl->IsDescendant(This, pTest))

#define IWindow_SetIOwner(This, pOwner) \
    ((This)->lpVtbl->SetIOwner(This, pOwner))

#define IWindow_GetIOwner(This) \
    ((This)->lpVtbl->GetIOwner(This))

#define IWindow_BringWindowToTop(This) \
    ((This)->lpVtbl->BringWindowToTop(This))

#define IWindow_AdjustIZOrder(This, pInsertAfter) \
    ((This)->lpVtbl->AdjustIZOrder(This, pInsertAfter))

#define IWindow_InsertIChild(This, pNewChild, pInsertAfter) \
    ((This)->lpVtbl->InsertIChild(This, pNewChild, pInsertAfter))

#define IWindow_RemoveIChild(This, pChild) \
    ((This)->lpVtbl->RemoveIChild(This, pChild))

#define IWindow_CreateChildrenFromXml(This, pszXml) \
    ((This)->lpVtbl->CreateChildrenFromXml(This, pszXml))

#define IWindow_CreateChildrenFromResId(This, pszResId) \
    ((This)->lpVtbl->CreateChildrenFromResId(This, pszResId))

#define IWindow_InitFromResId(This, pszResId) \
    ((This)->lpVtbl->InitFromResId(This, pszResId))

#define IWindow_FindIChildByID(This, nId) \
    ((This)->lpVtbl->FindIChildByID(This, nId))

#define IWindow_FindIChildByName(This, pszName) \
    ((This)->lpVtbl->FindIChildByName(This, pszName))

#define IWindow_FindIChildByNameA(This, pszName) \
    ((This)->lpVtbl->FindIChildByNameA(This, pszName))

#define IWindow_DestroyIChild(This, pChild) \
    ((This)->lpVtbl->DestroyIChild(This, pChild))

#define IWindow_DestroyAllChildren(This) \
    ((This)->lpVtbl->DestroyAllChildren(This))

#define IWindow_Destroy(This) \
    ((This)->lpVtbl->Destroy(This))

#define IWindow_GetNextLayoutIChild(This, pCurChild) \
    ((This)->lpVtbl->GetNextLayoutIChild(This, pCurChild))

#define IWindow_GetChildrenLayoutRect(This, prc) \
    ((This)->lpVtbl->GetChildrenLayoutRect(This, prc))

#define IWindow_GetDesiredSize(This, pSize, nParentWid, nParentHei) \
    ((This)->lpVtbl->GetDesiredSize(This, pSize, nParentWid, nParentHei))

#define IWindow_GetBkgndColor(This) \
    ((This)->lpVtbl->GetBkgndColor(This))

#define IWindow_SetWindowText(This, lpszText) \
    ((This)->lpVtbl->SetWindowText(This, lpszText))

#define IWindow_SetWindowTextU8(This, lpszText) \
    ((This)->lpVtbl->SetWindowTextU8(This, lpszText))

#define IWindow_GetWindowText(This, pBuf, nBufLen, bRawText) \
    ((This)->lpVtbl->GetWindowText(This, pBuf, nBufLen, bRawText))

#define IWindow_GetWindowTextU8(This, pStr, bRawText) \
    ((This)->lpVtbl->GetWindowTextU8(This, pStr, bRawText))

#define IWindow_GetState(This) \
    ((This)->lpVtbl->GetState(This))

#define IWindow_ModifyState(This, dwStateAdd, dwStateRemove, bUpdate) \
    ((This)->lpVtbl->ModifyState(This, dwStateAdd, dwStateRemove, bUpdate))

#define IWindow_GetISelectedSiblingInGroup(This) \
    ((This)->lpVtbl->GetISelectedSiblingInGroup(This))

#define IWindow_GetISelectedChildInGroup(This) \
    ((This)->lpVtbl->GetISelectedChildInGroup(This))

#define IWindow_IsSiblingsAutoGroupped(This) \
    ((This)->lpVtbl->IsSiblingsAutoGroupped(This))

#define IWindow_CreateCaret(This, pBmp, nWid, nHeight) \
    ((This)->lpVtbl->CreateCaret(This, pBmp, nWid, nHeight))

#define IWindow_ShowCaret(This, bShow) \
    ((This)->lpVtbl->ShowCaret(This, bShow))

#define IWindow_SetCaretPos(This, x, y) \
    ((This)->lpVtbl->SetCaretPos(This, x, y))

#define IWindow_SetEventMute(This, bMute) \
    ((This)->lpVtbl->SetEventMute(This, bMute))

#define IWindow_SubscribeEvent(This, evtId, pSlot) \
    ((This)->lpVtbl->SubscribeEvent(This, evtId, pSlot))

#define IWindow_UnsubscribeEvent(This, evtId, pSlot) \
    ((This)->lpVtbl->UnsubscribeEvent(This, evtId, pSlot))

#define IWindow_AddEvent(This, dwEventID, pszEventHandlerName) \
    ((This)->lpVtbl->AddEvent(This, dwEventID, pszEventHandlerName))

#define IWindow_RemoveEvent(This, dwEventID) \
    ((This)->lpVtbl->RemoveEvent(This, dwEventID))

#define IWindow_FireEvent(This, evt) \
    ((This)->lpVtbl->FireEvent(This, evt))

#define IWindow_FireCommand(This) \
    ((This)->lpVtbl->FireCommand(This))

#define IWindow_FireCtxMenu(This, pt) \
    ((This)->lpVtbl->FireCtxMenu(This, pt))

#define IWindow_SwndProc(This, uMsg, wParam, lParam, lResult) \
    ((This)->lpVtbl->SwndProc(This, uMsg, wParam, lParam, lResult))

#define IWindow_SetSwndProc(This, swndProc) \
    ((This)->lpVtbl->SetSwndProc(This, swndProc))

#define IWindow_GetHostHwnd(This) \
    ((This)->lpVtbl->GetHostHwnd(This))

#define IWindow_GetTimelineHandlersMgr(This) \
    ((This)->lpVtbl->GetTimelineHandlersMgr(This))

#define IWindow_IsVideoCanvas(This) \
    ((This)->lpVtbl->IsVideoCanvas(This))

#define IWindow_RegisterDragDrop(This, pDragTarget) \
    ((This)->lpVtbl->RegisterDragDrop(This, pDragTarget))

#define IWindow_UnregisterDragDrop(This) \
    ((This)->lpVtbl->UnregisterDragDrop(This))

#define IWindow_SetLayer(This, nLayer) \
    ((This)->lpVtbl->SetLayer(This, nLayer))

#define IWindow_GetLayer(This) \
    ((This)->lpVtbl->GetLayer(This))

#define IWindow_SetAnimatorValue(This, pHolder, fraction, state) \
    ((This)->lpVtbl->SetAnimatorValue(This, pHolder, fraction, state))

#define IWindow_SetPivot(This, x, y) \
    ((This)->lpVtbl->SetPivot(This, x, y))

#define IWindow_GetPivot(This, x, y) \
    ((This)->lpVtbl->GetPivot(This, x, y))

#ifdef __cplusplus
}
#endif

#endif /* __SWINDOW_CAPI_H__ */
