#ifndef __SADAPTER_CAPI_H__
#define __SADAPTER_CAPI_H__

#include "../SAdapter-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for Adapter Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ILvDataSetObserver C API Macros */
#define ILvDataSetObserver_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ILvDataSetObserver_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ILvDataSetObserver_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ILvDataSetObserver_onChanged(This) \
    ((This)->lpVtbl->onChanged(This))

#define ILvDataSetObserver_onInvalidated(This) \
    ((This)->lpVtbl->onInvalidated(This))

#define ILvDataSetObserver_OnItemChanged(This, iItem) \
    ((This)->lpVtbl->OnItemChanged(This, iItem))

/* ILvAdapter C API Macros */
#define ILvAdapter_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ILvAdapter_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ILvAdapter_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ILvAdapter_registerDataSetObserver(This, observer) \
    ((This)->lpVtbl->registerDataSetObserver(This, observer))

#define ILvAdapter_unregisterDataSetObserver(This, observer) \
    ((This)->lpVtbl->unregisterDataSetObserver(This, observer))

#define ILvAdapter_getCount(This) \
    ((This)->lpVtbl->getCount(This))

#define ILvAdapter_getView(This, position, pItem, xmlTemplate) \
    ((This)->lpVtbl->getView(This, position, pItem, xmlTemplate))

#define ILvAdapter_getItemViewType(This, position, dwState) \
    ((This)->lpVtbl->getItemViewType(This, position, dwState))

#define ILvAdapter_getViewTypeCount(This) \
    ((This)->lpVtbl->getViewTypeCount(This))

#define ILvAdapter_getViewDesiredSize(This, ret, position, pItem, wid, hei) \
    ((This)->lpVtbl->getViewDesiredSize(This, ret, position, pItem, wid, hei))

#define ILvAdapter_isEmpty(This) \
    ((This)->lpVtbl->isEmpty(This))

#define ILvAdapter_getItemDesc(This, position, pDesc) \
    ((This)->lpVtbl->getItemDesc(This, position, pDesc))

#define ILvAdapter_InitByTemplate(This, xmlTemplate) \
    ((This)->lpVtbl->InitByTemplate(This, xmlTemplate))

#define ILvAdapter_QueryInterface(This, id, ppObj) \
    ((This)->lpVtbl->QueryInterface(This, id, ppObj))

/*
 * IMcAdapter interface macros
 * IMcAdapter inherits from ILvAdapter; use ILvAdapter_ macros for inherited methods.
 */

/* IMcAdapter specific interface macros */
#define IMcAdapter_GetColumnName(This, iCol, pName) \
    ((This)->lpVtbl->GetColumnName(This, iCol, pName))

#define IMcAdapter_IsColumnVisible(This, iCol) \
    ((This)->lpVtbl->IsColumnVisible(This, iCol))

#define IMcAdapter_OnSort(This, iCol, pFmts, nCols) \
    ((This)->lpVtbl->OnSort(This, iCol, pFmts, nCols))

#define IMcAdapter_SetColumnsWidth(This, pColWid, nCols) \
    ((This)->lpVtbl->SetColumnsWidth(This, pColWid, nCols))

/* ITvDataSetObserver C API Macros */
#define ITvDataSetObserver_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ITvDataSetObserver_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ITvDataSetObserver_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ITvDataSetObserver_onBranchChanged(This, hBranch) \
    ((This)->lpVtbl->onBranchChanged(This, hBranch))

#define ITvDataSetObserver_onBranchInvalidated(This, hBranch, bInvalidParents, bInvalidChildren) \
    ((This)->lpVtbl->onBranchInvalidated(This, hBranch, bInvalidParents, bInvalidChildren))

#define ITvDataSetObserver_onBranchExpandedChanged(This, hBranch, bExpandedOld, bExpandedNew) \
    ((This)->lpVtbl->onBranchExpandedChanged(This, hBranch, bExpandedOld, bExpandedNew))

#define ITvDataSetObserver_notifyItemBeforeRemove(This, hItem) \
    ((This)->lpVtbl->notifyItemBeforeRemove(This, hItem))

