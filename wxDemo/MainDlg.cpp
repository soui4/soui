// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	
#define kLogTag "wxdemo"

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND")), m_pEmojiMenu(NULL)
{
}

CMainDlg::~CMainDlg()
{
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	CGlobalUnits::instance()->m_strTest = "1111111";

	STabCtrl* pMainOptTab = FindChildByName2<STabCtrl>(L"tab_main_opt");
	IWindow* pPageMessage = pMainOptTab->GetPage(0);
	IWindow* pPageContact = pMainOptTab->GetPage(1);
	IWindow* pPageCollect = pMainOptTab->GetPage(2);

	SImageButton* pBtnMessage = FindChildByName2<SImageButton>(L"btn_message");
	SImageButton* pBtnContact = FindChildByName2<SImageButton>(L"btn_contact");
	SImageButton* pBtnCollect = FindChildByName2<SImageButton>(L"btn_favorites");

	pBtnMessage->SetCheck(TRUE);
	pBtnContact->SetCheck(FALSE);
	pBtnCollect->SetCheck(FALSE);

	SListView* pLvMessage = FindChildByName2<SListView>(L"lv_message");
	pLvMessage->EnableScrollBar(SSB_HORZ, FALSE);

	m_pMessageAdapter = new CLvMessageAdapter(pLvMessage, this);
	pLvMessage->SetAdapter(m_pMessageAdapter);
	m_pMessageAdapter->Release();

	STabCtrl* pTabMessageComm = (STabCtrl*)pPageMessage->FindIChildByName(L"tab_msg_comm");
	SASSERT(pTabMessageComm);

	//测试添加文件助手
	{
		SStringW sstrID;
		sstrID.Format(L"%d", 99999);

		SStringW sstrPage;
		sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_FILEHELPER'/></page>", sstrID);
		pTabMessageComm->InsertItem(sstrPage);

		SStringW sstrAvatar = L"";
		SStringW sstrName = L"文件传输助手";
		SStringW sstrContent = L"测试1231241231231231123124123123123123122312";
		SStringW sstrTime = L"昨天";
		bool bReminder = false;

		int nType = 0;
		m_pMessageAdapter->AddItem(sstrID, nType, sstrAvatar, sstrName, sstrContent, sstrTime, bReminder);
	}

	//测试添加个人
	{
		for (int i = 0; i < 5; i++)
		{
			SStringW sstrID;
			sstrID.Format(L"%d", i);

			SStringW sstrPage;
			sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_PERSONALCOMM'/></page>", sstrID);
			pTabMessageComm->InsertItem(sstrPage);

			SStringW sstrAvatar = L"";
			SStringW sstrName;
			sstrName.Format(L"个人测试%d", i);
			SStringW sstrContent = L"测试1231241231231231123124123123123123122312";
			SStringW sstrTime = L"12:09";
			bool bReminder = false;
			if (i < 2) bReminder = true;

			int nType = 1;
			m_pMessageAdapter->AddItem(sstrID, nType, sstrAvatar, sstrName, sstrContent, sstrTime, bReminder);
		}
	}

	//测试添加群
	for (int i = 0; i < 5; i++)
	{
		SStringW sstrID;
		sstrID.Format(L"%d", i);

		SStringW sstrPage;
		sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_GROUPCOMM'/></page>", sstrID);
		pTabMessageComm->InsertItem(sstrPage);

		SStringW sstrAvatar = L"";
		SStringW sstrName;
		sstrName.Format(L"群测试%d", i);
		SStringW sstrContent = L"测试1231241231231231123124123123123123122312";
		SStringW sstrTime = L"12:09";
		bool bReminder = false;
		if (i % 2 == 0) bReminder = true;

		int nType = 2;
		m_pMessageAdapter->AddItem(sstrID, nType, sstrAvatar, sstrName, sstrContent, sstrTime, bReminder);
	}

	return 0;
}

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
		SXmlNode xmlLang = xmlLang.root().child(L"language");
		lang->Load(&xmlLang, 1);//1=LD_XML
		wchar_t lngName[TR_MAX_NAME_LEN] = {0};
		lang->GetName(lngName);
        pTransMgr->SetLanguage(lngName);
		pTransMgr->InstallTranslator(lang);
        GetRoot()->SDispatchMessage(UM_SETLANGUAGE, 0, 0);
	}
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
	#ifdef _WIN32
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
	#endif//_WIN32
}


void CMainDlg::OnBnClickMessage()
{
	STabCtrl* pMainOptTab = FindChildByName2<STabCtrl>(L"tab_main_opt");

	SImageButton* pBtnMessage = FindChildByName2<SImageButton>(L"btn_message");
	SImageButton* pBtnContact = FindChildByName2<SImageButton>(L"btn_contact");
	SImageButton* pBtnCollect = FindChildByName2<SImageButton>(L"btn_favorites");

	pBtnMessage->SetCheck(TRUE);
	pBtnContact->SetCheck(FALSE);
	pBtnCollect->SetCheck(FALSE);

	pMainOptTab->SetCurSel(0);
}

