// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

class CMainDlg : public SHostWnd
{
public:
	CMainDlg();
	~CMainDlg();

	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnSize(UINT nType, CSize size);
	
	void OnBtnMenu();
	void OnBtnMsgBox();
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

protected:
	void OnLanguageChanged();

	void OnBtnScale(int nID);
	void OnLanguage(int nID);
	void OnMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl);

	void InitListView();

	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
		EVENT_ID_COMMAND_RANGE(R.id.lang_cn,R.id.lang_en,OnLanguage)
		EVENT_NAME_COMMAND(L"btn_menu",OnBtnMenu)
		EVENT_ID_COMMAND_RANGE(R.id.btn_scale_100,R.id.btn_scale_200,OnBtnScale)
		EVENT_MAP_END2(SHostWnd)
	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		COMMAND_ID_HANDLER_EX(102,OnMenuCmd)
		COMMAND_RANGE_HANDLER_EX(100,101,OnMenuCmd)

		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
private:
	int	m_LangID;
	SAutoRefPtr<ISkinPool> m_scalePool;
};
