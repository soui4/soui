#include "souistd.h"
#include "helper/STileViewItemLocator.h"

SNSBEGIN
//////////////////////////////////////////////////////////////////////////
// STileViewItemLocatorFix
STileViewItemLocator::STileViewItemLocator(SWindow *owner, int nItemHei, int nItemWid, int nMarginSize /*= 0*/)
    : m_nItemHeight((float)nItemHei, px)
    , m_nItemWidth((float)nItemWid, px)
    , m_nItemMargin((float)nMarginSize, px)
    , m_nTileViewWidth(0.f, px)
    , m_nCountInRow(1)
    , m_pOwner(owner)
{
}

STileViewItemLocator::STileViewItemLocator(SWindow *owner, LPCWSTR szItemHei, LPCWSTR szItemWid, SLayoutSize marginSize)
    : m_nItemHeight(GETLAYOUTSIZE(szItemHei))
    , m_nItemWidth(GETLAYOUTSIZE(szItemWid))
    , m_nItemMargin(marginSize)
    , m_nTileViewWidth(0, px)
    , m_nCountInRow(1)
    , m_scale(100)
    , m_pOwner(owner)
{
}

int STileViewItemLocator::GetScrollLineSize() const
{
    return GetItemLineHeight();
}

int STileViewItemLocator::GetMarginSize() const
{
    return m_nItemMargin.toPixelSize(m_scale);
}

int STileViewItemLocator::GetCountInRow() const
{
    return m_nCountInRow;
}

int STileViewItemLocator::GetItemLineHeight() const
{
    return m_nItemHeight.toPixelSize(m_scale) + m_nItemMargin.toPixelSize(m_scale);
}

int STileViewItemLocator::GetItemWidth() const
{
    return m_nItemWidth.toPixelSize(m_scale) + m_nItemMargin.toPixelSize(m_scale);
}

int STileViewItemLocator::SetScale(int scale)
{
    m_scale = scale;
    return 0;
}

int STileViewItemLocator::Position2Item(int position)
{
    if (!m_adapter)
    {
        return -1;
    }
    int nRet = position / GetItemLineHeight() * m_nCountInRow;

    if (nRet < 0)
    {
        nRet = 0;
    }
    if (nRet > m_adapter->getCount())
    {
        nRet = m_adapter->getCount();
    }
    return nRet;
}

int STileViewItemLocator::Item2Position(int iItem)
{
    return (iItem / m_nCountInRow) * GetItemLineHeight();
}

int STileViewItemLocator::GetTotalHeight()
{
    if (!m_adapter || m_adapter->getCount() == 0)
    {
        return 0;
    }
    int row_count = (m_adapter->getCount() - 1) / m_nCountInRow + 1;
    return GetItemLineHeight() * row_count + m_nItemMargin.toPixelSize(m_scale);
}

void STileViewItemLocator::SetItemHeight(int iItem, int nHeight)
{
}

int STileViewItemLocator::GetItemHeight(int iItem) const
{
    return m_nItemHeight.toPixelSize(m_scale);
}

void STileViewItemLocator::SetAdapter(ILvAdapter *pAdapter)
{
    m_adapter = pAdapter;
}

RECT STileViewItemLocator::GetItemRect(int iItem)
{
    //返回相对于TileView内部的Rect
    int nRowIdx, nColIdx;
    GetItemRowAndColIndex(iItem, &nRowIdx, &nColIdx);

    int margin = m_nItemMargin.toPixelSize(m_scale);
    CRect rect;
    rect.left = margin + nColIdx * (m_nItemWidth.toPixelSize(m_scale) + margin);
    rect.top = margin + nRowIdx * (m_nItemHeight.toPixelSize(m_scale) + margin);
    rect.right = rect.left + m_nItemWidth.toPixelSize(m_scale);
    rect.bottom = rect.top + m_nItemHeight.toPixelSize(m_scale);

    if ((m_pOwner->GetStyle().GetAlign() & SwndStyle::Align_MaskX) != SwndStyle::Align_Left)
    {
        CRect rcClient = m_pOwner->GetClientRect();
        int totalWid = m_nItemWidth.toPixelSize(m_scale) * m_nCountInRow + (margin + 1) * m_nCountInRow;
        switch (m_pOwner->GetStyle().GetAlign() & SwndStyle::Align_MaskX)
        {
        case SwndStyle::Align_Center:
            rect.OffsetRect((rcClient.Width() - totalWid) / 2, 0);
            break;
        case SwndStyle::Align_Right:
            rect.OffsetRect((rcClient.Width() - totalWid), 0);
            break;
        }
    }

    return rect;
}

void STileViewItemLocator::SetTileViewWidth(int width, BOOL bDpiAware)
{
    m_nTileViewWidth.setSize((float)width, bDpiAware ? dp : px);
    m_nCountInRow = m_nTileViewWidth.toPixelSize(m_scale) / (m_nItemWidth.toPixelSize(m_scale) + m_nItemMargin.toPixelSize(m_scale));
    if (m_nCountInRow == 0)
        m_nCountInRow = 1;
}

void STileViewItemLocator::GetItemRowAndColIndex(int iItem, int *pRow, int *pCol)
{
    SASSERT(pRow && pCol);
    int &row = *pRow;
    int &col = *pCol;
    row = (iItem + 1) / m_nCountInRow;
    if (row * m_nCountInRow - 1 >= iItem)
    {
        row--;
    }
    col = m_nCountInRow - ((row + 1) * m_nCountInRow - iItem);
}

BOOL STileViewItemLocator::IsLastInRow(int iItem)
{
    return iItem % m_nCountInRow == m_nCountInRow - 1;
}

int STileViewItemLocator::GetUpItem(int iItem)
{
    int up_idx = iItem - m_nCountInRow;
    //没有上一行了，返回原值
    return up_idx < 0 ? iItem : up_idx;
}

int STileViewItemLocator::GetDownItem(int iItem)
{
    int up_idx = iItem + m_nCountInRow;
    if (up_idx < m_adapter->getCount())
    {
        return up_idx;
    }
    else if ((m_adapter->getCount() - 1) / m_nCountInRow == iItem / m_nCountInRow)
    {
        //没有下一行了，返回原值
        return iItem;
    }
    //有下一行，但同一列没有元素，返回下一行最后一个元素
    return m_adapter->getCount() - 1;
}

SNSEND
