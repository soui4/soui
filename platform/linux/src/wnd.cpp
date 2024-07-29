#include <platform.h>
#include <wnd.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <cairo/cairo-xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_icccm.h>
#include <map>
#include <mutex>
#include <string>
#include "class.h"
#include "SConnection.h"
#include "sdc.h"
#include "uimsg.h"

using namespace SOUI;

#define CLS_WINDOW "window"

enum WndState {
    Normal = 0,
    Minimized = 1,
    Maximized = 2,
};

struct _Window{
    std::recursive_mutex mutex;
    pthread_t tid;
    SConnection *mConnection;
    HDC           hdc;
    HBITMAP       bmp;
    std::string title; 
    RECT          rc;
    WndState      state;
    ATOM          clsAtom;
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


static BOOL _GetCursorPos(SConnection* conn,LPPOINT ppt) {
    // 获取当前鼠标位置的请求
    xcb_query_pointer_cookie_t pointer_cookie = xcb_query_pointer(conn->connection, conn->screen->root);
    xcb_query_pointer_reply_t* pointer_reply = xcb_query_pointer_reply(conn->connection, pointer_cookie, NULL);
    if (!pointer_reply) {
        fprintf(stderr,"Failed to get mouse position\n");
        return FALSE;
    }
    ppt->x = pointer_reply->root_x;
    ppt->y = pointer_reply->root_y;
    // 释放资源
    free(pointer_reply);
    return TRUE;
}

static BOOL InitWndDC(HWND hwnd, int cx, int cy) {
    assert(hwnd);
    WndObj wndObj = WndObj::fromHwnd(hwnd);
    assert(wndObj);
    cx = std::max(cx, 1);
    cy = std::max(cy, 1);
    cairo_surface_t* surface = cairo_xcb_surface_create(wndObj->mConnection->connection, hwnd, xcb_aux_find_visual_by_id(wndObj->mConnection->screen, wndObj->mConnection->screen->root_visual), cx, cy);
    wndObj->bmp = InitGdiObj(OBJ_BITMAP, surface);
    wndObj->hdc = new _SDC(hwnd);
    SelectObject(wndObj->hdc, wndObj->bmp);
    return TRUE;
}

/***********************************************************************
 *           WIN_CreateWindowEx
 *
 * Implementation of CreateWindowEx().
 */
static HWND WIN_CreateWindowEx( CREATESTRUCT *cs, LPCSTR className, HINSTANCE module)
{
    WNDCLASSEX info={0};
    ATOM clsAtom = GetClassInfoEx( module, className, &info );
    if (!clsAtom){
        if(strcmp(className,CLS_WINDOW)==0){
            //built in class
            info.cbSize = sizeof(info);
            info.cbClsExtra = 0;
            info.lpfnWndProc = DefWindowProc;
            info.lpszClassName = CLS_WINDOW;
            clsAtom = RegisterClassEx(&info);
        }else{
            return FALSE;
        }    
    }
    void * p = malloc(sizeof(_Window)+info.cbWndExtra-sizeof(DWORD));
    _Window *pWnd = new(p)_Window();
    
    pWnd->tid = pthread_self();
    pWnd->hdc = nullptr;

    SConnection *conn = SConnMgr::instance()->getConnection(pWnd->tid);
    HWND hParent = cs->hwndParent;
    if (!hParent) hParent = conn->screen->root;

    pWnd->mConnection = conn;
    pWnd->state = Normal;
    pWnd->dwStyle = cs->style;
    pWnd->dwExStyle = cs->dwExStyle;
    pWnd->hInstance = module;
    pWnd->clsAtom = clsAtom;

    HWND hWnd = xcb_generate_id(pWnd->mConnection->connection);
    static const uint32_t evt_mask = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY
        | XCB_EVENT_MASK_PROPERTY_CHANGE
        | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION
        | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW
        | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE;
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t value_list[3] = { conn->screen->white_pixel };
    if (!(cs->style & (WS_THICKFRAME|WS_BORDER))) {
        mask |= XCB_CW_OVERRIDE_REDIRECT;
        value_list[1] = 1;
        value_list[2] = evt_mask;
    }
    else {
        value_list[1] = evt_mask;
    }
    xcb_void_cookie_t cookie = xcb_create_window_checked(pWnd->mConnection->connection, XCB_COPY_FROM_PARENT, hWnd,
                      hParent, cs->x, cs->y, std::max(cs->cx,1u), std::max(cs->cy,1u), 10,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, pWnd->mConnection->screen->root_visual, mask,
                      value_list);
    xcb_generic_error_t * err = xcb_request_check(pWnd->mConnection->connection, cookie);
    if (err) {
        printf("xcb_create_window failed, errcode=%d\n", err->error_code);
        free(err);
        return 0;
    }
    pWnd->rc.left = cs->x;
    pWnd->rc.top = cs->y;
    pWnd->rc.right = cs->x + cs->cx;
    pWnd->rc.bottom = cs->y + cs->cy;

    const uint32_t vals[2] = { cs->cx, cs->cy};
    xcb_configure_window(pWnd->mConnection->connection, hWnd, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, vals);
    pWnd->title = cs->lpszName;
    xcb_change_property(pWnd->mConnection->connection, XCB_PROP_MODE_REPLACE, hWnd,
        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, pWnd->title.length(), pWnd->title.c_str());

    xcb_change_property(pWnd->mConnection->connection, XCB_PROP_MODE_REPLACE, hWnd, conn->WM_PROTOCOLS_ATOM, XCB_ATOM_ATOM, 32, 1, &conn->WM_DELETE_WINDOW_ATOM);

    if(cs->style & WS_VISIBLE)
        xcb_map_window(pWnd->mConnection->connection, hWnd);

    xcb_flush(pWnd->mConnection->connection);

    pWnd->parent = hParent;
    pWnd->winproc = info.lpfnWndProc;
    {
        std::unique_lock<std::recursive_mutex> lock(mutex_wnd);
        map_wnd.insert(std::make_pair(hWnd,pWnd));
    }
    InitWndDC(hWnd, cs->cx, cs->cy);
    if(0!=SendMessage(hWnd,WM_CREATE,0,(LPARAM)cs)){
        std::unique_lock<std::recursive_mutex> lock(mutex_wnd);
        auto it = map_wnd.find(hWnd);
        assert(it!=map_wnd.end());

        _Window *wndObj = it->second;
        map_wnd.erase(it);
        if(wndObj->hdc){
            DeleteDC(wndObj->hdc);
        }
        if (wndObj->bmp) {
            DeleteObject(wndObj->bmp);
        }
        //delete wndObj and release resource of the window object
        xcb_unmap_window(wndObj->mConnection->connection,hWnd);
        xcb_destroy_window(wndObj->mConnection->connection, hWnd);
        xcb_flush(wndObj->mConnection->connection);
        free(wndObj);
        hWnd = 0;
    }
    return hWnd;
}

/***********************************************************************
 *		CreateWindowExW (USER32.@)
 */
HWND WINAPI  CreateWindowEx(DWORD exStyle, LPCSTR className,
    LPCSTR windowName, DWORD style, INT x,
    INT y, INT width, INT height,
    HWND parent, HMENU menu,
    HINSTANCE instance, LPVOID data)
{
    char szClassName[100];
    if (IS_INTRESOURCE(className)) {
        if (!GetAtomName((ATOM)LOWORD(className), szClassName,100))
            return 0;
        className = szClassName;
    }
    CREATESTRUCT cs;
    cs.lpCreateParams = data;
    cs.hInstance = instance;
    cs.hMenu = menu;
    cs.hwndParent = parent;
    cs.x = x;
    cs.y = y;
    cs.cx = width;
    cs.cy = height;
    cs.style = style;
    cs.lpszName = windowName;
    cs.lpszClass = className;
    cs.dwExStyle = exStyle;

    return WIN_CreateWindowEx(&cs, className, instance);
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
    if (wndObj->bmp) {
        DeleteObject(wndObj->bmp);
        wndObj->bmp = nullptr;
    }
    map_wnd.erase(it);

    //delete wndObj and release resource of the window object
    xcb_unmap_window(wndObj->mConnection->connection,hWnd);
    xcb_destroy_window(wndObj->mConnection->connection, hWnd);
    xcb_flush(wndObj->mConnection->connection);
    free(wndObj);

    return TRUE;
}

BOOL WINAPI IsWindow(HWND hWnd)
{
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    return !!wndObj;
}


BOOL ClientToScreen(HWND hWnd, LPPOINT ppt)
{
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
    ppt->x += wndObj->rc.left;
    ppt->y += wndObj->rc.top;
    return TRUE;
}
BOOL ScreenToClient(HWND hWnd, LPPOINT ppt)
{
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
    ppt->x -= wndObj->rc.left;
    ppt->y -= wndObj->rc.top;
    return TRUE;
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

HWND SetCapture(
   HWND hWnd
){
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return 0;
    HWND oldCapture = wndObj->mConnection->SetCapture(hWnd);
    //todo:hjx
    SendMessage(hWnd,WM_CAPTURECHANGED,0,hWnd);
    return oldCapture;
}

BOOL ReleaseCapture(){
    SConnection *conn = SConnMgr::instance()->getConnection();
    return conn->ReleaseCapture();
}

HWND GetCapture(){
    SConnection *conn = SConnMgr::instance()->getConnection();
    return conn->GetCapture();
}

static HRESULT HandleNcTestCode(HWND hWnd, UINT htCode) {
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if (!wndObj)
        return -1;

    POINT ptClick;
    if (!_GetCursorPos(wndObj->mConnection, &ptClick))
        return -1;
    if(!(htCode>=HTCAPTION && htCode <=HTBOTTOMRIGHT))
        return -2;
     
    wndObj->mConnection->SetTimerBlock(true);
    RECT rcWnd = wndObj->rc;
    BOOL bQuit = FALSE;
    SetCapture(hWnd);
    for (; !bQuit;) {
        MSG msg;
        if (!WaitMessage())
            break;
        while (PeekMessage(&msg, hWnd, 0, 0, TRUE)) {
            if (msg.message == WM_QUIT) {
                bQuit = TRUE;
                wndObj->mConnection->postMsg(msg.hwnd, msg.message, msg.wParam, msg.lParam);
                break;
            }
            else if (msg.message == WM_LBUTTONUP) {
                bQuit = TRUE;
                break;
            }
            else if (msg.message == WM_MOUSEMOVE) {
                POINT ptNow;
                _GetCursorPos(wndObj->mConnection, &ptNow);
                switch (htCode) {
                case HTCAPTION:
                {//move window
                    int32_t x = rcWnd.left + ptNow.x - ptClick.x;
                    int32_t y = rcWnd.top + ptNow.y - ptClick.y;
                    SetWindowPos(hWnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                }
                    break;
                case HTTOP:
                {//
                    RECT rcNow = rcWnd;
                    rcNow.top += ptNow.y - ptClick.y;
                    SetWindowPos(hWnd, 0, rcNow.left, rcNow.top, rcNow.right-rcNow.left, rcNow.bottom-rcNow.top, SWP_NOZORDER);
                }
                break;
                case HTBOTTOM:
                {//
                    RECT rcNow = rcWnd;
                    rcNow.bottom += ptNow.y - ptClick.y;
                    SetWindowPos(hWnd, 0, rcNow.left, rcNow.top, rcNow.right - rcNow.left, rcNow.bottom - rcNow.top, SWP_NOZORDER|SWP_NOMOVE);
                }
                break;
                case HTLEFT:
                {
                    RECT rcNow = rcWnd;
                    rcNow.left += ptNow.x - ptClick.x;
                    SetWindowPos(hWnd, 0, rcNow.left, rcNow.top, rcNow.right - rcNow.left, rcNow.bottom - rcNow.top, SWP_NOZORDER);
                }
                break;
                case HTRIGHT:
                {
                    RECT rcNow = rcWnd;
                    rcNow.right += ptNow.x - ptClick.x;
                    SetWindowPos(hWnd, 0, rcNow.left, rcNow.top, rcNow.right - rcNow.left, rcNow.bottom - rcNow.top, SWP_NOZORDER|SWP_NOMOVE);
                }
                break;
                case HTTOPLEFT:
                {
                    RECT rcNow = rcWnd;
                    rcNow.left += ptNow.x - ptClick.x;
                    rcNow.top += ptNow.y - ptClick.y;
                    SetWindowPos(hWnd, 0, rcNow.left, rcNow.top, rcNow.right - rcNow.left, rcNow.bottom - rcNow.top, SWP_NOZORDER);
                }
                break;
                case HTTOPRIGHT: 
                {
                    RECT rcNow = rcWnd;
                    rcNow.right += ptNow.x - ptClick.x;
                    rcNow.top += ptNow.y - ptClick.y;
                    SetWindowPos(hWnd, 0, rcNow.left, rcNow.top, rcNow.right - rcNow.left, rcNow.bottom - rcNow.top, SWP_NOZORDER);
                }
                break;
                case HTBOTTOMLEFT:
                {
                    RECT rcNow = rcWnd;
                    rcNow.left += ptNow.x - ptClick.x;
                    rcNow.bottom += ptNow.y - ptClick.y;
                    SetWindowPos(hWnd, 0, rcNow.left, rcNow.top, rcNow.right - rcNow.left, rcNow.bottom - rcNow.top, SWP_NOZORDER);
                }
                break;
                case HTSIZE:
                case HTBOTTOMRIGHT:
                {
                    RECT rcNow = rcWnd;
                    rcNow.right += ptNow.x - ptClick.x;
                    rcNow.bottom += ptNow.y - ptClick.y;
                    SetWindowPos(hWnd, 0, rcNow.left, rcNow.top, rcNow.right - rcNow.left, rcNow.bottom - rcNow.top, SWP_NOZORDER|SWP_NOMOVE);
                }
                break;
                }
            }
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
    ReleaseCapture();
    wndObj->mConnection->SetTimerBlock(false);
    return 0;
}

LRESULT CallWindowProc(WNDPROC proc, HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if (!wndObj)
        return -1;
    switch (msg) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MOUSEMOVE:
        {
            POINT pt;
            _GetCursorPos(wndObj->mConnection,&pt);
            int htCode = proc(hWnd,WM_NCHITTEST,0,MAKELPARAM(pt.x,pt.y));
            if(htCode>HTCAPTION && htCode <HTBORDER){
                if(msg == WM_MOUSEMOVE){
                    //todo: set cursor according to htCode
                    LPCSTR cursorId = nullptr;
                    switch(htCode){
                        case HTLEFT:
                        case HTRIGHT:
                        cursorId = IDC_SIZENS;
                        break;
                        case HTTOP:
                        case HTBOTTOM:
                        cursorId = IDC_SIZEWE;
                        break;
                        case HTTOPLEFT:
                        case HTBOTTOMRIGHT:
                        cursorId = IDC_SIZENWSE;
                        break;
                        case HTTOPRIGHT:
                        case HTBOTTOMLEFT:
                        cursorId = IDC_SIZENESW;
                        break;
                        case HTSIZE:
                        cursorId = IDC_SIZENESW;
                        break;              
                    }
                    SetCursor(LoadCursor(wndObj->hInstance,cursorId));
                }else if(msg == WM_LBUTTONDOWN){
                    HandleNcTestCode(hWnd,htCode);
                }
            }else{
                WNDCLASSEX wc;
                GetClassInfoEx(wndObj->hInstance,MAKEINTRESOURCE(wndObj->clsAtom),&wc);
                SetCursor(wc.hCursor);
            }
        }
        break;
    case WM_MOVE:
        OffsetRect(&wndObj->rc,GET_X_LPARAM(lp)-wndObj->rc.left,GET_Y_LPARAM(lp)-wndObj->rc.top);
        break;
    
    case WM_PAINT:
        if (lp) {
                HGDIOBJ hrgn = (HGDIOBJ)lp;
                if(GetObjectType(hrgn) == OBJ_REGION){
                    HDC hdc = GetDC(hWnd);
                    SelectClipRgn(hdc, hrgn);
                    ReleaseDC(hWnd,hdc);
                }
            }
        proc(hWnd,WM_ERASEBKGND,0,(LPARAM)wndObj->hdc);
        break;
    case UM_STATE:
        switch (wp) {
        case SIZE_MINIMIZED:
            wndObj->state = Minimized;
            break;
        case SIZE_MAXIMIZED:
            wndObj->state = Maximized;
            break;
        case SIZE_RESTORED:
            wndObj->state = Normal;
            lp = MAKELPARAM(wndObj->rc.right-wndObj->rc.left,wndObj->rc.bottom-wndObj->rc.top);
            CallWindowProc(proc,hWnd,WM_SIZE,0,lp);//call size again
            break;
        }
        //printf("wm_state, wp=%u\n",wp);
        return 1;
    case WM_SIZE:
        wp = wndObj->state;
		SIZE sz = { GET_X_LPARAM(lp),GET_Y_LPARAM(lp) };
		if (wndObj->bmp && (sz.cx != wndObj->rc.right - wndObj->rc.left || sz.cy != wndObj->rc.bottom - wndObj->rc.top))
		{
			wndObj->rc.right = wndObj->rc.left + sz.cx;
			wndObj->rc.bottom = wndObj->rc.top + sz.cy;
			cairo_xcb_surface_set_size((cairo_surface_t*)GetGdiObjPtr(wndObj->bmp), sz.cx, sz.cy);
		}
        //printf("wm_size, wp=%u, size=(%u,%u)\n", wp,sz.cx,sz.cy);
		break;
    }
    return proc(hWnd, msg, wp, lp);
}

LRESULT SendMessage(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    WndObj pWnd = WndObj::fromHwnd(hWnd);
    if(!pWnd)
        return -1;
    SConnection *connWnd = SConnMgr::instance()->getConnection(pWnd->tid);
    if(!connWnd)
        return -1;
    SConnection *connCur = SConnMgr::instance()->getConnection();
    if(connWnd == connCur){
        //same thread,call wndproc directly.
        WNDPROC wndProc = (WNDPROC)GetWindowLongPtr(hWnd,GWL_WNDPROC);
        assert(wndProc);
        return CallWindowProc(wndProc,hWnd,msg,wp,lp);
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

BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags)
{
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
    WINDOWPOS wndPos;
    wndPos.hwnd = hWnd;
    wndPos.hwndInsertAfter = hWndInsertAfter;
    wndPos.x=x;
    wndPos.y=y;
    wndPos.cx=cx;
    wndPos.cy = cy;
    wndPos.flags = uFlags;
    SendMessage(hWnd,WM_WINDOWPOSCHANGING,0,(LPARAM)&wndPos);
    if(wndPos.cx < 1)
        wndPos.cx = 1;
    if(wndPos.cy < 1)
        wndPos.cy=1;
    
    SendMessage(hWnd,WM_WINDOWPOSCHANGED,0,(LPARAM)&wndPos);
    return TRUE;
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
        .window = (xcb_window_t)hWnd,
        .type = conn->_NET_WM_STATE_ATOM
    };
    event.data.data32[0] = bEnable?0:1;
    event.data.data32[1] = conn->_NET_WM_STATE_DEMANDS_ATTENTION_ATOM;
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
    SConnection* conn = SConnMgr::instance()->getConnection();
    return _GetCursorPos(conn, ppt);
}

HWND WindowFromPoint(POINT pt)
{
    return 0;
}

UINT_PTR SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc)
{
    if (hWnd) {
        WndObj wndObj = WndObj::fromHwnd(hWnd);
        if (!wndObj)
            return 0;
        return wndObj->mConnection->SetTimer(hWnd, nIDEvent, uElapse, lpTimerFunc);
    }
    else {
        if (!lpTimerFunc)
            return 0;
        SConnection* conn = SConnMgr::instance()->getConnection();
        return conn->SetTimer(hWnd, nIDEvent, uElapse, lpTimerFunc);
    }
}

BOOL KillTimer(HWND hWnd, UINT_PTR uIDEvent)
{
    if (hWnd) {
        WndObj wndObj = WndObj::fromHwnd(hWnd);
        if (!wndObj)
            return FALSE;
        return wndObj->mConnection->KillTimer(hWnd, uIDEvent);
    }
    else {
        SConnection* conn = SConnMgr::instance()->getConnection();
        return conn->KillTimer(hWnd, uIDEvent);
    }
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
    xcb_flush(wndObj->mConnection->connection);
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
    *rc = wndObj->rc;
    return TRUE;
}

static void ChangeNetWmState(SConnection *conn, xcb_window_t wnd, bool bSet,xcb_atom_t one,xcb_atom_t two) {
    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.window = wnd;
    event.format = 32;
    event.sequence=0;
    event.type = conn->_NET_WM_STATE_ATOM;
    event.data.data32[0] = bSet?1:0;
    event.data.data32[1] = one;
    event.data.data32[2] = two;
    event.data.data32[3]=event.data.data32[4]=0;
    xcb_send_event(conn->connection, false, conn->screen->root, XCB_EVENT_MASK_STRUCTURE_NOTIFY|XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char*)&event);
    xcb_flush(conn->connection);
}

static void SendSysCommand(SConnection *conn, xcb_window_t wnd, uint32_t cmd) {
    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.window = wnd;
    event.format = 32;
    event.sequence=0;
    event.type = conn->WM_CHANGE_STATE_ATOM;
    event.data.data32[0] = cmd;
    xcb_send_event(conn->connection, false, conn->screen->root, XCB_EVENT_MASK_STRUCTURE_NOTIFY|XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char*)&event);
    xcb_flush(conn->connection);
}


static void SendSysRestore(SConnection *conn, xcb_window_t wnd) {

    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.window = wnd;
    event.format = 32;
    event.sequence=0;
    event.type = conn->_NET_WM_STATE_ATOM;
    event.data.data32[0] = 0;
    event.data.data32[1] = conn->_NET_WM_STATE_MAXIMIZED_VERT_ATOM;
    event.data.data32[2] = conn->_NET_WM_STATE_MAXIMIZED_HORZ_ATOM;
    event.data.data32[3] = 0;
    event.data.data32[4] = 0;

    xcb_send_event(conn->connection, false, conn->screen->root, XCB_EVENT_MASK_STRUCTURE_NOTIFY|XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char*)&event);
    xcb_flush(conn->connection);
}


