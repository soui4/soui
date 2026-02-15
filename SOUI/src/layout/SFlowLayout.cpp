#include "souistd.h"
#include "layout/SFlowLayout.h"
#include "helper/SplitString.h"
#include <core/SWnd.h>

SNSBEGIN

SFlowLayoutParam::SFlowLayoutParam()
{
    Clear();
}

void SFlowLayoutParam::Clear()
{
    width.setWrapContent();
    height.setWrapContent();
    extend_left.setSize(0, dp);
    extend_top.setSize(0, dp);
    extend_right.setSize(0, dp);
    extend_bottom.setSize(0, dp);
    gravity = G_Undefined;
}

BOOL SFlowLayoutParam::IsMatchParent(ORIENTATION orientation) const
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

BOOL SFlowLayoutParam::IsWrapContent(ORIENTATION orientation) const
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

BOOL SFlowLayoutParam::IsSpecifiedSize(ORIENTATION orientation) const
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

BOOL SFlowLayoutParam::GetSpecifiedSize(ORIENTATION orientation, LAYOUTSIZE *pLayoutSize) const
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

HRESULT SFlowLayoutParam::OnAttrSize(const SStringW &strValue, BOOL bLoading)
{
    SStringWList szStr;
    if (2 != SplitString(strValue, L',', szStr))
        return E_FAIL;

    OnAttrWidth(szStr[0], bLoading);
    OnAttrHeight(szStr[1], bLoading);
    return S_OK;
}

HRESULT SFlowLayoutParam::OnAttrWidth(const SStringW &strValue, BOOL bLoading)
{
    if (strValue.CompareNoCase(L"matchParent") == 0)
        width.setMatchParent();
    else if (strValue.CompareNoCase(L"wrapContent") == 0)
        width.setWrapContent();
    else
        width = GETLAYOUTSIZE(strValue);
    return S_OK;
}

HRESULT SFlowLayoutParam::OnAttrHeight(const SStringW &strValue, BOOL bLoading)
{
    if (strValue.CompareNoCase(L"matchParent") == 0)
        height.setMatchParent();
    else if (strValue.CompareNoCase(L"wrapContent") == 0)
        height.setWrapContent();
    else
        height = GETLAYOUTSIZE(strValue);
    return S_OK;
}

HRESULT SFlowLayoutParam::OnAttrExtend(const SStringW &strValue, BOOL bLoading)
{
    SStringWList szStr;
    if (4 != SplitString(strValue, L',', szStr))
        return E_FAIL;

    extend_left = GETLAYOUTSIZE(szStr[0]);
    extend_top = GETLAYOUTSIZE(szStr[1]);
    extend_right = GETLAYOUTSIZE(szStr[2]);
    extend_bottom = GETLAYOUTSIZE(szStr[3]);
    return S_OK;
}

void *SFlowLayoutParam::GetRawData()
{
    return this;
}

ILayoutParam *SFlowLayoutParam::Clone() const
{
    SFlowLayoutParam *pClone = new SFlowLayoutParam();
    memcpy(pClone, this, sizeof(SFlowLayoutParam));
    return pClone;
}

void SFlowLayoutParam::SetMatchParent(ORIENTATION orientation)
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

void SFlowLayoutParam::SetWrapContent(ORIENTATION orientation)
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

void SFlowLayoutParam::SetSpecifiedSize(ORIENTATION orientation, const LAYOUTSIZE *pLayoutSize)
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

BOOL SFlowLayoutParam::SetAnimatorValue(IPropertyValuesHolder *pHolder, float fraction, ANI_STATE state)
{
    return FALSE;
}

SFlowLayout::SFlowLayout(void)
    : m_orientation(Horz)
    , m_gravity(G_Undefined)
{
    m_xInterval.setSize(0, dp);
    m_yInterval.setSize(0, dp);
}

SFlowLayout::~SFlowLayout(void)
{
}

BOOL SFlowLayout::IsParamAcceptable(const ILayoutParam *pLayoutParam) const
{
    return !!pLayoutParam->IsClass(SFlowLayoutParam::GetClassName());
}

ILayoutParam *SFlowLayout::CreateLayoutParam() const
{
    ILayoutParam *pLayoutParam = new SFlowLayoutParam();
    return pLayoutParam;
}

