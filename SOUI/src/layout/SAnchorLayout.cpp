#include <souistd.h>
#include <layout/SAnchorLayout.h>
#include <helper/SplitString.h>
#include <core/SWnd.h>
#pragma warning(push)
#pragma warning(disable : 4985) // disable the warning message during the include
#include <math.h>               // this is where I would normally get the warning message
#pragma warning(pop)

SNSBEGIN

SAnchorLayoutParam::SAnchorLayoutParam()
{
    Clear();
}

SAnchorLayoutParam::SAnchorLayoutParam(const SAnchorLayoutParam &other)
{
    width = other.width;
    height = other.height;
    pos = other.pos;
}

SAnchorLayoutParam::~SAnchorLayoutParam()
{
}
BOOL SAnchorLayoutParam::IsMatchParent(ORIENTATION orientation) const
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

BOOL SAnchorLayoutParam::IsSpecifiedSize(ORIENTATION orientation) const
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

BOOL SAnchorLayoutParam::IsWrapContent(ORIENTATION orientation) const
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

BOOL SAnchorLayoutParam::GetSpecifiedSize(ORIENTATION orientation, LAYOUTSIZE *pLayoutSize) const
{
    if (orientation == Vert)
    {
        *pLayoutSize = height;
        return TRUE;
    }
    else if (orientation == Horz)
    {
        *pLayoutSize = width;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

HRESULT SAnchorLayoutParam::OnAttrOffset(const SStringW &strValue, BOOL bLoading)
{
    float fx, fy;
    if (2 != swscanf(strValue, L"%f,%f", &fx, &fy))
    {
        return E_FAIL;
    }
    pos.fOffsetX = fx;
    pos.fOffsetY = fy;
    return S_OK;
}

BOOL SAnchorLayoutParam::ParsePosition(const SStringW &strValue, AnchorPos &aniPos)
{
    if (strValue.IsEmpty())
    {
        aniPos.type = APT_Invalid;
        return FALSE;
    }
    SStringWList posStr;
    int ret = SplitString(strValue, L',', posStr);
    if (ret < 2)
    {
        aniPos.type = APT_Invalid;
        return FALSE;
    }
    aniPos.x = GETLAYOUTSIZE(posStr[0]);
    aniPos.y = GETLAYOUTSIZE(posStr[1]);
    if (ret > 2)
    {
        aniPos.type = _wtoi(posStr[2]);
    }
    else
    {
        aniPos.type = APT_Left_Top;
    }
    return TRUE;
}

HRESULT SAnchorLayoutParam::OnAttrPos(const SStringW &strValue, BOOL bLoading)
{
    ParsePosition(strValue, pos);
    return S_OK;
}

HRESULT SAnchorLayoutParam::OnAttrSize(const SStringW &strValue, BOOL bLoading)
{
    SStringWList szStr;
    if (2 != SplitString(strValue, L',', szStr))
        return E_FAIL;

    OnAttrWidth(szStr[0], bLoading);
    OnAttrHeight(szStr[1], bLoading);
    return S_OK;
}

HRESULT SAnchorLayoutParam::OnAttrHeight(const SStringW &strValue, BOOL bLoading)
{
    if (strValue.CompareNoCase(L"matchParent") == 0 || strValue.CompareNoCase(L"full") == 0)
        height.setMatchParent();
    else if (strValue.CompareNoCase(L"wrapContent") == 0)
        height.setWrapContent();
    else
        height = GETLAYOUTSIZE(strValue);
    return S_OK;
}
HRESULT SAnchorLayoutParam::OnAttrWidth(const SStringW &strValue, BOOL bLoading)
{
    if (strValue.CompareNoCase(L"matchParent") == 0 || strValue.CompareNoCase(L"full") == 0)
        width.setMatchParent();
    else if (strValue.CompareNoCase(L"wrapContent") == 0)
        width.setWrapContent();
    else
        width = GETLAYOUTSIZE(strValue);
    return S_OK;
}

void SAnchorLayoutParam::Clear()
{
    pos.type = APT_Invalid;
    pos.fOffsetX = pos.fOffsetY = 0.0f;
    width.setWrapContent();
    height.setWrapContent();
}

void SAnchorLayoutParam::SetMatchParent(ORIENTATION orientation)
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

void SAnchorLayoutParam::SetWrapContent(ORIENTATION orientation)
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

void SAnchorLayoutParam::SetSpecifiedSize(ORIENTATION orientation, const LAYOUTSIZE *pLayoutSize)
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

void *SAnchorLayoutParam::GetRawData()
{
    return (SAnchorLayoutParamStruct *)this;
}

ILayoutParam *SAnchorLayoutParam::Clone() const
{
    return new SAnchorLayoutParam(*this);
}

BOOL SAnchorLayoutParam::SetAnimatorValue(IPropertyValuesHolder *pHolder, float fraction, ANI_STATE state)
{
    SStringW strProp = pHolder->GetPropertyName();

    if (strProp.CompareNoCase(LayoutProperty::POSITION) == 0)
    {
        if (state == ANI_START)
        {
            if (m_aniPosHolder)
                return FALSE;
            m_aniPosHolder = pHolder;
        }
        else if (state == ANI_END)
        {
            pHolder->GetEndValue(&pos);
            m_aniPosHolder = NULL;
        }
        m_fAniFraction = fraction;
        return TRUE;
    }
    if (strProp.CompareNoCase(LayoutProperty::POSITION_X) == 0)
    {
        pHolder->GetAnimatedValue(fraction, &pos.x);
        return TRUE;
    }
    if (strProp.CompareNoCase(LayoutProperty::POSITION_Y) == 0)
    {
        pHolder->GetAnimatedValue(fraction, &pos.y);
        return TRUE;
    }
    if (strProp.CompareNoCase(LayoutProperty::OFFSET_X) == 0)
    {
        pHolder->GetAnimatedValue(fraction, &pos.fOffsetX);
        return TRUE;
    }
    if (strProp.CompareNoCase(LayoutProperty::OFFSET_Y) == 0)
    {
        pHolder->GetAnimatedValue(fraction, &pos.fOffsetY);
        return TRUE;
    }
    if (strProp.CompareNoCase(LayoutProperty::WIDTH) == 0)
    {
        SWindow::GetAnimatedLayoutSize(pHolder, fraction, width);
        return TRUE;
    }
    if (strProp.CompareNoCase(LayoutProperty::HEIGHT) == 0)
    {
        SWindow::GetAnimatedLayoutSize(pHolder, fraction, height);
        return TRUE;
    }
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////
CPoint SAnchorLayout::Anchor2Pos(const CRect &rcParent, int type)
{
    CPoint pt;
    switch (type)
    {
    case APT_Left_Top:
        pt.x = rcParent.left;
        pt.y = rcParent.top;
        break;
    case APT_Center_Top:
        pt.x = rcParent.CenterPoint().x;
        pt.y = rcParent.top;
        break;
    case APT_Right_Top:
        pt.x = rcParent.right;
        pt.y = rcParent.top;
        break;
    case APT_Left_Center:
        pt.x = rcParent.left;
        pt.y = rcParent.CenterPoint().y;
        break;
    case APT_Center_Center:
        pt.x = rcParent.CenterPoint().x;
        pt.y = rcParent.CenterPoint().y;
        break;
    case APT_Right_Center:
        pt.x = rcParent.right;
        pt.y = rcParent.CenterPoint().y;
        break;
    case APT_Left_Bottom:
        pt.x = rcParent.left;
        pt.y = rcParent.bottom;
        break;
    case APT_Center_Bottom:
        pt.x = rcParent.CenterPoint().x;
        pt.y = rcParent.bottom;
        break;
    case APT_Right_Bottom:
        pt.x = rcParent.right;
        pt.y = rcParent.bottom;
        break;
    default:
        break;
    }
    return pt;
}
SAnchorLayout::SAnchorLayout(void)
    : m_pfnPosition2Point(DefaultPosition2Point)
    , m_pUserData(NULL)
{
}

SAnchorLayout::~SAnchorLayout(void)
{
}

BOOL SAnchorLayout::IsParamAcceptable(const ILayoutParam *pLayoutParam) const
{
    return !!pLayoutParam->IsClass(SAnchorLayoutParam::GetClassName());
}

ILayoutParam *SAnchorLayout::CreateLayoutParam() const
{
    return new SAnchorLayoutParam();
}

SIZE SAnchorLayout::MeasureChildren(const IWindow *pParent, int nWidth, int nHeight) const
{
    return CSize();
}

POINT SAnchorLayout::DefaultPosition2Point(const AnchorPos &pos, const CRect &rcParent, const CSize &szChild, int nScale, void *userData)
{
    CPoint pt = Anchor2Pos(rcParent, pos.type);
    pt.x += pos.x.toPixelSize(nScale);
    pt.y += pos.y.toPixelSize(nScale);
    pt.Offset(pos.fOffsetX * szChild.cx, pos.fOffsetY * szChild.cy);
    return pt;
}
POINT SAnchorLayout::CalcPoint4Animator(const AnchorPos &start, const AnchorPos &end, float fraction, const CRect &rcParent, const CSize &szChild, int nScale) const
{
    POINT ptStart = m_pfnPosition2Point(start, rcParent, szChild, nScale, m_pUserData);
    if (end.type == APT_Invalid)
        return ptStart;
    POINT ptEnd = m_pfnPosition2Point(end, rcParent, szChild, nScale, m_pUserData);
    CPoint pt;
    pt.x = ptStart.x + (ptEnd.x - ptStart.x) * fraction;
    pt.y = ptStart.y + (ptEnd.y - ptStart.y) * fraction;
    return pt;
}

static int _InterpolateLayoutSize(const IWindow *pChild, int parentSize, int autoSize, const SLayoutSize &start, const SLayoutSize &end, float fraction)
{
    int nStart = start.isMatchParent() ? parentSize : start.isWrapContent() ? autoSize : start.toPixelSize(pChild->GetScale());
    int nEnd = end.isMatchParent() ? parentSize : end.isWrapContent() ? autoSize : end.toPixelSize(pChild->GetScale());
    return nStart + (nEnd - nStart) * fraction;
}
void SAnchorLayout::LayoutChildren(IWindow *pParent)
{
    CRect rcParent;
    pParent->GetChildrenLayoutRect(&rcParent);
    IWindow *pChild = pParent->GetNextLayoutIChild(NULL);
    while (pChild)
    {
        const SAnchorLayoutParam *pParam = (const SAnchorLayoutParam *)pChild->GetLayoutParam();
        SIZE szChild = { SIZE_WRAP_CONTENT, SIZE_WRAP_CONTENT };
        if (pParam->IsSpecifiedSize(Horz))
        {
            SLayoutSize layoutSize;
            pParam->GetSpecifiedSize(Horz, &layoutSize);
            szChild.cx = layoutSize.toPixelSize(pChild->GetScale());
        }
        else if (pParam->IsMatchParent(Horz))
        {
            szChild.cx = rcParent.Width();
        }
        if (pParam->IsSpecifiedSize(Vert))
        {
            SLayoutSize layoutSize;
            pParam->GetSpecifiedSize(Vert, &layoutSize);
            szChild.cy = layoutSize.toPixelSize(pChild->GetScale());
        }
        else if (pParam->IsMatchParent(Vert))
        {
            szChild.cy = rcParent.Height();
        }
        if (szChild.cx == SIZE_WRAP_CONTENT || szChild.cy == SIZE_WRAP_CONTENT)
        {
            CSize szCalc;
            ((IWindow *)pChild)->GetDesiredSize(&szCalc, szChild.cx, szChild.cy);
            if (szChild.cx == SIZE_WRAP_CONTENT)
            {
                szChild.cx = szCalc.cx;
            }
            if (szChild.cy == SIZE_WRAP_CONTENT)
            {
                szChild.cy = szCalc.cy;
            }
        }
        CPoint pt;
        if (SAutoRefPtr<IPropertyValuesHolder> pHolder = pParam->m_aniPosHolder)
        {
            float fraction = pParam->m_fAniFraction;
            CSize szAuto = szChild;
            if (pHolder->GetValueType() == PROP_TYPE_VARIANT && pHolder->GetValueSize() == sizeof(AnchorPos))
            {
                AnchorPos aniPos[2];
                int idx[2];
                float segFraction = pHolder->Fraction2Index(fraction, idx);
                pHolder->GetValueByIndex(idx[0], &aniPos[0], sizeof(AnchorPos));
                pHolder->GetValueByIndex(idx[1], &aniPos[1], sizeof(AnchorPos));
                pt = CalcPoint4Animator(aniPos[0], aniPos[1], segFraction, rcParent, szChild, pChild->GetScale());
            }
            if (szChild.cx == rcParent.Width())
            {
                pt.x = rcParent.left;
            }
            if (szChild.cy == rcParent.Height())
            {
                pt.y = rcParent.top;
            }
        }
        else
        {
            pt = m_pfnPosition2Point(pParam->pos, rcParent, szChild, pChild->GetScale(), m_pUserData);
            if (pParam->width.isMatchParent())
            {
                pt.x = rcParent.left;
            }
            if (pParam->height.isMatchParent())
            {
                pt.y = rcParent.top;
            }
        }
        ((SWindow *)pChild)->OnRelayout(CRect(pt, szChild));
        pChild = pParent->GetNextLayoutIChild(pChild);
    }
}

SNSEND
