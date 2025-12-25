#include "stdafx.h"
#include "PeaceAckDlg.h"

CPeaceAckDlg::CPeaceAckDlg(LPCSTR pszDescUtf8) : SHostDialog(_T("LAYOUT:dlg_peace_ack"))
{
    m_strDesc = S_CA2T(pszDescUtf8,CP_UTF8);
}

BOOL CPeaceAckDlg::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
    FindChildByName("edit_desc")->SetWindowText(m_strDesc);
    return TRUE;
}

void CPeaceAckDlg::OnClose()
{
    EndDialog(IDCANCEL);
}

void CPeaceAckDlg::OnOK()
{
    EndDialog(IDOK);
}
