#ifndef __SADAPTERBASE__H__
#define __SADAPTERBASE__H__

#include <interface/SAdapter-i.h>
#include <souicoll.h>
#include <control/STree.h>

SNSBEGIN

/**
 * @class SLvObserverMgr
 * @brief Class managing observers of the list data set
 */
class SLvObserverMgr {
  public:
    /**
     * @brief Constructor
     */
    SLvObserverMgr()
    {
    }

    /**
     * @brief Destructor; releases all observers
     */
    ~SLvObserverMgr()
    {
        SPOSITION pos = m_lstObserver.GetHeadPosition();
        while (pos)
        {
            ILvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
            pObserver->Release();
        }
        m_lstObserver.RemoveAll();
    }

    /**
     * @brief Register an observer
     * @param observer observer object pointer
     */
    void registerObserver(ILvDataSetObserver *observer)
    {
        SASSERT(observer);
        if (m_lstObserver.Find(observer))
            return;
        m_lstObserver.AddTail(observer);
        observer->AddRef();
    }

    /**
     * @brief Unregister an observer
     * @param observer observer object pointer
     */
    void unregisterObserver(ILvDataSetObserver *observer)
    {
        SASSERT(observer);
        SPOSITION pos = m_lstObserver.Find(observer);
        if (!pos)
            return;
        m_lstObserver.RemoveAt(pos);
        observer->Release();
    }

    /**
     * @brief Notify all observers that the data set has changed
     */
    void notifyChanged()
    {
        SPOSITION pos = m_lstObserver.GetHeadPosition();
        while (pos)
        {
            ILvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
            pObserver->onChanged();
        }
    }

    /**
     * @brief Notify all observers that the data set has been invalidated
     */
    void notifyInvalidated()
    {
        SPOSITION pos = m_lstObserver.GetHeadPosition();
        while (pos)
        {
            ILvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
            pObserver->onInvalidated();
        }
    }

    /**
     * @brief Notify all observers that the data of the specified item has changed
     * @param iItem item index
     */
    void notifyItemChanged(int iItem)
    {
        SPOSITION pos = m_lstObserver.GetHeadPosition();
        while (pos)
        {
            ILvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
            pObserver->OnItemChanged(iItem);
        }
    }

  protected:
    SList<ILvDataSetObserver *> m_lstObserver; /**<  Observer list */
};

/**
 * @class LvAdatperImpl
 * @brief List adapter implementation template class
 * @tparam BaseClass base class type
 */
template <class BaseClass>
class LvAdatperImpl : public BaseClass {
  public:
    /**
     * @brief Notify all observers that the data set has changed
     */
    void notifyDataSetChanged()
    {
        m_obzMgr.notifyChanged();
    }

    /**
     * @brief Notify all observers that the data set has been invalidated
     */
    void notifyDataSetInvalidated()
    {
        m_obzMgr.notifyInvalidated();
    }

    /**
     * @brief Notify all observers that the data of the specified item has changed
     * @param iItem item index
     */
    void notifyItemDataChanged(int iItem)
    {
        m_obzMgr.notifyItemChanged(iItem);
    }

    /**
     * @brief Register a data set observer
     * @param observer observer object pointer
     */
    STDMETHOD_(void, registerDataSetObserver)(ILvDataSetObserver *observer) OVERRIDE
    {
        m_obzMgr.registerObserver(observer);
    }

    /**
     * @brief Unregister a data set observer
     * @param observer observer object pointer
     */
    STDMETHOD_(void, unregisterDataSetObserver)(ILvDataSetObserver *observer) OVERRIDE
    {
        m_obzMgr.unregisterObserver(observer);
    }

