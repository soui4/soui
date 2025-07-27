#ifndef __STREEVIEWITEMLOCATOR_CAPI_H__
#define __STREEVIEWITEMLOCATOR_CAPI_H__

#include "../STreeViewItemLocator-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Tree view item locator constants
 */
#define TREEVIEW_INVALID_ITEM   NULL
#define TREEVIEW_INVALID_POS    -1

/*
 * C API Helper Macros for STreeViewItemLocator Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ITreeViewItemLocator C API Macros */
#define ITreeViewItemLocator_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ITreeViewItemLocator_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ITreeViewItemLocator_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

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

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* ITreeViewItemLocator Helper Functions */
static inline long ITreeViewItemLocator_AddRef_C(ITreeViewItemLocator* pThis)
{
    return ITreeViewItemLocator_AddRef(pThis);
}

static inline long ITreeViewItemLocator_Release_C(ITreeViewItemLocator* pThis)
{
    return ITreeViewItemLocator_Release(pThis);
}

static inline void ITreeViewItemLocator_SetAdapter_C(ITreeViewItemLocator* pThis, ITvAdapter* pAdapter)
{
    ITreeViewItemLocator_SetAdapter(pThis, pAdapter);
}

static inline void ITreeViewItemLocator_OnBranchChanged_C(ITreeViewItemLocator* pThis, HSTREEITEM hItem)
{
    ITreeViewItemLocator_OnBranchChanged(pThis, hItem);
}

static inline void ITreeViewItemLocator_OnBranchExpandedChanged_C(ITreeViewItemLocator* pThis, HSTREEITEM hItem, BOOL bExpandedOld, BOOL bExpandedNew)
{
    ITreeViewItemLocator_OnBranchExpandedChanged(pThis, hItem, bExpandedOld, bExpandedNew);
}

static inline int ITreeViewItemLocator_GetTotalHeight_C(const ITreeViewItemLocator* pThis)
{
    return ITreeViewItemLocator_GetTotalHeight(pThis);
}

static inline int ITreeViewItemLocator_GetTotalWidth_C(const ITreeViewItemLocator* pThis)
{
    return ITreeViewItemLocator_GetTotalWidth(pThis);
}

static inline int ITreeViewItemLocator_Item2Position_C(const ITreeViewItemLocator* pThis, HSTREEITEM hItem)
{
    return ITreeViewItemLocator_Item2Position(pThis, hItem);
}

static inline HSTREEITEM ITreeViewItemLocator_Position2Item_C(const ITreeViewItemLocator* pThis, int position)
{
    return ITreeViewItemLocator_Position2Item(pThis, position);
}

static inline int ITreeViewItemLocator_GetScrollLineSize_C(const ITreeViewItemLocator* pThis)
{
    return ITreeViewItemLocator_GetScrollLineSize(pThis);
}

static inline void ITreeViewItemLocator_SetItemWidth_C(ITreeViewItemLocator* pThis, HSTREEITEM hItem, int nWidth)
{
    ITreeViewItemLocator_SetItemWidth(pThis, hItem, nWidth);
}

static inline int ITreeViewItemLocator_GetItemWidth_C(const ITreeViewItemLocator* pThis, HSTREEITEM hItem)
{
    return ITreeViewItemLocator_GetItemWidth(pThis, hItem);
}

static inline void ITreeViewItemLocator_SetItemHeight_C(ITreeViewItemLocator* pThis, HSTREEITEM hItem, int nHeight)
{
    ITreeViewItemLocator_SetItemHeight(pThis, hItem, nHeight);
}

static inline int ITreeViewItemLocator_GetItemHeight_C(const ITreeViewItemLocator* pThis, HSTREEITEM hItem)
{
    return ITreeViewItemLocator_GetItemHeight(pThis, hItem);
}

static inline int ITreeViewItemLocator_GetItemIndent_C(const ITreeViewItemLocator* pThis, HSTREEITEM hItem)
{
    return ITreeViewItemLocator_GetItemIndent(pThis, hItem);
}

