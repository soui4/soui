#ifndef __SADAPTERBASE__H__
#define __SADAPTERBASE__H__

#include "interface/SAdapter-i.h"
#include <souicoll.h>
#include "control/STree.h"

SNSBEGIN

/**
 * @class SLvObserverMgr
 * @brief 管理列表数据集观察者的类
 */
class SLvObserverMgr {
  public:
    /**
     * @brief 构造函数
     */
    SLvObserverMgr()
    {
    }

    /**
     * @brief 析构函数，释放所有观察者
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
     * @brief 注册一个观察者
     * @param observer 观察者对象指针
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
     * @brief 注销一个观察者
     * @param observer 观察者对象指针
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
     * @brief 通知所有观察者数据集已更改
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
     * @brief 通知所有观察者数据集已失效
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
     * @brief 通知所有观察者指定项的数据已更改
     * @param iItem 项索引
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
    SList<ILvDataSetObserver *> m_lstObserver; ///< 观察者列表
};

/**
 * @class LvAdatperImpl
 * @brief 列表适配器实现模板类
 * @tparam BaseClass 基类类型
 */
template <class BaseClass>
class LvAdatperImpl : public BaseClass {
  public:
    /**
     * @brief 通知所有观察者数据集已更改
     */
    void notifyDataSetChanged()
    {
        m_obzMgr.notifyChanged();
    }

    /**
     * @brief 通知所有观察者数据集已失效
     */
    void notifyDataSetInvalidated()
    {
        m_obzMgr.notifyInvalidated();
    }

    /**
     * @brief 通知所有观察者指定项的数据已更改
     * @param iItem 项索引
     */
    void notifyItemDataChanged(int iItem)
    {
        m_obzMgr.notifyItemChanged(iItem);
    }

    /**
     * @brief 注册一个数据集观察者
     * @param observer 观察者对象指针
     */
    STDMETHOD_(void, registerDataSetObserver)(ILvDataSetObserver *observer) OVERRIDE
    {
        m_obzMgr.registerObserver(observer);
    }

    /**
     * @brief 注销一个数据集观察者
     * @param observer 观察者对象指针
     */
    STDMETHOD_(void, unregisterDataSetObserver)(ILvDataSetObserver *observer) OVERRIDE
    {
        m_obzMgr.unregisterObserver(observer);
    }

    /**
     * @brief 获取视图
     * @param position 位置索引
     * @param pItem 项窗口对象
     * @param pXmlTemplate XML模板对象
     */
    STDMETHOD_(void, getView)(int position, IWindow *pItem, IXmlNode *pXmlTemplate) OVERRIDE
    {
        SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pItem);
        SXmlNode xmlTemplate(pXmlTemplate);
        getView(position, pItemPanel, xmlTemplate);
    }

    /**
     * @brief 获取视图（模板函数）
     * @param position 位置索引
     * @param pItem 项面板对象
     * @param xmlTemplate XML模板对象
     */
    STDMETHOD_(void, getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate)
    {
        SASSERT(FALSE);
    }

    /**
     * @brief 获取项视图类型
     * @param position 位置索引
     * @return 视图类型
     */
    STDMETHOD_(int, getItemViewType)(int position)
    {
        (position);
        return 0;
    }

    /**
     * @brief 获取项视图类型（重载）
     * @param position 位置索引
     * @param dwState 状态
     * @return 视图类型
     */
    STDMETHOD_(int, getItemViewType)(THIS_ int position, DWORD dwState) OVERRIDE
    {
        (dwState);
        return getItemViewType(position);
    }

    /**
     * @brief 获取视图类型数量
     * @return 视图类型数量
     */
    STDMETHOD_(int, getViewTypeCount)() OVERRIDE
    {
        return 1;
    }

    /**
     * @brief 检查数据集是否为空
     * @return 数据集为空返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, isEmpty)(THIS) OVERRIDE
    {
        return getCount() <= 0;
    }

    /**
     * @brief 获取数据集项数量
     * @return 数据集项数量
     */
    STDMETHOD_(int, getCount)(THIS) PURE;

