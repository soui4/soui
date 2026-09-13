#include "souistd.h"
#include "layout/SouiLayout.h"
#include "helper/SplitString.h"
#include <core/SWnd.h>
#pragma warning(push)
#pragma warning(disable : 4985) /**< disable the warning message during the include */
#include <math.h>               /**< this is where I would normally get the warning message */
#pragma warning(pop)

SNSBEGIN
enum
{
    POS_INIT = 0x11000000, /**< Initial value of the coordinate */
    POS_WAIT = 0x12000000, /**< Coordinate calculation depends on the layout of other windows */
};

SouiLayoutParam::SouiLayoutParam()
{
    Clear();
}

BOOL SouiLayoutParam::IsMatchParent(ORIENTATION orientation) const
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

BOOL SouiLayoutParam::IsSpecifiedSize(ORIENTATION orientation) const
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

BOOL SouiLayoutParam::IsWrapContent(ORIENTATION orientation) const
{
    switch (orientation)
    {
    case Horz:
        return width.isWrapContent() || (nCount == 0 && !width.isValid());
    case Vert:
        return height.isWrapContent() || (nCount == 0 && !height.isValid());
    case Any:
        return IsWrapContent(Horz) || IsWrapContent(Vert);
    case Both:
    default:
        return IsWrapContent(Horz) && IsWrapContent(Vert);
    }
}

BOOL SouiLayoutParam::GetSpecifiedSize(ORIENTATION orientation, LAYOUTSIZE *pLayoutSize) const
{
    if (orientation == Vert)
        *pLayoutSize = height;
    else
        *pLayoutSize = width;
    return TRUE;
}

HRESULT SouiLayoutParam::OnAttrOffset(const SStringW &strValue, BOOL bLoading)
{
    float fx, fy;
    if (2 != swscanf(strValue, L"%f,%f", &fx, &fy))
    {
        return E_FAIL;
    }
    fOffsetX = fx;
    fOffsetY = fy;
    return S_OK;
}

BOOL SouiLayoutParam::ParsePosition12(const SStringW &strPos1, const SStringW &strPos2)
{
    if (strPos1.IsEmpty() || strPos2.IsEmpty())
        return FALSE;
    POS_INFO pos1, pos2;
    if (!StrPos2ItemPos(strPos1, pos1) || !StrPos2ItemPos(strPos2, pos2))
        return FALSE;
    if (pos1.pit == PIT_SIZE || pos2.pit == PIT_SIZE) // The first 2 attributes cannot be of size type
        return FALSE;
    posLeft = pos1;
    posTop = pos2;
    nCount = 2;
    return TRUE;
}

BOOL SouiLayoutParam::ParsePosition34(const SStringW &strPos3, const SStringW &strPos4)
{
    if (strPos3.IsEmpty() || strPos4.IsEmpty())
        return FALSE;
    POS_INFO pos3, pos4;
    if (!StrPos2ItemPos(strPos3, pos3) || !StrPos2ItemPos(strPos4, pos4))
        return FALSE;

    posRight = pos3;
    posBottom = pos4;
    nCount = 4;
    return TRUE;
}

