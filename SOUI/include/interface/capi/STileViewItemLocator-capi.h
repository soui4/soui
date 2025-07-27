#ifndef __STILEVIEWITEMLOCATOR_CAPI_H__
#define __STILEVIEWITEMLOCATOR_CAPI_H__

#include "../STileViewItemLocator-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Tile view item locator constants
 */
#define TILEVIEW_SCALE_NORMAL   100
#define TILEVIEW_INVALID_ITEM   -1
#define TILEVIEW_INVALID_POS    -1

/*
 * C API Helper Macros for STileViewItemLocator Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ITileViewItemLocator C API Macros */
#define ITileViewItemLocator_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ITileViewItemLocator_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ITileViewItemLocator_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

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

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* ITileViewItemLocator Helper Functions */
static inline long ITileViewItemLocator_AddRef_C(ITileViewItemLocator* pThis)
{
    return ITileViewItemLocator_AddRef(pThis);
}

static inline long ITileViewItemLocator_Release_C(ITileViewItemLocator* pThis)
{
    return ITileViewItemLocator_Release(pThis);
}

static inline void ITileViewItemLocator_SetAdapter_C(ITileViewItemLocator* pThis, ILvAdapter* pAdapter)
{
    ITileViewItemLocator_SetAdapter(pThis, pAdapter);
}

static inline void ITileViewItemLocator_OnDataSetChanged_C(ITileViewItemLocator* pThis)
{
    ITileViewItemLocator_OnDataSetChanged(pThis);
}

static inline int ITileViewItemLocator_GetItemHeight_C(const ITileViewItemLocator* pThis, int iItem)
{
    return ITileViewItemLocator_GetItemHeight(pThis, iItem);
}

static inline void ITileViewItemLocator_SetItemHeight_C(ITileViewItemLocator* pThis, int iItem, int nHeight)
{
    ITileViewItemLocator_SetItemHeight(pThis, iItem, nHeight);
}

static inline RECT ITileViewItemLocator_GetItemRect_C(ITileViewItemLocator* pThis, int iItem)
{
    return ITileViewItemLocator_GetItemRect(pThis, iItem);
}

static inline void ITileViewItemLocator_SetTileViewWidth_C(ITileViewItemLocator* pThis, int width, BOOL bDpiAware)
{
    ITileViewItemLocator_SetTileViewWidth(pThis, width, bDpiAware);
}

static inline void ITileViewItemLocator_GetItemRowAndColIndex_C(ITileViewItemLocator* pThis, int iItem, int* row, int* col)
{
    ITileViewItemLocator_GetItemRowAndColIndex(pThis, iItem, row, col);
}

static inline BOOL ITileViewItemLocator_IsLastInRow_C(ITileViewItemLocator* pThis, int iItem)
{
    return ITileViewItemLocator_IsLastInRow(pThis, iItem);
}

static inline int ITileViewItemLocator_GetUpItem_C(ITileViewItemLocator* pThis, int iItem)
{
    return ITileViewItemLocator_GetUpItem(pThis, iItem);
}

static inline int ITileViewItemLocator_GetDownItem_C(ITileViewItemLocator* pThis, int iItem)
{
    return ITileViewItemLocator_GetDownItem(pThis, iItem);
}

static inline int ITileViewItemLocator_GetTotalHeight_C(ITileViewItemLocator* pThis)
{
    return ITileViewItemLocator_GetTotalHeight(pThis);
}

static inline int ITileViewItemLocator_Item2Position_C(ITileViewItemLocator* pThis, int iItem)
{
    return ITileViewItemLocator_Item2Position(pThis, iItem);
}

static inline int ITileViewItemLocator_Position2Item_C(ITileViewItemLocator* pThis, int position)
{
    return ITileViewItemLocator_Position2Item(pThis, position);
}

static inline int ITileViewItemLocator_GetScrollLineSize_C(const ITileViewItemLocator* pThis)
{
    return ITileViewItemLocator_GetScrollLineSize(pThis);
}

