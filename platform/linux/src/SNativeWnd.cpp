#include <SNativeWnd.h>
#include "UiState.h"
#include <string.h>

SNSBEGIN

class SNativeHelper{
public:
    static SNativeHelper* instance(){
        SNativeHelper _this;
        return &_this;
    }

    BOOL Init(HINSTANCE hInst, LPCTSTR pszClassName, BOOL bImeApp){
        WNDCLASSEX wndCls={sizeof(WNDCLASSEX),0};
        wndCls.lpszClassName = pszClassName;
        wndCls.lpfnWndProc = SNativeWnd::StartWindowProc;
        m_atom = RegisterClassEx(&wndCls);
    }

    void * GetSharePtr(){
        return m_ptr;
    }

    void SetSharePtr(void *ptr){
        m_ptr = ptr;
    }

    ATOM getAtom(){
        return m_atom;
    }
private:
SNativeHelper():m_atom(0),m_ptr(nullptr){

}
~SNativeHelper(){}

ATOM m_atom;
void * m_ptr;
};

BOOL SNativeWnd::InitWndClass(HINSTANCE hInst,LPCTSTR pszHostClassName,BOOL bImeApp){
    return TRUE;
}

SNativeWnd::SNativeWnd():m_hWnd(0),mConnection(nullptr),mScreen(nullptr),m_gc(0){

}

SNativeWnd::~SNativeWnd(void)
{
}

HWND  SNativeWnd::CreateNative
(LPCTSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, int nID, LPVOID lpParam ){
    SNativeHelper::instance()->SetSharePtr(this);
    HWND hWnd =::CreateWindowEx(dwExStyle,(LPCSTR)SNativeHelper::instance()->getAtom(),lpWindowName,dwStyle,x,y,nWidth,nHeight,hWndParent,nID,0,lpParam);
    SNativeHelper::instance()->SetSharePtr(nullptr);
    SThreadUiState *state = SUiState::instance()->getThreadUiState();
    mConnection = state->connection;
    mScreen = state->screen;

    //todo:hjx    
    return m_hWnd;
}

HWND  SNativeWnd::GetHwnd(){
    return m_hWnd;
}

BOOL SNativeWnd::DestroyWindow(){
    BOOL bRet = ::DestroyWindow(m_hWnd);
    m_hWnd = 0;
    return bRet;
}

void SNativeWnd::OnFinalMessage(HWND hWnd)
{ 
    //todo:hjx
    m_hWnd = 0;
}

LRESULT CALLBACK SNativeWnd::StartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SNativeWnd *pThis = (SNativeWnd *)SNativeHelper::instance()->GetSharePtr();
    ::SetWindowLongPtr(hWnd,GWLP_OPAQUE,(LONG_PTR)pThis);
    ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);
    return WindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK SNativeWnd::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SNativeWnd *pThis = (SNativeWnd *)::GetWindowLongPtr(hWnd,GWLP_OPAQUE); 
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
        pThis->m_hWnd = 0;
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

 BOOL SNativeWnd::SetLayeredWindowAlpha(THIS_ BYTE byAlpha) {
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
    if(nCmdShow & SW_SHOW)
        xcb_map_window(mConnection, m_hWnd);
    else
        xcb_unmap_window(mConnection, m_hWnd);
    xcb_flush(mConnection);
    return TRUE;
}

BOOL SNativeWnd::MoveWindow2(LPCRECT lpRect, BOOL bRepaint /*= TRUE*/)
{
    return MoveWindow(lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top, bRepaint);
}

BOOL SNativeWnd::MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint /*= TRUE*/)
{
    SASSERT(m_hWnd!=0);
    {
            const unsigned coords[] = {static_cast<unsigned>(x),
                               static_cast<unsigned>(y)};
    xcb_configure_window(mConnection, m_hWnd,
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);

    }
    {
    const uint32_t vals[2] = {(uint32_t)nWidth, (uint32_t)nHeight};
    xcb_configure_window(mConnection, m_hWnd,
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                         vals);
    }
    return TRUE;
}