    /**
     * @brief 获取项描述
     * @param position 位置索引
     * @return 项描述字符串
     */
    STDMETHOD_(SStringW, getItemDesc)(int position)
    {
        return L"";
    }

    /**
     * @brief 获取项描述（重载）
     * @param position 位置索引
     * @param pDesc 描述字符串对象
     */
    STDMETHOD_(void, getItemDesc)(int position, IStringW *pDesc) OVERRIDE
    {
        SStringW strDesc = getItemDesc(position);
        pDesc->Copy(&strDesc);
    }

    /**
     * @brief 通过模板初始化适配器
     * @param xmlTemplate XML模板对象
     */
    STDMETHOD_(void, InitByTemplate)(SXmlNode xmlTemplate)
    {
        (xmlTemplate);
    }

    /**
     * @brief 通过模板初始化适配器（重载）
     * @param pXmlTemplate XML模板对象指针
     */
    STDMETHOD_(void, InitByTemplate)(IXmlNode *pXmlTemplate) OVERRIDE
    {
        SXmlNode xmlTempalte(pXmlTemplate);
        InitByTemplate(xmlTempalte);
    }

    /**
     * @brief 获取视图所需大小
     * @param ret 返回的大小对象
     * @param position 位置索引
     * @param pItem 项面板对象
     * @param wid 宽度
     * @param hei 高度
     */
    STDMETHOD_(void, getViewDesiredSize)(SIZE *ret, int position, SItemPanel *pItem, int wid, int hei)
    {
        pItem->GetDesiredSize(ret, wid, hei);
    }

    /**
     * @brief 获取视图所需大小（重载）
     * @param ret 返回的大小对象
     * @param position 位置索引
     * @param pItem 项窗口对象
     * @param wid 宽度
     * @param hei 高度
     */
    STDMETHOD_(void, getViewDesiredSize)(SIZE *ret, int position, IWindow *pItem, int wid, int hei) OVERRIDE
    {
        SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pItem);
        getViewDesiredSize(ret, position, pItemPanel, wid, hei);
    }

    /**
     * @brief 查询接口
     * @param id 接口ID
     * @param ppObj 接口对象指针
     * @return 查询结果
     */
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef **ppObj) OVERRIDE
    {
        return E_NOINTERFACE;
    }

  protected:
    SLvObserverMgr m_obzMgr; ///< 观察者管理器
};

/**
 * @class SAdapterBase
 * @brief 列表适配器基类
 */
class SAdapterBase : public TObjRefImpl<LvAdatperImpl<ILvAdapter> > {
  public:
    /**
     * @brief 构造函数
     */
    SAdapterBase()
    {
    }

    /**
     * @brief 析构函数
     */
    ~SAdapterBase()
    {
    }
};

/**
 * @class SMcAdapterBase
 * @brief 多列适配器基类
 */
class SMcAdapterBase : public TObjRefImpl<LvAdatperImpl<IMcAdapter> > {
  public:
    /**
     * @brief 构造函数
     */
    SMcAdapterBase()
    {
    }

    /**
     * @brief 析构函数
     */
    ~SMcAdapterBase()
    {
    }

    /**
     * @brief 排序项
     * @param iCol 列索引
     * @param pFmts 格式数组
     * @param nCols 列数量
     * @return 排序结果
     */
    STDMETHOD_(BOOL, OnSort)(int iCol, UINT *pFmts, int nCols) OVERRIDE
    {
        (iCol);
        (pFmts);
        (nCols);
        return FALSE;
    }

    /**
     * @brief 获取列名
     * @param iCol 列索引
     * @return 列名字符串
     */
    STDMETHOD_(SStringW, GetColumnName)(int iCol) SCONST
    {
        return L"";
    }

    /**
     * @brief 获取列名（重载）
     * @param iCol 列索引
     * @param pName 列名字符串对象
     */
    STDMETHOD_(void, GetColumnName)(THIS_ int iCol, IStringW *pName) SCONST
    {
        SStringW strName = GetColumnName(iCol);
        pName->Copy(&strName);
    }