    /**
     * @brief Get the view
     * @param position position index
     * @param pItem item window object
     * @param pXmlTemplate XML template object
     */
    STDMETHOD_(void, getView)(int position, IWindow *pItem, IXmlNode *pXmlTemplate) OVERRIDE
    {
        SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pItem);
        SXmlNode xmlTemplate(pXmlTemplate);
        getView(position, pItemPanel, xmlTemplate);
    }

    /**
     * @brief Get the view (template function)
     * @param position position index
     * @param pItem item panel object
     * @param xmlTemplate XML template object
     */
    STDMETHOD_(void, getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate)
    {
        SASSERT(FALSE);
    }

    /**
     * @brief Get the item view type
     * @param position position index
     * @return view type
     */
    STDMETHOD_(int, getItemViewType)(int position)
    {
        return 0;
    }

    /**
     * @brief Get the item view type (overload)
     * @param position position index
     * @param dwState state
     * @return view type
     */
    STDMETHOD_(int, getItemViewType)(THIS_ int position, DWORD dwState) OVERRIDE
    {
        return getItemViewType(position);
    }

    /**
     * @brief Get the number of view types
     * @return number of view types
     */
    STDMETHOD_(int, getViewTypeCount)() OVERRIDE
    {
        return 1;
    }

    /**
     * @brief Check whether the data set is empty
     * @return Returns TRUE if the data set is empty, FALSE otherwise
     */
    STDMETHOD_(BOOL, isEmpty)(THIS) OVERRIDE
    {
        return getCount() <= 0;
    }

    /**
     * @brief Get the number of items in the data set
     * @return number of items in the data set
     */
    STDMETHOD_(int, getCount)(THIS) PURE;

    /**
     * @brief Get the item description
     * @param position position index
     * @return item description string
     */
    STDMETHOD_(SStringW, getItemDesc)(int position)
    {
        return L"";
    }

    /**
     * @brief Get the item description (overload)
     * @param position position index
     * @param pDesc description string object
     */
    STDMETHOD_(void, getItemDesc)(int position, IStringW *pDesc) OVERRIDE
    {
        SStringW strDesc = getItemDesc(position);
        pDesc->Copy(&strDesc);
    }

    /**
     * @brief Initialize the adapter via template
     * @param xmlTemplate XML template object
     */
    STDMETHOD_(void, InitByTemplate)(SXmlNode xmlTemplate)
    {
    }

    /**
     * @brief Initialize the adapter via template (overload)
     * @param pXmlTemplate XML template object pointer
     */
    STDMETHOD_(void, InitByTemplate)(IXmlNode *pXmlTemplate) OVERRIDE
    {
        SXmlNode xmlTempalte(pXmlTemplate);
        InitByTemplate(xmlTempalte);
    }

    /**
     * @brief Get the required size of the view
     * @param ret returned size object
     * @param position position index
     * @param pItem item panel object
     * @param wid width
     * @param hei height
     */
    STDMETHOD_(void, getViewDesiredSize)(SIZE *ret, int position, SItemPanel *pItem, int wid, int hei)
    {
        pItem->GetDesiredSize(ret, wid, hei);
    }

    /**
     * @brief Get the required size of the view (overload)
     * @param ret returned size object
     * @param position position index
     * @param pItem item window object
     * @param wid width
     * @param hei height
     */
    STDMETHOD_(void, getViewDesiredSize)(SIZE *ret, int position, IWindow *pItem, int wid, int hei) OVERRIDE
    {
        SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pItem);
        getViewDesiredSize(ret, position, pItemPanel, wid, hei);
    }

    /**
     * @brief Query interface
     * @param id interface ID
     * @param ppObj interface object pointer
     * @return query result
     */
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef **ppObj) OVERRIDE
    {
        return E_NOINTERFACE;
    }

  protected:
    SLvObserverMgr m_obzMgr; /**<  Observer manager */
};

/**
 * @class SAdapterBase
 * @brief Base class for list adapters
 */
class SAdapterBase : public TObjRefImpl<LvAdatperImpl<ILvAdapter>> {
  public:
    /**
     * @brief Constructor
     */
    SAdapterBase()
    {
    }

    /**
     * @brief Destructor
     */
    ~SAdapterBase()
    {
    }
};

/**
 * @class SMcAdapterBase
 * @brief Base class for multi-column adapters
 */
