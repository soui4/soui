#include <SNativeWnd.h>
#include "UiState.h"
#include <string.h>

SNSBEGIN

SNativeWnd::SNativeWnd():m_hWnd(0),mConnection(nullptr),mScreen(nullptr),m_gc(0){

}

SNativeWnd::~SNativeWnd(void)
{
}

HWND  SNativeWnd::CreateNative
(LPCTSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, int nID, LPVOID lpParam ){
    m_id = nID;

    SThreadUiState *state = SUiState::instance()->getThreadUiState();
    mConnection = state->connection;
    mScreen = state->screen;

    m_hWnd = xcb_generate_id(mConnection);

    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t value_list[2] = {
        mScreen->black_pixel,
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS |
            XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
            XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
            XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | 
            XCB_EVENT_MASK_RESIZE_REDIRECT|XCB_EVENT_MASK_FOCUS_CHANGE|
            XCB_EVENT_MASK_VISIBILITY_CHANGE|XCB_EVENT_MASK_STRUCTURE_NOTIFY};

    xcb_create_window(mConnection, XCB_COPY_FROM_PARENT, m_hWnd,
                      mScreen->root, x, y, nWidth, nHeight, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, mScreen->root_visual, mask,
                      value_list);

    m_gc = xcb_generate_id(mConnection);
// 设置绘图上下文属性
    uint32_t value_mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    value_list[0] = mScreen->black_pixel;
    value_list[1] = 0;

    xcb_create_gc(mConnection, m_gc, m_hWnd, value_mask, value_list);

    xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, m_hWnd,
        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(lpWindowName), lpWindowName);

    xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, m_hWnd, state->wm_protocols_atom, XCB_ATOM_ATOM, 32, 1, &state->wm_delete_window_atom);

    xcb_map_window(mConnection, m_hWnd);

    const unsigned coords[] = {static_cast<unsigned>(x),
                               static_cast<unsigned>(y)};
    xcb_configure_window(mConnection, m_hWnd,
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);

    state->onWndCreate(m_hWnd,this);
    xcb_flush(mConnection);
    
    return m_hWnd;
}

HWND  SNativeWnd::GetHwnd(){
    return m_hWnd;
}

BOOL SNativeWnd::DestroyWindow(){
    if(!m_hWnd)
        return FALSE;
    xcb_destroy_window(mConnection,m_hWnd);
    SThreadUiState *state = SUiState::instance()->getThreadUiState();
    state->onWndDestroy(m_hWnd);
    m_hWnd = 0;
    return TRUE;
}

void SNativeWnd::OnFinalMessage(HWND hWnd)
{ 
    //todo:hjx
    m_hWnd = NULL;
}

LRESULT CALLBACK SNativeWnd::WindowProc(SNativeWnd * pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SNativeWnd *pThis = (SNativeWnd *)pWnd; 
    MSG msg = { pThis->m_hWnd, uMsg, wParam, lParam };
    const MSG *pOldMsg = pThis->m_pCurrentMsg;
    pThis->m_pCurrentMsg = &msg;
    LRESULT lRes = 0;
    if (pThis->m_msgHandlerInfo.fun)
    {
        BOOL bHandled = pThis->m_msgHandlerInfo.fun(&msg, &lRes, pThis->m_msgHandlerInfo.ctx);
        if (bHandled)
            return lRes;
    }

    // pass to the message map to process
    BOOL bRet = pThis->ProcessWindowMessage(pThis->m_hWnd, uMsg, wParam, lParam, lRes, 0);
    // restore saved value for the current message
    SASSERT(pThis->m_pCurrentMsg == &msg);

    // do the default processing if message was not handled
    if (!bRet)
    {
        if (uMsg != WM_NCDESTROY)
            lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
        else
        {
            lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
            // mark window as destryed
            pThis->m_bDestoryed = TRUE;
        }
    }
    if ((pThis->m_bDestoryed) && pOldMsg == NULL)
    {
        // clear out window handle
        HWND hWndThis = pThis->m_hWnd;
        pThis->m_hWnd = NULL;
        pThis->m_bDestoryed = FALSE;
        // clean up after window is destroyed
        pThis->m_pCurrentMsg = pOldMsg;
        pThis->OnFinalMessage(hWndThis);
    }
    else
    {
        pThis->m_pCurrentMsg = pOldMsg;
    }
    return lRes;
}

BOOL SNativeWnd::SubclassWindow(HWND hWnd)
{
    return FALSE;
}

HWND SNativeWnd::UnsubclassWindow(BOOL bForce /*= FALSE*/)
{
    return 0;
}

