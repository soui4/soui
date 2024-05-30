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
    PostMessage(WM_QUIT,1);
    //todo: PostThreadMessage not work now
    //PostThreadMessage(GetCurrentThreadId(),WM_QUIT,0,0);  
}

TEST(Window,loop){
    SNativeWnd::InitWndClass(0,"soui_host",FALSE);
    SNativeWnd2 wnd;
    HWND hWnd = wnd.CreateNative("test window",WS_POPUP|WS_VISIBLE,0,0,0,400,400,0);
    if(hWnd){
        SMessageLoop loop(nullptr);
        int ret = loop.Run();
        wnd.DestroyWindow();

        EXPECT_EQ(ret,1);
    }
}