class SMcAdapterBase : public TObjRefImpl<LvAdatperImpl<IMcAdapter>> {
  public:
    /**
     * @brief Constructor
     */
    SMcAdapterBase()
    {
    }

    /**
     * @brief Destructor
     */
    ~SMcAdapterBase()
    {
    }

    /**
     * @brief Sort items
     * @param iCol column index
     * @param pFmts format array
     * @param nCols number of columns
     * @return sort result
     */
    STDMETHOD_(BOOL, OnSort)(int iCol, UINT *pFmts, int nCols) OVERRIDE
    {
        return FALSE;
    }

    /**
     * @brief Get the column name
     * @param iCol column index
     * @return column name string
     */
    STDMETHOD_(SStringW, GetColumnName)(int iCol) SCONST
    {
        return L"";
    }

    /**
     * @brief Get the column name (overload)
     * @param iCol column index
     * @param pName column name string object
     */
    STDMETHOD_(void, GetColumnName)(THIS_ int iCol, IStringW *pName) SCONST
    {
        SStringW strName = GetColumnName(iCol);
        pName->Copy(&strName);
    }

    /**
     * @brief Check whether the column is visible
     * @param iCol column index
     * @return Returns TRUE if the column is visible, FALSE otherwise
     */
    STDMETHOD_(BOOL, IsColumnVisible)(THIS_ int iCol) SCONST OVERRIDE
    {
        return TRUE;
    }

    /**
     * @brief Initialize the adapter via template
     * @param xmlTemplate XML template object
     */
    STDMETHOD_(void, InitByTemplate)(THIS_ SXmlNode xmlTemplate)
    {
    }

    /**
     * @brief Initialize the adapter via template (overload)
     * @param xmlTemplate XML template object pointer
     */
    STDMETHOD_(void, InitByTemplate)(THIS_ IXmlNode *xmlTemplate) OVERRIDE
    {
        SXmlNode xmlNode(xmlTemplate);
        InitByTemplate(xmlNode);
    }

    /**
     * @brief Set the column width
     * @param pColWid column width array
     * @param nCols number of columns
     */
    STDMETHOD_(void, SetColumnsWidth)(THIS_ int *pColWid, int nCols) OVERRIDE
    {
    }
};

/**
 * @class STvObserverMgr
 * @brief Class managing observers of the tree data set
 */
class STvObserverMgr {
  public:
    /**
     * @brief Constructor
     */
    STvObserverMgr()
    {
    }

    /**
     * @brief Destructor; releases all observers
     */
    ~STvObserverMgr()
    {
        SPOSITION pos = m_lstObserver.GetHeadPosition();
        while (pos)
        {
            ITvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
            pObserver->Release();
        }
        m_lstObserver.RemoveAll();
    }

    /**
     * @brief Register an observer
     * @param observer observer object pointer
     */
    void registerObserver(ITvDataSetObserver *observer)
    {
        SASSERT(observer);
        if (m_lstObserver.Find(observer))
            return;
        m_lstObserver.AddTail(observer);
        observer->AddRef();
    }

    /**
     * @brief Unregister an observer
     * @param observer observer object pointer
     */
    void unregisterObserver(ITvDataSetObserver *observer)
    {
        SASSERT(observer);
        SPOSITION pos = m_lstObserver.Find(observer);
        if (!pos)
            return;
        m_lstObserver.RemoveAt(pos);
        observer->Release();
    }

    /**
     * @brief Notify all observers that a branch has changed
     * @param hBranch branch handle
     */
    void notifyChanged(HSTREEITEM hBranch)
    {
        SPOSITION pos = m_lstObserver.GetHeadPosition();
        while (pos)
        {
            ITvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
            pObserver->onBranchChanged(hBranch);
        }
    }

