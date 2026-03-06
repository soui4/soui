#include "souistd.h"
#include "layout/SGridLayout.h"
#include <core/SWnd.h>
SNSBEGIN

/**
 * @class GridLayoutState
 * @brief 网格布局的状态对象，用于存储测量/布局过程中的临时状态变量
 */
class GridLayoutState {
  public:
    // 单元格信息
    CSize *pCellsSize;              // 每个cell的大小
    bool *pCellsOccupy;             // cell占用标记
    SGridLayoutParam **pCellsParam; // cell的布局参数
    IWindow **pCellsChild;          // cell的窗口指针
    int *pCellsColSpan;             // cell的列跨度
    int *pCellsRowSpan;             // cell的行跨度

    // 行列信息
    float *pRowsHeight;          // 每行的高度
    float *pColsWidth;           // 每列的宽度
    float *pRowsWeight;          // 每行的权重
    float *pColsWeight;          // 每列的权重
    bool *pCols_WeightAllocated; // 列的权重是否被分配
    bool *pRows_WeightAllocated; // 行的权重是否被分配

    int nCols; // 列数
    int nRows; // 行数

    /**
     * @brief 分配状态对象的内存
     */
    GridLayoutState(int cols, int rows)
    {
        nCols = cols;
        nRows = rows;

        pCellsSize = new CSize[nCols * nRows];
        pCellsOccupy = new bool[nCols * nRows];
        pCellsParam = new SGridLayoutParam *[nCols * nRows];
        pCellsChild = new IWindow *[nCols * nRows];
        pCellsColSpan = new int[nCols * nRows];
        pCellsRowSpan = new int[nCols * nRows];

        pRowsHeight = new float[nRows];
        pColsWidth = new float[nCols];
        pRowsWeight = new float[nRows];
        pColsWeight = new float[nCols];
        pCols_WeightAllocated = new bool[nCols];
        pRows_WeightAllocated = new bool[nRows];

        Initialize();
    }

    ~GridLayoutState()
    {
        delete[] pCellsOccupy;
        delete[] pCellsChild;
        delete[] pCellsParam;
        delete[] pCellsColSpan;
        delete[] pCellsRowSpan;
        delete[] pCols_WeightAllocated;
        delete[] pRows_WeightAllocated;
        delete[] pRowsHeight;
        delete[] pColsWidth;
        delete[] pRowsWeight;
        delete[] pColsWeight;
        delete[] pCellsSize;
    }

    void UpdateColWidth(int iCol)
    {
        float fColWidth = 0;
        for (int i = 0; i < nRows; i++)
        {
            fColWidth = smax(fColWidth, pCellsSize[i * nCols + iCol].cx);
        }
        pColsWidth[iCol] = fColWidth;
    }

    void UpdateRowHeight(int iRow)
    {
        float fRowHeight = 0;
        for (int i = 0; i < nCols; i++)
        {
            fRowHeight = smax(fRowHeight, pCellsSize[iRow * nCols + i].cy);
        }
        pRowsHeight[iRow] = fRowHeight;
    }

  private:
    /**
     * @brief 初始化状态对象的所有字段
     */
    void Initialize()
    {
        // 初始化单元格数组
        for (int i = 0; i < nCols * nRows; i++)
        {
            pCellsOccupy[i] = false;
            pCellsSize[i] = CSize(0, 0);
            pCellsParam[i] = NULL;
            pCellsChild[i] = NULL;
            pCellsColSpan[i] = 0;
            pCellsRowSpan[i] = 0;
        }

        // 初始化行数组
        for (int i = 0; i < nRows; i++)
        {
            pRowsHeight[i] = 0;
            pRowsWeight[i] = 0.0f;
            pRows_WeightAllocated[i] = false;
        }

        // 初始化列数组
        for (int i = 0; i < nCols; i++)
        {
            pColsWidth[i] = 0;
            pColsWeight[i] = 0.0f;
            pCols_WeightAllocated[i] = false;
        }
    }
};

static BOOL FindNextAvailableCell(bool *pCellsOccupy, int nCols, int nRows, int &iRow, int &iCol, ORIENTATION orientation);

GridGravity SGridLayoutParam::parseGridGravity(const SStringW &strValue)
{
    struct ValueMap
    {
        GridGravity gridGravity;
        LPCWSTR pszGravity;
    } map[] = {
        { gLeft, L"left" }, { gTop, L"top" }, { gCenter, L"center" }, { gRight, L"right" }, { gBottom, L"bottom" }, { gFill, L"fill" },
    };

    for (int i = 0; i < ARRAYSIZE(map); i++)
    {
        if (strValue.CompareNoCase(map[i].pszGravity) == 0)
        {
            return map[i].gridGravity;
        }
    }
    return gUndef;
}

HRESULT SGridLayoutParam::OnAttrSize(const SStringW &strValue, BOOL bLoading)
{
    SStringWList szStr;
    if (2 != SplitString(strValue, L',', szStr))
        return E_FAIL;

    width = GETLAYOUTSIZE(szStr[0]);
    height = GETLAYOUTSIZE(szStr[1]);
    return S_OK;
}

