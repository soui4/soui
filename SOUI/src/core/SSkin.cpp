//////////////////////////////////////////////////////////////////////////
//   File Name: SSkinPool
//////////////////////////////////////////////////////////////////////////
#include "souistd.h"
#include "core/Sskin.h"
#include "helper/SDIBHelper.h"

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
// SSkinImgList
SSkinImgList::SSkinImgList()
    : m_nStates(1)
    , m_bTile(FALSE)
    , m_bVertical(FALSE)
    , m_filterLevel(kUndef_FilterLevel)
    , m_bAutoFit(TRUE)
    , m_bLazyLoad(FALSE)
{
}

SSkinImgList::~SSkinImgList()
{
}

SIZE SSkinImgList::GetSkinSize() const
{
    SIZE ret = { 0, 0 };
    if (GetImage())
        ret = GetImage()->Size();
    if (m_bVertical)
        ret.cy /= m_nStates;
    else
        ret.cx /= m_nStates;
    return ret;
}

int SSkinImgList::GetStates() const
{
    return m_nStates;
}

void SSkinImgList::OnInitFinished(IXmlNode *xmlNode)
{
    __baseCls::OnInitFinished(xmlNode);
    if (!m_bLazyLoad && !m_strSrc.IsEmpty())
    {
        m_pImg.Attach(LOADIMAGE2(m_strSrc));
    }
}

void SSkinImgList::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    if (!GetImage())
        return;
    SIZE sz = GetSkinSize();
    RECT rcSrc = { 0, 0, sz.cx, sz.cy };
    if (m_bVertical)
        OffsetRect(&rcSrc, 0, iState * sz.cy);
    else
        OffsetRect(&rcSrc, iState * sz.cx, 0);
    pRT->DrawBitmapEx(rcDraw, GetImage(), &rcSrc, GetExpandMode(), byAlpha);
}

UINT SSkinImgList::GetExpandMode() const
{
    if (m_bAutoFit)
        return MAKELONG(m_bTile ? EM_TILE : EM_STRETCH, m_filterLevel);
    else
        return MAKELONG(EM_NULL, m_filterLevel);
}

HRESULT SSkinImgList::OnAttrSrc(const SStringW &value, BOOL bLoading)
{
    m_strSrc = value;
    if (!bLoading)
    {
        m_pImg.Attach(LOADIMAGE2(m_strSrc));
    }
    return S_OK;
}

bool SSkinImgList::SetImage(IBitmapS *pImg)
{
    m_pImg = pImg;
    m_strSrc.Empty();
    m_bLazyLoad = FALSE;
    return true;
}

IBitmapS *SSkinImgList::GetImage() const
{
    if (m_pImg)
        return m_pImg;
    if (m_bLazyLoad && !m_strSrc.IsEmpty())
    {
        m_pImg.Attach(LOADIMAGE2(m_strSrc));
        m_strSrc.Empty();
    }
    return m_pImg;
}

void SSkinImgList::OnColorize(COLORREF cr)
{
    if (!m_bEnableColorize)
        return;
    if (cr == m_crColorize)
        return;
    m_crColorize = cr;

    IBitmapS *pImg = GetImage();
    if (m_imgBackup)
    { // restore
        LPCVOID pSrc = m_imgBackup->GetPixelBits();
        LPVOID pDst = pImg->LockPixelBits();
        memcpy(pDst, pSrc, pImg->Width() * pImg->Height() * 4);
        pImg->UnlockPixelBits(pDst);
    }
    else
    {
        if (!pImg)
            return;
        if (S_OK != pImg->Clone(&m_imgBackup))
            return;
    }

    if (cr != 0)
        SDIBHelper::Colorize(pImg, cr);
    else
        m_imgBackup = NULL; // free backup
}

void SSkinImgList::_Scale(ISkinObj *skinObj, int nScale)
{
    __baseCls::_Scale(skinObj, nScale);
    SSkinImgList *pRet = sobj_cast<SSkinImgList>(skinObj);
    pRet->m_nStates = m_nStates;
    pRet->m_bTile = m_bTile;
    pRet->m_bVertical = m_bVertical;
    pRet->m_filterLevel = m_filterLevel;
    pRet->m_bAutoFit = m_bAutoFit;
    pRet->m_state2Index = m_state2Index;
    pRet->m_bLazyLoad = FALSE;

    CSize szSkin = GetSkinSize();
    szSkin.cx = MulDiv(szSkin.cx, nScale, 100);
    szSkin.cy = MulDiv(szSkin.cy, nScale, 100);
    if (m_bVertical)
    {
        szSkin.cy *= m_nStates;
    }
    else
    {
        szSkin.cx *= m_nStates;
    }

    if (m_imgBackup)
    {
        m_imgBackup->Scale2(&pRet->m_imgBackup, szSkin.cx, szSkin.cy, kHigh_FilterLevel);
    }
    IBitmapS *pImg = GetImage();
    if (pImg)
    {
        m_pImg->Scale2(&pRet->m_pImg, szSkin.cx, szSkin.cy, kHigh_FilterLevel);
    }
}

