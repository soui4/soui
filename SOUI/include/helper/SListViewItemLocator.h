#ifndef __SLISTVIEWITEMLOCATOR__H__
#define __SLISTVIEWITEMLOCATOR__H__

#include "interface/SListViewItemLocator-i.h"
#include "control/STree.h"

SNSBEGIN

/**
 * @class SListViewItemLocatorFix
 * @brief 固定高度的列表视图项定位器
 */
class SOUI_EXP SListViewItemLocatorFix : public TObjRefImpl<IListViewItemLocator> {
  public:
    /**
     * @brief 构造函数
     * @param nItemHei 每个项的高度
     * @param nDividerSize 分隔符的高度（默认为0）
     */
    SListViewItemLocatorFix(SLayoutSize nItemHei, SLayoutSize nDividerSize = SLayoutSize());

  public:
    /**
     * @brief 设置适配器
     * @param pAdapter 列表适配器对象指针
     */
    STDMETHOD_(void, SetAdapter)(THIS_ ILvAdapter *pAdapter) OVERRIDE;

    /**
     * @brief 数据集发生变化时调用
     */
    STDMETHOD_(void, OnDataSetChanged)(THIS) OVERRIDE
    {
    }

    /**
     * @brief 检查是否为固定高度
     * @return 固定高度返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, IsFixHeight)(THIS) SCONST OVERRIDE;

    /**
     * @brief 获取指定项的高度
     * @param iItem 项索引
     * @return 项的高度
     */
    STDMETHOD_(int, GetItemHeight)(THIS_ int iItem) SCONST OVERRIDE;

    /**
     * @brief 设置指定项的高度
     * @param iItem 项索引
     * @param nHeight 新的高度
     */
    STDMETHOD_(void, SetItemHeight)(THIS_ int iItem, int nHeight) OVERRIDE;

    /**
     * @brief 获取总高度
     * @return 总高度
     */
    STDMETHOD_(int, GetTotalHeight)(THIS) OVERRIDE;

    /**
     * @brief 将项索引转换为位置
     * @param iItem 项索引
     * @return 位置
     */
    STDMETHOD_(int, Item2Position)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief 将位置转换为项索引
     * @param position 位置
     * @return 项索引
     */
    STDMETHOD_(int, Position2Item)(THIS_ int position) OVERRIDE;

