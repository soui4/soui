#include <souistd.h>
#include <core/SHostPresenter.h>
#include <src/updatelayeredwindow/SUpdateLayeredWindow.h>
SNSBEGIN

SHostPresenter::SHostPresenter(IHostWnd *pHostWnd)
    : m_pHostWnd(pHostWnd)
{
}

SHostPresenter::~SHostPresenter(void)
{
}

void SHostPresenter::OnHostCreate(THIS)
{
}

void SHostPresenter::OnHostDestroy(THIS)
{
}

void SHostPresenter::OnHostResize(THIS_ SIZE szHost)
{
}

void SHostPresenter::OnHostPresent(THIS_ HDC hdc, IRenderTarget *pMemRT, LPCRECT rcInvalid, BYTE byAlpha)
{
    #ifdef _WIN32
    if (m_pHostWnd->IsTranslucent())
    {
        UpdateLayerFromRenderTarget(pMemRT, byAlpha, rcInvalid);
    }
    else
    {
        BOOL bGetDC = hdc == 0;
        if (bGetDC)
            hdc = m_pHostWnd->GetNativeWnd()->GetDC();
        HDC memdc = pMemRT->GetDC(1);
        ::BitBlt(hdc, rcInvalid->left, rcInvalid->top, RectWidth(rcInvalid), RectHeight(rcInvalid), memdc, rcInvalid->left, rcInvalid->top, SRCCOPY);
        pMemRT->ReleaseDC(memdc, NULL);
        if (bGetDC)
            m_pHostWnd->GetNativeWnd()->ReleaseDC(hdc);
    }
    #endif//_WIN32
}

void SHostPresenter::OnHostAlpha(THIS_ BYTE byAlpha)
{
    #ifdef _WIN32
    ::SetLayeredWindowAttributes(m_pHostWnd->GetNativeWnd()->GetHwnd(), 0, byAlpha, LWA_ALPHA);
    #endif//_WIN32
}

void SHostPresenter::UpdateLayerFromRenderTarget(IRenderTarget *pRT, BYTE byAlpha, LPCRECT prcDirty)
{
    #ifdef _WIN32
    CRect rc;
    m_pHostWnd->GetNativeWnd()->GetWindowRect(&rc);
    CRect rcDirty = prcDirty ? (*prcDirty) : CRect(0, 0, rc.Width(), rc.Height());
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, byAlpha, AC_SRC_ALPHA };

    CPoint ptDst = rc.TopLeft();
    CSize szDst = rc.Size();
    CPoint ptSrc;

    HDC hdc = pRT->GetDC(1);
    S_UPDATELAYEREDWINDOWINFO info = { sizeof(info), NULL, &ptDst, &szDst, hdc, &ptSrc, 0, &bf, ULW_ALPHA, &rcDirty };
    SWndSurface::SUpdateLayeredWindowIndirect(m_pHostWnd->GetNativeWnd()->GetHwnd(), &info);
    pRT->ReleaseDC(hdc, NULL);
    #endif//_WIN32
}

SNSEND