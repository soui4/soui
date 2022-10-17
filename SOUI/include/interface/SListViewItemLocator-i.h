#pragma once

#include <interface/SAdapter-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IListViewItemLocator
DECLARE_INTERFACE_(IListViewItemLocator, IObjRef)
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

    //-----------------------------------------------------

    /**
     * @brief 设置适配器
     * @param pAdapter ILvAdapter *--适配器指针
     * @return
     */
    STDMETHOD_(void, SetAdapter)(THIS_ ILvAdapter * pAdapter) PURE;

    /**
     * @brief 数据改变后的处理
     * @return
     */
    STDMETHOD_(void, OnDataSetChanged)(THIS) PURE;

    /**
     * @brief 获取表项固定高度标志
     * @return TRUE--固定高度，FALSE--可变高度
     */
    STDMETHOD_(BOOL, IsFixHeight)(CTHIS) SCONST PURE;

    /**
     * @brief 获取表项高度
     * @param iItem int--表项索引
     * @return int--表项高度
     */
    STDMETHOD_(int, GetItemHeight)(CTHIS_ int iItem) SCONST PURE;

    /**
     * @brief 设定表项高度
     * @param iItem int--表项索引
     * @param nHeight int--表项高度
     * @return
     */
    STDMETHOD_(void, SetItemHeight)(THIS_ int iItem, int nHeight) PURE;

    /**
     * @brief 获取列表总高度
     * @return int--列表总高度
     */
    STDMETHOD_(int, GetTotalHeight)(THIS) PURE;

    /**
     * @brief 获取表项在列表布局中的坐标
     * @param iItem int--表项索引
     * @return int--坐标
     */
    STDMETHOD_(int, Item2Position)(THIS_ int iItem) PURE;

    /**
     * @brief 将坐标转换成表项索引
     * @param position int--表项坐标
     * @return int--表项索引
     */
    STDMETHOD_(int, Position2Item)(THIS_ int position) PURE;

    /**
     * @brief 获取滚动行大小
     * @return int--滚动行大小
     */
    STDMETHOD_(int, GetScrollLineSize)(CTHIS) SCONST PURE;

    /**
     * @brief 获取分割符大小
     * @return int--分割符大小
     */
    STDMETHOD_(int, GetDividerSize)(CTHIS) SCONST PURE;

    /**
     * @brief 设定放大比例
     * @param nScale int--放大比例
     * @return
     * @remark 100为原始大小
     */
    STDMETHOD_(void, SetScale)(THIS_ int nScale) PURE;
};

SNSEND