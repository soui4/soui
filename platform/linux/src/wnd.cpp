#include <wnd.h>
#include <xcb/xcb.h>
#include <map>
#include <mutex>
#include <string>
#include "UiState.h"
using namespace SOUI;

#define CLS_WINDOW "window"

HWND WIN_CreateWindowEx( CREATESTRUCT *cs, LPCSTR className, HINSTANCE module);

struct _Window{
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

/***********************************************************************
 *		CreateWindowExW (USER32.@)
 */
HWND WINAPI  CreateWindowEx( DWORD exStyle, LPCSTR className,
                                 LPCSTR windowName, DWORD style, INT x,
                                 INT y, INT width, INT height,
                                 HWND parent, HMENU menu,
                                 HINSTANCE instance, LPVOID data )
{
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
    std::unique_lock<std::recursive_mutex> lock(mutex_wnd);
    auto it = map_wnd.find(hWnd);
    if(it==map_wnd.end())
        return FALSE;
    delete it->second;
    map_wnd.erase(it);
    return TRUE;
}

BOOL WINAPI IsWindow(HWND hWnd)
{
    std::unique_lock<std::recursive_mutex> lock(mutex_wnd);
    auto it = map_wnd.find(hWnd);
    return it != map_wnd.end();
}

HDC WINAPI GetDC(HWND hWnd)
{
    return 0;
}
void WINAPI ReleaseDC(HWND hwnd, HDC hdc)
{
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
    std::unique_lock<std::recursive_mutex> lock(mutex_wnd);
    auto it = map_wnd.find(hWnd);
    if(it == map_wnd.end())
        return FALSE;
    _Window *pWnd = it->second;
 
    auto trdUiState = SUiState::instance()->getThreadUiState2(pWnd->tid);
    xcb_client_message_event_t ev;  
    ev.response_type = XCB_CLIENT_MESSAGE;  
    ev.format = 32; // 数据格式为32位  
    ev.window = hWnd; // 目标窗口  
    ev.type = trdUiState->wm_window;
    ev.data.data32[0] = msg;
    ev.data.data32[1] = wp&0xffffffff; 
    ev.data.data32[2] = (wp&0xffffffff00000000)>>32; 
    ev.data.data32[3] = lp&0xffffffff; 
    ev.data.data32[4] = (lp&0xffffffff00000000)>>32; 

    xcb_void_cookie_t cookie = xcb_send_event(trdUiState->connection, 0 /* 不广播 */, 0, XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);  
  
    // 检查发送是否成功（尽管这通常不是必需的，因为发送失败的情况很少）  
    xcb_generic_error_t *error = xcb_request_check(trdUiState->connection, cookie);  
    if (error) {  
        // 处理错误
        fprintf(stderr, "Error sending event: %d\n", error->error_code);  
        free(error); 
        return FALSE;
    }
    return TRUE;
}

LRESULT SendMessage(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    std::unique_lock<std::recursive_mutex> lock(mutex_wnd);
    auto it = map_wnd.find(hWnd);
    if(it == map_wnd.end())
        return -1;
    _Window *pWnd = it->second;

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

BOOL ShowWindow(HWND hWnd, int nCmdShow)
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

HRESULT DefWindowProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    return 0;
}