//////////////////////////////////////////////////////////////////////////
//  SSkinImgCenter
void SSkinImgCenter::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    SIZE szSkin = GetSkinSize();
    CRect rcTarget = *rcDraw;
    CPoint pt;
    pt.x = rcTarget.left + (rcTarget.Width() - szSkin.cx) / 2;
    pt.y = rcTarget.top + (rcTarget.Height() - szSkin.cy) / 2;

    rcTarget = CRect(pt, szSkin);

    RECT rcSrc = { 0, 0, szSkin.cx, szSkin.cy };
    if (m_bVertical)
        OffsetRect(&rcSrc, 0, iState * szSkin.cy);
    else
        OffsetRect(&rcSrc, iState * szSkin.cx, 0);

    pRT->DrawBitmapEx(rcTarget, GetImage(), &rcSrc, GetExpandMode(), byAlpha);
}

//////////////////////////////////////////////////////////////////////////
//  SSkinImgFrame
SSkinImgFrame::SSkinImgFrame()
    : m_rcMargin(0, 0, 0, 0)
{
}

void SSkinImgFrame::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    if (!GetImage())
        return;
    SIZE sz = GetSkinSize();
    CPoint pt;
    if (IsVertical())
        pt.y = sz.cy * iState;
    else
        pt.x = sz.cx * iState;
    CRect rcSour(pt, sz);
    pRT->DrawBitmap9Patch(rcDraw, GetImage(), &rcSour, &m_rcMargin, GetExpandMode(), byAlpha);
}

UINT SSkinImgFrame::GetExpandMode() const
{
    return MAKELONG(m_bTile ? EM_TILE : EM_STRETCH, m_filterLevel);
}

void SSkinImgFrame::_Scale(ISkinObj *skinObj, int nScale)
{
    SSkinImgList::_Scale(skinObj, nScale);
    SSkinImgFrame *pClone = sobj_cast<SSkinImgFrame>(skinObj);
    pClone->m_rcMargin.left = MulDiv(m_rcMargin.left, nScale, 100);
    pClone->m_rcMargin.top = MulDiv(m_rcMargin.top, nScale, 100);
    pClone->m_rcMargin.right = MulDiv(m_rcMargin.right, nScale, 100);
    pClone->m_rcMargin.bottom = MulDiv(m_rcMargin.bottom, nScale, 100);
}

//////////////////////////////////////////////////////////////////////////
// SSkinButton
SSkinButton::SSkinButton()
    : m_nCornerRadius(2)
    , m_fCornerPercent(0.0)
{
    m_colors.m_crBorder[0] = RGB(0x70, 0x70, 0x70);
    m_colors.m_crBorder[1] = CR_INVALID; // 不改变 原有的效果
    m_colors.m_crBorder[2] = CR_INVALID;
    m_colors.m_crBorder[3] = CR_INVALID;

    m_colors.m_crUp[0] = (RGB(0xEE, 0xEE, 0xEE));
    m_colors.m_crDown[0] = (RGB(0xD6, 0xD6, 0xD6));
    m_colors.m_crUp[1] = (RGB(0xEE, 0xEE, 0xEE));
    m_colors.m_crDown[1] = (RGB(0xE0, 0xE0, 0xE0));
    m_colors.m_crUp[2] = (RGB(0xCE, 0xCE, 0xCE));
    m_colors.m_crDown[2] = (RGB(0xC0, 0xC0, 0xC0));
    m_colors.m_crUp[3] = (RGB(0x8E, 0x8E, 0x8E));
    m_colors.m_crDown[3] = (RGB(0x80, 0x80, 0x80));
}

