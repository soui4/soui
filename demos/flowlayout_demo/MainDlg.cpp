// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	
#include <helper/SMenuEx.h>
#include <helper/slog.h>
#define kLogTag "MainDlg"
CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
}

CMainDlg::~CMainDlg()
{
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	{
		SCheckGroup *pCheckGroup = FindChildByID2<SCheckGroup>(R.id.ckg_area);
		SCheckItem vOptions[] = {
			{TRUE, _T("50-80")},
			{FALSE, _T("80-100")},
			{FALSE, _T("100-130")},
			{FALSE, _T("130-150")},
			{FALSE, _T("<50")},
			{FALSE, _T("150-250")},
			{FALSE, _T("250-500")},
			{FALSE, _T(">500")},
		};
		pCheckGroup->SetOptions(vOptions, ARRAYSIZE(vOptions));
	}
	{
		SCheckGroup *pCheckGroup = FindChildByID2<SCheckGroup>(R.id.ckg_type);
		SCheckItem vOptions[] = {
		 {FALSE, _T("单位食堂")},
		 {FALSE, _T("学校食堂")},
		 {FALSE, _T("中央厨房")},
		 {FALSE, _T("智慧厨房")},
		 {FALSE, _T("大型酒店")},
		 {FALSE, _T("自自主厨房")},
		 {FALSE, _T("连锬餐饮")},
		 {FALSE, _T("口小吃")},
		 {FALSE, _T("火锅")},
		 {FALSE, _T("烧烤")},
		 {FALSE, _T("快餐")},
		 {FALSE, _T("面馆")},
		 {FALSE, _T("八大菜系")},
		 {FALSE, _T("其他")},
		};
		pCheckGroup->SetOptions(vOptions, ARRAYSIZE(vOptions));
	}

	return 0;
}
//TODO:消息映射
void CMainDlg::OnClose()
{
	SNativeWnd::DestroyWindow();
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


void CMainDlg::OnCkgArea(IEvtArgs *e)
{
	EventGroupCheck *e2 = sobj_cast<EventGroupCheck>(e);
	SCheckGroup *pCkgArea = sobj_cast<SCheckGroup>(e->Sender());
	int item = e2->iItem;
	SLOGI()<<"OnCkgArea:"<<item;
}

void CMainDlg::OnCkgType(IEvtArgs *e)
{
	EventGroupCheck *e2 = sobj_cast<EventGroupCheck>(e);
	SCheckGroup *pCkgType = sobj_cast<SCheckGroup>(e->Sender());
	int item = e2->iItem;
	SLOGI()<<"OnCkgType:"<<item;
}