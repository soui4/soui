#pragma once

#include <interface/obj-ref-i.h>
#include <interface/sapp-i.h>
#include <interface/shostwnd-i.h>
#include <interface/sstring-i.h>
#include <interface/sxml-i.h>
#include <interface/SResProvider-i.h>
#include <interface/SEvtArgs-i.h>
#include <interface/smenu-i.h>
SNSBEGIN

#undef INTERFACE
#define INTERFACE ISouiFactory
DECLARE_INTERFACE_(ISouiFactory, IObjRef)
{
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    STDMETHOD_(HRESULT, CreateApp)
    (THIS_ IApplication * *ppRet, IRenderFactory * pRenderFac, HMODULE hInst,
     LPCTSTR pszHostClassName, BOOL bImeApp) PURE;
    STDMETHOD_(HRESULT, CreateNativeWnd)(THIS_ INativeWnd * *ppRet) PURE;
    STDMETHOD_(HRESULT, CreateHostWnd)(THIS_ IHostWnd * *ppRet, LPCTSTR pszResID) PURE;
    STDMETHOD_(HRESULT, CreateHostDialog)(THIS_ IHostDialog * *ppRet, LPCTSTR pszResID) PURE;
    STDMETHOD_(HRESULT, CreateStringA)(THIS_ IStringA * *ppRet, LPCSTR pszSrc) PURE;
    STDMETHOD_(HRESULT, CreateStringW)(THIS_ IStringW * *ppRet, LPCWSTR pszStr) PURE;
    STDMETHOD_(HRESULT, CreateXmlDoc)(THIS_ IXmlDoc * *ppRet) PURE;
    STDMETHOD_(HRESULT, CreateResProvider)(THIS_ BUILTIN_RESTYPE resType, IObjRef * *pObj) PURE;
    STDMETHOD_(HRESULT, CreateFuncSlot)(THIS_ FunCallback fun, void *ctx, IEvtSlot **ppSlot) PURE;
	STDMETHOD_(HRESULT, CreateMenu)(THIS_ IMenu**ppRet, HMENU hMenu DEF_VAL(NULL)) PURE;
};

SNSEND

#ifdef DLL_CORE
#ifdef SOUI_EXPORTS
#define SOUI_EXP __declspec(dllexport)
#else
#define SOUI_EXP __declspec(dllimport)
#endif // SOUI_EXPORTS
#else
#define SOUI_EXP
#endif

EXTERN_C HRESULT SOUI_EXP CreateSouiFactory(IObjRef **ppRet);
