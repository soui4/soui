#pragma once

#include <interface/obj-ref-i.h>
#include <interface/SWindow-i.h>
#include <interface/sstring-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ILvDataSetObserver
DECLARE_INTERFACE_(ILvDataSetObserver, IObjRef)
{
	//!添加引用
	/*!
	*/
	STDMETHOD_(long,AddRef) (THIS) PURE;

	//!释放引用
	/*!
	*/
	STDMETHOD_(long,Release) (THIS) PURE;

	//!释放对象
	/*!
	*/
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;

    /**
     * This method is called when the entire data set has changed,
     * most likely through a call to {@link Cursor#requery()} on a {@link Cursor}.
     */
    STDMETHOD_(void, onChanged)(THIS) PURE;

    /**
     * This method is called when the entire data becomes invalid,
     * most likely through a call to {@link Cursor#deactivate()} or {@link Cursor#close()} on a
     * {@link Cursor}.
     */
    STDMETHOD_(void, onInvalidated)(THIS) PURE;

    STDMETHOD_(void, OnItemChanged)(THIS_ int iItem) PURE;
};

#undef INTERFACE
#define INTERFACE ILvAdapter
DECLARE_INTERFACE_(ILvAdapter, IObjRef)
{
	//!添加引用
	/*!
	*/
	STDMETHOD_(long,AddRef) (THIS) PURE;

	//!释放引用
	/*!
	*/
	STDMETHOD_(long,Release) (THIS) PURE;

	//!释放对象
	/*!
	*/
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;

    /**
     * Register an observer that is called when changes happen to the data used by this adapter.
     *
     * @param observer the object that gets notified when the data set changes.
     */
    STDMETHOD_(void, registerDataSetObserver)(THIS_ ILvDataSetObserver * observer) PURE;

    /**
     * Unregister an observer that has previously been registered with this
     * adapter via {@link #registerDataSetObserver}.
     *
     * @param observer the object to unregister.
     */
    STDMETHOD_(void, unregisterDataSetObserver)(THIS_ ILvDataSetObserver * observer) PURE;

    /**
     * How many items are in the data set represented by this Adapter.
     *
     * @return Count of items.
     */
    STDMETHOD_(int, getCount)(THIS) PURE;

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
    STDMETHOD_(void, getView)(THIS_ int position, IWindow *pItem, IXmlNode *xmlTemplate) PURE;

    /**
     * Get the type of View that will be created by {@link #getView} for the specified item.
     *
     * @param position The position of the item within the adapter's data set whose view type we
     *        want.
     * @param dwState the state of view.
     * @return An integer representing the type of View. Two views should share the same type if one
     *         can be converted to the other in {@link #getView}. Note: Integers must be in the
     *         range 0 to {@link #getViewTypeCount} - 1. {@link #IGNORE_ITEM_VIEW_TYPE} can
     *         also be returned.
     * @see #IGNORE_ITEM_VIEW_TYPE
     */
    STDMETHOD_(int, getItemViewType)(THIS_ int position, DWORD dwState) PURE;

    /**
     * <p>
     * Returns the number of types of Views that will be created by
     * {@link #getView}. Each type represents a set of views that can be
     * converted in {@link #getView}. If the adapter always returns the same
     * type of View for all items, this method should return 1.
     * </p>
     * <p>
     * This method will only be called when when the adapter is set on the
     * the {@link AdapterView}.
     * </p>
     *
     * @return The number of types of Views that will be created by this adapter
     */
    STDMETHOD_(int, getViewTypeCount)(THIS) PURE;

    STDMETHOD_(SIZE, getViewDesiredSize)(THIS_ int position, IWindow *pItem, int wid, int hei) PURE;

    /**
     * @return true if this adapter doesn't contain any data.  This is used to determine
     * whether the empty view should be displayed.  A typical implementation will return
     * getCount() == 0 but since getCount() includes the headers and footers, specialized
     * adapters might want a different behavior.
     */
    STDMETHOD_(BOOL, isEmpty)(THIS) PURE;

    /**
     * @return a custom defined data that associate with the specified item.
     */
    STDMETHOD_(ULONG_PTR, getItemData)(THIS_ int position) PURE;

    /**
     * @return a string object that associate with the specified item.
     */
    STDMETHOD_(void, getItemDesc)(THIS_ int positoin, IStringW *pDesc) PURE;

    /**
     * init adapter from the specified template xml data
     */
    STDMETHOD_(void, InitByTemplate)(IXmlNode * xmlTemplate) PURE;
};

