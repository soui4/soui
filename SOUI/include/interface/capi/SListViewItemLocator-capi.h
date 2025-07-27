#ifndef __SLISTVIEWITEMLOCATOR_CAPI_H__
#define __SLISTVIEWITEMLOCATOR_CAPI_H__

#include "../SListViewItemLocator-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * List view item locator constants
 */
#define LISTVIEW_SCALE_NORMAL   100
#define LISTVIEW_INVALID_ITEM   -1
#define LISTVIEW_INVALID_POS    -1

/*
 * C API Helper Macros for SListViewItemLocator Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IListViewItemLocator C API Macros */
#define IListViewItemLocator_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IListViewItemLocator_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IListViewItemLocator_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

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

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IListViewItemLocator Helper Functions */
static inline long IListViewItemLocator_AddRef_C(IListViewItemLocator* pThis)
{
    return IListViewItemLocator_AddRef(pThis);
}

static inline long IListViewItemLocator_Release_C(IListViewItemLocator* pThis)
{
    return IListViewItemLocator_Release(pThis);
}

static inline void IListViewItemLocator_SetAdapter_C(IListViewItemLocator* pThis, ILvAdapter* pAdapter)
{
    IListViewItemLocator_SetAdapter(pThis, pAdapter);
}

static inline void IListViewItemLocator_OnDataSetChanged_C(IListViewItemLocator* pThis)
{
    IListViewItemLocator_OnDataSetChanged(pThis);
}

static inline BOOL IListViewItemLocator_IsFixHeight_C(const IListViewItemLocator* pThis)
{
    return IListViewItemLocator_IsFixHeight(pThis);
}

static inline int IListViewItemLocator_GetItemHeight_C(const IListViewItemLocator* pThis, int iItem)
{
    return IListViewItemLocator_GetItemHeight(pThis, iItem);
}

static inline void IListViewItemLocator_SetItemHeight_C(IListViewItemLocator* pThis, int iItem, int nHeight)
{
    IListViewItemLocator_SetItemHeight(pThis, iItem, nHeight);
}

static inline int IListViewItemLocator_GetTotalHeight_C(IListViewItemLocator* pThis)
{
    return IListViewItemLocator_GetTotalHeight(pThis);
}

static inline int IListViewItemLocator_Item2Position_C(IListViewItemLocator* pThis, int iItem)
{
    return IListViewItemLocator_Item2Position(pThis, iItem);
}

static inline int IListViewItemLocator_Position2Item_C(IListViewItemLocator* pThis, int position)
{
    return IListViewItemLocator_Position2Item(pThis, position);
}

static inline int IListViewItemLocator_GetScrollLineSize_C(const IListViewItemLocator* pThis)
{
    return IListViewItemLocator_GetScrollLineSize(pThis);
}

static inline int IListViewItemLocator_GetDividerSize_C(const IListViewItemLocator* pThis)
{
    return IListViewItemLocator_GetDividerSize(pThis);
}

static inline void IListViewItemLocator_SetScale_C(IListViewItemLocator* pThis, int nScale)
{
    IListViewItemLocator_SetScale(pThis, nScale);
}

/*
 * Convenience macros for common list view item locator operations
 */

/* Adapter management shortcuts */
#define IListViewItemLocator_SetLvAdapter(This, adapter) \
    IListViewItemLocator_SetAdapter(This, adapter)

#define IListViewItemLocator_RefreshData(This) \
    IListViewItemLocator_OnDataSetChanged(This)

#define IListViewItemLocator_NotifyDataChanged(This) \
    IListViewItemLocator_OnDataSetChanged(This)

/* Height management shortcuts */
#define IListViewItemLocator_HasFixedHeight(This) \
    IListViewItemLocator_IsFixHeight(This)

#define IListViewItemLocator_HasVariableHeight(This) \
    (!IListViewItemLocator_IsFixHeight(This))

#define IListViewItemLocator_GetHeight(This, item) \
    IListViewItemLocator_GetItemHeight(This, item)

