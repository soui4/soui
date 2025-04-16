#ifndef __STILEVIEWITEMLOCATOR__H__
#define __STILEVIEWITEMLOCATOR__H__

#include <interface/STileViewItemLocator-i.h>

SNSBEGIN

/**
 * @class STileViewItemLocator
 * @brief 瓷砖视图项定位器类
 */
class SOUI_EXP STileViewItemLocator : public TObjRefImpl<ITileViewItemLocator> {
  public:
    /**
     * @brief 构造函数
     * @param owner 所属窗口对象指针
     * @param nItemHei 每个项的高度
     * @param nItemWid 每个项的宽度
     * @param nMarginSize 项之间的间距（默认为0）
     */
    STileViewItemLocator(SWindow *owner, int nItemHei, int nItemWid, int nMarginSize = 0);

    /**
     * @brief 构造函数
     * @param owner 所属窗口对象指针
     * @param szItemHei 每个项的高度（字符串形式）
     * @param szItemWid 每个项的宽度（字符串形式）
     * @param marginSize 项之间的间距（默认为SLayoutSize()）
     */
    STileViewItemLocator(SWindow *owner, LPCWSTR szItemHei, LPCWSTR szItemWid, SLayoutSize marginSize = SLayoutSize());

  public:
    /**
     * @brief 设置适配器
     * @param pAdapter 列表适配器对象指针
     */
    STDMETHOD_(void, SetAdapter)(THIS_ ILvAdapter *pAdapter) OVERRIDE;

    /**
     * @brief 数据集改变时调用
     */
    STDMETHOD_(void, OnDataSetChanged)(THIS) OVERRIDE
    {
    }

    /**
     * @brief 获取指定项的高度
     * @param iItem 项索引
     * @return 项的高度
     */
    STDMETHOD_(int, GetItemHeight)(THIS_ int iItem) SCONST OVERRIDE;

    /**
     * @brief 设置指定项的高度
     * @param iItem 项索引
     * @param nHeight 新的高度
     */
    STDMETHOD_(void, SetItemHeight)(THIS_ int iItem, int nHeight) OVERRIDE;

    /**
     * @brief 获取指定项的矩形区域（相对于TileView）
     * @param iItem 项索引
     * @return 项的矩形区域
     */
    STDMETHOD_(RECT, GetItemRect)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief 设置TileView的宽度（在TileView的OnSize中调用）
     * @param width TileView的宽度
     * @param bDpiAware 是否考虑DPI缩放
     */
    STDMETHOD_(void, SetTileViewWidth)(THIS_ int width, BOOL bDpiAware) OVERRIDE;

    /**
     * @brief 获取指定项的行和列索引
     * @param iItem 项索引
     * @param row 行索引指针
     * @param col 列索引指针
     */
    STDMETHOD_(void, GetItemRowAndColIndex)(THIS_ int iItem, int *row, int *col) OVERRIDE;

    /**
     * @brief 判断指定项是否为每行的最后一个元素
     * @param iItem 项索引
     * @return 如果是最后一个元素返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, IsLastInRow)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief 获取上一行同一列的元素索引
     * @param iItem 项索引
     * @return 上一行同一列的元素索引
     */
    STDMETHOD_(int, GetUpItem)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief 获取下一行同一列的元素索引
     * @param iItem 项索引
     * @return 下一行同一列的元素索引
     */
    STDMETHOD_(int, GetDownItem)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief 获取总高度
     * @return 总高度
     */
    STDMETHOD_(int, GetTotalHeight)(THIS) OVERRIDE;

    /**
     * @brief 将项索引转换为位置
     * @param iItem 项索引
     * @return 位置
     */
    STDMETHOD_(int, Item2Position)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief 将位置转换为项索引
     * @param position 位置
     * @return 项索引
     */
    STDMETHOD_(int, Position2Item)(THIS_ int position) OVERRIDE;

    /**
     * @brief 获取滚动行大小
     * @return 滚动行大小
     */
    STDMETHOD_(int, GetScrollLineSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief 获取间距大小
     * @return 间距大小
     */
    STDMETHOD_(int, GetMarginSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief 设置缩放比例
     * @param scale 缩放比例
     * @return 新的缩放比例
     */
    STDMETHOD_(int, SetScale)(THIS_ int scale) OVERRIDE;

    /**
     * @brief 获取每行的项数量
     * @return 每行的项数量
     */
    STDMETHOD_(int, GetCountInRow)(THIS) SCONST OVERRIDE;

    /**
     * @brief 获取项的行高
     * @return 项的行高
     */
    STDMETHOD_(int, GetItemLineHeight)(THIS) SCONST OVERRIDE;

    /**
     * @brief 获取项的宽度
     * @return 项的宽度
     */
    STDMETHOD_(int, GetItemWidth)(THIS) SCONST OVERRIDE;

  protected:
    int m_scale;                  // 缩放比例
    SLayoutSize m_nItemWidth;     // 项宽
    SLayoutSize m_nItemHeight;    // 项高
    SLayoutSize m_nTileViewWidth; // TileView宽度（用于计算m_nCountInRow）
    SLayoutSize m_nItemMargin;    // 块间距
    int m_nCountInRow;            // 每行的项个数

    SAutoRefPtr<ILvAdapter> m_adapter; // 列表适配器对象指针
    SWindow *m_pOwner;                 // 所属窗口对象指针
};

SNSEND

#endif // __STILEVIEWITEMLOCATOR__H__