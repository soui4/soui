﻿#include <souistd.h>
#include <helper/SMenu.h>
#include <helper/SMenuEx.h>
#include <Scintilla.h>
#include <commdlg.h>
#include <resprovider-zip/zipresprovider-param.h>
#include "common.h"
#include "ScintillaWnd.h"

using namespace SOUI;
#define kLogTag "test_hostwnd"
#define SYS_MENUID_TEST 1000

void RunMenu(POINT pt) {
    HMENU hMenu = CreatePopupMenu();
    if (hMenu)
    {
        for (int i = 0; i < 6; i++)
            InsertMenu(hMenu, -1, MF_BYPOSITION | (i == 1 ? MF_GRAYED : MFT_RADIOCHECK), 10 + i, SStringT().Format(_T("menu_%d"), i).c_str());

        MENUITEMINFO info;
        info.fMask = MIIM_SUBMENU;
        info.hSubMenu = CreatePopupMenu();
        for (int i = 0; i < 6; i++)
            InsertMenu(info.hSubMenu, -1, MF_BYPOSITION | MF_CHECKED, 20 + i, SStringT().Format(_T("child menu_1_%d"), i).c_str());
        SetMenuItemInfo(hMenu, 0, TRUE, &info);

        info.hSubMenu = CreatePopupMenu();
        for (int i = 0; i < 6; i++)
            InsertMenu(info.hSubMenu, -1, MF_BYPOSITION, 30 + i, SStringT().Format(_T("child menu_2_%d"), i).c_str());

        MENUITEMINFO info2;
        info2.fMask = MIIM_SUBMENU;
        info2.hSubMenu = CreatePopupMenu();
        for (int i = 0; i < 6; i++)
            InsertMenu(info2.hSubMenu, -1, MF_BYPOSITION, 40 + i, SStringT().Format(_T("child menu_3_%d"), i).c_str());
        SetMenuItemInfo(info.hSubMenu, 4, TRUE, &info2);

        SetMenuItemInfo(hMenu, 3, TRUE, &info);

        TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, GetActiveWindow(), NULL);

        TPMPARAMS params = { sizeof(TPMPARAMS), { 300, 300, 400, 400 } };

        //TrackPopupMenuEx(hMenu, 0, 300, 300, GetActiveWindow(),&params);

        DestroyMenu(hMenu);
    }
}

class CSouiWnd : public SHostWnd{
public:
    CSouiWnd(): SHostWnd("layout:xml_soui"){}

    void OnFinalMessage(HWND hWnd) override{
        SHostWnd::OnFinalMessage(hWnd);
        delete this;
    }
};

class CMainDlg : public SHostWnd {
    CScintillaWnd* m_pSciter;
public:
    CMainDlg(LPCSTR pszLayout) :SHostWnd(pszLayout), m_pSciter(NULL){}

    void OnClose();
    void OnBtnResize() {
        SetTimer(10, 10);
    }

    void OnBtnMenu() {
        CRect rcBtn = FindChildByName("btn_menu")->GetWindowRect();
        CPoint pt(rcBtn.right, rcBtn.top);
        ClientToScreen(&pt);
        pt.x += 10;
        RunMenu(pt);
    }
    void OnBtnSMenu() {
        CRect rcBtn = FindChildByName("btn_smenu")->GetWindowRect();
        CPoint pt(rcBtn.right, rcBtn.top);
        ClientToScreen(&pt);
        pt.x += 10;
        // ʹ���Ի�˵�
        SMenu menu;
        menu.LoadMenu(_T("LAYOUT:XML_MENUTEST"));
        menu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
    }

    void OnBtnSMenuEx() {
        CRect rcBtn = FindChildByName("btn_smenuex")->GetWindowRect();
        CPoint pt(rcBtn.right, rcBtn.top);
        ClientToScreen(&pt);
        pt.x += 10;
        // ʹ���Ի�˵�
        SMenuEx menu;
        menu.LoadMenu(_T("LAYOUT:XML_MENUEXTEST"));
        menu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
    }


    void OnRealWndCreate(IEvtArgs* e) {
        SRealWnd* pRealWnd = sobj_cast<SRealWnd>(e->Sender());
        EventRealWndCreate* e2 = sobj_cast<EventRealWndCreate>(e);
        CScintillaWnd* pWnd = new CScintillaWnd();
        BOOL bOK = pWnd->Create(pRealWnd->GetRealWindowName()->c_str(), WS_CHILD, CRect(0, 0, 0, 0), m_hWnd, pRealWnd->GetID(), SApplication::getSingleton().GetModule());
        if (!bOK)
        {
            SASSERT(FALSE);
            delete pWnd;
            return ;
        }
        pRealWnd->SetData(pWnd);
        e2->hRet = pWnd->m_hWnd;
    }

