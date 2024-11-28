#include <stdio.h>
#include <tchar.h>

#pragma warning(disable:4800)
#pragma warning(disable:4996)

#include <gtest/gtest.h>
#include <xml/SXml.h>
#include <string/sstringa.h>
#include <string/sstringw.h>
#include <string/strcpcvt.h>
#include <helper/SRwLock.h>
#include <core/SNativeWnd.h>
#include <core/SMsgLoop.h>
#include <wtl.mini/souimisc.h>
#define _WTYPES_NS SOUI

#include <wtl.mini/msgcrack.h>
#include <commgr2.h>
#include <interface/SRender-i.h>
#include <string>

#include <souistd.h>
#include <SouiFactory.h>
#include <helper/SSemaphore.h>
#include "common.h"

#if defined(__linux__) || _MSC_VER >= 1700 // VS2012
#include <functional>
#include <thread>
#endif

#ifndef _WIN32
#include <cairo/cairo-xcb.h>
#include <xcb/xcb_aux.h>
#endif//_WIN32
#include "../components/resprovider-zip/zipresprovider-param.h"
#include <helper/SFunctor.hpp>
#include <helper/SMenu.h>
#include <helper/SMenuEx.h>
#include <Scintilla.h>

using namespace SOUI;


#define kLogTag "fun_test"

static const TCHAR * kPath_TestPng =_T("/demo/uires/image/soui.png");
static const TCHAR * kPath_TestXml =_T("/demo2/uires/xml/dlg_main.xml");
static const TCHAR* kPath_TestYyLogo = _T("/fun_test/uires/image/yy.png");

TEST(sowinapi, global_0){
    HGLOBAL h = GlobalAlloc(0,100);
    char * ptr = (char*)GlobalLock(h);
    for(int i=0;i<100;i++){
        ptr[i]=i;
    }
    GlobalUnlock(h);
    GlobalFree(h);
}

TEST(sowinapi, global_moveable){
    HGLOBAL h = GlobalAlloc(LMEM_MOVEABLE,100);
    char * ptr = (char*)GlobalLock(h);
    for(int i=0;i<100;i++){
        ptr[i]=i;
    }
    GlobalUnlock(h);
    GlobalFree(h);
}

TEST(sowinapi, badptr){
    char * p = new char[100];
    EXPECT_FALSE(IsBadReadPtr(p,100));
    //EXPECT_TRUE(IsBadWritePtr((int*)0x12345678,100));//as debugger will catch SIGSEGV at first, don't open this line for debug mode.
    delete []p;
}

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

TEST(soui, semaphore) {
    SSemaphore semaphore;
    semaphore.notify();
}

class TaskHost {
public:
    TaskHost(int id_) :id(id_) {}
    void task0() {
        SSLOGI() << "task0 done,id="<<id;
    }

    void task1(int p1) {
        SSLOGI() << "task0 done,id=" << id<< " p1="<<p1;
    }

    void task2(int p1,std::string & p2) {
        SSLOGI() << "task0 done,id=" << id << " p1=" << p1<<" p2="<<p2.c_str();
    }
private:
    int id;
};
TEST(soui, taskloop) {
    SComMgr2 comMgr;
    SAutoRefPtr<ITaskLoop> taskLoop;
    comMgr.CreateTaskLoop((IObjRef**)&taskLoop);
    EXPECT_TRUE(taskLoop);
    if (taskLoop) {
        TaskHost host(100);
        taskLoop->start("test_loop", Normal);
        STaskHelper::post(taskLoop, &host, &TaskHost::task0,false);
        STaskHelper::post(taskLoop, &host, &TaskHost::task1,200, false);
        //wait the last task done, then quit
        STaskHelper::post(taskLoop, &host, &TaskHost::task2, 300,std::string("work task"), true); 
        taskLoop->stop();       
    }
}

TEST(file, createfile){
	std::tstring srcDir = getSourceDir();
    std::tstring strZip = srcDir + _T("/fun_test/uires.zip");
    HANDLE hFile = CreateFile(strZip.c_str(),GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,0,0);
    EXPECT_TRUE(hFile != INVALID_HANDLE_VALUE);
    if(hFile != INVALID_HANDLE_VALUE){
        char szBuf[100];
        DWORD readed=0;
        BOOL bRead=ReadFile(hFile,szBuf,100,&readed,0);
        EXPECT_TRUE(bRead);
        CloseHandle(hFile);
    }
}

TEST(com, resprovider_zip) {
	std::tstring srcDir = getSourceDir();
	SComMgr2 comMgr;
	SAutoRefPtr<IResProvider> zipProvider;
	BOOL bLoad=comMgr.CreateResProvider_ZIP((IObjRef**)&zipProvider);
    EXPECT_TRUE(bLoad);
    if (bLoad) {
        ZIPRES_PARAM param;
        std::tstring strZip = srcDir + _T("/fun_test/uires.zip");
        ZipFile(&param, NULL, strZip.c_str(), "souizip");
        BOOL initDone = zipProvider->Init((WPARAM)&param,0);
        EXPECT_TRUE(initDone);
        if (initDone) {
            size_t szBuf = zipProvider->GetRawBufferSize(NULL, _T("uires.idx"));
            EXPECT_TRUE(szBuf > 0);
            SAutoBuf buf;
            char* pBuf = buf.Allocate(szBuf);
            zipProvider->GetRawBuffer(NULL, _T("uires.idx"), pBuf, szBuf);
            SXmlDoc xmlDoc;
            BOOL parseXml = xmlDoc.LoadBuffer(pBuf, szBuf, 0, enc_utf8);
            EXPECT_TRUE(parseXml);
            HICON hIcon = zipProvider->LoadIcon(_T("ICON_LOGO"), 64, 64);
            EXPECT_TRUE(hIcon);
            if (hIcon) {
                DestroyIcon(hIcon);
            }
        }
    }
}


