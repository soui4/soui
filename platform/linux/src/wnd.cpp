#include <wnd.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <map>
#include <mutex>
#include <string>
#include "class.h"
#include "UiState.h"
using namespace SOUI;

#define CLS_WINDOW "window"

HWND WIN_CreateWindowEx( CREATESTRUCT *cs, LPCSTR className, HINSTANCE module);

struct _Window{
    std::recursive_mutex mutex;
    pthread_t tid;
    xcb_connection_t *mConnection;
    xcb_screen_t *mScreen;
    uint32_t gc;

    std::string title; 
    UINT_PTR           objOpaque;
    HWND               parent;        /* Window parent */
    HWND               owner;         /* Window owner */
    WNDPROC            winproc;       /* Window procedure */
    HINSTANCE          hInstance;     /* Window hInstance (from CreateWindow) */
    UINT               dwStyle;       /* Window style (from CreateWindow) */
    UINT               dwExStyle;     /* Extended style (from CreateWindowEx) */
    UINT_PTR           wIDmenu;       /* ID or hmenu (from CreateWindow) */
    UINT               helpContext;   /* Help context ID */
    UINT               flags;         /* Misc. flags (see below) */
    DWORD_PTR          userdata;      /* User private data */
    int                cbWndExtra;    /* class cbWndExtra at window creation */
    DWORD              extra[1];
};

static std::map<HWND,_Window*> map_wnd;
static std::recursive_mutex mutex_wnd;

static _Window * get_win_ptr(HWND hWnd){
    std::unique_lock<std::recursive_mutex> lock(mutex_wnd);
    auto it = map_wnd.find(hWnd);
    if(it == map_wnd.end())
        return nullptr;
    return it->second;
}

static LONG_PTR get_win_data( const void *ptr, UINT size )
{
    if (size == sizeof(WORD))
    {
        WORD ret;
        memcpy( &ret, ptr, sizeof(ret) );
        return ret;
    }
    else if (size == sizeof(DWORD))
    {
        DWORD ret;
        memcpy( &ret, ptr, sizeof(ret) );
        return ret;
    }
    else
    {
        LONG_PTR ret;
        memcpy( &ret, ptr, sizeof(ret) );
        return ret;
    }
}

/* helper for set_window_long */
static inline void set_win_data( void *ptr, LONG_PTR val, UINT size )
{
    if (size == sizeof(WORD))
    {
        WORD newval = val;
        memcpy( ptr, &newval, sizeof(newval) );
    }
    else if (size == sizeof(DWORD))
    {
        DWORD newval = val;
        memcpy( ptr, &newval, sizeof(newval) );
    }
    else
    {
        memcpy( ptr, &val, sizeof(val) );
    }
}

extern bool NtUserGetAtomName( ATOM atomName, UNICODE_STRING *str );
/***********************************************************************
 *		CreateWindowExW (USER32.@)
 */
HWND WINAPI  CreateWindowEx( DWORD exStyle, LPCSTR className,
                                 LPCSTR windowName, DWORD style, INT x,
                                 INT y, INT width, INT height,
                                 HWND parent, HMENU menu,
                                 HINSTANCE instance, LPVOID data )
{
    char szClassName[100];
    if(IS_INTRESOURCE(className)){
        UNICODE_STRING str={100,szClassName,0};
        if(!NtUserGetAtomName((ATOM)LOWORD(className),&str))
            return 0;
        className = szClassName;
    }
    CREATESTRUCT cs;
    cs.lpCreateParams = data;
    cs.hInstance      = instance;
    cs.hMenu          = menu;
    cs.hwndParent     = parent;
    cs.x              = x;
    cs.y              = y;
    cs.cx             = width;
    cs.cy             = height;
    cs.style          = style;
    cs.lpszName       = windowName;
    cs.lpszClass      = className;
    cs.dwExStyle      = exStyle;

    return WIN_CreateWindowEx( &cs, className, instance);
}

/***********************************************************************
 *           WIN_CreateWindowEx
 *
 * Implementation of CreateWindowEx().
 */
