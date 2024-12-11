// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "CLvMessageAdapter.h"
#include "CGlobalUnits.h"

class CMainDlg : public SHostWnd
	, public CLvMessageAdapter::IListen
{
public:
	virtual void OnMessageItemClick(int& nIndex);
public:
	CMainDlg();
	~CMainDlg();

	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnSize(UINT nType, CSize size);
	//托盘通知消息处理函数
	void OnShellTrayNotify(IEvtArgs * e);
	//演示如何响应菜单事件
	void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

public:
	void OnBnClickMessage();
	void OnBnClickContact();
	void OnBnClickCollect();

	bool OnEditMessageSearchSetFocus(EventArgs* pEvt);
	bool OnEditMessageSearchKillFocus(EventArgs* pEvt);
	bool OnEditMessageSearchChanged(EventArgs* e);
protected:
	void OnLanguage(int nID);
	void OnLanguageBtnCN();
	void OnLanguageBtnJP();
	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
		EVENT_NAME_COMMAND(L"zh_cn", OnLanguageBtnCN)
		EVENT_NAME_COMMAND(L"jp", OnLanguageBtnJP)
		EVENT_NAME_COMMAND(L"btn_message", OnBnClickMessage)
		EVENT_NAME_COMMAND(L"btn_contact", OnBnClickContact)
		EVENT_NAME_COMMAND(L"btn_favorites", OnBnClickCollect)

		EVENT_NAME_HANDLER(L"edit_msg_search", EventSetFocus::EventID, OnEditMessageSearchSetFocus)
		EVENT_NAME_HANDLER(L"edit_msg_search", EventKillFocus::EventID, OnEditMessageSearchKillFocus)
		EVENT_NAME_HANDLER(L"edit_msg_search", EventRENotify::EventID, OnEditMessageSearchChanged)
	EVENT_MAP_END2(SHostWnd)
		
	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
	//托盘消息处理
		//MESSAGE_HANDLER(WM_ICONNOTIFY, OnIconNotify)
		MSG_WM_COMMAND(OnCommand)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

private:
	CLvMessageAdapter* m_pMessageAdapter;
};