#undef INTERFACE
#define INTERFACE IMcAdapter
DECLARE_INTERFACE_(IMcAdapter, ILvAdapter)
{
		//!添加引用
	/*!
	*/
	STDMETHOD_(long,AddRef) (THIS) PURE;

	//!释放引用
	/*!
	*/
	STDMETHOD_(long,Release) (THIS) PURE;

	//!释放对象
	/*!
	*/
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;

    /**
     * Register an observer that is called when changes happen to the data used by this adapter.
     *
     * @param observer the object that gets notified when the data set changes.
     */
    STDMETHOD_(void, registerDataSetObserver)(THIS_ ILvDataSetObserver * observer) PURE;

    /**
     * Unregister an observer that has previously been registered with this
     * adapter via {@link #registerDataSetObserver}.
     *
     * @param observer the object to unregister.
     */
    STDMETHOD_(void, unregisterDataSetObserver)(THIS_ ILvDataSetObserver * observer) PURE;

    /**
     * How many items are in the data set represented by this Adapter.
     *
     * @return Count of items.
     */
    STDMETHOD_(int, getCount)(THIS) PURE;

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
    STDMETHOD_(void, getView)(THIS_ int position, IWindow *pItem, IXmlNode *xmlTemplate) PURE;

    /**
     * Get the type of View that will be created by {@link #getView} for the specified item.
     *
     * @param position The position of the item within the adapter's data set whose view type we
     *        want.
     * @param dwState the state of view.
     * @return An integer representing the type of View. Two views should share the same type if one
     *         can be converted to the other in {@link #getView}. Note: Integers must be in the
     *         range 0 to {@link #getViewTypeCount} - 1. {@link #IGNORE_ITEM_VIEW_TYPE} can
     *         also be returned.
     * @see #IGNORE_ITEM_VIEW_TYPE
     */
    STDMETHOD_(int, getItemViewType)(THIS_ int position, DWORD dwState) PURE;

    /**
     * <p>
     * Returns the number of types of Views that will be created by
     * {@link #getView}. Each type represents a set of views that can be
     * converted in {@link #getView}. If the adapter always returns the same
     * type of View for all items, this method should return 1.
     * </p>
     * <p>
     * This method will only be called when when the adapter is set on the
     * the {@link AdapterView}.
     * </p>
     *
     * @return The number of types of Views that will be created by this adapter
     */
    STDMETHOD_(int, getViewTypeCount)(THIS) PURE;

    STDMETHOD_(SIZE, getViewDesiredSize)(THIS_ int position, IWindow *pItem, int wid, int hei) PURE;

    /**
     * @return true if this adapter doesn't contain any data.  This is used to determine
     * whether the empty view should be displayed.  A typical implementation will return
     * getCount() == 0 but since getCount() includes the headers and footers, specialized
     * adapters might want a different behavior.
     */
    STDMETHOD_(BOOL, isEmpty)(THIS) PURE;

    /**
     * @return a custom defined data that associate with the specified item.
     */
    STDMETHOD_(ULONG_PTR, getItemData)(THIS_ int position) PURE;

    /**
     * @return a string object that associate with the specified item.
     */
    STDMETHOD_(void, getItemDesc)(THIS_ int positoin, IStringW *pDesc) PURE;

    /**
     * init adapter from the specified template xml data
     */
    STDMETHOD_(void, InitByTemplate)(IXmlNode * xmlTemplate) PURE;

    //获取一个列在模板中对应的窗口名称
    // int iCol: 列序号
    STDMETHOD_(void, GetColumnName)(THIS_ int iCol, IStringW *pName) SCONST PURE;

    //由Adapter决定一列是否需要显示
    // int iCol: 列序号
    // remark:默认显示
    STDMETHOD_(BOOL, IsColumnVisible)(THIS_ int iCol) SCONST PURE;

    //排序接口
    // int iCol:排序列
    // UINT *pFmts [in, out]:当前列排序标志
    // int nCols:总列数,stFlags数组长度
    STDMETHOD_(BOOL, OnSort)(THIS_ int iCol, UINT *pFmts, int nCols) PURE;
};


#undef INTERFACE
#define INTERFACE ITvDataSetObserver
DECLARE_INTERFACE_(ITvDataSetObserver, IObjRef)
{
		//!添加引用
	/*!
	*/
	STDMETHOD_(long,AddRef) (THIS) PURE;

	//!释放引用
	/*!
	*/
	STDMETHOD_(long,Release) (THIS) PURE;

	//!释放对象
	/*!
	*/
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;

    /**
     * This method is called when the entire data set has changed,
     * most likely through a call to {@link Cursor#requery()} on a {@link Cursor}.
     */
    STDMETHOD_(void, onBranchChanged)(THIS_ HSTREEITEM hBranch) PURE;

    /**
     * This method is called when the entire data becomes invalid,
     * most likely through a call to {@link Cursor#deactivate()} or {@link Cursor#close()} on a
     * {@link Cursor}.
     */
    STDMETHOD_(void, onBranchInvalidated)
    (THIS_ HSTREEITEM hBranch, BOOL bInvalidParents, BOOL bInvalidChildren) PURE;

    STDMETHOD_(void, onBranchExpandedChanged)
    (THIS_ HSTREEITEM hBranch, BOOL bExpandedOld, BOOL bExpandedNew) PURE;
};

