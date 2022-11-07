#pragma once

#include <interface/SAdapter-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ITreeViewItemLocator
DECLARE_INTERFACE_(ITreeViewItemLocator, IObjRef)
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
    //----------------------------------------------

    /**
     * @brief 设置适配器
     * @param pAdapter ITvAdapter *--适配器
     * @return
     */
    STDMETHOD_(void, SetAdapter)(THIS_ ITvAdapter * pAdapter) PURE;

    /**
     * @brief 树列表项数据发生变化后的响应
     * @param hItem HSTREEITEM--树列表项
     * @return
     */
    STDMETHOD_(void, OnBranchChanged)(THIS_ HSTREEITEM hItem) PURE;

    /**
     * @brief 树列表项展开状态发生变化
     * @param hItem HSTREEITEM--树列表项
     * @param bExpandedOld BOOL--原展开状态标志
     * @param bExpandedNew BOOL--新展开状态标志
     * @return
     */
    STDMETHOD_(void, OnBranchExpandedChanged)
    (THIS_ HSTREEITEM hItem, BOOL bExpandedOld, BOOL bExpandedNew) PURE;

    /**
     * @brief 获取总高度
     * @return int--总高度
     */
    STDMETHOD_(int, GetTotalHeight)(CTHIS) SCONST PURE;

    /**
     * @brief 获取总宽度
     * @return int--总宽度
     */
    STDMETHOD_(int, GetTotalWidth)(CTHIS) SCONST PURE;

    /**
     * @brief 从列表项转换为显示位置
     * @param hItem HSTREEITEM--树列表项
     * @return int--显示位置
     */
    STDMETHOD_(int, Item2Position)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 显示位置转换成列表项
     * @param position int--显示位置
     * @return
     */
    STDMETHOD_(HSTREEITEM, Position2Item)(CTHIS_ int position) SCONST PURE;

    /**
     * @brief 获取滚动行大小
     * @return int--滚动行大小
     */
    STDMETHOD_(int, GetScrollLineSize)(CTHIS) SCONST PURE;

    /**
     * @brief 设置表项宽度
     * @param hItem HSTREEITEM--表项
     * @param nWidth int--宽度
     * @return
     */
    STDMETHOD_(void, SetItemWidth)(THIS_ HSTREEITEM hItem, int nWidth) PURE;

    /**
     * @brief 获取表项宽度
     * @param hItem HSTREEITEM--表项
     * @return int--表项宽度
     */
    STDMETHOD_(int, GetItemWidth)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 设置表项高度
     * @param hItem HSTREEITEM--表项
     * @param nHeight int--表项高度
     * @return
     */
    STDMETHOD_(void, SetItemHeight)(THIS_ HSTREEITEM hItem, int nHeight) PURE;

    /**
     * @brief 获取表项高度
     * @param hItem HSTREEITEM--表项
     * @return int--表项高度
     */
    STDMETHOD_(int, GetItemHeight)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

    /**
     * @brief 获取表项缩进
     * @param hItem HSTREEITEM--表项
     * @return int--表项缩进
     */
    STDMETHOD_(int, GetItemIndent)(CTHIS_ HSTREEITEM hItem) SCONST PURE;

	STDMETHOD_(int, GetIndent)(CTHIS) SCONST PURE;

	STDMETHOD_(void, SetIndent)(THIS_ int nIndent) PURE;
};

SNSEND