BOOL SouiLayoutParam::StrPos2ItemPos(const SStringW &strPos, POS_INFO &pos)
{
    if (strPos.IsEmpty())
        return FALSE;

    if (strPos.Left(4) == L"sib.")
    {
        int nOffset = 0;
        if (strPos.Mid(4, 5) == L"left@")
        {
            pos.pit = PIT_SIB_LEFT;
            nOffset = 5;
        }
        else if (strPos.Mid(4, 6) == L"right@")
        {
            pos.pit = PIT_SIB_RIGHT;
            nOffset = 6;
        }
        else if (strPos.Mid(4, 4) == L"top@")
        {
            pos.pit = PIT_SIB_TOP;
            nOffset = 4;
        }
        else if (strPos.Mid(4, 7) == L"bottom@")
        {
            pos.pit = PIT_SIB_BOTTOM;
            nOffset = 7;
        }
        else
        {
            return FALSE;
        }
        SStringW strValue = strPos.Mid(4 + nOffset);
        SStringWList values;
        if (SplitString(strValue, L':', values) != 2)
            return FALSE;
        pos.nRefID = _wtoi(values[0]);
        if (pos.nRefID == 0)
            return FALSE;
        pos.nPos = GETLAYOUTSIZE(values[1]);

        if (pos.nPos.fSize < 0)
        {
            pos.nPos.fSize *= -1;
            pos.bMinus = true;
        }
        else
        {
            pos.bMinus = false;
        }
    }
    else
    {
        LPCWSTR pszPos = strPos;
        switch (pszPos[0])
        {
        case POSFLAG_REFCENTER:
            pos.pit = PIT_CENTER, pszPos++;
            break;
        case POSFLAG_PERCENT:
            pos.pit = PIT_PERCENT, pszPos++;
            break;
        case POSFLAG_REFPREV_NEAR:
            pos.pit = PIT_PREV_NEAR, pszPos++;
            break;
        case POSFLAG_REFNEXT_NEAR:
            pos.pit = PIT_NEXT_NEAR, pszPos++;
            break;
        case POSFLAG_REFPREV_FAR:
            pos.pit = PIT_PREV_FAR, pszPos++;
            break;
        case POSFLAG_REFNEXT_FAR:
            pos.pit = PIT_NEXT_FAR, pszPos++;
            break;
        case POSFLAG_SIZE:
            pos.pit = PIT_SIZE, pszPos++;
            break;
        default:
            pos.pit = PIT_NORMAL;
            break;
        }

        pos.nRefID = -1; // not ref sibling using id
        if (pszPos[0] == L'-')
        {
            pos.bMinus = true;
            pszPos++;
        }
        else
        {
            pos.bMinus = false;
        }
        pos.nPos = GETLAYOUTSIZE(pszPos);
    }

    return TRUE;
}

HRESULT SouiLayoutParam::OnAttrPos(const SStringW &strValue, BOOL bLoading)
{
    SStringWList strLst;
    SplitString(strValue, L',', strLst);
    if (strLst.GetCount() != 2 && strLst.GetCount() != 4)
    {
        SSLOGW() << "Parse pos attribute failed, strPos=" << strValue;
        return E_INVALIDARG;
    }
    // Add space compatibility in the pos attribute.
    for (size_t i = 0; i < strLst.GetCount(); i++)
    {
        strLst.GetAt(i).TrimBlank();
    }
    BOOL bRet = TRUE;

    bRet = ParsePosition12(strLst[0], strLst[1]);
    if (strLst.GetCount() == 4)
    {
        bRet = ParsePosition34(strLst[2], strLst[3]);
    }
    if (bRet && nCount == 4)
    { // Check whether the X,Y directions fill the parent window
        if ((posLeft.pit == PIT_NORMAL && posLeft.nPos.isZero() && (!posTop.bMinus)) && (posRight.pit == PIT_NORMAL && posRight.nPos.isZero() && posBottom.bMinus))
        {
            width.setMatchParent();
        }
        else if (posRight.pit == PIT_SIZE)
        {
            if (posRight.bMinus)
                width.setWrapContent();
            else
                width = posRight.nPos;
        }
        else
        {
            width.setInvalid();
        }

        if ((posTop.pit == PIT_NORMAL && posTop.nPos.isZero() && (!posTop.bMinus)) && (posBottom.pit == PIT_NORMAL && posBottom.nPos.isZero() && posBottom.bMinus))
        {
            height.setMatchParent();
        }
        else if (posBottom.pit == PIT_SIZE)
        {
            if (posBottom.bMinus)
                height.setWrapContent();
            else
                height = posBottom.nPos;
        }
        else
        {
            height.setInvalid();
        }
    }
    else
    {
        if (!width.isValid())
            SetWrapContent(Horz);
        if (!height.isValid())
            SetWrapContent(Vert);
    }

    return S_OK;
}

HRESULT SouiLayoutParam::OnAttrSize(const SStringW &strValue, BOOL bLoading)
{
    SStringWList szStr;
    if (2 != SplitString(strValue, L',', szStr))
        return E_FAIL;

    width = GETLAYOUTSIZE(szStr[0]);
    height = GETLAYOUTSIZE(szStr[1]);
    return S_OK;
}