    /**
     * @brief Notify all observers that a branch has been invalidated
     * @param hBranch branch handle
     * @param bInvalidParents whether parent nodes are invalidated
     * @param bInvalidChildren whether child nodes are invalidated
     */
    void notifyInvalidated(HSTREEITEM hBranch, bool bInvalidParents, bool bInvalidChildren)
    {
        SPOSITION pos = m_lstObserver.GetHeadPosition();
        while (pos)
        {
            ITvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
            pObserver->onBranchInvalidated(hBranch, bInvalidParents, bInvalidChildren);
        }
    }

    /**
     * @brief Notify all observers that the branch expanded state has changed
     * @param hBranch branch handle
     * @param bExpandedOld previous expanded state
     * @param bExpandedNew new expanded state
     */
    void notifyExpandChanged(HSTREEITEM hBranch, BOOL bExpandedOld, BOOL bExpandedNew)
    {
        SPOSITION pos = m_lstObserver.GetHeadPosition();
        while (pos)
        {
            ITvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
            pObserver->onBranchExpandedChanged(hBranch, bExpandedOld, bExpandedNew);
        }
    }

    /**
     * @brief Notify all observers that an item is about to be removed
     * @param hItem item handle
     */
    void notifyItemBeforeRemove(HSTREEITEM hItem)
    {
        SPOSITION pos = m_lstObserver.GetHeadPosition();
        while (pos)
        {
            ITvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
            pObserver->notifyItemBeforeRemove(hItem);
        }
    }

  protected:
    SList<ITvDataSetObserver *> m_lstObserver; /**<  Observer list */
};

/**
 * @class TvAdatperImpl
 * @brief Tree adapter implementation template class
 * @tparam BaseClass base class type
 */
template <class BaseClass>
class TvAdatperImpl : public BaseClass {
  public:
    /**
     * @brief Notify all observers that a branch has changed
     * @param hBranch branch handle
     */
    void notifyBranchChanged(HSTREEITEM hBranch)
    {
        m_obzMgr.notifyChanged(hBranch);
    }

    /**
     * @brief Notify all observers that a branch has been invalidated
     * @param hBranch branch handle
     * @param bInvalidParents whether parent nodes are invalidated
     * @param bInvalidChildren whether child nodes are invalidated
     */
    void notifyBranchInvalidated(HSTREEITEM hBranch, bool bInvalidParents = true, bool bInvalidChildren = true)
    {
        m_obzMgr.notifyInvalidated(hBranch, bInvalidParents, bInvalidChildren);
    }

    /**
     * @brief Notify all observers that the branch expanded state has changed
     * @param hBranch branch handle
     * @param bExpandedOld previous expanded state
     * @param bExpandedNew new expanded state
     */
    void notifyBranchExpandChanged(HSTREEITEM hBranch, BOOL bExpandedOld, BOOL bExpandedNew)
    {
        m_obzMgr.notifyExpandChanged(hBranch, bExpandedOld, bExpandedNew);
    }

    /**
     * @brief Notify all observers that an item is about to be removed
     * @param hItem item handle
     */
    void notifyItemBeforeRemove(HSTREEITEM hItem)
    {
        m_obzMgr.notifyItemBeforeRemove(hItem);
    }

    /**
     * @brief Register a data set observer
     * @param observer observer object pointer
     */
    STDMETHOD_(void, registerDataSetObserver)(ITvDataSetObserver *observer) OVERRIDE
    {
        m_obzMgr.registerObserver(observer);
    }

    /**
     * @brief Unregister a data set observer
     * @param observer observer object pointer
     */
    STDMETHOD_(void, unregisterDataSetObserver)(ITvDataSetObserver *observer) OVERRIDE
    {
        m_obzMgr.unregisterObserver(observer);
    }

  protected:
    STvObserverMgr m_obzMgr; /**<  Observer manager */
};

/**
 * @class STreeAdapterBase
 * @brief Tree adapter base class template
 * @tparam T data type
 */
template <typename T>
class STreeAdapterBase : public TObjRefImpl<TvAdatperImpl<ITvAdapter>> {
  public:
    /**
     * @brief Data release function type
     */
    typedef void (*FunTvItemDataFreer)(T cb);

