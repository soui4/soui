#include <SHostPresenter.h>
#include "SUpdateLayeredWindow.h"

SNSBEGIN

SHostPresenter::SHostPresenter(INativeWnd* pHostWnd)
    : m_pNativeWnd(pHostWnd)
    , m_bTranslucent(FALSE)
{
}

SHostPresenter::~SHostPresenter(void)
{
}

void SHostPresenter::SetHostTranlucent(BOOL bTranslucent) {
    m_bTranslucent = bTranslucent;
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
    if (m_bTranslucent)
    {
        UpdateLayerFromRenderTarget(pMemRT, byAlpha, rcInvalid);
    }
    else
    {
        BOOL bGetDC = hdc == 0;
        if (bGetDC)
            hdc = m_pNativeWnd->GetDC();
        HDC memdc = pMemRT->GetDC(1);
        ::BitBlt(hdc, rcInvalid->left, rcInvalid->top, rcInvalid->right- rcInvalid->left, rcInvalid->bottom- rcInvalid->top, memdc, rcInvalid->left, rcInvalid->top, SRCCOPY);
        pMemRT->ReleaseDC(memdc, NULL);
        if (bGetDC)
            m_pNativeWnd->ReleaseDC(hdc);
    }
}

void SHostPresenter::OnHostAlpha(THIS_ BYTE byAlpha)
{
    m_pNativeWnd->SetLayeredWindowAlpha(byAlpha);
}

void SHostPresenter::UpdateLayerFromRenderTarget(IRenderTarget *pRT, BYTE byAlpha, LPCRECT prcDirty)
{
    RECT rc;
    m_pNativeWnd->GetWindowRect(&rc);
    RECT rcDirty = { 0 };
    if (prcDirty) {
        rcDirty = *prcDirty;
    }
    else {
        rcDirty.right = rc.right - rc.left;
        rcDirty.bottom = rc.bottom - rc.top;
    }
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, byAlpha, AC_SRC_ALPHA };

    POINT ptDst = { rc.left,rc.top };
    SIZE szDst = { rc.right - rc.left,rc.bottom - rc.top };
    POINT ptSrc = { 0 };

    HDC hdc = pRT->GetDC(1);
    S_UPDATELAYEREDWINDOWINFO info = { sizeof(info), NULL, &ptDst, &szDst, hdc, &ptSrc, 0, &bf, ULW_ALPHA, &rcDirty };
    SWndSurface::SUpdateLayeredWindowIndirect(m_pNativeWnd->GetHwnd(), &info);
    pRT->ReleaseDC(hdc, NULL);
}

SNSEND