#include <souistd.h>
#include "SouiRealWndHandler.h"

SNSBEGIN

CSouiRealWndHandler::CSouiRealWndHandler(void) {}
CSouiRealWndHandler::~CSouiRealWndHandler(void) {}

HWND CSouiRealWndHandler::OnRealWndCreate(IWindow *pRealWnd1) {
    SRealWnd *pRealWnd = sobj_cast<SRealWnd>(pRealWnd1);
    return ::CreateWindowEx(pRealWnd->GetRealStyleEx(),
                            pRealWnd->GetRealClassName()->c_str(),
                            pRealWnd->GetRealWindowName()->c_str(),
                            pRealWnd->GetRealStyle()|WS_CHILD,
                            0,0,0,0,
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
