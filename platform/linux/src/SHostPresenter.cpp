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

}

void SHostPresenter::OnHostAlpha(THIS_ BYTE byAlpha)
{
    m_pNativeWnd->SetLayeredWindowAlpha(byAlpha);
}

SNSEND