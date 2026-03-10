#include "souistd.h"
#include "layout/SFrameLayout.h"
#include <core/SWnd.h>
SNSBEGIN

DockPosition SFrameLayoutParam::parseDockPosition(const SStringW &strValue)
{
    struct ValueMap
    {
        DockPosition dockPos;
        LPCWSTR pszDock;
    } map[] = {
        { DockNone, L"none" }, { DockLeft, L"left" }, { DockTop, L"top" }, { DockRight, L"right" }, { DockBottom, L"bottom" }, { DockMainView, L"mainview" }, { DockMainView, L"main" },
    };

    for (int i = 0; i < ARRAYSIZE(map); i++)
    {
        if (strValue.CompareNoCase(map[i].pszDock) == 0)
        {
            return map[i].dockPos;
        }
    }
    return DockNone;
}

HRESULT SFrameLayoutParam::OnAttrSize(const SStringW &strValue, BOOL bLoading)
{
    SStringWList szStr;
    if (2 != SplitString(strValue, L',', szStr))
        return E_FAIL;

    width = GETLAYOUTSIZE(szStr[0]);
    height = GETLAYOUTSIZE(szStr[1]);
    return S_OK;
}

HRESULT SFrameLayoutParam::OnAttrDockPosition(const SStringW &strValue, BOOL bLoading)
{
    dockPos = parseDockPosition(strValue);
    return S_OK;
}

SFrameLayoutParam::SFrameLayoutParam()
{
    Clear();
}

void SFrameLayoutParam::Clear()
{
    width.setWrapContent();
    height.setWrapContent();
    dockPos = DockNone;
    strDockRelativeTo.Empty();
    extend_left.setSize(0, px);
    extend_right.setSize(0, px);
    extend_top.setSize(0, px);
    extend_bottom.setSize(0, px);
    weight = 0.0f;
    gravity = G_Left;
}

void SFrameLayoutParam::SetMatchParent(ORIENTATION orientation)
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

void SFrameLayoutParam::SetWrapContent(ORIENTATION orientation)
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

void SFrameLayoutParam::SetSpecifiedSize(ORIENTATION orientation, const LAYOUTSIZE *pLayoutSize)
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

BOOL SFrameLayoutParam::IsMatchParent(ORIENTATION orientation) const
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

BOOL SFrameLayoutParam::IsWrapContent(ORIENTATION orientation) const
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

BOOL SFrameLayoutParam::IsSpecifiedSize(ORIENTATION orientation) const
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

BOOL SFrameLayoutParam::GetSpecifiedSize(ORIENTATION orientation, LAYOUTSIZE *pLayoutSize) const
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

void *SFrameLayoutParam::GetRawData()
{
    return (SFrameLayoutParamStruct *)this;
}

ILayoutParam *SFrameLayoutParam::Clone() const
{
    SFrameLayoutParam *pRet = new SFrameLayoutParam();
    memcpy(pRet->GetRawData(), (SFrameLayoutParamStruct *)this, sizeof(SFrameLayoutParamStruct));
    return pRet;
}

BOOL SFrameLayoutParam::SetAnimatorValue(IPropertyValuesHolder *pHolder, float fraction, ANI_STATE state)
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

SFrameLayout::SFrameLayout(void)
    : m_enableDockMode(DockMode_Any)
{
}

SFrameLayout::~SFrameLayout(void)
{
}

void SFrameLayout::SetEnableDockMode(DockMode mode)
{
    m_enableDockMode = mode;
}

DockMode SFrameLayout::GetEnableDockMode() const
{
    return m_enableDockMode;
}

DockMode SFrameLayout::parseDockMode(const SStringW &strValue)
{
    struct ValueMap
    {
        DockMode dockMode;
        LPCWSTR pszMode;
    } map[] = {
        { DockMode_None, L"none" }, { DockMode_Left, L"left" }, { DockMode_Top, L"top" }, { DockMode_Right, L"right" }, { DockMode_Bottom, L"bottom" }, { DockMode_Any, L"any" },
    };

    for (int i = 0; i < ARRAYSIZE(map); i++)
    {
        if (strValue.CompareNoCase(map[i].pszMode) == 0)
        {
            return map[i].dockMode;
        }
    }
    return DockMode_Any;
}
HRESULT SFrameLayout::OnAttrEnableDockMode(const SStringW &strValue, BOOL bLoading)
{
    m_enableDockMode = parseDockMode(strValue);
    return S_OK;
}

