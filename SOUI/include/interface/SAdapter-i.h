#ifndef __SADAPTER_I__H__
#define __SADAPTER_I__H__

#include <interface/obj-ref-i.h>
#include <interface/SWindow-i.h>
#include <interface/sstring-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ILvDataSetObserver
DECLARE_INTERFACE_(ILvDataSetObserver, IObjRef)
{
    /**
     * @brief Add reference
     * @return long -- reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Release reference
     * @return long -- reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Called when the entire data set changes
     * @return void
     */
    STDMETHOD_(void, onChanged)(THIS) PURE;

    /**
     * @brief Called when the entire data set becomes invalid
     * @return void
     */
    STDMETHOD_(void, onInvalidated)(THIS) PURE;

    /**
     * @brief Notify that list item data has changed
     * @param iItem int -- list index
     * @return void
     */
    STDMETHOD_(void, OnItemChanged)(THIS_ int iItem) PURE;
};

#undef INTERFACE
#define INTERFACE ILvAdapter
DECLARE_INTERFACE_(ILvAdapter, IObjRef)
{
    /**
     * @brief Add reference
     * @return long -- reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Release reference
     * @return long -- reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Register an observer, called when data changes
     * @param observer ILvDataSetObserver* -- observer object
     * @return void
     */
    STDMETHOD_(void, registerDataSetObserver)(THIS_ ILvDataSetObserver * observer) PURE;

    /**
     * @brief Unregister an observer
     * @param observer ILvDataSetObserver* -- observer object
     * @return void
     */
    STDMETHOD_(void, unregisterDataSetObserver)(THIS_ ILvDataSetObserver * observer) PURE;

    /**
     * @brief Get the number of items in the data set
     * @return int -- number of items
     */
    STDMETHOD_(int, getCount)(THIS) PURE;

    /**
     * @brief Get the view displaying data at the specified position
     * @param position int -- position in the data set
     * @param pItem IWindow* -- old view, reuse it if possible
     * @param xmlTemplate IXmlNode* -- provided XML template
     * @return void
     */
    STDMETHOD_(void, getView)(THIS_ int position, IWindow *pItem, IXmlNode *xmlTemplate) PURE;

    /**
     * @brief Get the view type that will be created by getView
     * @param position int -- position in the data set
     * @param dwState DWORD -- view state
     * @return int -- view type
     */
    STDMETHOD_(int, getItemViewType)(THIS_ int position, DWORD dwState) PURE;

    /**
     * @brief Return the number of view types that will be created by getView
     * @return int -- number of view types
     */
    STDMETHOD_(int, getViewTypeCount)(THIS) PURE;

    /**
     * @brief Compute the display size of a list item
     * @param ret SIZE* -- list item size
     * @param position int -- list item index
     * @param pItem IWindow* -- list item window
     * @param wid int -- parent window width
     * @param hei int -- parent window height
     * @return void
     */
    STDMETHOD_(void, getViewDesiredSize)(THIS_ SIZE * ret, int position, IWindow *pItem, int wid, int hei) PURE;

    /**
     * @brief Determine whether the data is empty
     * @return BOOL -- TRUE: empty, FALSE: not empty
     */
    STDMETHOD_(BOOL, isEmpty)(THIS) PURE;

    /**
     * @brief Get the list item description
     * @param positoin int -- list item index
     * @param pDesc IStringW* -- returned string description
     * @return void
     */
    STDMETHOD_(void, getItemDesc)(THIS_ int positoin, IStringW *pDesc) PURE;

    /**
     * @brief Initialize the adapter from the specified template XML data
     * @param xmlTemplate IXmlNode* -- XML template
     * @return void
     */
    STDMETHOD_(void, InitByTemplate)(THIS_ IXmlNode * xmlTemplate) PURE;

    /**
     * @brief Query a user-defined interface from the adapter
     * @param id REFGUID -- interface ID
     * @param ppObj IObjRef** -- returned interface
     * @return HRESULT -- S_OK: success
     */
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef * *ppObj) PURE;
};