/* ITvAdapter C API Macros */
#define ITvAdapter_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ITvAdapter_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ITvAdapter_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ITvAdapter_registerDataSetObserver(This, observer) \
    ((This)->lpVtbl->registerDataSetObserver(This, observer))

#define ITvAdapter_unregisterDataSetObserver(This, observer) \
    ((This)->lpVtbl->unregisterDataSetObserver(This, observer))

#define ITvAdapter_GetItemDataByIndex(This, hItem, idx) \
    ((This)->lpVtbl->GetItemDataByIndex(This, hItem, idx))

#define ITvAdapter_SetItemDataByIndex(This, hItem, idx, data) \
    ((This)->lpVtbl->SetItemDataByIndex(This, hItem, idx, data))

#define ITvAdapter_GetParentItem(This, hItem) \
    ((This)->lpVtbl->GetParentItem(This, hItem))

#define ITvAdapter_GetFirstChildItem(This, hItem) \
    ((This)->lpVtbl->GetFirstChildItem(This, hItem))

#define ITvAdapter_GetLastChildItem(This, hItem) \
    ((This)->lpVtbl->GetLastChildItem(This, hItem))

#define ITvAdapter_GetPrevSiblingItem(This, hItem) \
    ((This)->lpVtbl->GetPrevSiblingItem(This, hItem))

#define ITvAdapter_GetNextSiblingItem(This, hItem) \
    ((This)->lpVtbl->GetNextSiblingItem(This, hItem))

#define ITvAdapter_HasChildren(This, hItem) \
    ((This)->lpVtbl->HasChildren(This, hItem))

#define ITvAdapter_IsItemVisible(This, hItem) \
    ((This)->lpVtbl->IsItemVisible(This, hItem))

#define ITvAdapter_GetFirstVisibleItem(This) \
    ((This)->lpVtbl->GetFirstVisibleItem(This))

#define ITvAdapter_GetLastVisibleItem(This) \
    ((This)->lpVtbl->GetLastVisibleItem(This))

#define ITvAdapter_IsDecendentItem(This, hItem, hChild) \
    ((This)->lpVtbl->IsDecendentItem(This, hItem, hChild))

#define ITvAdapter_GetPrevVisibleItem(This, hItem) \
    ((This)->lpVtbl->GetPrevVisibleItem(This, hItem))

#define ITvAdapter_GetNextVisibleItem(This, hItem) \
    ((This)->lpVtbl->GetNextVisibleItem(This, hItem))

#define ITvAdapter_ExpandItem(This, hItem, code) \
    ((This)->lpVtbl->ExpandItem(This, hItem, code))

#define ITvAdapter_IsItemExpanded(This, hItem) \
    ((This)->lpVtbl->IsItemExpanded(This, hItem))

#define ITvAdapter_SetItemExpanded(This, hItem, bExpanded) \
    ((This)->lpVtbl->SetItemExpanded(This, hItem, bExpanded))

#define ITvAdapter_getView(This, hItem, pItem, pXmlTemplate) \
    ((This)->lpVtbl->getView(This, hItem, pItem, pXmlTemplate))

#define ITvAdapter_getViewType(This, hItem) \
    ((This)->lpVtbl->getViewType(This, hItem))

#define ITvAdapter_getViewTypeCount(This) \
    ((This)->lpVtbl->getViewTypeCount(This))

#define ITvAdapter_getViewDesiredSize(This, ret, hItem, pItem, wid, hei) \
    ((This)->lpVtbl->getViewDesiredSize(This, ret, hItem, pItem, wid, hei))

#define ITvAdapter_isViewWidthMatchParent(This) \
    ((This)->lpVtbl->isViewWidthMatchParent(This))

#define ITvAdapter_InitByTemplate(This, pXmlTemplate) \
    ((This)->lpVtbl->InitByTemplate(This, pXmlTemplate))

#define ITvAdapter_QueryInterface(This, id, ppObj) \
    ((This)->lpVtbl->QueryInterface(This, id, ppObj))

#ifdef __cplusplus
}
#endif

#endif /* __SADAPTER_CAPI_H__ */
