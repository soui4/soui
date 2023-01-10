#pragma once

#include "interface/SAdapter-i.h"
#include <souicoll.h>
#include "control/STree.h"

SNSBEGIN
class SLvObserverMgr {
  public:
    SLvObserverMgr()
    {
    }
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

    void registerObserver(ILvDataSetObserver *observer)
    {
        SASSERT(observer);
        if (m_lstObserver.Find(observer))
            return;
        m_lstObserver.AddTail(observer);
        observer->AddRef();
    }

    /**
     * Removes a previously registered observer. The observer must not be null and it
     * must already have been registered.
     * @param observer the observer to unregister
     * @throws IllegalArgumentException the observer is null
     * @throws IllegalStateException the observer is not yet registered
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

    void notifyChanged()
    {
        SPOSITION pos = m_lstObserver.GetHeadPosition();
        while (pos)
        {
            ILvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
            pObserver->onChanged();
        }
    }

    void notifyInvalidated()
    {
        SPOSITION pos = m_lstObserver.GetHeadPosition();
        while (pos)
        {
            ILvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
            pObserver->onInvalidated();
        }
    }

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
    SList<ILvDataSetObserver *> m_lstObserver;
};

template <class BaseClass>
class LvAdatperImpl : public BaseClass {
  public:
    void notifyDataSetChanged()
    {
        m_obzMgr.notifyChanged();
    }

    /**
     * Notifies the attached observers that the underlying data is no longer valid
     * or available. Once invoked this adapter is no longer valid and should
     * not report further data set changes.
     */
    void notifyDataSetInvalidated()
    {
        m_obzMgr.notifyInvalidated();
    }

    void notifyItemDataChanged(int iItem)
    {
        m_obzMgr.notifyItemChanged(iItem);
    }

    STDMETHOD_(void, registerDataSetObserver)(ILvDataSetObserver *observer) OVERRIDE
    {
        m_obzMgr.registerObserver(observer);
    }

    STDMETHOD_(void, unregisterDataSetObserver)(ILvDataSetObserver *observer) OVERRIDE
    {
        m_obzMgr.unregisterObserver(observer);
    }

    STDMETHOD_(void, getView)(int position, IWindow *pItem, IXmlNode *pXmlTemplate) OVERRIDE
    {
        SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pItem);
        SXmlNode xmlTemplate(pXmlTemplate);
        getView(position, pItemPanel, xmlTemplate);
    }

	STDMETHOD_(void, getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate) {
		SASSERT(FALSE);
	}

    STDMETHOD_(int, getItemViewType)(int position)
    {
        (position);
        return 0;
    }

    STDMETHOD_(int, getItemViewType)(THIS_ int position, DWORD dwState) OVERRIDE
    {
        (dwState);
        return getItemViewType(position);
    }

    STDMETHOD_(int, getViewTypeCount)() OVERRIDE
    {
        return 1;
    }

    STDMETHOD_(BOOL, isEmpty)(THIS) OVERRIDE
    {
        return getCount() <= 0;
    }

    STDMETHOD_(int, getCount)(THIS) PURE;

    STDMETHOD_(SStringW, getItemDesc)(int position)
    {
        return L"";
    }

    STDMETHOD_(void, getItemDesc)(int position, IStringW *pDesc) OVERRIDE
    {
        SStringW strDesc = getItemDesc(position);
        pDesc->Copy(&strDesc);
    }

    STDMETHOD_(void, InitByTemplate)(SXmlNode xmlTemplate)
    {
        (xmlTemplate);
    }

    STDMETHOD_(void, InitByTemplate)(IXmlNode *pXmlTemplate) OVERRIDE
    {
        SXmlNode xmlTempalte(pXmlTemplate);
        InitByTemplate(xmlTempalte);
    }

    STDMETHOD_(void, getViewDesiredSize)(SIZE *ret,int position, SItemPanel *pItem, int wid, int hei)
    {
        pItem->GetDesiredSize(ret,wid, hei);
    }

    STDMETHOD_(void, getViewDesiredSize)(SIZE *ret,int position, IWindow *pItem, int wid, int hei) OVERRIDE
    {
        SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pItem);
        getViewDesiredSize(ret,position, pItemPanel, wid, hei);
    }

    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef **ppObj) OVERRIDE
    {
        return E_NOINTERFACE;
    }

  protected:
    SLvObserverMgr m_obzMgr;
};

