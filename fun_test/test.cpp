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

#include <commgr2.h>
#include <interface/SRender-i.h>
#include <string>

#include <souistd.h>
#include <SouiFactory.h>

using namespace SOUI;

int flash2() {
    // 连接到 X 服务器
    xcb_connection_t* connection = xcb_connect(NULL, NULL);
    const xcb_setup_t* setup = xcb_get_setup(connection);
    xcb_screen_t* screen = xcb_setup_roots_iterator(setup).data;

    // 创建一个有边框的窗口
    xcb_window_t window = xcb_generate_id(connection);

    int wid = 300, hei = 300;

    // 创建窗口
    uint32_t vals[] = { screen->white_pixel, XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_EXPOSURE };
    xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root,
        100, 100, wid, hei, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual, XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
        vals);

    // 设置窗口边框
    uint32_t border_color = 0x00ff00; // Green color for the border
    xcb_change_window_attributes(connection, window, XCB_CW_BORDER_PIXEL, &border_color);

    // 显示窗口
    xcb_map_window(connection, window);

    // 刷新连接
    xcb_flush(connection);

    // Cairo绘制
    xcb_generic_event_t* event;
    cairo_surface_t* surface = cairo_xcb_surface_create(connection, window, xcb_aux_find_visual_by_id(screen, screen->root_visual), wid, hei);
    cairo_t* cr = cairo_create(surface);

    while ((event = xcb_wait_for_event(connection))) {
        switch (event->response_type & ~0x80) {
        case XCB_EXPOSE: {
            // 绘制红色矩形
            cairo_set_source_rgb(cr, 1.0, 0.0, 0.0); // 设置颜色为红色
            cairo_rectangle(cr, 0, 0, wid, hei); // 矩形大小
            cairo_fill(cr); // 填充矩形

            //cairo_surface_flush(surface);
            xcb_flush(connection);
            break;
        }
        case XCB_CONFIGURE_NOTIFY: {
            // 处理窗口大小变化事件
            xcb_configure_notify_event_t* configure_event = (xcb_configure_notify_event_t*)event;
            // 更新绘制区域大小
            wid = configure_event->width, hei = configure_event->height;
            cairo_surface_destroy(surface);
            cairo_destroy(cr);

            surface = cairo_xcb_surface_create(connection, window, xcb_aux_find_visual_by_id(screen, screen->root_visual), wid, hei);
            cr = cairo_create(surface);
            break;
        }
        default:
            break;
        }
        free(event);
    }

    // 清理资源
    cairo_surface_destroy(surface);
    cairo_destroy(cr);

    // 断开连接
    xcb_disconnect(connection);

    return 0;
}

int flash() {
    // 连接到 X 服务器
    xcb_connection_t* connection = xcb_connect(NULL, NULL);
    const xcb_setup_t* setup = xcb_get_setup(connection);
    xcb_screen_t* screen = xcb_setup_roots_iterator(setup).data;

    // 创建一个有边框的窗口
    xcb_window_t window = xcb_generate_id(connection);

    int wid = 300, hei = 300;
    // 创建窗口
    uint32_t values[] = { screen->white_pixel, XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_EXPOSURE };
    xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root,
        100, 100, wid, hei, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual, XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
        values
    );

    // 显示窗口
    xcb_map_window(connection, window);

    // 刷新连接
    xcb_flush(connection);

    // 双缓冲绘制
    xcb_generic_event_t* event;
    xcb_gcontext_t gc = xcb_generate_id(connection);

    uint32_t val[] = { screen->black_pixel, 0 };
    xcb_create_gc(connection, gc, window, XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES, val);

    xcb_pixmap_t pixmap = xcb_generate_id(connection);
    xcb_create_pixmap(connection, screen->root_depth, pixmap, window, 1, 1);

    while ((event = xcb_wait_for_event(connection))) {
        switch (event->response_type & ~0x80) {
        case XCB_EXPOSE: {
            // 双缓冲绘制
            xcb_rectangle_t rect = { 0, 0, (uint16_t)wid, (uint16_t)hei };
            uint32_t color = 0xff0000;
            xcb_change_gc(connection, gc, XCB_GC_FOREGROUND, &color);
            xcb_poly_fill_rectangle(connection, pixmap, gc, 1, &rect);

            xcb_copy_area(connection, pixmap, window, gc, 0, 0, 0, 0, wid, hei);
            xcb_flush(connection);
            break;
        }
        case XCB_CONFIGURE_NOTIFY: {
            // 处理窗口大小变化事件
            xcb_configure_notify_event_t* configure_event = (xcb_configure_notify_event_t*)event;
            // 更新绘制区域大小
            xcb_free_pixmap(connection, pixmap);
            pixmap = xcb_generate_id(connection);
            wid = configure_event->width, hei=configure_event->height;
            xcb_create_pixmap(connection, screen->root_depth, pixmap, window,wid,hei );
            break;
        }
        default:
            break;
        }
        free(event);
    }

    // 清理资源
    xcb_free_pixmap(connection, pixmap);
    xcb_free_gc(connection, gc);

    // 断开连接
    xcb_disconnect(connection);

    return 0;
}

