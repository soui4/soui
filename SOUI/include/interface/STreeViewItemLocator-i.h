#ifndef __STREEVIEWITEMLOCATOR_I__H__
#define __STREEVIEWITEMLOCATOR_I__H__

#include <interface/SAdapter-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ITreeViewItemLocator
DECLARE_INTERFACE_(ITreeViewItemLocator, IObjRef)
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
    //----------------------------------------------

    /**
     * @brief Set the adapter
     * @param pAdapter ITvAdapter *--Adapter
     * @return
     */
    STDMETHOD_(void, SetAdapter)(THIS_ ITvAdapter * pAdapter) PURE;

    /**
     * @brief Response after the tree list item data changes
     * @param hItem HSTREEITEM--Tree list item
     * @return
     */
    STDMETHOD_(void, OnBranchChanged)(THIS_ HSTREEITEM hItem) PURE;

    /**
     * @brief Tree list item expand state changed
     * @param hItem HSTREEITEM--Tree list item
     * @param bExpandedOld BOOL--Original expanded state flag
     * @param bExpandedNew BOOL--New expanded state flag
     * @return
     */
    STDMETHOD_(void, OnBranchExpandedChanged)
    (THIS_ HSTREEITEM hItem, BOOL bExpandedOld, BOOL bExpandedNew) PURE;

    /**
     * @brief Get the total height
     * @return int--Total height
     */
    STDMETHOD_(int, GetTotalHeight)(CTHIS) SCONST PURE;

    /**
     * @brief Get the total width
     * @return int--Total width
     */
    STDMETHOD_(int, GetTotalWidth)(CTHIS) SCONST PURE;

    /**
     * @brief Convert from list item to display position
     * @param hItem HSTREEITEM--Tree list item
     * @return int--Display position
     */
    STDMETHOD_(int, Item2Position)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Convert display position to list item
     * @param position int--Display position
     * @return
     */
    STDMETHOD_(HSTREEITEM, Position2Item)(CTHIS_ int position) SCONST PURE;

    /**
     * @brief Set the default item height
     * @param nHeight int--Default item height
     * @return
     */
    STDMETHOD_(void, SetDefItemHeight)(THIS_ int nHeight) PURE;

    /**
     * @brief Get scroll row size
     * @return int--scroll row size
     */
    STDMETHOD_(int, GetScrollLineSize)(CTHIS) SCONST PURE;

    /**
     * @brief Set the item width
     * @param hItem HSTREEITEM--Item
     * @param nWidth int--Width
     * @return
     */
    STDMETHOD_(void, SetItemWidth)(THIS_ HSTREEITEM hItem, int nWidth) PURE;

    /**
     * @brief Get the item width
     * @param hItem HSTREEITEM--Item
     * @return int--Item width
     */
    STDMETHOD_(int, GetItemWidth)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Set the item height
     * @param hItem HSTREEITEM--Item
     * @param nHeight int--Item height
     * @return
     */
    STDMETHOD_(void, SetItemHeight)(THIS_ HSTREEITEM hItem, int nHeight) PURE;

    /**
     * @brief Get the item height
     * @param hItem HSTREEITEM--Item
     * @return int--Item height
     */
    STDMETHOD_(int, GetItemHeight)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Get the item indentation
     * @param hItem HSTREEITEM--Item
     * @return int--Item indentation
     */
    STDMETHOD_(int, GetItemIndent)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief Get the indentation
     * @return int--Indentation
     */
    STDMETHOD_(int, GetIndent)(CTHIS) SCONST PURE;

    /**
     * @brief Set the indentation
     * @param nIndent int--Indentation
     * @return
     */
    STDMETHOD_(void, SetIndent)(THIS_ int nIndent) PURE;
};

SNSEND
#endif /**< __STREEVIEWITEMLOCATOR_I__H__ */