bool SouiLayoutParam::IsOffsetRequired(ORIENTATION orientation) const
{
    return fabs(orientation == Vert ? fOffsetY : fOffsetX) > 0.00000001f;
}

int GetPosExtra(const POS_INFO &pos, int nScale)
{
    return pos.bMinus ? pos.nPos.toPixelSize(nScale) : 0;
}

int SouiLayoutParam::GetExtraSize(ORIENTATION orientation, int nScale) const
{
    if (nCount != 4)
        return 0;
    if (orientation == Horz)
        return GetPosExtra(posRight, nScale);
    else
        return GetPosExtra(posBottom, nScale);
}

void SouiLayoutParam::Clear()
{
    nCount = 0;
    fOffsetX = fOffsetY = 0.0f;

    width.setWrapContent();
    height.setWrapContent();
}

void SouiLayoutParam::SetMatchParent(ORIENTATION orientation)
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

void SouiLayoutParam::SetWrapContent(ORIENTATION orientation)
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

void SouiLayoutParam::SetSpecifiedSize(ORIENTATION orientation, const LAYOUTSIZE *pLayoutSize)
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

void *SouiLayoutParam::GetRawData()
{
    return (SouiLayoutParamStruct *)this;
}

ILayoutParam *SouiLayoutParam::Clone() const
{
    SouiLayoutParam *pRet = new SouiLayoutParam();
    memcpy(pRet->GetRawData(), (void *)(SouiLayoutParamStruct *)this, sizeof(SouiLayoutParamStruct));
    return pRet;
}

BOOL SouiLayoutParam::SetAnimatorValue(IPropertyValuesHolder *pHolder, float fraction, ANI_STATE state)
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
    if (strPropName.CompareNoCase(LayoutProperty::OFFSET_X) == 0)
    {
        pHolder->GetAnimatedValue(fraction, &fOffsetX);
        return TRUE;
    }
    if (strPropName.CompareNoCase(LayoutProperty::OFFSET_Y) == 0)
    {
        pHolder->GetAnimatedValue(fraction, &fOffsetY);
        return TRUE;
    }
    return FALSE;
}

///////////////////////////////////////////////////////////////////////

SouiLayout::SouiLayout(void)
{
}

SouiLayout::~SouiLayout(void)
{
}

BOOL SouiLayout::IsParamAcceptable(const ILayoutParam *pLayoutParam) const
{
    return !!pLayoutParam->IsClass(SouiLayoutParam::GetClassName());
}

ILayoutParam *SouiLayout::CreateLayoutParam() const
{
    return new SouiLayoutParam();
}

BOOL SouiLayout::IsWaitingPos(int nPos) const
{
    return nPos == POS_INIT || nPos == POS_WAIT;
}