SGridLayoutParam::SGridLayoutParam()
{
    Clear();
}

void SGridLayoutParam::Clear()
{
    width.setWrapContent();
    height.setWrapContent();
    nColSpan = nRowSpan = 1;
    layoutGravityX = gUndef;
    layoutGravityY = gUndef;
    fColWeight = 0.0f;
    fRowWeight = 0.0f;
}

void SGridLayoutParam::SetMatchParent(ORIENTATION orientation)
{
    switch (orientation)
    {
    case Horz:
        width.setMatchParent();
        break;
    case Vert:
        height.setMatchParent();
        break;
    case Both:
        width.setMatchParent();
        height.setMatchParent();
        break;
    }
}

void SGridLayoutParam::SetWrapContent(ORIENTATION orientation)
{
    switch (orientation)
    {
    case Horz:
        width.setWrapContent();
        break;
    case Vert:
        height.setWrapContent();
        break;
    case Both:
        width.setWrapContent();
        height.setWrapContent();
        break;
    }
}

void SGridLayoutParam::SetSpecifiedSize(ORIENTATION orientation, const LAYOUTSIZE *pLayoutSize)
{
    switch (orientation)
    {
    case Horz:
        width = *pLayoutSize;
        break;
    case Vert:
        height = *pLayoutSize;
        break;
    case Both:
        width = height = *pLayoutSize;
        break;
    }
}

BOOL SGridLayoutParam::IsMatchParent(ORIENTATION orientation) const
{
    switch (orientation)
    {
    case Horz:
        return width.isMatchParent();
    case Vert:
        return height.isMatchParent();
    case Any:
        return IsMatchParent(Horz) || IsMatchParent(Vert);
    case Both:
    default:
        return IsMatchParent(Horz) && IsMatchParent(Vert);
    }
}

BOOL SGridLayoutParam::IsWrapContent(ORIENTATION orientation) const
{
    switch (orientation)
    {
    case Horz:
        return width.isWrapContent();
    case Vert:
        return height.isWrapContent();
    case Any:
        return IsWrapContent(Horz) || IsWrapContent(Vert);
    case Both:
    default:
        return IsWrapContent(Horz) && IsWrapContent(Vert);
    }
}

BOOL SGridLayoutParam::IsSpecifiedSize(ORIENTATION orientation) const
{
    switch (orientation)
    {
    case Horz:
        return width.isSpecifiedSize();
    case Vert:
        return height.isSpecifiedSize();
    case Any:
        return IsSpecifiedSize(Horz) || IsSpecifiedSize(Vert);
    case Both:
    default:
        return IsSpecifiedSize(Horz) && IsSpecifiedSize(Vert);
    }
}

BOOL SGridLayoutParam::GetSpecifiedSize(ORIENTATION orientation, LAYOUTSIZE *pLayoutSize) const
{
    switch (orientation)
    {
    case Horz:
        *pLayoutSize = width;
        return TRUE;
    case Vert:
        *pLayoutSize = height;
        return TRUE;
    case Any:
    case Both:
    default:
        SASSERT_MSGA(FALSE, "GetSpecifiedSize can only be applied for Horz or Vert");
        return FALSE;
    }
}

void *SGridLayoutParam::GetRawData()
{
    return (SGridLayoutParamStruct *)this;
}

ILayoutParam *SGridLayoutParam::Clone() const
{
    SGridLayoutParam *pRet = new SGridLayoutParam();
    memcpy(pRet->GetRawData(), (SGridLayoutParamStruct *)this, sizeof(SGridLayoutParamStruct));
    return pRet;
}

BOOL SGridLayoutParam::SetAnimatorValue(IPropertyValuesHolder *pHolder, float fraction, ANI_STATE state)
{
    SStringW strPropName = pHolder->GetPropertyName();
    if (strPropName.CompareNoCase(LayoutProperty::WIDTH) == 0)
    {
        SWindow::GetAnimatedLayoutSize(pHolder, fraction, width);
        return TRUE;
    }
    if (strPropName.CompareNoCase(LayoutProperty::HEIGHT) == 0)
    {
        SWindow::GetAnimatedLayoutSize(pHolder, fraction, height);
        return TRUE;
    }
    return FALSE;
}
//////////////////////////////////////////////////////////////////////////
SGridLayout::SGridLayout(void)
    : m_GravityX(gCenter)
    , m_GravityY(gCenter)
    , m_nCols(-1)
    , m_nRows(-1)
    , m_orientation(Horz)
{
}

SGridLayout::~SGridLayout(void)
{
}

BOOL SGridLayout::IsParamAcceptable(const ILayoutParam *pLayoutParam) const
{
    return pLayoutParam->IsClass(SGridLayoutParam::GetClassName());
}

ILayoutParam *SGridLayout::CreateLayoutParam() const
{
    return new SGridLayoutParam();
}