int frameless_window() {
    // 连接到 X 服务器
    xcb_connection_t* connection = xcb_connect(NULL, NULL);
    const xcb_setup_t* setup = xcb_get_setup(connection);
    xcb_screen_t* screen = xcb_setup_roots_iterator(setup).data;

    // 创建一个没有窗口外框的窗口
    xcb_window_t window = xcb_generate_id(connection);

    // 创建窗口
    xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root,
        0, 0, 800, 600, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual, 0, NULL);

    // 设置窗口类型为 _NET_WM_WINDOW_TYPE_DOCK
    const char* type_name = "_NET_WM_WINDOW_TYPE";
    xcb_intern_atom_cookie_t atom_cookie = xcb_intern_atom(connection, 0, strlen(type_name), type_name);
    xcb_intern_atom_reply_t* atom_reply = xcb_intern_atom_reply(connection, atom_cookie, NULL);
    xcb_atom_t window_type = atom_reply->atom;

    const char* type_dock_name = "_NET_WM_WINDOW_TYPE_SPLASH";
    xcb_intern_atom_cookie_t type_dock_cookie = xcb_intern_atom(connection, 0, strlen(type_dock_name), type_dock_name);
    xcb_intern_atom_reply_t* type_dock_reply = xcb_intern_atom_reply(connection, type_dock_cookie, NULL);
    xcb_atom_t window_type_dock = type_dock_reply->atom;

    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, window_type, XCB_ATOM_ATOM, 32, 1, &window_type_dock);


    // 显示窗口
    xcb_map_window(connection, window);

    // 刷新连接
    xcb_flush(connection);

    // 进入事件循环
    xcb_generic_event_t* event;
    while ((event = xcb_wait_for_event(connection))) {
        switch (event->response_type & ~0x80) {
        case XCB_KEY_PRESS: {
            // 处理键盘按键事件
            break;
        }
        case XCB_EXPOSE: {
            // 处理重绘事件
            break;
        }
        default: {
            // 其他事件处理
            break;
        }
        }
        free(event);
    }

    // 断开连接
    xcb_disconnect(connection);

    return 0;
}

TEST(window, frameless) {
    //frameless_window();
    flash2();
}

//获得源文件根路径,区分在VS中远程调试及在vscode中本机运行两种模式
std::string getSourceDir() {
    char szPath[MAX_PATH];
    GetModuleFileName(nullptr, szPath, MAX_PATH);
    //if run on remote linux machine, path should like /home/user/.vs/pathname/xxx/out/build/Linux-GCC-Debug/bin
    //convert this path to /home/user/.vs/pathname/xxx/src
    char* pdotvs = strstr(szPath, ".vs");
    if (pdotvs) {
        //run on remote linux machine
        char* out = strstr(szPath, "out");
        SASSERT(out);
        strcpy(out, "src");
        return szPath;
    }
    else {
        char* build = strstr(szPath, "/build/bin");
        SASSERT(build);
        build[0] = 0;
        return szPath;
    }
}

static const char * kPath_TestPng ="/demo/uires/image/soui.png";
static const char * kPath_TestXml ="/demo2/uires/xml/dlg_main.xml";
static const char * kPath_SysRes = "/soui-sys-resource";
static const char * kPath_TestRes = "/fun_test/uires";

TEST(soui, region_and) {
    RECT rc1 = { 0,0,600,400 };
    RECT rc2 = { 10,10,56,18 };
    RECT rcCom = { 0,0,600,400 };
    HRGN rgn1 = CreateRectRgnIndirect(&rc1);
    HRGN rgn2 = CreateRectRgnIndirect(&rc2);
    CombineRgn(rgn1, rgn1, rgn2, RGN_OR);
    DeleteObject(rgn2);
    RECT rc3;
    GetRgnBox(rgn1, &rc3);
    DeleteObject(rgn1);
    EXPECT_EQ(memcmp(&rc3, &rcCom, sizeof(RECT)), 0);
}