HRESULT DefWindowProc(HWND hWnd,UINT msg,WPARAM wp,LPARAM lp){
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return -1;
    switch(msg){
        case WM_ERASEBKGND:
        {
            WNDCLASSEX info={0};
            //GetClassInfoEx return the atom value instead an bool
            ATOM clsAtom = GetClassInfoEx( wndObj->hInstance, MAKEINTRESOURCE(wndObj->clsAtom), &info );
            if(clsAtom && info.hbrBackground){
                FillRect((HDC)lp,&wndObj->rc,info.hbrBackground);
            }
        }
        break;
        case WM_GETMINMAXINFO:
            return -1;//not handle
        case WM_WINDOWPOSCHANGING:
        {
            WINDOWPOS *lpWndPos = (WINDOWPOS*)lp;
            if(!(lpWndPos->flags & (SWP_NOSIZE|SWP_NOMOVE))){
                MINMAXINFO info={0};
                if(0==SendMessage(hWnd,WM_GETMINMAXINFO,0,(LPARAM)&info)){
                    if(lpWndPos->cx > info.ptMaxTrackSize.x)
                        lpWndPos->cx = info.ptMaxTrackSize.x;
                    if(lpWndPos->cx < info.ptMinTrackSize.x)
                        lpWndPos->cx = info.ptMinTrackSize.x;

                    if(lpWndPos->cy > info.ptMaxTrackSize.y)
                        lpWndPos->cy = info.ptMaxTrackSize.y;
                    if(lpWndPos->cy < info.ptMinTrackSize.y)
                        lpWndPos->cy = info.ptMinTrackSize.y;

                    if(lpWndPos->x > info.ptMaxPosition.x)
                        lpWndPos->x = info.ptMaxPosition.x;
                    if(lpWndPos->y > info.ptMaxPosition.y)
                        lpWndPos->y = info.ptMaxPosition.y;
                }
            }
            
        }
        break;
    case WM_SYSCOMMAND:
    {
        WORD action = wp & 0xfff0;
        switch (action) {
        case SC_MOVE:
            if((wp&0x0f) == HTCAPTION) 
                HandleNcTestCode(hWnd, HTCAPTION);
            break;
        case SC_MINIMIZE:
            SendSysCommand(wndObj->mConnection, hWnd, XCB_ICCCM_WM_STATE_ICONIC);
            break;
        case SC_MAXIMIZE:
            ChangeNetWmState(wndObj->mConnection,hWnd,true,wndObj->mConnection->_NET_WM_STATE_MAXIMIZED_HORZ_ATOM,wndObj->mConnection->_NET_WM_STATE_MAXIMIZED_VERT_ATOM);
            break;
        case SC_RESTORE:
            SendSysRestore(wndObj->mConnection, hWnd);
            break;
        case SC_CLOSE:
            SendMessage(hWnd, WM_CLOSE, 0, 0);
            break;
        }
    }
        break;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    case WM_WINDOWPOSCHANGED:
        {
            WINDOWPOS & wndPos = *(WINDOWPOS*)lp;
            if(!(wndPos.flags  & SWP_NOMOVE)){
                const int32_t coords[] = {static_cast<int32_t>(wndPos.x),
                                    static_cast<int32_t>(wndPos.y)};
                xcb_configure_window(wndObj->mConnection->connection, hWnd,
                                XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);
                SendMessage(hWnd,WM_MOVE,0,MAKELPARAM(wndPos.x,wndPos.y));
            }
            if(!(wndPos.flags & SWP_NOSIZE)){
                const uint32_t coords[] = {static_cast<uint32_t>(wndPos.cx),
                                    static_cast<uint32_t>(wndPos.cy)};
                xcb_configure_window(wndObj->mConnection->connection, hWnd,
                                XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, coords);
                SendMessage(hWnd,WM_SIZE,0,MAKELPARAM(wndPos.cx,wndPos.cy));
            }
            int showCmd = -1;
            if(wndPos.flags & SWP_SHOWWINDOW)
            {
                if(wndPos.flags & SWP_NOACTIVATE)
                    showCmd = SW_SHOW;
                else
                    showCmd = SW_SHOWNOACTIVATE;
            }
            else if (wndPos.flags & SWP_HIDEWINDOW)
            {
                showCmd = SW_HIDE;
            }            
            if(showCmd!=-1) 
                ShowWindow(wndPos.hwnd,showCmd);
            xcb_flush(wndObj->mConnection->connection);
            if((!wndPos.flags & SWP_NOREDRAW)){
                InvalidateRect(hWnd,nullptr,TRUE);
            }
        }
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
    BOOL bVisible = IsWindowVisible(hWnd);
    BOOL bNew = nCmdShow & SW_SHOW;
    if (bVisible == bNew)
        return TRUE;
    if (nCmdShow & SW_SHOW)
    {
        xcb_map_window(wndObj->mConnection->connection, hWnd);
        InvalidateRect(hWnd, nullptr, TRUE);
    }
    else
    {
        xcb_unmap_window(wndObj->mConnection->connection, hWnd);
    }    
    xcb_flush(wndObj->mConnection->connection);
    SendMessage(hWnd, WM_SHOWWINDOW, bNew, 0);
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
    xcb_connection_t *connection=wndObj->mConnection->connection;
    RECT rcWnd;
    if(!lpRect){
        GetClientRect(hWnd,&rcWnd);
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
    return SetWindowPos(hWnd,0,x,y,nWidth,nHeight,SWP_NOZORDER|(bRepaint?0:SWP_NOREDRAW));
}

BOOL IsWindowVisible(HWND hWnd)
{
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
    xcb_connection_t *connection=wndObj->mConnection->connection;

    xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(connection, hWnd);
    xcb_get_window_attributes_reply_t *reply = xcb_get_window_attributes_reply(connection, cookie, NULL);
    if (!reply)
        return FALSE;
    uint8_t mapState = reply->map_state;
    free(reply);
    return mapState == XCB_MAP_STATE_VIEWABLE;
}


BOOL IsZoomed(HWND hWnd){
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
    return wndObj->state == Maximized;
}

BOOL IsIconic(HWND hWnd){
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return FALSE;
    return wndObj->state == Minimized;
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
    xcb_change_property(wndObj->mConnection->connection, XCB_PROP_MODE_REPLACE, hWnd,
        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, wndObj->title.length(), wndObj->title.c_str());
    xcb_flush(wndObj->mConnection->connection);
    return TRUE;
}

HDC GetDC(HWND hWnd){
    if(!hWnd){
        SConnection* conn = SConnMgr::instance()->getConnection();
        return conn->GetDC();
    }
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj || !wndObj->hdc)
        return 0;
    return wndObj->hdc;
}

int ReleaseDC(HWND hWnd,HDC hdc){
    if(!hWnd){
        SConnection* conn = SConnMgr::instance()->getConnection();
        return conn->ReleaseDC(hdc);;
    }
    WndObj wndObj = WndObj::fromHwnd(hWnd);
    if(!wndObj)
        return 0;
    xcb_flush(wndObj->mConnection->connection);
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
