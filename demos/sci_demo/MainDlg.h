// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

class CMainDlg : public SHostDialog
{
public:
    CMainDlg();
    ~CMainDlg();

    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
    void OnClose();

    //HostWnd真实窗口消息处理
    BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        CHAIN_MSG_MAP(SOUI::SHostDialog)
    END_MSG_MAP()
};