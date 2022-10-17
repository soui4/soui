#pragma once
#include <interface/SAdapter-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ITileViewItemLocator
DECLARE_INTERFACE_(ITileViewItemLocator, IObjRef)
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
    //--------------------------------------------------

    /**
     * @brief 设置关联的适配器
     * @param pAdapter ILvAdapter *--适配器指针
     * @return void
     */
    STDMETHOD_(void, SetAdapter)(THIS_ ILvAdapter * pAdapter) PURE;

    /**
     * @brief 数据集发生变化后的响应方法
     * @return
     */
    STDMETHOD_(void, OnDataSetChanged)(THIS) PURE;

    /**
     * @brief 获取表项高度
     * @param iItem int -- 表项索引
     * @return int--表项高度
     */
    STDMETHOD_(int, GetItemHeight)(CTHIS_ int iItem) SCONST PURE;

    /**
     * @brief 设定表项的高度
     * @param iItem int--表项索引
     * @param nHeight int--表项高度
     * @return
     */
    STDMETHOD_(void, SetItemHeight)(THIS_ int iItem, int nHeight) PURE;

    /**
     * @brief 获取表项的显示位置
     * @param iItem int--表项索引
     * @return RECT--表项显示矩形
     * @remark 获取item的CRect(相对于TileView)
     */
    STDMETHOD_(RECT, GetItemRect)(THIS_ int iItem) PURE;

    /**
     * @brief 设置TileView宽度
     * @param width int--宽度
     * @param bDpiAware BOOL--dpi aware flag
     * @return
     * @remark 在TileView的OnSize中调用
     */
    STDMETHOD_(void, SetTileViewWidth)(THIS_ int width, BOOL bDpiAware) PURE;

    /**
     * @brief 获取item的行、列位置
     * @param iItem int--表项索引
     * @param [out] row int*--行号
     * @param [out] col int*--列号
     * @return
     */
    STDMETHOD_(void, GetItemRowAndColIndex)(THIS_ int iItem, int *row, int *col) PURE;

    /**
     * @brief 是否为一行的最后一个元素
     * @param iItem int--表项索引
     * @return TRUE--是
     */
    STDMETHOD_(BOOL, IsLastInRow)(THIS_ int iItem) PURE;

    /**
     * @brief 获取上一行，同一列的元素index
     * @param iItem int--表项索引
     * @return int--上一行，同一列的元素index
     */
    STDMETHOD_(int, GetUpItem)(THIS_ int iItem) PURE;

    /**
     * @brief 获取下一行，同一列的元素index
     * @param iItem int--表项索引
     * @return int--下一行，同一列的元素index
     */
    STDMETHOD_(int, GetDownItem)(THIS_ int iItem) PURE;

    /**
     * @brief 计算列表显示总高度
     * @return int--列表显示总高度
     */
    STDMETHOD_(int, GetTotalHeight)(THIS) PURE;

    /**
     * @brief 获取表项在滚动条上的Y位置
     * @param iItem int--表项索引
     * @return int--Y轴位置
     */
    STDMETHOD_(int, Item2Position)(THIS_ int iItem) PURE;

    /**
     * @brief 将滚动条位置转换成表项索引
     * @param position int--滚动条位置
     * @return int--表项索引
     */
    STDMETHOD_(int, Position2Item)(THIS_ int position) PURE;

    /**
     * @brief 获取行滚动高度
     * @return int--行滚动高度
     */
    STDMETHOD_(int, GetScrollLineSize)(CTHIS) SCONST PURE;

    /**
     * @brief 获取列表边距
     * @return int--列表边距
     */
    STDMETHOD_(int, GetMarginSize)(CTHIS) SCONST PURE;

    /**
     * @brief 设置放大倍数
     * @param scale int--放大倍数，以100为基数
     * @return
     */
    STDMETHOD_(int, SetScale)(THIS_ int scale) PURE;

    /**
     * @brief 获取列数
     * @return int--列数
     */
    STDMETHOD_(int, GetCountInRow)(CTHIS) SCONST PURE;

    /**
     * @brief 获取行高
     * @return int--行高
     */
    STDMETHOD_(int, GetItemLineHeight)(CTHIS) SCONST PURE;

    /**
     * @brief 获取表项显示宽度
     * @return int--表项显示宽度
     */
    STDMETHOD_(int, GetItemWidth)(CTHIS) SCONST PURE;
};

SNSEND