void SSkinButton::_DrawByIndex(IRenderTarget *pRT, LPCRECT prcDraw, int iState, BYTE byAlpha) const
{
    int nCorner = m_nCornerRadius;
    if (m_fCornerPercent > 0.0)
    {
        int nW = prcDraw->right - prcDraw->left;
        int nH = prcDraw->bottom - prcDraw->top;
        nCorner = (nW < nH) ? (int)(nW * m_fCornerPercent) : (int)(nH * m_fCornerPercent);
    }
    // 只有 在 需要渐变的情况下 才 需要 这个
    if (m_colors.m_crUp[iState] != m_colors.m_crDown[iState])
    {
        CRect rcDraw = *prcDraw;
        rcDraw.DeflateRect(1, 1);
        CPoint ptCorner(nCorner, nCorner);
        GradientItem gradients[2] = { m_colors.m_crUp[iState], 0.0f, m_colors.m_crDown[iState], 1.0f };
        pRT->DrawGradientRect(rcDraw, TRUE, ptCorner, gradients, 2, byAlpha);
    }
    else
    {
        SColor cr(m_colors.m_crDown[iState]);
        cr.updateAlpha(byAlpha);
        if (nCorner > 0)
            pRT->FillSolidRoundRect(prcDraw, CPoint(nCorner, nCorner), cr.toCOLORREF());
        else
            pRT->FillSolidRect(prcDraw, cr.toCOLORREF());
    }

    if (CR_INVALID == m_colors.m_crBorder[iState]) //  不改变 原因的 效果
        iState = 0;
    // 画 边框
    SAutoRefPtr<IPenS> pPen, pOldPen;
    pRT->CreatePen(PS_SOLID, m_colors.m_crBorder[iState], 1, &pPen);
    pRT->SelectObject(pPen, (IRenderObj **)&pOldPen);
    pRT->DrawRoundRect(prcDraw, CPoint(nCorner, nCorner));
    pRT->SelectObject(pOldPen, NULL);
}

int SSkinButton::GetStates() const
{
    return 4;
}

void SSkinButton::SetColors(COLORREF crUp[4], COLORREF crDown[4], COLORREF crBorder[4])
{
    memcpy(m_colors.m_crUp, crUp, 4 * sizeof(COLORREF));
    memcpy(m_colors.m_crDown, crDown, 4 * sizeof(COLORREF));
    memcpy(m_colors.m_crBorder, crBorder, 4 * sizeof(COLORREF));
}

void SSkinButton::OnColorize(COLORREF cr)
{
    if (!m_bEnableColorize)
        return;
    if (m_crColorize == cr)
        return;
    if (cr == 0)
    {
        memcpy(&m_colors, &m_colorsBackup, sizeof(BTNCOLORS));
        m_crColorize = 0;
    }
    else
    {
        if (m_crColorize != 0)
        { //从备份里获取数据
            memcpy(&m_colors, &m_colorsBackup, sizeof(BTNCOLORS));
        }
        else
        { //将数据备份
            memcpy(&m_colorsBackup, &m_colors, sizeof(BTNCOLORS));
        }
        m_crColorize = cr;

        //调整颜色值
        for (int i = 0; i < 4; i++)
        {
            SDIBHelper::Colorize(m_colors.m_crBorder[i], m_crColorize);
            SDIBHelper::Colorize(m_colors.m_crDown[i], m_crColorize);
            SDIBHelper::Colorize(m_colors.m_crUp[i], m_crColorize);
        }
    }
}

ISkinObj *SSkinButton::Scale(int nScale)
{
    return NULL;
}

//////////////////////////////////////////////////////////////////////////
// SSkinGradation
SSkinGradation::SSkinGradation()
    : m_bVert(TRUE)
    , m_crFrom(CR_INVALID)
    , m_crTo(CR_INVALID)
{
}

void SSkinGradation::_DrawByIndex(IRenderTarget *pRT, LPCRECT prcDraw, int iState, BYTE byAlpha) const
{
    GradientItem gradients[2] = { m_crFrom, 0.0f, m_crTo, 1.0f };
    pRT->DrawGradientRect(prcDraw, m_bVert, CPoint(), gradients, 2, byAlpha);
}

ISkinObj *SSkinGradation::Scale(int nScale)
{
    return NULL;
}

//////////////////////////////////////////////////////////////////////////
SGradientDesc::SGradientDesc()
{
    // set default to linear gradient
    m_type = linear;
    m_angle = 0.0f;
    m_radius = SLayoutSize(100);
    m_centerX = m_centerY = 0.5;
}

static int GradientItemCmp(const void *_p1, const void *_p2)
{
    const GradientItem *p1 = (const GradientItem *)_p1;
    const GradientItem *p2 = (const GradientItem *)_p2;
    float diff = p1->pos - p2->pos;
    if (diff > 0.0f)
        return 1;
    else if (diff < 0.0f)
        return -1;
    else
        return 0;
}

