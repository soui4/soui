#ifndef _SHOSTPRESENTER_H_
#define _SHOSTPRESENTER_H_
#include <interface/SHostPresenter-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN
class SHostWnd;
#ifdef _WIN32

struct S_UPDATELAYEREDWINDOWINFO;
class SWndSurface {
  public:
    static BOOL Init();
    static BOOL SUpdateLayeredWindowIndirect(HWND hWnd, const S_UPDATELAYEREDWINDOWINFO *pULWInfo);
};

class SOUI_EXP SHostPresenter : public TObjRefImpl<IHostPresenter> {
  protected:
    SHostWnd *m_pHostWnd;

  public:
    SHostPresenter(SHostWnd *pHostWnd);
    ~SHostPresenter(void);

  public:
    STDMETHOD_(void, OnHostCreate)(THIS) OVERRIDE;
    STDMETHOD_(void, OnHostDestroy)(THIS) OVERRIDE;
    STDMETHOD_(void, OnHostResize)(THIS_ SIZE szHost) OVERRIDE;
    STDMETHOD_(void, OnHostPresent)(THIS_ HDC hdc, IRenderTarget *pMemRT, LPCRECT rcDirty, BYTE byAlpha) OVERRIDE;

  protected:
    void UpdateLayerFromRenderTarget(IRenderTarget *pRT, BYTE byAlpha, LPCRECT prcDirty = NULL);
};

#else
class SOUI_EXP SHostPresenter : public TObjRefImpl<IHostPresenter> {
  protected:
    SHostWnd *m_pHostWnd;

  public:
    SHostPresenter(SHostWnd *pHostWnd);
    ~SHostPresenter(void);

  public:
    STDMETHOD_(void, OnHostCreate)(THIS) OVERRIDE;
    STDMETHOD_(void, OnHostDestroy)(THIS) OVERRIDE;
    STDMETHOD_(void, OnHostResize)(THIS_ SIZE szHost) OVERRIDE;
    STDMETHOD_(void, OnHostPresent)(THIS_ HDC hdc, IRenderTarget *pMemRT, LPCRECT rcDirty, BYTE byAlpha) OVERRIDE;
};
#endif //_WIN32

SNSEND
#endif //_SHOSTPRESENTER_H_