    /**
     * @brief 检查列是否可见
     * @param iCol 列索引
     * @return 列可见返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, IsColumnVisible)(THIS_ int iCol) SCONST OVERRIDE
    {
        (iCol);
        return TRUE;
    }

    /**
     * @brief 通过模板初始化适配器
     * @param xmlTemplate XML模板对象
     */
    STDMETHOD_(void, InitByTemplate)(THIS_ SXmlNode xmlTemplate)
    {
    }

    /**
     * @brief 通过模板初始化适配器（重载）
     * @param xmlTemplate XML模板对象指针
     */
    STDMETHOD_(void, InitByTemplate)(THIS_ IXmlNode *xmlTemplate) OVERRIDE
    {
        SXmlNode xmlNode(xmlTemplate);
        InitByTemplate(xmlNode);
    }

    /**
     * @brief 设置列宽
     * @param pColWid 列宽数组
     * @param nCols 列数量
     */
    STDMETHOD_(void, SetColumnsWidth)(THIS_ int *pColWid, int nCols) OVERRIDE
    {
        (pColWid);
        (nCols);
    }
};

/**
 * @class STvObserverMgr
 * @brief 管理树形数据集观察者的类
 */
class STvObserverMgr {
  public:
    /**
     * @brief 构造函数
     */
    STvObserverMgr()
    {
    }

    /**
     * @brief 析构函数，释放所有观察者
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
     * @brief 注册一个观察者
     * @param observer 观察者对象指针
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
     * @brief 注销一个观察者
     * @param observer 观察者对象指针
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
     * @brief 通知所有观察者分支已更改
     * @param hBranch 分支句柄
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
     * @brief 通知所有观察者分支已失效
     * @param hBranch 分支句柄
     * @param bInvalidParents 父节点是否失效
     * @param bInvalidChildren 子节点是否失效
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
     * @brief 通知所有观察者分支展开状态已更改
     * @param hBranch 分支句柄
     * @param bExpandedOld 旧展开状态
     * @param bExpandedNew 新展开状态
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
     * @brief 通知所有观察者项将被移除
     * @param hItem 项句柄
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
    SList<ITvDataSetObserver *> m_lstObserver; ///< 观察者列表
};

/**
 * @class TvAdatperImpl
 * @brief 树形适配器实现模板类
 * @tparam BaseClass 基类类型
 */
template <class BaseClass>
class TvAdatperImpl : public BaseClass {
  public:
    /**
     * @brief 通知所有观察者分支已更改
     * @param hBranch 分支句柄
     */
    void notifyBranchChanged(HSTREEITEM hBranch)
    {
        m_obzMgr.notifyChanged(hBranch);
    }

    /**
     * @brief 通知所有观察者分支已失效
     * @param hBranch 分支句柄
     * @param bInvalidParents 父节点是否失效
     * @param bInvalidChildren 子节点是否失效
     */
    void notifyBranchInvalidated(HSTREEITEM hBranch, bool bInvalidParents = true, bool bInvalidChildren = true)
    {
        m_obzMgr.notifyInvalidated(hBranch, bInvalidParents, bInvalidChildren);
    }

    /**
     * @brief 通知所有观察者分支展开状态已更改
     * @param hBranch 分支句柄
     * @param bExpandedOld 旧展开状态
     * @param bExpandedNew 新展开状态
     */
    void notifyBranchExpandChanged(HSTREEITEM hBranch, BOOL bExpandedOld, BOOL bExpandedNew)
    {
        m_obzMgr.notifyExpandChanged(hBranch, bExpandedOld, bExpandedNew);
    }

    /**
     * @brief 通知所有观察者项将被移除
     * @param hItem 项句柄
     */
    void notifyItemBeforeRemove(HSTREEITEM hItem)
    {
        m_obzMgr.notifyItemBeforeRemove(hItem);
    }