#define IListViewItemLocator_SetHeight(This, item, height) \
    IListViewItemLocator_SetItemHeight(This, item, height)

#define IListViewItemLocator_GetListHeight(This) \
    IListViewItemLocator_GetTotalHeight(This)

/* Position conversion shortcuts */
#define IListViewItemLocator_ItemToPos(This, item) \
    IListViewItemLocator_Item2Position(This, item)

#define IListViewItemLocator_PosToItem(This, pos) \
    IListViewItemLocator_Position2Item(This, pos)

#define IListViewItemLocator_GetItemPosition(This, item) \
    IListViewItemLocator_Item2Position(This, item)

#define IListViewItemLocator_GetPositionItem(This, pos) \
    IListViewItemLocator_Position2Item(This, pos)

/* Scroll and divider shortcuts */
#define IListViewItemLocator_GetLineSize(This) \
    IListViewItemLocator_GetScrollLineSize(This)

#define IListViewItemLocator_GetSeparatorSize(This) \
    IListViewItemLocator_GetDividerSize(This)

/* Scale shortcuts */
#define IListViewItemLocator_SetNormalScale(This) \
    IListViewItemLocator_SetScale(This, LISTVIEW_SCALE_NORMAL)

#define IListViewItemLocator_SetScale125(This) \
    IListViewItemLocator_SetScale(This, 125)

#define IListViewItemLocator_SetScale150(This) \
    IListViewItemLocator_SetScale(This, 150)

#define IListViewItemLocator_SetScale200(This) \
    IListViewItemLocator_SetScale(This, 200)

/* Safe list view item locator operations */
#define IListViewItemLocator_SafeSetAdapter(This, adapter) \
    do { \
        if ((This) && (adapter)) { \
            IListViewItemLocator_SetAdapter(This, adapter); \
        } \
    } while(0)

#define IListViewItemLocator_SafeSetItemHeight(This, item, height) \
    do { \
        if ((This) && (item) >= 0 && (height) > 0) { \
            IListViewItemLocator_SetItemHeight(This, item, height); \
        } \
    } while(0)

#define IListViewItemLocator_SafeSetScale(This, scale) \
    do { \
        if ((This) && (scale) > 0) { \
            IListViewItemLocator_SetScale(This, scale); \
        } \
    } while(0)

#define IListViewItemLocator_SafeGetItemHeight(This, item) \
    ((This) && (item) >= 0 ? IListViewItemLocator_GetItemHeight(This, item) : 0)

#define IListViewItemLocator_SafeItem2Position(This, item) \
    ((This) && (item) >= 0 ? IListViewItemLocator_Item2Position(This, item) : LISTVIEW_INVALID_POS)

#define IListViewItemLocator_SafePosition2Item(This, pos) \
    ((This) && (pos) >= 0 ? IListViewItemLocator_Position2Item(This, pos) : LISTVIEW_INVALID_ITEM)

/*
 * List view item locator helper functions
 */
static inline BOOL IListViewItemLocator_IsValid_C(IListViewItemLocator* pThis)
{
    return (pThis != NULL);
}

static inline BOOL IListViewItemLocator_IsValidItem_C(IListViewItemLocator* pThis, int iItem)
{
    if (!pThis || iItem < 0) return FALSE;
    
    // We can't directly check item count without adapter access
    // For now, just check if item is non-negative
    return TRUE;
}

static inline int IListViewItemLocator_GetAverageItemHeight_C(IListViewItemLocator* pThis, int itemCount)
{
    if (!pThis || itemCount <= 0) return 0;
    
    int totalHeight = IListViewItemLocator_GetTotalHeight(pThis);
    return totalHeight / itemCount;
}