HWND WIN_CreateWindowEx( CREATESTRUCT *cs, LPCSTR className, HINSTANCE module)
{
    WNDCLASSEX info={0};
    if (!GetClassInfoEx( module, className, &info )){
        if(strcmp(className,CLS_WINDOW)==0){
            //built in class
            info.cbSize = sizeof(info);
            info.cbClsExtra = 0;
            info.lpfnWndProc = DefWindowProc;
            info.lpszClassName = CLS_WINDOW;
            RegisterClassEx(&info);
        }else{
            return FALSE;
        }    
    }
    void * p = malloc(sizeof(_Window)+info.cbWndExtra-sizeof(DWORD));
    _Window *pWnd = new(p)_Window();
    
    pWnd->tid = pthread_self();
    SThreadUiState *state = SUiState::instance()->getThreadUiState2(pWnd->tid);
    pWnd->mConnection = state->connection;
    pWnd->mScreen = state->screen;

    HWND hWnd = xcb_generate_id(pWnd->mConnection);

    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t value_list[2] = {
        pWnd->mScreen->black_pixel,
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS |
            XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
            XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
            XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | 
            XCB_EVENT_MASK_RESIZE_REDIRECT|XCB_EVENT_MASK_FOCUS_CHANGE|
            XCB_EVENT_MASK_VISIBILITY_CHANGE|XCB_EVENT_MASK_STRUCTURE_NOTIFY};

    xcb_create_window(pWnd->mConnection, XCB_COPY_FROM_PARENT, hWnd,
                      pWnd->mScreen->root, cs->x, cs->y, cs->cx, cs->cy, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, pWnd->mScreen->root_visual, mask,
                      value_list);

    uint32_t m_gc = xcb_generate_id(pWnd->mConnection);
// 设置绘图上下文属性
    uint32_t value_mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    value_list[0] = pWnd->mScreen->black_pixel;
    value_list[1] = 0;

    xcb_create_gc(pWnd->mConnection, m_gc, hWnd, value_mask, value_list);
    pWnd->title = cs->lpszName;
    xcb_change_property(pWnd->mConnection, XCB_PROP_MODE_REPLACE, hWnd,
        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, pWnd->title.length(), pWnd->title.c_str());

    xcb_change_property(pWnd->mConnection, XCB_PROP_MODE_REPLACE, hWnd, state->wm_protocols_atom, XCB_ATOM_ATOM, 32, 1, &state->wm_delete_window_atom);

    if(cs->style & WS_VISIBLE)
        xcb_map_window(pWnd->mConnection, hWnd);

    const unsigned coords[] = {static_cast<unsigned>(cs->x),
                               static_cast<unsigned>(cs->y)};
    xcb_configure_window(pWnd->mConnection, hWnd,
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);

    //state->onWndCreate(m_hWnd,this);
    xcb_flush(pWnd->mConnection);

    pWnd->gc = m_gc;
    pWnd->parent = cs->hwndParent;
    pWnd->winproc = info.lpfnWndProc;
    {
        std::unique_lock<std::recursive_mutex> lock(mutex_wnd);
        map_wnd.insert(std::make_pair(hWnd,pWnd));
    }
    SendMessage(hWnd,WM_CREATE,0,(LPARAM)cs);
    return hWnd;
}

BOOL WINAPI DestroyWindow(HWND hWnd){
    if(!IsWindow(hWnd))
        return FALSE;
    SendMessage(hWnd,WM_DESTROY,0,0);
    SendMessage(hWnd,WM_NCDESTROY,0,0);
    std::unique_lock<std::recursive_mutex> lock(mutex_wnd);
    auto it = map_wnd.find(hWnd);
    if(it==map_wnd.end())
        return FALSE;
    _Window *wndObj = it->second;
    map_wnd.erase(it);

    //delete wndObj and release resource of the window object
    xcb_free_gc(wndObj->mConnection,wndObj->gc);
    xcb_destroy_window(wndObj->mConnection, hWnd); 
    xcb_flush(wndObj->mConnection);
    delete wndObj;

    return TRUE;
}