static BOOL ParseGradientColors(const SStringW &value, SArray<GradientItem> &output)
{
    SStringWList lstInfo;
    SplitString(value, ',', lstInfo);

    for (UINT i = 0; i < lstInfo.GetCount(); i++)
    {
        SStringWList lstColorInfo;
        SplitString(lstInfo[i], '|', lstColorInfo);
        GradientItem gradient;
        if (lstColorInfo.GetCount() == 2)
        {
            gradient.cr = GETCOLOR(lstColorInfo[0]);
            gradient.pos = (float)_wtof(lstColorInfo[1]);
        }
        else
        {
            gradient.cr = GETCOLOR(lstColorInfo[0]);
            gradient.pos = 1.0f * i / (lstInfo.GetCount() - 1);
        }
        output.Add(gradient);
    }
    qsort(output.GetData(), output.GetCount(), sizeof(GradientItem), GradientItemCmp);
    if (output.GetCount() < 2)
        return FALSE;
    output[0].pos = 0.0f;
    output[output.GetCount() - 1].pos = 1.0f;
    return TRUE;
}

HRESULT SGradientDesc::OnAttrColors(const SStringW &value, BOOL bLoading)
{
    m_arrGradient.RemoveAll();
    return ParseGradientColors(value, m_arrGradient) ? S_FALSE : E_INVALIDARG;
}