    void OnRealWndDestroy(IEvtArgs* e) {
        SRealWnd* pRealWnd = sobj_cast<SRealWnd>(e->Sender());
        CScintillaWnd* pWnd = (CScintillaWnd*)pRealWnd->GetData();
        if (pWnd)
        {
            pWnd->DestroyWindow();
            delete pWnd;
        }
    }

    void OnRealWndInit(IEvtArgs* e) {
        SRealWnd* pRealWnd = sobj_cast<SRealWnd>(e->Sender());
        CScintillaWnd *pWnd = (CScintillaWnd*)pRealWnd->GetData();

    }
    void OnInit(IEvtArgs* e) {
        SRealWnd* pRealWnd = FindChildByName2<SRealWnd>("wnd_sci");
        if (pRealWnd) {
            m_pSciter = (CScintillaWnd*)pRealWnd->GetData();
            SASSERT(m_pSciter);
            m_pSciter->SendMessage(SCI_USEPOPUP, 0, 0);
            //load xml
            SAutoBuf buf;
            SApplication::getSingletonPtr()->LoadRawBuffer(_T("layout"), _T("XML_MAINWND"), NULL, buf);
            m_pSciter->SendMessage(SCI_SETTEXT, buf.size(), (LPARAM)(char*)buf);
        }
        SWindow* edit = FindChildByName("edit_test");
        if (edit) {
            ::RegisterDragDrop(m_hWnd, GetDropTarget());
        }
    }

    void OnShowMsgbox(){ 
        MessageBox(m_hWnd, _T("TEST MSGBOX!!!"), _T("test msgbox"),MB_OK);
    }

    void OnBtnSoui(){
        CSouiWnd * pwnd = new CSouiWnd;
        CRect rc = GetWindowRect();
        pwnd->CreateEx(m_hWnd,WS_POPUP|WS_VISIBLE,WS_EX_LAYERED,rc.right+5,rc.top,0,0);
    }
    void OnBtnSaveRtf(){
        SRichEdit *pEdit = FindChildByName2<SRichEdit>("re_test");
        if(pEdit){
            SOUI::SStringT srcDir = getSourceDir() + _T("/uires/rtf/test.rtf");
            pEdit->SaveRtf(srcDir.c_str());
        }else{
            SLOGE()<<"find re_test as richedit failed!";
        }
    }
    void OnBtnPickColor(){
        static COLORREF acrCustClr[16]; // array of custom colors
        CHOOSECOLOR cc;                 // common dialog box structure 
        // Initialize CHOOSECOLOR 
        ZeroMemory(&cc, sizeof(cc));
        cc.lStructSize = sizeof(cc);
        cc.hwndOwner = m_hWnd;
        cc.lpCustColors = (LPDWORD)acrCustClr;
        cc.rgbResult = RGBA(255,0,0,255);
        cc.Flags = CC_FULLOPEN | CC_RGBINIT;
        if(ChooseColor(&cc)){
            #ifdef _WIN32
            cc.rgbResult |= 0xff000000;
            #endif
            SStringA str=SStringA().Format("#%02x%02x%02x%02x",
            GetRValue(cc.rgbResult),
            GetGValue(cc.rgbResult),
            GetBValue(cc.rgbResult),
            GetAValue(cc.rgbResult));
            SWindow *pWnd = FindChildByName("wnd_color");
            if(pWnd) 
                pWnd->SetAttributeA("colorBkgnd",str,FALSE);
            SWindow *pTxt = FindChildByName("txt_color");
            if(pTxt)
                pTxt->SetWindowText(S_CA2T(str));
        }
    }
    void OnBtnOpenFile()
    {
        TCHAR szBuf[MAX_PATH];
        OPENFILENAME ofn = { 0 };
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_hWnd;
        ofn.lpstrFile = szBuf;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = _T("Select Xml File");
        ofn.lpstrDefExt = _T("xml");
        ofn.lpstrFilter = _T("Xml Files\0*.xml\0");
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;
        if(GetOpenFileName(&ofn))
        {
            SWindow *pWnd = FindChildByName("txt_open_file");
            if(pWnd) 
                pWnd->SetWindowText(szBuf);
        }
    }

    void OnBtnSaveFile()
    {
        TCHAR szBuf[MAX_PATH];
        OPENFILENAME ofn = { 0 };
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_hWnd;
        ofn.lpstrFile = szBuf;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = _T("Select Txt File");
        ofn.lpstrDefExt = _T("txt");
        ofn.lpstrFilter = _T("txt Files\0*.txt\0");
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;
        if(GetSaveFileName(&ofn))
        {
            SWindow *pWnd = FindChildByName("txt_save_file");
            if(pWnd) 
                pWnd->SetWindowText(szBuf);
        }
    }

