#ifndef _EMOJIDLG_H_
#define _EMOJIDLG_H_

#include "CEmotionTileViewAdapter.h"

class CEmojiDlg : public SHostWnd
	, public CEmotionTileViewAdapter::IListener
{
public:
	struct IListener
	{
		virtual void EmotionTileViewItemClick(const std::string& strID) = 0;
	};
public:
	CEmojiDlg(IListener* pListner);
	~CEmojiDlg(void);

public:
	void OnEmotionItemClick(const std::string& strID);
protected:
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
	void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);
	void OnSize(UINT nType, SOUI::CSize size);
	void OnLButtonDown(UINT nFlags, SOUI::CPoint pt);
	void OnTimer(UINT_PTR idEvent);
	void OnSysCommand(UINT nID, SOUI::CPoint pt);

protected:

	BEGIN_MSG_MAP_EX(CEmojiDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_COMMAND(OnCommand)
		MSG_WM_SIZE(OnSize)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_SYSCOMMAND(OnSysCommand)

		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

public:
	void SetNoSel();

private:
	IListener* m_pListner;
};

#endif//_EMOJIDLG_H_