BOOL WINAPI IsWindow(HWND hWnd)
{
    std::unique_lock<std::recursive_mutex> lock(mutex_wnd);
    auto it = map_wnd.find(hWnd);
    return it != map_wnd.end();
}


BOOL ClientToScreen(HWND hWnd, LPPOINT ppt)
{
    return 0;
}
BOOL ScreenToClient(HWND hWnd, LPPOINT ppt)
{
    return 0;
}

BOOL GetClipBox(HDC hdc, RECT *pRc)
{
    return 0;
}

HMONITOR
MonitorFromWindow(HWND hwnd, DWORD dwFlags)
{
    return 0;
}

BOOL GetMonitorInfo(HMONITOR hMonitor, LPMONITORINFO lpmi)
{
    return FALSE;
}


BOOL PostMessage(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    _Window *pWnd = get_win_ptr(hWnd);
    if(!pWnd)
        return -1;
    std::unique_lock<std::recursive_mutex> lock(pWnd->mutex);
 
    auto trdUiState = SUiState::instance()->getThreadUiState2(pWnd->tid);
    xcb_client_message_event_t ev;  
    memset(&ev,0,sizeof(ev));
    ev.response_type = XCB_CLIENT_MESSAGE;  
    ev.format = 32; // 数据格式为32位  
    ev.window = hWnd; // 目标窗口  
    ev.type = trdUiState->wm_window;
    ev.data.data32[0] = msg;
    ev.data.data32[1] = wp&0xffffffff; 
    ev.data.data32[2] = (wp&0xffffffff00000000)>>32; 
    ev.data.data32[3] = lp&0xffffffff; 
    ev.data.data32[4] = (lp&0xffffffff00000000)>>32; 

    xcb_void_cookie_t cookie = xcb_send_event(trdUiState->connection, 0 /* 不广播 */, hWnd, XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);  
  
    // 检查发送是否成功（尽管这通常不是必需的，因为发送失败的情况很少）  
    xcb_generic_error_t *error = xcb_request_check(trdUiState->connection, cookie);  
    if (error) {  
        // 处理错误
        fprintf(stderr, "Error sending event: %d\n", error->error_code);  
        free(error); 
        return FALSE;
    }
    xcb_flush(trdUiState->connection);
    return TRUE;
}

LRESULT SendMessage(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    _Window *pWnd = get_win_ptr(hWnd);
    if(!pWnd)
        return -1;
    std::unique_lock<std::recursive_mutex> lock(pWnd->mutex);

    SOUI::SThreadUiState *trdUiState = SOUI::SUiState::instance()->getThreadUiState2(pWnd->tid);
    if(!trdUiState)
        return -1;
    SOUI::SThreadUiState *trdUiStateCur = SOUI::SUiState::instance()->getThreadUiState();
    if(trdUiState == trdUiStateCur){
        //same thread,call wndproc directly.
        WNDPROC wndProc = (WNDPROC)GetWindowLongPtr(hWnd,GWL_WNDPROC);
        assert(wndProc);
        return wndProc(hWnd,msg,wp,lp);
    }else{
        return PostMessage(hWnd,msg,wp,lp);
        //todo, hjx dont care about the result right now.
    }
}

int MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
    return 0;
}

BOOL SetForegroundWindow(HWND hWnd)
{
    return 0;
}

HWND GetForegroundWindow()
{
    return 0;
}

BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
    return 0;
}

static LONG_PTR SetWindowLongSize(HWND hWnd,int nIndex,LONG_PTR data,uint32_t size);
static LONG_PTR GetWindowLongSize(HWND hWnd,int nIndex,uint32_t size);

LONG SetWindowLong(HWND hWnd, int nIndex,LONG data){
    return SetWindowLongSize(hWnd,nIndex,data,sizeof(LONG));
}

