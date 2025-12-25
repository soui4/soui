#pragma once

class CPeaceAckDlg : public SHostDialog, public SDpiHandler<CPeaceAckDlg>
{
public:
    CPeaceAckDlg(LPCSTR pszDescUtf8); 
public:
    void OnClose();
    void OnOK();
    //soui消息
    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_close", OnClose)
        EVENT_ID_COMMAND(IDOK,OnOK)        
        EVENT_ID_COMMAND(IDCANCEL,OnClose)        
    EVENT_MAP_END2(SHostDialog)
    
    //HostWnd真实窗口消息处理
    BEGIN_MSG_MAP_EX(CPeaceAckDlg)
        CHAIN_MSG_MAP(SDpiHandler<CPeaceAckDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
        CHAIN_MSG_MAP(SHostDialog)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

private:
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
public:
    SStringT m_strDesc;
};