    /**
     * @struct ItemInfo
     * @brief TreeNode data structure
     */
    struct ItemInfo
    {
        ULONG_PTR userData[DATA_INDEX_NUMBER]; /**<  User data array */
        T data;                                /**<  Data object */
    };

    /**
     * @class TreeDataFreer
     * @brief Tree node data releaser class
     */
    class TreeDataFreer : public CSTree<ItemInfo>::IDataFreer {
      public:
        /**
         * @brief Constructor
         */
        TreeDataFreer()
            : m_dataFreer(NULL)
        {
        }

        FunTvItemDataFreer m_dataFreer; /**<  Data release function */

        /**
         * @brief Release data
         * @param data data object reference
         */
        void OnDataFree(ItemInfo &data) override
        {
            if (m_dataFreer)
                m_dataFreer(data.data);
        }
    };

  public:
    /**
     * @brief Constructor
     */
    STreeAdapterBase()
    {
        memset(m_rootUserData, 0, sizeof(m_rootUserData));
        m_tree.SetDataFreer(&m_treeFreer);
    }

    /**
     * @brief Destructor
     */
    ~STreeAdapterBase()
    {
        m_tree.DeleteAllItems();
    }

    /**
     * @brief Get the data at the specified index in hItem
     * @param hItem item handle
     * @param idx index
     * @return user data
     */
    STDMETHOD_(ULONG_PTR, GetItemDataByIndex)(HSTREEITEM hItem, DATA_INDEX idx) const OVERRIDE
    {
        if (hItem == ITEM_ROOT)
            return m_rootUserData[idx];
        ItemInfo &ii = m_tree.GetItemRef((HSTREEITEM)hItem);
        return ii.userData[idx];
    }

    /**
     * @brief Save the data at the specified index in hItem
     * @param hItem item handle
     * @param idx index
     * @param data data
     */
    STDMETHOD_(void, SetItemDataByIndex)(HSTREEITEM hItem, DATA_INDEX idx, ULONG_PTR data) OVERRIDE
    {
        if (hItem == ITEM_ROOT)
            m_rootUserData[idx] = data;
        else
        {
            ItemInfo &ii = m_tree.GetItemRef((HSTREEITEM)hItem);
            ii.userData[idx] = data;
        }
    }

    /**
     * @brief Get the parent item
     * @param hItem item handle
     * @return parent item handle
     */
    STDMETHOD_(HSTREEITEM, GetParentItem)(HSTREEITEM hItem) const OVERRIDE
    {
        if (hItem == ITEM_ROOT)
            return ITEM_NULL;
        HSTREEITEM hParent = m_tree.GetParentItem((HSTREEITEM)hItem);
        if (hParent == 0)
            hParent = ITEM_ROOT;
        return (HSTREEITEM)hParent;
    }

    /**
     * @brief Check whether there are child items
     * @param hItem item handle
     * @return Returns TRUE if there are child items, FALSE otherwise
     */
    STDMETHOD_(BOOL, HasChildren)(HSTREEITEM hItem) const OVERRIDE
    {
        return GetFirstChildItem(hItem) != ITEM_NULL;
    }

    /**
     * @brief Get the first child item
     * @param hItem item handle
     * @return first child item handle
     */
    STDMETHOD_(HSTREEITEM, GetFirstChildItem)(HSTREEITEM hItem) const OVERRIDE
    {
        SASSERT(hItem != ITEM_NULL);
        return (HSTREEITEM)m_tree.GetChildItem((HSTREEITEM)hItem, TRUE);
    }

    /**
     * @brief Get the last child item
     * @param hItem item handle
     * @return last child item handle
     */
    STDMETHOD_(HSTREEITEM, GetLastChildItem)(HSTREEITEM hItem) const OVERRIDE
    {
        SASSERT(hItem != ITEM_NULL);
        return (HSTREEITEM)m_tree.GetChildItem((HSTREEITEM)hItem, FALSE);
    }

