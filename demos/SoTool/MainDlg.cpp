// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "MainDlg.h"
#include "DragDrop.h"


CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:dlg_main"))
{
}

CMainDlg::~CMainDlg()
{
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	::RegisterDragDrop(m_hWnd, GetDropTarget());

    m_imgMergerHandler.OnInit(GetRoot());
    m_codeLineCounter.OnInit(GetRoot());
    m_2UnicodeHandler.OnInit(GetRoot());
    m_folderScanHandler.OnInit(GetRoot());
    m_calcMd5Handler.OnInit(GetRoot());
    m_windowHelperHander.OnInit(GetRoot());
	return 0;
}


void CMainDlg::OnClose()
{
	DestroyWindow();
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}