BOOL SNativeWnd::IsWindowVisible() const
{
    SASSERT(IsWindow());
    xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(mConnection, m_hWnd);
    xcb_get_window_attributes_reply_t *reply = xcb_get_window_attributes_reply(mConnection, cookie, NULL);
    if (!reply)
        return FALSE;
    uint8_t mapState = reply->map_state;
    free(reply);
    return mapState == XCB_MAP_STATE_VIEWABLE;
}

BOOL SNativeWnd::IsZoomed() const
{
    SASSERT(IsWindow());
    SThreadUiState *state = SUiState::instance()->getThreadUiState();
    xcb_get_property_cookie_t cookie = xcb_get_property(mConnection, 0, m_hWnd, XCB_ATOM_ATOM, state->wm_stat_atom, 0, 1024);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(mConnection, cookie, NULL);
    if (!reply) return FALSE;
    xcb_atom_t *atoms = (xcb_atom_t*)xcb_get_property_value(reply);
    int num_atoms = xcb_get_property_value_length(reply) / sizeof(xcb_atom_t);

    //todo: 遍历窗口类型属性，查找最大化和最小化状态
free(reply);
    return FALSE;
}

BOOL SNativeWnd::IsIconic() const
{
    SASSERT(IsWindow());
    SThreadUiState *state = SUiState::instance()->getThreadUiState();
    xcb_get_property_cookie_t cookie = xcb_get_property(mConnection, 0, m_hWnd, XCB_ATOM_ATOM, state->wm_stat_atom, 0, 1024);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(mConnection, cookie, NULL);
    if (!reply) return FALSE;
    xcb_atom_t *atoms = (xcb_atom_t*)xcb_get_property_value(reply);
    int num_atoms = xcb_get_property_value_length(reply) / sizeof(xcb_atom_t);
    free(reply);
    return FALSE;
}

int SNativeWnd::GetWindowText(LPTSTR lpszStringBuf, int nMaxCount) const
{
    SASSERT(IsWindow());
    if(nMaxCount < m_strTitle.length())
    {
        strncpy(lpszStringBuf,m_strTitle.c_str(),nMaxCount);
        return nMaxCount;
    }else{
        strcpy(lpszStringBuf, m_strTitle.c_str());
        return m_strTitle.length();
    }
}

BOOL SNativeWnd::SetWindowText(LPCTSTR lpszString)
{
    SASSERT(IsWindow());
    m_strTitle = lpszString;
    xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, m_hWnd, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, m_strTitle.length(), m_strTitle.c_str());

    return TRUE;
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
    SASSERT(IsWindow());
    return ::SetFocus(m_hWnd);
}

HWND SNativeWnd::SetCapture()
{
    SASSERT(IsWindow());
    //return ::SetCapture(m_hWnd);
    return 0;
}

HWND SNativeWnd::GetCapture()
{
    //return ::GetCapture();
    return 0;
}

BOOL SNativeWnd::ReleaseCapture()
{
    //return ::ReleaseCapture();
    return 0;
}

BOOL SNativeWnd::ShowCaret()
{
    SASSERT(IsWindow());
    return ::ShowCaret(m_hWnd);
}

BOOL SNativeWnd::HideCaret()
{
    SASSERT(IsWindow());
    return ::HideCaret(m_hWnd);
}

BOOL SNativeWnd::CreateCaret(HBITMAP hBitmap)
{
    SASSERT(IsWindow());
    return ::CreateCaret(m_hWnd, hBitmap, 0, 0);
}

int SNativeWnd::ReleaseDC(HDC hDC)
{
    SASSERT(IsWindow());
    //return ::ReleaseDC(m_hWnd, hDC);
    return 0;
}

HDC SNativeWnd::GetWindowDC()
{
    SASSERT(IsWindow());
    //return ::GetWindowDC(m_hWnd);
    return 0;
}

HDC SNativeWnd::GetDC()
{
    SASSERT(IsWindow());
    //return ::GetDC(m_hWnd);
    return 0;
}

BOOL SNativeWnd::KillTimer(UINT_PTR nIDEvent)
{
    SASSERT(IsWindow());
    return ::KillTimer(m_hWnd, nIDEvent);
}

UINT_PTR SNativeWnd::SetTimer(UINT_PTR nIDEvent, UINT nElapse, void(CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD) /*= NULL*/)
{
    SASSERT(IsWindow());
    return ::SetTimer(m_hWnd, nIDEvent, nElapse, (TIMERPROC)lpfnTimer);
}

