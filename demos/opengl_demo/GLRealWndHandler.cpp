#include "stdafx.h"
#include "GLRealWndHandler.h"
#include "GLWnd.h"

HWND CGLRealWndHandler::OnRealWndCreate(IWindow *pReal)
{
    SAutoRefPtr<IRealWnd> pRealWnd;
    pReal->QueryInterface(IID_IRealWnd, (IObjRef **)&pRealWnd);
    if (!pRealWnd)
        return 0;

    // 只接管 wndclass 指向本 demo 的 GL 窗口，其它真窗口类型不插手
    const IStringT *pszClass = pRealWnd->GetRealClassName();
    if (!pszClass || pszClass->CompareNoCase(CGLWnd::GetWndClassName()) != 0)
        return 0;

    RECT rc = {0, 0, 0, 0};
    pReal->GetClientRect(&rc);

    CGLWnd *pWnd = new CGLWnd();
    if (!pWnd->Create(pRealWnd->GetRealWindowName()->c_str(), WS_CHILD, rc,
                      pReal->GetContainer()->GetHostHwnd(),
                      SApplication::getSingleton().GetModule()))
    {
        delete pWnd;
        return 0;
    }
    pRealWnd->SetData(pWnd);
    return pWnd->m_hWnd;
}

void CGLRealWndHandler::OnRealWndDestroy(IWindow *pReal)
{
    SAutoRefPtr<IRealWnd> pRealWnd;
    pReal->QueryInterface(IID_IRealWnd, (IObjRef **)&pRealWnd);
    if (!pRealWnd)
        return;

    CGLWnd *pWnd = (CGLWnd *)pRealWnd->GetData();
    if (pWnd)
    {
        pRealWnd->SetData(NULL);
        pWnd->DestroyWindow();
        delete pWnd;
    }
}

BOOL CGLRealWndHandler::OnRealWndInit(IWindow *pRealWnd)
{
    return FALSE;
}

BOOL CGLRealWndHandler::OnRealWndPosition(IWindow *pRealWnd, const RECT *rcWnd)
{
    return FALSE;
}