class SAdapterBase : public TObjRefImpl<LvAdatperImpl<ILvAdapter>> {
  public:
    SAdapterBase()
    {
    }

    ~SAdapterBase()
    {
    }
};

class SMcAdapterBase : public TObjRefImpl<LvAdatperImpl<IMcAdapter>> {
  public:
    SMcAdapterBase()
    {
    }

    ~SMcAdapterBase()
    {
    }

    STDMETHOD_(BOOL, OnSort)(int iCol, UINT *pFmts, int nCols) OVERRIDE
    {
        (iCol);
        (pFmts);
        (nCols);
        return FALSE;
    }

    STDMETHOD_(SStringW, GetColumnName)(int iCol) SCONST
    {
        return L"";
    }

    STDMETHOD_(void, GetColumnName)(THIS_ int iCol, IStringW *pName) SCONST
    {
        SStringW strName = GetColumnName(iCol);
        pName->Copy(&strName);
    }

    STDMETHOD_(BOOL, IsColumnVisible)(THIS_ int iCol) SCONST OVERRIDE
    {
        (iCol);
        return TRUE;
    }

    STDMETHOD_(void, InitByTemplate)(THIS_ SXmlNode xmlTemplate)
    {
    }

    STDMETHOD_(void, InitByTemplate)(THIS_ IXmlNode *xmlTemplate) OVERRIDE
    {
        SXmlNode xmlNode(xmlTemplate);
        InitByTemplate(xmlNode);
    }
};

class STvObserverMgr {
  public:
    STvObserverMgr()
    {
    }
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

    void registerObserver(ITvDataSetObserver *observer)
    {
        SASSERT(observer);
        if (m_lstObserver.Find(observer))
            return;
        m_lstObserver.AddTail(observer);
        observer->AddRef();
    }

    /**
     * Removes a previously registered observer. The observer must not be null and it
     * must already have been registered.
     * @param observer the observer to unregister
     * @throws IllegalArgumentException the observer is null
     * @throws IllegalStateException the observer is not yet registered
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

    void notifyChanged(HSTREEITEM hBranch)
    {
        SPOSITION pos = m_lstObserver.GetHeadPosition();
        while (pos)
        {
            ITvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
            pObserver->onBranchChanged(hBranch);
        }
    }

    void notifyInvalidated(HSTREEITEM hBranch, bool bInalidParents, bool invalidChildren)
    {
        SPOSITION pos = m_lstObserver.GetHeadPosition();
        while (pos)
        {
            ITvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
            pObserver->onBranchInvalidated(hBranch, bInalidParents, invalidChildren);
        }
    }

    void notifyExpandChanged(HSTREEITEM hBranch, BOOL bExpandedOld, BOOL bExpandedNew)
    {
        SPOSITION pos = m_lstObserver.GetHeadPosition();
        while (pos)
        {
            ITvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
            pObserver->onBranchExpandedChanged(hBranch, bExpandedOld, bExpandedNew);
        }
    }

	void notifyItemBeforeRemove(HSTREEITEM hItem){
		SPOSITION pos = m_lstObserver.GetHeadPosition();
		while (pos)
		{
			ITvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
			pObserver->notifyItemBeforeRemove(hItem);
		}
	}
  protected:
    SList<ITvDataSetObserver *> m_lstObserver;
};

template <class BaseClass>
class TvAdatperImpl : public BaseClass {
  public:
    void notifyBranchChanged(HSTREEITEM hBranch)
    {
        m_obzMgr.notifyChanged(hBranch);
    }

    /**
     * Notifies the attached observers that the underlying data is no longer valid
     * or available. Once invoked this adapter is no longer valid and should
     * not report further data set changes.
     */
    void notifyBranchInvalidated(HSTREEITEM hBranch, bool bInvalidParents = true, bool bInvalidChildren = true)
    {
        m_obzMgr.notifyInvalidated(hBranch, bInvalidParents, bInvalidChildren);
    }