    /**
     * @brief 注册一个数据集观察者
     * @param observer 观察者对象指针
     */
    STDMETHOD_(void, registerDataSetObserver)(ITvDataSetObserver *observer) OVERRIDE
    {
        m_obzMgr.registerObserver(observer);
    }

    /**
     * @brief 注销一个数据集观察者
     * @param observer 观察者对象指针
     */
    STDMETHOD_(void, unregisterDataSetObserver)(ITvDataSetObserver *observer) OVERRIDE
    {
        m_obzMgr.unregisterObserver(observer);
    }

  protected:
    STvObserverMgr m_obzMgr; ///< 观察者管理器
};

/**
 * @class STreeAdapterBase
 * @brief 树形适配器基类模板
 * @tparam T 数据类型
 */
template <typename T>
class STreeAdapterBase : public TObjRefImpl<TvAdatperImpl<ITvAdapter> > {
  public:
    /**
     * @brief 数据释放函数类型
     */
    typedef void (*FunTvItemDataFreer)(T cb);

    /**
     * @struct ItemInfo
     * @brief 树节点数据结构
     */
    struct ItemInfo
    {
        ULONG_PTR userData[DATA_INDEX_NUMBER]; ///< 用户数据数组
        T data;                                ///< 数据对象
    };

    /**
     * @class TreeDataFreer
     * @brief 树节点数据释放器类
     */
    class TreeDataFreer : public CSTree<ItemInfo>::IDataFreer {
      public:
        /**
         * @brief 构造函数
         */
        TreeDataFreer()
            : m_dataFreer(NULL)
        {
        }

        FunTvItemDataFreer m_dataFreer; ///< 数据释放函数

        /**
         * @brief 释放数据
         * @param data 数据对象引用
         */
        void OnDataFree(ItemInfo &data) override
        {
            if (m_dataFreer)
                m_dataFreer(data.data);
        }
    };

  public:
    /**
     * @brief 构造函数
     */
    STreeAdapterBase()
    {
        memset(m_rootUserData, 0, sizeof(m_rootUserData));
        m_tree.SetDataFreer(&m_treeFreer);
    }

    /**
     * @brief 析构函数
     */
    ~STreeAdapterBase()
    {
        m_tree.DeleteAllItems();
    }

    /**
     * @brief 获取hItem中的指定索引的数据
     * @param hItem 项句柄
     * @param idx 索引
     * @return 用户数据
     */
    STDMETHOD_(ULONG_PTR, GetItemDataByIndex)(HSTREEITEM hItem, DATA_INDEX idx) const OVERRIDE
    {
        if (hItem == ITEM_ROOT)
            return m_rootUserData[idx];
        ItemInfo &ii = m_tree.GetItemRef((HSTREEITEM)hItem);
        return ii.userData[idx];
    }

    /**
     * @brief 保存hItem指定索引的数据
     * @param hItem 项句柄
     * @param idx 索引
     * @param data 数据
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
     * @brief 获取父项
     * @param hItem 项句柄
     * @return 父项句柄
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
     * @brief 检查是否有子项
     * @param hItem 项句柄
     * @return 有子项返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, HasChildren)(HSTREEITEM hItem) const OVERRIDE
    {
        return GetFirstChildItem(hItem) != ITEM_NULL;
    }

    /**
     * @brief 获取第一个子项
     * @param hItem 项句柄
     * @return 第一个子项句柄
     */
    STDMETHOD_(HSTREEITEM, GetFirstChildItem)(HSTREEITEM hItem) const OVERRIDE
    {
        SASSERT(hItem != ITEM_NULL);
        return (HSTREEITEM)m_tree.GetChildItem((HSTREEITEM)hItem, TRUE);
    }

    /**
     * @brief 获取最后一个子项
     * @param hItem 项句柄
     * @return 最后一个子项句柄
     */
    STDMETHOD_(HSTREEITEM, GetLastChildItem)(HSTREEITEM hItem) const OVERRIDE
    {
        SASSERT(hItem != ITEM_NULL);
        return (HSTREEITEM)m_tree.GetChildItem((HSTREEITEM)hItem, FALSE);
    }

