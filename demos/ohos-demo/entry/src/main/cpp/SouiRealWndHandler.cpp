/**
 * SouiRealWndHandler.cpp
 *
 * SOUI RealWnd 处理器（OHOS 版本），对应 Android 端 CSouiRealWndHandler。
 * 通过 CreateWindowEx 创建虚拟 HWND（由 swinx OHOS 平台层映射到 ArkTS 组件）。
 */
#include <souistd.h>
#include "SouiRealWndHandler.h"

SNSBEGIN

CSouiRealWndHandler::CSouiRealWndHandler(void) {}

HWND CSouiRealWndHandler::OnRealWndCreate(IWindow *pRealWnd1) {
    SRealWnd *pRealWnd = sobj_cast<SRealWnd>(pRealWnd1);
    return ::CreateWindowEx(pRealWnd->GetRealStyleEx(),
                            pRealWnd->GetRealClassName()->c_str(),
                            pRealWnd->GetRealWindowName()->c_str(),
                            pRealWnd->GetRealStyle() | WS_CHILD,
                            0, 0, 0, 0,
                            pRealWnd->GetContainer()->GetHostHwnd(),
                            pRealWnd->GetID(),
                            SApplication::getSingleton().GetModule(),
                            NULL);
}

void CSouiRealWndHandler::OnRealWndDestroy(IWindow *pRealWnd1) {
    SRealWnd *pRealWnd = sobj_cast<SRealWnd>(pRealWnd1);
    ::DestroyWindow(pRealWnd->GetRealHwnd());
}

BOOL CSouiRealWndHandler::OnRealWndInit(IWindow *pRealWnd) {
    return FALSE;
}

BOOL CSouiRealWndHandler::OnRealWndPosition(THIS_ IWindow * pRealWnd, const RECT *rcWnd) {
    return FALSE;
}

SNSEND