LONG_PTR SetWindowLongPtr(HWND hWnd, int nIndex,LONG_PTR data){
    return SetWindowLongSize(hWnd,nIndex,data,sizeof(LONG_PTR));
}

static LONG_PTR SetWindowLongSize(HWND hWnd,int nIndex,LONG_PTR data,uint32_t size){
    _Window *wndObj = get_win_ptr(hWnd);
    if(!wndObj)
        return 0;
    LONG_PTR retval = GetWindowLongSize(hWnd,nIndex,size);
    std::unique_lock<std::recursive_mutex> lock(wndObj->mutex);
    switch(nIndex){
        case GWL_STYLE:
        wndObj->dwStyle = data;
        break;
        case GWL_EXSTYLE:
        wndObj->dwExStyle = data;
        break;
        case GWL_HINSTANCE:
        wndObj->hInstance=data;
        break;
        case GWL_HWNDPARENT:
        wndObj->parent=data;
        break;
        case GWL_ID:
        wndObj->wIDmenu=data;
        break;
        case GWL_USERDATA:
        wndObj->userdata=data;
        break;
        case GWL_WNDPROC:
        wndObj->winproc = (WNDPROC)data;
        break;
        case GWLP_OPAQUE:
        wndObj->objOpaque = data;
        break;
        default:
        if(nIndex>=0 && nIndex<wndObj->cbWndExtra-size){
            set_win_data( (char *)wndObj->extra + nIndex, data, size );
        }   
    }
    return retval;
}


LONG GetWindowLong(HWND hWnd, int nIndex)
{
    return (LONG)GetWindowLongSize(hWnd,nIndex,sizeof(LONG));
}

LONG_PTR GetWindowLongPtr(HWND hWnd,int nIndex){
    return GetWindowLongSize(hWnd,nIndex,sizeof(LONG_PTR));
}

static LONG_PTR GetWindowLongSize(HWND hWnd,int nIndex,uint32_t size){
    _Window *wndObj = get_win_ptr(hWnd);
    if(!wndObj)
        return 0;
    std::unique_lock<std::recursive_mutex> lock(wndObj->mutex);
    switch(nIndex){
        case GWL_STYLE:
        return wndObj->dwStyle;
        case GWL_EXSTYLE:
        return wndObj->dwExStyle;
        case GWL_HINSTANCE:
        return wndObj->hInstance;
        case GWL_HWNDPARENT:
        return wndObj->parent;
        case GWL_ID:
        return wndObj->wIDmenu;
        case GWL_USERDATA:
        return wndObj->userdata;
        case GWL_WNDPROC:
        return (LONG_PTR)wndObj->winproc;
        case GWLP_OPAQUE:
        return wndObj->objOpaque;
        default:
        if(nIndex>=0 && nIndex<wndObj->cbWndExtra-size){
            return get_win_data( (char *)wndObj->extra + nIndex, size );
        }   
    }
    return 0;
}



BOOL CreateCaret(HWND hWnd, HBITMAP hBitmap, int nWidth, int nHeight)
{
    return 0;
}

BOOL DestroyCaret(VOID)
{
    return FALSE;
}
BOOL HideCaret(HWND hWnd)
{
    return 0;
}

BOOL ShowCaret(HWND hWnd)
{
    return 0;
}

BOOL SetCaretPos(int X, int Y)
{
    return 0;
}

BOOL GetCaretPos(LPPOINT lpPoint)
{
    return 0;
}

HWND GetActiveWindow()
{
    return 0;
}

HWND GetDesktopWindow()
{
    return 0;
}

BOOL IsWindowEnabled(HWND hWnd)
{
    return 0;
}

void EnableWindow(HWND hWnd, BOOL bEnable)
{
}

HWND SetActiveWindow(HWND hWnd)
{
    return 0;
}

HWND GetParent(HWND hWnd)
{
    return (HWND)GetWindowLongPtr(hWnd,GWLP_HWNDPARENT);
}

HWND SetParent(HWND hWnd, HWND hParent){
    return (HWND)SetWindowLongPtr(hWnd,GWLP_HWNDPARENT,hParent);
}

