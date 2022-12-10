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
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

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
    /**
     * @brief 通知列表项数据变化
     * @param iItem 列表索引
     * @return 无
     */
    STDMETHOD_(void, OnItemChanged)(THIS_ int iItem) PURE;
};

#undef INTERFACE
#define INTERFACE ILvAdapter
DECLARE_INTERFACE_(ILvAdapter, IObjRef)
{
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

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

    /**
     * @brief 计算列表项显示大小
	 * @param [output] SIZE* ret -- 列表项大小
     * @param position int -- 列表项索引
     * @param pItem IWindow -- 列表项窗口
     * @param wid int -- 父窗口宽度
     * @param hei int -- 父窗口高度
     * @return void
     */
    STDMETHOD_(void, getViewDesiredSize)(THIS_ SIZE* ret, int position, IWindow *pItem, int wid, int hei) PURE;

    /**
     * @brief 判断数据是否为空
     * @return true if this adapter doesn't contain any data.  This is used to determine
     * whether the empty view should be displayed.  A typical implementation will return
     * getCount() == 0 but since getCount() includes the headers and footers, specialized
     * adapters might want a different behavior.
     */
    STDMETHOD_(BOOL, isEmpty)(THIS) PURE;

    /**
     * @brief 获取列表项描述
     * @param positoin int--列表项索引
     * @param [out] pDesc IStringW* 字符串描述返回值
     * @return a string object that associate with the specified item.
     */
    STDMETHOD_(void, getItemDesc)(THIS_ int positoin, IStringW *pDesc) PURE;

    /**
     * @brief init adapter from the specified template xml data
     * @param xmlTemplate IXmlNode*--XML模板
     * @return void
     */
    STDMETHOD_(void, InitByTemplate)(THIS_ IXmlNode * xmlTemplate) PURE;

    /**
     * @brief query user defined interface from adapter
     * @param REFGUID id-- __uuidof(interface)
     * @param [out] IObjRef ** ppObj -- the return interface
     * @return HRESULT S_OK--SUCCEED
     */
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef * *ppObj) PURE;
};

#undef INTERFACE
#define INTERFACE IMcAdapter
DECLARE_INTERFACE_(IMcAdapter, ILvAdapter)
{
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

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

    /**
     * @brief 计算列表项显示大小
	 * @param [output] SIZE* ret -- 列表项大小
     * @param position int -- 列表项索引
     * @param pItem IWindow -- 列表项窗口
     * @param wid int -- 父窗口宽度
     * @param hei int -- 父窗口高度
     * @return void
     */
    STDMETHOD_(void, getViewDesiredSize)(THIS_ SIZE* ret, int position, IWindow *pItem, int wid, int hei) PURE;


    /**
     * @return true if this adapter doesn't contain any data.  This is used to determine
     * whether the empty view should be displayed.  A typical implementation will return
     * getCount() == 0 but since getCount() includes the headers and footers, specialized
     * adapters might want a different behavior.
     */
    STDMETHOD_(BOOL, isEmpty)(THIS) PURE;

    /**
     * @return a string object that associate with the specified item.
     */
    STDMETHOD_(void, getItemDesc)(THIS_ int positoin, IStringW *pDesc) PURE;

    /**
     * init adapter from the specified template xml data
     */
    STDMETHOD_(void, InitByTemplate)(THIS_ IXmlNode * xmlTemplate) PURE;

	/**
     * @brief query user defined interface from adapter
     * @param REFGUID id-- __uuidof(interface)
     * @param [out] IObjRef ** ppObj -- the return interface
     * @return HRESULT S_OK--SUCCEED
     */
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef * *ppObj) PURE;

    /**
     * @brief 获取一个列在模板中对应的窗口名称
     * @param iCol int--列索引
     * @param [out] pName IStringW--列名
     * @return void
     */
    STDMETHOD_(void, GetColumnName)(CTHIS_ int iCol, IStringW *pName) SCONST PURE;

    //
    // int iCol: 列序号
    // remark:默认显示

    /**
     * @brief 由Adapter决定一列是否需要显示
     * @param iCol int--列序号
     * @return TRUE--显示, FALSE--隐藏
     * @remark 默认显示
     */
    STDMETHOD_(BOOL, IsColumnVisible)(CTHIS_ int iCol) SCONST PURE;

    /**
     * @brief 排序接口
     * @param iCol int--排序列
     * @param [in, out] pFmts UINT *当前列排序标志
     * @param nCols int 总列数,pFmts数组长度
     * @return TRUE--执行排序，FALSE--没有执行排序
     */
    STDMETHOD_(BOOL, OnSort)(THIS_ int iCol, UINT *pFmts, int nCols) PURE;
};

#undef INTERFACE
#define INTERFACE ITvDataSetObserver
DECLARE_INTERFACE_(ITvDataSetObserver, IObjRef)
{
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

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

    /**
     * @brief This method is called when the specified tree item data becomes invalid,
     * @param hBranch HSTREEITEM--target branch
     * @param bInvalidParents TRUE--target's parent became invalid
     * @param bInvalidChildren TRUE--target's children became invalid
     * @return
     */
    STDMETHOD_(void, onBranchInvalidated)
    (THIS_ HSTREEITEM hBranch, BOOL bInvalidParents, BOOL bInvalidChildren) PURE;

    /**
     * @brief This method is called when the specified tree item expend state changed
     * @param hBranch  HSTREEITEM--target branch
     * @param bExpandedOld BOOL old expand state
     * @param bExpandedNew BOOL new expand state
     * @return
     */
    STDMETHOD_(void, onBranchExpandedChanged)
    (THIS_ HSTREEITEM hBranch, BOOL bExpandedOld, BOOL bExpandedNew) PURE;


	/**
     * @brief This method is called when the specified tree item is going to be removed
     * @param hItem  HSTREEITEM--target item
     * @return
     */
	STDMETHOD_(void, notifyItemBeforeRemove)
		(THIS_ HSTREEITEM hItem) PURE;

};