    /**
     * @brief 获取前一个兄弟项
     * @param hItem 项句柄
     * @return 前一个兄弟项句柄
     */
    STDMETHOD_(HSTREEITEM, GetPrevSiblingItem)(HSTREEITEM hItem) const OVERRIDE
    {
        SASSERT(hItem != ITEM_NULL && hItem != ITEM_ROOT);
        return (HSTREEITEM)m_tree.GetPrevSiblingItem((HSTREEITEM)hItem);
    }

    /**
     * @brief 获取下一个兄弟项
     * @param hItem 项句柄
     * @return 下一个兄弟项句柄
     */
    STDMETHOD_(HSTREEITEM, GetNextSiblingItem)(HSTREEITEM hItem) const OVERRIDE
    {
        SASSERT(hItem != ITEM_NULL && hItem != ITEM_ROOT);
        return (HSTREEITEM)m_tree.GetNextSiblingItem((HSTREEITEM)hItem);
    }

    /**
     * @brief 检查是否为后代项
     * @param hItem 父项句柄
     * @param hChild 子项句柄
     * @return 是后代项返回TRUE，否则返回FALSE
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
     * @brief 获取项视图类型
     * @param hItem 项句柄
     * @return 视图类型
     */
    STDMETHOD_(int, getViewType)(HSTREEITEM hItem) const OVERRIDE
    {
        return 0;
    }

    /**
     * @brief 获取视图类型数量
     * @return 视图类型数量
     */
    STDMETHOD_(int, getViewTypeCount)() const OVERRIDE
    {
        return 1;
    }

    /**
     * @brief 获取视图
     * @param hItem 项句柄
     * @param pItem 项面板对象
     * @param xmlTemplate XML模板对象
     */
    STDMETHOD_(void, getView)(THIS_ HSTREEITEM hItem, SItemPanel *pItem, SXmlNode xmlTemplate)
    {
    }

    /**
     * @brief 获取视图（重载）
     * @param hItem 项句柄
     * @param pItem 项窗口对象
     * @param pXmlTemplate XML模板对象指针
     */
    STDMETHOD_(void, getView)(THIS_ HSTREEITEM hItem, IWindow *pItem, IXmlNode *pXmlTemplate) OVERRIDE
    {
        SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pItem);
        SXmlNode xmlTemplate(pXmlTemplate);
        return getView(hItem, pItemPanel, xmlTemplate);
    }

    /**
     * @brief 获取视图所需大小
     * @param ret 返回的大小对象
     * @param hItem 项句柄
     * @param pItem 项面板对象
     * @param wid 宽度
     * @param hei 高度
     */
    STDMETHOD_(void, getViewDesiredSize)(SIZE *ret, HSTREEITEM hItem, SItemPanel *pItem, int wid, int hei)
    {
        pItem->GetDesiredSize(ret, wid, hei);
    }

