#pragma once

#include <helper/obj-ref-impl.hpp>

namespace SOUI
{
    class CSouiRealWndHandler :public TObjRefImpl2<IRealWndHandler,CSouiRealWndHandler>
    {
    public:
        CSouiRealWndHandler(void);
        ~CSouiRealWndHandler(void);

    /**
     * SRealWnd::OnRealWndCreate
     * @brief    窗口创建
     * @param    SRealWnd *pRealWnd -- 窗口指针
     *
     * Describe  窗口创建
     */
    STDMETHOD_(HWND, OnRealWndCreate)(THIS_ IWindow * pRealWnd) OVERRIDE;

    /**
     * SRealWnd::OnRealWndDestroy
     * @brief    销毁窗口
     * @param    SRealWnd *pRealWnd -- 窗口指针
     *
     * Describe  销毁窗口
     */
    STDMETHOD_(void, OnRealWndDestroy)(THIS_ IWindow * pRealWnd) OVERRIDE;

    /**
     * SRealWnd::OnRealWndInit
     * @brief    初始化窗口
     * @param    SRealWnd *pRealWnd -- 窗口指针
     * @return   BOOL -- FALSE:交由系统处理，TRUE:用户处理
     *
     * Describe  初始化窗口
     */
    STDMETHOD_(BOOL, OnRealWndInit)(THIS_ IWindow * pRealWnd) OVERRIDE;

    /**
     * SRealWnd::OnRealWndPosition
     * @brief    调整窗口位置
     * @param    SRealWnd *pRealWnd -- 窗口指针
     * @return   BOOL -- FALSE：交由SOUI处理; TRUE:用户管理窗口的移动
     *
     * Describe  调整窗口大小
     */
	STDMETHOD_(BOOL, OnRealWndPosition)(THIS_ IWindow * pRealWnd, const RECT *rcWnd) OVERRIDE;

    };

}