static inline int ITileViewItemLocator_GetMarginSize_C(const ITileViewItemLocator* pThis)
{
    return ITileViewItemLocator_GetMarginSize(pThis);
}

static inline int ITileViewItemLocator_SetScale_C(ITileViewItemLocator* pThis, int scale)
{
    return ITileViewItemLocator_SetScale(pThis, scale);
}

static inline int ITileViewItemLocator_GetCountInRow_C(const ITileViewItemLocator* pThis)
{
    return ITileViewItemLocator_GetCountInRow(pThis);
}

static inline int ITileViewItemLocator_GetItemLineHeight_C(const ITileViewItemLocator* pThis)
{
    return ITileViewItemLocator_GetItemLineHeight(pThis);
}

static inline int ITileViewItemLocator_GetItemWidth_C(const ITileViewItemLocator* pThis)
{
    return ITileViewItemLocator_GetItemWidth(pThis);
}

/*
 * Convenience macros for common tile view item locator operations
 */

/* Adapter management shortcuts */
#define ITileViewItemLocator_SetLvAdapter(This, adapter) \
    ITileViewItemLocator_SetAdapter(This, adapter)

#define ITileViewItemLocator_RefreshData(This) \
    ITileViewItemLocator_OnDataSetChanged(This)

/* Grid navigation shortcuts */
#define ITileViewItemLocator_GetRowCol(This, item, row, col) \
    ITileViewItemLocator_GetItemRowAndColIndex(This, item, row, col)

#define ITileViewItemLocator_IsRowEnd(This, item) \
    ITileViewItemLocator_IsLastInRow(This, item)

#define ITileViewItemLocator_GetPrevRowItem(This, item) \
    ITileViewItemLocator_GetUpItem(This, item)

#define ITileViewItemLocator_GetNextRowItem(This, item) \
    ITileViewItemLocator_GetDownItem(This, item)

/* Dimension shortcuts */
#define ITileViewItemLocator_GetColumns(This) \
    ITileViewItemLocator_GetCountInRow(This)

#define ITileViewItemLocator_GetRowHeight(This) \
    ITileViewItemLocator_GetItemLineHeight(This)

#define ITileViewItemLocator_GetTileWidth(This) \
    ITileViewItemLocator_GetItemWidth(This)

#define ITileViewItemLocator_GetListHeight(This) \
    ITileViewItemLocator_GetTotalHeight(This)

/* Position conversion shortcuts */
#define ITileViewItemLocator_ItemToPos(This, item) \
    ITileViewItemLocator_Item2Position(This, item)

#define ITileViewItemLocator_PosToItem(This, pos) \
    ITileViewItemLocator_Position2Item(This, pos)

/* Scale shortcuts */
#define ITileViewItemLocator_SetNormalScale(This) \
    ITileViewItemLocator_SetScale(This, TILEVIEW_SCALE_NORMAL)

#define ITileViewItemLocator_SetScale125(This) \
    ITileViewItemLocator_SetScale(This, 125)

#define ITileViewItemLocator_SetScale150(This) \
    ITileViewItemLocator_SetScale(This, 150)

#define ITileViewItemLocator_SetScale200(This) \
    ITileViewItemLocator_SetScale(This, 200)

/* Safe operations */
#define ITileViewItemLocator_SafeSetAdapter(This, adapter) \
    do { \
        if ((This) && (adapter)) { \
            ITileViewItemLocator_SetAdapter(This, adapter); \
        } \
    } while(0)

#define ITileViewItemLocator_SafeSetWidth(This, width, dpiAware) \
    do { \
        if ((This) && (width) > 0) { \
            ITileViewItemLocator_SetTileViewWidth(This, width, dpiAware); \
        } \
    } while(0)

#define ITileViewItemLocator_SafeGetItemRect(This, item, pRect) \
    do { \
        if ((This) && (item) >= 0 && (pRect)) { \
            *(pRect) = ITileViewItemLocator_GetItemRect(This, item); \
        } \
    } while(0)