int SNativeWnd::MapWindowRect(HWND hWndTo, LPRECT lpRect) const
{
    SASSERT(IsWindow());
    //return ::MapWindowPoints(m_hWnd, hWndTo, (LPPOINT)lpRect, 2);
    return 0;
}

int SNativeWnd::MapWindowPoints(HWND hWndTo, LPPOINT lpPoint, UINT nCount) const
{
    SASSERT(IsWindow());
    //return ::MapWindowPoints(m_hWnd, hWndTo, lpPoint, nCount);
    return 0;
}

BOOL SNativeWnd::ScreenToClient2(LPRECT lpRect) const
{
    SASSERT(IsWindow());
    if (!::ScreenToClient(m_hWnd, (LPPOINT)lpRect))
         return FALSE;
    return ::ScreenToClient(m_hWnd, ((LPPOINT)lpRect) + 1);
}

BOOL SNativeWnd::ScreenToClient(LPPOINT lpPoint) const
{
    SASSERT(IsWindow());
    return ::ScreenToClient(m_hWnd, lpPoint);
}

BOOL SNativeWnd::ClientToScreen2(LPRECT lpRect) const
{
    SASSERT(IsWindow());
     if (!::ClientToScreen(m_hWnd, (LPPOINT)lpRect))
         return FALSE;
     return ::ClientToScreen(m_hWnd, ((LPPOINT)lpRect) + 1);
}

BOOL SNativeWnd::ClientToScreen(LPPOINT lpPoint) const
{
    SASSERT(IsWindow());
    return ::ClientToScreen(m_hWnd, lpPoint);
}

BOOL SNativeWnd::GetClientRect(LPRECT lpRect) const
{
    SASSERT(IsWindow());
 //   return ::GetClientRect(m_hWnd, lpRect);
 return 0;
}

BOOL SNativeWnd::GetWindowRect(LPRECT lpRect) const
{
    SASSERT(IsWindow());
 //   return ::GetWindowRect(m_hWnd, lpRect);
 return 0;
}

BOOL SNativeWnd::InvalidateRect(LPCRECT lpRect, BOOL bErase /*= TRUE*/)
{
    SASSERT(IsWindow());
//    return ::InvalidateRect(m_hWnd, lpRect, bErase);
return 0;
}

BOOL SNativeWnd::Invalidate(BOOL bErase /*= TRUE*/)
{
    SASSERT(IsWindow());
 //   return ::InvalidateRect(m_hWnd, NULL, bErase);
 return 0;
}

BOOL SNativeWnd::IsWindow() SCONST
{
    return m_hWnd!=0;
}

BOOL SNativeWnd::SetWindowPos(HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags)
{
  SASSERT(IsWindow());
  return ::SetWindowPos(m_hWnd, hWndInsertAfter, x, y, cx, cy, nFlags);
}

BOOL SNativeWnd::IsWindowEnabled() const
{
    SASSERT(IsWindow());
    return ::IsWindowEnabled(m_hWnd);
}

HWND SNativeWnd::SetParent(HWND hWndNewParent)
{
    SASSERT(IsWindow());
    return ::SetParent(m_hWnd, hWndNewParent);
}

HWND SNativeWnd::GetParent()
{
    SASSERT(IsWindow());
    return ::GetParent(m_hWnd);
}

LONG_PTR SNativeWnd::SetWindowLongPtr(int nIndex, LONG_PTR dwNewLong)
{
    SASSERT(IsWindow());
    return ::SetWindowLongPtr(m_hWnd, nIndex, dwNewLong);
}

LONG_PTR SNativeWnd::GetWindowLongPtr(int nIndex) const
{
    SASSERT(IsWindow());
    return ::GetWindowLongPtr(m_hWnd, nIndex);
}

DWORD SNativeWnd::GetExStyle() const
{
    SASSERT(IsWindow());
    //return (DWORD)::GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
    return 0;
}

DWORD SNativeWnd::GetStyle() const
{
    SASSERT(IsWindow());
    //return (DWORD)::GetWindowLongPtr(m_hWnd, GWL_STYLE);
    return 0;
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