    void notifyBranchExpandChanged(HSTREEITEM hBranch, BOOL bExpandedOld, BOOL bExpandedNew)
    {
        m_obzMgr.notifyExpandChanged(hBranch, bExpandedOld, bExpandedNew);
    }
	
	//notify the item will be removed
	void notifyItemBeforeRemove(HSTREEITEM hItem){
		m_obzMgr.notifyItemBeforeRemove(hItem);
	}

    STDMETHOD_(void, registerDataSetObserver)(ITvDataSetObserver *observer) OVERRIDE
    {
        m_obzMgr.registerObserver(observer);
    }

    STDMETHOD_(void, unregisterDataSetObserver)(ITvDataSetObserver *observer) OVERRIDE
    {
        m_obzMgr.unregisterObserver(observer);
    }

  protected:
    STvObserverMgr m_obzMgr;
};

template <typename T>
class STreeAdapterBase : public TObjRefImpl<TvAdatperImpl<ITvAdapter>> {
  public:
    STreeAdapterBase()
    {
        memset(m_rootUserData, 0, sizeof(m_rootUserData));
    }
    ~STreeAdapterBase()
    {
    }

    struct ItemInfo
    {
        ULONG_PTR userData[DATA_INDEX_NUMBER];
        T data;
    };

    //获取hItem中的指定索引的数据
    STDMETHOD_(ULONG_PTR, GetItemDataByIndex)(HSTREEITEM hItem, DATA_INDEX idx) const OVERRIDE
    {
        if (hItem == ITEM_ROOT)
            return m_rootUserData[idx];
        ItemInfo &ii = m_tree.GetItemRef((HSTREEITEM)hItem);
        return ii.userData[idx];
    }

    //保存hItem指定索引的数据
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

    STDMETHOD_(HSTREEITEM, GetParentItem)(HSTREEITEM hItem) const OVERRIDE
    {
        if (hItem == ITEM_ROOT)
            return ITEM_NULL;
        HSTREEITEM hParent = m_tree.GetParentItem((HSTREEITEM)hItem);
        if (hParent == NULL)
            hParent = ITEM_ROOT;
        return (HSTREEITEM)hParent;
    }

    STDMETHOD_(BOOL, HasChildren)(HSTREEITEM hItem) const OVERRIDE
    {
        return GetFirstChildItem(hItem) != ITEM_NULL;
    }

    STDMETHOD_(HSTREEITEM, GetFirstChildItem)(HSTREEITEM hItem) const OVERRIDE
    {
        SASSERT(hItem != ITEM_NULL);
        return (HSTREEITEM)m_tree.GetChildItem((HSTREEITEM)hItem, TRUE);
    }

    STDMETHOD_(HSTREEITEM, GetLastChildItem)(HSTREEITEM hItem) const OVERRIDE
    {
        SASSERT(hItem != ITEM_NULL);
        return (HSTREEITEM)m_tree.GetChildItem((HSTREEITEM)hItem, FALSE);
    }

    STDMETHOD_(HSTREEITEM, GetPrevSiblingItem)(HSTREEITEM hItem) const OVERRIDE
    {
        SASSERT(hItem != ITEM_NULL && hItem != ITEM_ROOT);
        return (HSTREEITEM)m_tree.GetPrevSiblingItem((HSTREEITEM)hItem);
    }

    STDMETHOD_(HSTREEITEM, GetNextSiblingItem)(HSTREEITEM hItem) const OVERRIDE
    {
        SASSERT(hItem != ITEM_NULL && hItem != ITEM_ROOT);
        return (HSTREEITEM)m_tree.GetNextSiblingItem((HSTREEITEM)hItem);
    }

	STDMETHOD_(BOOL,IsDecendentItem)(CTHIS_ HSTREEITEM hItem,HSTREEITEM hChild) const OVERRIDE{
		HSTREEITEM hParent = GetParentItem(hChild);
		while(hParent){
			if(hParent == hItem)
				return TRUE;
			hParent = GetParentItem(hParent);
		}
		return FALSE;
	}