#define ITEM_NULL 0
#define ITEM_ROOT 0xFFFF0000

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
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

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

    /**
     * @brief 获取hItem中的指定索引的数据
     * @param hItem HSTREEITEM--表项索引
     * @param idx DATA_INDEX--数据类型
     * @return
     */
    STDMETHOD_(ULONG_PTR, GetItemDataByIndex)(CTHIS_ HSTREEITEM hItem, DATA_INDEX idx) SCONST PURE;

    /**
     * @brief 保存hItem指定索引的数据
     * @param hItem HSTREEITEM--表项索引
     * @param idx DATA_INDEX--数据类型
     * @param data ULONG_PTR--数据
     * @return
     */
    STDMETHOD_(void, SetItemDataByIndex)
    (THIS_ HSTREEITEM hItem, DATA_INDEX idx, ULONG_PTR data) PURE;

    /**
     * @brief 获取父级表项
     * @param hItem HSTREEITEM--表项索引
     * @return HSTREEITEM 父级表项
     */
    STDMETHOD_(HSTREEITEM, GetParentItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 获取第一个子项
     * @param hItem HSTREEITEM--表项索引
     * @return HSTREEITEM--第一个子项
     */
    STDMETHOD_(HSTREEITEM, GetFirstChildItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 获取最后一个子项
     * @param hItem HSTREEITEM--表项索引
     * @return HSTREEITEM--最后一个子项
     */
    STDMETHOD_(HSTREEITEM, GetLastChildItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 获取前一个兄弟项
     * @param hItem HSTREEITEM--表项索引
     * @return HSTREEITEM--前一个兄弟项
     */
    STDMETHOD_(HSTREEITEM, GetPrevSiblingItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 获取后一个兄弟项
     * @param hItem HSTREEITEM--表项索引
     * @return HSTREEITEM--后一个兄弟项
     */
    STDMETHOD_(HSTREEITEM, GetNextSiblingItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 是否包含子项
     * @param hItem HSTREEITEM--表项索引
     * @return TRUE--有子项，FALSE--无子项
     */
    STDMETHOD_(BOOL, HasChildren)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 表项是否当前可见
     * @param hItem HSTREEITEM--表项索引
     * @return TRUE--可见，FALSE--不可见，被折叠
     */
    STDMETHOD_(BOOL, IsItemVisible)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 获取第一个可见项
     * @return HSTREEITEM--第一个可见项
     */
    STDMETHOD_(HSTREEITEM, GetFirstVisibleItem)(CTHIS) SCONST PURE;

    /**
     * @brief 获取最后一个可见项
     * @return HSTREEITEM--最后一个可见项
     */
    STDMETHOD_(HSTREEITEM, GetLastVisibleItem)(CTHIS) SCONST PURE;

	/**
     * @brief 测试一个节点是否为另一个节点的子孙节点
     * @param HSTREEITEM--hItem 父节点
	 * @param HSTREEITEM--hChild 被测试的子节点
	 * @return TRUE--是子孙节点
     */
	STDMETHOD_(BOOL,IsDecendentItem)(CTHIS_ HSTREEITEM hItem,HSTREEITEM hChild) SCONST PURE;

    /**
     * @brief 获取前一个可见项
     * @return HSTREEITEM--前一个可见项
     */
    STDMETHOD_(HSTREEITEM, GetPrevVisibleItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 获取后一个可见项
     * @return HSTREEITEM--后一个可见项
     */
    STDMETHOD_(HSTREEITEM, GetNextVisibleItem)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 展开/折叠子项
     * @param hItem HSTREEITEM--目标项
     * @param code  int TVC_COLLAPSE,TVC_EXPAND,TVC_TOGGLE
     * @return
     */
    STDMETHOD_(void, ExpandItem)(THIS_ HSTREEITEM hItem, UINT code) PURE;

    /**
     * @brief 判断表项是否展开
     * @param hItem HSTREEITEM--目标项
     * @return TRUE--展开，FALSE--折叠
     */
    STDMETHOD_(BOOL, IsItemExpanded)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 设置表项展开标志
     * @param hItem HSTREEITEM--目标项
     * @param bExpanded TRUE--展开，FALSE--折叠
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
     * @brief 获取表项对应的模板ID
     * @param hItem HSTREEITEM--目标项
     * @return int--模板ID
     */
    STDMETHOD_(int, getViewType)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 获取表项模板数量
     * @return int--模板数量
     */
    STDMETHOD_(int, getViewTypeCount)(CTHIS) SCONST PURE;

    /**
     * @brief 获取表项大小
	 * @param SIZE *ret -- 窗口大小
     * @param hItem HSTREEITEM--目标项
     * @param pItem IWindow *--窗口指针
     * @param wid int--父窗口宽度
     * @param hei int--你窗口高度
     * @return void
     */
    STDMETHOD_(void, getViewDesiredSize)
    (THIS_ SIZE *ret, HSTREEITEM hItem, IWindow * pItem, int wid, int hei) PURE;

    /**
     * @brief 定义行宽度和treeview客户区宽度相同
     * @return TRUE--表项宽度占满父窗口，FALSE--表项宽度和父窗口无关
     */
    STDMETHOD_(BOOL, isViewWidthMatchParent)(CTHIS) SCONST PURE;

    /**
     * @brief init adapter from the specified template xml data
     * @param pXmlTemplate IXmlNode *--XML模板
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