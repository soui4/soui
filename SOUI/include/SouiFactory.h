#pragma once
#include <interface/SFactory-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

class SOUI_EXP SouiFactory : public TObjRefImpl<ISouiFactory> {
  public:
    SouiFactory(void);
    ~SouiFactory(void);

  public:
    STDMETHOD_(HRESULT, CreateApp)
    (THIS_ IApplication **ppRet,
     IRenderFactory *pRenderFac,
     HMODULE hInst,
     LPCTSTR pszHostClassName = _T("SOUIHOST"),
     BOOL bImeApp = FALSE) OVERRIDE;
    STDMETHOD_(HRESULT, CreateNativeWnd)(THIS_ INativeWnd **ppRet) OVERRIDE;
    STDMETHOD_(HRESULT, CreateHostWnd)(THIS_ IHostWnd **ppRet, LPCTSTR pszResID = NULL) OVERRIDE;
    STDMETHOD_(HRESULT, CreateHostDialog)
    (THIS_ IHostDialog **ppRet, LPCTSTR pszResID = NULL) OVERRIDE;
    STDMETHOD_(HRESULT, CreateStringA)(THIS_ IStringA **ppRet, LPCSTR pszSrc) OVERRIDE;
    STDMETHOD_(HRESULT, CreateStringW)(THIS_ IStringW **ppRet, LPCWSTR pszSrc) OVERRIDE;
    STDMETHOD_(HRESULT, CreateXmlDoc)(THIS_ IXmlDoc **ppRet) OVERRIDE;
    STDMETHOD_(HRESULT, CreateResProvider)(THIS_ BUILTIN_RESTYPE resType, IObjRef **pObj) OVERRIDE;
    STDMETHOD_(HRESULT, CreateFuncSlot)
    (THIS_ FunCallback fun, void *ctx, IEvtSlot **ppSlot) OVERRIDE;
	STDMETHOD_(HRESULT, CreateMenu)(THIS_ IMenu**ppRet, HMENU hMenu DEF_VAL(NULL)) OVERRIDE;
	STDMETHOD_(HRESULT, CreateMenuEx)(THIS_ IMenuEx**ppRet) OVERRIDE;
};

SNSEND