BOOL SFrameLayout::IsParamAcceptable(const ILayoutParam *pLayoutParam) const
{
    return pLayoutParam->IsClass(SFrameLayoutParam::GetClassName());
}

ILayoutParam *SFrameLayout::CreateLayoutParam() const
{
    return new SFrameLayoutParam();
}

void SFrameLayout::CollectChildren(const IWindow *pParent, SList<ChildInfo> &lstChildren) const
{
    const IWindow *pChild = pParent->GetNextLayoutIChild(NULL);
    while (pChild)
    {
        if (pChild->IsVisible(TRUE))
        {
            ChildInfo info;
            info.pWnd = (IWindow *)pChild;
            info.pParam = (SFrameLayoutParam *)pChild->GetLayoutParam();

            // 检查子dockbar的dockPos是否与父布局的enableDockMode匹配
            bool bDockModeMatch = false;
            switch (info.pParam->dockPos)
            {
            case DockLeft:
                bDockModeMatch = (m_enableDockMode & DockMode_Left);
                break;
            case DockTop:
                bDockModeMatch = (m_enableDockMode & DockMode_Top);
                break;
            case DockRight:
                bDockModeMatch = (m_enableDockMode & DockMode_Right);
                break;
            case DockBottom:
                bDockModeMatch = (m_enableDockMode & DockMode_Bottom);
                break;
            case DockMainView:
            case DockNone:
                bDockModeMatch = true; // mainview和none模式不受dockMode限制
                break;
            }

            if (bDockModeMatch)
            {
                lstChildren.AddTail(info);
            }
        }
        pChild = pParent->GetNextLayoutIChild(pChild);
    }
}

void SFrameLayout::CalcExtendRect(const ChildInfo &info, CRect &rcExtend, int nScale) const
{
    rcExtend.left = info.pParam->extend_left.toPixelSize(nScale);
    rcExtend.top = info.pParam->extend_top.toPixelSize(nScale);
    rcExtend.right = info.pParam->extend_right.toPixelSize(nScale);
    rcExtend.bottom = info.pParam->extend_bottom.toPixelSize(nScale);
}

CSize SFrameLayout::MeasureChild(const ChildInfo &info, int nWidth, int nHeight) const
{
    CSize szDesired(nWidth, nHeight);

    int nScale = info.pWnd->GetScale();

    if (info.pParam->width.isSpecifiedSize())
        szDesired.cx = info.pParam->width.toPixelSize(nScale);
    if (info.pParam->height.isSpecifiedSize())
        szDesired.cy = info.pParam->height.toPixelSize(nScale);

    info.pWnd->GetDesiredSize(&szDesired, szDesired.cx, szDesired.cy);

    return szDesired;
}

SIZE SFrameLayout::MeasureChildren(const IWindow *pParent, int nWidth, int nHeight) const
{
    SList<ChildInfo> lstChildren;
    CollectChildren(pParent, lstChildren);

    int nTotalWidth = 0;
    int nTotalHeight = 0;

    SPOSITION pos = lstChildren.GetHeadPosition();
    while (pos)
    {
        ChildInfo &info = lstChildren.GetNext(pos);
        if (!info.pWnd->IsVisible(TRUE))
            continue;

        CSize sz = MeasureChild(info, SIZE_WRAP_CONTENT, SIZE_WRAP_CONTENT);

        if (info.pParam->dockPos == DockMainView)
        {
            nTotalWidth = smax(nTotalWidth, sz.cx);
            nTotalHeight = smax(nTotalHeight, sz.cy);
        }
        else
        {
            if (info.pParam->dockPos == DockLeft || info.pParam->dockPos == DockRight)
            {
                nTotalWidth += sz.cx;
                nTotalHeight = smax(nTotalHeight, sz.cy);
            }
            else if (info.pParam->dockPos == DockTop || info.pParam->dockPos == DockBottom)
            {
                nTotalHeight += sz.cy;
                nTotalWidth = smax(nTotalWidth, sz.cx);
            }
        }
    }

    return CSize(nTotalWidth, nTotalHeight);
}

