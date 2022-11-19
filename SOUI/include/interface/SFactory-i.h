#pragma once

#include <interface/obj-ref-i.h>
#include <interface/sapp-i.h>
#include <interface/shostwnd-i.h>
#include <interface/sstring-i.h>
#include <interface/sxml-i.h>
#include <interface/SResProvider-i.h>
#include <interface/SEvtArgs-i.h>
#include <interface/smenu-i.h>
#include <interface/smenuex-i.h>
#include <interface/STimer-i.h>
#include <soui_exp.h>
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

	STDMETHOD_(IApplication *, CreateApp)(THIS_ IRenderFactory *pRenderFac,HMODULE hInst,LPCTSTR pszHostClassName DEF_VAL(_T("SOUIHOST")), BOOL bImeApp DEF_VAL(FALSE)) PURE;
	STDMETHOD_(INativeWnd *, CreateNativeWnd)(THIS) PURE;
	STDMETHOD_(IHostWnd *, CreateHostWnd)(THIS_ LPCTSTR pszResID DEF_VAL(NULL)) PURE;
	STDMETHOD_(IHostDialog *, CreateHostDialog)(THIS_ LPCTSTR pszResID DEF_VAL(NULL)) PURE;
	STDMETHOD_(IStringA *, CreateStringA)(THIS_ LPCSTR pszSrc) PURE;
	STDMETHOD_(IStringW *, CreateStringW)(THIS_ LPCWSTR pszSrc) PURE;
	STDMETHOD_(IXmlDoc *, CreateXmlDoc)(THIS) PURE;
	STDMETHOD_(IResProvider*, CreateResProvider)(THIS_ BUILTIN_RESTYPE resType) PURE;
	STDMETHOD_(IEvtSlot *, CreateFuncSlot)(THIS_ FunCallback fun, void *ctx) PURE;
	STDMETHOD_(IMenu*, CreateMenu)(THIS_  HMENU hMenu DEF_VAL(NULL)) PURE;
	STDMETHOD_(IMenuEx*,CreateMenuEx)(THIS) PURE;
	STDMETHOD_(ITimer*,CreateTimer)(THIS_ IEvtSlot *pEvtSlot) PURE;
};

SNSEND

#ifdef __cplusplus
typedef SOUI::ISouiFactory * ISouiFactoryPtr;
#else
typedef ISouiFactory * ISouiFactoryPtr;
#endif

EXTERN_C ISouiFactoryPtr SOUI_EXP CreateSouiFactory();