static inline int IListViewItemLocator_GetVisibleItemCount_C(IListViewItemLocator* pThis, int containerHeight)
{
    if (!pThis || containerHeight <= 0) return 0;
    
    if (IListViewItemLocator_IsFixHeight(pThis)) {
        int itemHeight = IListViewItemLocator_GetItemHeight(pThis, 0);
        if (itemHeight > 0) {
            return (containerHeight + itemHeight - 1) / itemHeight; // Ceiling division
        }
    }
    
    // For variable height, we need to calculate differently
    // This is a simplified estimation
    int totalHeight = IListViewItemLocator_GetTotalHeight(pThis);
    if (totalHeight > 0) {
        // Estimate based on proportion
        return (containerHeight * 10) / totalHeight; // Rough estimate
    }
    
    return 0;
}

static inline BOOL IListViewItemLocator_IsItemVisible_C(IListViewItemLocator* pThis, int iItem, int scrollPos, int containerHeight)
{
    if (!pThis || iItem < 0 || containerHeight <= 0) return FALSE;
    
    int itemPos = IListViewItemLocator_Item2Position(pThis, iItem);
    int itemHeight = IListViewItemLocator_GetItemHeight(pThis, iItem);
    
    if (itemPos == LISTVIEW_INVALID_POS) return FALSE;
    
    int itemTop = itemPos - scrollPos;
    int itemBottom = itemTop + itemHeight;
    
    return (itemBottom > 0 && itemTop < containerHeight);
}

/*
 * List view item locator state management
 */
typedef struct ListViewItemLocatorState {
    IListViewItemLocator* locator;
    ILvAdapter* adapter;
    int totalHeight;
    int scrollLineSize;
    int dividerSize;
    BOOL isFixHeight;
    BOOL isValid;
} ListViewItemLocatorState;

static inline void ListViewItemLocatorState_Init(ListViewItemLocatorState* state, IListViewItemLocator* locator)
{
    if (state) {
        state->locator = locator;
        if (locator) {
            state->adapter = NULL; // Can't get adapter directly
            state->totalHeight = IListViewItemLocator_GetTotalHeight(locator);
            state->scrollLineSize = IListViewItemLocator_GetScrollLineSize(locator);
            state->dividerSize = IListViewItemLocator_GetDividerSize(locator);
            state->isFixHeight = IListViewItemLocator_IsFixHeight(locator);
            state->isValid = TRUE;
        } else {
            memset(state, 0, sizeof(ListViewItemLocatorState));
        }
    }
}

static inline void ListViewItemLocatorState_Update(ListViewItemLocatorState* state)
{
    if (state && state->locator) {
        ListViewItemLocatorState_Init(state, state->locator);
    }
}

static inline void ListViewItemLocatorState_SetAdapter(ListViewItemLocatorState* state, ILvAdapter* adapter)
{
    if (state && state->locator) {
        IListViewItemLocator_SetAdapter(state->locator, adapter);
        state->adapter = adapter;
        ListViewItemLocatorState_Update(state);
    }
}

static inline void ListViewItemLocatorState_RefreshData(ListViewItemLocatorState* state)
{
    if (state && state->locator) {
        IListViewItemLocator_OnDataSetChanged(state->locator);
        ListViewItemLocatorState_Update(state);
    }
}

/*
 * Reference counting helpers
 */
#define IListViewItemLocator_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IListViewItemLocator_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void IListViewItemLocator_DebugInfo_C(IListViewItemLocator* pThis)
{
    if (!pThis) {
        printf("ListViewItemLocator: NULL\n");
        return;
    }
    
    printf("ListViewItemLocator: %p\n", pThis);
    printf("  Fixed Height: %s\n", IListViewItemLocator_IsFixHeight(pThis) ? "Yes" : "No");
    printf("  Total Height: %d\n", IListViewItemLocator_GetTotalHeight(pThis));
    printf("  Scroll Line Size: %d\n", IListViewItemLocator_GetScrollLineSize(pThis));
    printf("  Divider Size: %d\n", IListViewItemLocator_GetDividerSize(pThis));
    
    if (IListViewItemLocator_IsFixHeight(pThis)) {
        printf("  Item Height: %d\n", IListViewItemLocator_GetItemHeight(pThis, 0));
    }
}
#else
#define IListViewItemLocator_DebugInfo_C(This) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SLISTVIEWITEMLOCATOR_CAPI_H__ */