int SouiLayout::PositionItem2Value(SList<WndPos> *pLstChilds, SPOSITION position, const POS_INFO &pos, int nMax, BOOL bX, int nScale) const
{
    int nRet = POS_WAIT;

    switch (pos.pit)
    {
    case PIT_CENTER: // Reference center
        if (nMax != SIZE_WRAP_CONTENT)
            nRet = pos.nPos.toPixelSize(nScale) * (pos.bMinus ? -1 : 1) + nMax / 2;
        break;
    case PIT_NORMAL:
        if (pos.bMinus)
        { // Reference right or bottom
            if (nMax != SIZE_WRAP_CONTENT)
                nRet = nMax - pos.nPos.toPixelSize(nScale);
        }
        else
        {
            nRet = pos.nPos.toPixelSize(nScale);
        }
        break;
    case PIT_PERCENT:
        if (nMax != SIZE_WRAP_CONTENT)
        {
            float fPercent = pos.nPos.fSize;
            if (fPercent < 0.0f)
                fPercent = 0.0f;
            if (fPercent > 100.0f)
                fPercent = 100.0f;
            if (pos.bMinus)
                nRet = (int)((100.0f - fPercent) * nMax / 100);
            else
                nRet = (int)(fPercent * nMax / 100);
        }
        break;
    case PIT_PREV_NEAR:
    case PIT_PREV_FAR:
    {
        SPOSITION positionPrev = pLstChilds->Prev(position);
        int nRef = POS_WAIT;
        if (positionPrev)
        {
            WndPos wndPos = pLstChilds->GetAt(positionPrev);
            if (bX)
            {
                if (!wndPos.bWaitOffsetX)
                    nRef = (pos.pit == PIT_PREV_NEAR) ? wndPos.rc.right : wndPos.rc.left;
            }
            else
            {
                if (!wndPos.bWaitOffsetY)
                    nRef = (pos.pit == PIT_PREV_NEAR) ? wndPos.rc.bottom : wndPos.rc.top;
            }
        }
        else
        {
            nRef = 0;
        }
        if (!IsWaitingPos(nRef))
            nRet = nRef + pos.nPos.toPixelSize(nScale) * (pos.bMinus ? -1 : 1);
    }
    break;
    case PIT_NEXT_NEAR:
    case PIT_NEXT_FAR:
    {
        SPOSITION positionNext = pLstChilds->Next(position);
        int nRef = nMax;
        if (positionNext)
        {
            nRef = POS_WAIT;
            WndPos wndPos = pLstChilds->GetAt(positionNext);
            if (bX)
            {
                if (!wndPos.bWaitOffsetX)
                    nRef = (pos.pit == PIT_NEXT_NEAR) ? wndPos.rc.left : wndPos.rc.right;
            }
            else
            {
                if (!wndPos.bWaitOffsetY)
                    nRef = (pos.pit == PIT_NEXT_NEAR) ? wndPos.rc.top : wndPos.rc.bottom;
            }
        }
        if (!IsWaitingPos(nRef))
            nRet = nRef + pos.nPos.toPixelSize(nScale) * (pos.bMinus ? -1 : 1);
    }
    break;
    case PIT_SIB_LEFT:  // PIT_SIB_LEFT == PIT_SIB_TOP
    case PIT_SIB_RIGHT: // PIT_SIB_RIGHT == PIT_SIB_BOTTOM
    {
        WndPos wndPos = pLstChilds->GetAt(position);
        SASSERT(pos.nRefID > 0);

        WndPos wndPosRef = { 0 };
        SPOSITION posTmp = pLstChilds->GetHeadPosition();
        while (posTmp)
        {
            WndPos wp = pLstChilds->GetNext(posTmp);
            if (wp.pWnd->GetID() == pos.nRefID)
            {
                wndPosRef = wp;
                break;
            }
        }
        if (!wndPosRef.pWnd)
        { // When not found, use parent window info
            wndPosRef.rc = CRect(0, 0, nMax, nMax);
            wndPosRef.bWaitOffsetX = wndPosRef.bWaitOffsetY = false;
        }
        CRect rcRef = wndPosRef.rc;

        if (bX)
        {
            if (!wndPosRef.bWaitOffsetX)
            {
                LONG refPos = (pos.pit == PIT_SIB_LEFT) ? rcRef.left : rcRef.right;
                if (IsWaitingPos(refPos))
                    nRet = POS_WAIT;
                else
                    nRet = refPos + pos.nPos.toPixelSize(nScale) * (pos.bMinus ? -1 : 1);
            }
        }
        else
        {
            if (!wndPosRef.bWaitOffsetY)
            {
                LONG refPos = (pos.pit == PIT_SIB_TOP) ? rcRef.top : rcRef.bottom; // PIT_SIB_TOP == PIT_SIB_LEFT
                if (IsWaitingPos(refPos))
                    nRet = POS_WAIT;
                else
                    nRet = refPos + pos.nPos.toPixelSize(nScale) * (pos.bMinus ? -1 : 1);
            }
        }
    }
    break;
    }

    return nRet;
}

