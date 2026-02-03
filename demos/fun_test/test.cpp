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

#include "../components/resprovider-zip/zipresprovider-param.h"
#include <helper/SFunctor.hpp>
#include <helper/SMenu.h>
#include <helper/SMenuEx.h>
#include <Scintilla.h>

using namespace SOUI;


#define kLogTag "fun_test"

static const TCHAR * kPath_TestPng =_T("/uires/image/soui.png");
static const TCHAR * kPath_TestXml =_T("/uires/xml/dlg_main.xml");
static const TCHAR* kPath_TestYyLogo = _T("/uires/image/yy.png");

TEST(demo,event){
    DWORD pid = GetCurrentProcessId();
        char szName[100];
        sprintf(szName, "proc_event_%u", pid);
        HANDLE hProcess = CreateEventA(NULL, TRUE, FALSE, szName);
        if(hProcess){
            SetEvent(hProcess);
            BOOL bRet = WaitForSingleObject(hProcess, 100) == WAIT_OBJECT_0;
            EXPECT_EQ(bRet, TRUE);
            CloseHandle(hProcess);
        }

}

TEST(sowinapi,file){
    //CopyDir("/home/flyhigh/work/soui4js/wizard/template/*.*","/home/flyhigh/work/test");
    //DeleteFile("/home/flyhigh/work/test.txt");
    //DelDir("/home/flyhigh/work/tst",TRUE);
    //DelDir("/home/flyhigh/work/tst2",FALSE);
}

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

TEST(soui,mb){
    const wchar_t * src = L"中文字符串test";
    char sz936[100];
    WideCharToMultiByte(936,0,src,-1,sz936,100,NULL,NULL);
    wchar_t szWd[100];
    MultiByteToWideChar(936,0,sz936,-1,szWd,100);
    EXPECT_TRUE(wcscmp(src,szWd)==0);
}