TEST(soui, region_diff) {
    RECT rc1 = { 0,0,600,400 };
    RECT rc2 = { 0,0,100,400 };
    RECT rcCom = { 100,0,600,400 };
    HRGN rgn1 = CreateRectRgnIndirect(&rc1);
    HRGN rgn2 = CreateRectRgnIndirect(&rc2);
    CombineRgn(rgn1, rgn1, rgn2, RGN_DIFF);
    DeleteObject(rgn2);
    RECT rc3;
    GetRgnBox(rgn1, &rc3);
    DeleteObject(rgn1);
    EXPECT_EQ(memcmp(&rc3, &rcCom, sizeof(RECT)), 0);
}

TEST(soui, region_or) {
    RECT rc1 = { 0,0,600,400 };
    RECT rc2 = { 600,0,700,400 };
    RECT rcCom = { 0,0,700,400 };
    HRGN rgn1 = CreateRectRgnIndirect(&rc1);
    HRGN rgn2 = CreateRectRgnIndirect(&rc2);
    CombineRgn(rgn1, rgn1, rgn2, RGN_OR);
    DeleteObject(rgn2);
    RECT rc3;
    GetRgnBox(rgn1, &rc3);
    DeleteObject(rgn1);
    EXPECT_EQ(memcmp(&rc3, &rcCom, sizeof(RECT)), 0);
}

HBITMAP LoadPng(const char * path){
    SComMgr2 comMgr("libimgdecoder-stb");
    SAutoRefPtr<IImgDecoderFactory> decoder;
    EXPECT_EQ(comMgr.CreateImgDecoder((IObjRef**)&decoder),TRUE);
    if(!decoder){
        printf("load image decoder failed!\n");
        return 0;
    }
    SAutoRefPtr<IImgX> img;
    decoder->CreateImgX(&img);
    EXPECT_EQ(img->LoadFromFileA(path),true);
    
    IImgFrame * pFrame = img->GetFrame(0);
    if (!pFrame) {
        printf("get frame failed!\n");
        return 0;
    }
    UINT wid,hei;
    pFrame->GetSize(&wid,&hei);
    BITMAPINFO bmi;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biWidth = wid;
    bmi.bmiHeader.biHeight = hei;
    HBITMAP bmp = CreateDIBSection(0,&bmi,0,NULL,0,0);
    if(bmp){
        UpdateDIBPixmap(bmp,wid,hei,32,wid*4,pFrame->GetPixels());
    }
    return bmp;
}


TEST(Util,sstring){
    SStringT str="test";
    SStringT str2 ="soui";
    SStringT str3 = str+_T("/")+"soui";
    EXPECT_EQ(str3=="test/soui",true);
}


TEST(Util,com_load){
    std::string srcDir = getSourceDir();
    HBITMAP bmp = LoadPng((srcDir+kPath_TestPng).c_str());
    EXPECT_EQ(bmp!=0,TRUE);
    if(bmp){
        DeleteObject(bmp);
    }
}

TEST(Util,Xml_Load){
    std::string srcDir = getSourceDir();
    SXmlDoc xml;
    EXPECT_EQ(xml.load_file((srcDir+kPath_TestXml).c_str()),true);
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
    HBITMAP m_bmp;
public:
SNativeWnd2():m_bmp(0){};

protected:
void OnClose();
void OnPaint(HDC hdc);
void OnSize(UINT state,SIZE sz);
void OnDestroy();
int  OnCreate(LPCREATESTRUCT lpCs);

BEGIN_MSG_MAP_EX(SNativeWnd2)
 MSG_WM_CLOSE(OnClose)
 MSG_WM_PAINT(OnPaint)
 MSG_WM_SIZE(OnSize)
 MSG_WM_CREATE(OnCreate)
 MSG_WM_DESTROY(OnDestroy)
END_MSG_MAP()
};

void SNativeWnd2::OnDestroy(){
    if(m_bmp){
        DeleteObject(m_bmp);
        m_bmp=0;
    }
    SetMsgHandled(FALSE);
}

int  SNativeWnd2::OnCreate(LPCREATESTRUCT lpCs){
    std::string srcDir = getSourceDir();
    m_bmp = LoadPng((srcDir+kPath_TestPng).c_str());
    if(!m_bmp) return 1;
    SetMsgHandled(FALSE);
    return 0;
}

void SNativeWnd2::OnSize(UINT state,SIZE sz){
    SetMsgHandled(FALSE);
}

