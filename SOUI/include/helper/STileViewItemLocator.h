#ifndef __STILEVIEWITEMLOCATOR__H__
#define __STILEVIEWITEMLOCATOR__H__

#include <interface/STileViewItemLocator-i.h>

SNSBEGIN

/**
 * @class STileViewItemLocator
 * @brief Tile view item locator class
 */
class SOUI_EXP STileViewItemLocator : public TObjRefImpl<ITileViewItemLocator> {
  public:
    /**
     * @brief Constructor
     * @param owner Pointer to the owning window object
     * @param nItemHei Height of each item
     * @param nItemWid Width of each item
     * @param nMarginSize Spacing between items (default is 0)
     */
    STileViewItemLocator(SWindow *owner, int nItemHei, int nItemWid, int nMarginSize = 0);

    /**
     * @brief Constructor
     * @param owner Pointer to the owning window object
     * @param szItemHei Height of each item (as string)
     * @param szItemWid Width of each item (as string)
     * @param marginSize Spacing between items (default is SLayoutSize())
     */
    STileViewItemLocator(SWindow *owner, LPCWSTR szItemHei, LPCWSTR szItemWid, SLayoutSize marginSize = SLayoutSize());

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
     * @brief Get the rectangle of the specified item (relative to TileView)
     * @param iItem Item index
     * @return Rectangle of the item
     */
    STDMETHOD_(RECT, GetItemRect)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief Set the width of TileView (called in TileView's OnSize)
     * @param width Width of TileView
     * @param bDpiAware Whether to consider DPI scaling
     */
    STDMETHOD_(void, SetTileViewWidth)(THIS_ int width, BOOL bDpiAware) OVERRIDE;

    /**
     * @brief Get the row and column index of the specified item
     * @param iItem Item index
     * @param row Pointer to row index
     * @param col Pointer to column index
     */
    STDMETHOD_(void, GetItemRowAndColIndex)(THIS_ int iItem, int *row, int *col) OVERRIDE;

    /**
     * @brief Determine whether the specified item is the last element of its row
     * @param iItem Item index
     * @return Returns TRUE if it is the last element, otherwise FALSE
     */
    STDMETHOD_(BOOL, IsLastInRow)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief Get the element index of the same column in the previous row
     * @param iItem Item index
     * @return Element index of the same column in the previous row
     */
    STDMETHOD_(int, GetUpItem)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief Get the element index of the same column in the next row
     * @param iItem Item index
     * @return Element index of the same column in the next row
     */
    STDMETHOD_(int, GetDownItem)(THIS_ int iItem) OVERRIDE;

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
     * @brief Get the margin size
     * @return Margin size
     */
    STDMETHOD_(int, GetMarginSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set the scale ratio
     * @param scale Scale ratio
     * @return New scale ratio
     */
    STDMETHOD_(int, SetScale)(THIS_ int scale) OVERRIDE;

    /**
     * @brief Get the number of items per row
     * @return Number of items per row
     */
    STDMETHOD_(int, GetCountInRow)(THIS) SCONST OVERRIDE;

    /**
     * @brief Get the row height of the item
     * @return Row height of the item
     */
    STDMETHOD_(int, GetItemLineHeight)(THIS) SCONST OVERRIDE;

    /**
     * @brief Get the width of the item
     * @return Width of the item
     */
    STDMETHOD_(int, GetItemWidth)(THIS) SCONST OVERRIDE;

  protected:
    int m_scale;                  /**< Scale */
    SLayoutSize m_nItemWidth;     /**< Item width */
    SLayoutSize m_nItemHeight;    /**< Item height */
    SLayoutSize m_nTileViewWidth; /**< TileView width (used to compute m_nCountInRow) */
    SLayoutSize m_nItemMargin;    /**< Block spacing */
    int m_nCountInRow;            /**< Number of items per row */

    SAutoRefPtr<ILvAdapter> m_adapter; /**< Pointer to list adapter object */
    SWindow *m_pOwner;                 /**< Pointer to the owning window object */
};

SNSEND

#endif /**< __STILEVIEWITEMLOCATOR__H__ */