void SFlowLayout::LayoutChildren(IWindow *pParent)
{
    CRect rcParent;
    pParent->GetChildrenLayoutRect(&rcParent);

    int nChilds = pParent->GetChildrenCount();
    if (nChilds == 0)
        return;

    // Collect child sizes
    SIZE *pSize = new SIZE[nChilds];
    IWindow **pChilds = new IWindow *[nChilds];
    memset(pSize, 0, sizeof(SIZE) * nChilds);

    int iChild = 0;
    IWindow *pChild = pParent->GetNextLayoutIChild(NULL);
    while (pChild)
    {
        const SFlowLayoutParam *pFlowLayoutParam = (const SFlowLayoutParam *)pChild->GetLayoutParam();
        int nScale = pChild->GetScale();

        // Get desired size for the child
        CSize szChild(SIZE_WRAP_CONTENT, SIZE_WRAP_CONTENT);
        if (pFlowLayoutParam->IsMatchParent(Horz))
            szChild.cx = rcParent.Width();
        else if (pFlowLayoutParam->IsSpecifiedSize(Horz))
        {
            SLayoutSize layoutSize;
            pFlowLayoutParam->GetSpecifiedSize(Horz, &layoutSize);
            szChild.cx = layoutSize.toPixelSize(nScale);
            szChild.cx += pFlowLayoutParam->extend_left.toPixelSize(nScale) + pFlowLayoutParam->extend_right.toPixelSize(nScale);
        }

        if (pFlowLayoutParam->IsMatchParent(Vert))
            szChild.cy = rcParent.Height();
        else if (pFlowLayoutParam->IsSpecifiedSize(Vert))
        {
            SLayoutSize layoutSize;
            pFlowLayoutParam->GetSpecifiedSize(Vert, &layoutSize);
            szChild.cy = layoutSize.toPixelSize(nScale);
            szChild.cy += pFlowLayoutParam->extend_top.toPixelSize(nScale) + pFlowLayoutParam->extend_bottom.toPixelSize(nScale);
        }

        if (szChild.cx == SIZE_WRAP_CONTENT || szChild.cy == SIZE_WRAP_CONTENT)
        {
            CSize szCalc;
            pChild->GetDesiredSize(&szCalc, szChild.cx, szChild.cy);
            if (szChild.cx == SIZE_WRAP_CONTENT)
            {
                szChild.cx = szCalc.cx;
                szChild.cx += pFlowLayoutParam->extend_left.toPixelSize(nScale) + pFlowLayoutParam->extend_right.toPixelSize(nScale);
            }
            if (szChild.cy == SIZE_WRAP_CONTENT)
            {
                szChild.cy = szCalc.cy;
                szChild.cy += pFlowLayoutParam->extend_top.toPixelSize(nScale) + pFlowLayoutParam->extend_bottom.toPixelSize(nScale);
            }
        }

        pChilds[iChild] = pChild;
        pSize[iChild] = szChild;

        iChild++;
        pChild = pParent->GetNextLayoutIChild(pChild);
    }

    // Layout children based on orientation
    if (m_orientation == Horz)
    {
        // Horizontal flow layout
        int nYInterval = m_yInterval.toPixelSize(pParent->GetScale());
        int nXInterval = m_xInterval.toPixelSize(pParent->GetScale());

        int xOffset = rcParent.left;
        int yOffset = rcParent.top;
        int nLineHeight = 0;

        // Collect items for current line
        int nLineStart = 0;

        for (int i = 0; i < nChilds; i++)
        {
            SWindow *pChild = (SWindow *)pChilds[i];
            SFlowLayoutParam *pFlowLayoutParam = (SFlowLayoutParam *)pChild->GetLayoutParam();
            int nScale = pChild->GetScale();
            Gravity gravity = pFlowLayoutParam->gravity == G_Undefined ? m_gravity : pFlowLayoutParam->gravity;
            if (gravity == G_Undefined)
                gravity = G_Left;

            CSize szChild = pSize[i];

            // Check if we need to wrap to next line
            if (xOffset != rcParent.left && xOffset + szChild.cx > rcParent.right)
            {
                // Layout current line
                nLineHeight = 0;
                xOffset = rcParent.left;
                for (int j = nLineStart; j < i; j++)
                {
                    nLineHeight = smax(nLineHeight, pSize[j].cy);
                }

                // Apply alignment for all items in current line
                int xPos = rcParent.left;
                for (int j = nLineStart; j < i; j++)
                {
                    SWindow *pLineChild = (SWindow *)pChilds[j];
                    SFlowLayoutParam *pLineLayoutParam = (SFlowLayoutParam *)pLineChild->GetLayoutParam();
                    int nLineScale = pLineChild->GetScale();
                    Gravity lineGravity = pLineLayoutParam->gravity == G_Undefined ? m_gravity : pLineLayoutParam->gravity;
                    if (lineGravity == G_Undefined)
                        lineGravity = G_Left;

                    CSize szLineChild = pSize[j];
                    CRect rcLineChild(CPoint(xPos, yOffset), szLineChild);

                    // Apply gravity
                    if (lineGravity == G_Center)
                        rcLineChild.OffsetRect(0, (nLineHeight - szLineChild.cy) / 2);
                    else if (lineGravity == G_Bottom)
                        rcLineChild.OffsetRect(0, nLineHeight - szLineChild.cy);

                    CRect rcLineChild2 = rcLineChild;
                    rcLineChild2.DeflateRect(pLineLayoutParam->extend_left.toPixelSize(nLineScale), pLineLayoutParam->extend_top.toPixelSize(nLineScale), pLineLayoutParam->extend_right.toPixelSize(nLineScale), pLineLayoutParam->extend_bottom.toPixelSize(nLineScale));

                    pLineChild->OnRelayout(rcLineChild2);
                    xPos += szLineChild.cx + nXInterval;
                }

                yOffset += nLineHeight + nYInterval;
                xOffset = rcParent.left;
                nLineHeight = 0;
                nLineStart = i;
            }

            nLineHeight = smax(nLineHeight, szChild.cy);
            xOffset += szChild.cx + nXInterval;
        }

        // Layout last line
        if (nLineStart < nChilds)
        {
            nLineHeight = 0;
            for (int j = nLineStart; j < nChilds; j++)
            {
                nLineHeight = smax(nLineHeight, pSize[j].cy);
            }

            int xPos = rcParent.left;
            for (int j = nLineStart; j < nChilds; j++)
            {
                SWindow *pLineChild = (SWindow *)pChilds[j];
                SFlowLayoutParam *pLineLayoutParam = (SFlowLayoutParam *)pLineChild->GetLayoutParam();
                int nLineScale = pLineChild->GetScale();
                Gravity lineGravity = pLineLayoutParam->gravity == G_Undefined ? m_gravity : pLineLayoutParam->gravity;
                if (lineGravity == G_Undefined)
                    lineGravity = G_Left;

                CSize szLineChild = pSize[j];
                CRect rcLineChild(CPoint(xPos, yOffset), szLineChild);

                // Apply gravity
                if (lineGravity == G_Center)
                    rcLineChild.OffsetRect(0, (nLineHeight - szLineChild.cy) / 2);
                else if (lineGravity == G_Bottom)
                    rcLineChild.OffsetRect(0, nLineHeight - szLineChild.cy);

                CRect rcLineChild2 = rcLineChild;
                rcLineChild2.DeflateRect(pLineLayoutParam->extend_left.toPixelSize(nLineScale), pLineLayoutParam->extend_top.toPixelSize(nLineScale), pLineLayoutParam->extend_right.toPixelSize(nLineScale), pLineLayoutParam->extend_bottom.toPixelSize(nLineScale));

                pLineChild->OnRelayout(rcLineChild2);
                xPos += szLineChild.cx + nXInterval;
            }
        }
    }
    else
    {
        // Vertical flow layout
        int nYInterval = m_yInterval.toPixelSize(pParent->GetScale());
        int nXInterval = m_xInterval.toPixelSize(pParent->GetScale());

        int xOffset = rcParent.left;
        int yOffset = rcParent.top;
        int nLineWidth = 0;

        // Collect items for current column
        int nLineStart = 0;

        for (int i = 0; i < nChilds; i++)
        {
            SWindow *pChild = (SWindow *)pChilds[i];
            SFlowLayoutParam *pFlowLayoutParam = (SFlowLayoutParam *)pChild->GetLayoutParam();
            int nScale = pChild->GetScale();
            Gravity gravity = pFlowLayoutParam->gravity == G_Undefined ? m_gravity : pFlowLayoutParam->gravity;
            if (gravity == G_Undefined)
                gravity = G_Top;

            CSize szChild = pSize[i];

            // Check if we need to wrap to next column
            if (yOffset != rcParent.top && yOffset + szChild.cy > rcParent.bottom)
            {
                // Layout current column
                nLineWidth = 0;
                yOffset = rcParent.top;
                for (int j = nLineStart; j < i; j++)
                {
                    nLineWidth = smax(nLineWidth, pSize[j].cx);
                }

                // Apply alignment for all items in current column
                int yPos = rcParent.top;
                for (int j = nLineStart; j < i; j++)
                {
                    SWindow *pLineChild = (SWindow *)pChilds[j];
                    SFlowLayoutParam *pLineLayoutParam = (SFlowLayoutParam *)pLineChild->GetLayoutParam();
                    int nLineScale = pLineChild->GetScale();
                    Gravity lineGravity = pLineLayoutParam->gravity == G_Undefined ? m_gravity : pLineLayoutParam->gravity;
                    if (lineGravity == G_Undefined)
                        lineGravity = G_Top;

                    CSize szLineChild = pSize[j];
                    CRect rcLineChild(CPoint(xOffset, yPos), szLineChild);

                    // Apply gravity
                    if (lineGravity == G_Center)
                        rcLineChild.OffsetRect((nLineWidth - szLineChild.cx) / 2, 0);
                    else if (lineGravity == G_Right)
                        rcLineChild.OffsetRect(nLineWidth - szLineChild.cx, 0);

                    CRect rcLineChild2 = rcLineChild;
                    rcLineChild2.DeflateRect(pLineLayoutParam->extend_left.toPixelSize(nLineScale), pLineLayoutParam->extend_top.toPixelSize(nLineScale), pLineLayoutParam->extend_right.toPixelSize(nLineScale), pLineLayoutParam->extend_bottom.toPixelSize(nLineScale));

                    pLineChild->OnRelayout(rcLineChild2);
                    yPos += szLineChild.cy + nYInterval;
                }

                xOffset += nLineWidth + nXInterval;
                yOffset = rcParent.top;
                nLineWidth = 0;
                nLineStart = i;
            }

            nLineWidth = smax(nLineWidth, szChild.cx);
            yOffset += szChild.cy + nYInterval;
        }

        // Layout last column
        if (nLineStart < nChilds)
        {
            nLineWidth = 0;
            for (int j = nLineStart; j < nChilds; j++)
            {
                nLineWidth = smax(nLineWidth, pSize[j].cx);
            }

            int yPos = rcParent.top;
            for (int j = nLineStart; j < nChilds; j++)
            {
                SWindow *pLineChild = (SWindow *)pChilds[j];
                SFlowLayoutParam *pLineLayoutParam = (SFlowLayoutParam *)pLineChild->GetLayoutParam();
                int nLineScale = pLineChild->GetScale();
                Gravity lineGravity = pLineLayoutParam->gravity == G_Undefined ? m_gravity : pLineLayoutParam->gravity;
                if (lineGravity == G_Undefined)
                    lineGravity = G_Top;

                CSize szLineChild = pSize[j];
                CRect rcLineChild(CPoint(xOffset, yPos), szLineChild);

                // Apply gravity
                if (lineGravity == G_Center)
                    rcLineChild.OffsetRect((nLineWidth - szLineChild.cx) / 2, 0);
                else if (lineGravity == G_Right)
                    rcLineChild.OffsetRect(nLineWidth - szLineChild.cx, 0);

                CRect rcLineChild2 = rcLineChild;
                rcLineChild2.DeflateRect(pLineLayoutParam->extend_left.toPixelSize(nLineScale), pLineLayoutParam->extend_top.toPixelSize(nLineScale), pLineLayoutParam->extend_right.toPixelSize(nLineScale), pLineLayoutParam->extend_bottom.toPixelSize(nLineScale));

                pLineChild->OnRelayout(rcLineChild2);
                yPos += szLineChild.cy + nYInterval;
            }
        }
    }

    delete[] pSize;
    delete[] pChilds;
}

