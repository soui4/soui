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
    if (m_pHostWnd->IsTranslucent())
    {
        UpdateLayerFromRenderTarget(pMemRT, byAlpha, rcInvalid);
    }
    else
    {
        BOOL bGetDC = hdc == 0;
        if (bGetDC)
            hdc = m_pHostWnd->GetDC();
        HDC memdc = pMemRT->GetDC(0);
        ::BitBlt(hdc, rcInvalid->left, rcInvalid->top, RectWidth(rcInvalid), RectHeight(rcInvalid), memdc, rcInvalid->left, rcInvalid->top, SRCCOPY);
        pMemRT->ReleaseDC(memdc);
        if (bGetDC)
            m_pHostWnd->ReleaseDC(hdc);
    }
}

void SHostPresenter::OnHostAlpha(THIS_ BYTE byAlpha)
{
    ::SetLayeredWindowAttributes(m_pHostWnd->GetHwnd(), 0, byAlpha, LWA_ALPHA);
}

void SHostPresenter::UpdateLayerFromRenderTarget(IRenderTarget *pRT, BYTE byAlpha, LPCRECT prcDirty)
{
    CRect rc;
    m_pHostWnd->GetWindowRect(&rc);
    CRect rcDirty = prcDirty ? (*prcDirty) : CRect(0, 0, rc.Width(), rc.Height());
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, byAlpha, AC_SRC_ALPHA };

    //注意：下面这几个参数不能直接在info参数里直接使用&rc.Size()，否则会被编译器优化掉，导致参数错误
    CPoint ptDst = rc.TopLeft();
    CSize szDst = rc.Size();
    CPoint ptSrc;

    HDC hdc = pRT->GetDC(0);
    S_UPDATELAYEREDWINDOWINFO info = { sizeof(info), NULL, &ptDst, &szDst, hdc, &ptSrc, 0, &bf, ULW_ALPHA, &rcDirty };
    SWndSurface::SUpdateLayeredWindowIndirect(m_pHostWnd->GetHwnd(), &info);
    pRT->ReleaseDC(hdc);
}

SNSEND