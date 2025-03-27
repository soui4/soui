// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	
#include "DropTargetImpl.h"
#include <atl.mini/SComCli.h>
#include <algorithm>

#define kLogTag "wxdemo"

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND")), m_pEmojiMenu(NULL)
{
    m_bMessageSearchResultEmpty = false;
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

	SListView *pLvMessageSearch = FindChildByName2<SListView>(L"lv_message_search");
    pLvMessageSearch->EnableScrollBar(SSB_HORZ, FALSE);
    m_pMessageSearchAdapter = new CLvMessageSearchAdapter(pLvMessageSearch, this);
    pLvMessageSearch->SetAdapter(m_pMessageSearchAdapter);
    m_pMessageSearchAdapter->Release();
	//设置search的适配器 TODO：
	pLvMessageSearch->SetVisible(FALSE, TRUE);

	STabCtrl* pTabMessageComm = (STabCtrl*)pPageMessage->FindIChildByName(L"tab_msg_comm");
	SASSERT(pTabMessageComm);

	//测试添加文件助手
	{
		SStringW sstrID;
		sstrID.Format(L"%d", 99999);

		SStringW sstrPage;
        sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_FILEHELPER'/></page>", sstrID.c_str());
		int iPage = pTabMessageComm->InsertItem(sstrPage);
		SWindow* pPage = static_cast<SWindow*>(pTabMessageComm->GetPage(iPage));
		SEdit* pInputEdit = pPage->FindChildByName2<SEdit>("edit_send");
		if (pInputEdit) {
			HRESULT hr = ::RegisterDragDrop(m_hWnd, GetDropTarget());
			SComPtr<IDropTarget> dropTarget(new CEditDropTarget(pInputEdit),FALSE);
			RegisterDragDrop(pInputEdit->GetSwnd(), dropTarget);
		}
        SStringW sstrAvatar = L"";
		SStringW sstrName = L"文件传输助手";
		SStringW sstrContent = L"测试1231241231231231123124123123123123122312";
		SStringW sstrTime = L"昨天";
		bool bReminder = false;

		int nType = 0;
		m_pMessageAdapter->AddItem(sstrID, nType, sstrAvatar, sstrName, sstrContent, sstrTime, bReminder);

		//测试生成搜索简拼、全拼
        SStringT sstrEncode = CGlobalUnits::instance()->EncodeChinese(S_CW2T(sstrName));
        SStringT sstrSimple, sstrFull;
        CGlobalUnits::instance()->GetSimpleAndFull(sstrEncode, sstrSimple, sstrFull);
        std::string strID = S_CW2A(sstrID).c_str();
        //将源字符串-ID、全拼-ID、简拼-ID保存在全局搜索结果中以备后续使用
        CGlobalUnits::instance()->m_mapMessageChineseSearch.insert(std::make_pair(S_CW2T(sstrName), SEARCH_INFO(nType, strID)));

        CGlobalUnits::instance()->m_mapMessagePinyinSearch.insert(std::make_pair(sstrSimple, SEARCH_INFO(nType, strID)));
        CGlobalUnits::instance()->m_mapMessagePinyinSearch.insert(std::make_pair(sstrFull, SEARCH_INFO(nType, strID)));
	}

	//测试添加个人
	{
		for (int i = 0; i < 5; i++)
		{
            SStringW sstrID;
			sstrID.Format(L"%d", i);

			SStringW sstrPage;
			sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_PERSONALCOMM'/></page>", sstrID.c_str());
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

			//测试生成搜索简拼、全拼
            SStringT sstrEncode = CGlobalUnits::instance()->EncodeChinese(S_CW2T(sstrName));
            SStringT sstrSimple, sstrFull;
            CGlobalUnits::instance()->GetSimpleAndFull(sstrEncode, sstrSimple, sstrFull);
            std::string strID = S_CW2A(sstrID).c_str();
            //将源字符串-ID、全拼-ID、简拼-ID保存在全局搜索结果中以备后续使用
            CGlobalUnits::instance()->m_mapMessageChineseSearch.insert(std::make_pair(S_CW2T(sstrName), SEARCH_INFO(nType, strID)));

            CGlobalUnits::instance()->m_mapMessagePinyinSearch.insert(std::make_pair(sstrSimple, SEARCH_INFO(nType, strID)));
            CGlobalUnits::instance()->m_mapMessagePinyinSearch.insert(std::make_pair(sstrFull, SEARCH_INFO(nType, strID)));
		}
	}

	//测试添加群
	for (int i = 0; i < 5; i++)
	{
		SStringW sstrID;
		sstrID.Format(L"%d", i);

		SStringW sstrPage;
		sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_GROUPCOMM'/></page>", sstrID.c_str());
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

		//测试生成搜索简拼、全拼
        SStringT sstrEncode = CGlobalUnits::instance()->EncodeChinese(S_CW2T(sstrName));
        SStringT sstrSimple, sstrFull;
        CGlobalUnits::instance()->GetSimpleAndFull(sstrEncode, sstrSimple, sstrFull);
        std::string strID = S_CW2A(sstrID).c_str();
        //将源字符串-ID、全拼-ID、简拼-ID保存在全局搜索结果中以备后续使用
        CGlobalUnits::instance()->m_mapMessageChineseSearch.insert(std::make_pair(S_CW2T(sstrName), SEARCH_INFO(nType, strID)));

        CGlobalUnits::instance()->m_mapMessagePinyinSearch.insert(std::make_pair(sstrSimple, SEARCH_INFO(nType, strID)));
        CGlobalUnits::instance()->m_mapMessagePinyinSearch.insert(std::make_pair(sstrFull, SEARCH_INFO(nType, strID)));
	}

	//测试添加公众号
	{
		for (int i = 0; i < 5; i++)
		{
			SStringW sstrID;
			sstrID.Format(L"%d", i);

			SStringW sstrPage;
			sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_GZH'/></page>", sstrID.c_str());
			pTabMessageComm->InsertItem(sstrPage);

			SStringW sstrAvatar = L"";
			SStringW sstrName;
			sstrName.Format(L"公众号测试%d", i);
			SStringW sstrContent = L"测试1231241231231231123124123123123123122312";
			SStringW sstrTime = L"2024-12-12";
			bool bReminder = false;
			int nType = -1;
			m_pMessageAdapter->AddItem(sstrID, nType, sstrAvatar, sstrName, sstrContent, sstrTime, bReminder);

			//测试生成搜索简拼、全拼
            SStringT sstrEncode = CGlobalUnits::instance()->EncodeChinese(S_CW2T(sstrName));
            SStringT sstrSimple, sstrFull;
            CGlobalUnits::instance()->GetSimpleAndFull(sstrEncode, sstrSimple, sstrFull);
            std::string strID = S_CW2A(sstrID).c_str();
            //将源字符串-ID、全拼-ID、简拼-ID保存在全局搜索结果中以备后续使用
            CGlobalUnits::instance()->m_mapMessageChineseSearch.insert(std::make_pair(S_CW2T(sstrName), SEARCH_INFO(nType, strID)));

            CGlobalUnits::instance()->m_mapMessagePinyinSearch.insert(std::make_pair(sstrSimple, SEARCH_INFO(nType, strID)));
            CGlobalUnits::instance()->m_mapMessagePinyinSearch.insert(std::make_pair(sstrFull, SEARCH_INFO(nType, strID)));
		}
	}

	//测试添加订阅号
	{
		SStringW sstrID;
		sstrID.Format(L"%d", 9999);

		SStringW sstrPage;
		sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_DYH'/></page>", sstrID.c_str());
		pTabMessageComm->InsertItem(sstrPage);

		SStringW sstrAvatar = L"";
		SStringW sstrName = L"订阅号测试";
		SStringW sstrContent = L"测试1231241231231231123124123123123123122312";
		SStringW sstrTime = L"2024-12-12";
		bool bReminder = false;
		int nType = -2;
		m_pMessageAdapter->AddItem(sstrID, nType, sstrAvatar, sstrName, sstrContent, sstrTime, bReminder);
	}

	//测试添加新闻
	{
		SStringW sstrID;
		sstrID.Format(L"%d", 9999);

		SStringW sstrPage;
		sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_NEWS'/></page>", sstrID.c_str());
		pTabMessageComm->InsertItem(sstrPage);

		SStringW sstrAvatar = L"";
		SStringW sstrName = L"新闻测试";
		SStringW sstrContent = L"测试1231241231231231123124123123123123122312";
		SStringW sstrTime = L"2024-12-12";
		bool bReminder = false;
		int nType = -3;
		m_pMessageAdapter->AddItem(sstrID, nType, sstrAvatar, sstrName, sstrContent, sstrTime, bReminder);
	}
	int nLvMessageSel = 0;
	pLvMessage->SetSel(nLvMessageSel, TRUE);
	OnMessageItemClick(nLvMessageSel);

	//通讯录相关初始化
	STreeView* pTreeView = FindChildByName2<STreeView>("tv_contact");
	pLvMessage->EnableScrollBar(SSB_HORZ, FALSE);

	m_pContactAdapter = new CTvContactAdapter(this);
	pTreeView->SetAdapter(m_pContactAdapter);
	m_pContactAdapter->Release();

	//通讯录-新朋友适配器设置
    STabCtrl *pTabContactInfo = (STabCtrl *)pPageContact->FindIChildByName(L"tab_contact_info");
    SASSERT(pTabContactInfo);
    SListView * pLvNewFriend = (SListView*)pTabContactInfo->GetPage(pTabContactInfo->GetPageIndex(_T("page_newfriend"), TRUE))->FindIChildByName(L"lv_newfrined");
    pLvNewFriend->EnableScrollBar(SSB_HORZ, FALSE);
	m_pNewFriendAdapter = new CLvNewFriendAdapter(pLvNewFriend, this);
    pLvNewFriend->SetAdapter(m_pNewFriendAdapter);
    m_pNewFriendAdapter->Release();

	for (int i = 0; i < 20; i++)
	{
        SStringW sstrID;
        sstrID.Format(L"%d", i);
        SStringW sstrAvatar = L"";
        SStringW sstrName;
        sstrName.Format(L"新朋友%d", i);
        SStringW sstrMessage;
        sstrMessage.Format(L"你好 我是新朋友%d 这是一条验证消息", i);
		int nState = 0;
        if (i == 0)
            nState = 2;
        else
            nState = i % 2;

        m_pNewFriendAdapter->AddItem(sstrID, sstrAvatar, sstrName, sstrMessage, nState);
	}

	//通讯录-公众号适配器设置
    STileView *pTileViewGZH = (STileView *)pTabContactInfo->GetPage(pTabContactInfo->GetPageIndex(_T("page_gzh"), TRUE))->FindIChildByName(L"tileview_gzh");
    pTileViewGZH->EnableScrollBar(SSB_HORZ, FALSE);
    m_pGZHTileViewAdapter = new CGZHTileViewAdapter(this);
    pTileViewGZH->SetAdapter(m_pGZHTileViewAdapter);
    m_pGZHTileViewAdapter->Release();
	for (int i = 0; i < 10; i++)
	{
		SStringW sstrID;
        sstrID.Format(L"%d", i);
		SStringW sstrName;
        sstrName.Format(L"公众号%d", i);
        std::string strAvatar = "";
        std::string strID = S_CW2A(sstrID).c_str();
        std::string strName = S_CW2A(sstrName).c_str();
		m_pGZHTileViewAdapter->AddItem(strID, strName, strAvatar);
	}

	//通讯录-订阅号适配器设置
    STileView *pTileViewDYH = (STileView *)pTabContactInfo->GetPage(pTabContactInfo->GetPageIndex(_T("page_dyh"), TRUE))->FindIChildByName(L"tileview_dyh");
    pTileViewDYH->EnableScrollBar(SSB_HORZ, FALSE);
    m_pDYHTileViewAdapter = new CDYHTileViewAdapter(this);
    pTileViewDYH->SetAdapter(m_pDYHTileViewAdapter);
    m_pDYHTileViewAdapter->Release();
    for (int i = 0; i < 10; i++)
    {
        SStringW sstrID;
        sstrID.Format(L"%d", i);
        SStringW sstrName;
        sstrName.Format(L"订阅号%d", i);
        std::string strAvatar = "";
        std::string strID = S_CW2A(sstrID).c_str();
        std::string strName = S_CW2A(sstrName).c_str();
        m_pDYHTileViewAdapter->AddItem(strID, strName, strAvatar);
    }

	//通讯录-群成员适配器设置
    STileView *pTileViewGrpmbr = (STileView *)pTabContactInfo->GetPage(pTabContactInfo->GetPageIndex(_T("page_group"), TRUE))->FindIChildByName(L"tileview_grpmbr");
    pTileViewGrpmbr->EnableScrollBar(SSB_HORZ, FALSE);
    m_pGrpmbrTileViewAdapter = new CGrpMbrTileViewAdapter(this);
    pTileViewGrpmbr->SetAdapter(m_pGrpmbrTileViewAdapter);
    m_pGrpmbrTileViewAdapter->Release();
	for (int i = 0; i < 40; i++)
    {
        SStringW sstrID;
        sstrID.Format(L"%d", i);
        SStringW sstrName;
        sstrName.Format(L"群成员%d", i);
        std::string strAvatar = "";
        std::string strID = S_CW2A(sstrID).c_str();
        std::string strName = S_CW2A(sstrName).c_str();
        m_pGrpmbrTileViewAdapter->AddItem(strID, strName, strAvatar);
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
	pSearchCancel->SetVisible(TRUE, TRUE);

    //需要展示搜索页面
    SListView *pLvMessage = FindChildByName2<SListView>(L"lv_message");
    SListView *pLvMessageSearch = FindChildByName2<SListView>(L"lv_message_search");
	pLvMessage->SetVisible(FALSE, TRUE);
	pLvMessageSearch->SetVisible(TRUE, TRUE);
	return true;
}

bool CMainDlg::OnEditMessageSearchKillFocus(EventArgs* pEvt)
{
	SImageButton* pSearchCancel = FindChildByName2<SImageButton>(L"btn_msg_search_cancel");
	pSearchCancel->SetVisible(FALSE, TRUE);

	//重新展示消息列表
    if (m_bMessageSearchResultEmpty)
	{
        SListView *pLvMessage = FindChildByName2<SListView>(L"lv_message");
        SListView *pLvMessageSearch = FindChildByName2<SListView>(L"lv_message_search");
		pLvMessage->SetVisible(TRUE, TRUE);
		pLvMessageSearch->SetVisible(FALSE, TRUE);
	}

	return true;
}
#include <algorithm>
bool CMainDlg::OnEditMessageSearchChanged(EventArgs* e)
{
    /*
     * 大致处理逻辑
     * 1、获取edit内容
     * 2、判断是拼音搜索还是中文搜索
     * 3、根据上述匹配拿到搜索结果然后重新构建搜索列表信息
     */
    EventRENotify *e2 = sobj_cast<EventRENotify>(e);
    if (e2->iNotify != EN_CHANGE)
        return false;
    SEdit *pEdit = sobj_cast<SEdit>(e2->sender);
    SStringT sstrInput = pEdit->GetWindowText();
    std::wstring wstrInput = S_CT2W(sstrInput).c_str();

	std::vector<SEARCH_INFO> vecSearchResult;
    if (!sstrInput.IsEmpty())
    {
        m_bMessageSearchResultEmpty = false;
        if (!CGlobalUnits::instance()->IsIncludeChinese(sstrInput)) //拼音搜索
        {
            //遍历构建的搜索表，然后查找字符串，如果找到了结果则将结果记录一下
			std::multimap<SStringT, SEARCH_INFO>::iterator iter = CGlobalUnits::instance()->m_mapMessagePinyinSearch.begin();
            for (; iter != CGlobalUnits::instance()->m_mapMessagePinyinSearch.end(); iter++)
			{
                SStringT sstrPinyin = iter->first;
                std::wstring wstrName = S_CT2W(sstrPinyin).c_str();
				if (wstrName.find(wstrInput) != std::string::npos)
                    vecSearchResult.push_back(iter->second);
			}

            //有可能查到多个将结果去重处理
            std::sort(vecSearchResult.begin(), vecSearchResult.end());
            vecSearchResult.erase(std::unique(vecSearchResult.begin(), vecSearchResult.end()), vecSearchResult.end());
        }
        else
        {
            //汉字搜索
			std::multimap<SStringT, SEARCH_INFO>::iterator iter = CGlobalUnits::instance()->m_mapMessageChineseSearch.begin();
            for (; iter != CGlobalUnits::instance()->m_mapMessageChineseSearch.end(); iter++)
            {
                SStringT sstrPinyin = iter->first;
                std::wstring wstrName = S_CT2W(sstrPinyin).c_str();
                if (wstrName.find(wstrInput) != std::string::npos)
                    vecSearchResult.push_back(iter->second);
            }
            //有可能查到多个将结果去重处理
            sort(vecSearchResult.begin(), vecSearchResult.end());
            vecSearchResult.erase(unique(vecSearchResult.begin(), vecSearchResult.end()), vecSearchResult.end());
        }

        //重新构建列表
        m_pMessageSearchAdapter->DeleteAllItem();
		for (int i = 0; i < vecSearchResult.size(); i++)
		{
            int nType = vecSearchResult[i].m_nType;
            std::string strID = vecSearchResult[i].m_strID;
			//根据type、id去全局或者别的地方去查找到对应的头像、name等数据然后将数据添加到列表中
            switch (nType)
            {
            case 0:
            {
				//file helper
                SStringW sstrID = S_CA2W(strID.c_str());
				SStringW sstrAvatar = L"";
				SStringW sstrName = L"文件传输助手";
                m_pMessageSearchAdapter->AddItem(sstrID, nType, sstrAvatar, sstrName);
            }
			break;
            case -1:
			{
				//gzh
                SStringW sstrID = S_CA2W(strID.c_str());
				SStringW sstrAvatar = L"";
				SStringW sstrName = L"公众号测试" + sstrID;
                m_pMessageSearchAdapter->AddItem(sstrID, nType, sstrAvatar, sstrName);
			}
			break;
            case -2:
            {
				//dyh
				SStringW sstrID = S_CA2W(strID.c_str());
				SStringW sstrAvatar = L"";
				SStringW sstrName = L"订阅号测试" + sstrID;
                m_pMessageSearchAdapter->AddItem(sstrID, nType, sstrAvatar, sstrName);
			}
			break;
            case -3:
            {
                //news
            }
            break;
            case 1:
            {
                //personal 
				SStringW sstrID = S_CA2W(strID.c_str());
				SStringW sstrAvatar = L"";
				SStringW sstrName = L"个人测试" + sstrID;
                m_pMessageSearchAdapter->AddItem(sstrID, nType, sstrAvatar, sstrName);
            }
            break;
            case 2:
            {
                //group
				SStringW sstrID = S_CA2W(strID.c_str());
				SStringW sstrAvatar = L"";
				SStringW sstrName = L"群测试" + sstrID;
                m_pMessageSearchAdapter->AddItem(sstrID, nType, sstrAvatar, sstrName);
            }
            break;
            default:
                break;
            }
		}

    }
    else
    {
        // todo:
        m_bMessageSearchResultEmpty = true;
        m_pMessageSearchAdapter->DeleteAllItem();
    }
	return true;
}

void CMainDlg::OnMessageSearchItemClick(int &nIndex)
{
    //消息页搜索列表项点击事件
	//隐藏搜索列表展示消息列表
	//将聊天页切换到对应的页面、将对应的列表项选中、将滚动条滚动到相应位置
}

void CMainDlg::OnBnClickMsgSearchCancel()
{
    m_bMessageSearchResultEmpty = true;

	SListView *pLvMessage = FindChildByName2<SListView>(L"lv_message");
    SListView *pLvMessageSearch = FindChildByName2<SListView>(L"lv_message_search");
	pLvMessage->SetVisible(TRUE, TRUE);
	pLvMessageSearch->SetVisible(FALSE, TRUE);
}

bool CMainDlg::OnEditContactSearchSetFocus(EventArgs* pEvt)
{
	//展示搜索框中的取消搜索按钮
	SImageButton* pSearchCancel = FindChildByName2<SImageButton>(L"btn_contact_search_cancel");
	pSearchCancel->SetAttribute(L"show", L"1");

	return true;
}

bool CMainDlg::OnEditContactSearchKillFocus(EventArgs* pEvt)
{
	SImageButton* pSearchCancel = FindChildByName2<SImageButton>(L"btn_contact_search_cancel");
	pSearchCancel->SetAttribute(L"show", L"0");

	//重新展示消息列表


	return true;
}

bool CMainDlg::OnEditContactSearchChanged(EventArgs* e)
{
	return true;
}


void CMainDlg::OnEmotionItemClick(const std::string& strID)
{
	SLOGI() << "OnEmotionItemClick,id=" << strID.c_str();
	//根据ID找到对应的图片然后将图片添加到re中
	std::map<std::string, std::string>::iterator iter = CGlobalUnits::instance()->m_mapEmojisIndex.find(strID);
	if (iter != CGlobalUnits::instance()->m_mapEmojisIndex.end())
	{
		EndMenuEx(-1);

		STabCtrl *pMainOptTab = FindChildByName2<STabCtrl>(L"tab_main_opt");
        IWindow *pPageMessage = pMainOptTab->GetPage(0);
        STabCtrl *pTabMessageComm = (STabCtrl *)pPageMessage->FindIChildByName(L"tab_msg_comm");
        SASSERT(pTabMessageComm);

		SWindow *pPage = static_cast<SWindow *>(pTabMessageComm->GetPage(pTabMessageComm->GetCurSel()));
        SRichEdit *edit = pPage->FindChildByName2<SRichEdit>("edit_send");
        if (edit)
        {
            edit->SetSel(-1);
            SStringA str = SStringA().Format("emoji id=%s", strID.c_str());
            edit->ReplaceSel(S_CA2T(str));
        }
	}
}

void CMainDlg::OnBnClickEmotion(IEvtArgs *e)
{
	HDC hdc = GetDC();
	IFontPtr font = SFontPool::GetFont(L"宋体",100);
	const LOGFONT * lf = font->LogFont();
	HFONT hf = CreateFontIndirect(lf);
	HGDIOBJ oldFont = SelectObject(hdc,hf);
	CRect rc2(0,0,200,200);
	DrawText(hdc,_T("test"),12,&rc2,DT_CENTER);
	SelectObject(hdc,oldFont);
	ReleaseDC(hdc);

	SWindow* btn = sobj_cast<SWindow>(e->Sender());
	CRect rc = btn->GetWindowRect();
	ClientToScreen2(&rc);

	SMenuEx menuEmoji;
	menuEmoji.LoadMenuU8("smenuex:emoji");
	m_pEmojiMenu = &menuEmoji;
	menuEmoji.TrackPopupMenu(TPM_BOTTOMALIGN, rc.left, rc.top, m_hWnd, GetScale());
	m_pEmojiMenu = NULL;
}

void CMainDlg::OnBnClickImage()
{
#ifdef _WIN32
    CFileDialogEx openDlg(TRUE, _T("图片"), 0, 6, _T("图片文件\0*.gif;*.bmp;*.jpg;*.png\0\0"));
    if (openDlg.DoModal() == IDOK)
    {
        SStringT strFile = openDlg.m_szFileName;
        int nFileSize;
        FILE *fp = _tfopen(strFile, _T("rb"));
        if (fp)
        {
            fseek(fp, 0L, SEEK_END);
            nFileSize = ftell(fp);
            rewind(fp);
            fclose(fp);
        }
        else
            return;

		//此处可先处理图片大小限制
		//TODO:

		//将图片插入到对应的send_richedit中
		//TODO:
    }
#endif //_WIN32
}

void CMainDlg::OnBnClickFile()
{
#ifdef _WIN32
    CFileDialogEx openDlg(TRUE, _T("文件"), 0, 6, _T("文件\0*.*\0\0"));
    if (openDlg.DoModal() == IDOK)
    {
        SStringT strFile = openDlg.m_szFileName;
        int nFileSize;
        FILE *fp = _tfopen(strFile, _T("rb"));
        if (fp)
        {
            fseek(fp, 0L, SEEK_END);
            nFileSize = ftell(fp);
            rewind(fp);
            fclose(fp);
        }
        else
            return;

        //此处可先处理文件大小限制
        // TODO:

        //将文件插入到对应的send_richedit中
        // TODO:
    }
#endif //_WIN32
}

void CMainDlg::OnBnClickCapture()
{
#ifdef _WIN32
	CSnapshotDlg dlg;
	CWindowEnumer::EnumAllTopWindow();
	dlg.DoModal(NULL);
#endif//_WIN32
}

void CMainDlg::OnBnClickCaptureSetting()
{
	//
}

void CMainDlg::OnBnClickHistory()
{
	//打开历史记录窗口
	//TODO:
}

void CMainDlg::OnBnClickAudio()
{
	//发送语音聊天请求
	//TODO:
}

void CMainDlg::OnBnClickVideo()
{
    //发送视频聊天请求
    // TODO:
}

void CMainDlg::OnBnClickLive()
{
    //发送群直播请求
    // TODO:
}

void CMainDlg::OnBnClickAudioVideo()
{
    //发送群语音视频聊天请求
    // TODO:
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

void CMainDlg::ContactTVItemClick(int nGID, const std::string& strID)
{
	//根据GID、ID做处理
    STabCtrl *pMainOptTab = FindChildByName2<STabCtrl>(L"tab_main_opt");
    IWindow *pPageContact = pMainOptTab->GetPage(1);
    STabCtrl *pContactInfoTab = (STabCtrl *)pPageContact->FindIChildByName(L"tab_contact_info");

    SStatic *pTitle = (SStatic *)pPageContact->FindIChildByName(L"text_contact_title");
    SASSERT(pTitle);
    switch (nGID)
    {
    case 1:
    {
        //新的朋友
        pTitle->SetWindowText(_T("新的朋友"));
        pContactInfoTab->SetCurSel(pContactInfoTab->GetPageIndex(_T("page_newfriend"), TRUE));
    }
    break;
    case 2:
    {
        //公众号
        pTitle->SetWindowText(_T("公众号"));
        pContactInfoTab->SetCurSel(pContactInfoTab->GetPageIndex(_T("page_gzh"), TRUE));
    }
    break;
    case 3:
    {
        //订阅号
        pTitle->SetWindowText(_T("订阅号"));
        pContactInfoTab->SetCurSel(pContactInfoTab->GetPageIndex(_T("page_dyh"), TRUE));
    }
    break;
    case 4:
    {
        //群聊
        GroupsMap::iterator iter = CGlobalUnits::instance()->m_mapGroups.find(strID);
        if (iter != CGlobalUnits::instance()->m_mapGroups.end())
        {
            std::string strName = iter->second.m_strGroupName;
            pTitle->SetWindowText(S_CA2T(strName.c_str()));

            pContactInfoTab->SetCurSel(pContactInfoTab->GetPageIndex(_T("page_group"), TRUE));

			//需要设置群详情页的信息
			//TODO:
        }
    }
    break;
    case 5:
    {
        //好友
		PersonalsMap::iterator iter = CGlobalUnits::instance()->m_mapPersonals.find(strID);
        if (iter != CGlobalUnits::instance()->m_mapPersonals.end())
        {
            std::string strName = iter->second.m_strName;
            pTitle->SetWindowText(S_CA2T(strName.c_str()));

            pContactInfoTab->SetCurSel(pContactInfoTab->GetPageIndex(_T("page_personal"), TRUE));

			//需要设置用户详情页的信息
			//TODO:
        }
    }
    break;
    default:
        break;
    }
}

void CMainDlg::ContactTVItemDBClick(int nGID, const std::string& strID)
{
	//根据GID、ID做处理
}

void CMainDlg::ContactTVItemRClick(int nGID, const std::string& strID)
{
	//根据GID、ID做处理
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