LRESULT SNativeWnd::ForwardNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    LRESULT lResult = 0;
    switch (uMsg)
    {
    case WM_COMMAND:
    case WM_NOTIFY:
    case WM_PARENTNOTIFY:
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
    case WM_COMPAREITEM:
    case WM_DELETEITEM:
    case WM_VKEYTOITEM:
    case WM_CHARTOITEM:
    case WM_HSCROLL:
    case WM_VSCROLL:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
        bHandled = TRUE;
        //lResult = ::SendMessage(GetParent(), uMsg, wParam, lParam);
        break;
    default:
        bHandled = FALSE;
        break;
    }
    return lResult;
}

LRESULT SNativeWnd::ReflectNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    return 0;
}

BOOL SNativeWnd::DefaultReflectionHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult)
{
    return FALSE;
}

LRESULT SNativeWnd::DefWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return 0;
    //return ::CallWindowProc(m_pfnSuperWindowProc, m_hWnd, uMsg, wParam, lParam);
}

LRESULT SNativeWnd::DefWindowProc()
{
    const MSG *pMsg = m_pCurrentMsg;
    LRESULT lRes = 0;
    if (pMsg != NULL)
        lRes = DefWindowProc(pMsg->message, pMsg->wParam, pMsg->lParam);
    return lRes;
}

BOOL SNativeWnd::CenterWindow(HWND hWndCenter /*= NULL*/)
{
    SASSERT(m_hWnd!=0);
    return FALSE;
}

BOOL SNativeWnd::ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags /*= 0*/)
{
    return FALSE;
}

BOOL SNativeWnd::ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags /*= 0*/)
{
    return FALSE;
}

BOOL SNativeWnd::ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult, DWORD dwMsgMapID /*= 0*/)
{
    return FALSE;
}

BOOL SNativeWnd::UpdateLayeredWindow(HDC hdcDst, POINT *pptDst, SIZE *psize, HDC hdcSrc, POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags)
{
    return FALSE;
}

BOOL SNativeWnd::SetLayeredWindowAttributes(COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
    return FALSE;}

int SNativeWnd::SetWindowRgn(HRGN hRgn, BOOL bRedraw /*=TRUE*/)
{
    return 0;
}

BOOL SNativeWnd::ShowWindow(int nCmdShow)
{
    SASSERT(m_hWnd!=0);
    return ::ShowWindow(m_hWnd, nCmdShow);
}

BOOL SNativeWnd::MoveWindow2(LPCRECT lpRect, BOOL bRepaint /*= TRUE*/)
{
    return MoveWindow(lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top, bRepaint);
}

BOOL SNativeWnd::MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint /*= TRUE*/)
{
    SASSERT(m_hWnd!=0);
    return ::MoveWindow(m_hWnd, x, y, nWidth, nHeight, bRepaint);
}

BOOL SNativeWnd::IsWindowVisible() const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::IsWindowVisible(m_hWnd);
}

BOOL SNativeWnd::IsZoomed() const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::IsZoomed(m_hWnd);
}

BOOL SNativeWnd::IsIconic() const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::IsIconic(m_hWnd);
}

int SNativeWnd::GetWindowText(LPTSTR lpszStringBuf, int nMaxCount) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::GetWindowText(m_hWnd, lpszStringBuf, nMaxCount);
}

BOOL SNativeWnd::SetWindowText(LPCTSTR lpszString)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetWindowText(m_hWnd, lpszString);
}

BOOL SNativeWnd::SendNotifyMessage(UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
    return FALSE;
}

BOOL SNativeWnd::PostMessage(UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
    return FALSE;
}

LRESULT SNativeWnd::SendMessage(UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
    return FALSE;
}

HWND SNativeWnd::SetFocus()
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetFocus(m_hWnd);
}

HWND SNativeWnd::SetCapture()
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetCapture(m_hWnd);
}

HWND SNativeWnd::GetCapture()
{
    return ::GetCapture();
}

BOOL SNativeWnd::ReleaseCapture()
{
    return ::ReleaseCapture();
}

BOOL SNativeWnd::ShowCaret()
{
    SASSERT(::IsWindow(m_hWnd));
    return ::ShowCaret(m_hWnd);
}

BOOL SNativeWnd::HideCaret()
{
    SASSERT(::IsWindow(m_hWnd));
    return ::HideCaret(m_hWnd);
}

BOOL SNativeWnd::CreateCaret(HBITMAP hBitmap)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::CreateCaret(m_hWnd, hBitmap, 0, 0);
}

int SNativeWnd::ReleaseDC(HDC hDC)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::ReleaseDC(m_hWnd, hDC);
}

HDC SNativeWnd::GetWindowDC()
{
    SASSERT(::IsWindow(m_hWnd));
    return ::GetWindowDC(m_hWnd);
}

