#ifndef __SFACTORY_I__H__
#define __SFACTORY_I__H__

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
#include <interface/SValueAnimator-i.h>
#include <soui_exp.h>
SNSBEGIN

#undef INTERFACE
#define INTERFACE ISouiFactory
DECLARE_INTERFACE_(ISouiFactory, IObjRef)
{
    /** Add reference */
    //
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /** Release reference */
    //
    STDMETHOD_(long, Release)(THIS) PURE;

    /** Release object */
    //
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /** CreateApp
     * @brief Create IApp object
     * @return IApplication * -- App object
     */
    STDMETHOD_(IApplication *, CreateApp)(THIS_ IRenderFactory * pRenderFac, HMODULE hInst, LPCTSTR pszHostClassName DEF_VAL(_T("SOUIHOST")), BOOL bImeApp DEF_VAL(FALSE)) PURE;

    /** CreateNativeWnd
     * @brief Create window object
     * @return INativeWnd * -- window object
     */
    STDMETHOD_(INativeWnd *, CreateNativeWnd)(THIS) PURE;

    /** CreateHostWnd
     * @brief Create SOUI window object
     * @return IHostWnd * -- SOUI window object
     */
    STDMETHOD_(IHostWnd *, CreateHostWnd)(THIS_ LPCTSTR pszResID DEF_VAL(NULL)) PURE;

    /** CreateHostDialog
     * @brief Create SOUI dialog object
     * @return IHostDialog * -- SOUI dialog object
     */
    STDMETHOD_(IHostDialog *, CreateHostDialog)(THIS_ LPCTSTR pszResID DEF_VAL(NULL)) PURE;

    /** CreateStringA
     * @brief Create IStringA object
     * @return IStringA * -- string object
     */
    STDMETHOD_(IStringA *, CreateStringA)(THIS_ LPCSTR pszSrc) PURE;

    /** CreateStringW
     * @brief Create IStringW object
     * @return IStringW * -- string object
     */
    STDMETHOD_(IStringW *, CreateStringW)(THIS_ LPCWSTR pszSrc) PURE;

    /** CreateXmlDoc
     * @brief Create IXmlDoc object
     * @return IXmlDoc * -- Xml object
     */
    STDMETHOD_(IXmlDoc *, CreateXmlDoc)(THIS) PURE;

    /** CreateResProvider
     * @brief Create IResProvider object
     * @return IResProvider * -- resource object
     */
    STDMETHOD_(IResProvider *, CreateResProvider)(THIS_ BUILTIN_RESTYPE resType) PURE;

    /** CreateFuncSlot
     * @brief Create IEvtSlot object
     * @return IEvtSlot * -- Event Slot object
     */
    STDMETHOD_(IEvtSlot *, CreateFuncSlot)(THIS_ FunCallback fun, void *ctx) PURE;

    /** CreateMenu
     * @brief Create Menu object
     * @return IMenu * -- Menu object
     */
    STDMETHOD_(IMenu *, CreateMenu)(THIS_ HMENU hMenu DEF_VAL(0)) PURE;

    /** CreateMenuEx
     * @brief Create MenuEx object
     * @return IMenuEx * -- MenuEx object
     */
    STDMETHOD_(IMenuEx *, CreateMenuEx)(THIS) PURE;

    /** CreateTimer
     * @brief Create timer object
     * @return ITimer * -- timer object
     */
    STDMETHOD_(ITimer *, CreateTimer)(THIS_ IEvtSlot * pEvtSlot) PURE;

    /** CreateAnimatorGroup
     * @brief Create animation group
     * @return IAnimatorGroup * -- animation group
     */
    STDMETHOD_(IAnimatorGroup *, CreateAnimatorGroup)(THIS) PURE;
};

SNSEND

#ifdef __cplusplus
typedef SNS::ISouiFactory *ISouiFactoryPtr;
#else
typedef ISouiFactory *ISouiFactoryPtr;
#endif

EXTERN_C ISouiFactoryPtr SOUI_EXP CreateSouiFactory();

#endif /**< __SFACTORY_I__H__ */