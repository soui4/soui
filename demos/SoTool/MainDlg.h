// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ImageMergerHandler.h"
#include "CodeLineCounterHandler.h"
#include "2UnicodeHandler.h"
#include "FolderScanHandler.h"
#include "CalcMd5Handler.h"

class CMainDlg : public SOUI::SHostWnd
{
public:
	CMainDlg();
	~CMainDlg();

	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnSize(UINT nType, SOUI::CSize size);

	void OnBtnMsgBox();
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
protected:
	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
		CHAIN_EVENT_MAP_MEMBER(m_imgMergerHandler)
        CHAIN_EVENT_MAP_MEMBER(m_codeLineCounter)
        CHAIN_EVENT_MAP_MEMBER(m_2UnicodeHandler)
        CHAIN_EVENT_MAP_MEMBER(m_folderScanHandler)
        CHAIN_EVENT_MAP_MEMBER(m_calcMd5Handler)
	EVENT_MAP_END2(SOUI::SHostWnd)
	
	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
	
protected:
    CImageMergerHandler     m_imgMergerHandler;
    CCodeLineCounterHandler m_codeLineCounter;
    C2UnicodeHandler        m_2UnicodeHandler;
    CFolderScanHandler      m_folderScanHandler;
    CCalcMd5Handler         m_calcMd5Handler;
};
