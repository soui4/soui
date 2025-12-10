#ifndef __SMENUEX_CAPI_H__
#define __SMENUEX_CAPI_H__

#include "../smenuex-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IObjRef base interface macros */
#define IMenuEx_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IMenuEx_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IMenuEx_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IMenuEx specific interface macros */
#define IMenuEx_LoadMenu(This, resId) \
    ((This)->lpVtbl->LoadMenu(This, resId))

#define IMenuEx_LoadMenu2(This, xmlNode) \
    ((This)->lpVtbl->LoadMenu2(This, xmlNode))

#define IMenuEx_InsertMenu(This, uPos, uFlag, dwID, lpNewItem, iIcon) \
    ((This)->lpVtbl->InsertMenu(This, uPos, uFlag, dwID, lpNewItem, iIcon))

#define IMenuEx_AppendMenu(This, uFlag, dwID, lpNewItem, iIcon) \
    ((This)->lpVtbl->AppendMenu(This, uFlag, dwID, lpNewItem, iIcon))

#define IMenuEx_CheckMenuItem(This, uIdCheckItem, uCheck) \
    ((This)->lpVtbl->CheckMenuItem(This, uIdCheckItem, uCheck))

#define IMenuEx_EnableMenuItem(This, uIDEnableItem, uEnable) \
    ((This)->lpVtbl->EnableMenuItem(This, uIDEnableItem, uEnable))

#define IMenuEx_CheckMenuRadioItem(This, idFirst, idLast, idCheck, uFlags) \
    ((This)->lpVtbl->CheckMenuRadioItem(This, idFirst, idLast, idCheck, uFlags))

#define IMenuEx_SetMenuUserData(This, uID, dwUserData) \
    ((This)->lpVtbl->SetMenuUserData(This, uID, dwUserData))

#define IMenuEx_GetMenuUserData(This, uID) \
    ((This)->lpVtbl->GetMenuUserData(This, uID))

#define IMenuEx_DeleteMenu(This, uPosition, uFlags) \
    ((This)->lpVtbl->DeleteMenu(This, uPosition, uFlags))

#define IMenuEx_DestroyMenu(This) \
    ((This)->lpVtbl->DestroyMenu(This))

#define IMenuEx_GetSubMenu(This, uPosition) \
    ((This)->lpVtbl->GetSubMenu(This, uPosition))

#define IMenuEx_TrackPopupMenu(This, uFlags, x, y, hwnd, prcRect, uAnimate) \
    ((This)->lpVtbl->TrackPopupMenu(This, uFlags, x, y, hwnd, prcRect, uAnimate))

#define IMenuEx_GetMenuItemCount(This) \
    ((This)->lpVtbl->GetMenuItemCount(This))

#define IMenuEx_GetMenuItemID(This, nPos) \
    ((This)->lpVtbl->GetMenuItemID(This, nPos))

#define IMenuEx_RemoveMenu(This, uPosition, uFlags) \
    ((This)->lpVtbl->RemoveMenu(This, uPosition, uFlags))

#define IMenuEx_HiliteMenuItem(This, hwnd, uIDHiliteItem, uHilite) \
    ((This)->lpVtbl->HiliteMenuItem(This, hwnd, uIDHiliteItem, uHilite))

#define IMenuEx_GetMenuContextHelpId(This) \
    ((This)->lpVtbl->GetMenuContextHelpId(This))

#define IMenuEx_SetMenuContextHelpId(This, dwContextHelpId) \
    ((This)->lpVtbl->SetMenuContextHelpId(This, dwContextHelpId))

#define IMenuEx_GetMenuData(This) \
    ((This)->lpVtbl->GetMenuData(This))

#define IMenuEx_SetMenuData(This, menuData) \
    ((This)->lpVtbl->SetMenuData(This, menuData))

#define IMenuEx_ModifyMenu(This, uPosition, uFlags, lpNewItem, iIcon) \
    ((This)->lpVtbl->ModifyMenu(This, uPosition, uFlags, lpNewItem, iIcon))

#define IMenuEx_GetMenuBar(This) \
    ((This)->lpVtbl->GetMenuBar(This))

#define IMenuEx_GetMenuWindow(This) \
    ((This)->lpVtbl->GetMenuWindow(This))

#define IMenuEx_InsertMenuByXml(This, uPos, xmlNode) \
    ((This)->lpVtbl->InsertMenuByXml(This, uPos, xmlNode))

#define IMenuEx_GetMenuWrapper(This) \
    ((This)->lpVtbl->GetMenuWrapper(This))

#ifdef __cplusplus
}
#endif

#endif /* __SMENUEX_CAPI_H__ */