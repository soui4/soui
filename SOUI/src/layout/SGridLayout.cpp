#include "souistd.h"
#include "layout/SGridLayout.h"
#include <core/SWnd.h>
SNSBEGIN

/**
 * @class GridLayoutState
 * @brief State object for grid layout, used to store temporary state variables during measurement/layout
 */
class GridLayoutState {
  public:
    /** Cell info */
    CSize *pCellsSize;              /**< Size of each cell */
    bool *pCellsOccupy;             /**< Cell occupancy flag */
    SGridLayoutParam **pCellsParam; /**< Cell layout params */
    IWindow **pCellsChild;          /**< Cell window pointer */
    int *pCellsColSpan;             /**< Cell column span */
    int *pCellsRowSpan;             /**< Cell row span */

    /** Row/column info */
    float *pRowsHeight;          /**< Height of each row */
    float *pColsWidth;           /**< Width of each column */
    float *pRowsWeight;          /**< Weight of each row */
    float *pColsWeight;          /**< Weight of each column */
    bool *pCols_WeightAllocated; /**< Whether column weight has been allocated */
    bool *pRows_WeightAllocated; /**< Whether row weight has been allocated */

    int nCols; /**< Column count */
    int nRows; /**< Row count */

    /**
     * @brief Allocate memory for the state object
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
     * @brief Initialize all fields of the state object
     */
    void Initialize()
    {
        // Initialize cell array
        for (int i = 0; i < nCols * nRows; i++)
        {
            pCellsOccupy[i] = false;
            pCellsSize[i] = CSize(0, 0);
            pCellsParam[i] = NULL;
            pCellsChild[i] = NULL;
            pCellsColSpan[i] = 0;
            pCellsRowSpan[i] = 0;
        }

        // Initialize row array
        for (int i = 0; i < nRows; i++)
        {
            pRowsHeight[i] = 0;
            pRowsWeight[i] = 0.0f;
            pRows_WeightAllocated[i] = false;
        }

        // Initialize column array
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
    memcpy(pRet->GetRawData(), (void *)(SGridLayoutParamStruct *)this, sizeof(SGridLayoutParamStruct));
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
///////////////////////////////////////////////////////////////////////
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
 * MeasureChildren computes the size of gridlayout's child windows
 * Following Android GridLayout's design, uses a two-pass measurement approach
 *
 * Pass 1: measure the natural size of all children (wrap_content)
 * Pass 2 (optional): distribute remaining space according to weights
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

    // Create and initialize the state object
    GridLayoutState state(nCols, nRows);

    // Create an alias to maintain code compatibility
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

    // Pass 1: measure all children without constraints
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

        // Find the next available position
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

        // Adjust span to ensure it stays within bounds
        colSpan = smin(colSpan, nCols - iCol);
        rowSpan = smin(rowSpan, nRows - iRow);

        // Check for span conflicts; if conflict, reduce span
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

        // Measure child, apply cell constraints
        CSize szCell(SIZE_WRAP_CONTENT, SIZE_WRAP_CONTENT);
        int nScale = ((IWindow *)pCell)->GetScale();

        // Apply width/height constraints
        if (pLayoutParam->width.isSpecifiedSize())
            szCell.cx = pLayoutParam->width.toPixelSize(nScale);
        if (pLayoutParam->height.isSpecifiedSize())
            szCell.cy = pLayoutParam->height.toPixelSize(nScale);

        ((IWindow *)pCell)->GetDesiredSize(&szCell, szCell.cx, szCell.cy);
        // Record cell info
        pCellsChild[iRow * nCols + iCol] = (IWindow *)pCell;
        pCellsParam[iRow * nCols + iCol] = (SGridLayoutParam *)pLayoutParam;
        pCellsColSpan[iRow * nCols + iCol] = colSpan;
        pCellsRowSpan[iRow * nCols + iCol] = rowSpan;

        // Mark occupancy and store size
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

        // Update row/column max size and weight (use float to preserve precision)
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

        // Move to the next cell
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

    // Pass 2: calculate total size
    float totalWidth = 0.0f;
    float totalHeight = 0.0f;

    for (int x = 0; x < nCols; x++)
        totalWidth += pColsWidth[x];

    for (int y = 0; y < nRows; y++)
        totalHeight += pRowsHeight[y];

    totalWidth += xInter * (nCols - 1);
    totalHeight += yInter * (nRows - 1);

    // Handle weight distribution (if parent container has constraints)
    bool bColWeightAllocated = false; // Mark whether the column had weight distribution
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
                    pCols_WeightAllocated[i] = true; // Mark this column as participating in distribution
                    remainSpace -= extra;
                    totalColWeight -= pColsWeight[i];
                }
            }

            // Recalculate total width
            totalWidth = 0.0f;
            for (int x = 0; x < nCols; x++)
                totalWidth += pColsWidth[x];
            totalWidth += xInter * (nCols - 1);
        }
    }

    bool bRowWeightAllocated = false; // Mark whether the row had weight distribution

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
                    pRows_WeightAllocated[i] = true; // Mark this row as participating in distribution
                    totalRowWeight -= pRowsWeight[i];
                }
            }

            // Recalculate total height
            totalHeight = 0;
            for (int y = 0; y < nRows; y++)
                totalHeight += pRowsHeight[y];
            totalHeight += yInter * (nRows - 1);
        }
    }

    // Pass 3: re-measure cells that meet the condition using the distributed size
    // Condition: the weight in one direction was distributed in Pass 2, the cell's weight in that direction is non-zero, and the other direction is WRAP_CONTENT
    for (int y = 0; y < nRows; y++)
    {
        for (int x = 0; x < nCols; x++)
        {
            int iCell = y * nCols + x;

            // Skip non-primary cells
            if (!pCellsChild[iCell])
                continue;

            const SGridLayoutParam *pCell_Param = pCellsParam[iCell];
            IWindow *pCell_Wnd = pCellsChild[iCell];
            int nScale = pCell_Wnd->GetScale();
            int iCellColSpan = pCellsColSpan[iCell];
            int iCellRowSpan = pCellsRowSpan[iCell];

            // Check whether the column's weight was distributed, the cell's colWeight is non-zero, and height is WRAP_CONTENT
            bool bColWeightMatched = false;
            if (bColWeightAllocated && pCell_Param->fColWeight > 0.0f && pCell_Param->height.isWrapContent())
            {
                // Check whether the column containing this cell participated in weight distribution
                for (int xx = 0; xx < iCellColSpan; xx++)
                {
                    if (pCols_WeightAllocated[x + xx])
                    {
                        bColWeightMatched = true;
                        break;
                    }
                }
            }

            // Check whether the row's weight was distributed, the cell's rowWeight is non-zero, and width is WRAP_CONTENT
            bool bRowWeightMatched = false;
            if (bRowWeightAllocated && pCell_Param->fRowWeight > 0.0f && pCell_Param->width.isWrapContent())
            {
                // Check whether the row containing this cell participated in weight distribution
                for (int yy = 0; yy < iCellRowSpan; yy++)
                {
                    if (pRows_WeightAllocated[y + yy])
                    {
                        bRowWeightMatched = true;
                        break;
                    }
                }
            }

            // If column weight was distributed, re-measure height using the distributed column width
            if (bColWeightMatched)
            {
                // Calculate the total width allocated to this cell
                float cellWidth = 0;
                for (int xx = 0; xx < iCellColSpan; xx++)
                    cellWidth += pColsWidth[x + xx];
                cellWidth += xInter * (iCellColSpan - 1);

                // Re-measure height using the new width
                CSize szCell_Remeasure((int)(cellWidth + 0.5f), SIZE_WRAP_CONTENT);
                pCell_Wnd->GetDesiredSize(&szCell_Remeasure, szCell_Remeasure.cx, szCell_Remeasure.cy);

                // Update row height
                float newCellHeight = (float)szCell_Remeasure.cy / iCellRowSpan;
                pCellsSize[y * nCols + x].cy = (LONG)newCellHeight;
                state.UpdateRowHeight(y);
            }
            // If row weight was distributed, re-measure width using the distributed row height
            else if (bRowWeightMatched)
            {
                // Calculate the total height allocated to this cell
                float cellHeight = 0;
                for (int yy = 0; yy < iCellRowSpan; yy++)
                    cellHeight += pRowsHeight[y + yy];
                cellHeight += yInter * (iCellRowSpan - 1);

                // Re-measure width using the new height
                CSize szCell_Remeasure(SIZE_WRAP_CONTENT, (int)(cellHeight + 0.5f));
                pCell_Wnd->GetDesiredSize(&szCell_Remeasure, szCell_Remeasure.cx, szCell_Remeasure.cy);

                // Update column width
                float newCellWidth = (float)szCell_Remeasure.cx / iCellColSpan;
                pCellsSize[y * nCols + x].cx = newCellWidth;
                state.UpdateColWidth(x);
            }
        }
    }

    // Recalculate total size (after pass 3 re-measurement)
    totalWidth = 0.0f;
    totalHeight = 0.0f;
    for (int x = 0; x < nCols; x++)
        totalWidth += pColsWidth[x];
    for (int y = 0; y < nRows; y++)
        totalHeight += pRowsHeight[y];
    totalWidth += xInter * (nCols - 1);
    totalHeight += yInter * (nRows - 1);

    // Ensure result does not exceed constraints
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

    // Create and initialize the state object
    GridLayoutState state(nCols, nRows);

    // Create an alias to maintain code compatibility
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

    // Pass 1: measure and record all children
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

        // Find the next available position
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

        // Adjust span
        colSpan = smin(colSpan, nCols - iCol);
        rowSpan = smin(rowSpan, nRows - iRow);

        // Check conflicts
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

        // Measure child, apply cell constraints
        CSize szCell(SIZE_WRAP_CONTENT, SIZE_WRAP_CONTENT);

        int nScale = ((IWindow *)pCell)->GetScale();

        // Apply width constraint
        if (pLayoutParam->width.isSpecifiedSize())
            szCell.cx = pLayoutParam->width.toPixelSize(nScale);
        // Apply height constraint
        if (pLayoutParam->height.isSpecifiedSize())
            szCell.cy = pLayoutParam->height.toPixelSize(nScale);

        pCell->GetDesiredSize(&szCell, szCell.cx, szCell.cy);

        // Record cell info
        pCellsChild[iRow * nCols + iCol] = (IWindow *)pCell;
        pCellsParam[iRow * nCols + iCol] = pLayoutParam;
        pCellsColSpan[iRow * nCols + iCol] = colSpan;
        pCellsRowSpan[iRow * nCols + iCol] = rowSpan;

        // Mark occupancy and store size
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

        // Update row/column size and weight (use float to preserve precision)
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

        // Move to the next cell
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

    // Pass 2: handle weight distribution
    float totalWidth = 0.0f;
    float totalHeight = 0.0f;

    for (int x = 0; x < nCols; x++)
        totalWidth += pColsWidth[x];

    for (int y = 0; y < nRows; y++)
        totalHeight += pRowsHeight[y];

    // Weight distribution (horizontal)
    bool bColWeightAllocated = false; // Mark whether the column had weight distribution
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
                    pCols_WeightAllocated[i] = true; // Mark this column as participating in distribution
                    remainSpace -= extra;
                    totalColWeight -= pColsWeight[i];
                }
            }
        }
    }

    // Weight distribution (vertical)
    bool bRowWeightAllocated = false; // Mark whether the row had weight distribution
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
                    pRows_WeightAllocated[i] = true; // Mark this row as participating in distribution
                    remainSpace -= extra;
                    totalRowWeight -= pRowsWeight[i];
                }
            }
        }
    }

    // Pass 3: re-measure cells that meet the condition using the distributed size
    // Condition: the weight in one direction was distributed in Pass 2, the cell's weight in that direction is non-zero, and the other direction is WRAP_CONTENT
    for (int y = 0; y < nRows; y++)
    {
        for (int x = 0; x < nCols; x++)
        {
            int iCell = y * nCols + x;

            // Skip non-primary cells
            if (!pCellsChild[iCell])
                continue;

            const SGridLayoutParam *pCell_Param = pCellsParam[iCell];
            IWindow *pCell_Wnd = pCellsChild[iCell];
            int nScale = pCell_Wnd->GetScale();
            int iCellColSpan = pCellsColSpan[iCell];
            int iCellRowSpan = pCellsRowSpan[iCell];

            // Check whether the column's weight was distributed, the cell's colWeight is non-zero, and height is WRAP_CONTENT
            bool bColWeightMatched = false;
            if (bColWeightAllocated && pCell_Param->fColWeight > 0.0f && pCell_Param->height.isWrapContent())
            {
                // Check whether the column containing this cell participated in weight distribution
                for (int xx = 0; xx < iCellColSpan; xx++)
                {
                    if (pCols_WeightAllocated[x + xx])
                    {
                        bColWeightMatched = true;
                        break;
                    }
                }
            }

            // Check whether the row's weight was distributed, the cell's rowWeight is non-zero, and width is WRAP_CONTENT
            bool bRowWeightMatched = false;
            if (bRowWeightAllocated && pCell_Param->fRowWeight > 0.0f && pCell_Param->width.isWrapContent())
            {
                // Check whether the row containing this cell participated in weight distribution
                for (int yy = 0; yy < iCellRowSpan; yy++)
                {
                    if (pRows_WeightAllocated[y + yy])
                    {
                        bRowWeightMatched = true;
                        break;
                    }
                }
            }

            // If column weight was distributed, re-measure height using the distributed column width
            if (bColWeightMatched && pRowsWeight[y] == 0.f)
            {
                // Calculate the total width allocated to this cell
                float cellWidth = 0;
                for (int xx = 0; xx < iCellColSpan; xx++)
                    cellWidth += pColsWidth[x + xx];
                cellWidth += xInter * (iCellColSpan - 1);

                // Re-measure height using the new width
                CSize szCell_Remeasure((int)(cellWidth + 0.5f), SIZE_WRAP_CONTENT);
                pCell_Wnd->GetDesiredSize(&szCell_Remeasure, szCell_Remeasure.cx, szCell_Remeasure.cy);

                // Update row height
                float newCellHeight = (float)szCell_Remeasure.cy / iCellRowSpan;
                pCellsSize[y * nCols + x].cy = (LONG)newCellHeight;
                state.UpdateRowHeight(y);
            }
            // If row weight was distributed, re-measure width using the distributed row height
            else if (bRowWeightMatched && pColsWeight[x] == 0.f)
            {
                // Calculate the total height allocated to this cell
                float cellHeight = 0;
                for (int yy = 0; yy < iCellRowSpan; yy++)
                    cellHeight += pRowsHeight[y + yy];
                cellHeight += yInter * (iCellRowSpan - 1);

                // Re-measure width using the new height
                CSize szCell_Remeasure(SIZE_WRAP_CONTENT, (int)(cellHeight + 0.5f));
                pCell_Wnd->GetDesiredSize(&szCell_Remeasure, szCell_Remeasure.cx, szCell_Remeasure.cy);

                // Update column width
                float newCellWidth = (float)szCell_Remeasure.cx / iCellColSpan;
                pCellsSize[y * nCols + x].cx = newCellWidth;
                state.UpdateColWidth(x);
            }
        }
    }

    // Pass 4: calculate and apply child window positions
    CPoint pt = rcParent.TopLeft();

    for (int y = 0; y < nRows; y++)
    {
        for (int x = 0; x < nCols; x++)
        {
            int iCell = y * nCols + x;

            // Skip empty cells or non-primary cells
            if (!pCellsChild[iCell] || pCellsColSpan[iCell] == 0)
            {
                pt.x += (int)(pColsWidth[x] + 0.5f) + xInter;
                continue;
            }

            IWindow *pCellWnd = pCellsChild[iCell];
            SGridLayoutParam *pLayoutParam = pCellsParam[iCell];
            int colSpan = pCellsColSpan[iCell];
            int rowSpan = pCellsRowSpan[iCell];

            // Calculate cell's total width and height (convert from float to int)
            float szCellWidthF = 0.0f;
            float szCellHeightF = 0.0f;

            for (int xx = 0; xx < colSpan; xx++)
                szCellWidthF += pColsWidth[x + xx];

            for (int yy = 0; yy < rowSpan; yy++)
                szCellHeightF += pRowsHeight[y + yy];

            int szCellWidth = (int)(szCellWidthF + 0.5f) + xInter * (colSpan - 1);
            int szCellHeight = (int)(szCellHeightF + 0.5f) + yInter * (rowSpan - 1);

            // Get child's desired size
            CSize szDesired = *(pCellsSize + y * nCols + x);
            // Apply alignment
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
 * Helper: find the next available grid position based on orientation
 * @param pCellsOccupy Grid occupancy flag array
 * @param nCols Number of columns
 * @param nRows Number of rows
 * @param iRow Current row (input/output)
 * @param iCol Current column (input/output)
 * @param orientation Orientation
 * @return Whether an available position was found
 */
static BOOL FindNextAvailableCell(bool *pCellsOccupy, int nCols, int nRows, int &iRow, int &iCol, ORIENTATION orientation)
{
    if (orientation == Horz)
    {
        // Horizontal orientation: search in the current row first, then from 0 in the rows below
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
        // Vertical orientation: search in the current column first, then from 0 in the columns to the right
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
