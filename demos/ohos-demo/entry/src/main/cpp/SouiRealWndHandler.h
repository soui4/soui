/**
 * SouiRealWndHandler.h
 *
 * SOUI RealWnd 处理器（OHOS 版本），对应 Android 端 CSouiRealWndHandler。
 * 处理 SRealWnd 控件创建/销毁真实窗口（如 NativeEditView）的回调。
 */
#pragma once

#include <helper/obj-ref-impl.hpp>

SNSBEGIN

class CSouiRealWndHandler : public TObjRefImpl2<IRealWndHandler, CSouiRealWndHandler> {
public:
    CSouiRealWndHandler(void);
    ~CSouiRealWndHandler(void) override = default;

    STDMETHOD_(HWND, OnRealWndCreate)(THIS_ IWindow * pRealWnd) OVERRIDE;
    STDMETHOD_(void, OnRealWndDestroy)(THIS_ IWindow * pRealWnd) OVERRIDE;
    STDMETHOD_(BOOL, OnRealWndInit)(THIS_ IWindow * pRealWnd) OVERRIDE;
    STDMETHOD_(BOOL, OnRealWndPosition)(THIS_ IWindow * pRealWnd, const RECT *rcWnd) OVERRIDE;
};

SNSEND