    /**
     * @brief 获取滚动行大小
     * @return 滚动行大小
     */
    STDMETHOD_(int, GetScrollLineSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief 获取分隔符大小
     * @return 分隔符大小
     */
    STDMETHOD_(int, GetDividerSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief 设置缩放比例
     * @param nScale 缩放比例
     */
    STDMETHOD_(void, SetScale)(THIS_ int nScale) OVERRIDE;

  protected:
    /**
     * @brief 获取固定项的高度
     * @return 固定项的高度
     */
    int GetFixItemHeight() const;

    SLayoutSize m_nItemHeight;         ///< 每个项的高度
    SLayoutSize m_nDividerSize;        ///< 分隔符的高度
    int m_nScale;                      ///< 缩放比例
    SAutoRefPtr<ILvAdapter> m_adapter; ///< 列表适配器对象指针
};

/**
 * @class SListViewItemLocatorFlex
 * @brief 可变高度的列表视图项定位器
 */
class SOUI_EXP SListViewItemLocatorFlex : public TObjRefImpl<IListViewItemLocator> {
  public:
    /**
     * @brief 构造函数
     * @param nItemHei 每个项的高度
     * @param nDividerSize 分隔符的高度（默认为0）
     */
    SListViewItemLocatorFlex(SLayoutSize nItemHei, SLayoutSize nDividerSize = SLayoutSize());

    /**
     * @brief 析构函数
     */
    ~SListViewItemLocatorFlex();

    /**
     * @brief 设置适配器
     * @param pAdapter 列表适配器对象指针
     */
    STDMETHOD_(void, SetAdapter)(THIS_ ILvAdapter *pAdapter) OVERRIDE;

    /**
     * @brief 数据集发生变化时调用
     */
    STDMETHOD_(void, OnDataSetChanged)(THIS) OVERRIDE;

    /**
     * @brief 检查是否为固定高度
     * @return 固定高度返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, IsFixHeight)(THIS) SCONST OVERRIDE;

    /**
     * @brief 获取指定项的高度
     * @param iItem 项索引
     * @return 项的高度
     */
    STDMETHOD_(int, GetItemHeight)(THIS_ int iItem) SCONST OVERRIDE;

    /**
     * @brief 设置指定项的高度
     * @param iItem 项索引
     * @param nHeight 新的高度
     */
    STDMETHOD_(void, SetItemHeight)(THIS_ int iItem, int nHeight) OVERRIDE;

    /**
     * @brief 获取总高度
     * @return 总高度
     */
    STDMETHOD_(int, GetTotalHeight)(THIS) OVERRIDE;

    /**
     * @brief 将项索引转换为位置
     * @param iItem 项索引
     * @return 位置
     */
    STDMETHOD_(int, Item2Position)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief 将位置转换为项索引
     * @param position 位置
     * @return 项索引
     */
    STDMETHOD_(int, Position2Item)(THIS_ int position) OVERRIDE;

    /**
     * @brief 获取滚动行大小
     * @return 滚动行大小
     */
    STDMETHOD_(int, GetScrollLineSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief 获取分隔符大小
     * @return 分隔符大小
     */
    STDMETHOD_(int, GetDividerSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief 设置缩放比例
     * @param nScale 缩放比例
     */
    STDMETHOD_(void, SetScale)(THIS_ int nScale) OVERRIDE;

  protected:
    /**
     * @brief 初始化索引
     * @param hParent 父项句柄
     * @param nItems 项数量
     * @param nSubBranchSize 子分支大小
     */
    void InitIndex(HSTREEITEM hParent, int nItems, int nSubBranchSize);

    /**
     * @brief 获取固定项的高度
     * @return 固定项的高度
     */
    int GetFixItemHeight() const;

    /**
     * @brief 获取索引深度
     * @return 索引深度
     */
    int GetIndexDeep() const;

    /**
     * @brief 清除索引
     */
    void Clear();

    /**
     * @brief 将分支转换为偏移量
     * @param hBranch 分支句柄
     * @return 偏移量
     */
    int Branch2Offset(HSTREEITEM hBranch) const;

    /**
     * @brief 将分支转换为索引
     * @param hBranch 分支句柄
     * @return 索引
     */
    int Branch2Index(HSTREEITEM hBranch) const;

    /**
     * @brief 将偏移量转换为分支
     * @param hParent 父项句柄
     * @param nOffset 偏移量
     * @return 分支句柄
     */
    HSTREEITEM Offset2Branch(HSTREEITEM hParent, int nOffset);

    SLayoutSize m_nItemHeight;  ///< 每个项的高度
    SLayoutSize m_nDividerSize; ///< 分隔符的高度
    int m_nScale;               ///< 缩放比例

    /**
     * @struct BranchInfo
     * @brief 分支信息结构
     */
    struct BranchInfo
    {
        int nBranchHei;  ///< 分支高度
        int nBranchSize; ///< 分支中包含的节点数量
    };

    CSTree<BranchInfo> m_itemPosIndex; ///< 记录分支高度

    /**
     * @class SegmentInfo
     * @brief 段信息类
     */
    class SegmentInfo {
      public:
        /**
         * @brief 构造函数
         * @param nItems 项数量
         * @param hBranch 分支句柄
         */
        SegmentInfo(int nItems, HSTREEITEM hBranch)
            : hItem(hBranch)
        {
            this->nItems = nItems;
            pItemHeight = new int[nItems];
            memset(pItemHeight, 0xff, nItems * sizeof(int));
        }

        /**
         * @brief 析构函数
         */
        ~SegmentInfo()
        {
            if (pItemHeight)
                delete[] pItemHeight;
        }

        HSTREEITEM hItem; ///< 分支句柄
        int nItems;       ///< 项数量
        int *pItemHeight; ///< 段中每一个项的高度
    };

    SArray<SegmentInfo *> m_segments;  ///< 段信息数组
    SAutoRefPtr<ILvAdapter> m_adapter; ///< 列表适配器对象指针
};

SNSEND

#endif // __SLISTVIEWITEMLOCATOR__H__