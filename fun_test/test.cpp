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

static const char * kPath_TestPng ="/home/flyhigh/work/soui4/demo/uires/image/soui.png";
static const char * kPath_TestXml ="/home/flyhigh/work/soui4/demo2/uires/xml/dlg_main.xml";
static const char * kPath_SysRes = "/home/flyhigh/work/soui4/soui-sys-resource";
static const char * kPath_TestRes = "/home/flyhigh/work/soui4/fun_test/uires";

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
    HBITMAP bmp = LoadPng(kPath_TestPng);
    EXPECT_EQ(bmp!=0,TRUE);
    if(bmp){
        DeleteObject(bmp);
    }
}

TEST(Util,Xml_Load){
    SXmlDoc xml;
    EXPECT_EQ(xml.load_file(kPath_TestXml),true);
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
    m_bmp = LoadPng(kPath_TestPng);
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
        BitBlt(hdc,0,0,rcWnd.right,rcWnd.bottom,memdc,0,0,SRCCOPY);
        RestoreDC(memdc, -1);

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

    TextOut(hdc,10,10,str.c_str(),-1);

    SelectObject(hdc,oldBr);
    DeleteObject(br);

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
    AlphaBlend(hdc,100,100,128,128,memdc,0,0,64,64,bf);
    SelectObject(memdc,oldBmp);
    DeleteDC(memdc);

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
    HWND hWnd = wnd.CreateNative("test window", WS_POPUP, 0, 0, 0, 0, 0, 0);
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

int run_app(HINSTANCE hInst){
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFac;
    comMgr.CreateRender_GDI((IObjRef**)&renderFac);
    SAutoRefPtr<IImgDecoderFactory> imgDecoder;
    comMgr.CreateImgDecoder((IObjRef**)&imgDecoder);
    renderFac->SetImgDecoderFactory(imgDecoder);
    
    SApplication app(renderFac,hInst);
    SouiFactory sfac;
    SAutoRefPtr<IResProvider> sysResouce(sfac.CreateResProvider(RES_FILE));
    sysResouce->Init((LPARAM)kPath_SysRes, 0);
    app.LoadSystemNamedResource(sysResouce);
    SAutoRefPtr<IResProvider> testResouce(sfac.CreateResProvider(RES_FILE));
    testResouce->Init((LPARAM)kPath_TestRes, 0);
    app.AddResProvider(testResouce);

    SHostWnd hostWnd("layout:XML_MAINWND");
    hostWnd.Create(0);
    hostWnd.ShowWindow(SW_SHOW);
    app.Run(hostWnd.m_hWnd);
    
    return 0;
}

TEST(demo,app){
    HINSTANCE hInst = 0;
    EXPECT_EQ(run_app(hInst),0);
}