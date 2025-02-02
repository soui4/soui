#include <souistd.h>
#include <core/SHostPresenter.h>
#include <core/SHostWnd.h>
SNSBEGIN

#ifdef _WIN32
struct S_UPDATELAYEREDWINDOWINFO
{
    DWORD cbSize;
    HDC hdcDst;
    POINT *pptDst;
    SIZE *psize;
    HDC hdcSrc;
    POINT *pptSrc;
    COLORREF crKey;
    BLENDFUNCTION *pblend;
    DWORD dwFlags;
    RECT *prcDirty;
};

typedef BOOL(WINAPI *FunUpdateLayeredWindow)(HWND hwnd, HDC hdcDst, const POINT *pptDst, const SIZE *psize, HDC hdcSrc, const POINT *pptSrc, COLORREF crKey, const BLENDFUNCTION *pblend, DWORD dwflags);
typedef BOOL(WINAPI *FunUpdateLayeredWindowIndirect)(HWND hwnd, const S_UPDATELAYEREDWINDOWINFO *pULWInfo);

static FunUpdateLayeredWindow s_funUpdateLayeredWindow = NULL;
static FunUpdateLayeredWindowIndirect s_funUpdateLayeredWindowIndirect = NULL;

BOOL WINAPI _SUpdateLayeredWindowIndirect(HWND hWnd, const S_UPDATELAYEREDWINDOWINFO *info)
{
    SASSERT(s_funUpdateLayeredWindow);
    return (*s_funUpdateLayeredWindow)(hWnd, info->hdcDst, info->pptDst, info->psize, info->hdcSrc, info->pptSrc, info->crKey, info->pblend, info->dwFlags);
}

BOOL SWndSurface::Init()
{
    HMODULE hUser32 = GetModuleHandle(_T("user32"));
    if (!hUser32)
    {
        SASSERT(FALSE);
        return FALSE;
    }
    s_funUpdateLayeredWindow = (FunUpdateLayeredWindow)GetProcAddress(hUser32, "UpdateLayeredWindow");
    if (!s_funUpdateLayeredWindow)
    {
        SASSERT(FALSE);
        return FALSE;
    }
#ifdef _WIN64 // X64中使用UpdateLayeredWindowIndirect在Win7测试显示不出内容，只能使用UpdateLayeredWindow,fuck
    // MS.
    s_funUpdateLayeredWindowIndirect = _SUpdateLayeredWindowIndirect;
#else
    s_funUpdateLayeredWindowIndirect = (FunUpdateLayeredWindowIndirect)GetProcAddress(hUser32, "UpdateLayeredWindowIndirect");
    if (!s_funUpdateLayeredWindowIndirect)
        s_funUpdateLayeredWindowIndirect = _SUpdateLayeredWindowIndirect;
#endif
    return TRUE;
}

BOOL SWndSurface::SUpdateLayeredWindowIndirect(HWND hWnd, const S_UPDATELAYEREDWINDOWINFO *pULWInfo)
{
    BOOL bRet = s_funUpdateLayeredWindowIndirect(hWnd, pULWInfo);
    return bRet;
}

//===========================================================
SHostPresenter::SHostPresenter(SHostWnd *pHostWnd)
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
        HDC memdc = pMemRT->GetDC(1);
        ::BitBlt(hdc, rcInvalid->left, rcInvalid->top, rcInvalid->right - rcInvalid->left, rcInvalid->bottom - rcInvalid->top, memdc, rcInvalid->left, rcInvalid->top, SRCCOPY);
        pMemRT->ReleaseDC(memdc, NULL);
        if (bGetDC)
            m_pHostWnd->ReleaseDC(hdc);
    }
}

void SHostPresenter::UpdateLayerFromRenderTarget(IRenderTarget *pRT, BYTE byAlpha, LPCRECT prcDirty)
{
    RECT rc;
    ((SNativeWnd *)m_pHostWnd)->GetWindowRect(&rc);
    RECT rcDirty = { 0 };
    if (prcDirty)
    {
        rcDirty = *prcDirty;
    }
    else
    {
        rcDirty.right = rc.right - rc.left;
        rcDirty.bottom = rc.bottom - rc.top;
    }
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, byAlpha, AC_SRC_ALPHA };

    POINT ptDst = { rc.left, rc.top };
    SIZE szDst = { rc.right - rc.left, rc.bottom - rc.top };
    POINT ptSrc = { 0 };

    HDC hdc = pRT->GetDC(1);
    S_UPDATELAYEREDWINDOWINFO info = { sizeof(info), NULL, &ptDst, &szDst, hdc, &ptSrc, 0, &bf, ULW_ALPHA, &rcDirty };
    SWndSurface::SUpdateLayeredWindowIndirect(m_pHostWnd->GetNative()->GetHwnd(), &info);
    pRT->ReleaseDC(hdc, NULL);
}

#else
SHostPresenter::SHostPresenter(SHostWnd *pHostWnd)
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
    BOOL bGetDC = hdc == 0;
    if (bGetDC)
        hdc = m_pHostWnd->GetDC();
    HDC memdc = pMemRT->GetDC(1);
    ::BitBlt(hdc, rcInvalid->left, rcInvalid->top, rcInvalid->right - rcInvalid->left, rcInvalid->bottom - rcInvalid->top, memdc, rcInvalid->left, rcInvalid->top, SRCCOPY);
    if (m_pHostWnd->GetHostAttr().m_bTranslucent && m_pHostWnd->GetHostAttr().m_bAutoShape)
    {
        HBITMAP hBmp = GetCurrentObject(memdc, OBJ_BITMAP);
        HRGN hRgn = CreateRegionFromBitmap(hBmp, 0, 0xFF000000);
        SetWindowRgn(m_pHostWnd->m_hWnd, hRgn, FALSE);
        if (hRgn)
            DeleteObject(hRgn);
    }
    pMemRT->ReleaseDC(memdc, NULL);
    if (bGetDC)
        m_pHostWnd->ReleaseDC(hdc);
}

#endif //_WIN32
SNSEND