#define evt_name1 _T("/tmp/hello_event_soui1")
#define evt_name2 _T("/tmp/hello_event_soui2")

void thread_fun() {
    HANDLE evt = CreateEvent(NULL, FALSE, FALSE, evt_name1);
    HANDLE evt2 = CreateEvent(NULL, FALSE, FALSE, evt_name2);
    Sleep(300);
    printf("wakeup WaitForSingleObject,ts=%u\n", GetTickCount());
    SetEvent(evt);
    
    Sleep(300);
    printf("wakeup WaitForMultipleObjects1 wait all, ts=%u\n",GetTickCount());
    SetEvent(evt);
    Sleep(200);
    printf("wakeup WaitForMultipleObjects2 wait all, ts=%u\n", GetTickCount());
    SetEvent(evt2);

    Sleep(100);
    printf("wakeup WaitForMultipleObjects2 wait any, ts=%u\n", GetTickCount());
    SetEvent(evt2);

    CloseHandle(evt);
    CloseHandle(evt2);
}

#if defined(__linux__) || _MSC_VER >= 1700 // VS2012
TEST(Sycn, Event) {
    if(1){
        HANDLE evt = CreateEvent(NULL,FALSE, FALSE, evt_name1);
        HANDLE evt2 = CreateEvent(NULL, FALSE, FALSE, evt_name2);
        HANDLE evts[] = { evt,evt2 };
        std::thread trd(thread_fun);
        int ret = WaitForSingleObject(evts[0], 1000);
        printf("WaitForSingleObject ret=%d,ts=%u\n", ret,GetTickCount());
        EXPECT_EQ(ret, WAIT_OBJECT_0);

        ret = WaitForMultipleObjects(2,evts, TRUE,5000);
        printf("WaitForMultipleObjects wait all ret=%d,ts=%u\n", ret, GetTickCount());
        EXPECT_EQ(ret, WAIT_OBJECT_0);

        ret = WaitForMultipleObjects(2, evts, FALSE, 5000);
        printf("WaitForMultipleObjects wait any ret=%d,ts=%u\n", ret, GetTickCount());
        EXPECT_EQ(ret, WAIT_OBJECT_0+1);

        trd.join();

        CloseHandle(evt);
        CloseHandle(evt2);
    }
}
#endif

TEST(Util,sstringa){
    SStringA str=("test");
    SStringA str2 =("soui");
    SStringA str3 = str+("/")+("soui");
    EXPECT_TRUE(str3==("test/soui"));
    SStringA str4 = SStringA().Format("test %d", 100);
    EXPECT_TRUE(str4 == ("test 100"));
}



TEST(Util, sstringw) {
    SStringW str = L"test";
    SStringW str2 = L"soui";
    SStringW str3 = str + L"/"+ L"soui";
    EXPECT_TRUE(str3 == L"test/soui");

    SStringW str4 = SStringW().Format(L"test %s", L"fuck");
    EXPECT_TRUE(str4 == L"test fuck");
}

TEST(Util,png_load){
    std::tstring srcDir = getSourceDir();
    HBITMAP bmp = LoadPng((srcDir+kPath_TestPng).c_str());
    EXPECT_TRUE(bmp!=0);
    if(bmp){
        DeleteObject(bmp);
    }
}

TEST(Util,Xml_Load){
    std::tstring srcDir = getSourceDir();
    SXmlDoc xml;
    EXPECT_TRUE(xml.load_file((srcDir+kPath_TestXml).c_str()));
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
    SStringW wstr = S_CA2W(src,CP_ACP);
    
    SStringA src2 = S_CW2A(wstr,CP_ACP);
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

TEST(Util, LoadIconMem) {
    std::tstring path = getSourceDir() + _T("/fun_test/uires/image/soui.ico");
    HICON icon1 = (HICON)LoadImage(0, path.c_str(), IMAGE_ICON, 32, 32, LR_DEFAULTSIZE|LR_LOADFROMFILE);
    EXPECT_TRUE(icon1 != 0);
    if (icon1) {
        DestroyIcon(icon1);
    }
    FILE* f = _tfopen(path.c_str(), _T("rb"));
    if (f) {
        fseek(f, 0, SEEK_END);
        int len = ftell(f);
        rewind(f);
        std::vector<BYTE> buf(len);
        fread(&buf[0], 1, len, f);
        HICON hIcon = (HICON)LoadIconFromMemory(&buf[0], buf.size(), TRUE, 32, 32, LR_DEFAULTSIZE);
        EXPECT_TRUE(hIcon != 0);
        if(hIcon) DestroyIcon(hIcon);
    }
}

TEST(memory, handle) {
    HLOCAL hData = LocalAlloc(LMEM_MOVEABLE| LMEM_ZEROINIT, 100);
    void* p = LocalLock(hData);
    HLOCAL htest = LocalHandle(p);
    EXPECT_EQ(hData, htest);
    LocalFree(hData);
}

int run_window();
TEST(demo,window){
//    EXPECT_EQ(run_window(), 1);
}

int run_app(HINSTANCE hInst);
TEST(demo,app){
    EXPECT_EQ(run_app(0),0);
}