void SFrameLayout::LayoutDockTopBottom(IWindow *pParent, SList<ChildInfo *> &lstChildren, CRect &rcAvailable, BOOL bIsTop, const CRect &rcParent)
{
    int nTotalHeight = 0;
    SList<ChildInfo *> lstOrdered;

    // 处理相对停靠关系
    SList<ChildInfo *> lstRemaining = lstChildren;
    while (!lstRemaining.IsEmpty())
    {
        SPOSITION pos = lstRemaining.GetHeadPosition();
        while (pos)
        {
            SPOSITION posCurrent = pos;
            ChildInfo *pInfo = lstRemaining.GetNext(pos);

            if (pInfo->pParam->strDockRelativeTo.IsEmpty())
            {
                lstOrdered.AddTail(pInfo);
                lstRemaining.RemoveAt(posCurrent);
            }
            else
            {
                BOOL bRelativeFound = FALSE;
                SPOSITION posOrdered = lstOrdered.GetHeadPosition();
                while (posOrdered)
                {
                    ChildInfo *pOrderedInfo = lstOrdered.GetNext(posOrdered);
                    bool bMatch = pInfo->pParam->strDockRelativeTo.CompareNoCase(pOrderedInfo->pWnd->GetName()) == 0;
                    if (bMatch)
                    {
                        lstOrdered.InsertAfter(posOrdered, pInfo);
                        lstRemaining.RemoveAt(posCurrent);
                        bRelativeFound = TRUE;
                        break;
                    }
                }
            }
        }
    }

    // 将窗口分组到不同的行中
    // 规则：如果窗口没有dockRelativeTo，或者其dockRelativeTo引用的是前一行的最后一个窗口，则另起一行
    SList<SList<ChildInfo *>> lstRows;

    SPOSITION pos = lstOrdered.GetHeadPosition();
    while (pos)
    {
        ChildInfo *pInfo = lstOrdered.GetNext(pos);

        if (pInfo->pParam->strDockRelativeTo.IsEmpty())
        {
            // 没有dockRelativeTo，另起一行
            SList<ChildInfo *> newRow;
            newRow.AddTail(pInfo);
            lstRows.AddTail(newRow);
        }
        else
        {
            // 有dockRelativeTo，找到引用的窗口，将当前窗口添加到同一行
            BOOL bAdded = FALSE;
            SPOSITION posRow = lstRows.GetHeadPosition();
            while (posRow)
            {
                SList<ChildInfo *> &row = lstRows.GetNext(posRow);
                SPOSITION posRowChild = row.GetHeadPosition();
                while (posRowChild)
                {
                    ChildInfo *pRowChild = row.GetNext(posRowChild);
                    if (pInfo->pParam->strDockRelativeTo.CompareNoCase(pRowChild->pWnd->GetName()) == 0)
                    {
                        // 找到引用的窗口，添加到同一行
                        row.AddTail(pInfo);
                        bAdded = TRUE;
                        break;
                    }
                }
                if (bAdded)
                    break;
            }
            if (!bAdded)
            {
                // 没有找到引用的窗口，另起一行
                SList<ChildInfo *> newRow;
                newRow.AddTail(pInfo);
                lstRows.AddTail(newRow);
            }
        }
    }

    // 计算总行高
    SPOSITION posRow = lstRows.GetHeadPosition();
    while (posRow)
    {
        SList<ChildInfo *> &row = lstRows.GetNext(posRow);
        int rowHeight = 0;

        SPOSITION posRowChild = row.GetHeadPosition();
        while (posRowChild)
        {
            ChildInfo *pInfo = row.GetNext(posRowChild);
            int nScale = pInfo->pWnd->GetScale();
            CRect rcExtend;
            CalcExtendRect(*pInfo, rcExtend, nScale);
            CSize sz = MeasureChild(*pInfo, rcParent.Width(), SIZE_WRAP_CONTENT);
            rowHeight = smax(rowHeight, sz.cy + rcExtend.top + rcExtend.bottom);
        }

        nTotalHeight += rowHeight;
    }

    // 调整可用区域
    if (bIsTop)
    {
        rcAvailable.top += nTotalHeight;
    }
    else
    {
        rcAvailable.bottom -= nTotalHeight;
    }

    // 布局子窗口
    int yOffset = bIsTop ? rcParent.top : rcParent.bottom - nTotalHeight;

    posRow = lstRows.GetHeadPosition();
    while (posRow)
    {
        SList<ChildInfo *> &row = lstRows.GetNext(posRow);
        int rowHeight = 0;
        float totalWeight = 0.0f;
        int fixedWidthSum = 0;
        int availableWidth = rcAvailable.Width();

        // 计算当前行的高度、总权重和固定宽度总和
        SPOSITION posRowChild = row.GetHeadPosition();
        while (posRowChild)
        {
            ChildInfo *pInfo = row.GetNext(posRowChild);
            int nScale = pInfo->pWnd->GetScale();
            CRect rcExtend;
            CalcExtendRect(*pInfo, rcExtend, nScale);
            CSize sz = MeasureChild(*pInfo, rcParent.Width(), SIZE_WRAP_CONTENT);
            rowHeight = smax(rowHeight, sz.cy + rcExtend.top + rcExtend.bottom);

            // 计算总权重和固定宽度总和
            if (pInfo->pParam->weight > 0)
            {
                totalWeight += pInfo->pParam->weight;
            }
            else
            {
                fixedWidthSum += sz.cx + rcExtend.left + rcExtend.right;
            }
        }

        // 计算剩余宽度
        int remainingWidth = availableWidth - fixedWidthSum;

        // 布局当前行的窗口
        int xOffset = rcParent.left;
        posRowChild = row.GetHeadPosition();
        while (posRowChild)
        {
            ChildInfo *pInfo = row.GetNext(posRowChild);
            int nScale = pInfo->pWnd->GetScale();
            CRect rcExtend;
            CalcExtendRect(*pInfo, rcExtend, nScale);

            // 计算窗口宽度
            CSize sz;
            if (pInfo->pParam->weight > 0 && totalWeight > 0)
            {
                // 使用权重计算宽度
                int weightWidth = (int)(remainingWidth * (pInfo->pParam->weight / totalWeight));
                sz = MeasureChild(*pInfo, weightWidth, SIZE_WRAP_CONTENT);
                sz.cx = weightWidth;
            }
            else
            {
                // 使用默认宽度
                sz = MeasureChild(*pInfo, rcParent.Width(), SIZE_WRAP_CONTENT);
            }

            // 应用gravity
            Gravity gravity = pInfo->pParam->gravity == G_Undefined ? G_Left : pInfo->pParam->gravity;
            int x = xOffset + rcExtend.left;
            int y = yOffset + rcExtend.top;

            // 垂直方向对齐
            if (gravity == G_Center)
            {
                y += (rowHeight - (sz.cy + rcExtend.top + rcExtend.bottom)) / 2;
            }
            else if (gravity == G_Bottom)
            {
                y += rowHeight - (sz.cy + rcExtend.top + rcExtend.bottom);
            }

            CRect rcWnd;
            rcWnd.left = x;
            rcWnd.right = x + sz.cx;

            if (bIsTop)
            {
                rcWnd.top = y;
                rcWnd.bottom = y + sz.cy;
            }
            else
            {
                rcWnd.bottom = y + sz.cy;
                rcWnd.top = y;
            }
            if(rcWnd.right > rcParent.right)
                rcWnd.right = rcParent.right;
            if(rcWnd.left > rcWnd.right)
                rcWnd.left = rcWnd.right;
            sz.cx = rcWnd.right - rcWnd.left;
            ((SWindow *)pInfo->pWnd)->OnRelayout(rcWnd);

            // 更新xOffset，为下一个窗口留出空间
            xOffset += sz.cx + rcExtend.left + rcExtend.right;
        }

        // 更新yOffset，为下一行留出空间
        yOffset += rowHeight;
    }
}