    void OnBtnPickFolder(){
        #ifndef _WIN32
        BROWSEINFO info={0};
        TCHAR szBuf[MAX_PATH];
        info.lpszPath = szBuf;
        info.nMaxPath = MAX_PATH;
        if(PickFolder(&info)){
            SWindow *pWnd = FindChildByName("txt_folder");
            if(pWnd) 
                pWnd->SetWindowText(szBuf);
        }
        #endif//!_WIN32
    }

    void OnBtnChooseFont(){
        LOGFONT logfont;
        CHOOSEFONT cf;
        memset(&logfont, 0, sizeof(logfont));
        memset(&cf, 0, sizeof(cf));
        cf.lStructSize = sizeof(cf);
        cf.lpLogFont = &logfont;
        cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
        if(ChooseFont(&cf)){
            SWindow *pWnd = FindChildByName("txt_font");
            if(pWnd) 
                pWnd->SetWindowText(logfont.lfFaceName);
        }
    }

    void OnBtnForkNormal(){
        SStringT strApp = SApplication::getSingletonPtr()->GetAppDir();
        strApp += _T("/demo");
        //open child normally
            SHELLEXECUTEINFO exec={0};
            exec.cbSize=sizeof(exec);
            exec.lpVerb=_T("open");
            exec.lpFile = strApp.c_str();
            if(ShellExecuteEx(&exec)){

            }
        return;
    }
    void OnBtnForkRoot(){
        SStringT strApp = SApplication::getSingletonPtr()->GetAppDir();
        strApp += _T("/demo");
        //open child normally
            SHELLEXECUTEINFO exec={0};
            exec.cbSize=sizeof(exec);
            exec.lpVerb=_T("runas");
            exec.lpFile = strApp.c_str();
            if(ShellExecuteEx(&exec)){

            }
    }

    EVENT_MAP_BEGIN()
        EVENT_ID_COMMAND(1, OnClose)
        EVENT_NAME_COMMAND(L"btn_resize", OnBtnResize)
        EVENT_NAME_COMMAND(L"btn_menu", OnBtnMenu)
        EVENT_NAME_COMMAND(L"btn_smenu", OnBtnSMenu)
        EVENT_NAME_COMMAND(L"btn_msgbox", OnShowMsgbox)
        EVENT_NAME_COMMAND(L"btn_smenuex", OnBtnSMenuEx)
        EVENT_NAME_COMMAND(L"btn_soui", OnBtnSoui)
        EVENT_NAME_COMMAND(L"btn_save_rtf",OnBtnSaveRtf)
        EVENT_NAME_COMMAND(L"btn_pick_color",OnBtnPickColor)
        EVENT_NAME_COMMAND(L"btn_open_file",OnBtnOpenFile)
        EVENT_NAME_COMMAND(L"btn_save_file",OnBtnSaveFile)
        EVENT_NAME_COMMAND(L"btn_pick_folder",OnBtnPickFolder)
        EVENT_NAME_COMMAND(L"btn_font",OnBtnChooseFont)
        EVENT_NAME_COMMAND(L"btn_fork_normal",OnBtnForkNormal)
        EVENT_NAME_COMMAND(L"btn_fork_root",OnBtnForkRoot)
        EVENT_HANDLER(EventRealWndCreate::EventID, OnRealWndCreate)
        EVENT_HANDLER(EventRealWndDestroy::EventID, OnRealWndDestroy)
        EVENT_HANDLER(EventInit::EventID,OnInit)
        EVENT_MAP_END2(SHostWnd)
    
protected:
    void OnTimer(UINT_PTR id) {
        if (id == 10) {
            static int nCount = 0;
            if (++nCount > 150)
            {
                KillTimer(id);
            }
            CRect rc = GetClientRect();
            SetWindowPos(0, 0, 0, rc.right + 1, rc.bottom, SWP_NOZORDER | SWP_NOMOVE);
        }
        SetMsgHandled(FALSE);
    }
    void OnSize(int flag, CSize sz) {
        SetMsgHandled(FALSE);
        if (0)
        {//test SetWindowRgn done
            HRGN hrgn = CreateEllipticRgn(0, 0, sz.cx, sz.cy);
            SetWindowRgn(hrgn, TRUE);
            DeleteObject(hrgn);
        }
    }

    LRESULT OnNotify(int idCtrl, LPNMHDR pnmh) {
        if (m_pSciter && idCtrl == m_pSciter->GetDlgCtrlID() && pnmh->code == SCN_MODIFIED)
        {
            SCNotification* pNotify = (SCNotification*)pnmh;
            if (pNotify->modificationType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT))
            {
                m_pSciter->UpdateLineNumberWidth();
            }
        }
        SetMsgHandled(FALSE);
        return 0;
    }

    void OnSysCommand(UINT nID, CPoint lParam){
        switch(nID){
        case SYS_MENUID_TEST:
            SLOGI() << "sys command test";
            break;
        default:
            SetMsgHandled(FALSE);
            break;
        }
    }

   BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_SIZE(OnSize)
        MSG_WM_SYSCOMMAND(OnSysCommand)
        MSG_WM_NOTIFY(OnNotify)
        CHAIN_MSG_MAP(SHostWnd)
        END_MSG_MAP()
};