    STDMETHOD_(int, getViewType)(HSTREEITEM hItem) const OVERRIDE
    {
        return 0;
    }

    STDMETHOD_(int, getViewTypeCount)() const OVERRIDE
    {
        return 1;
    }

    STDMETHOD_(void, getView)(THIS_ HSTREEITEM hItem, SItemPanel *pItem, SXmlNode xmlTemplate)
    {
    }

    STDMETHOD_(void, getView)
    (THIS_ HSTREEITEM hItem, IWindow *pItem, IXmlNode *pXmlTemplate) OVERRIDE
    {
        SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pItem);
        SXmlNode xmlTemplate(pXmlTemplate);
        return getView(hItem, pItemPanel, xmlTemplate);
    }

    STDMETHOD_(void, getViewDesiredSize)(SIZE *ret,HSTREEITEM hItem, SItemPanel *pItem, int wid, int hei)
    {
        pItem->GetDesiredSize(ret,wid, hei);
    }

    STDMETHOD_(void, getViewDesiredSize)
    (SIZE *ret,HSTREEITEM hItem, IWindow *pItem, int wid, int hei) OVERRIDE
    {
        SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pItem);
        getViewDesiredSize(ret,hItem, pItemPanel, wid, hei);
    }

    STDMETHOD_(void, InitByTemplate)(SXmlNode xmlTemplate)
    {
        (xmlTemplate);
    }

    STDMETHOD_(void, InitByTemplate)(IXmlNode *pXmlTemplate) OVERRIDE
    {
        SXmlNode xmlTemplate(pXmlTemplate);
        return InitByTemplate(xmlTemplate);
    }

    STDMETHOD_(BOOL, isViewWidthMatchParent)() const OVERRIDE
    {
        return FALSE;
    }

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

    STDMETHOD_(BOOL, IsItemExpanded)(HSTREEITEM hItem) const OVERRIDE
    {
        if (hItem == ITEM_ROOT)
            return TRUE; //虚拟根节点自动展开
        return (BOOL)GetItemDataByIndex(hItem, DATA_INDEX_ITEM_EXPANDED);
    }

    STDMETHOD_(void, SetItemExpanded)(HSTREEITEM hItem, BOOL bExpanded) OVERRIDE
    {
        SetItemDataByIndex(hItem, DATA_INDEX_ITEM_EXPANDED, bExpanded);
    }

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

    STDMETHOD_(HSTREEITEM, GetFirstVisibleItem)() const OVERRIDE
    {
        return GetFirstChildItem(ITEM_ROOT);
    }

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

    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef **ppObj) OVERRIDE
    {
        return E_NOINTERFACE;
    }

  public:
    HSTREEITEM InsertItem(const T &data, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST)
    {
        ItemInfo ii = { 0 };
        ii.data = data;
        return m_tree.InsertItem(ii, hParent, hInsertAfter);
    }

    void DeleteItem(HSTREEITEM hItem,BOOL bNotifyChange = TRUE)
    {
		HSTREEITEM hParent = GetParentItem(hItem);
		if(!hParent) hParent = STVI_ROOT;
		if(bNotifyChange){
			notifyItemBeforeRemove(hItem);
		}
		m_tree.DeleteItem(hItem);
		if(bNotifyChange) {
			notifyBranchChanged(hParent);
		}
    }

    BOOL DeleteItemEx(HSTREEITEM hItem)
    {
        return m_tree.DeleteItemEx(hItem);
    }

    const T &GetItemData(HSTREEITEM hItem) const
    {
        SASSERT(hItem != STVI_ROOT);
        ItemInfo &ii = m_tree.GetItemRef((HSTREEITEM)hItem);
        return ii.data;
    }

    void SetItemData(HSTREEITEM hItem, const T &data)
    {
        SASSERT(hItem != STVI_ROOT);
        ItemInfo& ii = m_tree.GetItemRef((HSTREEITEM)hItem);
        ii.data = data;
    }
  protected:
    CSTree<ItemInfo> m_tree;
    ULONG_PTR m_rootUserData[DATA_INDEX_NUMBER];
};
SNSEND