void SNativeWnd2::OnPaint(HDC hdc){
    RECT rcWnd;
    GetClientRect(&rcWnd);
    PAINTSTRUCT ps;
    hdc = BeginPaint(m_hWnd,&ps);
    if(1){
        HDC memdc = CreateCompatibleDC(hdc);
        HBITMAP bmp=CreateCompatibleBitmap(hdc,rcWnd.right,rcWnd.bottom);
        HGDIOBJ oldBmp = SelectObject(memdc,bmp);
        HBRUSH hbr = CreateSolidBrush(RGBA(255,0,0,255));
        int save = SaveDC(memdc);
        RestoreDC(memdc, save);

        save = SaveDC(memdc);     
        HRGN hrgn = CreateRectRgn(0,0,100,100);
        HRGN hrgn2 = CreateRectRgn(100,100,500,500);
        CombineRgn(hrgn,hrgn,hrgn2,RGN_OR);
        //SelectClipRgn(memdc, hrgn);
        ExtSelectClipRgn(memdc,hrgn,RGN_COPY);
        RECT rcClip;
        GetClipBox(memdc, &rcClip);

        DeleteObject(hrgn);
        DeleteObject(hrgn2);

        FillRect(memdc,&rcWnd,hbr);
        RestoreDC(memdc, -1);
        const char* txt = "hello soui";
        SIZE szTxt;
        GetTextExtentPoint32(memdc, txt, -1, &szTxt);
        CRect rc(CPoint(10, 70), szTxt);
        SetTextColor(memdc, RGBA(0, 255, 0, 255));
        DrawText(memdc, txt, -1,&rc,DT_SINGLELINE);

        //BitBlt(hdc, 0, 0, rcWnd.right, rcWnd.bottom, memdc, 0, 0, SRCCOPY);
        StretchBlt(hdc,0,0,rcWnd.right,rcWnd.bottom,memdc,0,0, rcWnd.right, -rcWnd.bottom,SRCCOPY);

        SelectObject(memdc,oldBmp);
        DeleteObject(bmp);
        DeleteDC(memdc);
    }


    SetTextColor(hdc,RGBA(255,0,0,255));

    LOGFONT lf={0};
    lf.lfHeight = 50;
    strcpy(lf.lfFaceName,"Ani");
    HFONT hFont = CreateFontIndirect(&lf);
    HFONT oldFont = SelectObject(hdc,hFont);

    std::string str = "hello soui";
    SIZE sz={0};
    GetTextExtentPoint32(hdc,str.c_str(),str.length(),&sz);
    HPEN pen = CreatePen(PS_SOLID,1,RGBA(0,0,255,255));
    HPEN oldPen = SelectObject(hdc,pen);

    HBRUSH br = CreateSolidBrush(RGBA(128,128,128,255));
    HBRUSH oldBr = SelectObject(hdc,br);
    Rectangle(hdc,10,10,10+sz.cx,10+sz.cy);
    Rectangle(hdc,0,100,64,164);

    TextOut(hdc,10,20,str.c_str(),-1);

    SelectObject(hdc,oldBr);
    DeleteObject(br);

    {//test arc api
        RECT rcArc = { 10,50,60,200 };
        int wid = rcArc.right - rcArc.left;
        int hei = rcArc.bottom - rcArc.top;
        OffsetRect(&rcArc, 10, 10);
        auto oldbr = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Ellipse(hdc, rcArc.left, rcArc.top, rcArc.right, rcArc.bottom);
        OffsetRect(&rcArc, wid, 0);
        Arc(hdc, rcArc.left, rcArc.top, rcArc.right, rcArc.bottom, rcArc.left, (rcArc.top + rcArc.bottom) / 2, (rcArc.left + rcArc.right) / 2, rcArc.top);
        OffsetRect(&rcArc, wid, 0);
        SelectObject(hdc, oldbr);
        HPEN pen = CreatePen(PS_DASH, 2, RGBA(0, 128, 0, 255));
        HPEN oldPen = SelectObject(hdc, pen);
        Pie(hdc, rcArc.left, rcArc.top, rcArc.right, rcArc.bottom, rcArc.left, (rcArc.top + rcArc.bottom) / 2, (rcArc.left + rcArc.right) / 2, rcArc.top);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    SelectObject(hdc,oldPen);
    DeleteObject(pen);

    SelectObject(hdc,oldFont);
    DeleteObject(hFont);

    //draw hbitmap
    HDC memdc = CreateCompatibleDC(hdc);
    HBITMAP oldBmp = SelectObject(memdc,m_bmp);

    RECT rc2={0,100,64,100+64};
    //BitBlt(hdc,rc2.left,rc2.top,rc2.right-rc2.left,rc2.bottom-rc2.top,memdc,64,64,SRCCOPY);
    BLENDFUNCTION bf;
    bf.SourceConstantAlpha = 128;//set alpha to 0.5
    AlphaBlend(hdc,100,100,256,256,memdc,0,0,128,128,bf);
    SelectObject(memdc,oldBmp);
    DeleteDC(memdc);

    {//test gradient
        TRIVERTEX        vert[3];
        GRADIENT_RECT    gRect[2];
        vert[0].x = 10;
        vert[0].y = 300;
        vert[0].Red = 0x0000;
        vert[0].Green = 0x0000;
        vert[0].Blue = 0x0000;
        vert[0].Alpha = 0xffff;

        vert[1].x = 200;
        vert[1].y = 350;
        vert[1].Red = 0x0000;
        vert[1].Green = 0x0000;
        vert[1].Blue = 0xff00;
        vert[1].Alpha = 0xffff;

        vert[2].x = 300;
        vert[2].y = 300;
        vert[2].Red = 0x0000;
        vert[2].Green = 0xff00;
        vert[2].Blue = 0x0000;
        vert[2].Alpha = 0xffff;

        gRect[0].UpperLeft = 0;
        gRect[0].LowerRight = 1;
        gRect[1].UpperLeft = 1;
        gRect[1].LowerRight = 2;
        GradientFill(hdc, vert, 3, &gRect, 2, GRADIENT_FILL_RECT_H);
    }
    EndPaint(m_hWnd,&ps);
}
void SNativeWnd2::OnClose(){
    //PostMessage(WM_QUIT,1);
    //SetWindowPos(0, 0, 0, 600, 600, SWP_NOMOVE | SWP_NOZORDER);
    PostThreadMessage(GetCurrentThreadId(),WM_QUIT,1,0);  
}

int run_window() {
    int ret = 0;
    SNativeWnd::InitWndClass(0, "soui_host", FALSE);
    SNativeWnd2 wnd;
    HWND hWnd = wnd.CreateNative("test window", WS_POPUP|WS_BORDER, 0, 0, 0, 0, 0, 0);
    if (hWnd) {
        wnd.SetWindowPos(0, 0, 0, 600, 600, SWP_NOMOVE | SWP_NOZORDER);
        wnd.ShowWindow(SW_SHOW);
        SMessageLoop loop(nullptr);
        ret = loop.Run();
        wnd.DestroyWindow();
    }
    return ret;
}
TEST(demo,window){
    //EXPECT_EQ(run_window(), 1);
}

class CMainDlg : public SHostWnd {
public:
    CMainDlg(LPCSTR pszLayout) :SHostWnd(pszLayout) {}

    void OnClose();
    EVENT_MAP_BEGIN()
        EVENT_ID_COMMAND(1,OnClose)
    EVENT_MAP_END2(SHostWnd)
};

void CMainDlg::OnClose() {
    PostMessage(WM_QUIT);
}

int run_app(HINSTANCE hInst){
    std::string srcDir = getSourceDir();

    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFac;
    comMgr.CreateRender_GDI((IObjRef**)&renderFac);
    SAutoRefPtr<IImgDecoderFactory> imgDecoder;
    comMgr.CreateImgDecoder((IObjRef**)&imgDecoder);
    renderFac->SetImgDecoderFactory(imgDecoder);
    
    SApplication app(renderFac,hInst);
    SouiFactory sfac;
    SAutoRefPtr<IResProvider> sysResouce(sfac.CreateResProvider(RES_FILE));
    std::string sysRes = srcDir + kPath_SysRes;
    sysResouce->Init((LPARAM)sysRes.c_str(), 0);
    app.LoadSystemNamedResource(sysResouce);
    SAutoRefPtr<IResProvider> testResouce(sfac.CreateResProvider(RES_FILE));
    std::string appRes = srcDir + kPath_TestRes;
    testResouce->Init((LPARAM)appRes.c_str(), 0);
    app.AddResProvider(testResouce);

    CMainDlg hostWnd("layout:XML_MAINWND");
    hostWnd.Create(0);
    hostWnd.ShowWindow(SW_SHOW);
    app.Run(hostWnd.m_hWnd);
    
    return 0;
}

TEST(demo,app){
    HINSTANCE hInst = 0;
    //EXPECT_EQ(run_app(hInst),0);
}