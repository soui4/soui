/**
 * @file       GLRealWndHandler.h
 * @brief      把 <realwnd/> 变成 OpenGL 子窗口的 IRealWndHandler。
 *
 * @details    SApplication 全局只保存一个 IRealWndHandler，SRealWnd 创建时通过
 *             GETREALWNDHANDLER 取它。默认实现只是把事件转发给控件，
 *             这里替换成"创建 SNativeWnd 派生的 GL 子窗口"，
 *             由 pRealWnd->SetData() 挂对象指针，销毁时再取回来释放。
 */

#ifndef __SOUI_GL_REALWND_HANDLER_H__
#define __SOUI_GL_REALWND_HANDLER_H__

#include <helper/obj-ref-impl.hpp>
#include <interface/SRealWndHandler-i.h>

class CGLRealWndHandler : public TObjRefImpl2<IRealWndHandler, CGLRealWndHandler>
{
  public:
    /** @brief 创建真窗口：只有 wndclass 匹配 CGLWnd 的才接管，其它返回 0 交给默认逻辑 */
    HWND WINAPI OnRealWndCreate(IWindow *pRealWnd) OVERRIDE;

    /** @brief 销毁真窗口并释放挂在上面的 CGLWnd */
    void WINAPI OnRealWndDestroy(IWindow *pRealWnd) OVERRIDE;

    /** @brief 返回 FALSE：初始化由 SOUI 内部继续完成 */
    BOOL WINAPI OnRealWndInit(IWindow *pRealWnd) OVERRIDE;

    /** @brief 返回 FALSE：位置/大小交给 SRealWnd::SetRealWndPos 用 SetWindowPos 处理 */
    BOOL WINAPI OnRealWndPosition(IWindow *pRealWnd, const RECT *rcWnd) OVERRIDE;
};

#endif // __SOUI_GL_REALWND_HANDLER_H__
