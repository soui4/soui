// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"

CMainDlg::CMainDlg() : SHostDialog(_T("LAYOUT:sci.demo"))
{
}

CMainDlg::~CMainDlg()
{
}

BOOL CMainDlg::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
    return 0;
}

void CMainDlg::OnClose()
{
    SNativeWnd::DestroyWindow();
}