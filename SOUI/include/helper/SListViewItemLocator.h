#ifndef __SLISTVIEWITEMLOCATOR__H__
#define __SLISTVIEWITEMLOCATOR__H__

#include "interface/SListViewItemLocator-i.h"
#include "control/STree.h"

SNSBEGIN

/**
 * @class SListViewItemLocatorFix
 * @brief List view item locator with fixed height
 */
class SOUI_EXP SListViewItemLocatorFix : public TObjRefImpl<IListViewItemLocator> {
  public:
    /**
     * @brief Constructor
     * @param nItemHei Height of each item
     * @param nDividerSize Height of the divider (default is 0)
     */
    SListViewItemLocatorFix(SLayoutSize nItemHei, SLayoutSize nDividerSize = SLayoutSize());

  public:
    /**
     * @brief Set adapter
     * @param pAdapter Pointer to the list adapter object
     */
    STDMETHOD_(void, SetAdapter)(THIS_ ILvAdapter *pAdapter) OVERRIDE;

    /**
     * @brief Called when the data set changes
     */
    STDMETHOD_(void, OnDataSetChanged)(THIS) OVERRIDE
    {
    }

    /**
     * @brief Check whether it is fixed height
     * @return Returns TRUE for fixed height, otherwise returns FALSE
     */
    STDMETHOD_(BOOL, IsFixHeight)(THIS) SCONST OVERRIDE;

    /**
     * @brief Get the height of the specified item
     * @param iItem Item index
     * @return Item height
     */
    STDMETHOD_(int, GetItemHeight)(THIS_ int iItem) SCONST OVERRIDE;

    /**
     * @brief Set the height of the specified item
     * @param iItem Item index
     * @param nHeight New height
     */
    STDMETHOD_(void, SetItemHeight)(THIS_ int iItem, int nHeight) OVERRIDE;

    /**
     * @brief Get total height
     * @return Total height
     */
    STDMETHOD_(int, GetTotalHeight)(THIS) OVERRIDE;

    /**
     * @brief Convert item index to position
     * @param iItem Item index
     * @return Position
     */
    STDMETHOD_(int, Item2Position)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief Convert position to item index
     * @param position Position
     * @return Item index
     */
    STDMETHOD_(int, Position2Item)(THIS_ int position) OVERRIDE;

