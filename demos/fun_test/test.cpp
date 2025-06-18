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
int run_window();
TEST(demo,window){
    //EXPECT_EQ(run_window(), 1);
}

int run_app(HINSTANCE hInst);
TEST(demo,app){
    EXPECT_EQ(run_app(0),0);
}