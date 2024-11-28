#ifndef _SHOSTPRESENTER_H_
#define _SHOSTPRESENTER_H_
#include <interface/SHostPresenter-i.h>
#include <interface/SNativeWnd-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

#ifdef _WIN32
struct S_UPDATELAYEREDWINDOWINFO {
	DWORD cbSize;
	HDC hdcDst;
	POINT* pptDst;
	SIZE* psize;
	HDC hdcSrc;
	POINT* pptSrc;
	COLORREF crKey;
	BLENDFUNCTION* pblend;
	DWORD dwFlags;
	RECT* prcDirty;
};

class SWndSurface {
public:
	static BOOL Init();
	static BOOL SUpdateLayeredWindowIndirect(HWND hWnd, const S_UPDATELAYEREDWINDOWINFO* pULWInfo);
};


class SOUI_EXP SHostPresenter : public TObjRefImpl<IHostPresenter> {
  protected:
    INativeWnd *m_pNativeWnd;
    BOOL m_bTranslucent;
  public:
    SHostPresenter(INativeWnd*pHostWnd);
    ~SHostPresenter(void);

  public:
      STDMETHOD_(void, SetHostTranlucent)(THIS_ BOOL bTranslucent) OVERRIDE;
    STDMETHOD_(void, OnHostCreate)(THIS) OVERRIDE;
    STDMETHOD_(void, OnHostDestroy)(THIS) OVERRIDE;
    STDMETHOD_(void, OnHostResize)(THIS_ SIZE szHost) OVERRIDE;
    STDMETHOD_(void, OnHostPresent)(THIS_ HDC hdc, IRenderTarget *pMemRT, LPCRECT rcDirty, BYTE byAlpha) OVERRIDE;
    STDMETHOD_(void, OnHostAlpha)(THIS_ BYTE byAlpha) OVERRIDE;

  protected:
    void UpdateLayerFromRenderTarget(IRenderTarget *pRT, BYTE byAlpha, LPCRECT prcDirty = NULL);
};

#else
class SOUI_EXP SHostPresenter : public TObjRefImpl<IHostPresenter>{
  protected:
    INativeWnd *m_pNativeWnd;
    BOOL m_bTranslucent;
  public:
    SHostPresenter(INativeWnd*pHostWnd);
    ~SHostPresenter(void);

  public:
    STDMETHOD_(void, SetHostTranlucent)(THIS_ BOOL bTranslucent) OVERRIDE;
    STDMETHOD_(void, OnHostCreate)(THIS) OVERRIDE;
    STDMETHOD_(void, OnHostDestroy)(THIS) OVERRIDE;
    STDMETHOD_(void, OnHostResize)(THIS_ SIZE szHost) OVERRIDE;
    STDMETHOD_(void, OnHostPresent)(THIS_ HDC hdc, IRenderTarget *pMemRT, LPCRECT rcDirty, BYTE byAlpha) OVERRIDE;
    STDMETHOD_(void, OnHostAlpha)(THIS_ BYTE byAlpha) OVERRIDE;

};
#endif//_WIN32

SNSEND
#endif//_SHOSTPRESENTER_H_