/**
 * MeasureChildren 计算gridlayout的子窗口大小
 * 参考Android GridLayout的设计，采用两遍测量方法
 *
 * 第一遍：测量所有子元素的自然大小（wrap_content）
 * 第二遍（可选）：根据权重分配剩余空间
 */
SIZE SGridLayout::MeasureChildren(const IWindow *pParent, int nWidth, int nHeight) const
{
    if (m_nCols == -1 && m_nRows == -1)
        return CSize();

    int nCols = m_nCols;
    int nRows = m_nRows;

    int nCells = CalcCells(pParent);
    if (nRows == -1)
        nRows = (nCells + nCols - 1) / nCols;
    else if (nCols == -1)
        nCols = (nCells + nRows - 1) / nRows;

    int xInter = m_xInterval.toPixelSize(pParent->GetScale());
    int yInter = m_yInterval.toPixelSize(pParent->GetScale());
    if (xInter < 0)
        xInter = 0;
    if (yInter < 0)
        yInter = 0;

    // 创建并初始化状态对象
    GridLayoutState state(nCols, nRows);

    // 创建别名以保持代码兼容性
    CSize *&pCellsSize = state.pCellsSize;
    bool *&pCellsOccupy = state.pCellsOccupy;
    SGridLayoutParam **&pCellsParam = state.pCellsParam;
    IWindow **&pCellsChild = state.pCellsChild;
    int *&pCellsColSpan = state.pCellsColSpan;
    int *&pCellsRowSpan = state.pCellsRowSpan;
    float *&pRowsHeight = state.pRowsHeight;
    float *&pColsWidth = state.pColsWidth;
    float *&pRowsWeight = state.pRowsWeight;
    float *&pColsWeight = state.pColsWeight;
    bool *&pCols_WeightAllocated = state.pCols_WeightAllocated;
    bool *&pRows_WeightAllocated = state.pRows_WeightAllocated;

    // 第一遍：测量所有子元素，不加约束
    int iRow = 0, iCol = 0;
    const IWindow *pCell = pParent->GetNextLayoutIChild(NULL);

    while (pCell)
    {
        const SGridLayoutParam *pLayoutParam = (const SGridLayoutParam *)pCell->GetLayoutParam();
        SASSERT(pLayoutParam);

        int colSpan = pLayoutParam->nColSpan;
        int rowSpan = pLayoutParam->nRowSpan;

        colSpan = smin(colSpan, nCols - iCol);
        rowSpan = smin(rowSpan, nRows - iRow);
        SASSERT(colSpan >= 1);
        SASSERT(rowSpan >= 1);

        // 找到下一个可用位置
        while (iRow < nRows && pCellsOccupy[iRow * nCols + iCol])
        {
            if (m_orientation == Horz)
            {
                iCol++;
                if (iCol >= nCols)
                {
                    iCol = 0;
                    iRow++;
                }
            }
            else // Vert
            {
                iRow++;
                if (iRow >= nRows)
                {
                    iRow = 0;
                    iCol++;
                }
            }
        }

        if (iRow >= nRows || iCol >= nCols)
            break;

        // 调整span，确保不超出边界
        colSpan = smin(colSpan, nCols - iCol);
        rowSpan = smin(rowSpan, nRows - iRow);

        // 检查跨度是否有冲突，有冲突则减少跨度
        for (int y = 0; y < rowSpan; y++)
            for (int x = 0; x < colSpan; x++)
            {
                int iCell = (iRow + y) * nCols + iCol + x;
                if (pCellsOccupy[iCell])
                {
                    rowSpan = y + 1;
                    if (y == 0)
                        colSpan = x + 1;
                    break;
                }
            }

        // 测量子元素，应用单元格约束
        CSize szCell(SIZE_WRAP_CONTENT, SIZE_WRAP_CONTENT);
        int nScale = ((IWindow *)pCell)->GetScale();

        // 应用width/height约束
        if (pLayoutParam->width.isSpecifiedSize())
            szCell.cx = pLayoutParam->width.toPixelSize(nScale);
        if (pLayoutParam->height.isSpecifiedSize())
            szCell.cy = pLayoutParam->height.toPixelSize(nScale);

        ((IWindow *)pCell)->GetDesiredSize(&szCell, szCell.cx, szCell.cy);
        // 记录cell的信息
        pCellsChild[iRow * nCols + iCol] = (IWindow *)pCell;
        pCellsParam[iRow * nCols + iCol] = (SGridLayoutParam *)pLayoutParam;
        pCellsColSpan[iRow * nCols + iCol] = colSpan;
        pCellsRowSpan[iRow * nCols + iCol] = rowSpan;

        // 标记占用并存储大小
        for (int y = 0; y < rowSpan; y++)
            for (int x = 0; x < colSpan; x++)
            {
                int iCell = (iRow + y) * nCols + iCol + x;
                pCellsOccupy[iCell] = true;
                if (x == 0 && y == 0)
                {
                    pCellsSize[iCell] = szCell;
                }
            }

        // 更新行列的最大尺寸和权重（使用float以保留精度）
        float cellWidth = (float)szCell.cx / colSpan;
        float cellHeight = (float)szCell.cy / rowSpan;
        float fColWeight = pLayoutParam->fColWeight / colSpan;
        float fRowWeight = pLayoutParam->fRowWeight / rowSpan;
        for (int x = 0; x < colSpan; x++)
        {
            if (pColsWidth[iCol + x] < cellWidth)
                pColsWidth[iCol + x] = cellWidth;
            if (pColsWeight[iCol + x] < fColWeight)
                pColsWeight[iCol + x] = fColWeight;
        }

        for (int y = 0; y < rowSpan; y++)
        {
            if (pRowsHeight[iRow + y] < cellHeight)
                pRowsHeight[iRow + y] = cellHeight;
            if (pRowsWeight[iRow + y] < fRowWeight)
                pRowsWeight[iRow + y] = fRowWeight;
        }

        // 移到下一个cell
        if (m_orientation == Horz)
        {
            iCol++;
            if (iCol >= nCols)
            {
                iCol = 0;
                iRow++;
            }
        }
        else // Vert
        {
            iRow++;
            if (iRow >= nRows)
            {
                iRow = 0;
                iCol++;
            }
        }

        pCell = pParent->GetNextLayoutIChild(pCell);
    }

    // 第二遍：计算总尺寸
    float totalWidth = 0.0f;
    float totalHeight = 0.0f;

    for (int x = 0; x < nCols; x++)
        totalWidth += pColsWidth[x];

    for (int y = 0; y < nRows; y++)
        totalHeight += pRowsHeight[y];

    totalWidth += xInter * (nCols - 1);
    totalHeight += yInter * (nRows - 1);

    // 处理权重分配（如果父容器有约束）
    bool bColWeightAllocated = false; // 标记列是否进行了权重分配
    if (nWidth > 0)
    {
        float totalColWeight = 0.0f;
        for (int i = 0; i < nCols; i++)
            totalColWeight += pColsWeight[i];

        if (totalColWeight > 0.0f && totalWidth < nWidth)
        {
            bColWeightAllocated = true;
            float remainSpace = nWidth - totalWidth;
            for (int i = 0; i < nCols; i++)
            {
                if (pColsWeight[i] > 0.0f)
                {
                    float extra = remainSpace * pColsWeight[i] / totalColWeight;
                    pColsWidth[i] += extra;
                    pCols_WeightAllocated[i] = true; // 标记该列参与了分配
                    remainSpace -= extra;
                    totalColWeight -= pColsWeight[i];
                }
            }

            // 重新计算总宽度
            totalWidth = 0.0f;
            for (int x = 0; x < nCols; x++)
                totalWidth += pColsWidth[x];
            totalWidth += xInter * (nCols - 1);
        }
    }

    bool bRowWeightAllocated = false; // 标记行是否进行了权重分配

    if (nHeight > 0)
    {
        float totalRowWeight = 0.0f;
        for (int i = 0; i < nRows; i++)
            totalRowWeight += pRowsWeight[i];

        if (totalRowWeight > 0.0f && totalHeight < nHeight)
        {
            bRowWeightAllocated = true;
            float remainSpace = nHeight - totalHeight;
            for (int i = 0; i < nRows; i++)
            {
                if (pRowsWeight[i] > 0.0f)
                {
                    float extra = remainSpace * pRowsWeight[i] / totalRowWeight;
                    pRowsHeight[i] += extra;
                    pRows_WeightAllocated[i] = true; // 标记该行参与了分配
                    totalRowWeight -= pRowsWeight[i];
                }
            }

            // 重新计算总高度
            totalHeight = 0;
            for (int y = 0; y < nRows; y++)
                totalHeight += pRowsHeight[y];
            totalHeight += yInter * (nRows - 1);
        }
    }

    // 第三遍：对于满足条件的单元格，用分配后的尺寸重新测量
    // 条件：某方向的weight在Pass 2中参与了分配，且单元格那方向的weight不为0，另一方向是WRAP_CONTENT
    for (int y = 0; y < nRows; y++)
    {
        for (int x = 0; x < nCols; x++)
        {
            int iCell = y * nCols + x;

            // 跳过非主cell
            if (!pCellsChild[iCell])
                continue;

            const SGridLayoutParam *pCell_Param = pCellsParam[iCell];
            IWindow *pCell_Wnd = pCellsChild[iCell];
            int nScale = pCell_Wnd->GetScale();
            int iCellColSpan = pCellsColSpan[iCell];
            int iCellRowSpan = pCellsRowSpan[iCell];

            // 检查列的weight是否被分配了，且cell的colWeight不为0，height是WRAP_CONTENT
            bool bColWeightMatched = false;
            if (bColWeightAllocated && pCell_Param->fColWeight > 0.0f && pCell_Param->height.isWrapContent())
            {
                // 检查该cell所在列是否参与了weight分配
                for (int xx = 0; xx < iCellColSpan; xx++)
                {
                    if (pCols_WeightAllocated[x + xx])
                    {
                        bColWeightMatched = true;
                        break;
                    }
                }
            }

            // 检查行的weight是否被分配了，且cell的rowWeight不为0，width是WRAP_CONTENT
            bool bRowWeightMatched = false;
            if (bRowWeightAllocated && pCell_Param->fRowWeight > 0.0f && pCell_Param->width.isWrapContent())
            {
                // 检查该cell所在行是否参与了weight分配
                for (int yy = 0; yy < iCellRowSpan; yy++)
                {
                    if (pRows_WeightAllocated[y + yy])
                    {
                        bRowWeightMatched = true;
                        break;
                    }
                }
            }

            // 如果列权重被分配了，用分配的列宽重新测量高度
            if (bColWeightMatched)
            {
                // 计算这个cell分配到的总宽度
                float cellWidth = 0;
                for (int xx = 0; xx < iCellColSpan; xx++)
                    cellWidth += pColsWidth[x + xx];
                cellWidth += xInter * (iCellColSpan - 1);

                // 用新的宽度重新测量高度
                CSize szCell_Remeasure((int)(cellWidth + 0.5f), SIZE_WRAP_CONTENT);
                pCell_Wnd->GetDesiredSize(&szCell_Remeasure, szCell_Remeasure.cx, szCell_Remeasure.cy);

                // 更新行高
                float newCellHeight = (float)szCell_Remeasure.cy / iCellRowSpan;
                pCellsSize[y * nCols + x].cy = newCellHeight;
                state.UpdateRowHeight(y);
            }
            // 如果行权重被分配了，用分配的行高重新测量宽度
            else if (bRowWeightMatched)
            {
                // 计算这个cell分配到的总高度
                float cellHeight = 0;
                for (int yy = 0; yy < iCellRowSpan; yy++)
                    cellHeight += pRowsHeight[y + yy];
                cellHeight += yInter * (iCellRowSpan - 1);

                // 用新的高度重新测量宽度
                CSize szCell_Remeasure(SIZE_WRAP_CONTENT, (int)(cellHeight + 0.5f));
                pCell_Wnd->GetDesiredSize(&szCell_Remeasure, szCell_Remeasure.cx, szCell_Remeasure.cy);

                // 更新列宽
                float newCellWidth = (float)szCell_Remeasure.cx / iCellColSpan;
                pCellsSize[y * nCols + x].cx = newCellWidth;
                state.UpdateColWidth(x);
            }
        }
    }

    // 重新计算总尺寸（在第三遍重新测量后）
    totalWidth = 0.0f;
    totalHeight = 0.0f;
    for (int x = 0; x < nCols; x++)
        totalWidth += pColsWidth[x];
    for (int y = 0; y < nRows; y++)
        totalHeight += pRowsHeight[y];
    totalWidth += xInter * (nCols - 1);
    totalHeight += yInter * (nRows - 1);

    // 确保结果不超过约束
    if (nWidth > 0)
        totalWidth = smin(totalWidth, nWidth);
    if (nHeight > 0)
        totalHeight = smin(totalHeight, nHeight);

    return CSize((int)(totalWidth + 0.5f), (int)(totalHeight + 0.5f));
}