SIZE SFlowLayout::MeasureChildren(const IWindow *pParent, int nWidth, int nHeight) const
{
    SIZE szRet = { 0, 0 };
    int nChilds = pParent->GetChildrenCount();
    if (nChilds == 0)
        return szRet;

    // Collect child sizes
    SIZE *pSize = new SIZE[nChilds];
    memset(pSize, 0, sizeof(SIZE) * nChilds);

    int iChild = 0;
    IWindow *pChild = pParent->GetNextLayoutIChild(NULL);
    while (pChild)
    {
        const SFlowLayoutParam *pFlowLayoutParam = (const SFlowLayoutParam *)pChild->GetLayoutParam();
        int nScale = pChild->GetScale();

        CSize szChild(SIZE_WRAP_CONTENT, SIZE_WRAP_CONTENT);
        if (pFlowLayoutParam->IsMatchParent(Horz))
            szChild.cx = nWidth;
        else if (pFlowLayoutParam->IsSpecifiedSize(Horz))
        {
            SLayoutSize layoutSize;
            pFlowLayoutParam->GetSpecifiedSize(Horz, &layoutSize);
            szChild.cx = layoutSize.toPixelSize(nScale);
            szChild.cx += pFlowLayoutParam->extend_left.toPixelSize(nScale) + pFlowLayoutParam->extend_right.toPixelSize(nScale);
        }

        if (pFlowLayoutParam->IsMatchParent(Vert))
            szChild.cy = nHeight;
        else if (pFlowLayoutParam->IsSpecifiedSize(Vert))
        {
            SLayoutSize layoutSize;
            pFlowLayoutParam->GetSpecifiedSize(Vert, &layoutSize);
            szChild.cy = layoutSize.toPixelSize(nScale);
            szChild.cy += pFlowLayoutParam->extend_top.toPixelSize(nScale) + pFlowLayoutParam->extend_bottom.toPixelSize(nScale);
        }

        if (szChild.cx == SIZE_WRAP_CONTENT || szChild.cy == SIZE_WRAP_CONTENT)
        {
            CSize szCalc;
            pChild->GetDesiredSize(&szCalc, szChild.cx, szChild.cy);
            if (szChild.cx == SIZE_WRAP_CONTENT)
            {
                szChild.cx = szCalc.cx;
                szChild.cx += pFlowLayoutParam->extend_left.toPixelSize(nScale) + pFlowLayoutParam->extend_right.toPixelSize(nScale);
            }
            if (szChild.cy == SIZE_WRAP_CONTENT)
            {
                szChild.cy = szCalc.cy;
                szChild.cy += pFlowLayoutParam->extend_top.toPixelSize(nScale) + pFlowLayoutParam->extend_bottom.toPixelSize(nScale);
            }
        }

        pSize[iChild] = szChild;
        iChild++;
        pChild = pParent->GetNextLayoutIChild(pChild);
    }

    // Calculate measured size based on orientation
    if (m_orientation == Horz)
    {
        int nYInterval = m_yInterval.toPixelSize(pParent->GetScale());
        int nXInterval = m_xInterval.toPixelSize(pParent->GetScale());

        int nMaxWidth = nWidth == SIZE_WRAP_CONTENT ? 0x0FFF : nWidth;
        int xOffset = 0;
        int nLineHeight = 0;
        int nTotalHeight = 0;

        for (int i = 0; i < nChilds; i++)
        {
            CSize szChild = pSize[i];

            // Check if we need to wrap to next line
            if (xOffset != 0 && xOffset + szChild.cx > nMaxWidth)
            {
                xOffset = 0;
                nTotalHeight += nLineHeight + nYInterval;
                nLineHeight = 0;
            }

            nLineHeight = smax(nLineHeight, szChild.cy);
            xOffset += szChild.cx + nXInterval;
        }

        nTotalHeight += nLineHeight;
        szRet.cx = xOffset > 0 ? xOffset - nXInterval : 0;
        szRet.cy = nTotalHeight;
    }
    else
    {
        int nYInterval = m_yInterval.toPixelSize(pParent->GetScale());
        int nXInterval = m_xInterval.toPixelSize(pParent->GetScale());

        int nMaxHeight = nHeight == SIZE_WRAP_CONTENT ? 0x0FFF : nHeight;
        int yOffset = 0;
        int nLineWidth = 0;
        int nTotalWidth = 0;

        for (int i = 0; i < nChilds; i++)
        {
            CSize szChild = pSize[i];

            // Check if we need to wrap to next column
            if (yOffset != 0 && yOffset + szChild.cy > nMaxHeight)
            {
                yOffset = 0;
                nTotalWidth += nLineWidth + nXInterval;
                nLineWidth = 0;
            }

            nLineWidth = smax(nLineWidth, szChild.cx);
            yOffset += szChild.cy + nYInterval;
        }

        nTotalWidth += nLineWidth;
        szRet.cx = nTotalWidth;
        szRet.cy = yOffset > 0 ? yOffset - nYInterval : 0;
    }

    delete[] pSize;
    return szRet;
}

SNSEND
