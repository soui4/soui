#ifndef __SMENU_CAPI_H__
#define __SMENU_CAPI_H__

#include "../smenu-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IObjRef base interface macros */
#define IMenu_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IMenu_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IMenu_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IMenu specific interface macros */
#define IMenu_Attach(This, hMenu) \
    ((This)->lpVtbl->Attach(This, hMenu))

#define IMenu_Detach(This) \
    ((This)->lpVtbl->Detach(This))

#define IMenu_GetHMenu(This) \
    ((This)->lpVtbl->GetHMenu(This))

#define IMenu_LoadMenu(This, resId) \
    ((This)->lpVtbl->LoadMenu(This, resId))

#define IMenu_LoadMenuU8(This, resId) \
    ((This)->lpVtbl->LoadMenuU8(This, resId))

#define IMenu_LoadMenu2(This, xmlNode) \
    ((This)->lpVtbl->LoadMenu2(This, xmlNode))

#define IMenu_SetIconSkin(This, icons) \
    ((This)->lpVtbl->SetIconSkin(This, icons))

#define IMenu_InsertMenu(This, uPos, uFlag, dwID, lpNewItem, iIcon, hIcon) \
    ((This)->lpVtbl->InsertMenu(This, uPos, uFlag, dwID, lpNewItem, iIcon, hIcon))

#define IMenu_AppendMenu(This, uFlag, dwID, lpNewItem, iIcon, hIcon) \
    ((This)->lpVtbl->AppendMenu(This, uFlag, dwID, lpNewItem, iIcon, hIcon))

#define IMenu_CheckMenuItem(This, uIdCheckItem, uCheck) \
    ((This)->lpVtbl->CheckMenuItem(This, uIdCheckItem, uCheck))

#define IMenu_EnableMenuItem(This, uIDEnableItem, uEnable) \
    ((This)->lpVtbl->EnableMenuItem(This, uIDEnableItem, uEnable))

#define IMenu_CheckMenuRadioItem(This, idFirst, idLast, idCheck, uFlags) \
    ((This)->lpVtbl->CheckMenuRadioItem(This, idFirst, idLast, idCheck, uFlags))

#define IMenu_SetMenuUserData(This, uID, dwUserData) \
    ((This)->lpVtbl->SetMenuUserData(This, uID, dwUserData))

#define IMenu_GetMenuUserData(This, uID) \
    ((This)->lpVtbl->GetMenuUserData(This, uID))

#define IMenu_DeleteMenu(This, uPosition, uFlags) \
    ((This)->lpVtbl->DeleteMenu(This, uPosition, uFlags))

#define IMenu_TrackPopupMenu(This, uFlags, x, y, hWnd, prcRect, nScale) \
    ((This)->lpVtbl->TrackPopupMenu(This, uFlags, x, y, hWnd, prcRect, nScale))

#define IMenu_DestroyMenu(This) \
    ((This)->lpVtbl->DestroyMenu(This))

#define IMenu_GetSubMenu(This, uPosition) \
    ((This)->lpVtbl->GetSubMenu(This, uPosition))

#define IMenu_TrackPopupMenu(This, uFlags, x, y, hwnd, prcRect) \
    ((This)->lpVtbl->TrackPopupMenu(This, uFlags, x, y, hwnd, prcRect))

#define IMenu_GetMenuItemCount(This) \
    ((This)->lpVtbl->GetMenuItemCount(This))

#define IMenu_GetMenuItemID(This, nPos) \
    ((This)->lpVtbl->GetMenuItemID(This, nPos))

#define IMenu_RemoveMenu(This, uPosition, uFlags) \
    ((This)->lpVtbl->RemoveMenu(This, uPosition, uFlags))

#define IMenu_HiliteMenuItem(This, hwnd, uIDHiliteItem, uHilite) \
    ((This)->lpVtbl->HiliteMenuItem(This, hwnd, uIDHiliteItem, uHilite))

#define IMenu_GetMenuContextHelpId(This) \
    ((This)->lpVtbl->GetMenuContextHelpId(This))

#define IMenu_SetMenuContextHelpId(This, dwContextHelpId) \
    ((This)->lpVtbl->SetMenuContextHelpId(This, dwContextHelpId))

#define IMenu_GetMenuData(This) \
    ((This)->lpVtbl->GetMenuData(This))

#define IMenu_SetMenuData(This, menuData) \
    ((This)->lpVtbl->SetMenuData(This, menuData))

#define IMenu_ModifyMenu(This, uPosition, uFlags, lpNewItem, iIcon) \
    ((This)->lpVtbl->ModifyMenu(This, uPosition, uFlags, lpNewItem, iIcon))

#ifdef __cplusplus
}
#endif

#endif /* __SMENU_CAPI_H__ */