void SGridLayout::LayoutChildren(IWindow *pParent)
{
    if (m_nCols == -1 && m_nRows == -1)
        return;

    int nCols = m_nCols;
    int nRows = m_nRows;

    int nCells = CalcCells(pParent);
    if (nRows == -1)
        nRows = (nCells + nCols - 1) / nCols;
    else if (nCols == -1)
        nCols = (nCells + nRows - 1) / nRows;

    CRect rcParent;
    pParent->GetChildrenLayoutRect(&rcParent);

    int xInter = m_xInterval.toPixelSize(pParent->GetScale());
    int yInter = m_yInterval.toPixelSize(pParent->GetScale());
    if (xInter < 0)
        xInter = 0;
    if (yInter < 0)
        yInter = 0;

    // 创建并初始化状态对象
    GridLayoutState state(nCols, nRows);

    // 创建别名以保持代码兼容性
    bool *&pCellsOccupy = state.pCellsOccupy;
    IWindow **&pCellsChild = state.pCellsChild;
    SGridLayoutParam **&pCellsParam = state.pCellsParam;
    int *&pCellsColSpan = state.pCellsColSpan;
    int *&pCellsRowSpan = state.pCellsRowSpan;
    float *&pRowsHeight = state.pRowsHeight;
    float *&pColsWidth = state.pColsWidth;
    float *&pRowsWeight = state.pRowsWeight;
    float *&pColsWeight = state.pColsWeight;
    bool *&pCols_WeightAllocated = state.pCols_WeightAllocated;
    bool *&pRows_WeightAllocated = state.pRows_WeightAllocated;
    CSize *&pCellsSize = state.pCellsSize;

    // 第一遍：测量并记录所有子元素
    int iRow = 0, iCol = 0;
    IWindow *pCell = pParent->GetNextLayoutIChild(NULL);

    while (pCell)
    {
        SGridLayoutParam *pLayoutParam = (SGridLayoutParam *)pCell->GetLayoutParam();
        SASSERT(pLayoutParam);

        int colSpan = pLayoutParam->nColSpan;
        int rowSpan = pLayoutParam->nRowSpan;

        colSpan = smin(colSpan, nCols - iCol);
        rowSpan = smin(rowSpan, nRows - iRow);
        SASSERT(colSpan >= 1);
        SASSERT(rowSpan >= 1);

        // 找到下一个可用位置
        while (iRow < nRows && pCellsOccupy[iRow * nCols + iCol])
        {
            if (m_orientation == Horz)
            {
                iCol++;
                if (iCol >= nCols)
                {
                    iCol = 0;
                    iRow++;
                }
            }
            else // Vert
            {
                iRow++;
                if (iRow >= nRows)
                {
                    iRow = 0;
                    iCol++;
                }
            }
        }

        if (iRow >= nRows || iCol >= nCols)
            break;

        // 调整span
        colSpan = smin(colSpan, nCols - iCol);
        rowSpan = smin(rowSpan, nRows - iRow);

        // 检查冲突
        for (int y = 0; y < rowSpan; y++)
            for (int x = 0; x < colSpan; x++)
            {
                int iCell = (iRow + y) * nCols + iCol + x;
                if (pCellsOccupy[iCell])
                {
                    rowSpan = y + 1;
                    if (y == 0)
                        colSpan = x + 1;
                    break;
                }
            }

        // 测量子元素，应用单元格约束
        CSize szCell(SIZE_WRAP_CONTENT, SIZE_WRAP_CONTENT);

        int nScale = ((IWindow *)pCell)->GetScale();

        // 应用width约束
        if (pLayoutParam->width.isSpecifiedSize())
            szCell.cx = pLayoutParam->width.toPixelSize(nScale);
        // 应用height约束
        if (pLayoutParam->height.isSpecifiedSize())
            szCell.cy = pLayoutParam->height.toPixelSize(nScale);

        pCell->GetDesiredSize(&szCell, szCell.cx, szCell.cy);

        // 记录cell信息
        pCellsChild[iRow * nCols + iCol] = (IWindow *)pCell;
        pCellsParam[iRow * nCols + iCol] = pLayoutParam;
        pCellsColSpan[iRow * nCols + iCol] = colSpan;
        pCellsRowSpan[iRow * nCols + iCol] = rowSpan;

        // 标记占用并存储大小
        for (int y = 0; y < rowSpan; y++)
            for (int x = 0; x < colSpan; x++)
            {
                int iCell = (iRow + y) * nCols + iCol + x;
                pCellsOccupy[iCell] = true;
                if (x == 0 && y == 0)
                {
                    pCellsSize[iCell] = szCell;
                }
            }

        // 更新行列尺寸和权重（使用float以保留精度）
        float cellWidth = (float)szCell.cx / colSpan;
        float cellHeight = (float)szCell.cy / rowSpan;
        float fColWeight = pLayoutParam->fColWeight / colSpan;
        float fRowWeight = pLayoutParam->fRowWeight / rowSpan;
        for (int x = 0; x < colSpan; x++)
        {
            if (pColsWidth[iCol + x] < cellWidth)
                pColsWidth[iCol + x] = cellWidth;
            if (pColsWeight[iCol + x] < fColWeight)
                pColsWeight[iCol + x] = fColWeight;
        }

        for (int y = 0; y < rowSpan; y++)
        {
            if (pRowsHeight[iRow + y] < cellHeight)
                pRowsHeight[iRow + y] = cellHeight;
            if (pRowsWeight[iRow + y] < fRowWeight)
                pRowsWeight[iRow + y] = fRowWeight;
        }

        // 移到下一个cell
        if (m_orientation == Horz)
        {
            iCol++;
            if (iCol >= nCols)
            {
                iCol = 0;
                iRow++;
            }
        }
        else // Vert
        {
            iRow++;
            if (iRow >= nRows)
            {
                iRow = 0;
                iCol++;
            }
        }

        pCell = pParent->GetNextLayoutIChild(pCell);
    }

    // 第二遍：处理权重分配
    float totalWidth = 0.0f;
    float totalHeight = 0.0f;

    for (int x = 0; x < nCols; x++)
        totalWidth += pColsWidth[x];

    for (int y = 0; y < nRows; y++)
        totalHeight += pRowsHeight[y];

    // 权重分配（水平方向）
    bool bColWeightAllocated = false; // 标记列是否进行了权重分配
    float totalColWeight = 0.0f;
    for (int i = 0; i < nCols; i++)
        totalColWeight += pColsWeight[i];

    {
        int netParentWid = rcParent.Width() - xInter * (nCols - 1);
        if (totalWidth < netParentWid && totalColWeight > 0.0f)
        {
            bColWeightAllocated = true;
            float remainSpace = netParentWid - totalWidth;
            for (int i = 0; i < nCols; i++)
            {
                if (pColsWeight[i] > 0.0f && totalColWeight > 0.0f)
                {
                    float extra = remainSpace * pColsWeight[i] / totalColWeight;
                    pColsWidth[i] += extra;
                    pCols_WeightAllocated[i] = true; // 标记该列参与了分配
                    remainSpace -= extra;
                    totalColWeight -= pColsWeight[i];
                }
            }
        }
    }

    // 权重分配（垂直方向）
    bool bRowWeightAllocated = false; // 标记行是否进行了权重分配
    float totalRowWeight = 0.0f;
    for (int i = 0; i < nRows; i++)
        totalRowWeight += pRowsWeight[i];

    {
        int netParentHei = rcParent.Height() - yInter * (nRows - 1);
        if (totalHeight < netParentHei && totalRowWeight > 0.0f)
        {
            bRowWeightAllocated = true;
            float remainSpace = netParentHei - totalHeight;
            for (int i = 0; i < nRows; i++)
            {
                if (pRowsWeight[i] > 0.0f && totalRowWeight > 0.0f)
                {
                    float extra = remainSpace * pRowsWeight[i] / totalRowWeight;
                    pRowsHeight[i] += extra;
                    pRows_WeightAllocated[i] = true; // 标记该行参与了分配
                    remainSpace -= extra;
                    totalRowWeight -= pRowsWeight[i];
                }
            }
        }
    }

    // 第三遍：对于满足条件的单元格，用分配后的尺寸重新测量
    // 条件：某方向的weight在Pass 2中参与了分配，且单元格那方向的weight不为0，另一方向是WRAP_CONTENT
    for (int y = 0; y < nRows; y++)
    {
        for (int x = 0; x < nCols; x++)
        {
            int iCell = y * nCols + x;

            // 跳过非主cell
            if (!pCellsChild[iCell])
                continue;

            const SGridLayoutParam *pCell_Param = pCellsParam[iCell];
            IWindow *pCell_Wnd = pCellsChild[iCell];
            int nScale = pCell_Wnd->GetScale();
            int iCellColSpan = pCellsColSpan[iCell];
            int iCellRowSpan = pCellsRowSpan[iCell];

            // 检查列的weight是否被分配了，且cell的colWeight不为0，height是WRAP_CONTENT
            bool bColWeightMatched = false;
            if (bColWeightAllocated && pCell_Param->fColWeight > 0.0f && pCell_Param->height.isWrapContent())
            {
                // 检查该cell所在列是否参与了weight分配
                for (int xx = 0; xx < iCellColSpan; xx++)
                {
                    if (pCols_WeightAllocated[x + xx])
                    {
                        bColWeightMatched = true;
                        break;
                    }
                }
            }

            // 检查行的weight是否被分配了，且cell的rowWeight不为0，width是WRAP_CONTENT
            bool bRowWeightMatched = false;
            if (bRowWeightAllocated && pCell_Param->fRowWeight > 0.0f && pCell_Param->width.isWrapContent())
            {
                // 检查该cell所在行是否参与了weight分配
                for (int yy = 0; yy < iCellRowSpan; yy++)
                {
                    if (pRows_WeightAllocated[y + yy])
                    {
                        bRowWeightMatched = true;
                        break;
                    }
                }
            }

            // 如果列权重被分配了，用分配的列宽重新测量高度
            if (bColWeightMatched)
            {
                // 计算这个cell分配到的总宽度
                float cellWidth = 0;
                for (int xx = 0; xx < iCellColSpan; xx++)
                    cellWidth += pColsWidth[x + xx];
                cellWidth += xInter * (iCellColSpan - 1);

                // 用新的宽度重新测量高度
                CSize szCell_Remeasure((int)(cellWidth + 0.5f), SIZE_WRAP_CONTENT);
                pCell_Wnd->GetDesiredSize(&szCell_Remeasure, szCell_Remeasure.cx, szCell_Remeasure.cy);

                // 更新行高
                float newCellHeight = (float)szCell_Remeasure.cy / iCellRowSpan;
                pCellsSize[y * nCols + x].cy = newCellHeight;
                state.UpdateRowHeight(y);
            }
            // 如果行权重被分配了，用分配的行高重新测量宽度
            else if (bRowWeightMatched)
            {
                // 计算这个cell分配到的总高度
                float cellHeight = 0;
                for (int yy = 0; yy < iCellRowSpan; yy++)
                    cellHeight += pRowsHeight[y + yy];
                cellHeight += yInter * (iCellRowSpan - 1);

                // 用新的高度重新测量宽度
                CSize szCell_Remeasure(SIZE_WRAP_CONTENT, (int)(cellHeight + 0.5f));
                pCell_Wnd->GetDesiredSize(&szCell_Remeasure, szCell_Remeasure.cx, szCell_Remeasure.cy);

                // 更新列宽
                float newCellWidth = (float)szCell_Remeasure.cx / iCellColSpan;
                pCellsSize[y * nCols + x].cx = newCellWidth;
                state.UpdateColWidth(x);
            }
        }
    }

    // 第四遍：计算和应用子窗口位置
    CPoint pt = rcParent.TopLeft();

    for (int y = 0; y < nRows; y++)
    {
        for (int x = 0; x < nCols; x++)
        {
            int iCell = y * nCols + x;

            // 跳过空cell或非主cell
            if (!pCellsChild[iCell] || pCellsColSpan[iCell] == 0)
            {
                pt.x += (int)(pColsWidth[x] + 0.5f) + xInter;
                continue;
            }

            IWindow *pCellWnd = pCellsChild[iCell];
            SGridLayoutParam *pLayoutParam = pCellsParam[iCell];
            int colSpan = pCellsColSpan[iCell];
            int rowSpan = pCellsRowSpan[iCell];

            // 计算cell的总宽度和高度（从float转换为int）
            float szCellWidthF = 0.0f;
            float szCellHeightF = 0.0f;

            for (int xx = 0; xx < colSpan; xx++)
                szCellWidthF += pColsWidth[x + xx];

            for (int yy = 0; yy < rowSpan; yy++)
                szCellHeightF += pRowsHeight[y + yy];

            int szCellWidth = (int)(szCellWidthF + 0.5f) + xInter * (colSpan - 1);
            int szCellHeight = (int)(szCellHeightF + 0.5f) + yInter * (rowSpan - 1);

            // 获取子元素的期望尺寸
            CSize szDesired = *(pCellsSize + y * nCols + x);
            // 应用对齐方式
            CPoint pt2 = pt;

            GridGravity gx = pLayoutParam->layoutGravityX;
            if (gx == gUndef)
                gx = m_GravityX;

            switch (gx)
            {
            case gUndef:
            case gLeft:
                break;
            case gCenter:
                pt2.x += (szCellWidth - szDesired.cx) / 2;
                break;
            case gRight:
                pt2.x += (szCellWidth - szDesired.cx);
                break;
            case gFill:
                szDesired.cx = szCellWidth;
                break;
            }

            GridGravity gy = pLayoutParam->layoutGravityY;
            if (gy == gUndef)
                gy = m_GravityY;

            switch (gy)
            {
            case gUndef:
            case gTop:
                break;
            case gCenter:
                pt2.y += (szCellHeight - szDesired.cy) / 2;
                break;
            case gBottom:
                pt2.y += (szCellHeight - szDesired.cy);
                break;
            case gFill:
                szDesired.cy = szCellHeight;
                break;
            }

            CRect rcCell(pt2, szDesired);
            ((SWindow *)pCellWnd)->OnRelayout(rcCell);

            pt.x += (int)(pColsWidth[x] + 0.5f) + xInter;
        }

        pt.x = rcParent.left;
        pt.y += (int)(pRowsHeight[y] + 0.5f) + yInter;
    }
}

