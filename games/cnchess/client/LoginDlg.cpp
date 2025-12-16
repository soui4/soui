#include "stdafx.h"
#include "LoginDlg.h"

CLoginDlg::CLoginDlg() : SHostDialog(_T("LAYOUT:XML_LOGIN"))
{
}

BOOL CLoginDlg::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
    return TRUE;
}

void CLoginDlg::OnClose()
{
    EndDialog(IDCANCEL);
}

void CLoginDlg::OnLogin()
{
    SWindow * pEdtSvr = FindChildByName2<SEdit>(L"edt_svr");
    SWindow * pEdtName = FindChildByName2<SEdit>(L"edt_name");
    m_strSvr = pEdtSvr->GetWindowText();
    m_strName = pEdtName->GetWindowText();
    SComboBox * pComboSex = FindChildByName2<SComboBox>(L"cbx_sex");
    int iSel = pComboSex->GetCurSel();
    m_cSex = pComboSex->GetItemData(iSel);

    EndDialog(IDOK);
}

void CLoginDlg::OnBtnLocal()
{
    SWindow * pEdtSvr = FindChildByName2<SEdit>(L"edt_svr");
    pEdtSvr->SetWindowText(_T("127.0.0.1:3310"));
}