#include <platform.h>
#include <wnd.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <cairo/cairo-xcb.h>
#include <xcb/xcb_aux.h>
#include <map>
#include <mutex>
#include <string>
#include "class.h"
#include "SConnection.h"
#include "sdc.h"

using namespace SOUI;

#define CLS_WINDOW "window"

HWND WIN_CreateWindowEx( CREATESTRUCT *cs, LPCSTR className, HINSTANCE module);

struct _Window{
    std::recursive_mutex mutex;
    pthread_t tid;
    xcb_connection_t *mConnection;
    xcb_screen_t *mScreen;
    HDC           hdc;
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

class WndObj{
    public:
    WndObj(const WndObj &src){
        wnd=src.wnd;
        if(wnd){
            wnd->mutex.lock();
        }
    }

    ~WndObj(){
        if(wnd){
            wnd->mutex.unlock();
        }
    }

    static WndObj fromHwnd(HWND hWnd){
        std::unique_lock<std::recursive_mutex> lock(mutex_wnd);
        _Window *wnd = get_win_ptr(hWnd);
        return WndObj(wnd);
    }

    _Window * operator ->(){
        return wnd;
    }

    bool operator !() const{
        return wnd==nullptr;
    }

    operator bool() const{
        return wnd!=nullptr;
    }
    private:

    WndObj(_Window *pWnd):wnd(pWnd){
        if(wnd){
            wnd->mutex.lock();
        }
    }


    _Window * wnd;
};
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
    pWnd->hdc = nullptr;

    SConnection *state = SConnMgr::instance()->getConnection(pWnd->tid);
    pWnd->mConnection = state->connection;
    pWnd->mScreen = state->screen;
    pWnd->dwStyle = cs->style;
    pWnd->dwExStyle = cs->dwExStyle;
    pWnd->hInstance = module;
    HWND hWnd = xcb_generate_id(pWnd->mConnection);

    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t value_list[2] = {
        pWnd->mScreen->black_pixel,
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |XCB_EVENT_MASK_PROPERTY_CHANGE
        };

    xcb_create_window(pWnd->mConnection, XCB_COPY_FROM_PARENT, hWnd,
                      pWnd->mScreen->root, cs->x, cs->y, cs->cx, cs->cy, 10,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, pWnd->mScreen->root_visual, mask,
                      value_list);

    pWnd->title = cs->lpszName;
    xcb_change_property(pWnd->mConnection, XCB_PROP_MODE_REPLACE, hWnd,
        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, pWnd->title.length(), pWnd->title.c_str());

    xcb_change_property(pWnd->mConnection, XCB_PROP_MODE_REPLACE, hWnd, state->wm_protocols_atom, XCB_ATOM_ATOM, 32, 1, &state->wm_delete_window_atom);

    if(cs->style & WS_VISIBLE)
        xcb_map_window(pWnd->mConnection, hWnd);

    xcb_flush(pWnd->mConnection);

    pWnd->parent = cs->hwndParent;
    pWnd->winproc = info.lpfnWndProc;
    {
        std::unique_lock<std::recursive_mutex> lock(mutex_wnd);
        map_wnd.insert(std::make_pair(hWnd,pWnd));
    }
    pWnd->hdc = CreateDC(hWnd, cs->cx, cs->cy);
    if(0!=SendMessage(hWnd,WM_CREATE,0,(LPARAM)cs)){
        std::unique_lock<std::recursive_mutex> lock(mutex_wnd);
        auto it = map_wnd.find(hWnd);
        assert(it!=map_wnd.end());

        _Window *wndObj = it->second;
        map_wnd.erase(it);
        if(wndObj->hdc){
            DeleteDC(wndObj->hdc);
        }
        //delete wndObj and release resource of the window object
        xcb_unmap_window(wndObj->mConnection,hWnd);
        xcb_destroy_window(wndObj->mConnection, hWnd); 
        xcb_flush(wndObj->mConnection);
        free(wndObj);
        hWnd = 0;
    }
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
    if(wndObj->hdc)
    {
        DeleteDC(wndObj->hdc);
        wndObj->hdc = nullptr;
    }

