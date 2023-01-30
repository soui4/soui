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

	//////////////////////////////////////////////////////////////////////////

	/** CreateApp
	 * @brief 创建IApp对象
	 * @return IApplication * -- App对象
	 */
	STDMETHOD_(IApplication *, CreateApp)(THIS_ IRenderFactory *pRenderFac,HMODULE hInst,LPCTSTR pszHostClassName DEF_VAL(_T("SOUIHOST")), BOOL bImeApp DEF_VAL(FALSE)) PURE;

	/** CreateNativeWnd
	 * @brief 创建窗口对象
	 * @return INativeWnd * -- 窗口对象
	 */
	STDMETHOD_(INativeWnd *, CreateNativeWnd)(THIS) PURE;

	/** CreateHostWnd
	 * @brief 创建SOUI窗口对象
	 * @return IHostWnd * -- SOUI窗口对象
	 */
	STDMETHOD_(IHostWnd *, CreateHostWnd)(THIS_ LPCTSTR pszResID DEF_VAL(NULL)) PURE;


	/** CreateHostDialog
	 * @brief 创建SOUI对话框对象
	 * @return IHostDialog * -- SOUI对话框对象
	 */
	STDMETHOD_(IHostDialog *, CreateHostDialog)(THIS_ LPCTSTR pszResID DEF_VAL(NULL)) PURE;

	/** CreateStringA
	 * @brief 创建IStringA对象
	 * @return IStringA * -- 字符串对象
	 */
	STDMETHOD_(IStringA *, CreateStringA)(THIS_ LPCSTR pszSrc) PURE;

	/** CreateStringW
	 * @brief 创建IStringW对象
	 * @return IStringW * -- 字符串对象
	 */
	STDMETHOD_(IStringW *, CreateStringW)(THIS_ LPCWSTR pszSrc) PURE;

	/** CreateXmlDoc
	 * @brief 创建IXmlDoc对象
	 * @return IXmlDoc * -- Xml对象
	 */
	STDMETHOD_(IXmlDoc *, CreateXmlDoc)(THIS) PURE;

	/** CreateResProvider
	 * @brief 创建IResProvider对象
	 * @return IResProvider * -- 资源包对象
	 */
	STDMETHOD_(IResProvider*, CreateResProvider)(THIS_ BUILTIN_RESTYPE resType) PURE;

	/** CreateFuncSlot
	 * @brief 创建IEvtSlot对象
	 * @return IEvtSlot * -- Event Slot对象
	 */
	STDMETHOD_(IEvtSlot *, CreateFuncSlot)(THIS_ FunCallback fun, void *ctx) PURE;

	/** CreateMenu
	 * @brief 创建Menu对象
	 * @return IMenu * -- Menu对象
	 */
	STDMETHOD_(IMenu*, CreateMenu)(THIS_  HMENU hMenu DEF_VAL(NULL)) PURE;

	/** CreateMenuEx
	 * @brief 创建MenuEx对象
	 * @return IMenuEx * -- MenuEx对象
	 */
	STDMETHOD_(IMenuEx*,CreateMenuEx)(THIS) PURE;

	/** CreateTimer
	 * @brief 创建定时器对象
	 * @return ITimer * -- 定时器对象
	 */
	STDMETHOD_(ITimer*,CreateTimer)(THIS_ IEvtSlot *pEvtSlot) PURE;
};

SNSEND

#ifdef __cplusplus
typedef SOUI::ISouiFactory * ISouiFactoryPtr;
#else
typedef ISouiFactory * ISouiFactoryPtr;
#endif

EXTERN_C ISouiFactoryPtr SOUI_EXP CreateSouiFactory();