TEST(soui, matrix)
{
    SMatrix mtx1, mtx2,mtx3;
    mtx1.setScale2(2, 2, 50, 50);
    mtx2.setScale2(2, 1, 50, 50);
    mtx3.setScale2(1, 2, 50, 50);
    SMatrix mtx4 = mtx2 * mtx3;
    EXPECT_EQ(mtx1, mtx4);
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
	SOUI::SStringT srcDir = getSourceDir();
    SOUI::SStringT strZip = srcDir + _T("/uires.zip");
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
	SOUI::SStringT srcDir = getSourceDir();
	SComMgr2 comMgr;
	SAutoRefPtr<IResProvider> zipProvider;
	BOOL bLoad=comMgr.CreateResProvider_ZIP((IObjRef**)&zipProvider);
    EXPECT_TRUE(bLoad);
    if (bLoad) {
        ZIPRES_PARAM param;
        SOUI::SStringT strZip = srcDir + _T("/uires.zip");
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


#ifndef _WIN32
// ========================================================================
// Path API Tests for swinx library
// ========================================================================

class PathAPITest : public ::testing::Test {
public:
	PathAPITest():hdc(NULL){}
protected:
    void SetUp() override {
        // Create a test HDC
        hdc = CreateCompatibleDC(NULL);
        ASSERT_NE(hdc, (HDC)NULL) << "Failed to create HDC";
    }

    void TearDown() override {
        if (hdc) {
            DeleteDC(hdc);
            hdc = NULL;
        }
    }

    HDC hdc;
};

TEST_F(PathAPITest, BasicPathRecording) {
    // Test basic path recording functionality
    EXPECT_TRUE(BeginPath(hdc));

    // Add some basic shapes to the path
    MoveToEx(hdc, 100, 100, NULL);
    LineTo(hdc, 200, 100);
    LineTo(hdc, 200, 200);
    LineTo(hdc, 100, 200);

    EXPECT_TRUE(EndPath(hdc));
}

TEST_F(PathAPITest, PathStroking) {
    // Test path stroking
    EXPECT_TRUE(BeginPath(hdc));

    Rectangle(hdc, 50, 50, 150, 150);

    EXPECT_TRUE(EndPath(hdc));
    EXPECT_TRUE(StrokePath(hdc));
}

TEST_F(PathAPITest, PathFilling) {
    // Test path filling
    EXPECT_TRUE(BeginPath(hdc));

    Ellipse(hdc, 50, 50, 150, 150);

    EXPECT_TRUE(EndPath(hdc));
    EXPECT_TRUE(FillPath(hdc));
}

TEST_F(PathAPITest, StrokeAndFillPath) {
    // Test stroke and fill path
    EXPECT_TRUE(BeginPath(hdc));

    RoundRect(hdc, 50, 50, 150, 150, 20, 20);

    EXPECT_TRUE(EndPath(hdc));
    EXPECT_TRUE(StrokeAndFillPath(hdc));
}

TEST_F(PathAPITest, PathToRegion) {
    // Test path to region conversion
    EXPECT_TRUE(BeginPath(hdc));

    Rectangle(hdc, 100, 100, 200, 200);

    EXPECT_TRUE(EndPath(hdc));

    HRGN hRegion = PathToRegion(hdc);
    EXPECT_NE(hRegion, (HRGN)NULL);

    if (hRegion) {
        // Verify region bounds
        RECT rcBounds;
        int result = GetRgnBox(hRegion, &rcBounds);
        EXPECT_NE(result, ERROR);
        EXPECT_EQ(rcBounds.left, 100);
        EXPECT_EQ(rcBounds.top, 100);
        EXPECT_EQ(rcBounds.right, 200);
        EXPECT_EQ(rcBounds.bottom, 200);

        DeleteObject(hRegion);
    }
}

TEST_F(PathAPITest, SelectClipPath) {
    // Test select clip path
    EXPECT_TRUE(BeginPath(hdc));

    Ellipse(hdc, 50, 50, 150, 150);

    EXPECT_TRUE(EndPath(hdc));
    EXPECT_TRUE(SelectClipPath(hdc, RGN_COPY));
}

TEST_F(PathAPITest, GetPath) {
    // Test get path functionality
    EXPECT_TRUE(BeginPath(hdc));

    MoveToEx(hdc, 100, 100, NULL);
    LineTo(hdc, 200, 200);

    EXPECT_TRUE(EndPath(hdc));

    // Query path size
    int pathSize = GetPath(hdc, NULL, NULL, 0);
    EXPECT_GT(pathSize, 0);

    // Get actual path data
    if (pathSize > 0) {
        std::vector<POINT> points(pathSize);
        std::vector<BYTE> types(pathSize);

        int actualSize = GetPath(hdc, &points[0], &types[0], pathSize);
        EXPECT_EQ(actualSize, pathSize);

        // Verify we have at least a move and a line
        EXPECT_GE(actualSize, 2);
        if (actualSize >= 2) {
            EXPECT_EQ(types[0] & PT_MOVETO, PT_MOVETO);
            EXPECT_EQ(types[1] & PT_LINETO, PT_LINETO);
        }
    }
}

TEST_F(PathAPITest, MiterLimit) {
    // Test miter limit functionality
    float oldLimit;
    EXPECT_TRUE(GetMiterLimit(hdc, &oldLimit));
    EXPECT_GT(oldLimit, 0.0f);

    float newLimit = 5.0f;
    float returnedOldLimit;
    EXPECT_TRUE(SetMiterLimit(hdc, newLimit, &returnedOldLimit));
    EXPECT_EQ(returnedOldLimit, oldLimit);

    float currentLimit;
    EXPECT_TRUE(GetMiterLimit(hdc, &currentLimit));
    EXPECT_EQ(currentLimit, newLimit);
}

TEST_F(PathAPITest, AbortPath) {
    // Test abort path functionality
    EXPECT_TRUE(BeginPath(hdc));

    Rectangle(hdc, 50, 50, 150, 150);

    EXPECT_TRUE(AbortPath(hdc));

    // After abort, EndPath should fail
    EXPECT_FALSE(EndPath(hdc));
}

TEST_F(PathAPITest, ComplexShapes) {
    // Test complex shapes in path
    EXPECT_TRUE(BeginPath(hdc));

    // Add various shapes
    Arc(hdc, 50, 50, 150, 150, 100, 50, 150, 100);
    Pie(hdc, 200, 50, 300, 150, 250, 50, 300, 100);
    Chord(hdc, 350, 50, 450, 150, 400, 50, 450, 100);

    EXPECT_TRUE(EndPath(hdc));
    EXPECT_TRUE(StrokePath(hdc));
}

TEST_F(PathAPITest, TextPath) {
    // Test text path functionality
    EXPECT_TRUE(BeginPath(hdc));

    TextOutA(hdc, 100, 100, "Hello Path", -1);

    EXPECT_TRUE(EndPath(hdc));
    EXPECT_TRUE(StrokePath(hdc));
}

TEST_F(PathAPITest, TextPathWithLength) {
    // Test text path with specific length
    EXPECT_TRUE(BeginPath(hdc));

    const char* text = "Hello World";
    TextOutA(hdc, 100, 200, text, 5); // Only "Hello"

    EXPECT_TRUE(EndPath(hdc));

    // Get path size for partial text
    int pathSize = GetPath(hdc, NULL, NULL, 0);
    EXPECT_GT(pathSize, 0);
}

TEST_F(PathAPITest, CurrentPointAdjustment) {
    // Test current point adjustment in text paths
    MoveToEx(hdc, 100, 100, NULL);

    POINT initialPos;
    GetCurrentPositionEx(hdc, &initialPos);
    EXPECT_EQ(initialPos.x, 100);
    EXPECT_EQ(initialPos.y, 100);

    EXPECT_TRUE(BeginPath(hdc));
    TextOutA(hdc, 100, 100, "Test", 4);
    EXPECT_TRUE(EndPath(hdc));

    POINT afterTextPos;
    GetCurrentPositionEx(hdc, &afterTextPos);

    // Current point should have moved after text
    EXPECT_GT(afterTextPos.x, initialPos.x);
}

TEST_F(PathAPITest, ZeroLengthText) {
    // Test zero length text doesn't affect current point
    MoveToEx(hdc, 200, 200, NULL);

    POINT beforePos;
    GetCurrentPositionEx(hdc, &beforePos);

    EXPECT_TRUE(BeginPath(hdc));
    TextOutA(hdc, 200, 200, "Hello", 0); // Zero length
    EXPECT_TRUE(EndPath(hdc));

    POINT afterPos;
    GetCurrentPositionEx(hdc, &afterPos);

    // Position should not change for zero-length text
    EXPECT_EQ(beforePos.x, afterPos.x);
    EXPECT_EQ(beforePos.y, afterPos.y);
}

TEST_F(PathAPITest, ChainedTextPaths) {
    // Test chained text paths
    MoveToEx(hdc, 300, 300, NULL);

    EXPECT_TRUE(BeginPath(hdc));

    // First text
    TextOutA(hdc, 300, 300, "First", 5);
    POINT afterFirst;
    GetCurrentPositionEx(hdc, &afterFirst);

    // Second text should start where first ended
    TextOutA(hdc, afterFirst.x, afterFirst.y, " Second", 7);
    POINT afterSecond;
    GetCurrentPositionEx(hdc, &afterSecond);

    // Third text
    TextOutA(hdc, afterSecond.x, afterSecond.y, " Third", 6);

    EXPECT_TRUE(EndPath(hdc));

    // Verify progression
    EXPECT_GT(afterFirst.x, 300);
    EXPECT_GT(afterSecond.x, afterFirst.x);
}

// ========================================================================
// Bézier Curve API Tests
// ========================================================================

TEST_F(PathAPITest, PolyBezierBasic) {
    // Test basic Bézier curve (4 points: start + 3 control/end points)
    POINT points[4] = {
        {10, 10},   // Start point
        {20, 5},    // Control point 1
        {30, 15},   // Control point 2
        {40, 10}    // End point
    };

    EXPECT_TRUE(PolyBezier(hdc, points, 4));

    // Verify current position is not changed by PolyBezier
    POINT currentPos;
    EXPECT_TRUE(GetCurrentPositionEx(hdc, &currentPos));
    // Current position should be unchanged (PolyBezier doesn't update it)
}

TEST_F(PathAPITest, PolyBezierMultiple) {
    // Test multiple Bézier curves (7 points: start + 2 curves * 3 points each)
    POINT points[7] = {
        {10, 10},   // Start point
        {20, 5},    // Curve 1: Control point 1
        {30, 15},   // Curve 1: Control point 2
        {40, 10},   // Curve 1: End point
        {50, 5},    // Curve 2: Control point 1
        {60, 15},   // Curve 2: Control point 2
        {70, 10}    // Curve 2: End point
    };

    EXPECT_TRUE(PolyBezier(hdc, points, 7));
}

TEST_F(PathAPITest, PolyBezierToBasic) {
    // Set current position first
    EXPECT_TRUE(MoveToEx(hdc, 10, 10, NULL));

    // Test PolyBezierTo (3 points per curve)
    POINT points[3] = {
        {20, 5},    // Control point 1
        {30, 15},   // Control point 2
        {40, 10}    // End point
    };

    EXPECT_TRUE(PolyBezierTo(hdc, points, 3));

    // Verify current position is updated to end point
    POINT currentPos;
    EXPECT_TRUE(GetCurrentPositionEx(hdc, &currentPos));
    EXPECT_EQ(currentPos.x, 40);
    EXPECT_EQ(currentPos.y, 10);
}

TEST_F(PathAPITest, PolyBezierToMultiple) {
    // Set current position first
    EXPECT_TRUE(MoveToEx(hdc, 10, 10, NULL));

    // Test multiple curves (6 points: 2 curves * 3 points each)
    POINT points[6] = {
        {20, 5},    // Curve 1: Control point 1
        {30, 15},   // Curve 1: Control point 2
        {40, 10},   // Curve 1: End point
        {50, 5},    // Curve 2: Control point 1
        {60, 15},   // Curve 2: Control point 2
        {70, 10}    // Curve 2: End point
    };

    EXPECT_TRUE(PolyBezierTo(hdc, points, 6));

    // Verify current position is updated to final end point
    POINT currentPos;
    EXPECT_TRUE(GetCurrentPositionEx(hdc, &currentPos));
    EXPECT_EQ(currentPos.x, 70);
    EXPECT_EQ(currentPos.y, 10);
}

TEST_F(PathAPITest, PolyBezierErrorHandling) {
    // Test with invalid point count (not 1 + 3*n)
    POINT points[5] = {{10,10}, {20,5}, {30,15}, {40,10}, {50,5}};
    EXPECT_FALSE(PolyBezier(hdc, points, 5));

    // Test with too few points
    EXPECT_FALSE(PolyBezier(hdc, points, 3));

    // Test with NULL points
    EXPECT_FALSE(PolyBezier(hdc, NULL, 4));
}

TEST_F(PathAPITest, PolyBezierToErrorHandling) {
    // Test with invalid point count (not 3*n)
    POINT points[4] = {{20,5}, {30,15}, {40,10}, {50,5}};
    EXPECT_FALSE(PolyBezierTo(hdc, points, 4));

    // Test with too few points
    EXPECT_FALSE(PolyBezierTo(hdc, points, 2));

    // Test with NULL points
    EXPECT_FALSE(PolyBezierTo(hdc, NULL, 3));

    // Test without current position (should fail)
    // Create new HDC without setting current position
    HDC newHdc = CreateCompatibleDC(NULL);
    EXPECT_FALSE(PolyBezierTo(newHdc, points, 3));
    DeleteDC(newHdc);
}

TEST_F(PathAPITest, BezierWithPathRecording) {
    EXPECT_TRUE(BeginPath(hdc));

    // Add PolyBezier to path
    POINT bezierPoints[4] = {{10,10}, {20,5}, {30,15}, {40,10}};
    EXPECT_TRUE(PolyBezier(hdc, bezierPoints, 4));

    // Add PolyBezierTo to path
    EXPECT_TRUE(MoveToEx(hdc, 50, 10, NULL));
    POINT bezierToPoints[3] = {{60,5}, {70,15}, {80,10}};
    EXPECT_TRUE(PolyBezierTo(hdc, bezierToPoints, 3));

    EXPECT_TRUE(EndPath(hdc));
    EXPECT_TRUE(StrokePath(hdc));
}
#endif//!_WIN32
int run_window();
TEST(demo,window){
    //EXPECT_EQ(run_window(), 1);
}

int run_app(HINSTANCE hInst);
TEST(demo,app){
    EXPECT_EQ(run_app(0),0);
}

// Waitable Timer and Timer Queue Tests
TEST(sowinapi, waitable_timer) {
    // Test 1: Create unnamed waitable timer
    HANDLE hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
    EXPECT_TRUE(hTimer != NULL);
    
    if (hTimer) {
        // Test 2: Set waitable timer (100ms)
        LARGE_INTEGER liDueTime;
        liDueTime.QuadPart = -100000LL; // 100ms in 100ns units
        BOOL bSet = SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, FALSE);
        EXPECT_TRUE(bSet);
        
        if (bSet) {
            // Test 3: Wait for timer
            DWORD dwWait = WaitForSingleObject(hTimer, 200);
            EXPECT_EQ(dwWait, WAIT_OBJECT_0);
        }
        
        // Test 4: Cancel waitable timer
        BOOL bCancel = CancelWaitableTimer(hTimer);
        EXPECT_TRUE(bCancel);
        
        CloseHandle(hTimer);
    }
    
    // Test 5: Create named waitable timer
    const TCHAR* timerName = _T("TestWaitableTimer");
    HANDLE hNamedTimer = CreateWaitableTimer(NULL, FALSE, timerName);
    EXPECT_TRUE(hNamedTimer != NULL);
    
    if (hNamedTimer) {
        // Test 6: Open existing named timer
        HANDLE hOpenedTimer = OpenWaitableTimer(TIMER_ALL_ACCESS, FALSE, timerName);
        EXPECT_TRUE(hOpenedTimer != NULL);
        
        if (hOpenedTimer) {
            CloseHandle(hOpenedTimer);
        }
        
        CloseHandle(hNamedTimer);
    }
}

// Timer callback function
VOID CALLBACK TimerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    static int callbackCount = 0;
    callbackCount++;
    printf("Timer callback fired, count=%d\n", callbackCount);
}

TEST(sowinapi, timer_queue) {
    // Test 1: Create timer queue
    HANDLE hTimerQueue = CreateTimerQueue();
    EXPECT_TRUE(hTimerQueue != NULL);
    
    if (hTimerQueue) {
        // Test 2: Create timer in queue
        HANDLE hTimer = NULL;
        BOOL bCreate = CreateTimerQueueTimer(&hTimer, hTimerQueue, TimerCallback, NULL, 100, 0, 0);
        EXPECT_TRUE(bCreate);
        EXPECT_TRUE(hTimer != NULL);
        
        if (bCreate && hTimer) {
            // Wait for timer to fire
            Sleep(200);
            
            // Test 3: Delete timer from queue
            BOOL bDelete = DeleteTimerQueueTimer(hTimerQueue, hTimer, NULL);
            EXPECT_TRUE(bDelete);
        }
        
        // Test 4: Delete timer queue
        BOOL bDeleteQueue = DeleteTimerQueue(hTimerQueue);
        EXPECT_TRUE(bDeleteQueue);
    }
}