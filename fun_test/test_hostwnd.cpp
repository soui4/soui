#include <souistd.h>
#include <helper/SMenu.h>
#include <helper/SMenuEx.h>
#include <Scintilla.h>
#include <resprovider-zip/zipresprovider-param.h>
#include "common.h"
#include "ScintillaWnd.h"

using namespace SOUI;
#define kLogTag "test_hostwnd"

void RunMenu(POINT pt) {
    HMENU hMenu = CreatePopupMenu();
    if (hMenu)
    {
        for (int i = 0; i < 6; i++)
            InsertMenu(hMenu, -1, MF_BYPOSITION | (i == 1 ? MF_GRAYED : MFT_RADIOCHECK), 10 + i, SStringT().Format(_T("menu_%d"), i).c_str());

        MENUITEMINFO info;
        info.fMask = MIIM_SUBMENU;
        info.hSubMenu = CreatePopupMenu();
        // �����Ӳ˵�
        for (int i = 0; i < 6; i++)
            InsertMenu(info.hSubMenu, -1, MF_BYPOSITION | MF_CHECKED, 20 + i, SStringT().Format(_T("child menu_1����%d"), i).c_str());
        SetMenuItemInfo(hMenu, 0, TRUE, &info);

        info.hSubMenu = CreatePopupMenu();
        // �����Ӳ˵�
        for (int i = 0; i < 6; i++)
            InsertMenu(info.hSubMenu, -1, MF_BYPOSITION, 30 + i, SStringT().Format(_T("child menu_2����%d"), i).c_str());

        MENUITEMINFO info2;
        info2.fMask = MIIM_SUBMENU;
        info2.hSubMenu = CreatePopupMenu();
        for (int i = 0; i < 6; i++)
            InsertMenu(info2.hSubMenu, -1, MF_BYPOSITION, 40 + i, SStringT().Format(_T("child menu_3����%d"), i).c_str());
        SetMenuItemInfo(info.hSubMenu, 4, TRUE, &info2);

        SetMenuItemInfo(hMenu, 3, TRUE, &info);

        // ��ɫ�ҽ���
        //EnableMenuItem(hMenu, 0, MF_GRAYED | MF_BYPOSITION);
        // ���ò���ʾ��ɫ
       // EnableMenuItem(hMenu, 12, MF_DISABLED | MF_BYCOMMAND);
        // ʹ�ò˵�ID��ɫ�Ӳ˵�
        //EnableMenuItem(hMenu, 31, MF_GRAYED | MF_CHECKED);
        // ʹ�ò˵�ID��ɫ�Ӳ˵�
        //EnableMenuItem(hMenu, 42, MF_GRAYED);

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
    }

    void OnShowMsgbox(){ 
        MessageBox(m_hWnd, _T("TEST MSGBOX!!!"), _T("test msgbox"),MB_OK);
    }

    void OnBtnSoui(){
        CSouiWnd * pwnd = new CSouiWnd;
        CRect rc = GetWindowRect();
        pwnd->CreateEx(m_hWnd,WS_POPUP|WS_VISIBLE,WS_EX_LAYERED,rc.right+5,rc.top,0,0);
    }

    EVENT_MAP_BEGIN()
        EVENT_ID_COMMAND(1, OnClose)
        EVENT_NAME_COMMAND(L"btn_resize", OnBtnResize)
        EVENT_NAME_COMMAND(L"btn_menu", OnBtnMenu)
        EVENT_NAME_COMMAND(L"btn_smenu", OnBtnSMenu)
        EVENT_NAME_COMMAND(L"btn_msgbox", OnShowMsgbox)
        EVENT_NAME_COMMAND(L"btn_smenuex", OnBtnSMenuEx)
        EVENT_NAME_COMMAND(L"btn_soui", OnBtnSoui)
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

   BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_SIZE(OnSize)
        MSG_WM_NOTIFY(OnNotify)
        CHAIN_MSG_MAP(SHostWnd)
        END_MSG_MAP()
};

void CMainDlg::OnClose() {
    PostMessage(WM_QUIT);
}

int run_app(HINSTANCE hInst) {

    std::tstring srcDir = getSourceDir();

    CScintillaWnd::InitScintilla(hInst);
    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFac;
    BOOL useGdi = 1;
    if (useGdi)
        comMgr.CreateRender_GDI((IObjRef**)&renderFac);
    else
        comMgr.CreateRender_Skia((IObjRef**)&renderFac);
    SAutoRefPtr<IImgDecoderFactory> imgDecoder;
    comMgr.CreateImgDecoder((IObjRef**)&imgDecoder);
    renderFac->SetImgDecoderFactory(imgDecoder);

    SApplication app(renderFac, hInst);

    SouiFactory sfac;
    SAutoRefPtr<IResProvider> sysResouce(sfac.CreateResProvider(RES_FILE), FALSE);

    std::tstring sysRes = srcDir + kPath_SysRes;
    sysResouce->Init((LPARAM)sysRes.c_str(), 0);
    app.LoadSystemNamedResource(sysResouce);
    BOOL bLoadFileRes = TRUE;
    if (bLoadFileRes) {
        SAutoRefPtr<IResProvider> testResouce(sfac.CreateResProvider(RES_FILE), FALSE);
        std::tstring appRes = srcDir + kPath_TestRes;
        testResouce->Init((LPARAM)appRes.c_str(), 0);
        app.AddResProvider(testResouce);
    }
    else
    {
        SAutoRefPtr<IResProvider> testResouce;
        BOOL bLoad = comMgr.CreateResProvider_ZIP((IObjRef**)&testResouce);
        EXPECT_TRUE(bLoad);
        ZIPRES_PARAM param;
        std::tstring strZip = srcDir + kPath_ZipRes;
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
    hostWnd.CreateEx(0, WS_POPUP, WS_EX_LAYERED, 300, 100, 0, 0);
    hostWnd.ShowWindow(SW_SHOW);
    //hostWnd.SetLayeredWindowAttributes(0,200,LWA_ALPHA);
    int ret = app.Run(hostWnd.m_hWnd);
    SLOGI() << "soui app end, exit code=" << ret;
    CScintillaWnd::UninitScintilla();
    return 0;
}
