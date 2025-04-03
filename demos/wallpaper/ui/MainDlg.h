// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <event/SNotifyCenter.h>
#include "ImageAdapter.h"

class CMainDlg : public SHostWnd, public TAutoEventMapReg<CMainDlg>
{
public:
	CMainDlg();
	~CMainDlg();

	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnSize(UINT nType, CSize size);
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

protected:
	//soui消息
	void OnDownloadFinish(EventArgs *e);

	void OnCategorySelChange(EventArgs *e);

	EVENT_MAP_BEGIN()
		EVENT_ID_HANDLER(R.id.lb_catetory,EventLBSelChanged::EventID,OnCategorySelChange)
		EVENT_HANDLER(EventDownloadFinish::EventID,OnDownloadFinish)
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
	EVENT_MAP_END2(SHostWnd)
		
	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
private:

	SAutoRefPtr<CImageAdapter> m_imgAdapter;
};
