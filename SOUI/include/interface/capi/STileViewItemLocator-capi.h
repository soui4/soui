#ifndef __STILEVIEWITEMLOCATOR_CAPI_H__
#define __STILEVIEWITEMLOCATOR_CAPI_H__

#include "../STileViewItemLocator-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for ITileViewItemLocator Interface
 */

/* IObjRef base interface macros */
#define ITileViewItemLocator_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ITileViewItemLocator_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ITileViewItemLocator_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* ITileViewItemLocator specific interface macros */
#define ITileViewItemLocator_SetAdapter(This, pAdapter) \
    ((This)->lpVtbl->SetAdapter(This, pAdapter))

#define ITileViewItemLocator_OnDataSetChanged(This) \
    ((This)->lpVtbl->OnDataSetChanged(This))

#define ITileViewItemLocator_GetItemHeight(This, iItem) \
    ((This)->lpVtbl->GetItemHeight(This, iItem))

#define ITileViewItemLocator_SetItemHeight(This, iItem, nHeight) \
    ((This)->lpVtbl->SetItemHeight(This, iItem, nHeight))

#define ITileViewItemLocator_GetItemRect(This, iItem) \
    ((This)->lpVtbl->GetItemRect(This, iItem))

#define ITileViewItemLocator_SetTileViewWidth(This, width, bDpiAware) \
    ((This)->lpVtbl->SetTileViewWidth(This, width, bDpiAware))

#define ITileViewItemLocator_GetItemRowAndColIndex(This, iItem, row, col) \
    ((This)->lpVtbl->GetItemRowAndColIndex(This, iItem, row, col))

#define ITileViewItemLocator_IsLastInRow(This, iItem) \
    ((This)->lpVtbl->IsLastInRow(This, iItem))

#define ITileViewItemLocator_GetUpItem(This, iItem) \
    ((This)->lpVtbl->GetUpItem(This, iItem))

#define ITileViewItemLocator_GetDownItem(This, iItem) \
    ((This)->lpVtbl->GetDownItem(This, iItem))

#define ITileViewItemLocator_GetTotalHeight(This) \
    ((This)->lpVtbl->GetTotalHeight(This))

#define ITileViewItemLocator_Item2Position(This, iItem) \
    ((This)->lpVtbl->Item2Position(This, iItem))

#define ITileViewItemLocator_Position2Item(This, position) \
    ((This)->lpVtbl->Position2Item(This, position))

#define ITileViewItemLocator_GetScrollLineSize(This) \
    ((This)->lpVtbl->GetScrollLineSize(This))

#define ITileViewItemLocator_GetMarginSize(This) \
    ((This)->lpVtbl->GetMarginSize(This))

#define ITileViewItemLocator_SetScale(This, scale) \
    ((This)->lpVtbl->SetScale(This, scale))

#define ITileViewItemLocator_GetCountInRow(This) \
    ((This)->lpVtbl->GetCountInRow(This))

#define ITileViewItemLocator_GetItemLineHeight(This) \
    ((This)->lpVtbl->GetItemLineHeight(This))

#define ITileViewItemLocator_GetItemWidth(This) \
    ((This)->lpVtbl->GetItemWidth(This))

#ifdef __cplusplus
}
#endif

#endif /* __STILEVIEWITEMLOCATOR_CAPI_H__ */