BOOL GetCursorPos(LPPOINT ppt)
{
    return 0;
}

HWND WindowFromPoint(POINT pt)
{
    return 0;
}

UINT_PTR SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc)
{
    return 0;
}

BOOL KillTimer(HWND hWnd, UINT_PTR uIDEvent)
{
    return 0;
}

HWND GetFocus()
{
    return 0;
}

HWND SetFocus(HWND hWnd)
{
    return 0;
}

HDC BeginPaint(HWND hWnd, PAINTSTRUCT *ps){
    return 0;
}
void EndPaint(HWND hWnd, PAINTSTRUCT *ps){}

BOOL UpdateWindow(HWND hWnd)
{
    return 0;
}

BOOL GetClientRect(HWND hWnd, RECT *pRc)
{
    return 0;
}


BOOL GetWindowRect(HWND hWnd, RECT *rc)
{
    return false;
}

HRESULT DefWindowProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    switch(msg){
        case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    }
    return 0;
}

BOOL SetNativeWndAlpha(HWND hWnd, BYTE byAlpha){
    return FALSE;
}

BOOL ShowWindow(HWND hWnd, int nCmdShow)
{
    _Window *wndObj = get_win_ptr(hWnd);
    if(!wndObj)
        return FALSE;
    std::unique_lock<std::recursive_mutex> lock(wndObj->mutex);
    if(nCmdShow & SW_SHOW)
        xcb_map_window(wndObj->mConnection, hWnd);
    else
        xcb_unmap_window(wndObj->mConnection, hWnd);
    xcb_flush(wndObj->mConnection);
    return TRUE;
}

BOOL InvalidateRect(
  HWND       hWnd,
  const RECT *lpRect,
  BOOL       bErase
){
    _Window *wndObj = get_win_ptr(hWnd);
    if(!wndObj)
        return FALSE;
    std::unique_lock<std::recursive_mutex> lock(wndObj->mutex);
    xcb_connection_t *connection=wndObj->mConnection;
    RECT rcWnd;
    if(!lpRect){
        GetWindowRect(hWnd,&rcWnd);
        lpRect=&rcWnd;
    }
 // 发送曝光事件
    xcb_expose_event_t expose_event;
    expose_event.response_type = XCB_EXPOSE;
    expose_event.window = hWnd;
    expose_event.x = lpRect->left;
    expose_event.y = lpRect->top;
    expose_event.width = lpRect->right - lpRect->left;
    expose_event.height = lpRect->bottom - lpRect->top;
    xcb_send_event(connection, false, hWnd, XCB_EVENT_MASK_EXPOSURE, (const char *)&expose_event);
    xcb_flush(connection);
    return TRUE;
}

BOOL MoveWindow(HWND hWnd,int x, int y, int nWidth, int nHeight, BOOL bRepaint)
{
    _Window *wndObj = get_win_ptr(hWnd);
    if(!wndObj)
        return FALSE;
    std::unique_lock<std::recursive_mutex> lock(wndObj->mutex);
    xcb_connection_t *connection=wndObj->mConnection;
    {
            const unsigned coords[] = {static_cast<unsigned>(x),
                               static_cast<unsigned>(y)};
    xcb_configure_window(connection, hWnd,
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);

    }
    {
    const uint32_t vals[2] = {(uint32_t)nWidth, (uint32_t)nHeight};
    xcb_configure_window(connection, hWnd,
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                         vals);
    }

    //todo: repaint
    xcb_flush(connection);
    if(bRepaint){
        RECT rc={0,0,nWidth,nHeight};
        InvalidateRect(hWnd,&rc,TRUE);
    }
    return TRUE;
}