enum
{
    ITEM_NULL = 0,
    ITEM_ROOT = 0xFFFF0000,
};

typedef enum DATA_INDEX
{
    DATA_INDEX_ITEM_HEIGHT = 0, //行高
    DATA_INDEX_ITEM_WIDTH,      //行宽度
    DATA_INDEX_BRANCH_HEIGHT,   //分枝高度
    DATA_INDEX_BRANCH_WIDTH,    //分枝宽度,不包含indent
    DATA_INDEX_ITEM_EXPANDED,   //子项展开状态
    DATA_INDEX_ITEM_OFFSET,     //当前节点在父节点中的Y方向偏移
    DATA_INDEX_ITEM_USER,       //自定义数据
    DATA_INDEX_NUMBER
}DATA_INDEX;

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
	//!添加引用
	/*!
	*/
	STDMETHOD_(long,AddRef) (THIS) PURE;

	//!释放引用
	/*!
	*/
	STDMETHOD_(long,Release) (THIS) PURE;

	//!释放对象
	/*!
	*/
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;

    /**
     * Register an observer that is called when changes happen to the data used by this adapter.
     *
     * @param observer the object that gets notified when the data set changes.
     */
    STDMETHOD_(void, registerDataSetObserver)(THIS_ ITvDataSetObserver * observer) PURE;

    /**
     * Unregister an observer that has previously been registered with this
     * adapter via {@link #registerDataSetObserver}.
     *
     * @param observer the object to unregister.
     */
    STDMETHOD_(void, unregisterDataSetObserver)(THIS_ ITvDataSetObserver * observer) PURE;

    //获取hItem中的指定索引的数据
    STDMETHOD_(ULONG_PTR, GetItemDataByIndex)(THIS_ HSTREEITEM hItem, DATA_INDEX idx) SCONST PURE;

    //保存hItem指定索引的数据
    STDMETHOD_(void, SetItemDataByIndex)
    (THIS_ HSTREEITEM hItem, DATA_INDEX idx, ULONG_PTR data) PURE;

    STDMETHOD_(HSTREEITEM, GetParentItem)(THIS_ HSTREEITEM hItem) SCONST PURE;

    STDMETHOD_(HSTREEITEM, GetFirstChildItem)(THIS_ HSTREEITEM hItem) SCONST PURE;
    STDMETHOD_(HSTREEITEM, GetLastChildItem)(THIS_ HSTREEITEM hItem) SCONST PURE;
    STDMETHOD_(HSTREEITEM, GetPrevSiblingItem)(THIS_ HSTREEITEM hItem) SCONST PURE;
    STDMETHOD_(HSTREEITEM, GetNextSiblingItem)(THIS_ HSTREEITEM hItem) SCONST PURE;
    STDMETHOD_(BOOL, HasChildren)(THIS_ HSTREEITEM hItem) SCONST PURE;

    STDMETHOD_(BOOL, IsItemVisible)(THIS_ HSTREEITEM hItem) SCONST PURE;

    STDMETHOD_(HSTREEITEM, GetFirstVisibleItem)(THIS) SCONST PURE;
    STDMETHOD_(HSTREEITEM, GetLastVisibleItem)(THIS) SCONST PURE;
    STDMETHOD_(HSTREEITEM, GetPrevVisibleItem)(THIS_ HSTREEITEM hItem) SCONST PURE;
    STDMETHOD_(HSTREEITEM, GetNextVisibleItem)(THIS_ HSTREEITEM hItem) SCONST PURE;

    STDMETHOD_(void, ExpandItem)(THIS_ HSTREEITEM hItem, UINT code) PURE;

    STDMETHOD_(BOOL, IsItemExpanded)(THIS_ HSTREEITEM hItem) SCONST PURE;

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
    STDMETHOD_(void, getView)(THIS_ HSTREEITEM hItem, IWindow * pItem, IXmlNode * pXmlTemplate) PURE;

    STDMETHOD_(int, getViewType)(THIS_ HSTREEITEM hItem) SCONST PURE;

    STDMETHOD_(int, getViewTypeCount)(THIS) SCONST PURE;

    STDMETHOD_(SIZE, getViewDesiredSize)
    (THIS_ HSTREEITEM hItem, IWindow * pItem, int wid, int hei) PURE;

    //定义行宽度和treeview客户区宽度相同
    STDMETHOD_(BOOL, isViewWidthMatchParent)(THIS) SCONST PURE;

    /**
     * init adapter from the specified template xml data
     */
    STDMETHOD_(void, InitByTemplate)(IXmlNode * pXmlTemplate) PURE;
};

SNSEND