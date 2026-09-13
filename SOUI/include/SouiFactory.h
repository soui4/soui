#ifndef __SOUIFACTORY__H__
#define __SOUIFACTORY__H__
#include <interface/SFactory-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

class SOUI_EXP SouiFactory : public TObjRefImpl<ISouiFactory> {
  public:
    SouiFactory(void);
    ~SouiFactory(void);
    
    /**
      * @brief Gets the global factory instance.
      * @return Reference to the singleton instance.
      */
     static SouiFactory &instance(void);
  public:
    /**
     * @brief Create application object
     * @param pRenderFac render factory object
     * @param hInst module handle
     * @param pszHostClassName window class name
     * @param bImeApp whether to enable IME support
     * @return IApplication* application object
     */
    STDMETHOD_(IApplication *, CreateApp)(THIS_ IRenderFactory *pRenderFac, HMODULE hInst, LPCTSTR pszHostClassName DEF_VAL(_T("SOUIHOST")), BOOL bImeApp DEF_VAL(FALSE)) OVERRIDE;

    /**
     * @brief Create native window object
     * @return INativeWnd* native window object
     */
    STDMETHOD_(INativeWnd *, CreateNativeWnd)(THIS) OVERRIDE;

    /**
     * @brief Create host window object
     * @param pszResID resource ID
     * @return IHostWnd* host window object
     */
    STDMETHOD_(IHostWnd *, CreateHostWnd)(THIS_ LPCTSTR pszResID DEF_VAL(NULL)) OVERRIDE;

    /**
     * @brief Create host dialog object
     * @param pszResID resource ID
     * @return IHostDialog* host dialog object
     */
    STDMETHOD_(IHostDialog *, CreateHostDialog)(THIS_ LPCTSTR pszResID DEF_VAL(NULL)) OVERRIDE;

    /**
     * @brief Create ANSI string object
     * @param pszSrc source string
     * @return IStringA* ANSI string object
     */
    STDMETHOD_(IStringA *, CreateStringA)(THIS_ LPCSTR pszSrc) OVERRIDE;

    /**
     * @brief Create wide-character string object
     * @param pszSrc source string
     * @return IStringW* wide-character string object
     */
    STDMETHOD_(IStringW *, CreateStringW)(THIS_ LPCWSTR pszSrc) OVERRIDE;

    /**
     * @brief Create XML document object
     * @return IXmlDoc* XML document object
     */
    STDMETHOD_(IXmlDoc *, CreateXmlDoc)(THIS) OVERRIDE;

    /**
     * @brief Create resource provider object
     * @param resType built-in resource type
     * @return IResProvider* resource provider object
     */
    STDMETHOD_(IResProvider *, CreateResProvider)(THIS_ BUILTIN_RESTYPE resType) OVERRIDE;

    /**
     * @brief Create event slot object
     * @param fun callback function
     * @param ctx context pointer
     * @return IEvtSlot* event slot object
     */
    STDMETHOD_(IEvtSlot *, CreateFuncSlot)(THIS_ FunCallback fun, void *ctx) OVERRIDE;

    /**
     * @brief Create menu object
     * @param hMenu menu handle
     * @return IMenu* menu object
     */
    STDMETHOD_(IMenu *, CreateMenu)(THIS_ HMENU hMenu DEF_VAL(0)) OVERRIDE;

    /**
     * @brief Create extended menu object
     * @return IMenuEx* extended menu object
     */
    STDMETHOD_(IMenuEx *, CreateMenuEx)(THIS) OVERRIDE;

    /**
     * @brief Create timer object
     * @param pEvtSlot event slot object
     * @return ITimer* timer object
     */
    STDMETHOD_(ITimer *, CreateTimer)(THIS_ IEvtSlot *pEvtSlot) OVERRIDE;

    /**
     * @brief Create animator group object
     * @return IAnimatorGroup* animator group object
     */
    STDMETHOD_(IAnimatorGroup *, CreateAnimatorGroup)(THIS) OVERRIDE;
};

SNSEND
#endif /**< __SOUIFACTORY__H__ */