/**
 * 辅助方法：根据排列方向查找下一个可用的网格位置
 * @param pCellsOccupy 网格占用标记数组
 * @param nCols 列数
 * @param nRows 行数
 * @param iRow 当前行（输入/输出）
 * @param iCol 当前列（输入/输出）
 * @param orientation 排列方向
 * @return 是否找到可用位置
 */
static BOOL FindNextAvailableCell(bool *pCellsOccupy, int nCols, int nRows, int &iRow, int &iCol, ORIENTATION orientation)
{
    if (orientation == Horz)
    {
        // 水平排列：先在当前行查找，再到下面行从0开始查找
        bool bFind = false;
        for (int x = iCol + 1; x < nCols; x++)
        {
            if (!pCellsOccupy[iRow * nCols + x])
            {
                bFind = true;
                iCol = x;
                return TRUE;
            }
        }
        for (int y = iRow + 1; y < nRows && !bFind; y++)
            for (int x = 0; x < nCols; x++)
            {
                if (!pCellsOccupy[y * nCols + x])
                {
                    iRow = y;
                    iCol = x;
                    return TRUE;
                }
            }
        return FALSE;
    }
    else // GRID_LAYOUT_ORIENTATION_VERTICAL
    {
        // 垂直排列：先在当前列查找，再到右面列从0开始查找
        bool bFind = false;
        for (int y = iRow + 1; y < nRows; y++)
        {
            if (!pCellsOccupy[y * nCols + iCol])
            {
                bFind = true;
                iRow = y;
                return TRUE;
            }
        }
        for (int x = iCol + 1; x < nCols && !bFind; x++)
            for (int y = 0; y < nRows; y++)
            {
                if (!pCellsOccupy[y * nCols + x])
                {
                    iRow = y;
                    iCol = x;
                    return TRUE;
                }
            }
        return FALSE;
    }
}

int SGridLayout::CalcCells(const IWindow *pParent) const
{
    int nCells = 0;
    const IWindow *pCell = pParent->GetNextLayoutIChild(NULL);
    while (pCell)
    {
        const SGridLayoutParam *pParam = (const SGridLayoutParam *)pCell->GetLayoutParam();
        SASSERT(pParam);
        int nColSpan = pParam->nColSpan;
        int nRowSpan = pParam->nRowSpan;
        nCells += nColSpan * nRowSpan;
        pCell = pParent->GetNextLayoutIChild(pCell);
    }
    return nCells;
}

SNSEND
