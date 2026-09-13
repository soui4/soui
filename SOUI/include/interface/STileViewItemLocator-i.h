#ifndef __STILEVIEWITEMLOCATOR_I__H__
#define __STILEVIEWITEMLOCATOR_I__H__
#include <interface/SAdapter-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ITileViewItemLocator
DECLARE_INTERFACE_(ITileViewItemLocator, IObjRef)
{
    /** Add reference */
    //
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /** Release reference */
    //
    STDMETHOD_(long, Release)(THIS) PURE;

    /** Release object */
    //
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;
    //--------------------------------------------------

    /**
     * @brief Set the associated adapter
     * @param pAdapter ILvAdapter *--Adapter pointer
     * @return void
     */
    STDMETHOD_(void, SetAdapter)(THIS_ ILvAdapter * pAdapter) PURE;

    /**
     * @brief Response method after the data set changes
     * @return
     */
    STDMETHOD_(void, OnDataSetChanged)(THIS) PURE;

    /**
     * @brief Get the item height
     * @param iItem int -- Item index
     * @return int--Item height
     */
    STDMETHOD_(int, GetItemHeight)(CTHIS_ int iItem) SCONST PURE;

    /**
     * @brief Set the item height
     * @param iItem int--Item index
     * @param nHeight int--Item height
     * @return
     */
    STDMETHOD_(void, SetItemHeight)(THIS_ int iItem, int nHeight) PURE;

    /**
     * @brief Get the item display position
     * @param iItem int--Item index
     * @return RECT--Item display rectangle
     * @remark Get the CRect of the item (relative to TileView)
     */
    STDMETHOD_(RECT, GetItemRect)(THIS_ int iItem) PURE;

    /**
     * @brief Set the TileView width
     * @param width int--Width
     * @param bDpiAware BOOL--dpi aware flag
     * @return
     * @remark Called in TileView's OnSize
     */
    STDMETHOD_(void, SetTileViewWidth)(THIS_ int width, BOOL bDpiAware) PURE;

    /**
     * @brief Get the item's row and column position
     * @param iItem int--Item index
     * @param [out] row int*--Row number
     * @param [out] col int*--Column number
     * @return
     */
    STDMETHOD_(void, GetItemRowAndColIndex)(THIS_ int iItem, int *row, int *col) PURE;

    /**
     * @brief Whether it is the last element of a row
     * @param iItem int--Item index
     * @return TRUE--Yes
     */
    STDMETHOD_(BOOL, IsLastInRow)(THIS_ int iItem) PURE;

    /**
     * @brief Get the index of the element in the previous row, same column
     * @param iItem int--Item index
     * @return int--Index of the element in the previous row, same column
     */
    STDMETHOD_(int, GetUpItem)(THIS_ int iItem) PURE;

    /**
     * @brief Get the index of the element in the next row, same column
     * @param iItem int--Item index
     * @return int--Index of the element in the next row, same column
     */
    STDMETHOD_(int, GetDownItem)(THIS_ int iItem) PURE;

    /**
     * @brief Compute the total list display height
     * @return int--Total list display height
     */
    STDMETHOD_(int, GetTotalHeight)(THIS) PURE;

    /**
     * @brief Get the item's Y position on the scrollbar
     * @param iItem int--Item index
     * @return int--Y-axis position
     */
    STDMETHOD_(int, Item2Position)(THIS_ int iItem) PURE;

    /**
     * @brief Convert the scrollbar position to an item index
     * @param position int--Scrollbar position
     * @return int--Item index
     */
    STDMETHOD_(int, Position2Item)(THIS_ int position) PURE;

    /**
     * @brief Get the row scroll height
     * @return int--Row scroll height
     */
    STDMETHOD_(int, GetScrollLineSize)(CTHIS) SCONST PURE;

    /**
     * @brief Get the list margin
     * @return int--List margin
     */
    STDMETHOD_(int, GetMarginSize)(CTHIS) SCONST PURE;

    /**
     * @brief Set the zoom factor
     * @param scale int--Zoom factor, base is 100
     * @return
     */
    STDMETHOD_(int, SetScale)(THIS_ int scale) PURE;

    /**
     * @brief Get the column count
     * @return int--Column count
     */
    STDMETHOD_(int, GetCountInRow)(CTHIS) SCONST PURE;

    /**
     * @brief Get the row height
     * @return int--Row height
     */
    STDMETHOD_(int, GetItemLineHeight)(CTHIS) SCONST PURE;

    /**
     * @brief Get the item display width
     * @return int--Item display width
     */
    STDMETHOD_(int, GetItemWidth)(CTHIS) SCONST PURE;
};

SNSEND
#endif /**< __STILEVIEWITEMLOCATOR_I__H__ */