GradientInfo SGradientDesc::GetGradientInfo(int nScale) const
{
    GradientInfo ret;
    ret.type = m_type;
    switch (ret.type)
    {
    case linear:
        ret.angle = m_angle;
        break;
    case radial:
        ret.radius = (float)m_radius.toPixelSize(nScale);
        break;
    case sweep:
        ret.center.fX = m_centerX;
        ret.center.fY = m_centerY;
        break;
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////
SSkinGradation2::SSkinGradation2()
{
    m_ptCorner = SPoint::Make(0.f, 0.f);
    m_szCorner[0].setInvalid();
    m_szCorner[1].setInvalid();
    m_bEnableScale = false;
}

void SSkinGradation2::_DrawByIndex(IRenderTarget *pRT, LPCRECT prcDraw, int iState, BYTE byAlpha) const
{
    CRect rc(prcDraw);
    CPoint ptCorner;
    if (m_szCorner[0].isValid() && m_szCorner[1].isValid())
    {
        ptCorner.x = m_szCorner[0].toPixelSize(GetScale());
        ptCorner.y = m_szCorner[1].toPixelSize(GetScale());
    }
    else
    {
        ptCorner.x = (int)(rc.Width() / 2 * m_ptCorner.fX);
        ptCorner.y = (int)(rc.Height() / 2 * m_ptCorner.fY);
    }
    GradientInfo info = GetGradientInfo(GetScale());
    pRT->DrawGradientRectEx(prcDraw, ptCorner, m_arrGradient.GetData(), (int)m_arrGradient.GetCount(), &info, GetAlpha());
}

ISkinObj *SSkinGradation2::Scale(int nScale)
{
    return NULL;
}
//////////////////////////////////////////////////////////////////////////
// SScrollbarSkin
SSkinScrollbar::SSkinScrollbar()
    : m_nMargin(0)
    , m_bHasGripper(FALSE)
    , m_bHasInactive(FALSE)
{
}

CRect SSkinScrollbar::GetPartRect(int nSbCode, int nState, BOOL bVertical) const
{
    CSize sz = GetSkinSize();
    CSize szFrame(sz.cx / 9, sz.cx / 9);
    if (nSbCode == SB_CORNOR)
    {
        return CRect(CPoint(szFrame.cx * 8, 0), szFrame);
    }
    else if (nSbCode == SB_THUMBGRIPPER)
    {
        return CRect(CPoint(szFrame.cx * 8, (1 + (bVertical ? 0 : 1)) * szFrame.cy), szFrame);
    }
    else
    {
        if (nState == SBST_INACTIVE && !m_bHasInactive)
        {
            nState = SBST_NORMAL;
        }
        CRect rcRet;
        int iPart = -1;
        switch (nSbCode)
        {
        case SB_LINEUP:
            iPart = 0;
            break;
        case SB_LINEDOWN:
            iPart = 1;
            break;
        case SB_THUMBTRACK:
            iPart = 2;
            break;
        case SB_PAGEUP:
        case SB_PAGEDOWN:
            iPart = 3;
            break;
        }
        if (!bVertical)
            iPart += 4;

        return CRect(CPoint(szFrame.cx * iPart, szFrame.cy * nState), szFrame);
    }
}

void SSkinScrollbar::_DrawByState(IRenderTarget *pRT, LPCRECT prcDraw, DWORD dwState, BYTE byAlpha) const
{
    if (!GetImage())
        return;
    int nSbCode = LOWORD(dwState);
    int nState = LOBYTE(HIWORD(dwState));
    BOOL bVertical = HIBYTE(HIWORD(dwState));
    CRect rcMargin(0, 0, 0, 0);
    if (bVertical)
        rcMargin.top = m_nMargin, rcMargin.bottom = m_nMargin;
    else
        rcMargin.left = m_nMargin, rcMargin.right = m_nMargin;

    CRect rcSour = GetPartRect(nSbCode, nState, bVertical);

    pRT->DrawBitmap9Patch(prcDraw, GetImage(), &rcSour, &rcMargin, m_bTile ? EM_TILE : EM_STRETCH, byAlpha);

    if (nSbCode == SB_THUMBTRACK && m_bHasGripper)
    {
        rcSour = GetPartRect(SB_THUMBGRIPPER, 0, bVertical);
        CRect rcDraw = *prcDraw;

        if (bVertical)
            rcDraw.top += (rcDraw.Height() - rcSour.Height()) / 2, rcDraw.bottom = rcDraw.top + rcSour.Height();
        else
            rcDraw.left += (rcDraw.Width() - rcSour.Width()) / 2, rcDraw.right = rcDraw.left + rcSour.Width();
        pRT->DrawBitmap9Patch(&rcDraw, GetImage(), &rcSour, &rcMargin, m_bTile ? EM_TILE : EM_STRETCH, byAlpha);
    }
}

void SSkinScrollbar::_Scale(ISkinObj *skinObj, int nScale)
{
    __baseCls::_Scale(skinObj, nScale);

    SSkinScrollbar *pRet = sobj_cast<SSkinScrollbar>(skinObj);
    pRet->m_nMargin = MulDiv(m_nMargin, nScale, 100);
    pRet->m_bHasInactive = m_bHasInactive;
    pRet->m_bHasGripper = m_bHasGripper;
}

int SSkinScrollbar::GetIdealSize() const
{
    if (!GetImage())
        return 0;
    return GetImage()->Width() / 9;
}

//////////////////////////////////////////////////////////////////////////
// SSkinColor
SSkinColorRect::SSkinColorRect()
    : m_nRadius(0)
    , m_fCornerPercent(0.0)
    , m_nBorderWidth(0)
{
    m_crStates[0] = RGBA(255, 255, 255, 255);
    m_crStates[1] = CR_INVALID;
    m_crStates[2] = CR_INVALID;
    m_crStates[3] = CR_INVALID;
    m_crBorders[0] = m_crBorders[1] = m_crBorders[2] = m_crBorders[3] = CR_INVALID;
}

SSkinColorRect::~SSkinColorRect()
{
}

void SSkinColorRect::_DrawByIndex(IRenderTarget *pRT, LPCRECT prcDraw, int iState, BYTE byAlpha) const
{
    if (iState > 3)
        return;

    int nCorner = m_nRadius;
    if (m_fCornerPercent > 0.0)
    {
        int nW = prcDraw->right - prcDraw->left;
        int nH = prcDraw->bottom - prcDraw->top;
        nCorner = (nW < nH) ? (int)(nW * m_fCornerPercent) : (int)(nH * m_fCornerPercent);
    }
    int iBgColor = iState;
    if (m_crStates[iBgColor] == CR_INVALID)
        iBgColor = 0;
    if (m_crStates[iBgColor] != CR_INVALID)
    {
        SColor cr(m_crStates[iBgColor]);
        cr.updateAlpha(byAlpha);
        if (nCorner > 0)
            pRT->FillSolidRoundRect(prcDraw, CPoint(nCorner, nCorner), cr.toCOLORREF());
        else
            pRT->FillSolidRect(prcDraw, cr.toCOLORREF());
    }
    int iBorderColor = iState;
    if (m_crBorders[iBorderColor] == CR_INVALID)
        iBorderColor = 0;
    if (m_crBorders[iBorderColor] != CR_INVALID && m_nBorderWidth > 0)
    {
        SAutoRefPtr<IPenS> pen, oldPen;
        pRT->CreatePen(PS_SOLID, m_crBorders[iBorderColor], m_nBorderWidth, (IPenS **)&pen);
        pRT->SelectObject(pen, (IRenderObj **)&oldPen);
        if (nCorner > 0)
            pRT->DrawRoundRect(prcDraw, CPoint(nCorner, nCorner));
        else
            pRT->DrawRectangle(prcDraw);
        pRT->SelectObject(oldPen, NULL);
    }
}

int SSkinColorRect::GetStates() const
{
    int nStates = 4;
    for (int i = 3; i >= 0; i--)
    {
        if (m_crStates[i] == CR_INVALID)
            nStates--;
        else
            break;
    }
    return nStates;
}

ISkinObj *SSkinColorRect::Scale(int nScale)
{
    return NULL;
}

//////////////////////////////////////////////////////////////////////////

SSkinShape::SSkinShape()
    : m_shape(rectangle)
{
}

void SSkinShape::OnInitFinished(IXmlNode *pNode)
{
    __baseCls::OnInitFinished(pNode);
    SXmlNode xmlNode(pNode);
    SXmlNode xmlSolid = xmlNode.child(SShapeSolid::GetClassName());
    if (xmlSolid)
    {
        if (!m_solid)
            m_solid.Attach(new SShapeSolid());
        m_solid->InitFromXml(&xmlSolid);
    }
    SXmlNode xmlBitmap = xmlNode.child(SShapeBitmap::GetClassName());
    if (xmlBitmap)
    {
        if (!m_bitmap)
            m_bitmap.Attach(new SShapeBitmap());
        m_bitmap->InitFromXml(&xmlBitmap);
    }

    SXmlNode xmlGrident = xmlNode.child(SGradientBrush::GetClassName());
    if (xmlGrident)
    {
        if (!m_gradient)
            m_gradient.Attach(new SGradientBrush());
        m_gradient->InitFromXml(&xmlGrident);
    }
    SXmlNode xmlSize = xmlNode.child(SShapeSize::GetClassName());
    if (xmlSize)
    {
        if (!m_shapeSize)
            m_shapeSize.Attach(new SShapeSize());
        m_shapeSize->InitFromXml(&xmlSize);
    }
    SXmlNode xmlStoke = xmlNode.child(SStroke::GetClassName());
    if (xmlStoke)
    {
        if (!m_stroke)
            m_stroke.Attach(new SStroke());
        m_stroke->InitFromXml(&xmlStoke);
    }
    SXmlNode xmlConner = xmlNode.child(SCornerSize::GetClassName());
    if (xmlConner)
    {
        if (!m_cornerSize)
            m_cornerSize.Attach(new SCornerSize());
        m_cornerSize->InitFromXml(&xmlConner);
    }
    SXmlNode xmlRatioConner = xmlNode.child(SRatioCornerSize::GetClassName());
    if (xmlRatioConner)
    {
        if (!m_ratioCornerSize)
            m_ratioCornerSize.Attach(new SRatioCornerSize());
        m_ratioCornerSize->InitFromXml(&xmlRatioConner);
    }

    SXmlNode xmlRing = xmlNode.child(SShapeRing::GetClassName());
    if (xmlRing)
    {
        if (!m_ringParam)
            m_ringParam.Attach(new SShapeRing());
        m_ringParam->InitFromXml(&xmlRing);
    }
}

void SSkinShape::_Scale(ISkinObj *pObj, int nScale)
{
    __baseCls::_Scale(pObj, nScale);

    SSkinShape *pRet = sobj_cast<SSkinShape>(pObj);
    SASSERT(pRet);
    pRet->m_solid = m_solid;
    pRet->m_bitmap = m_bitmap; // todo:hjx
    pRet->m_gradient = m_gradient;
    pRet->m_shape = m_shape;
    pRet->m_shapeSize = m_shapeSize;
    pRet->m_cornerSize = m_cornerSize;
    pRet->m_stroke = m_stroke;
    pRet->m_ringParam = m_ringParam;
}

SIZE SSkinShape::GetSkinSize() const
{
    if (!m_shapeSize)
        return CSize();

    return CSize(m_shapeSize->m_width.toPixelSize(GetScale()), m_shapeSize->m_height.toPixelSize(GetScale()));
}

void SSkinShape::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    CRect rcDest(rcDraw);
    SAutoRefPtr<IBrushS> pBrush, oldBrush;
    if (m_solid)
    {
        pBrush = m_solid->CreateBrush(pRT, byAlpha);
    }
    else if (m_gradient != NULL)
    {
        pBrush = m_gradient->CreateBrush(pRT, GetScale(), byAlpha);
    }
    else if (m_bitmap)
    {
        pBrush = m_bitmap->CreateBrush(pRT, byAlpha);
    }

    SAutoRefPtr<IPenS> pPen, oldPen;
    if (m_stroke != NULL)
    {
        int nPenWidth = m_stroke->m_width.toPixelSize(GetScale());
        SColor color(m_stroke->m_color, byAlpha);
        pRT->CreatePen(m_stroke->GetStyle(), color.toCOLORREF(), nPenWidth, &pPen);
        rcDest.DeflateRect(nPenWidth / 2, nPenWidth / 2);
    }

    CPoint ptCorner = GetCornerSize(rcDest);
    if (pBrush)
    {
        pRT->SelectObject(pBrush, (IRenderObj **)&oldBrush);
        switch (m_shape)
        {
        case rectangle:
            if (ptCorner.x != 0 || ptCorner.y != 0)
                pRT->FillRoundRect(&rcDest, ptCorner);
            else
                pRT->FillRectangle(&rcDest);
            break;
        case oval:
            pRT->FillEllipse(&rcDest);
            break;
        }
        pRT->SelectObject(oldBrush, NULL);
    }
    if (pPen)
    {
        pRT->SelectObject(pPen, (IRenderObj **)&oldPen);
        ptCorner = GetCornerSize(rcDest);
        switch (m_shape)
        {
        case rectangle:
            if (ptCorner.x != 0 || ptCorner.y != 0)
                pRT->DrawRoundRect(&rcDest, ptCorner);
            else
                pRT->DrawRectangle(&rcDest);
            break;
        case oval:
            pRT->DrawEllipse(&rcDest);
            break;
        case ring:
        {
            POINT ptCenter = { (rcDest.left + rcDest.right) / 2, (rcDest.top + rcDest.bottom) / 2 };
            int nRadius = smin(rcDest.right - rcDest.left, rcDest.bottom - rcDest.top) / 2;
            RECT rcRing = { ptCenter.x - nRadius, ptCenter.y - nRadius, ptCenter.x + nRadius, ptCenter.y + nRadius };
            float startAngle = 0;
            float endAngle = 360;
            if (m_ringParam)
            {
                startAngle = m_ringParam->m_startAngle;
                endAngle = m_ringParam->m_sweepAngle;
            }
            pRT->DrawArc(&rcRing, startAngle, endAngle, false);
        }
        break;
        }
        pRT->SelectObject(oldPen, NULL);
    }
}

int SSkinShape::GetStates() const
{
    return 1;
}

POINT SSkinShape::GetCornerSize(const CRect &rc) const
{
    if (m_cornerSize)
        return m_cornerSize->GetConner(GetScale());
    if (m_ratioCornerSize)
        return m_ratioCornerSize->GetConner(rc);
    return CPoint();
}

SSkinShape::SStroke::SStroke()
    : m_color(CR_INVALID)
    , m_style(PS_SOLID)
    , m_endStyle(0)
    , m_joinStyle(0)
{
}

int SSkinShape::SStroke::GetStyle() const
{
    int ret = m_style | m_endStyle | m_joinStyle;
    if (m_endStyle || m_joinStyle)
    {
        ret |= PS_GEOMETRIC;
    }
    return ret;
}

IBrushS *SSkinShape::SShapeSolid::CreateBrush(IRenderTarget *pRT, BYTE byAlpha)
{
    if (m_crSolid == CR_INVALID)
        return NULL;
    SColor color(m_crSolid, byAlpha);
    IBrushS *ret = NULL;
    pRT->CreateSolidColorBrush(color.toCOLORREF(), &ret);
    return ret;
}

IBrushS *SSkinShape::SShapeBitmap::CreateBrush(IRenderTarget *pRT, BYTE byAlpha)
{
    if (!m_pImg)
        return NULL;
    IBrushS *ret = NULL;
    pRT->CreateBitmapBrush(m_pImg, m_tileX, m_tileY, &ret);
    return ret;
}

IBrushS *SSkinShape::SGradientBrush::CreateBrush(IRenderTarget *pRT, int nScale, BYTE byAlpha) const
{
    if (m_arrGradient.GetCount() < 2)
        return NULL;
    IBrushS *ret = NULL;
    GradientInfo info = GetGradientInfo(nScale);
    pRT->CreateGradientBrush(m_arrGradient.GetData(), (int)m_arrGradient.GetCount(), &info, byAlpha, kRepeat_TileMode, &ret);
    return ret;
}

HRESULT SSkinShape::SCornerSize::OnAttrRadius(const SStringW strValue, BOOL bLoading)
{
    SStringWList values;
    size_t nValues = SplitString(strValue, L',', values);
    if (nValues == 1)
    {
        m_radiusX = m_radiusY = GETLAYOUTSIZE(values[0]);
        return S_OK;
    }
    else if (nValues == 2)
    {
        m_radiusX = GETLAYOUTSIZE(values[0]);
        m_radiusY = GETLAYOUTSIZE(values[1]);
        return S_OK;
    }
    return E_INVALIDARG;
}

HRESULT SSkinShape::SRatioCornerSize::OnAttrRadius(const SStringW strValue, BOOL bLoading)
{
    SStringWList values;
    size_t nValues = SplitString(strValue, L',', values);
    if (nValues == 1)
    {
        swscanf_s(strValue.c_str(), L"%f", &m_radius.fX);
        m_radius.fY = m_radius.fX;
        ;
        return S_OK;
    }
    else if (nValues == 2)
    {
        swscanf_s(values[0].c_str(), L"%f", &m_radius.fX);
        swscanf_s(values[1].c_str(), L"%f", &m_radius.fY);
        return S_OK;
    }
    return E_INVALIDARG;
}

//////////////////////////////////////////////////////////////////////////
// SSKinGroup
int SSKinGroup::GetStates() const
{
    return 4;
}

void SSKinGroup::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    if ((int)iState >= GetStates())
        return;
    ISkinObj *pSkin = m_skins[iState];
    if (!pSkin)
        return;
    pSkin->DrawByIndex2(pRT, rcDraw, 0, byAlpha);
}

