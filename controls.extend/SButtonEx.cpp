#include "stdafx.h"
#include "SButtonEx.h"

namespace SOUI
{
SButtonEx::SButtonEx(void)
    : m_drawMode(HORZ_ICON_TEXT)
    , m_nSepSize(5)
    , m_hIcon(0)
    , m_nIconSize(32)
    , m_iIcon(0)
    , m_bIconVisible(true)
    , m_pIcon(NULL)
{
    m_bClipClient = TRUE;
	m_ptPushOffet[0]=m_ptPushOffet[1] = SLayoutSize(2);
}

SButtonEx::~SButtonEx(void)
{
    if (m_hIcon)
        DestroyIcon(m_hIcon);
}

CSize SButtonEx::GetDesiredSize(LPCRECT pRcContainer)
{
    CAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
    BeforePaintEx(pRT);
    return GetDesiredSize(pRT, pRcContainer);
}

CSize SButtonEx::GetDesiredSize(IRenderTarget *pRT, LPCRECT pRcContainer)
{
    CSize szIcon = GetIconSize();
    CSize szText;
    pRT->MeasureText(GetWindowText(), GetWindowText().GetLength(), &szText);

    CSize szRet;
    switch (m_drawMode)
    {
    case FREE_DRAW:
        szRet.cx = max(szIcon.cx + m_ptIcon[0].toPixelSize(GetScale()), szText.cx + m_ptText[0].toPixelSize(GetScale()));
        szRet.cy = max(szIcon.cy + m_ptIcon[1].toPixelSize(GetScale()), szText.cy + m_ptText[1].toPixelSize(GetScale()));
        break;
    case VERT_ICON_TEXT:
    case VERT_TEXT_ICON:
        szRet.cx = max(szIcon.cx, szText.cx);
        szRet.cy = szIcon.cy + szText.cy + m_nSepSize.toPixelSize(GetScale());
        break;
    case HORZ_ICON_TEXT:
    case HORZ_TEXT_ICON:
        szRet.cy = max(szIcon.cy, szText.cy);
        szRet.cx = szIcon.cx + szText.cx + m_nSepSize.toPixelSize(GetScale());
        break;
    default:
        SASSERT(FALSE);
        break;
    }
    CRect rcPadding = GetStyle().GetPadding();
    szRet.cx += rcPadding.left + rcPadding.right;
    szRet.cy += rcPadding.top + rcPadding.bottom;
    return szRet;
}

void SButtonEx::OnPaint(IRenderTarget *pRT)
{
    SPainter painter;
    BeforePaint(pRT, painter);

    CRect rcClient;
    GetClientRect(&rcClient);

    // draw background, copy from SButton
    if (m_pBgSkin)
    {
        if (m_byAlphaAni == 0xFF)
        { //不在动画过程中
            int iState = SState2Index::GetDefIndex(GetState(), true);
            m_pBgSkin->DrawByIndex(pRT, rcClient, iState);
        }
        else
        { //在动画过程中
            BYTE byNewAlpha = (BYTE)(((UINT)m_byAlphaAni * m_pBgSkin->GetAlpha()) >> 8);
            if (GetState() & WndState_Hover)
            {
                // get hover
                m_pBgSkin->DrawByIndex2(pRT, rcClient, 0, m_pBgSkin->GetAlpha());
                m_pBgSkin->DrawByIndex2(pRT, rcClient, 1, byNewAlpha);
            }
            else
            {
                // lose hover
                m_pBgSkin->DrawByIndex2(pRT, rcClient, 0, m_pBgSkin->GetAlpha());
                m_pBgSkin->DrawByIndex2(pRT, rcClient, 1, m_pBgSkin->GetAlpha() - byNewAlpha);
            }
        }
    }

    if (GetState() & (WndState_PushDown | WndState_Check))
    {
        rcClient.OffsetRect(m_ptPushOffet[0].toPixelSize(GetScale()), m_ptPushOffet[1].toPixelSize(GetScale()));
    }
    CRect rcPadding = GetStyle().GetPadding();

    rcClient.DeflateRect(rcPadding);

    // draw icon and text
    CPoint ptIcon = rcClient.TopLeft(), ptText = ptIcon;
    CSize szIcon = GetIconSize();
    CSize szDesired = GetDesiredSize(pRT, &rcClient);
    szDesired.cx -= rcPadding.left + rcPadding.right;
    szDesired.cy -= rcPadding.top + rcPadding.bottom;

    CSize szText;
    pRT->MeasureText(GetWindowText(), GetWindowText().GetLength(), &szText);

    switch (m_drawMode)
    {
    case FREE_DRAW:
        ptIcon += CPoint(m_ptIcon[0].toPixelSize(GetScale()), m_ptIcon[1].toPixelSize(GetScale()));
        DrawIcon(pRT, CRect(ptIcon, szIcon));
        ptText += CPoint(m_ptText[0].toPixelSize(GetScale()), m_ptText[1].toPixelSize(GetScale()));
        pRT->TextOut(ptText.x, ptText.y, GetWindowText(), GetWindowText().GetLength());
        break;
    case VERT_ICON_TEXT:
        ptIcon.x += (rcClient.Width() - szIcon.cx) / 2;
        ptIcon.y += (rcClient.Height() - szDesired.cy) / 2;
        DrawIcon(pRT, CRect(ptIcon, szIcon));
        ptText.x += (rcClient.Width() - szText.cx) / 2;
        ptText.y = ptIcon.y + szIcon.cy + m_nSepSize.toPixelSize(GetScale());
        pRT->TextOut(ptText.x, ptText.y, GetWindowText(), GetWindowText().GetLength());
        break;
    case VERT_TEXT_ICON:
        ptText.x += (rcClient.Width() - szText.cx) / 2;
        ptText.y += (rcClient.Height() - szDesired.cy) / 2;
        pRT->TextOut(ptText.x, ptText.y, GetWindowText(), GetWindowText().GetLength());
        ptIcon.x += (rcClient.Width() - szIcon.cx) / 2;
        ptIcon.y = ptText.y + szText.cy + m_nSepSize.toPixelSize(GetScale());
        DrawIcon(pRT, CRect(ptIcon, szIcon));
        break;
    case HORZ_ICON_TEXT:
        ptIcon.x += (rcClient.Width() - szDesired.cx) / 2;
        ptIcon.y += (rcClient.Height() - szIcon.cy) / 2;
        DrawIcon(pRT, CRect(ptIcon, szIcon));
        ptText.x = ptIcon.x + szIcon.cx + m_nSepSize.toPixelSize(GetScale());
        ptText.y += (rcClient.Height() - szText.cy) / 2;
        pRT->TextOut(ptText.x, ptText.y, GetWindowText(), GetWindowText().GetLength());
        break;
    case HORZ_TEXT_ICON:
        ptText.x += (rcClient.Width() - szDesired.cx) / 2;
        ptText.y += (rcClient.Height() - szText.cy) / 2;
        pRT->TextOut(ptText.x, ptText.y, GetWindowText(), GetWindowText().GetLength());
        ptIcon.x = ptText.x + szText.cx + m_nSepSize.toPixelSize(GetScale());
        ptIcon.y += (rcClient.Height() - szIcon.cy) / 2;
        DrawIcon(pRT, CRect(ptIcon, szIcon));
        break;
    }

    AfterPaint(pRT, painter);
}

HICON SButtonEx::SetIcon(HICON hIcon, int nIconSize)
{
    HICON hRet = m_hIcon;
    m_hIcon = hIcon;
    m_nIconSize = (float)nIconSize;
    return hRet;
}

CSize SButtonEx::GetIconSize()
{
    if (m_hIcon)
        return CSize(m_nIconSize.toPixelSize(GetScale()), m_nIconSize.toPixelSize(GetScale()));
    else if(m_pIcon)
    {
       return  m_pIcon->GetSkinSize();
    }
    return CSize();
}

void SButtonEx::DrawIcon(IRenderTarget *pRT, CRect rcIcon)
{
    if (!m_bIconVisible)
        return;
    if (m_hIcon)
    {
        pRT->DrawIconEx(rcIcon.left, rcIcon.top, m_hIcon, rcIcon.Width(), rcIcon.Height(),
                        DI_NORMAL);
    }
    else if(m_pIcon)
    {
        int iState = m_iIcon != -1 ? m_iIcon : SState2Index::GetDefIndex(GetState(), true);
        m_pIcon->DrawByIndex(pRT, rcIcon, iState);
    }
}

void SButtonEx::OnScaleChanged(int scale)
{
    __super::OnScaleChanged(scale);

    if(m_pIcon)
        GetScaleSkin(m_pIcon, scale);
}

void SButtonEx::SetIconVisible(bool bVisible)
{
    m_bIconVisible = bVisible;
    Invalidate();
}

} // namespace SOUI