void SFrameLayout::LayoutDockLeftRight(IWindow *pParent, SList<ChildInfo *> &lstChildren, CRect &rcAvailable, BOOL bIsLeft, const CRect &rcParent)
{
    int nTotalWidth = 0;
    SList<ChildInfo *> lstOrdered;

    // 处理相对停靠关系
    SList<ChildInfo *> lstRemaining = lstChildren;
    while (!lstRemaining.IsEmpty())
    {
        SPOSITION pos = lstRemaining.GetHeadPosition();
        while (pos)
        {
            SPOSITION posCurrent = pos;
            ChildInfo *pInfo = lstRemaining.GetNext(pos);

            if (pInfo->pParam->strDockRelativeTo.IsEmpty())
            {
                lstOrdered.AddTail(pInfo);
                lstRemaining.RemoveAt(posCurrent);
            }
            else
            {
                BOOL bRelativeFound = FALSE;
                SPOSITION posOrdered = lstOrdered.GetHeadPosition();
                while (posOrdered)
                {
                    ChildInfo *pOrderedInfo = lstOrdered.GetNext(posOrdered);
                    bool bMatch = pInfo->pParam->strDockRelativeTo.CompareNoCase(pOrderedInfo->pWnd->GetName()) == 0;
                    if (bMatch)
                    {
                        lstOrdered.InsertAfter(posOrdered, pInfo);
                        lstRemaining.RemoveAt(posCurrent);
                        bRelativeFound = TRUE;
                        break;
                    }
                }
            }
        }
    }

    // 将窗口分组到不同的列中
    // 规则：如果窗口没有dockRelativeTo，或者其dockRelativeTo引用的是前一列的最后一个窗口，则另起一列
    SList<SList<ChildInfo *>> lstColumns;

    SPOSITION pos = lstOrdered.GetHeadPosition();
    while (pos)
    {
        ChildInfo *pInfo = lstOrdered.GetNext(pos);

        if (pInfo->pParam->strDockRelativeTo.IsEmpty())
        {
            // 没有dockRelativeTo，另起一列
            SList<ChildInfo *> newColumn;
            newColumn.AddTail(pInfo);
            lstColumns.AddTail(newColumn);
        }
        else
        {
            // 有dockRelativeTo，找到引用的窗口，将当前窗口添加到同一列
            BOOL bAdded = FALSE;
            SPOSITION posColumn = lstColumns.GetHeadPosition();
            while (posColumn)
            {
                SList<ChildInfo *> &column = lstColumns.GetNext(posColumn);
                SPOSITION posColumnChild = column.GetHeadPosition();
                while (posColumnChild)
                {
                    ChildInfo *pColumnChild = column.GetNext(posColumnChild);
                    if (pInfo->pParam->strDockRelativeTo.CompareNoCase(pColumnChild->pWnd->GetName()) == 0)
                    {
                        // 找到引用的窗口，添加到同一列
                        column.AddTail(pInfo);
                        bAdded = TRUE;
                        break;
                    }
                }
                if (bAdded)
                    break;
            }
            if (!bAdded)
            {
                // 没有找到引用的窗口，另起一列
                SList<ChildInfo *> newColumn;
                newColumn.AddTail(pInfo);
                lstColumns.AddTail(newColumn);
            }
        }
    }

    // 计算总宽度
    SPOSITION posColumn = lstColumns.GetHeadPosition();
    while (posColumn)
    {
        SList<ChildInfo *> &column = lstColumns.GetNext(posColumn);
        int columnWidth = 0;

        SPOSITION posColumnChild = column.GetHeadPosition();
        while (posColumnChild)
        {
            ChildInfo *pInfo = column.GetNext(posColumnChild);
            int nScale = pInfo->pWnd->GetScale();
            CRect rcExtend;
            CalcExtendRect(*pInfo, rcExtend, nScale);
            CSize sz = MeasureChild(*pInfo, SIZE_WRAP_CONTENT, rcAvailable.Height());
            columnWidth = smax(columnWidth, sz.cx + rcExtend.left + rcExtend.right);
        }

        nTotalWidth += columnWidth;
    }

    // 调整可用区域
    if (bIsLeft)
    {
        rcAvailable.left += nTotalWidth;
    }
    else
    {
        rcAvailable.right -= nTotalWidth;
    }

    // 布局子窗口
    int xOffset = bIsLeft ? rcParent.left : rcParent.right - nTotalWidth;

    posColumn = lstColumns.GetHeadPosition();
    while (posColumn)
    {
        SList<ChildInfo *> &column = lstColumns.GetNext(posColumn);
        int columnWidth = 0;
        float totalWeight = 0.0f;
        int fixedHeightSum = 0;
        int availableHeight = rcAvailable.Height();

        // 计算当前列的宽度、总权重和固定高度总和
        SPOSITION posColumnChild = column.GetHeadPosition();
        while (posColumnChild)
        {
            ChildInfo *pInfo = column.GetNext(posColumnChild);
            int nScale = pInfo->pWnd->GetScale();
            CRect rcExtend;
            CalcExtendRect(*pInfo, rcExtend, nScale);
            CSize sz = MeasureChild(*pInfo, SIZE_WRAP_CONTENT, rcAvailable.Height());
            columnWidth = smax(columnWidth, sz.cx + rcExtend.left + rcExtend.right);

            // 计算总权重和固定高度总和
            if (pInfo->pParam->weight > 0)
            {
                totalWeight += pInfo->pParam->weight;
            }
            else
            {
                fixedHeightSum += sz.cy + rcExtend.top + rcExtend.bottom;
            }
        }

        // 计算剩余高度
        int remainingHeight = availableHeight - fixedHeightSum;

        // 布局当前列的窗口
        int yOffset = rcAvailable.top;
        posColumnChild = column.GetHeadPosition();
        while (posColumnChild)
        {
            ChildInfo *pInfo = column.GetNext(posColumnChild);
            int nScale = pInfo->pWnd->GetScale();
            CRect rcExtend;
            CalcExtendRect(*pInfo, rcExtend, nScale);

            // 计算窗口高度
            CSize sz;
            if (pInfo->pParam->weight > 0 && totalWeight > 0)
            {
                // 使用权重计算高度
                int weightHeight = (int)(remainingHeight * (pInfo->pParam->weight / totalWeight));
                sz = MeasureChild(*pInfo, SIZE_WRAP_CONTENT, weightHeight);
                sz.cy = weightHeight;
            }
            else
            {
                // 使用默认高度
                sz = MeasureChild(*pInfo, SIZE_WRAP_CONTENT, rcAvailable.Height());
            }

            // 应用gravity
            Gravity gravity = pInfo->pParam->gravity == G_Undefined ? G_Top : pInfo->pParam->gravity;
            int x = xOffset + rcExtend.left;
            int y = yOffset + rcExtend.top;

            // 水平方向对齐
            if (gravity == G_Center)
            {
                x += (columnWidth - (sz.cx + rcExtend.left + rcExtend.right)) / 2;
            }
            else if (gravity == G_Right)
            {
                x += columnWidth - (sz.cx + rcExtend.left + rcExtend.right);
            }

            CRect rcWnd;
            rcWnd.top = y;
            rcWnd.bottom = y + sz.cy;

            if (bIsLeft)
            {
                rcWnd.left = x;
                rcWnd.right = x + sz.cx;
            }
            else
            {
                rcWnd.right = x + sz.cx;
                rcWnd.left = x;
            }

            if(rcWnd.bottom > rcParent.bottom)
                rcWnd.bottom = rcParent.bottom;
            if(rcWnd.top > rcWnd.bottom)
                rcWnd.top = rcWnd.bottom;
            sz.cy = rcWnd.bottom - rcWnd.top;
            ((SWindow *)pInfo->pWnd)->OnRelayout(rcWnd);

            // 更新yOffset，为下一个窗口留出空间
            yOffset += sz.cy + rcExtend.top + rcExtend.bottom;
        }

        // 更新xOffset，为下一列留出空间
        xOffset += columnWidth;
    }
}

