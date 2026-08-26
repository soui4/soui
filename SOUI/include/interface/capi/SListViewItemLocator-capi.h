#ifndef __SLISTVIEWITEMLOCATOR_CAPI_H__
#define __SLISTVIEWITEMLOCATOR_CAPI_H__

#include "../SListViewItemLocator-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for IListViewItemLocator Interface
 */

/* IObjRef base interface macros */
#define IListViewItemLocator_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IListViewItemLocator_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IListViewItemLocator_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IListViewItemLocator specific interface macros */
#define IListViewItemLocator_SetAdapter(This, pAdapter) \
    ((This)->lpVtbl->SetAdapter(This, pAdapter))

#define IListViewItemLocator_OnDataSetChanged(This) \
    ((This)->lpVtbl->OnDataSetChanged(This))

#define IListViewItemLocator_IsFixHeight(This) \
    ((This)->lpVtbl->IsFixHeight(This))

#define IListViewItemLocator_GetItemHeight(This, iItem) \
    ((This)->lpVtbl->GetItemHeight(This, iItem))

#define IListViewItemLocator_SetItemHeight(This, iItem, nHeight) \
    ((This)->lpVtbl->SetItemHeight(This, iItem, nHeight))

#define IListViewItemLocator_GetTotalHeight(This) \
    ((This)->lpVtbl->GetTotalHeight(This))

#define IListViewItemLocator_Item2Position(This, iItem) \
    ((This)->lpVtbl->Item2Position(This, iItem))

#define IListViewItemLocator_Position2Item(This, position) \
    ((This)->lpVtbl->Position2Item(This, position))

#define IListViewItemLocator_GetScrollLineSize(This) \
    ((This)->lpVtbl->GetScrollLineSize(This))

#define IListViewItemLocator_GetDividerSize(This) \
    ((This)->lpVtbl->GetDividerSize(This))

#define IListViewItemLocator_SetScale(This, nScale) \
    ((This)->lpVtbl->SetScale(This, nScale))

#ifdef __cplusplus
}
#endif

#endif /* __SLISTVIEWITEMLOCATOR_CAPI_H__ */
