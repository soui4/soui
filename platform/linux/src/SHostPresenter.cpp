#include <SHostPresenter.h>

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
    FillRect(hdc, rcInvalid, GetStockObject(WHITE_BRUSH));
    BOOL bGetDC = hdc == 0;
    if (bGetDC)
        hdc = m_pNativeWnd->GetDC();
    HDC memdc = pMemRT->GetDC(1);
    ::BitBlt(hdc, rcInvalid->left, rcInvalid->top, rcInvalid->right-rcInvalid->left, rcInvalid->bottom-rcInvalid->top, memdc, rcInvalid->left, rcInvalid->top, SRCCOPY);
    pMemRT->ReleaseDC(memdc, NULL);
    if (bGetDC)
        m_pNativeWnd->ReleaseDC(hdc);
}

void SHostPresenter::OnHostAlpha(THIS_ BYTE byAlpha)
{
    m_pNativeWnd->SetLayeredWindowAlpha(byAlpha);
}

SNSEND