/*
 * Tile view item locator helper functions
 */
static inline BOOL ITileViewItemLocator_IsValid_C(ITileViewItemLocator* pThis)
{
    return (pThis != NULL);
}

static inline BOOL ITileViewItemLocator_IsValidItem_C(ITileViewItemLocator* pThis, int iItem)
{
    return (pThis != NULL && iItem >= 0);
}

static inline int ITileViewItemLocator_GetRowCount_C(ITileViewItemLocator* pThis, int totalItems)
{
    if (!pThis || totalItems <= 0) return 0;
    
    int columnsPerRow = ITileViewItemLocator_GetCountInRow(pThis);
    if (columnsPerRow <= 0) return 0;
    
    return (totalItems + columnsPerRow - 1) / columnsPerRow; // Ceiling division
}

static inline BOOL ITileViewItemLocator_IsItemVisible_C(ITileViewItemLocator* pThis, int iItem, int scrollPos, int containerHeight)
{
    if (!pThis || iItem < 0 || containerHeight <= 0) return FALSE;
    
    RECT itemRect = ITileViewItemLocator_GetItemRect(pThis, iItem);
    int itemTop = itemRect.top - scrollPos;
    int itemBottom = itemRect.bottom - scrollPos;
    
    return (itemBottom > 0 && itemTop < containerHeight);
}

/*
 * Tile view item locator state management
 */
typedef struct TileViewItemLocatorState {
    ITileViewItemLocator* locator;
    ILvAdapter* adapter;
    int totalHeight;
    int scrollLineSize;
    int marginSize;
    int countInRow;
    int itemLineHeight;
    int itemWidth;
    BOOL isValid;
} TileViewItemLocatorState;

static inline void TileViewItemLocatorState_Init(TileViewItemLocatorState* state, ITileViewItemLocator* locator)
{
    if (state) {
        state->locator = locator;
        if (locator) {
            state->adapter = NULL; // Can't get adapter directly
            state->totalHeight = ITileViewItemLocator_GetTotalHeight(locator);
            state->scrollLineSize = ITileViewItemLocator_GetScrollLineSize(locator);
            state->marginSize = ITileViewItemLocator_GetMarginSize(locator);
            state->countInRow = ITileViewItemLocator_GetCountInRow(locator);
            state->itemLineHeight = ITileViewItemLocator_GetItemLineHeight(locator);
            state->itemWidth = ITileViewItemLocator_GetItemWidth(locator);
            state->isValid = TRUE;
        } else {
            memset(state, 0, sizeof(TileViewItemLocatorState));
        }
    }
}

static inline void TileViewItemLocatorState_Update(TileViewItemLocatorState* state)
{
    if (state && state->locator) {
        TileViewItemLocatorState_Init(state, state->locator);
    }
}

/*
 * Reference counting helpers
 */
#define ITileViewItemLocator_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define ITileViewItemLocator_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Debugging helpers
 */
#ifdef _DEBUG
static inline void ITileViewItemLocator_DebugInfo_C(ITileViewItemLocator* pThis)
{
    if (!pThis) {
        printf("TileViewItemLocator: NULL\n");
        return;
    }
    
    printf("TileViewItemLocator: %p\n", pThis);
    printf("  Total Height: %d\n", ITileViewItemLocator_GetTotalHeight(pThis));
    printf("  Columns per Row: %d\n", ITileViewItemLocator_GetCountInRow(pThis));
    printf("  Item Line Height: %d\n", ITileViewItemLocator_GetItemLineHeight(pThis));
    printf("  Item Width: %d\n", ITileViewItemLocator_GetItemWidth(pThis));
    printf("  Margin Size: %d\n", ITileViewItemLocator_GetMarginSize(pThis));
    printf("  Scroll Line Size: %d\n", ITileViewItemLocator_GetScrollLineSize(pThis));
}
#else
#define ITileViewItemLocator_DebugInfo_C(This) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STILEVIEWITEMLOCATOR_CAPI_H__ */
