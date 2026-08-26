#ifndef __STREEVIEWITEMLOCATOR_CAPI_H__
#define __STREEVIEWITEMLOCATOR_CAPI_H__

#include "../STreeViewItemLocator-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for ITreeViewItemLocator Interface
 */

/* IObjRef base interface macros */
#define ITreeViewItemLocator_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ITreeViewItemLocator_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ITreeViewItemLocator_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* ITreeViewItemLocator specific interface macros */
#define ITreeViewItemLocator_SetAdapter(This, pAdapter) \
    ((This)->lpVtbl->SetAdapter(This, pAdapter))

#define ITreeViewItemLocator_OnBranchChanged(This, hItem) \
    ((This)->lpVtbl->OnBranchChanged(This, hItem))

#define ITreeViewItemLocator_OnBranchExpandedChanged(This, hItem, bExpandedOld, bExpandedNew) \
    ((This)->lpVtbl->OnBranchExpandedChanged(This, hItem, bExpandedOld, bExpandedNew))

#define ITreeViewItemLocator_GetTotalHeight(This) \
    ((This)->lpVtbl->GetTotalHeight(This))

#define ITreeViewItemLocator_GetTotalWidth(This) \
    ((This)->lpVtbl->GetTotalWidth(This))

#define ITreeViewItemLocator_Item2Position(This, hItem) \
    ((This)->lpVtbl->Item2Position(This, hItem))

#define ITreeViewItemLocator_Position2Item(This, position) \
    ((This)->lpVtbl->Position2Item(This, position))

#define ITreeViewItemLocator_SetDefItemHeight(This, nHeight) \
    ((This)->lpVtbl->SetDefItemHeight(This, nHeight))

#define ITreeViewItemLocator_GetScrollLineSize(This) \
    ((This)->lpVtbl->GetScrollLineSize(This))

#define ITreeViewItemLocator_SetItemWidth(This, hItem, nWidth) \
    ((This)->lpVtbl->SetItemWidth(This, hItem, nWidth))

#define ITreeViewItemLocator_GetItemWidth(This, hItem) \
    ((This)->lpVtbl->GetItemWidth(This, hItem))

#define ITreeViewItemLocator_SetItemHeight(This, hItem, nHeight) \
    ((This)->lpVtbl->SetItemHeight(This, hItem, nHeight))

#define ITreeViewItemLocator_GetItemHeight(This, hItem) \
    ((This)->lpVtbl->GetItemHeight(This, hItem))

#define ITreeViewItemLocator_GetItemIndent(This, hItem) \
    ((This)->lpVtbl->GetItemIndent(This, hItem))

#define ITreeViewItemLocator_GetIndent(This) \
    ((This)->lpVtbl->GetIndent(This))

#define ITreeViewItemLocator_SetIndent(This, nIndent) \
    ((This)->lpVtbl->SetIndent(This, nIndent))

#ifdef __cplusplus
}
#endif

#endif /* __STREEVIEWITEMLOCATOR_CAPI_H__ */