SIZE SouiLayout::MeasureChildren(const IWindow *pParent, int nWidth, int nHeight) const
{
    SList<WndPos> lstWndPos;

    const IWindow *pChild = pParent->GetNextLayoutIChild(NULL);
    while (pChild)
    {
        if (!pChild->IsFloat() && (pChild->IsVisible(FALSE) || pChild->IsDisplay()))
        { // Windows that are not visible and do not occupy space are excluded from calculation
            WndPos wndPos;
            wndPos.pWnd = (SWindow *)pChild;
            wndPos.rc = CRect(POS_INIT, POS_INIT, POS_INIT, POS_INIT);
            const SouiLayoutParam *pParam = (const SouiLayoutParam *)pChild->GetLayoutParam();
            wndPos.bWaitOffsetX = pParam->IsOffsetRequired(Horz);
            wndPos.bWaitOffsetY = pParam->IsOffsetRequired(Vert);
            lstWndPos.AddTail(wndPos);
        }
        pChild = pParent->GetNextLayoutIChild(pChild);
    }

    // Calculate child window position
    CalcPositionEx(&lstWndPos, nWidth, nHeight);

    // Calculate child window extent
    int nMaxX = 0, nMaxY = 0;
    SPOSITION pos = lstWndPos.GetHeadPosition();
    while (pos)
    {
        WndPos wndPos = lstWndPos.GetNext(pos);
        const SouiLayoutParam *pParam = (const SouiLayoutParam *)wndPos.pWnd->GetLayoutParam();
        int nScale = wndPos.pWnd->GetScale();
        if (!IsWaitingPos(wndPos.rc.right))
        {
            nMaxX = smax(nMaxX, (int)(wndPos.rc.right + pParam->GetExtraSize(Horz, nScale)));
        }
        if (!IsWaitingPos(wndPos.rc.bottom))
        {
            nMaxY = smax(nMaxY, (int)(wndPos.rc.bottom + pParam->GetExtraSize(Vert, nScale)));
        }
    }

    if (!IsWaitingPos(nWidth))
        nWidth = nMaxX;
    if (!IsWaitingPos(nHeight))
        nHeight = nMaxY;
    return CSize(nWidth, nHeight);
}

/**
Logic for computing child window container size:
1: A window referencing the parent's top-left corner is called a type-I deterministic window.
2: A window referencing a type-I window is called a type-II deterministic window.
3: A window referencing the parent's top-left corner or a type-I/II deterministic window on its left, and the parent's bottom-right corner on its right, is a type-I non-deterministic window; such windows are automatically converted to wrap_content size.
4: A window referencing the parent's bottom-right corner on both sides is a type-II non-deterministic window; such windows do not affect the parent's size.
5: Windows referencing type-I/II non-deterministic-size windows also do not affect the parent's size.

As long as a control's left position is determined, its right position can also be guaranteed determined.
If the left position cannot be determined, the control's size does not affect the parent's size.
*/
void SouiLayout::CalcPositionEx(SList<WndPos> *pListChildren, int nWidth, int nHeight) const
{
    CalcPostion(pListChildren, nWidth, nHeight);

    // Set children referencing the parent's right or bottom edge to wrap_content and compute their size

    int nResolved = 0;
    for (SPOSITION pos = pListChildren->GetHeadPosition(); pos; pListChildren->GetNext(pos))
    {
        WndPos &wndPos = pListChildren->GetAt(pos);
        const SouiLayoutParam *pLayoutParam = (const SouiLayoutParam *)wndPos.pWnd->GetLayoutParam();
        if (!IsWaitingPos(wndPos.rc.left) && !IsWaitingPos(wndPos.rc.top) && (IsWaitingPos(wndPos.rc.right) && IsWaitingPos(nWidth) || IsWaitingPos(wndPos.rc.bottom) && IsWaitingPos(nHeight)))
        {
            int nWid = IsWaitingPos(wndPos.rc.right) ? nWidth : (wndPos.rc.right - wndPos.rc.left);
            int nHei = IsWaitingPos(wndPos.rc.bottom) ? nHeight : (wndPos.rc.bottom - wndPos.rc.top);
            CSize szWnd;
            wndPos.pWnd->GetDesiredSize(&szWnd, nWid, nHei);
            if (pLayoutParam->IsWrapContent(Horz))
            {
                wndPos.rc.right = wndPos.rc.left + szWnd.cx;
                if (wndPos.bWaitOffsetX)
                {
                    wndPos.rc.OffsetRect((int)(wndPos.rc.Width() * pLayoutParam->fOffsetX), 0);
                    wndPos.bWaitOffsetX = false;
                }
                nResolved++;
            }
            if (pLayoutParam->IsWrapContent(Vert))
            {
                wndPos.rc.bottom = wndPos.rc.top + szWnd.cy;
                if (wndPos.bWaitOffsetY)
                {
                    wndPos.rc.OffsetRect(0, (int)(wndPos.rc.Height() * pLayoutParam->fOffsetY));
                    wndPos.bWaitOffsetY = false;
                }
                nResolved++;
            }
        }
    }
}