    /**
     * @brief 获取视图所需大小（重载）
     * @param ret 返回的大小对象
     * @param hItem 项句柄
     * @param pItem 项窗口对象
     * @param wid 宽度
     * @param hei 高度
     */
    STDMETHOD_(void, getViewDesiredSize)(SIZE *ret, HSTREEITEM hItem, IWindow *pItem, int wid, int hei) OVERRIDE
    {
        SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pItem);
        getViewDesiredSize(ret, hItem, pItemPanel, wid, hei);
    }

    /**
     * @brief 通过模板初始化适配器
     * @param xmlTemplate XML模板对象
     */
    STDMETHOD_(void, InitByTemplate)(SXmlNode xmlTemplate)
    {
        (xmlTemplate);
    }

    /**
     * @brief 通过模板初始化适配器（重载）
     * @param pXmlTemplate XML模板对象指针
     */
    STDMETHOD_(void, InitByTemplate)(IXmlNode *pXmlTemplate) OVERRIDE
    {
        SXmlNode xmlTemplate(pXmlTemplate);
        return InitByTemplate(xmlTemplate);
    }

    /**
     * @brief 检查视图宽度是否匹配父容器
     * @return 匹配返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, isViewWidthMatchParent)() const OVERRIDE
    {
        return FALSE;
    }

    /**
     * @brief 展开或折叠项
     * @param hItem 项句柄
     * @param uCode 展开/折叠代码
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
     * @brief 检查项是否展开
     * @param hItem 项句柄
     * @return 展开返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, IsItemExpanded)(HSTREEITEM hItem) const OVERRIDE
    {
        if (hItem == ITEM_ROOT)
            return TRUE; // 虚拟根节点自动展开
        return (BOOL)GetItemDataByIndex(hItem, DATA_INDEX_ITEM_EXPANDED);
    }

    /**
     * @brief 设置项展开状态
     * @param hItem 项句柄
     * @param bExpanded 展开状态
     */
    STDMETHOD_(void, SetItemExpanded)(HSTREEITEM hItem, BOOL bExpanded) OVERRIDE
    {
        SetItemDataByIndex(hItem, DATA_INDEX_ITEM_EXPANDED, bExpanded);
    }

    /**
     * @brief 检查项是否可见
     * @param hItem 项句柄
     * @return 可见返回TRUE，否则返回FALSE
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
     * @brief 获取第一个可见项
     * @return 第一个可见项句柄
     */
    STDMETHOD_(HSTREEITEM, GetFirstVisibleItem)() const OVERRIDE
    {
        return GetFirstChildItem(ITEM_ROOT);
    }

    /**
     * @brief 获取最后一个可见项
     * @return 最后一个可见项句柄
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
     * @brief 获取前一个可见项
     * @param hItem 项句柄
     * @return 前一个可见项句柄
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
     * @brief 获取下一个可见项
     * @param hItem 项句柄
     * @return 下一个可见项句柄
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
     * @brief 查询接口
     * @param id 接口ID
     * @param ppObj 接口对象指针
     * @return 查询结果
     */
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef **ppObj) OVERRIDE
    {
        return E_NOINTERFACE;
    }

  public:
    /**
     * @brief 插入项
     * @param data 数据对象
     * @param hParent 父项句柄
     * @param hInsertAfter 插入位置
     * @return 插入的项句柄
     */
    HSTREEITEM InsertItem(const T &data, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST)
    {
        ItemInfo ii = { 0 };
        ii.data = data;
        return m_tree.InsertItem(ii, hParent, hInsertAfter);
    }

    /**
     * @brief 删除项
     * @param hItem 项句柄
     * @param bNotifyChange 是否通知更改
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
     * @brief 删除项（扩展）
     * @param hItem 项句柄
     * @return 删除结果
     */
    BOOL DeleteItemEx(HSTREEITEM hItem)
    {
        return m_tree.DeleteItemEx(hItem);
    }

    /**
     * @brief 获取项数据
     * @param hItem 项句柄
     * @return 数据对象引用
     */
    const T &GetItemData(HSTREEITEM hItem) const
    {
        SASSERT(hItem != STVI_ROOT);
        ItemInfo &ii = m_tree.GetItemRef((HSTREEITEM)hItem);
        return ii.data;
    }

    /**
     * @brief 设置项数据
     * @param hItem 项句柄
     * @param data 数据对象
     */
    void SetItemData(HSTREEITEM hItem, const T &data)
    {
        SASSERT(hItem != STVI_ROOT);
        ItemInfo &ii = m_tree.GetItemRef((HSTREEITEM)hItem);
        ii.data = data;
    }

    /**
     * @brief 设置数据释放函数
     * @param freer 数据释放函数
     */
    void SetDataFreer(FunTvItemDataFreer freer)
    {
        m_treeFreer.m_dataFreer = freer;
    }

  protected:
    CSTree<ItemInfo> m_tree;                     ///< 树对象
    TreeDataFreer m_treeFreer;                   ///< 树数据释放器
    ULONG_PTR m_rootUserData[DATA_INDEX_NUMBER]; ///< 根节点用户数据
};

SNSEND

#endif // __SADAPTERBASE__H__
