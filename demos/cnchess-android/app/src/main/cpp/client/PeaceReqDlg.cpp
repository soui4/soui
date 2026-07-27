#include "stdafx.h"
#include "PeaceReqDlg.h"

CPeaceReqDlg::CPeaceReqDlg() : SHostDialog(_T("LAYOUT:dlg_peace_req"))
{
}

BOOL CPeaceReqDlg::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
    return TRUE;
}

void CPeaceReqDlg::OnClose()
{
    EndDialog(IDCANCEL);
}

void CPeaceReqDlg::OnOK()
{
    m_strDesc = FindChildByName("edit_desc")->GetWindowText();
    EndDialog(IDOK);
}