    /**
     * @brief Get the previous sibling item
     * @param hItem item handle
     * @return previous sibling item handle
     */
    STDMETHOD_(HSTREEITEM, GetPrevSiblingItem)(HSTREEITEM hItem) const OVERRIDE
    {
        SASSERT(hItem != ITEM_NULL && hItem != ITEM_ROOT);
        return (HSTREEITEM)m_tree.GetPrevSiblingItem((HSTREEITEM)hItem);
    }

    /**
     * @brief Get the next sibling item
     * @param hItem item handle
     * @return next sibling item handle
     */
    STDMETHOD_(HSTREEITEM, GetNextSiblingItem)(HSTREEITEM hItem) const OVERRIDE
    {
        SASSERT(hItem != ITEM_NULL && hItem != ITEM_ROOT);
        return (HSTREEITEM)m_tree.GetNextSiblingItem((HSTREEITEM)hItem);
    }

    /**
     * @brief Check whether it is a descendant item
     * @param hItem parent item handle
     * @param hChild child item handle
     * @return Returns TRUE if it is a descendant item, FALSE otherwise
     */
    STDMETHOD_(BOOL, IsDecendentItem)(CTHIS_ HSTREEITEM hItem, HSTREEITEM hChild) const OVERRIDE
    {
        HSTREEITEM hParent = GetParentItem(hChild);
        while (hParent)
        {
            if (hParent == hItem)
                return TRUE;
            hParent = GetParentItem(hParent);
        }
        return FALSE;
    }

    /**
     * @brief Get the item view type
     * @param hItem item handle
     * @return view type
     */
    STDMETHOD_(int, getViewType)(HSTREEITEM hItem) const OVERRIDE
    {
        return 0;
    }

    /**
     * @brief Get the number of view types
     * @return number of view types
     */
    STDMETHOD_(int, getViewTypeCount)() const OVERRIDE
    {
        return 1;
    }

    /**
     * @brief Get the view
     * @param hItem item handle
     * @param pItem item panel object
     * @param xmlTemplate XML template object
     */
    STDMETHOD_(void, getView)(THIS_ HSTREEITEM hItem, SItemPanel *pItem, SXmlNode xmlTemplate)
    {
    }

