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

#define IMenuEx_LoadMenuU8(This, resId) \
    ((This)->lpVtbl->LoadMenuU8(This, resId))

#define IMenuEx_LoadMenu2(This, xmlMenu) \
    ((This)->lpVtbl->LoadMenu2(This, xmlMenu))

#define IMenuEx_InsertMenu(This, uPosition, uFlags, id, strText, iIcon) \
    ((This)->lpVtbl->InsertMenu(This, uPosition, uFlags, id, strText, iIcon))

#define IMenuEx_AppendMenu(This, uFlags, id, lpNewItem, iIcon) \
    ((This)->lpVtbl->AppendMenu(This, uFlags, id, lpNewItem, iIcon))

#define IMenuEx_CheckMenuItem(This, uIdCheckItem, uCheck) \
    ((This)->lpVtbl->CheckMenuItem(This, uIdCheckItem, uCheck))

#define IMenuEx_CheckMenuRadioItem(This, idFirst, idLast, idCheck, uFlags) \
    ((This)->lpVtbl->CheckMenuRadioItem(This, idFirst, idLast, idCheck, uFlags))

#define IMenuEx_EnableMenuItem(This, uIDEnableItem, uEnable) \
    ((This)->lpVtbl->EnableMenuItem(This, uIDEnableItem, uEnable))

#define IMenuEx_DeleteMenu(This, uPosition, uFlags) \
    ((This)->lpVtbl->DeleteMenu(This, uPosition, uFlags))

#define IMenuEx_TrackPopupMenu(This, uFlags, x, y, hWnd, nScale) \
    ((This)->lpVtbl->TrackPopupMenu(This, uFlags, x, y, hWnd, nScale))

#define IMenuEx_DestroyMenu(This) \
    ((This)->lpVtbl->DestroyMenu(This))

#define IMenuEx_ModifyMenuString(This, uPosition, uFlags, lpItemString) \
    ((This)->lpVtbl->ModifyMenuString(This, uPosition, uFlags, lpItemString))

#define IMenuEx_SetMenuUserData(This, uPosition, uFlags, ulUserData) \
    ((This)->lpVtbl->SetMenuUserData(This, uPosition, uFlags, ulUserData))

#define IMenuEx_GetMenuUserData(This, uPosition, uFlags) \
    ((This)->lpVtbl->GetMenuUserData(This, uPosition, uFlags))

#define IMenuEx_GetContextHelpId(This) \
    ((This)->lpVtbl->GetContextHelpId(This))

#define IMenuEx_SetContextHelpId(This, dwId) \
    ((This)->lpVtbl->SetContextHelpId(This, dwId))

#define IMenuEx_GetSubMenu(This, nPos) \
    ((This)->lpVtbl->GetSubMenu(This, nPos))

#define IMenuEx_GetMenuString(This, uPosition, uFlags, lpItemString) \
    ((This)->lpVtbl->GetMenuString(This, uPosition, uFlags, lpItemString))

#define IMenuEx_GetHostWnd(This) \
    ((This)->lpVtbl->GetHostWnd(This))

#define IMenuEx_GetMenuItemCount(This) \
    ((This)->lpVtbl->GetMenuItemCount(This))

#ifdef __cplusplus
}
#endif

#endif /* __SMENUEX_CAPI_H__ */
