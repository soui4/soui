#include <stdio.h>
#include <gtest/gtest.h>
#include <xml/SXml.h>
#include <string/sstringa.h>
#include <string/sstringw.h>
#include <string/strcpcvt.h>
#include <helper/SRwLock.h>
#include <SNativeWnd.h>
#include <SMsgLoop.h>
#include <wtl.mini/msgcrack.h>
#include <xcb/xcb.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

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

BEGIN_MSG_MAP_EX(SNativeWnd2)
 MSG_WM_CLOSE(OnClose)
END_MSG_MAP()
};

void SNativeWnd2::OnClose(){
    //PostMessage(WM_QUIT,1);
    //todo: PostThreadMessage not work now
    PostThreadMessage(GetCurrentThreadId(),WM_QUIT,0,0);  
}


int main2() {
    // 建立与X服务器的连接并获取一个连接对象
    xcb_connection_t *connection = xcb_connect(NULL, NULL);

    // 创建一个事件结构体并对其进行初始化
    xcb_generic_event_t *event;
    xcb_window_t root = xcb_setup_roots_iterator(xcb_get_setup(connection)).data->root;
    // 设置事件的类型
    xcb_client_message_event_t client_message_event;
    memset(&client_message_event, 0, sizeof(client_message_event));
    client_message_event.response_type = XCB_CLIENT_MESSAGE;

    // 设置发送事件的窗口的ID（这里用根窗口的ID作为示例）
    client_message_event.window = root;

    std::string evt_name="myevent";
    // 设置事件的序列号（可以是任意值）
    client_message_event.sequence = 100;
    client_message_event.type = xcb_intern_atom_reply(connection, xcb_intern_atom(connection, 0, evt_name.length(), evt_name.c_str()), NULL)->atom;
    //client_message_event.format = 32;
    //client_message_event.data.data32[0] = 123;  // 自定义数据，可以根据需要进行修改

    // 发送事件到连接的事件队列
    xcb_send_event(connection, 0, root, XCB_EVENT_MASK_NO_EVENT, (const char *)&client_message_event);

    // 刷新连接以确保事件被发送
    xcb_flush(connection);

    // 接收事件
    while ((event = xcb_poll_for_event(connection)) != NULL) {
        // 处理事件，这里只是简单地打印事件类型
        printf("Received event: %d\n", event->response_type & ~0x80);
        free(event);
    }

    // 关闭连接
    xcb_disconnect(connection);

    return 0;
}

int main3() {
    // 建立与X服务器的连接并获取一个连接对象
    xcb_connection_t *connection = xcb_connect(NULL, NULL);

    // 创建一个事件结构体并对其进行初始化
    xcb_generic_event_t *event;

    // 发送事件到连接的事件队列
    xcb_client_message_event_t client_message_event;
    memset(&client_message_event, 0, sizeof(client_message_event));
    client_message_event.response_type = XCB_CLIENT_MESSAGE;
    client_message_event.window = xcb_setup_roots_iterator(xcb_get_setup(connection)).data->root;
    client_message_event.type = xcb_intern_atom_reply(connection, xcb_intern_atom(connection, 0, 16, "MY_EVENT_TYPE"), NULL)->atom;
    client_message_event.format = 32;
    client_message_event.data.data32[0] = 123;  // 自定义数据，可以根据需要进行修改

    xcb_send_event(connection, 0, xcb_setup_roots_iterator(xcb_get_setup(connection)).data->root,
                   XCB_EVENT_MASK_NO_EVENT, (const char *)&client_message_event);

    // 刷新连接以确保事件被发送
    xcb_flush(connection);

    // 接收事件
    while ((event = xcb_poll_for_event(connection)) != NULL) {
        // 处理事件，这里只是简单地打印事件类型
        printf("Received event: %d\n", event->response_type & ~0x80);
        free(event);
    }

    // 关闭连接
    xcb_disconnect(connection);

    return 0;
}



int main4() {
    // 打开与X服务器的连接
    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "无法打开显示连接\n");
        return 1;
    }

    // 获取默认屏幕和根窗口
    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);

    // 创建一个自定义事件类型
    Atom event_type = XInternAtom(display, "MY_EVENT_TYPE", False);

    // 创建一个窗口无关的事件
    XEvent event;
    memset(&event, 0, sizeof(event));
    event.xclient.type = ClientMessage;
    event.xclient.window = root;
    event.xclient.message_type = event_type;
    event.xclient.format = 32;
    event.xclient.data.l[0] = 123; // 自定义数据，可以根据需要进行修改

    // 发送事件到事件队列
    XSendEvent(display, root, False, NoEventMask, &event);

    // 刷新连接以确保事件被发送
    XFlush(display);

    // 接收事件
    XEvent received_event;
    XNextEvent(display, &received_event);

    // 检查接收到的事件类型是否为自定义事件类型
    if (received_event.type == ClientMessage && received_event.xclient.message_type == event_type) {
        // 处理事件，这里只是简单地打印事件数据
        printf("Received event: data = %ld\n", received_event.xclient.data.l[0]);
    }

    // 关闭显示连接
    XCloseDisplay(display);

    return 0;
}

TEST(Window,loop){
    main4();
}