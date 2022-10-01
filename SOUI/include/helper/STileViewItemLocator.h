#pragma once
#include <interface/STileViewItemLocator-i.h>

SNSBEGIN
class SOUI_EXP STileViewItemLocator : public TObjRefImpl<ITileViewItemLocator> {
  public:
    STileViewItemLocator(int nItemHei, int nItemWid, int nMarginSize = 0);
    STileViewItemLocator(LPCWSTR szItemHei, LPCWSTR szItemWid, SLayoutSize marginSize = SLayoutSize());

  public:
    STDMETHOD_(void, SetAdapter)(THIS_ ILvAdapter *pAdapter) OVERRIDE;

    STDMETHOD_(void, OnDataSetChanged)(THIS) OVERRIDE
    {
    }

    STDMETHOD_(int, GetItemHeight)(THIS_ int iItem) SCONST OVERRIDE;

    STDMETHOD_(void, SetItemHeight)(THIS_ int iItem, int nHeight) OVERRIDE;

    //获取item的CRect(相对于TileView)
    STDMETHOD_(RECT, GetItemRect)(THIS_ int iItem) OVERRIDE;

    //设置TileView宽度（在TileView的OnSize中调用）
    STDMETHOD_(void, SetTileViewWidth)(THIS_ int width, BOOL bDpiAware) OVERRIDE;

    //获取item的行、列位置
    STDMETHOD_(void, GetItemRowAndColIndex)(THIS_ int iItem, int *row, int *col) OVERRIDE;

    //是否为每行的最后一个元素
    STDMETHOD_(BOOL, IsLastInRow)(THIS_ int iItem) OVERRIDE;

    //获取上一行，同一列的元素index
    STDMETHOD_(int, GetUpItem)(THIS_ int iItem) OVERRIDE;
    //获取下一行，同一列的元素index
    STDMETHOD_(int, GetDownItem)(THIS_ int iItem) OVERRIDE;

    STDMETHOD_(int, GetTotalHeight)(THIS) OVERRIDE;

    STDMETHOD_(int, Item2Position)(THIS_ int iItem) OVERRIDE;

    STDMETHOD_(int, Position2Item)(THIS_ int position) OVERRIDE;

    STDMETHOD_(int, GetScrollLineSize)(THIS) SCONST OVERRIDE;

    STDMETHOD_(int, GetMarginSize)(THIS) SCONST OVERRIDE;

    STDMETHOD_(int, SetScale)(THIS_ int scale) OVERRIDE;

    STDMETHOD_(int, GetCountInRow)(THIS) SCONST OVERRIDE;

    STDMETHOD_(int, GetItemLineHeight)(THIS) SCONST OVERRIDE;

    STDMETHOD_(int, GetItemWidth)(THIS) SCONST OVERRIDE;

  protected:
    //行高（包括间隔）

    int m_scale;
    SLayoutSize m_nItemWidth;     // item宽
    SLayoutSize m_nItemHeight;    // item高
    SLayoutSize m_nTileViewWidth; // TileView宽度（用于计算m_nCountInRow）
    SLayoutSize m_nItemMargin;    //块间距
    int m_nCountInRow;            //每行的item个数

    SAutoRefPtr<ILvAdapter> m_adapter;
};

SNSEND