static const POS_INFO posRefLeft = { PIT_PREV_NEAR, -1, 1 };
static const POS_INFO posRefTop = { PIT_PREV_FAR, -1, 1 };

int SouiLayout::CalcPostion(SList<WndPos> *pListChildren, int nWidth, int nHeight) const
{
    int nResolvedAll = 0;

    int nResolvedStep1 = 0;
    int nResolvedStep2 = 0;
    do
    {
        nResolvedStep1 = 0;
        nResolvedStep2 = 0;

        // step 1: compute all coordinates that can be determined without calculating window size
        int nResolved = 0;
        do
        {
            nResolved = 0;
            for (SPOSITION pos = pListChildren->GetHeadPosition(); pos; pListChildren->GetNext(pos))
            {
                WndPos &wndPos = pListChildren->GetAt(pos);
                const SouiLayoutParam *pLayoutParam = (const SouiLayoutParam *)wndPos.pWnd->GetLayoutParam();
                int nScale = wndPos.pWnd->GetScale();
                if (IsWaitingPos(wndPos.rc.left))
                {
                    const POS_INFO &posRef = pLayoutParam->nCount >= 2 ? pLayoutParam->posLeft : posRefLeft;
                    wndPos.rc.left = PositionItem2Value(pListChildren, pos, posRef, nWidth, TRUE, nScale);
                    if (wndPos.rc.left != POS_WAIT)
                        nResolved++;
                }
                if (IsWaitingPos(wndPos.rc.top))
                {
                    const POS_INFO &posRef = pLayoutParam->nCount >= 2 ? pLayoutParam->posTop : posRefTop;
                    wndPos.rc.top = PositionItem2Value(pListChildren, pos, posRef, nHeight, FALSE, nScale);
                    if (wndPos.rc.top != POS_WAIT)
                        nResolved++;
                }
                if (IsWaitingPos(wndPos.rc.right))
                {
                    if (pLayoutParam->IsMatchParent(Horz))
                    {
                        wndPos.rc.right = nWidth;
                    }
                    else if (pLayoutParam->IsSpecifiedSize(Horz))
                    {
                        if (!IsWaitingPos(wndPos.rc.left))
                        {
                            SLayoutSize layoutSize;
                            pLayoutParam->GetSpecifiedSize(Horz, &layoutSize);
                            wndPos.rc.right = wndPos.rc.left + layoutSize.toPixelSize(nScale);
                            nResolved++;
                        }
                    }
                    else if (!pLayoutParam->IsWrapContent(Horz) && pLayoutParam->nCount == 4)
                    {
                        wndPos.rc.right = PositionItem2Value(pListChildren, pos, pLayoutParam->posRight, nWidth, TRUE, nScale);
                        if (wndPos.rc.right != POS_WAIT)
                            nResolved++;
                    }
                }
                if (IsWaitingPos(wndPos.rc.bottom))
                {
                    if (pLayoutParam->IsMatchParent(Vert))
                    {
                        wndPos.rc.bottom = nHeight;
                    }
                    else if (pLayoutParam->IsSpecifiedSize(Vert))
                    {
                        if (!IsWaitingPos(wndPos.rc.top))
                        {
                            SLayoutSize layoutSize;
                            pLayoutParam->GetSpecifiedSize(Vert, &layoutSize);
                            wndPos.rc.bottom = wndPos.rc.top + layoutSize.toPixelSize(nScale);
                            nResolved++;
                        }
                    }
                    else if (!pLayoutParam->IsWrapContent(Vert) && pLayoutParam->nCount == 4)
                    {
                        wndPos.rc.bottom = PositionItem2Value(pListChildren, pos, pLayoutParam->posBottom, nHeight, FALSE, nScale);
                        if (wndPos.rc.bottom != POS_WAIT)
                            nResolved++;
                    }
                }
            }

            nResolvedStep1 += nResolved;
        } while (nResolved);

        if (nResolvedStep1 > 0)
        {
            int nResolved = 0;
            // step 2: compute the size of wrap_content windows, and complete the offset operation for windows that can be determined
            do
            {
                nResolved = 0;
                for (SPOSITION pos = pListChildren->GetHeadPosition(); pos; pListChildren->GetNext(pos))
                {
                    WndPos &wndPos = pListChildren->GetAt(pos);
                    const SouiLayoutParam *pLayoutParam = (const SouiLayoutParam *)wndPos.pWnd->GetLayoutParam();
                    if (IsWaitingPos(wndPos.rc.left) || IsWaitingPos(wndPos.rc.top))
                        continue; // Only start calculation after at least one point is determined

                    if ((IsWaitingPos(wndPos.rc.right) && pLayoutParam->IsWrapContent(Horz)) || (IsWaitingPos(wndPos.rc.bottom) && pLayoutParam->IsWrapContent(Vert)))
                    { //
                        int nWid = IsWaitingPos(wndPos.rc.right) ? SIZE_WRAP_CONTENT : (wndPos.rc.right - wndPos.rc.left);
                        int nHei = IsWaitingPos(wndPos.rc.bottom) ? SIZE_WRAP_CONTENT : (wndPos.rc.bottom - wndPos.rc.top);
                        CSize szWnd;
                        wndPos.pWnd->GetDesiredSize(&szWnd, nWid, nHei);
                        if (pLayoutParam->IsWrapContent(Horz))
                        {
                            wndPos.rc.right = wndPos.rc.left + szWnd.cx;
                            nResolved++;
                        }
                        if (pLayoutParam->IsWrapContent(Vert))
                        {
                            wndPos.rc.bottom = wndPos.rc.top + szWnd.cy;
                            nResolved++;
                        }
                    }
                    if (!IsWaitingPos(wndPos.rc.right) && wndPos.bWaitOffsetX)
                    {
                        wndPos.rc.OffsetRect((int)(wndPos.rc.Width() * pLayoutParam->fOffsetX), 0);
                        wndPos.bWaitOffsetX = false;
                    }
                    if (!IsWaitingPos(wndPos.rc.bottom) && wndPos.bWaitOffsetY)
                    {
                        wndPos.rc.OffsetRect(0, (int)(wndPos.rc.Height() * pLayoutParam->fOffsetY));
                        wndPos.bWaitOffsetY = false;
                    }
                }
                nResolvedStep2 += nResolved;
            } while (nResolved);
        } // end if(nResolvedStep1>0)

        nResolvedAll += nResolvedStep1 + nResolvedStep2;
    } while (nResolvedStep2 || nResolvedStep1);

    return nResolvedAll;
}

