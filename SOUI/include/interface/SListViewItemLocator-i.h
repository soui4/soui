#ifndef __SLISTVIEWITEMLOCATOR_I__H__
#define __SLISTVIEWITEMLOCATOR_I__H__

#include <interface/SAdapter-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IListViewItemLocator
DECLARE_INTERFACE_(IListViewItemLocator, IObjRef)
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

    //-----------------------------------------------------

    /**
     * @brief Set adapter
     * @param pAdapter ILvAdapter *--adapter pointer
     * @return
     */
    STDMETHOD_(void, SetAdapter)(THIS_ ILvAdapter * pAdapter) PURE;

    /**
     * @brief Handle after data changes
     * @return
     */
    STDMETHOD_(void, OnDataSetChanged)(THIS) PURE;

    /**
     * @brief Get fixed item height flag
     * @return TRUE--fixed height, FALSE--variable height
     */
    STDMETHOD_(BOOL, IsFixHeight)(CTHIS) SCONST PURE;

    /**
     * @brief Get item height
     * @param iItem int--item index
     * @return int--item height
     */
    STDMETHOD_(int, GetItemHeight)(CTHIS_ int iItem) SCONST PURE;

    /**
     * @brief Set item height
     * @param iItem int--item index
     * @param nHeight int--item height
     * @return
     */
    STDMETHOD_(void, SetItemHeight)(THIS_ int iItem, int nHeight) PURE;

    /**
     * @brief Get total list height
     * @return int--total list height
     */
    STDMETHOD_(int, GetTotalHeight)(THIS) PURE;

    /**
     * @brief Get item coordinate in list layout
     * @param iItem int--item index
     * @return int--coordinate
     */
    STDMETHOD_(int, Item2Position)(THIS_ int iItem) PURE;

    /**
     * @brief Convert coordinate to item index
     * @param position int--item coordinate
     * @return int--item index
     */
    STDMETHOD_(int, Position2Item)(THIS_ int position) PURE;

    /**
     * @brief Get scroll row size
     * @return int--scroll row size
     */
    STDMETHOD_(int, GetScrollLineSize)(CTHIS) SCONST PURE;

    /**
     * @brief Get separator size
     * @return int--separator size
     */
    STDMETHOD_(int, GetDividerSize)(CTHIS) SCONST PURE;

    /**
     * @brief Set zoom ratio
     * @param nScale int--zoom ratio
     * @return
     * @remark 100 means original size
     */
    STDMETHOD_(void, SetScale)(THIS_ int nScale) PURE;
};

SNSEND
#endif /**< __SLISTVIEWITEMLOCATOR_I__H__ */