    /**
     * @brief Get the view (overload)
     * @param hItem item handle
     * @param pItem item window object
     * @param pXmlTemplate XML template object pointer
     */
    STDMETHOD_(void, getView)(THIS_ HSTREEITEM hItem, IWindow *pItem, IXmlNode *pXmlTemplate) OVERRIDE
    {
        SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pItem);
        SXmlNode xmlTemplate(pXmlTemplate);
        return getView(hItem, pItemPanel, xmlTemplate);
    }

    /**
     * @brief Get the required size of the view
     * @param ret returned size object
     * @param hItem item handle
     * @param pItem item panel object
     * @param wid width
     * @param hei height
     */
    STDMETHOD_(void, getViewDesiredSize)(SIZE *ret, HSTREEITEM hItem, SItemPanel *pItem, int wid, int hei)
    {
        pItem->GetDesiredSize(ret, wid, hei);
    }

    /**
     * @brief Get the required size of the view (overload)
     * @param ret returned size object
     * @param hItem item handle
     * @param pItem item window object
     * @param wid width
     * @param hei height
     */
    STDMETHOD_(void, getViewDesiredSize)(SIZE *ret, HSTREEITEM hItem, IWindow *pItem, int wid, int hei) OVERRIDE
    {
        SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pItem);
        getViewDesiredSize(ret, hItem, pItemPanel, wid, hei);
    }

    /**
     * @brief Initialize the adapter via template
     * @param xmlTemplate XML template object
     */
    STDMETHOD_(void, InitByTemplate)(SXmlNode xmlTemplate)
    {
        (xmlTemplate);
    }

    /**
     * @brief Initialize the adapter via template (overload)
     * @param pXmlTemplate XML template object pointer
     */
    STDMETHOD_(void, InitByTemplate)(IXmlNode *pXmlTemplate) OVERRIDE
    {
        SXmlNode xmlTemplate(pXmlTemplate);
        return InitByTemplate(xmlTemplate);
    }

    /**
     * @brief Check whether the view width matches the parent container
     * @return Returns TRUE if it matches, FALSE otherwise
     */
    STDMETHOD_(BOOL, isViewWidthMatchParent)() const OVERRIDE
    {
        return FALSE;
    }

    /**
     * @brief Expand or collapse an item
     * @param hItem item handle
     * @param uCode expand/collapse code
     */
    STDMETHOD_(void, ExpandItem)(HSTREEITEM hItem, UINT uCode) OVERRIDE
    {
        BOOL bExpandedOld = IsItemExpanded(hItem);
        BOOL bExpandedNew = bExpandedOld;
        switch (uCode)
        {
        case TVC_COLLAPSE:
            bExpandedNew = FALSE;
            break;
        case TVC_EXPAND:
            bExpandedNew = TRUE;
            break;
        case TVC_TOGGLE:
            bExpandedNew = !bExpandedOld;
            break;
        }
        if (bExpandedOld == bExpandedNew)
            return;

        SetItemExpanded(hItem, bExpandedNew);
        notifyBranchExpandChanged(hItem, bExpandedOld, bExpandedNew);
    }

    /**
     * @brief Check whether the item is expanded
     * @param hItem item handle
     * @return Returns TRUE if expanded, FALSE otherwise
     */
    STDMETHOD_(BOOL, IsItemExpanded)(HSTREEITEM hItem) const OVERRIDE
    {
        if (hItem == ITEM_ROOT)
            return TRUE; /**< Virtual root node is auto-expanded */
        return (BOOL)GetItemDataByIndex(hItem, DATA_INDEX_ITEM_EXPANDED);
    }

    /**
     * @brief Set the item expanded state
     * @param hItem item handle
     * @param bExpanded expanded state
     */
    STDMETHOD_(void, SetItemExpanded)(HSTREEITEM hItem, BOOL bExpanded) OVERRIDE
    {
        SetItemDataByIndex(hItem, DATA_INDEX_ITEM_EXPANDED, bExpanded);
    }

    /**
     * @brief Check whether the item is visible
     * @param hItem item handle
     * @return Returns TRUE if visible, FALSE otherwise
     */
    STDMETHOD_(BOOL, IsItemVisible)(HSTREEITEM hItem) const OVERRIDE
    {
        HSTREEITEM hParent = GetParentItem(hItem);
        while (hParent != ITEM_NULL)
        {
            if (!IsItemExpanded(hParent))
                return FALSE;
            hParent = GetParentItem(hParent);
        }
        return TRUE;
    }

    /**
     * @brief Get the first visible item
     * @return first visible item handle
     */
    STDMETHOD_(HSTREEITEM, GetFirstVisibleItem)() const OVERRIDE
    {
        return GetFirstChildItem(ITEM_ROOT);
    }

    /**
     * @brief Get the last visible item
     * @return last visible item handle
     */
    STDMETHOD_(HSTREEITEM, GetLastVisibleItem)() const OVERRIDE
    {
        HSTREEITEM hItem = GetLastChildItem(ITEM_ROOT);
        if (hItem == ITEM_NULL)
            return hItem;
        for (; IsItemExpanded(hItem);)
        {
            HSTREEITEM hChild = GetLastChildItem(hItem);
            if (hChild == ITEM_NULL)
                break;
            hItem = hChild;
        }
        return hItem;
    }

    /**
     * @brief Get the previous visible item
     * @param hItem item handle
     * @return previous visible item handle
     */
    STDMETHOD_(HSTREEITEM, GetPrevVisibleItem)(HSTREEITEM hItem) const OVERRIDE
    {
        SASSERT(IsItemVisible(hItem));
        HSTREEITEM hRet = GetPrevSiblingItem(hItem);
        if (hRet == ITEM_NULL)
        {
            hRet = GetParentItem(hItem);
            if (hRet == ITEM_ROOT)
                hRet = ITEM_NULL;
        }
        return hRet;
    }

    /**
     * @brief Get the next visible item
     * @param hItem item handle
     * @return next visible item handle
     */
    STDMETHOD_(HSTREEITEM, GetNextVisibleItem)(HSTREEITEM hItem) const OVERRIDE
    {
        SASSERT(IsItemVisible(hItem));
        if (IsItemExpanded(hItem))
        {
            HSTREEITEM hChild = GetFirstChildItem(hItem);
            if (hChild != ITEM_NULL)
                return hChild;
        }

        HSTREEITEM hParent = hItem;
        while (hParent != ITEM_NULL && hParent != ITEM_ROOT)
        {
            HSTREEITEM hRet = GetNextSiblingItem(hParent);
            if (hRet)
                return hRet;
            hParent = GetParentItem(hParent);
        }
        return ITEM_NULL;
    }

    /**
     * @brief Query interface
     * @param id interface ID
     * @param ppObj interface object pointer
     * @return query result
     */
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef **ppObj) OVERRIDE
    {
        return E_NOINTERFACE;
    }

  public:
    /**
     * @brief Insert an item
     * @param data data object
     * @param hParent parent item handle
     * @param hInsertAfter insert position
     * @return inserted item handle
     */
    HSTREEITEM InsertItem(const T &data, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST)
    {
        ItemInfo ii = { 0 };
        ii.data = data;
        return m_tree.InsertItem(ii, hParent, hInsertAfter);
    }

    /**
     * @brief Delete an item
     * @param hItem item handle
     * @param bNotifyChange whether to notify the change
     */
    void DeleteItem(HSTREEITEM hItem, BOOL bNotifyChange = TRUE)
    {
        HSTREEITEM hParent = GetParentItem(hItem);
        if (!hParent)
            hParent = STVI_ROOT;
        if (bNotifyChange)
        {
            notifyItemBeforeRemove(hItem);
        }
        m_tree.DeleteItem(hItem);
        if (bNotifyChange)
        {
            notifyBranchChanged(hParent);
        }
    }

    /**
     * @brief Delete an item (extended)
     * @param hItem item handle
     * @return deletion result
     */
    BOOL DeleteItemEx(HSTREEITEM hItem)
    {
        return m_tree.DeleteItemEx(hItem);
    }

    /**
     * @brief Get the item data
     * @param hItem item handle
     * @return data object reference
     */
    const T &GetItemData(HSTREEITEM hItem) const
    {
        SASSERT(hItem != STVI_ROOT);
        ItemInfo &ii = m_tree.GetItemRef((HSTREEITEM)hItem);
        return ii.data;
    }

    /**
     * @brief Get the item data
     * @param hItem item handle
     * @return data object reference
     */
    T &GetItemData(HSTREEITEM hItem)
    {
        SASSERT(hItem != STVI_ROOT);
        ItemInfo &ii = m_tree.GetItemRef((HSTREEITEM)hItem);
        return ii.data;
    }

    /**
     * @brief Set the item data
     * @param hItem item handle
     * @param data data object
     */
    void SetItemData(HSTREEITEM hItem, const T &data)
    {
        SASSERT(hItem != STVI_ROOT);
        ItemInfo &ii = m_tree.GetItemRef((HSTREEITEM)hItem);
        ii.data = data;
    }

    /**
     * @brief Set the data release function
     * @param freer data release function
     */
    void SetDataFreer(FunTvItemDataFreer freer)
    {
        m_treeFreer.m_dataFreer = freer;
    }

  protected:
    CSTree<ItemInfo> m_tree;                     /**<  Tree object */
    TreeDataFreer m_treeFreer;                   /**<  Tree data releaser */
    ULONG_PTR m_rootUserData[DATA_INDEX_NUMBER]; /**<  Root node user data */
};

SNSEND

#endif /**< __SADAPTERBASE__H__ */
