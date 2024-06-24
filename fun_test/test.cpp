#include <stdio.h>
#include <gtest/gtest.h>
#include <xml/SXml.h>
#include <string/sstringa.h>
#include <string/sstringw.h>
#include <string/strcpcvt.h>
#include <helper/SRwLock.h>
#include <SNativeWnd.h>
#include <SMsgLoop.h>
#include <wtl.mini/souimisc.h>
#define _WTYPES_NS SOUI

#include <wtl.mini/msgcrack.h>
#include <cairo/cairo-xcb.h>
#include <xcb/xcb_aux.h>

using namespace SOUI;

TEST(Util,Xml_Load){
    SXmlDoc xml;
    EXPECT_EQ(xml.load_file("/home/flyhigh/work/soui4/demo2/uires/xml/dlg_main.xml"),true);
    SXmlNode node=xml.root().first_child();
    while(node){
        const wchar_t * name = node.name();
        wprintf(L"node name=%s\n",name);
        node = node.next_sibling();
    }
}

TEST(Util,Str_Cvt){
    setlocale(LC_ALL, "");
    SStringA src = "再跌几个月得6.0了";
    SStringW wstr = S_CA2W(src,CP_UTF8);
    
    SStringA src2 = S_CW2A(wstr,CP_UTF8);
    bool beq = src == src2;
    
    EXPECT_EQ(beq,true);
}

TEST(Util, RwLock) {
    SRwLock lock;
    lock.LockShared();
    lock.UnlockShared();

    lock.LockExclusive();
    lock.UnlockExclusive();

    lock.LockShared();
    lock.LockShared();
    lock.UnlockShared();
    lock.UnlockShared();
    EXPECT_EQ(1, 1);
}

class SNativeWnd2: public SNativeWnd{
public:
SNativeWnd2(){};

protected:
void OnClose();
void OnPaint(HDC hdc);
void OnSize(UINT state,SIZE sz);
BEGIN_MSG_MAP_EX(SNativeWnd2)
 MSG_WM_CLOSE(OnClose)
 MSG_WM_PAINT(OnPaint)
 MSG_WM_SIZE(OnSize)
END_MSG_MAP()
};

void SNativeWnd2::OnSize(UINT state,SIZE sz){
    SetMsgHandled(FALSE);
}

void SNativeWnd2::OnPaint(HDC hdc){
    PAINTSTRUCT ps;
    hdc = BeginPaint(m_hWnd,&ps);
    RECT rcWnd;
    GetClientRect(&rcWnd);
    cairo_t *cr =(cairo_t*)hdc;
    cairo_set_source_rgb(cr, 1.0, 0.5, 0.0); // 设置绘图上下文的颜色
    cairo_rectangle(cr, rcWnd.left, rcWnd.top, rcWnd.right-rcWnd.left, rcWnd.bottom-rcWnd.top); // 绘制矩形
    cairo_fill(cr); // 填充矩形
    EndPaint(m_hWnd,&ps);
}
void SNativeWnd2::OnClose(){
    //PostMessage(WM_QUIT,1);
    PostThreadMessage(GetCurrentThreadId(),WM_QUIT,1,0);  
}

int
main2 ()
{
  xcb_connection_t    *c;
  xcb_screen_t        *screen;
  xcb_drawable_t       win;
  xcb_gcontext_t       foreground;
  xcb_generic_event_t *e;
  uint32_t             mask = 0;
  uint32_t             values[2];

  /* geometric objects */
  xcb_point_t          points[] = {
    {10, 10},
    {10, 20},
    {20, 10},
    {20, 20}};

  xcb_point_t          polyline[] = {
    {50, 10},
    { 5, 20},     /* rest of points are relative */
    {25,-20},
    {10, 10}};

  xcb_segment_t        segments[] = {
    {100, 10, 140, 30},
    {110, 25, 130, 60}};

  xcb_rectangle_t      rectangles[] = {
    { 10, 50, 40, 20},
    { 80, 50, 10, 40}};

  xcb_arc_t            arcs[] = {
    {10, 100, 60, 40, 0, 90 << 6},
    {90, 100, 55, 40, 0, 270 << 6}};

  /* Open the connection to the X server */
  c = xcb_connect (NULL, NULL);

  /* Get the first screen */
  screen = xcb_setup_roots_iterator (xcb_get_setup (c)).data;

  /* Create black (foreground) graphic context */
  win = screen->root;

  foreground = xcb_generate_id (c);
  mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
  values[0] = screen->black_pixel;
  values[1] = 0;
  xcb_create_gc (c, foreground, win, mask, values);

  /* Ask for our window's Id */
  win = xcb_generate_id(c);

  /* Create the window */
  mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = screen->white_pixel;
  values[1] = XCB_EVENT_MASK_EXPOSURE
  |XCB_EVENT_MASK_STRUCTURE_NOTIFY
  |XCB_EVENT_MASK_PROPERTY_CHANGE;
  xcb_create_window (c,                             /* Connection          */
                     XCB_COPY_FROM_PARENT,          /* depth               */
                     win,                           /* window Id           */
                     screen->root,                  /* parent window       */
                     0, 0,                          /* x, y                */
                     150, 150,                      /* width, height       */
                     10,                            /* border_width        */
                     XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
                     screen->root_visual,           /* visual              */
                     mask, values);                 /* masks */

  /* Map the window on the screen */
  xcb_map_window (c, win);


  /* We flush the request */
  xcb_flush (c);

  while ((e = xcb_wait_for_event (c))) {
    switch (e->response_type & ~0x80) {
        case XCB_CONFIGURE_NOTIFY:{
            xcb_configure_notify_event_t *e2 = (xcb_configure_notify_event_t *)e;
            printf("XCB_CONFIGURE_NOTIFY, wid=%d,hei=%d\n",e2->width,e2->height);
            break;
        }
    case XCB_EXPOSE: {
        xcb_get_geometry_cookie_t cookie = xcb_get_geometry(c, win);
        xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(c, cookie, NULL);
        printf("expose size:(%d,%d)\n",reply->width,reply->height);
        free(reply);
      /* We draw the points */
      xcb_poly_point (c, XCB_COORD_MODE_ORIGIN, win, foreground, 4, points);

      /* We draw the polygonal line */
      xcb_poly_line (c, XCB_COORD_MODE_PREVIOUS, win, foreground, 4, polyline);

      /* We draw the segements */
      xcb_poly_segment (c, win, foreground, 2, segments);

      /* We draw the rectangles */
      xcb_poly_rectangle (c, win, foreground, 2, rectangles);

      /* We draw the arcs */
      xcb_poly_arc (c, win, foreground, 2, arcs);

      /* We flush the request */
      xcb_flush (c);
        printf("xcb_expose\n");
      break;
    }
    default: {
      /* Unknown event type, ignore it */
      break;
    }
    }
    /* Free the Generic Event */
    free (e);
  }

  return 0;
}

TEST(Window,demo){
    EXPECT_EQ(main2(),0);
}

TEST(Window,loop){
/*    
    SNativeWnd::InitWndClass(0,"soui_host",FALSE);
    SNativeWnd2 wnd;
    HWND hWnd = wnd.CreateNative("test window",WS_POPUP|WS_VISIBLE,0,0,0,400,400,0);
    if(hWnd){
        SMessageLoop loop(nullptr);
        int ret = loop.Run();
        wnd.DestroyWindow();

        EXPECT_EQ(ret,1);
    }

    //*/
}