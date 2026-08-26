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

#define IMenu_LoadMenu2(This, xmlMenu) \
    ((This)->lpVtbl->LoadMenu2(This, xmlMenu))

#define IMenu_SetIconSkin(This, icons) \
    ((This)->lpVtbl->SetIconSkin(This, icons))

#define IMenu_InsertMenu(This, uPosition, uFlags, nIDNewItem, strText, iIcon, hIcon) \
    ((This)->lpVtbl->InsertMenu(This, uPosition, uFlags, nIDNewItem, strText, iIcon, hIcon))

#define IMenu_AppendMenu(This, uFlags, uIDNewItem, strText, iIcon, hIcon) \
    ((This)->lpVtbl->AppendMenu(This, uFlags, uIDNewItem, strText, iIcon, hIcon))

#define IMenu_EnableMenuItem(This, uIDEnableItem, uEnable) \
    ((This)->lpVtbl->EnableMenuItem(This, uIDEnableItem, uEnable))

#define IMenu_CheckMenuItem(This, uIdCheckItem, uCheck) \
    ((This)->lpVtbl->CheckMenuItem(This, uIdCheckItem, uCheck))

#define IMenu_CheckMenuRadioItem(This, idFirst, idLast, idCheck, uFlags) \
    ((This)->lpVtbl->CheckMenuRadioItem(This, idFirst, idLast, idCheck, uFlags))

#define IMenu_DeleteMenu(This, uPosition, uFlags) \
    ((This)->lpVtbl->DeleteMenu(This, uPosition, uFlags))

#define IMenu_TrackPopupMenu(This, uFlags, x, y, hWnd, prcRect, nScale) \
    ((This)->lpVtbl->TrackPopupMenu(This, uFlags, x, y, hWnd, prcRect, nScale))

#define IMenu_DestroyMenu(This) \
    ((This)->lpVtbl->DestroyMenu(This))

#define IMenu_ModifyMenuString(This, uPosition, uFlags, lpItemString) \
    ((This)->lpVtbl->ModifyMenuString(This, uPosition, uFlags, lpItemString))

#define IMenu_SetMenuUserData(This, uPosition, uFlags, ulUserData) \
    ((This)->lpVtbl->SetMenuUserData(This, uPosition, uFlags, ulUserData))

#define IMenu_GetMenuUserData(This, uPosition, uFlags) \
    ((This)->lpVtbl->GetMenuUserData(This, uPosition, uFlags))

#define IMenu_GetContextHelpId(This) \
    ((This)->lpVtbl->GetContextHelpId(This))

#define IMenu_SetContextHelpId(This, dwId) \
    ((This)->lpVtbl->SetContextHelpId(This, dwId))

#define IMenu_GetSubMenu(This, nPos) \
    ((This)->lpVtbl->GetSubMenu(This, nPos))

#define IMenu_GetMenuString(This, uPosition, uFlags, lpItemString) \
    ((This)->lpVtbl->GetMenuString(This, uPosition, uFlags, lpItemString))

#define IMenu_GetMenuItemCount(This) \
    ((This)->lpVtbl->GetMenuItemCount(This))

#ifdef __cplusplus
}
#endif

#endif /* __SMENU_CAPI_H__ */