HDC SNativeWnd::GetDC()
{
    SASSERT(::IsWindow(m_hWnd));
    return ::GetDC(m_hWnd);
}

BOOL SNativeWnd::KillTimer(UINT_PTR nIDEvent)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::KillTimer(m_hWnd, nIDEvent);
}

UINT_PTR SNativeWnd::SetTimer(UINT_PTR nIDEvent, UINT nElapse, void(CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD) /*= NULL*/)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetTimer(m_hWnd, nIDEvent, nElapse, (TIMERPROC)lpfnTimer);
}

int SNativeWnd::MapWindowRect(HWND hWndTo, LPRECT lpRect) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::MapWindowPoints(m_hWnd, hWndTo, (LPPOINT)lpRect, 2);
}

int SNativeWnd::MapWindowPoints(HWND hWndTo, LPPOINT lpPoint, UINT nCount) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::MapWindowPoints(m_hWnd, hWndTo, lpPoint, nCount);
}

BOOL SNativeWnd::ScreenToClient2(LPRECT lpRect) const
{
    SASSERT(::IsWindow(m_hWnd));
    if (!::ScreenToClient(m_hWnd, (LPPOINT)lpRect))
        return FALSE;
    return ::ScreenToClient(m_hWnd, ((LPPOINT)lpRect) + 1);
}

BOOL SNativeWnd::ScreenToClient(LPPOINT lpPoint) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::ScreenToClient(m_hWnd, lpPoint);
}

BOOL SNativeWnd::ClientToScreen2(LPRECT lpRect) const
{
    SASSERT(::IsWindow(m_hWnd));
    if (!::ClientToScreen(m_hWnd, (LPPOINT)lpRect))
        return FALSE;
    return ::ClientToScreen(m_hWnd, ((LPPOINT)lpRect) + 1);
}

BOOL SNativeWnd::ClientToScreen(LPPOINT lpPoint) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::ClientToScreen(m_hWnd, lpPoint);
}

BOOL SNativeWnd::GetClientRect(LPRECT lpRect) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::GetClientRect(m_hWnd, lpRect);
}

BOOL SNativeWnd::GetWindowRect(LPRECT lpRect) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::GetWindowRect(m_hWnd, lpRect);
}

BOOL SNativeWnd::InvalidateRect(LPCRECT lpRect, BOOL bErase /*= TRUE*/)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::InvalidateRect(m_hWnd, lpRect, bErase);
}

BOOL SNativeWnd::Invalidate(BOOL bErase /*= TRUE*/)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::InvalidateRect(m_hWnd, NULL, bErase);
}

BOOL SNativeWnd::IsWindow() SCONST
{
    return m_hWnd!=0;
}

BOOL SNativeWnd::SetWindowPos(HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetWindowPos(m_hWnd, hWndInsertAfter, x, y, cx, cy, nFlags);
}

BOOL SNativeWnd::IsWindowEnabled() const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::IsWindowEnabled(m_hWnd);
}

HWND SNativeWnd::SetParent(HWND hWndNewParent)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetParent(m_hWnd, hWndNewParent);
}

HWND SNativeWnd::GetParent()
{
    SASSERT(::IsWindow(m_hWnd));
    return ::GetParent(m_hWnd);
}

LONG_PTR SNativeWnd::SetWindowLongPtr(int nIndex, LONG_PTR dwNewLong)
{
    SASSERT(::IsWindow(m_hWnd));
    return ::SetWindowLongPtr(m_hWnd, nIndex, dwNewLong);
}

LONG_PTR SNativeWnd::GetWindowLongPtr(int nIndex) const
{
    SASSERT(::IsWindow(m_hWnd));
    return ::GetWindowLongPtr(m_hWnd, nIndex);
}

DWORD SNativeWnd::GetExStyle() const
{
    SASSERT(::IsWindow(m_hWnd));
    return (DWORD)::GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
}

DWORD SNativeWnd::GetStyle() const
{
    SASSERT(::IsWindow(m_hWnd));
    return (DWORD)::GetWindowLongPtr(m_hWnd, GWL_STYLE);
}

int SNativeWnd::GetDlgCtrlID() const
{
    return 0;
}

const MSG *SNativeWnd::GetCurrentMessage() const
{
    return m_pCurrentMsg;
}

void SNativeWnd::SetMsgHandler(THIS_ FunMsgHandler fun, void *ctx)
{
    m_msgHandlerInfo.fun = fun;
    m_msgHandlerInfo.ctx = ctx;
}

MsgHandlerInfo *SNativeWnd::GetMsgHandler()
{
    return &m_msgHandlerInfo;
}

SNSEND