void SFrameLayout::LayoutChildren(IWindow *pParent)
{
    SList<ChildInfo> lstChildren;
    CollectChildren(pParent, lstChildren);

    CRect rcParent;
    pParent->GetChildrenLayoutRect(&rcParent);

    CRect rcAvailable = rcParent;

    ChildInfo *pMainViewInfo = NULL;

    SList<ChildInfo *> lstLeft, lstTop, lstRight, lstBottom;

    SPOSITION pos = lstChildren.GetHeadPosition();
    while (pos)
    {
        ChildInfo &info = lstChildren.GetNext(pos);
        if (info.pParam->dockPos == DockMainView)
        {
            pMainViewInfo = &info;
        }
        else if (info.pParam->dockPos == DockLeft)
        {
            lstLeft.AddTail(&info);
        }
        else if (info.pParam->dockPos == DockTop)
        {
            lstTop.AddTail(&info);
        }
        else if (info.pParam->dockPos == DockRight)
        {
            lstRight.AddTail(&info);
        }
        else if (info.pParam->dockPos == DockBottom)
        {
            lstBottom.AddTail(&info);
        }
    }

    LayoutDockTopBottom(pParent, lstTop, rcAvailable, TRUE, rcParent);
    LayoutDockTopBottom(pParent, lstBottom, rcAvailable, FALSE, rcParent);

    LayoutDockLeftRight(pParent, lstLeft, rcAvailable, TRUE, rcParent);
    LayoutDockLeftRight(pParent, lstRight, rcAvailable, FALSE, rcParent);

    if (pMainViewInfo)
    {
        int nScale = pMainViewInfo->pWnd->GetScale();
        CRect rcExtend;
        CalcExtendRect(*pMainViewInfo, rcExtend, nScale);
        CRect rcMainView;
        rcMainView.left = rcAvailable.left + rcExtend.left;
        rcMainView.top = rcAvailable.top + rcExtend.top;
        rcMainView.right = rcAvailable.right - rcExtend.right;
        rcMainView.bottom = rcAvailable.bottom - rcExtend.bottom;
        ((SWindow *)pMainViewInfo->pWnd)->OnRelayout(rcMainView);
    }
}

SNSEND