void CMainDlg::OnClose() {
    PostMessage(WM_QUIT);
}

static VOID CALLBACK OnTimeout(HWND hwnd, UINT msg, UINT_PTR id, DWORD ts)
{
    static int count = 0;
    SLOGI() << "OnTimeout: id=" << id << " count=" << ++count;
    if (count>2)
        KillTimer(0, id);
}


int run_app(HINSTANCE hInst) {

    SStringT srcDir = getSourceDir();
    #ifndef _WIN32
    //test for AddFontResource
    AddFontResource((srcDir+_T("/../../simsun.ttc")).c_str());
    #endif//_WIN32
    UINT_PTR uid = SetTimer(0, 0, 5, OnTimeout);
    SLOGI() << "settimer: id=" << uid;
    CScintillaWnd::InitScintilla(hInst);
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFac;
    BOOL useGdi = 0;
    if (useGdi)
        comMgr.CreateRender_GDI((IObjRef**)&renderFac);
    else
        comMgr.CreateRender_Skia((IObjRef**)&renderFac);
    SAutoRefPtr<IImgDecoderFactory> imgDecoder;
    comMgr.CreateImgDecoder((IObjRef**)&imgDecoder);
    renderFac->SetImgDecoderFactory(imgDecoder);

    SApplication app(renderFac, hInst);
    {//test get base class name
        wchar_t szBaseClsName[MAX_OBJNAME];
        app.GetBaseClassName(SListView::GetClassName(), SListView::GetClassType(), szBaseClsName);
        SLOGI() << "base class name: " << szBaseClsName;
    }

    SouiFactory sfac;
    SAutoRefPtr<IResProvider> sysResouce(sfac.CreateResProvider(RES_FILE), FALSE);
    SStringT sysRes = srcDir + kPath_SysRes;
    sysResouce->Init((LPARAM)sysRes.c_str(), 0);
    app.LoadSystemNamedResource(sysResouce);
    BOOL bLoadFileRes = TRUE;
    if (bLoadFileRes) {
        SAutoRefPtr<IResProvider> testResouce(sfac.CreateResProvider(RES_FILE), FALSE);
        SStringT appRes = srcDir + kPath_TestRes;
        testResouce->Init((LPARAM)appRes.c_str(), 0);
        app.AddResProvider(testResouce);
    }
    else
    {
        SAutoRefPtr<IResProvider> testResouce;
        BOOL bLoad = comMgr.CreateResProvider_ZIP((IObjRef**)&testResouce);
        EXPECT_TRUE(bLoad);
        ZIPRES_PARAM param;
        SStringT strZip = srcDir + kPath_ZipRes;
        ZipFile(&param, renderFac, strZip.c_str(), kZip_Psw);
        BOOL initDone = testResouce->Init((WPARAM)&param, 0);
        app.AddResProvider(testResouce);
    }
    if (1) {//enable log
        SAutoRefPtr<ILogMgr> pLogMgr;
        if (comMgr.CreateLog4z((IObjRef**)&pLogMgr) && pLogMgr)
        {
            //uncomment next line to disable log mgr to output debug string.
            pLogMgr->setLoggerName("fun_test");

            //uncomment next line to record info level log.
            pLogMgr->setLoggerLevel(LOG_LEVEL_INFO);
            pLogMgr->start();
        }
        app.SetLogManager(pLogMgr);
    }

    SLOGI() << "start soui app";
    CMainDlg hostWnd("layout:XML_MAINWND");
    hostWnd.CreateEx(0, WS_POPUP|WS_SYSMENU, WS_EX_LAYERED, 300, 100, 0, 0);
    hostWnd.ShowWindow(SW_SHOW);
    //hostWnd.SetLayeredWindowAttributes(0,200,LWA_ALPHA);
    HMENU hMenu = GetSystemMenu(hostWnd.m_hWnd, FALSE);
    if(hMenu){
        MENUITEMINFO info;
        info.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID | MIIM_SUBMENU;
        info.cbSize = sizeof(info);
        info.fType = MFT_STRING;
        info.fState = MF_ENABLED;
        info.wID = SYS_MENUID_TEST;
        info.dwTypeData = (LPTSTR)_T("test");
        info.cch = 5;
        info.hSubMenu = NULL;
        InsertMenuItem(hMenu, 0, TRUE, &info);
    }
    int ret = app.Run(hostWnd.m_hWnd);
    SLOGI() << "soui app end, exit code=" << ret;
    CScintillaWnd::UninitScintilla();
    return 0;
}