void CMainDlg::OnBnClickContact()
{
	STabCtrl* pMainOptTab = FindChildByName2<STabCtrl>(L"tab_main_opt");

	SImageButton* pBtnMessage = FindChildByName2<SImageButton>(L"btn_message");
	SImageButton* pBtnContact = FindChildByName2<SImageButton>(L"btn_contact");
	SImageButton* pBtnCollect = FindChildByName2<SImageButton>(L"btn_favorites");

	pBtnMessage->SetCheck(FALSE);
	pBtnContact->SetCheck(TRUE);
	pBtnCollect->SetCheck(FALSE);

	pMainOptTab->SetCurSel(1);
}

void CMainDlg::OnBnClickCollect()
{
	STabCtrl* pMainOptTab = FindChildByName2<STabCtrl>(L"tab_main_opt");

	SImageButton* pBtnMessage = FindChildByName2<SImageButton>(L"btn_message");
	SImageButton* pBtnContact = FindChildByName2<SImageButton>(L"btn_contact");
	SImageButton* pBtnCollect = FindChildByName2<SImageButton>(L"btn_favorites");

	pBtnMessage->SetCheck(FALSE);
	pBtnContact->SetCheck(FALSE);
	pBtnCollect->SetCheck(TRUE);

	pMainOptTab->SetCurSel(2);
}

bool CMainDlg::OnEditMessageSearchSetFocus(EventArgs* pEvt)
{
	//展示搜索框中的取消搜索按钮
	SImageButton* pSearchCancel = FindChildByName2<SImageButton>(L"btn_msg_search_cancel");
	pSearchCancel->SetAttribute(L"show", L"1");

	//需要展示搜索页面
	//TODO:
	return true;
}

bool CMainDlg::OnEditMessageSearchKillFocus(EventArgs* pEvt)
{
	SImageButton* pSearchCancel = FindChildByName2<SImageButton>(L"btn_msg_search_cancel");
	pSearchCancel->SetAttribute(L"show", L"0");

	//重新展示消息列表
	return true;
}

bool CMainDlg::OnEditMessageSearchChanged(EventArgs* e)
{
	return true;
}

void CMainDlg::OnEmotionItemClick(const std::string& strID)
{
	SLOGI() << "OnEmotionItemClick,id=" << strID.c_str();
	//根据ID找到对应的图片然后将图片添加到re中
	auto iter = CGlobalUnits::instance()->m_mapEmojisIndex.find(strID);
	if (iter != CGlobalUnits::instance()->m_mapEmojisIndex.end())
	{
		//TODO:
	}
}

void CMainDlg::OnBnClickEmotion(IEvtArgs *e)
{
	SWindow* btn = sobj_cast<SWindow>(e->Sender());
	CRect rc = btn->GetWindowRect();
	ClientToScreen2(&rc);

	SMenuEx menuEmoji;
	menuEmoji.LoadMenuU8("menuex:emoji");
	m_pEmojiMenu = &menuEmoji;
	menuEmoji.TrackPopupMenu(TPM_BOTTOMALIGN, rc.left, rc.top, m_hWnd, GetScale());
	m_pEmojiMenu = nullptr;
}

void CMainDlg::OnBnClickImage()
{
	//
}

void CMainDlg::OnBnClickFile()
{
	//
}

void CMainDlg::OnBnClickCapture()
{
	//
}

void CMainDlg::OnBnClickCaptureSetting()
{
	//
}

void CMainDlg::OnBnClickHistory()
{
	//
}

void CMainDlg::OnBnClickAudio()
{
	//
}

void CMainDlg::OnBnClickVideo()
{
	//
}

void CMainDlg::OnBnClickLive()
{
	//
}

void CMainDlg::OnBnClickAudioVideo()
{
	//
}

void CMainDlg::OnMessageItemClick(int& nIndex)
{
	//
	STabCtrl* pMainOptTab = FindChildByName2<STabCtrl>(L"tab_main_opt");
	IWindow* pPageMessage = pMainOptTab->GetPage(0);
	STabCtrl* pTabMessageComm = (STabCtrl*)pPageMessage->FindIChildByName(L"tab_msg_comm");
	SASSERT(pTabMessageComm);

	pTabMessageComm->SetCurSel(nIndex);

	SStatic* pTitle = (SStatic*)pPageMessage->FindIChildByName(L"text_talk_title");
	CLvMessageAdapter::ItemData* pData = m_pMessageAdapter->GetItemData(nIndex);
	if (pData)
	{
		pTitle->SetWindowText(S_CW2T(pData->m_sstrName));
	}
}

LRESULT CMainDlg::OnMenuEvent(UINT msg, WPARAM wp, LPARAM lp) {
	IEvtArgs* e = (IEvtArgs*)lp;
	_HandleEvent(e);
	return 0;
}

void CMainDlg::OnInitEmojiMenu(IMenuEx* menuPopup, UINT nIndex) {
	SHostWnd* pMenuHost = static_cast<SHostWnd*>(menuPopup->GetHostWnd());
	STileView* pTileView = pMenuHost->FindChildByName2<STileView>(L"emoji_titleview");
	SASSERT(pTileView);
	CEmotionTileViewAdapter* pAdapter = new CEmotionTileViewAdapter(this);
	pTileView->SetAdapter(pAdapter);
	std::map<std::string, std::string>::iterator iter = CGlobalUnits::instance()->m_mapEmojisIndex.begin();
	for (; iter != CGlobalUnits::instance()->m_mapEmojisIndex.end(); iter++)
	{
		pAdapter->AddItem(iter->first.c_str());
	}
	pAdapter->Release();
	pTileView->SetSel(-1);

}
