﻿// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	
#include <helper/SMenuEx.h>

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
}

CMainDlg::~CMainDlg()
{
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	return 0;
}
[!if CHECKBOX_DYNAMIC_TRANSLATOR_SUPPORT]

void CMainDlg::OnLanguageBtnCN()
{
	OnLanguage(1);
}
void CMainDlg::OnLanguageBtnJP()
{
	OnLanguage(0);
}
void CMainDlg::OnLanguage(int nID)
{
	ITranslatorMgr *pTransMgr = SApplication::getSingletonPtr()->GetTranslator();
	SASSERT(pTransMgr);
	bool bCnLang = nID == 1;

	SXmlDoc xmlLang;
	if (SApplication::getSingletonPtr()->LoadXmlDocment(xmlLang, bCnLang ? _T("translator:lang_cn") : _T("translator:lang_jp")))
	{
		CAutoRefPtr<ITranslator> lang;
		pTransMgr->CreateTranslator(&lang);
		lang->Load(&xmlLang.root().child(L"language"), 1);//1=LD_XML
		TCHAR lngName[TR_MAX_NAME_LEN] = {0};
		lang->GetName(lngName);
        pTransMgr->SetLanguage(lngName);
		pTransMgr->InstallTranslator(lang);
        GetRoot()->SDispatchMessage(UM_SETLANGUAGE, 0, 0);
	}
}

[!endif]
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
[!if CHECKBOX_MAXIMIZE_BOX]

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
[!endif]

[!if CHECKBOX_SHELLNOTIFYICON]

//演示如何响应菜单事件
void CMainDlg::OnCommand(UINT uNotifyCode, int nID, HWND wndCtl)
{
	if (uNotifyCode == 0)
	{
		switch (nID)
		{
		case 6:
			PostMessage(WM_CLOSE);
			break;
		default:
			break;
		}
	}
}


void CMainDlg::OnShellTrayNotify(IEvtArgs * e)
{
	EventTrayNotify *e2 = sobj_cast<EventTrayNotify>(e);
	SShellTray *pTray = sobj_cast<SShellTray>(e->Sender());
	switch(e2->lp)
	{
	case WM_LBUTTONDOWN:
		if(IsWindowVisible())
		{
			ShowWindow(SW_HIDE);
		}else
		{
			ShowWindow(SW_SHOW);
			if(IsIconic())
			{
				OnRestore();
			}
		}
		break;
	case WM_RBUTTONDOWN:
		{
			SMenu menu;
			if (menu.LoadMenu(_T("SMENU:menu_tray")))
			{
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(m_hWnd);
				menu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
			}
		}
		break;
	}
}
[!endif]