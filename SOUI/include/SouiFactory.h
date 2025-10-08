#ifndef __SOUIFACTORY__H__
#define __SOUIFACTORY__H__
#include <interface/SFactory-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

class SOUI_EXP SouiFactory : public TObjRefImpl<ISouiFactory> {
  public:
    SouiFactory(void);
    ~SouiFactory(void);

  public:
    /**
     * @brief 创建应用程序对象
     * @param pRenderFac 渲染工厂对象
     * @param hInst 模块句柄
     * @param pszHostClassName 窗口类名
     * @param bImeApp 是否启用输入法支持
     * @return IApplication* 应用程序对象
     */
    STDMETHOD_(IApplication *, CreateApp)(THIS_ IRenderFactory *pRenderFac, HMODULE hInst, LPCTSTR pszHostClassName DEF_VAL(_T("SOUIHOST")), BOOL bImeApp DEF_VAL(FALSE)) OVERRIDE;

    /**
     * @brief 创建原生窗口对象
     * @return INativeWnd* 原生窗口对象
     */
    STDMETHOD_(INativeWnd *, CreateNativeWnd)(THIS) OVERRIDE;

    /**
     * @brief 创建宿主窗口对象
     * @param pszResID 资源ID
     * @return IHostWnd* 宿主窗口对象
     */
    STDMETHOD_(IHostWnd *, CreateHostWnd)(THIS_ LPCTSTR pszResID DEF_VAL(NULL)) OVERRIDE;

    /**
     * @brief 创建宿主对话框对象
     * @param pszResID 资源ID
     * @return IHostDialog* 宿主对话框对象
     */
    STDMETHOD_(IHostDialog *, CreateHostDialog)(THIS_ LPCTSTR pszResID DEF_VAL(NULL)) OVERRIDE;

    /**
     * @brief 创建ANSI字符串对象
     * @param pszSrc 源字符串
     * @return IStringA* ANSI字符串对象
     */
    STDMETHOD_(IStringA *, CreateStringA)(THIS_ LPCSTR pszSrc) OVERRIDE;

    /**
     * @brief 创建宽字符字符串对象
     * @param pszSrc 源字符串
     * @return IStringW* 宽字符字符串对象
     */
    STDMETHOD_(IStringW *, CreateStringW)(THIS_ LPCWSTR pszSrc) OVERRIDE;

    /**
     * @brief 创建XML文档对象
     * @return IXmlDoc* XML文档对象
     */
    STDMETHOD_(IXmlDoc *, CreateXmlDoc)(THIS) OVERRIDE;

    /**
     * @brief 创建资源提供者对象
     * @param resType 内置资源类型
     * @return IResProvider* 资源提供者对象
     */
    STDMETHOD_(IResProvider *, CreateResProvider)(THIS_ BUILTIN_RESTYPE resType) OVERRIDE;

    /**
     * @brief 创建事件槽对象
     * @param fun 回调函数
     * @param ctx 上下文指针
     * @return IEvtSlot* 事件槽对象
     */
    STDMETHOD_(IEvtSlot *, CreateFuncSlot)(THIS_ FunCallback fun, void *ctx) OVERRIDE;

    /**
     * @brief 创建菜单对象
     * @param hMenu 菜单句柄
     * @return IMenu* 菜单对象
     */
    STDMETHOD_(IMenu *, CreateMenu)(THIS_ HMENU hMenu DEF_VAL(0)) OVERRIDE;

    /**
     * @brief 创建扩展菜单对象
     * @return IMenuEx* 扩展菜单对象
     */
    STDMETHOD_(IMenuEx *, CreateMenuEx)(THIS) OVERRIDE;

    /**
     * @brief 创建定时器对象
     * @param pEvtSlot 事件槽对象
     * @return ITimer* 定时器对象
     */
    STDMETHOD_(ITimer *, CreateTimer)(THIS_ IEvtSlot *pEvtSlot) OVERRIDE;

    /**
     * @brief 创建动画组对象
     * @return IAnimatorGroup* 动画组对象
     */
    STDMETHOD_(IAnimatorGroup *, CreateAnimatorGroup)(THIS) OVERRIDE;

    /**
     * @brief 创建AppCfg对象
     * @return IAppCfg* AppCfg对象
  */
    STDMETHOD_(IAppCfg *, CreateAppCfg)(THIS) OVERRIDE;
};

SNSEND
#endif // __SOUIFACTORY__H__