#undef INTERFACE
#define INTERFACE IMcAdapter
DECLARE_INTERFACE_(IMcAdapter, ILvAdapter)
{
    /**
     * @brief Add reference
     * @return long -- reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Release reference
     * @return long -- reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Register an observer, called when data changes
     * @param observer ILvDataSetObserver* -- observer object
     * @return void
     */
    STDMETHOD_(void, registerDataSetObserver)(THIS_ ILvDataSetObserver * observer) PURE;

    /**
     * @brief Unregister an observer
     * @param observer ILvDataSetObserver* -- observer object
     * @return void
     */
    STDMETHOD_(void, unregisterDataSetObserver)(THIS_ ILvDataSetObserver * observer) PURE;

    /**
     * @brief Get the number of items in the data set
     * @return int -- number of items
     */
    STDMETHOD_(int, getCount)(THIS) PURE;

    /**
     * @brief Get the view displaying data at the specified position
     * @param position int -- position in the data set
     * @param pItem IWindow* -- old view, reuse it if possible
     * @param xmlTemplate IXmlNode* -- provided XML template
     * @return void
     */
    STDMETHOD_(void, getView)(THIS_ int position, IWindow *pItem, IXmlNode *xmlTemplate) PURE;

    /**
     * @brief Get the view type that will be created by getView
     * @param position int -- position in the data set
     * @param dwState DWORD -- view state
     * @return int -- view type
     */
    STDMETHOD_(int, getItemViewType)(THIS_ int position, DWORD dwState) PURE;

    /**
     * @brief Return the number of view types that will be created by getView
     * @return int -- number of view types
     */
    STDMETHOD_(int, getViewTypeCount)(THIS) PURE;

    /**
     * @brief Compute the display size of a list item
     * @param ret SIZE* -- list item size
     * @param position int -- list item index
     * @param pItem IWindow* -- list item window
     * @param wid int -- parent window width
     * @param hei int -- parent window height
     * @return void
     */
    STDMETHOD_(void, getViewDesiredSize)(THIS_ SIZE * ret, int position, IWindow *pItem, int wid, int hei) PURE;

    /**
     * @brief Determine whether the data is empty
     * @return BOOL -- TRUE: empty, FALSE: not empty
     */
    STDMETHOD_(BOOL, isEmpty)(THIS) PURE;

    /**
     * @brief Get the list item description
     * @param positoin int -- list item index
     * @param pDesc IStringW* -- returned string description
     * @return void
     */
    STDMETHOD_(void, getItemDesc)(THIS_ int positoin, IStringW *pDesc) PURE;

    /**
     * @brief Initialize the adapter from the specified template XML data
     * @param xmlTemplate IXmlNode* -- XML template
     * @return void
     */
    STDMETHOD_(void, InitByTemplate)(THIS_ IXmlNode * xmlTemplate) PURE;

    /**
     * @brief Query a user-defined interface from the adapter
     * @param id REFGUID -- interface ID
     * @param ppObj IObjRef** -- returned interface
     * @return HRESULT -- S_OK: success
     */
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef * *ppObj) PURE;

    /**
     * @brief Get the window name corresponding to a column in the template
     * @param iCol int -- column index
     * @param pName IStringW* -- column name
     * @return void
     */
    STDMETHOD_(void, GetColumnName)(CTHIS_ int iCol, IStringW *pName) SCONST PURE;

    /**
     * @brief Let the Adapter decide whether a column should be shown
     * @param iCol int -- column ordinal
     * @return BOOL -- TRUE: show, FALSE: hide
     * @remark Shown by default
     */
    STDMETHOD_(BOOL, IsColumnVisible)(CTHIS_ int iCol) SCONST PURE;

    /**
     * @brief Sort interface
     * @param iCol int -- sort column
     * @param pFmts UINT* -- current column sort flags
     * @param nCols int -- total number of columns, length of the pFmts array
     * @return BOOL -- TRUE: sorting performed, FALSE: sorting not performed
     */
    STDMETHOD_(BOOL, OnSort)(THIS_ int iCol, UINT *pFmts, int nCols) PURE;

    /**
     * @brief Set column width interface
     * @param pColWid int* -- column width data
     * @param nCols int -- total number of columns
     * @return void
     */
    STDMETHOD_(void, SetColumnsWidth)(THIS_ int *pColWid, int nCols) PURE;
};

#undef INTERFACE
#define INTERFACE ITvDataSetObserver
DECLARE_INTERFACE_(ITvDataSetObserver, IObjRef)
{
    /**
     * @brief Add reference
     * @return long -- reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Release reference
     * @return long -- reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Called when the entire data set changes
     * @param hBranch HSTREEITEM -- target branch
     * @return void
     */
    STDMETHOD_(void, onBranchChanged)(THIS_ HSTREEITEM hBranch) PURE;

    /**
     * @brief Called when the data of the specified tree item becomes invalid
     * @param hBranch HSTREEITEM -- target branch
     * @param bInvalidParents BOOL -- target's parents become invalid
     * @param bInvalidChildren BOOL -- target's children become invalid
     * @return void
     */
    STDMETHOD_(void, onBranchInvalidated)
    (THIS_ HSTREEITEM hBranch, BOOL bInvalidParents, BOOL bInvalidChildren) PURE;

    /**
     * @brief Called when the expand state of the specified tree item changes
     * @param hBranch HSTREEITEM -- target branch
     * @param bExpandedOld BOOL -- old expand state
     * @param bExpandedNew BOOL -- new expand state
     * @return void
     */
    STDMETHOD_(void, onBranchExpandedChanged)
    (THIS_ HSTREEITEM hBranch, BOOL bExpandedOld, BOOL bExpandedNew) PURE;

    /**
     * @brief Called when the specified tree item is about to be removed
     * @param hItem HSTREEITEM -- target item
     * @return void
     */
    STDMETHOD_(void, notifyItemBeforeRemove)
    (THIS_ HSTREEITEM hItem) PURE;
};

