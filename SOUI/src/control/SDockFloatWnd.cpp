#include <souistd.h>
#include <control/SDockFloatWnd.h>
#include <control/SDockBar.h>

SNSBEGIN

SDockFloatWnd::SDockFloatWnd()
    : m_pDockBar(NULL)
{
}

SDockFloatWnd::~SDockFloatWnd()
{
}

void SDockFloatWnd::OnFinalMessage(HWND hwnd)
{
    __baseCls::OnFinalMessage(hwnd);
    delete this;
}

BOOL SDockFloatWnd::Create(SDockBar *pDockBar, HWND hOwner, const CPoint &ptScreen, const CSize &szFloat)
{
    m_pDockBar = pDockBar;

    // Build an in-memory layout: a frame layout host fills the whole window.
    SXmlDoc xmlDoc;
    SXmlNode xmlSOUI = xmlDoc.root().append_child(L"SOUI");
    xmlSOUI.append_attribute(L"title").set_value(L"");
    xmlSOUI.append_attribute(L"resizable").set_value(true);
    xmlSOUI.append_attribute(L"toolWindow").set_value(true);

    SXmlNode xmlRoot = xmlSOUI.append_child(L"root");
    xmlRoot.append_attribute(L"layout").set_value(L"frame");

    SXmlNode xmlFloatHost = xmlRoot.append_child(L"window");
    xmlFloatHost.append_attribute(L"name").set_value(L"float_host");
    xmlFloatHost.append_attribute(L"layout").set_value(L"frame");
    xmlFloatHost.append_attribute(L"dock").set_value(L"mainview");

    HWND hWnd = CreateEx(hOwner, WS_POPUP | WS_CLIPCHILDREN, WS_EX_TOOLWINDOW, ptScreen.x, ptScreen.y, szFloat.cx, szFloat.cy, &xmlSOUI);
    if (!hWnd)
    {
        delete this;
        return FALSE;
    }

    SWindow *pFloatHost = FindChildByName(L"float_host");
    if (!pFloatHost)
    {
        DestroyWindow(); // OnFinalMessage will release this object
        return FALSE;
    }

    // Reparent the dock bar into the float host; its layout param is
    // mainview, so it fills the whole host.
    pFloatHost->InsertChild(pDockBar);
    ShowWindow(SW_SHOW);
    return TRUE;
}

void SDockFloatWnd::MoveTo(const CPoint &ptTopLeft)
{
    if (m_hWnd && ::IsWindow(m_hWnd))
    {
        ::SetWindowPos(m_hWnd, NULL, ptTopLeft.x, ptTopLeft.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void SDockFloatWnd::RequestClose()
{
    if (m_hWnd && ::IsWindow(m_hWnd))
    {
        ::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
    }
}

LRESULT SDockFloatWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (m_pDockBar && m_pDockBar->IsFloating())
    {
        // Dock the dock bar back; Dock() posts another WM_CLOSE to destroy
        // this window once the reparenting is finished.
        m_pDockBar->Dock();
        return 0;
    }
    SetMsgHandled(FALSE);
    return 0;
}

SNSEND
