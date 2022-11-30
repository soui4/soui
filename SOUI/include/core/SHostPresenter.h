#pragma once
#include <interface/SHostPresenter-i.h>
#include <helper/obj-ref-impl.hpp>
SNSBEGIN

class SOUI_EXP SHostPresenter : public TObjRefImpl<IHostPresenter>
{
protected:
	IHostWnd *m_pHostWnd;
public:
	SHostPresenter(IHostWnd *pHostWnd);
	~SHostPresenter(void);
public:
	STDMETHOD_(void,OnHostCreate)(THIS) OVERRIDE;
	STDMETHOD_(void,OnHostDestroy)(THIS) OVERRIDE;
	STDMETHOD_(void,OnHostResize)(THIS_ SIZE szHost) OVERRIDE;
	STDMETHOD_(void,OnHostPresent)(THIS_ HDC hdc,IRenderTarget *pMemRT,LPCRECT rcDirty,BYTE byAlpha) OVERRIDE;
	STDMETHOD_(void,OnHostAlpha)(THIS_ BYTE byAlpha) OVERRIDE;
protected:
	void UpdateLayerFromRenderTarget(IRenderTarget *pRT, BYTE byAlpha, LPCRECT prcDirty = NULL);
};

SNSEND