static inline int ITreeViewItemLocator_GetIndent_C(const ITreeViewItemLocator* pThis)
{
    return ITreeViewItemLocator_GetIndent(pThis);
}

static inline void ITreeViewItemLocator_SetIndent_C(ITreeViewItemLocator* pThis, int nIndent)
{
    ITreeViewItemLocator_SetIndent(pThis, nIndent);
}

/*
 * Convenience macros for common tree view item locator operations
 */

/* Adapter management shortcuts */
#define ITreeViewItemLocator_SetTvAdapter(This, adapter) \
    ITreeViewItemLocator_SetAdapter(This, adapter)

/* Branch management shortcuts */
#define ITreeViewItemLocator_NotifyBranchChanged(This, item) \
    ITreeViewItemLocator_OnBranchChanged(This, item)

#define ITreeViewItemLocator_NotifyExpanded(This, item, oldState, newState) \
    ITreeViewItemLocator_OnBranchExpandedChanged(This, item, oldState, newState)

#define ITreeViewItemLocator_ExpandBranch(This, item) \
    ITreeViewItemLocator_OnBranchExpandedChanged(This, item, FALSE, TRUE)

#define ITreeViewItemLocator_CollapseBranch(This, item) \
    ITreeViewItemLocator_OnBranchExpandedChanged(This, item, TRUE, FALSE)

/* Dimension shortcuts */
#define ITreeViewItemLocator_GetTreeHeight(This) \
    ITreeViewItemLocator_GetTotalHeight(This)

#define ITreeViewItemLocator_GetTreeWidth(This) \
    ITreeViewItemLocator_GetTotalWidth(This)

#define ITreeViewItemLocator_GetLineSize(This) \
    ITreeViewItemLocator_GetScrollLineSize(This)

/* Position conversion shortcuts */
#define ITreeViewItemLocator_ItemToPos(This, item) \
    ITreeViewItemLocator_Item2Position(This, item)

#define ITreeViewItemLocator_PosToItem(This, pos) \
    ITreeViewItemLocator_Position2Item(This, pos)

/* Item dimension shortcuts */
#define ITreeViewItemLocator_SetWidth(This, item, width) \
    ITreeViewItemLocator_SetItemWidth(This, item, width)

#define ITreeViewItemLocator_GetWidth(This, item) \
    ITreeViewItemLocator_GetItemWidth(This, item)

#define ITreeViewItemLocator_SetHeight(This, item, height) \
    ITreeViewItemLocator_SetItemHeight(This, item, height)

#define ITreeViewItemLocator_GetHeight(This, item) \
    ITreeViewItemLocator_GetItemHeight(This, item)

/* Indent shortcuts */
#define ITreeViewItemLocator_GetItemLevel(This, item) \
    ITreeViewItemLocator_GetItemIndent(This, item)

#define ITreeViewItemLocator_GetBaseIndent(This) \
    ITreeViewItemLocator_GetIndent(This)

#define ITreeViewItemLocator_SetBaseIndent(This, indent) \
    ITreeViewItemLocator_SetIndent(This, indent)

/* Safe operations */
#define ITreeViewItemLocator_SafeSetAdapter(This, adapter) \
    do { \
        if ((This) && (adapter)) { \
            ITreeViewItemLocator_SetAdapter(This, adapter); \
        } \
    } while(0)

#define ITreeViewItemLocator_SafeNotifyBranchChanged(This, item) \
    do { \
        if ((This) && (item)) { \
            ITreeViewItemLocator_OnBranchChanged(This, item); \
        } \
    } while(0)

#define ITreeViewItemLocator_SafeSetItemWidth(This, item, width) \
    do { \
        if ((This) && (item) && (width) > 0) { \
            ITreeViewItemLocator_SetItemWidth(This, item, width); \
        } \
    } while(0)

