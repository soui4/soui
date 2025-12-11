#pragma once

class CLoginDlg : public SHostDialog, public SDpiHandler<CLoginDlg>
{
public:
    CLoginDlg(); 
public:
    void OnBtnLocal();
    //soui消息
    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_close", OnClose)
        EVENT_NAME_COMMAND(L"btn_login", OnLogin)
        EVENT_NAME_COMMAND(L"btn_svr_local", OnBtnLocal)
    EVENT_MAP_END2(SHostDialog)
    
    //HostWnd真实窗口消息处理
    BEGIN_MSG_MAP_EX(CLoginDlg)
        CHAIN_MSG_MAP(SDpiHandler<CLoginDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
        CHAIN_MSG_MAP(SHostDialog)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

private:
    void OnClose();
    void OnLogin();
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
public:
    SStringT m_strSvr;
    SStringT m_strName;
    char m_cSex;
};