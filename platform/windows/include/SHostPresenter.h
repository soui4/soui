#ifndef __SHOSTPRESENTER__H__
#define __SHOSTPRESENTER__H__
#include <interface/SHostPresenter-i.h>
#include <helper/obj-ref-impl.hpp>
#include <platform_exp.h>
#include <interface/SNativeWnd-i.h>

SNSBEGIN

class PLATFORM_API SHostPresenter : public TObjRefImpl<IHostPresenter> {
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

SNSEND

#endif // __SHOSTPRESENTER__H__