    /**
     * @brief Get scroll line size
     * @return Scroll line size
     */
    STDMETHOD_(int, GetScrollLineSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief Get divider size
     * @return Divider size
     */
    STDMETHOD_(int, GetDividerSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set scale
     * @param nScale Scale
     */
    STDMETHOD_(void, SetScale)(THIS_ int nScale) OVERRIDE;

  protected:
    /**
     * @brief Get the height of the fixed item
     * @return Fixed item height
     */
    int GetFixItemHeight() const;

    SLayoutSize m_nItemHeight;         /**<  Height of each item */
    SLayoutSize m_nDividerSize;        /**<  Divider height */
    int m_nScale;                      /**<  Scale */
    SAutoRefPtr<ILvAdapter> m_adapter; /**<  Pointer to the list adapter object */
};

/**
 * @class SListViewItemLocatorFlex
 * @brief List view item locator with variable height
 */
class SOUI_EXP SListViewItemLocatorFlex : public TObjRefImpl<IListViewItemLocator> {
  public:
    /**
     * @brief Constructor
     * @param nItemHei Height of each item
     * @param nDividerSize Height of the divider (default is 0)
     */
    SListViewItemLocatorFlex(SLayoutSize nItemHei, SLayoutSize nDividerSize = SLayoutSize());

    /**
     * @brief Destructor
     */
    ~SListViewItemLocatorFlex();

    /**
     * @brief Set adapter
     * @param pAdapter Pointer to the list adapter object
     */
    STDMETHOD_(void, SetAdapter)(THIS_ ILvAdapter *pAdapter) OVERRIDE;

    /**
     * @brief Called when the data set changes
     */
    STDMETHOD_(void, OnDataSetChanged)(THIS) OVERRIDE;

    /**
     * @brief Check whether it is fixed height
     * @return Returns TRUE for fixed height, otherwise returns FALSE
     */
    STDMETHOD_(BOOL, IsFixHeight)(THIS) SCONST OVERRIDE;

    /**
     * @brief Get the height of the specified item
     * @param iItem Item index
     * @return Item height
     */
    STDMETHOD_(int, GetItemHeight)(THIS_ int iItem) SCONST OVERRIDE;

    /**
     * @brief Set the height of the specified item
     * @param iItem Item index
     * @param nHeight New height
     */
    STDMETHOD_(void, SetItemHeight)(THIS_ int iItem, int nHeight) OVERRIDE;

    /**
     * @brief Get total height
     * @return Total height
     */
    STDMETHOD_(int, GetTotalHeight)(THIS) OVERRIDE;

    /**
     * @brief Convert item index to position
     * @param iItem Item index
     * @return Position
     */
    STDMETHOD_(int, Item2Position)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief Convert position to item index
     * @param position Position
     * @return Item index
     */
    STDMETHOD_(int, Position2Item)(THIS_ int position) OVERRIDE;

    /**
     * @brief Get scroll line size
     * @return Scroll line size
     */
    STDMETHOD_(int, GetScrollLineSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief Get divider size
     * @return Divider size
     */
    STDMETHOD_(int, GetDividerSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set scale
     * @param nScale Scale
     */
    STDMETHOD_(void, SetScale)(THIS_ int nScale) OVERRIDE;

  protected:
    /**
     * @brief Initialize index
     * @param hParent Parent item handle
     * @param nItems Number of items
     * @param nSubBranchSize Sub-branch size
     */
    void InitIndex(HSTREEITEM hParent, int nItems, int nSubBranchSize);

    /**
     * @brief Get the height of the fixed item
     * @return Fixed item height
     */
    int GetFixItemHeight() const;

    /**
     * @brief Get index depth
     * @return Index depth
     */
    int GetIndexDeep() const;

    /**
     * @brief Clear index
     */
    void Clear();

    /**
     * @brief Convert branch to offset
     * @param hBranch Branch handle
     * @return Offset
     */
    int Branch2Offset(HSTREEITEM hBranch) const;

    /**
     * @brief Convert branch to index
     * @param hBranch Branch handle
     * @return Index
     */
    int Branch2Index(HSTREEITEM hBranch) const;

    /**
     * @brief Convert offset to branch
     * @param hParent Parent item handle
     * @param nOffset Offset
     * @return Branch handle
     */
    HSTREEITEM Offset2Branch(HSTREEITEM hParent, int nOffset);

    SLayoutSize m_nItemHeight;  /**<  Height of each item */
    SLayoutSize m_nDividerSize; /**<  Divider height */
    int m_nScale;               /**<  Scale */

    /**
     * @struct BranchInfo
     * @brief Branch information structure
     */
    struct BranchInfo
    {
        int nBranchHei;  /**<  Branch height */
        int nBranchSize; /**<  Number of nodes contained in the branch */
    };

    CSTree<BranchInfo> m_itemPosIndex; /**<  Record branch height */

    /**
     * @class SegmentInfo
     * @brief Segment information class
     */
    class SegmentInfo {
      public:
        /**
         * @brief Constructor
         * @param nItems Number of items
         * @param hBranch Branch handle
         */
        SegmentInfo(int nItems, HSTREEITEM hBranch)
            : hItem(hBranch)
        {
            this->nItems = nItems;
            pItemHeight = new int[nItems];
            memset(pItemHeight, 0xff, nItems * sizeof(int));
        }

        /**
         * @brief Destructor
         */
        ~SegmentInfo()
        {
            if (pItemHeight)
                delete[] pItemHeight;
        }

        HSTREEITEM hItem; /**<  Branch handle */
        int nItems;       /**<  Number of items */
        int *pItemHeight; /**<  Height of each item in the segment */
    };

    SArray<SegmentInfo *> m_segments;  /**<  Segment information array */
    SAutoRefPtr<ILvAdapter> m_adapter; /**<  Pointer to the list adapter object */
};

SNSEND

#endif /**< __SLISTVIEWITEMLOCATOR__H__ */