BOOL IsWindowVisible(HWND hWnd)
{
    _Window *wndObj = get_win_ptr(hWnd);
    if(!wndObj)
        return FALSE;
    std::unique_lock<std::recursive_mutex> lock(wndObj->mutex);
    xcb_connection_t *connection=wndObj->mConnection;

    xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(connection, hWnd);
    xcb_get_window_attributes_reply_t *reply = xcb_get_window_attributes_reply(connection, cookie, NULL);
    if (!reply)
        return FALSE;
    uint8_t mapState = reply->map_state;
    free(reply);
    return mapState == XCB_MAP_STATE_VIEWABLE;
}

static BOOL CheckWindowState(HWND hWnd, xcb_atom_t st){
    _Window *wndObj = get_win_ptr(hWnd);
    if(!wndObj)
        return FALSE;
    std::unique_lock<std::recursive_mutex> lock(wndObj->mutex);
    SThreadUiState *state = SUiState::instance()->getThreadUiState2(wndObj->tid);
    assert(state);
    xcb_connection_t *connection=wndObj->mConnection;

    xcb_get_property_cookie_t cookie = xcb_get_property(connection, 0, hWnd, XCB_ATOM_ATOM, state->wm_stat_atom, 0, 1024);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(connection, cookie, NULL);
    if (!reply) return FALSE;
    xcb_atom_t *atoms = (xcb_atom_t*)xcb_get_property_value(reply);
    int num_atoms = xcb_get_property_value_length(reply) / sizeof(xcb_atom_t);
    BOOL bRet = FALSE;
    for(int i=0;i<num_atoms;i++){
        if(atoms[i] == st)
        {
            bRet = TRUE;
            break;
        }
    }
    free(reply);
    return bRet;
} 

//todo:hjx 
enum WmState : uint32_t {
  WM_STATE_WITHDRAWN = 0,
  WM_STATE_NORMAL = 1,
  WM_STATE_ICONIC = 3,
};

BOOL IsZoomed(HWND hWnd){
    return CheckWindowState(hWnd,WM_STATE_ICONIC);
}

BOOL IsIconic(HWND hWnd){
    return CheckWindowState(hWnd,WM_STATE_ICONIC);
}

int GetWindowText(HWND hWnd, LPTSTR lpszStringBuf, int nMaxCount)
{
    _Window *wndObj = get_win_ptr(hWnd);
    if(!wndObj)
        return FALSE;
    std::unique_lock<std::recursive_mutex> lock(wndObj->mutex);
    int nRet = 0;
    if(nMaxCount>wndObj->title.length())
        strcpy(lpszStringBuf,wndObj->title.c_str()),nRet = wndObj->title.length();
    else
        strncpy(lpszStringBuf,wndObj->title.c_str(),nMaxCount),nRet=nMaxCount;
    return nRet;
}

int GetWindowTextLength(HWND hWnd){
    _Window *wndObj = get_win_ptr(hWnd);
    if(!wndObj)
        return 0;
    std::unique_lock<std::recursive_mutex> lock(wndObj->mutex);
    return wndObj->title.length();
}

BOOL SetWindowText(HWND hWnd , LPCTSTR lpszString){
    _Window *wndObj = get_win_ptr(hWnd);
    if(!wndObj)
        return FALSE;
    std::unique_lock<std::recursive_mutex> lock(wndObj->mutex);
    wndObj->title = lpszString;
    xcb_change_property(wndObj->mConnection, XCB_PROP_MODE_REPLACE, hWnd,
        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, wndObj->title.length(), wndObj->title.c_str());
    xcb_flush(wndObj->mConnection);
    return TRUE;
}

HDC GetDC(HWND hWnd){
    _Window *wndObj = get_win_ptr(hWnd);
    if(!wndObj)
        return 0;
    return wndObj->gc;
}

int ReleaseDC(HWND hWnd,HDC hdc){
    _Window *wndObj = get_win_ptr(hWnd);
    if(!wndObj)
        return 0;
    if(wndObj->gc!=hdc)
        return 0;
    //todo:hjx
    return 1;
}

int MapWindowPoints(HWND hWndFrom,HWND hWndTo, LPPOINT lpPoint, UINT nCount){
    //todo:hjx
    return 0;
}