SIZE SSKinGroup::GetSkinSize() const
{
    for (int i = 0; i < GetStates(); i++)
    {
        if (m_skins[i])
            return m_skins[i]->GetSkinSize();
    }
    return CSize();
}

void SSKinGroup::_Scale(ISkinObj *skinObj, int nScale)
{
    __baseCls::_Scale(skinObj, nScale);
    SSKinGroup *pRet = sobj_cast<SSKinGroup>(skinObj);
    for (int i = 0; i < ARRAYSIZE(m_skins); i++)
    {
        if (!m_skins[i])
            continue;
        pRet->m_skins[i].Attach(m_skins[i]->Scale(nScale));
    }
}

/*
IBitmap中的内存为RGBA格式，.9中使用alpha通道==0或者255来确定如何拉伸
*/
HRESULT SSkinImgFrame2::OnAttrSrc(const SStringW &strValue, BOOL bLoading)
{
    IBitmapS *pImg = LOADIMAGE2(strValue);
    if (!pImg)
        return E_FAIL;
    int nWid = pImg->Width();
    int nHei = pImg->Height();
    if (nWid < 2 || nHei < 2)
        return E_INVALIDARG;
    LPBYTE pBuf = (LPBYTE)pImg->LockPixelBits();

    int left, right, top, bottom;
    //检测第一扫描行中定义的left,right位置
    LPBYTE p = pBuf;
    int i = 1;
    while (i < nWid - 1 && p[3] == 0)
        i++, p += 4;
    left = i - 1;

    while (i < nWid - 1 && p[3] != 0)
        i++, p += 4;
    right = i - 1;

    //检测第一列中定义的top,bottom位置
    i = 1;
    p = pBuf + (nWid * 4);
    while (i < nHei - 1 && p[3] == 0)
        i++, p += nWid * 4;
    top = i - 1;
    while (i < nHei - 1 && p[3] != 0)
        i++, p += nWid * 4;
    bottom = i - 1;

    m_rcMargin.left = left;
    m_rcMargin.right = nWid - 2 - right;
    m_rcMargin.top = top;
    m_rcMargin.bottom = nHei - 2 - bottom;

    HRESULT hRet = S_OK;
    IBitmapS *pImgCenter = NULL;
    if (!pImg->GetRenderFactory()->CreateBitmap(&pImgCenter))
    {
        return E_OUTOFMEMORY;
    }
    hRet = pImgCenter->Init(nWid - 2, nHei - 2, NULL);
    if (hRet != S_OK)
    {
        return hRet;
    }
    LPBYTE pBuf2 = (LPBYTE)pImgCenter->LockPixelBits();

    LPBYTE pSrc = pBuf + (nWid * 4);
    LPBYTE pDst = pBuf2;

    i = 1;
    pSrc += 4;
    while (i < nHei - 1)
    {
        memcpy(pDst, pSrc, (nWid - 2) * 4);
        pDst += (nWid - 2) * 4;
        pSrc += nWid * 4;
        i++;
    }

    pImg->UnlockPixelBits(pBuf);
    pImgCenter->UnlockPixelBits(pBuf2);

    SetImage(pImgCenter);
    pImgCenter->Release();
    pImg->Release();

    return hRet;
}

SNSEND