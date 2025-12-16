#include "stdafx.h"
#include "LoginDlg.h"

CLoginDlg::CLoginDlg()
    : SHostWnd(_T("LAYOUT:XML_LOGIN"))
    , m_cSex(0)
{
}

CLoginDlg::~CLoginDlg()
{
}

BOOL CLoginDlg::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
    FindChildByName2<SComboView>(L"cbo_sex")->SetCurSel(0);
    FindChildByName2<SEdit>(L"edit_name")->SetWindowText(_T("象棋爱好者"));
    FindChildByName2<SEdit>(L"edit_svr")->SetWindowText(_T("ws://127.0.0.1:8080"));
    return TRUE;
}

void CLoginDlg::OnBtnOK()
{
    m_cSex = FindChildByName2<SComboView>(L"cbo_sex")->GetCurSel();
    m_strName = FindChildByName2<SEdit>(L"edit_name")->GetWindowText();
    m_strSvr = FindChildByName2<SEdit>(L"edit_svr")->GetWindowText();
    EndDialog(IDOK);
}

void CLoginDlg::OnBtnCancel()
{
    EndDialog(IDCANCEL);
}