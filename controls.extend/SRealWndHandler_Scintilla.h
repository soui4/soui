#pragma once

#include <helper/obj-ref-impl.hpp>

namespace SOUI
{
    class SRealWndHandler_Scintilla :public TObjRefImpl2<IRealWndHandler,SRealWndHandler_Scintilla>
    {
    public:
        SRealWndHandler_Scintilla(void);
        ~SRealWndHandler_Scintilla(void);

        /**
         * SRealWnd::OnRealWndCreate
         * @brief    创建真窗口
         * @param    SRealWnd * pRealWnd --  窗口指针
         * @return   HWND -- 创建出来的真窗口句柄
         * Describe  
         */    
        virtual HWND WINAPI OnRealWndCreate(IWindow *pRealWnd);

        /**
        * SRealWnd::OnRealWndDestroy
        * @brief    销毁窗口
        * @param    SRealWnd *pRealWnd -- 窗口指针
        *
        * Describe  销毁窗口
        */
        virtual void WINAPI OnRealWndDestroy(IWindow *pRealWnd);

        /**
        * SRealWnd::OnRealWndInit
        * @brief    初始化窗口
        * @param    SRealWnd *pRealWnd -- 窗口指针
        *
        * Describe  初始化窗口
        */
        virtual BOOL WINAPI OnRealWndInit(IWindow *pRealWnd);

        /**
        * SRealWnd::OnRealWndSize
        * @brief    调整窗口大小
        * @param    SRealWnd *pRealWnd -- 窗口指针
        * @return   BOOL -- TRUE:用户管理窗口的移动；FALSE：交由SOUI自己管理。
        * Describe  调整窗口大小, 从pRealWnd中获得窗口位置。
        */
        virtual BOOL WINAPI OnRealWndSize(IWindow *pRealWnd);

		virtual BOOL WINAPI OnRealWndPosition(IWindow *pRealWnd, const RECT *rcWnd);

	};

}
