#pragma once
#include <interface/SFactory-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

class SOUI_EXP SouiFactory : public TObjRefImpl<ISouiFactory> {
  public:
    SouiFactory(void);
    ~SouiFactory(void);

  public:
    STDMETHOD_(IApplication *, CreateApp)(THIS_ IRenderFactory *pRenderFac,HMODULE hInst,LPCTSTR pszHostClassName DEF_VAL(_T("SOUIHOST")), BOOL bImeApp DEF_VAL(FALSE)) OVERRIDE;
    STDMETHOD_(INativeWnd *, CreateNativeWnd)(THIS) OVERRIDE;
    STDMETHOD_(IHostWnd *, CreateHostWnd)(THIS_ LPCTSTR pszResID DEF_VAL(NULL)) OVERRIDE;
    STDMETHOD_(IHostDialog *, CreateHostDialog)(THIS_ LPCTSTR pszResID DEF_VAL(NULL)) OVERRIDE;
    STDMETHOD_(IStringA *, CreateStringA)(THIS_ LPCSTR pszSrc) OVERRIDE;
    STDMETHOD_(IStringW *, CreateStringW)(THIS_ LPCWSTR pszSrc) OVERRIDE;
    STDMETHOD_(IXmlDoc *, CreateXmlDoc)(THIS) OVERRIDE;
    STDMETHOD_(IResProvider*, CreateResProvider)(THIS_ BUILTIN_RESTYPE resType) OVERRIDE;
    STDMETHOD_(IEvtSlot *, CreateFuncSlot)(THIS_ FunCallback fun, void *ctx) OVERRIDE;
	STDMETHOD_(IMenu*, CreateMenu)(THIS_  HMENU hMenu DEF_VAL(NULL)) OVERRIDE;
	STDMETHOD_(IMenuEx*,CreateMenuEx)(THIS) OVERRIDE;
	STDMETHOD_(ITimer*,CreateTimer)(THIS_ IEvtSlot *pEvtSlot) OVERRIDE;
};

SNSEND