    map_wnd.erase(it);

    //delete wndObj and release resource of the window object
    xcb_unmap_window(wndObj->mConnection,hWnd);
    xcb_destroy_window(wndObj->mConnection, hWnd); 
    xcb_flush(wndObj->mConnection);
    free(wndObj);

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
    WndObj pWnd = WndObj::fromHwnd(hWnd);
    if(!pWnd)
        return FALSE;
    auto conn = SConnMgr::instance()->getConnection(pWnd->tid);
    conn->postMsg(hWnd,msg,wp,lp);
    return TRUE;
}

LRESULT CallWndProc(WNDPROC proc, HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if (!wndObj)
        return -1;
    switch (msg) {
    case WM_SIZE:
        SIZE sz = { GET_X_LPARAM(lp),GET_Y_LPARAM(lp) };
        if (wndObj->hdc)
        {
            cairo_xcb_surface_set_size((cairo_surface_t*)GetGdiObjPtr(GetCurrentObject(wndObj->hdc, OBJ_BITMAP)), sz.cx, sz.cy);
        }
        break;
    }
    return proc(hWnd, msg, wp, lp);
}

LRESULT SendMessage(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    WndObj pWnd = WndObj::fromHwnd(hWnd);
    if(!pWnd)
        return -1;
    SOUI::SConnection *trdUiState = SOUI::SConnMgr::instance()->getConnection(pWnd->tid);
    if(!trdUiState)
        return -1;
    SOUI::SConnection *trdUiStateCur = SOUI::SConnMgr::instance()->getConnection();
    if(trdUiState == trdUiStateCur){
        //same thread,call wndproc directly.
        WNDPROC wndProc = (WNDPROC)GetWindowLongPtr(hWnd,GWL_WNDPROC);
        assert(wndProc);
        return CallWndProc(wndProc,hWnd,msg,wp,lp);
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
    WndObj wndOj = WndObj::fromHwnd(hWnd);
    if(!wndOj)
        return FALSE;
    RECT rcWnd;
    GetWindowRect(hWnd,&rcWnd);
    if(!(uFlags & SWP_NOSIZE)){
        rcWnd.right = rcWnd.left+cx;
        rcWnd.bottom = rcWnd.top+cy;
    }
    if(!(uFlags & SWP_NOMOVE)){
        OffsetRect(&rcWnd,X-rcWnd.left,Y-rcWnd.top);
    }
    return MoveWindow(hWnd,rcWnd.left,rcWnd.top,rcWnd.right-rcWnd.left,rcWnd.bottom-rcWnd.top,!(uFlags&SWP_NOREDRAW));
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
    WndObj wndObj = WndObj::fromHwnd(hWnd);
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
        wndObj->hInstance=(HINSTANCE)data;
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
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return 0;
    switch(nIndex){
        case GWL_STYLE:
        return wndObj->dwStyle;
        case GWL_EXSTYLE:
        return wndObj->dwExStyle;
        case GWL_HINSTANCE:
        return (LONG_PTR)wndObj->hInstance;
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

BOOL EnableWindow(HWND hWnd, BOOL bEnable)
{
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj) 
        return FALSE;
    SConnection *conn = SConnMgr::instance()->getConnection(wndObj->tid);
    xcb_client_message_event_t event = {
        .response_type = XCB_CLIENT_MESSAGE,
        .format = 32,
        .sequence = 0,
        .window = hWnd,
        .type = conn->wm_stat_atom
    };
    event.data.data32[0] = bEnable?0:1;
    event.data.data32[1] = conn->wm_stat_enable_atom;
    xcb_send_event(
        conn->connection,
        0,
        hWnd,
        XCB_EVENT_MASK_STRUCTURE_NOTIFY,
        (const char *)&event
    );
    xcb_flush(conn->connection);
    return TRUE;
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
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return 0;
    wndObj->mutex.lock();
    return ps->hdc = GetDC(hWnd);
}

BOOL EndPaint(HWND hWnd, const PAINTSTRUCT *ps){
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
    wndObj->mutex.unlock();
    return ReleaseDC(hWnd,ps->hdc);
}

BOOL UpdateWindow(HWND hWnd)
{
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
    //todo:hjx
    xcb_flush(wndObj->mConnection);
    return TRUE;
}

BOOL GetClientRect(HWND hWnd, RECT *pRc)
{
    if(!GetWindowRect(hWnd,pRc))
        return FALSE;
    OffsetRect(pRc,-pRc->left,-pRc->top);
    return TRUE;
}


BOOL GetWindowRect(HWND hWnd, RECT *rc)
{
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
    
    xcb_get_geometry_cookie_t geometry_cookie = xcb_get_geometry(wndObj->mConnection, hWnd);
    xcb_get_geometry_reply_t *geometry_reply = xcb_get_geometry_reply(wndObj->mConnection, geometry_cookie, NULL);

    if (!geometry_reply)
        return FALSE;
    rc->left = geometry_reply->x;
    rc->top = geometry_reply->y;
    rc->right = rc->left + geometry_reply->width;
    rc->bottom = rc->top + geometry_reply->height;
    free(geometry_reply);
    return TRUE;
}

HDC CreateDC(HWND hwnd,int cx,int cy){
    assert(hwnd);
    WndObj wndObj = WndObj::fromHwnd(hwnd);
    if(wndObj){
        cairo_surface_t * surface = cairo_xcb_surface_create(wndObj->mConnection, hwnd, xcb_aux_find_visual_by_id(wndObj->mScreen,wndObj->mScreen->root_visual), cx,cy);
        HDC hdc = new _SDC(hwnd,InitGdiObj(OBJ_BITMAP,surface));
        return hdc;
    }else{
        SConnection *conn = SConnMgr::instance()->getConnection();
        hwnd = conn->screen->root;
        cairo_surface_t * surface = cairo_xcb_surface_create(conn->connection, hwnd, xcb_aux_find_visual_by_id(conn->screen,conn->screen->root_visual), cx,cy);
        HDC hdc = new _SDC(hwnd,InitGdiObj(OBJ_BITMAP,surface));
        return hdc;
    }

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
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
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
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
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
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
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
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
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
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
    SConnection *state = SConnMgr::instance()->getConnection(wndObj->tid);
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
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
    int nRet = 0;
    if(nMaxCount>wndObj->title.length())
        strcpy(lpszStringBuf,wndObj->title.c_str()),nRet = wndObj->title.length();
    else
        strncpy(lpszStringBuf,wndObj->title.c_str(),nMaxCount),nRet=nMaxCount;
    return nRet;
}

int GetWindowTextLength(HWND hWnd){
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return 0;
    return wndObj->title.length();
}

BOOL SetWindowText(HWND hWnd , LPCTSTR lpszString){
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
    wndObj->title = lpszString;
    xcb_change_property(wndObj->mConnection, XCB_PROP_MODE_REPLACE, hWnd,
        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, wndObj->title.length(), wndObj->title.c_str());
    xcb_flush(wndObj->mConnection);
    return TRUE;
}

HDC GetDC(HWND hWnd){
    if(!hWnd){
        return CreateCompatibleDC(nullptr);
    }
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj || !wndObj->hdc)
        return 0;
    return wndObj->hdc;
}

int ReleaseDC(HWND hWnd,HDC hdc){
    cairo_t* cairo_dc = (cairo_t*)hdc;
    if(!hWnd){
        DeleteDC(hdc);
        return 1;
    }
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return 0;
    xcb_flush(wndObj->mConnection);
    return 1;
}

int MapWindowPoints(HWND hWndFrom,HWND hWndTo, LPPOINT lpPoint, UINT nCount){
    RECT rcFrom,rcTo;
    if(!GetWindowRect(hWndFrom,&rcFrom))
        return 0;
    if(!GetWindowRect(hWndTo,&rcTo))
        return 0;
    int xDiff = rcTo.left-rcFrom.left;
    int yDiff = rcTo.top-rcFrom.top;

    for(UINT i=0;i<nCount;i++){
        lpPoint[i].x -= xDiff;
        lpPoint[i].y -= yDiff;
    }
    return MAKELONG(-xDiff,-yDiff);
}
