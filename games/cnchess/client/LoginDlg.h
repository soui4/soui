#pragma once

#include "resource.h"
#include "core/shostwnd.h"
#include "core/smsgloop.h"
#include "widgets/sedit.h"

class CLoginDlg : public SHostWnd
{
public:
    CLoginDlg();
    ~CLoginDlg();

    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
    void OnBtnOK();
    void OnBtnCancel();

protected:
    //soui消息
    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_ok", OnBtnOK)
        EVENT_NAME_COMMAND(L"btn_cancel", OnBtnCancel)
    EVENT_MAP_END()

    //HostWnd真实窗口消息处理
    BEGIN_MSG_MAP_EX(CLoginDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        CHAIN_MSG_MAP(SHostWnd)
    END_MSG_MAP()
public:
    SStringT m_strName;
    SStringT m_strSvr;
    int      m_cSex;
};