#define ITEM_NULL  0
#define ITEM_ROOT  STVI_ROOT
#define ITEM_LAST  STVI_LAST
#define ITEM_FIRST STVI_FIRST

typedef enum DATA_INDEX
{
    DATA_INDEX_ITEM_HEIGHT = 0, /**< Row height */
    DATA_INDEX_ITEM_WIDTH,      /**< Row width */
    DATA_INDEX_BRANCH_HEIGHT,   /**< Branch height */
    DATA_INDEX_BRANCH_WIDTH,    /**< Branch width, excluding indentation */
    DATA_INDEX_ITEM_EXPANDED,   /**< Child expand state */
    DATA_INDEX_ITEM_OFFSET,     /**< Y direction offset of the current node within its parent node */
    DATA_INDEX_ITEM_USER,       /**< Custom data */
    DATA_INDEX_NUMBER
} DATA_INDEX;

enum
{
    TVC_COLLAPSE = 0x0001,
    TVC_EXPAND = 0x0002,
    TVC_TOGGLE = 0x0003,
};

#undef INTERFACE
#define INTERFACE ITvAdapter
DECLARE_INTERFACE_(ITvAdapter, IObjRef)
{
    /**
     * @brief Add reference
     * @return long -- reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Release reference
     * @return long -- reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Register an observer, called when data changes
     * @param observer ITvDataSetObserver* -- observer object
     * @return void
     */
    STDMETHOD_(void, registerDataSetObserver)(THIS_ ITvDataSetObserver * observer) PURE;

    /**
     * @brief Unregister an observer
     * @param observer ITvDataSetObserver* -- observer object
     * @return void
     */
    STDMETHOD_(void, unregisterDataSetObserver)(THIS_ ITvDataSetObserver * observer) PURE;

    /**
     * @brief Get the data of the specified index in hItem
     * @param hItem HSTREEITEM -- item index
     * @param idx DATA_INDEX -- data type
     * @return ULONG_PTR -- data
     */
    STDMETHOD_(ULONG_PTR, GetItemDataByIndex)(CTHIS_ HSTREEITEM hItem, DATA_INDEX idx) SCONST PURE;

    /**
     * @brief Save the data of the specified index in hItem
     * @param hItem HSTREEITEM -- item index
     * @param idx DATA_INDEX -- data type
     * @param data ULONG_PTR -- data
     * @return void
     */
    STDMETHOD_(void, SetItemDataByIndex)
    (THIS_ HSTREEITEM hItem, DATA_INDEX idx, ULONG_PTR data) PURE;

    /**
     * @brief Get the parent item
     * @param hItem HSTREEITEM -- item index
     * @return HSTREEITEM -- parent item
     */
    STDMETHOD_(HSTREEITEM, GetParentItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Get the first child item
     * @param hItem HSTREEITEM -- item index
     * @return HSTREEITEM -- first child item
     */
    STDMETHOD_(HSTREEITEM, GetFirstChildItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Get the last child item
     * @param hItem HSTREEITEM -- item index
     * @return HSTREEITEM -- last child item
     */
    STDMETHOD_(HSTREEITEM, GetLastChildItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Get the previous sibling item
     * @param hItem HSTREEITEM -- item index
     * @return HSTREEITEM -- previous sibling item
     */
    STDMETHOD_(HSTREEITEM, GetPrevSiblingItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Get the next sibling item
     * @param hItem HSTREEITEM -- item index
     * @return HSTREEITEM -- next sibling item
     */
    STDMETHOD_(HSTREEITEM, GetNextSiblingItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Whether it has child items
     * @param hItem HSTREEITEM -- item index
     * @return BOOL -- TRUE: has children, FALSE: no children
     */
    STDMETHOD_(BOOL, HasChildren)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Whether the item is currently visible
     * @param hItem HSTREEITEM -- item index
     * @return BOOL -- TRUE: visible, FALSE: not visible (collapsed)
     */
    STDMETHOD_(BOOL, IsItemVisible)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Get the first visible item
     * @return HSTREEITEM -- first visible item
     */
    STDMETHOD_(HSTREEITEM, GetFirstVisibleItem)(CTHIS) SCONST PURE;

    /**
     * @brief Get the last visible item
     * @return HSTREEITEM--last visible item
     */
    STDMETHOD_(HSTREEITEM, GetLastVisibleItem)(CTHIS) SCONST PURE;

    /**
     * @brief Test whether one node is a descendant of another node
     * @param HSTREEITEM--hItem parent node
     * @param HSTREEITEM--hChild child node being tested
     * @return TRUE--is a descendant node
     */
    STDMETHOD_(BOOL, IsDecendentItem)(CTHIS_ HSTREEITEM hItem, HSTREEITEM hChild) SCONST PURE;

    /**
     * @brief Get the previous visible item
     * @return HSTREEITEM--previous visible item
     */
    STDMETHOD_(HSTREEITEM, GetPrevVisibleItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Get the next visible item
     * @return HSTREEITEM--next visible item
     */
    STDMETHOD_(HSTREEITEM, GetNextVisibleItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Expand/collapse child items
     * @param hItem HSTREEITEM--target item
     * @param code  int TVC_COLLAPSE,TVC_EXPAND,TVC_TOGGLE
     * @return
     */
    STDMETHOD_(void, ExpandItem)(THIS_ HSTREEITEM hItem, UINT code) PURE;

    /**
     * @brief Determine whether the item is expanded
     * @param hItem HSTREEITEM--target item
     * @return TRUE--expanded, FALSE--collapsed
     */
    STDMETHOD_(BOOL, IsItemExpanded)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Set the item expand flag
     * @param hItem HSTREEITEM--target item
     * @param bExpanded TRUE--expanded, FALSE--collapsed
     * @return
     */
    STDMETHOD_(void, SetItemExpanded)(THIS_ HSTREEITEM hItem, BOOL bExpanded) PURE;

    /**
     * Get a View that displays the data at the specified position in the data set. You can either
     * create a View manually or inflate it from an XML layout file. When the View is inflated, the
     * parent View (GridView, ListView...) will apply default layout parameters unless you use
     * {@link android.view.LayoutInflater#inflate(int, android.view.ViewGroup, boolean)}
     * to specify a root view and to prevent attachment to the root.
     *
     * @param position The position of the item within the adapter's data set of the item whose view
     *        we want.
     * @param pItem The old view to reuse, if possible. Note: You should check that this view
     *        is non-null and of an appropriate type before using. If it is not possible to convert
     *        this view to display the correct data, this method can create a new view.
     *        Heterogeneous lists can specify their number of view types, so that this View is
     *        always of the right type (see {@link #getViewTypeCount()} and
     *        {@link #getItemViewType(int,DWORD)}).
     * @param xmlTemplate the xml template provided by its owner
     */
    STDMETHOD_(void, getView)
    (THIS_ HSTREEITEM hItem, IWindow * pItem, IXmlNode * pXmlTemplate) PURE;

    /**
     * @brief Get the template ID corresponding to the item
     * @param hItem HSTREEITEM--target item
     * @return int--template ID
     */
    STDMETHOD_(int, getViewType)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Get the number of item templates
     * @return int--number of templates
     */
    STDMETHOD_(int, getViewTypeCount)(CTHIS) SCONST PURE;

    /**
     * @brief Get the item size
     * @param SIZE *ret -- window size
     * @param hItem HSTREEITEM--target item
     * @param pItem IWindow *--window pointer
     * @param wid int--parent window width
     * @param hei int--parent window height
     * @return void
     */
    STDMETHOD_(void, getViewDesiredSize)
    (THIS_ SIZE * ret, HSTREEITEM hItem, IWindow * pItem, int wid, int hei) PURE;

    /**
     * @brief Define that the row width equals the treeview client area width
     * @return TRUE--item width fills the parent window, FALSE--item width is independent of the parent window
     */
    STDMETHOD_(BOOL, isViewWidthMatchParent)(CTHIS) SCONST PURE;

    /**
     * @brief init adapter from the specified template xml data
     * @param pXmlTemplate IXmlNode *--XML template
     * @return void
     */
    STDMETHOD_(void, InitByTemplate)(THIS_ IXmlNode * pXmlTemplate) PURE;

    /**
     * @brief query user defined interface from adapter
     * @param REFGUID id-- __uuidof(interface)
     * @param [out] IObjRef ** ppObj -- the return interface
     * @return HRESULT S_OK--SUCCEED
     */
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef * *ppObj) PURE;
};

SNSEND
#endif /**< __SADAPTER_I__H__ */