void SouiLayout::LayoutChildren(IWindow *pParent)
{
    SList<WndPos> lstWndPos;

    IWindow *pChild = pParent->GetNextLayoutIChild(NULL);
    while (pChild)
    {
        WndPos wndPos;
        wndPos.pWnd = (SWindow *)pChild;
        wndPos.rc = CRect(POS_INIT, POS_INIT, POS_INIT, POS_INIT);
        const SouiLayoutParam *pParam = (const SouiLayoutParam *)pChild->GetLayoutParam();
        wndPos.bWaitOffsetX = pParam->IsOffsetRequired(Horz);
        wndPos.bWaitOffsetY = pParam->IsOffsetRequired(Vert);
        lstWndPos.AddTail(wndPos);

        pChild = pParent->GetNextLayoutIChild(pChild);
    }

    if (lstWndPos.IsEmpty())
        return;

    CRect rcParent;
    pParent->GetChildrenLayoutRect(&rcParent);
    // Calculate child window position
    CalcPostion(&lstWndPos, rcParent.Width(), rcParent.Height());

    // Offset window coordinates
    SPOSITION pos = lstWndPos.GetHeadPosition();
    while (pos)
    {
        WndPos wp = lstWndPos.GetNext(pos);
        wp.rc.OffsetRect(rcParent.TopLeft());
        wp.pWnd->OnRelayout(wp.rc);
    }
}

SNSEND
