#pragma once

#include <helper/obj-ref-impl.hpp>

SNSBEGIN

class CSouiRealWndHandler : public TObjRefImpl2<IRealWndHandler, CSouiRealWndHandler> {
public:
    CSouiRealWndHandler(void);
    ~CSouiRealWndHandler(void);

    STDMETHOD_(HWND, OnRealWndCreate)(THIS_ IWindow * pRealWnd) OVERRIDE;
    STDMETHOD_(void, OnRealWndDestroy)(THIS_ IWindow * pRealWnd) OVERRIDE;
    STDMETHOD_(BOOL, OnRealWndInit)(THIS_ IWindow * pRealWnd) OVERRIDE;
    STDMETHOD_(BOOL, OnRealWndPosition)(THIS_ IWindow * pRealWnd, const RECT *rcWnd) OVERRIDE;
};

SNSEND