#define ITreeViewItemLocator_SafeSetItemHeight(This, item, height) \
    do { \
        if ((This) && (item) && (height) > 0) { \
            ITreeViewItemLocator_SetItemHeight(This, item, height); \
        } \
    } while(0)

/*
 * Tree view item locator helper functions
 */
static inline BOOL ITreeViewItemLocator_IsValid_C(ITreeViewItemLocator* pThis)
{
    return (pThis != NULL);
}

static inline BOOL ITreeViewItemLocator_IsValidItem_C(HSTREEITEM hItem)
{
    return (hItem != TREEVIEW_INVALID_ITEM);
}

static inline BOOL ITreeViewItemLocator_IsItemVisible_C(ITreeViewItemLocator* pThis, HSTREEITEM hItem, int scrollPos, int containerHeight)
{
    if (!pThis || !hItem || containerHeight <= 0) return FALSE;
    
    int itemPos = ITreeViewItemLocator_Item2Position(pThis, hItem);
    if (itemPos == TREEVIEW_INVALID_POS) return FALSE;
    
    int itemHeight = ITreeViewItemLocator_GetItemHeight(pThis, hItem);
    int itemTop = itemPos - scrollPos;
    int itemBottom = itemTop + itemHeight;
    
    return (itemBottom > 0 && itemTop < containerHeight);
}

/*
 * Tree view item locator state management
 */
typedef struct TreeViewItemLocatorState {
    ITreeViewItemLocator* locator;
    ITvAdapter* adapter;
    int totalHeight;
    int totalWidth;
    int scrollLineSize;
    int baseIndent;
    BOOL isValid;
} TreeViewItemLocatorState;

static inline void TreeViewItemLocatorState_Init(TreeViewItemLocatorState* state, ITreeViewItemLocator* locator)
{
    if (state) {
        state->locator = locator;
        if (locator) {
            state->adapter = NULL; // Can't get adapter directly
            state->totalHeight = ITreeViewItemLocator_GetTotalHeight(locator);
            state->totalWidth = ITreeViewItemLocator_GetTotalWidth(locator);
            state->scrollLineSize = ITreeViewItemLocator_GetScrollLineSize(locator);
            state->baseIndent = ITreeViewItemLocator_GetIndent(locator);
            state->isValid = TRUE;
        } else {
            memset(state, 0, sizeof(TreeViewItemLocatorState));
        }
    }
}

static inline void TreeViewItemLocatorState_Update(TreeViewItemLocatorState* state)
{
    if (state && state->locator) {
        TreeViewItemLocatorState_Init(state, state->locator);
    }
}

static inline void TreeViewItemLocatorState_SetAdapter(TreeViewItemLocatorState* state, ITvAdapter* adapter)
{
    if (state && state->locator) {
        ITreeViewItemLocator_SetAdapter(state->locator, adapter);
        state->adapter = adapter;
        TreeViewItemLocatorState_Update(state);
    }
}

/*
 * Reference counting helpers
 */
#define ITreeViewItemLocator_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define ITreeViewItemLocator_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void ITreeViewItemLocator_DebugInfo_C(ITreeViewItemLocator* pThis)
{
    if (!pThis) {
        printf("TreeViewItemLocator: NULL\n");
        return;
    }
    
    printf("TreeViewItemLocator: %p\n", pThis);
    printf("  Total Height: %d\n", ITreeViewItemLocator_GetTotalHeight(pThis));
    printf("  Total Width: %d\n", ITreeViewItemLocator_GetTotalWidth(pThis));
    printf("  Scroll Line Size: %d\n", ITreeViewItemLocator_GetScrollLineSize(pThis));
    printf("  Base Indent: %d\n", ITreeViewItemLocator_GetIndent(pThis));
}
#else
#define ITreeViewItemLocator_DebugInfo_C(This) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STREEVIEWITEMLOCATOR_CAPI_H__ */
