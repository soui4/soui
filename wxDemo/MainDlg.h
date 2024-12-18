// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _MAINDLG_H_
#define _MAINDLG_H_

#include <helper/SMenuEx.h>
#include "CGlobalUnits.h"
#include "CEmotionTileViewAdapter.h"
#include "CLvMessageAdapter.h"
#include "CLvMessageSearchAdapter.h"
#include "CTvContactAdapter.h"

#include "CLvNewFriendAdapter.h"
#include "CGZHTileViewAdapter.h"
#include "CDYHTileViewAdapter.h"
#include "CGrpMbrTileViewAdapter.h"

#ifdef _WIN32
#include "snapshot/CWindowEnumer.h"
#include "snapshot/CSnapshotDlg.h"
#endif//_WIN32


class CMainDlg : public SHostWnd
	, public CLvMessageAdapter::IListen
    , public CLvMessageSearchAdapter::IListen
	, public CTvContactAdapter::IListen
    , public CLvNewFriendAdapter::IListen
    , public CGZHTileViewAdapter::IListen
    , public CDYHTileViewAdapter::IListen
    , public CGrpMbrTileViewAdapter::IListen
	, public CEmotionTileViewAdapter::IListener
{
public:
	virtual void OnMessageItemClick(int& nIndex);
	virtual void OnMessageSearchItemClick(int &nIndex);
	virtual void OnEmotionItemClick(const std::string& strID);

	virtual void ContactTVItemClick(int nGID, const std::string& strID);
	virtual void ContactTVItemDBClick(int nGID, const std::string& strID);
	virtual void ContactTVItemRClick(int nGID, const std::string& strID);
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
    void OnBnClickMsgSearchCancel();

	bool OnEditContactSearchSetFocus(EventArgs* pEvt);
	bool OnEditContactSearchKillFocus(EventArgs* pEvt);
	bool OnEditContactSearchChanged(EventArgs* e);

	void OnBnClickEmotion(IEvtArgs *e);
	void OnBnClickImage();
	void OnBnClickFile();
	void OnBnClickCapture();
	void OnBnClickCaptureSetting();
	void OnBnClickHistory();
	void OnBnClickAudio();
	void OnBnClickVideo();
	void OnBnClickLive();
	void OnBnClickAudioVideo();
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
		//EVENT_ID_HANDLER(R.id.tray_008,EventTrayNotify::EventID,OnShellTrayNotify)

		EVENT_NAME_COMMAND(L"btn_message", OnBnClickMessage)
		EVENT_NAME_COMMAND(L"btn_contact", OnBnClickContact)
		EVENT_NAME_COMMAND(L"btn_favorites", OnBnClickCollect)

        EVENT_NAME_COMMAND(L"btn_msg_search_cancel", OnBnClickMsgSearchCancel)
		EVENT_NAME_HANDLER(L"edit_msg_search", EventSetFocus::EventID, OnEditMessageSearchSetFocus)
		EVENT_NAME_HANDLER(L"edit_msg_search", EventKillFocus::EventID, OnEditMessageSearchKillFocus)
		EVENT_NAME_HANDLER(L"edit_msg_search", EventRENotify::EventID, OnEditContactSearchChanged)

		EVENT_NAME_HANDLER(L"btn_emotion",EventCmd::EventID, OnBnClickEmotion)
		EVENT_NAME_COMMAND(L"btn_image", OnBnClickImage)
		EVENT_NAME_COMMAND(L"btn_file", OnBnClickFile)
		EVENT_NAME_COMMAND(L"btn_snapshot", OnBnClickCapture)
		EVENT_NAME_COMMAND(L"btn_snapshot_arrow", OnBnClickCaptureSetting)
		EVENT_NAME_COMMAND(L"btn_history", OnBnClickHistory)
		EVENT_NAME_COMMAND(L"btn_audio", OnBnClickAudio)
		EVENT_NAME_COMMAND(L"btn_video", OnBnClickVideo)
		EVENT_NAME_COMMAND(L"btn_live", OnBnClickLive)
		EVENT_NAME_COMMAND(L"btn_audio_video", OnBnClickAudioVideo)

		EVENT_NAME_HANDLER(L"edit_contact_search", EventSetFocus::EventID, OnEditContactSearchSetFocus)
		EVENT_NAME_HANDLER(L"edit_contact_search", EventKillFocus::EventID, OnEditContactSearchKillFocus)
		EVENT_NAME_HANDLER(L"edit_contact_search", EventRENotify::EventID, OnEditMessageSearchChanged)
	EVENT_MAP_END2(SHostWnd)
		
	LRESULT OnMenuEvent(UINT msg, WPARAM wp, LPARAM lp);
	void OnInitEmojiMenu(IMenuEx* menuPopup, UINT nIndex);

	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
	//托盘消息处理
		//MESSAGE_HANDLER(WM_ICONNOTIFY, OnIconNotify)
		MSG_WM_COMMAND(OnCommand)
		MESSAGE_HANDLER_EX(UM_MENUEVENT, OnMenuEvent)
		MSG_WM_INITMENUPOPUP_EX(OnInitEmojiMenu)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

private:
	CLvMessageAdapter* m_pMessageAdapter;
	CTvContactAdapter* m_pContactAdapter;

	CLvMessageSearchAdapter *m_pMessageSearchAdapter;

	CLvNewFriendAdapter* m_pNewFriendAdapter;
    CGZHTileViewAdapter *m_pGZHTileViewAdapter;
    CDYHTileViewAdapter *m_pDYHTileViewAdapter;
    CGrpMbrTileViewAdapter *m_pGrpmbrTileViewAdapter;

	SMenuEx* m_pEmojiMenu;

	bool m_bMessageSearchResultEmpty;
};


#endif//_MAINDLG_H_