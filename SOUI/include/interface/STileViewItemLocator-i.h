#pragma once
#include <interface/SAdapter-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ITileViewItemLocator
DECLARE_INTERFACE_(ITileViewItemLocator, IObjRef)
{

    STDMETHOD_(void, SetAdapter)(THIS_ ILvAdapter * pAdapter) PURE;

    STDMETHOD_(void, OnDataSetChanged)(THIS) PURE;

    STDMETHOD_(int, GetItemHeight)(THIS_ int iItem) SCONST PURE;

    STDMETHOD_(void, SetItemHeight)(THIS_ int iItem, int nHeight) PURE;

    //获取item的CRect(相对于TileView)
    STDMETHOD_(RECT, GetItemRect)(THIS_ int iItem) PURE;

    //设置TileView宽度（在TileView的OnSize中调用）
    STDMETHOD_(void, SetTileViewWidth)(THIS_ int width, BOOL bDpiAware) PURE;

    //获取item的行、列位置
    STDMETHOD_(void, GetItemRowAndColIndex)(THIS_ int iItem, int *row, int *col) PURE;

    //是否为每行的最后一个元素
    STDMETHOD_(BOOL, IsLastInRow)(THIS_ int iItem) PURE;

    //获取上一行，同一列的元素index
    STDMETHOD_(int, GetUpItem)(THIS_ int iItem) PURE;
    //获取下一行，同一列的元素index
    STDMETHOD_(int, GetDownItem)(THIS_ int iItem) PURE;

    STDMETHOD_(int, GetTotalHeight)(THIS) PURE;

    STDMETHOD_(int, Item2Position)(THIS_ int iItem) PURE;

    STDMETHOD_(int, Position2Item)(THIS_ int position) PURE;

    STDMETHOD_(int, GetScrollLineSize)(THIS) SCONST PURE;

    STDMETHOD_(int, GetMarginSize)(THIS) SCONST PURE;

    STDMETHOD_(int, SetScale)(THIS_ int scale) PURE;

    STDMETHOD_(int, GetCountInRow)(THIS) SCONST PURE;

    STDMETHOD_(int, GetItemLineHeight)(THIS) SCONST PURE;

    STDMETHOD_(int, GetItemWidth)(THIS) SCONST PURE;
};

SNSEND