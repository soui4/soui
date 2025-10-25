// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include "helpapi.h"
#include "Dialog/DlgNewLayout.h"
#include "Dialog/DlgAbout.h"
#include "SysdataMgr.h"
#include <vector>
#include <algorithm>
#include <helper/SMenu.h>
#include <helper/SMenuEx.h>
#include "SImgCanvas.h"
#include "Dialog/DlgResMgr.h"
#include "Dialog/DlgInsertXmlElement.h"
#define kLogTag "maindlg"

#define UIRES_FILE	L"uires.idx"
CSysDataMgr g_SysDataMgr;

//----------------------------------------------------------------------------
CMainDlg::CMainDlg() 
: SHostWnd(_T("LAYOUT:XML_MAINWND"))
,m_pXmlEdtior(NULL)
,m_bAutoSave(TRUE)
,m_editXmlType(XML_UNKNOWN)
,m_UIResFileMgr(this)
,m_bIsOpen(FALSE)
{
}

CMainDlg::~CMainDlg()
{
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	m_treePro = FindChildByName2<STreeCtrl>(L"workspace_tree");
	m_treePro->SetListener(this);
	m_lbWorkSpaceXml = FindChildByName2<SListBox>(L"workspace_xmlfile_lb");
	m_btn_recentFile = FindChildByName2<SButton>(L"toolbar_btn_recent");
	
	m_tabWorkspace = FindChildByName2<STabCtrl>(L"workspace_tab");

	m_lvSkin = FindChildByName2<SListView>(L"lv_tb_skin");
	m_lvWidget = FindChildByName2<SListView>(L"lv_tb_widget");

	m_RecentFileMenu.LoadMenu(UIRES.smenu.menu_recent);

	LoadAppCfg();
	FindChildByID(R.id.chk_autosave)->SetCheck(m_bAutoSave);

	//======================================================================
	m_pXmlEdtior = new CXmlEditor(this);
	m_pXmlEdtior->Init(GetRoot(),this);
	SStringT strCfgDir = SApplication::getSingletonPtr()->GetAppDir() + _T("/Config");
	SApplication::getSingleton().SetFilePrefix(strCfgDir);
	g_SysDataMgr.LoadSysData(strCfgDir);

	{//init control listbox
		CWidgetTBAdapter * pAdapter = new CWidgetTBAdapter(g_SysDataMgr.getCtrlDefNode(),this);
		m_lvWidget->SetAdapter(pAdapter);
		pAdapter->Release();
	}
	{//init skin listbox
		CSkinTBAdapter * pAdapter = new CSkinTBAdapter(g_SysDataMgr.getSkinDefNode(),this);
		m_lvSkin->SetAdapter(pAdapter);
		pAdapter->Release();
	}


	HRESULT hr = ::RegisterDragDrop(m_hWnd, GetDropTarget());
	RegisterDragDrop(m_treePro->GetSwnd(), new CDropTarget(this));
	SWindow* MainWnd = FindChildByName2<SWindow>("UI_main_caption");
	if (MainWnd)
		RegisterDragDrop(MainWnd->GetSwnd(), new CDropTarget(this));

	UpdateToolbar();
	return 0;
}

void CMainDlg::LoadAppCfg()
{
	pugi::xml_document xmldoc;
	pugi::xml_parse_result result = xmldoc.load_file(SApplication::getSingletonPtr()->GetAppDir() + _T("/sEditor.cfg"));
	if (result)
	{
		m_vecRecentFile.clear();
		pugi::xml_node rootNode = xmldoc.child(L"root");
		if (!rootNode)
			return;

		m_bAutoSave = rootNode.attribute(L"autoSave").as_bool(true);

		pugi::xml_node recentNode = rootNode.child(L"recent_file");
		pugi::xml_node recentItem = recentNode.first_child();
		while (recentItem)
		{
			SStringT filepath = S_CW2T(recentItem.attribute(L"path").as_string());
			m_vecRecentFile.push_back(filepath);

			recentItem = recentItem.next_sibling();
		}
	}

	do 
	{
		;
	} while (m_RecentFileMenu.DeleteMenu(0, MF_BYCOMMAND | MF_BYPOSITION));
	
	UINT MenuId = MenuId_Start;
	for (std::vector<SStringT>::iterator it = m_vecRecentFile.begin();it != m_vecRecentFile.end();it++)
	{
		m_RecentFileMenu.InsertMenu(0, MF_BYCOMMAND|MF_BYPOSITION, MenuId++, (*it) + _T("  "));
	}
}

void CMainDlg::SaveAppCfg()
{
	pugi::xml_document xmlDoc;
	pugi::xml_node nodeRoot = xmlDoc.append_child(L"root");
	nodeRoot.append_attribute(L"autoSave").set_value(m_bAutoSave);
	// 声明
	pugi::xml_node pre = xmlDoc.prepend_child(pugi::node_declaration);
	pre.append_attribute(L"version") = "1.0";
	pre.append_attribute(L"encoding") = "utf-8";

	// 注释节点1
	pugi::xml_node nodeCommentStudents = nodeRoot.append_child(pugi::node_comment);
	nodeCommentStudents.set_value(L"SouiEditor Config");
	// 普通节点1
	pugi::xml_node nodeRecents = nodeRoot.append_child(L"recent_file");
	for (std::vector<SStringT>::iterator it = m_vecRecentFile.begin();it != m_vecRecentFile.end();it++)
	{
		pugi::xml_node nodeRecent = nodeRecents.append_child(L"item");
		nodeRecent.append_attribute(L"path").set_value(S_CT2W(*it));
	}

	xmlDoc.save_file(SApplication::getSingletonPtr()->GetAppDir() + _T("/sEditor.cfg"));
}

bool CMainDlg::OnTreeproContextMenu(CPoint pt)
{
	CPoint pt2 = pt;
	ClientToScreen(&pt2);

	HSTREEITEM Item = m_treePro->HitTest(pt);
	if (!Item) return false;

	SMenuEx menu;
	menu.LoadMenu(UIRES.smenu.menu_layoutfile);
	int cmd = menu.TrackPopupMenu(TPM_RETURNCMD, pt2.x, pt2.y, m_hWnd);

	return false;
}

void CMainDlg::OnClose()
{
	if (m_bIsOpen)
	{
		if(!CloseProject())
			return;
	}

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

void CMainDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	SetMsgHandled(FALSE);
	if (bShow && !m_cmdWorkspaceFile.IsEmpty())
	{
		OpenProject(m_cmdWorkspaceFile);
		m_bIsOpen = TRUE;
		m_cmdWorkspaceFile.Empty();
	}
}

void CMainDlg::OnCommand(UINT uNotifyCode, int nID, HWND wndCtl)
{
	if (uNotifyCode == 0)
	{
		if (nID >= MenuId_Start && nID <= (MenuId_Start + m_vecRecentFile.size()))
		{
			if (m_bIsOpen)
			{
				SLOGW()<<"请先关闭当前工程";
				return;
			}
			OpenProject(m_vecRecentFile[nID - MenuId_Start]);
		}
	}
}


void CMainDlg::OnBtnOpen()
{
	if (m_bIsOpen)
	{
		SLOGW()<<"请关闭当前工程后再打开新的";
		return;
	}
	CFileDialogEx OpenDlg(TRUE, NULL, _T("idx"), _T("uires.idx"), 6, _T("soui skin index(*.idx)\0*.idx\0All files (*.*)\0*.*\0\0"));
	if (IDOK == OpenDlg.DoModal())
	{
		OpenProject(OpenDlg.m_szFileName);
	}
}

void CMainDlg::OnBtnClose()
{
	if (!m_bIsOpen || m_strProPath.IsEmpty())
		return;

	CloseProject();

	LoadAppCfg();
}


bool CMainDlg::CloseProject()
{
	if(!CheckSave())
		return false;
	m_pXmlEdtior->CloseProject();
	m_treePro->RemoveAllItems();
	m_lbWorkSpaceXml->DeleteAll();
	m_UIResFileMgr.CloseProject();	

	m_editXmlType = XML_UNKNOWN;
	UpdateEditorToolbar();

	m_bIsOpen = FALSE;
	UpdateToolbar();
	return true;
}

//打开工程
void CMainDlg::OpenProject(SStringT strFileName)
{
	m_treePro->RemoveAllItems();
	m_lbWorkSpaceXml->DeleteAll();
	
	SStringT strFile = strFileName;
	int n = strFileName.ReverseFind(TPATH_SLASH);
	m_strUiresPath = strFileName;
	m_strProPath = strFileName.Mid(0, n);
	
	if (!FileIsExist(strFileName))
	{
		SStringT notestr;
		notestr.Format(_T("未能打开 %s, \n是否从最近列表中移除对它的引用?"), strFileName.c_str());
		if (SMessageBox(0, notestr, _T("提示"), MB_YESNO) == IDYES)
		{
			std::vector<SStringT>::iterator it = std::find(m_vecRecentFile.begin(), m_vecRecentFile.end(), strFileName);
			if (it != m_vecRecentFile.end())
			{				
				m_vecRecentFile.erase(it);
				SaveAppCfg();
				LoadAppCfg();
			}
		}
		
		return;
	}

	if(!m_UIResFileMgr.OpenProject(m_strUiresPath))
	{
		SLOGW()<<"open project file failed";
		return;
	}

	m_strUIResFile = strFileName;
	m_pXmlEdtior->SetProjectPath(strFileName);

	{
		SPOSITION pos = m_UIResFileMgr.m_mapLayoutFile.GetStartPosition();
		while(pos){
			const SMap<SStringT, SStringT>::CPair* item = m_UIResFileMgr.m_mapLayoutFile.GetNext(pos);
			SStringT strPath = item->m_value;
			SStringT strName = item->m_key;
			HSTREEITEM hitem = m_treePro->InsertItem(strName);  //strName = "xml_mainwnd"
			m_treePro->SetItemData(hitem, (LPARAM)new SStringT(strPath));  //strpath = "xml\dlg_maing.xml"
		}
	}
    {
        std::vector<SStringT> vecTemp;
        SPOSITION pos = m_UIResFileMgr.m_mapXmlFile.GetStartPosition();
        while (pos)
        {
            const SMap<SStringT, SStringT>::CPair *item = m_UIResFileMgr.m_mapXmlFile.GetAt(pos);
            vecTemp.push_back(item->m_value);
            m_UIResFileMgr.m_mapXmlFile.GetNext(pos);
        }

        m_lbWorkSpaceXml->AddString(S_CW2T(UIRES_FILE));
        std::sort(vecTemp.begin(), vecTemp.end(), SortSStringNoCase);
        std::vector<SStringT>::iterator it = vecTemp.begin();
        for (; it != vecTemp.end(); it++)
        {
            m_lbWorkSpaceXml->AddString(*it);
        }
    }

    m_bIsOpen = TRUE;
	UpdateToolbar();
	if (std::find(m_vecRecentFile.begin(), m_vecRecentFile.end(), strFileName) == m_vecRecentFile.end())
	{
		m_vecRecentFile.push_back(strFileName);
		if (m_vecRecentFile.size() > 10)
			m_vecRecentFile.erase(m_vecRecentFile.begin());
		SaveAppCfg();
	}
}

void CMainDlg::ReloadWorkspaceUIRes()
{
	if (!m_bIsOpen)
		return;

	m_UIResFileMgr.OpenProject(m_strUiresPath);
}

BOOL CMainDlg::OnBtnSave()
{
	if(!m_pXmlEdtior->isValidXml())
	{
		if(IDCANCEL == SMessageBox(m_hWnd,_T("编辑器中的XML文件格式有错误，确定要保存吗？"),_T("提示"),MB_OKCANCEL))
			return FALSE;
	}
	m_pXmlEdtior->SaveFile();
	return TRUE;
}

void CMainDlg::OnBtnNewLayout()
{
	if (m_strProPath.IsEmpty())
	{
		SLOGW()<<"请先打开工程";
		return;
	}
	SDlgNewLayout DlgNewDialog(_T("layout:UIDESIGNER_XML_NEW_LAYOUT"), m_strProPath);
	if (IDOK == DlgNewDialog.DoModal(m_hWnd))
	{
		CopyFile(CSysDataMgr::getSingleton().GetConfigDir() + _T("/LayoutTmpl/Dialog.xml"), DlgNewDialog.m_strPath, FALSE);
		m_UIResFileMgr.NewLayout(DlgNewDialog.m_strName, DlgNewDialog.m_strPath);

		SStringT *strShortPath = new SStringT(DlgNewDialog.m_strPath.Mid(m_strProPath.GetLength() + 1));

		//将文件名插入工程列表
		HSTREEITEM item = m_treePro->InsertItem(DlgNewDialog.m_strName);  //strName = "xml_mainwnd"
		m_treePro->SetItemText(item, DlgNewDialog.m_strName);
		m_treePro->SetItemData(item, (LPARAM)strShortPath);  //strpath = "xml\dlg_maing.xml"
		m_treePro->Invalidate();
	}
}

void CMainDlg::OnBtnNewInclude()
{
	if (m_strProPath.IsEmpty())
	{
		SLOGW()<<"请先打开工程";
		return;
	}
	SDlgNewLayout DlgNewDialog(_T("layout:UIDESIGNER_XML_NEW_LAYOUT"), m_strProPath);
	if (IDOK == DlgNewDialog.DoModal(m_hWnd))
	{
		CopyFile(CSysDataMgr::getSingleton().GetConfigDir() + _T("/LayoutTmpl/Include.xml"), DlgNewDialog.m_strPath, FALSE);
		m_UIResFileMgr.NewLayout(DlgNewDialog.m_strName, DlgNewDialog.m_strPath);

		SStringT *strShortPath = new SStringT(DlgNewDialog.m_strPath.Mid(m_strProPath.GetLength() + 1));

		//将文件名插入工程列表
		HSTREEITEM item = m_treePro->InsertItem(DlgNewDialog.m_strName);  //strName = "xml_mainwnd"
		m_treePro->SetItemText(item, DlgNewDialog.m_strName);
		m_treePro->SetItemData(item, (LPARAM)strShortPath);  //strpath = "xml\dlg_maing.xml"
		m_treePro->Invalidate();

	}
}


void CMainDlg::OnBtnResMgr()
{
	if (m_strProPath.IsEmpty())
	{
		SMessageBox(m_hWnd, _T("请先打开工程"), _T("提示"), MB_OK);
		return;
	}

	SResMgrDlg DlgSkin(m_strUIResFile, &m_UIResFileMgr);
	if (DlgSkin.DoModal(m_hWnd) == IDOK)
	{
		ReloadWorkspaceUIRes();
	}
}

void CMainDlg::OnBtnAbout()
{
	SDlgAbout dlg;
	dlg.DoModal(m_hWnd);
}

void CMainDlg::OnBtnRecentFile()
{
	CRect rect = m_btn_recentFile->GetWindowRect();
	ClientToScreen2(&rect);

	m_RecentFileMenu.TrackPopupMenu(0, rect.left, rect.bottom, m_hWnd);
}

void CMainDlg::OnTreeItemDbClick(IEvtArgs *pEvtBase)
{
	EventTCDbClick *pEvt = (EventTCDbClick*)pEvtBase;
	STreeCtrl *tree = (STreeCtrl*)pEvt->sender;

	SStringT *s = (SStringT*)tree->GetItemData(pEvt->hItem);
	SStringT strLayoutName;
	tree->GetItemText(pEvt->hItem, strLayoutName);

	if(!CheckSave())
		return;

	m_pXmlEdtior->LoadXml(*s, strLayoutName);
	m_editXmlType = XML_LAYOUT;
	UpdateEditorToolbar();
}


// 双击打开文件
void CMainDlg::OnWorkspaceXMLDbClick(IEvtArgs * pEvtBase)
{
	EventLBDbClick *pEvt = (EventLBDbClick*)pEvtBase;
	SListBox *listbox = (SListBox*)pEvt->sender;

	if (pEvt->nCurSel != -1)
	{
		SStringT filename = listbox->GetText(pEvt->nCurSel);
		if(!CheckSave())
			return;
		m_pXmlEdtior->LoadXml(filename,SStringT());
		BOOL bSkin = m_UIResFileMgr.IsSkinXml(filename);
		m_editXmlType = bSkin?XML_SKIN:XML_UNKNOWN;
		UpdateEditorToolbar();
	}
}

void CMainDlg::OnDeleteItem(STreeCtrl *pTreeCtrl,HSTREEITEM hItem,LPARAM lParam)
{
	SStringT * pPath = (SStringT*)lParam;
	delete pPath;
}


BOOL CMainDlg::OnDrop(LPCTSTR pszName)
{
	SStringT strName(pszName);
	if(!strName.EndsWith(_T("idx")))
		return FALSE;
	OpenProject(pszName);
	return TRUE;
}

void CMainDlg::OnInsertWidget(CWidgetTBAdapter::IconInfo *info)
{
	m_pXmlEdtior->InsertWidget(info->strContent);
}

void CMainDlg::OnInertSkin(CSkinTBAdapter::IconInfo * info)
{
	DlgInsertXmlElement dlg(&m_UIResFileMgr,g_SysDataMgr.getSkinDefNode().child(L"skins"),S_CT2W(info->strElement));
	if(IDOK==dlg.DoModal())
	{
		m_pXmlEdtior->InsertElement(dlg.GetXml());
	}
}

void CMainDlg::OnAutoCheck(IEvtArgs *e)
{
	EventSwndStateChanged *e2=sobj_cast<EventSwndStateChanged>(e);
	if(EventSwndStateChanged_CheckState(e2,WndState_Check))
	{
		m_bAutoSave = e2->dwNewState&WndState_Check;
	}
}


void CMainDlg::onScintillaSave(CScintillaWnd *pSci,LPCTSTR pszFileName)
{
}

void CMainDlg::onScintillaAutoComplete(CScintillaWnd *pSci,char ch)
{
	if(m_editXmlType == XML_UNKNOWN)
		return;

	long lStart = pSci->SendEditor(SCI_GETCURRENTPOS, 0, 0);
	int startPos = pSci->SendEditor(SCI_WORDSTARTPOSITION, lStart, true);

	if(ch == '_')
	{//test for skin_xxx
		SStringA strPrefix = pSci->GetNotePart(lStart - 1);
		SStringA str;
		if (strPrefix.CompareNoCase("skin") == 0)
			str = m_UIResFileMgr.GetSkinAutos(S_CA2T(strPrefix,CP_UTF8));
		if (!str.IsEmpty())
		{
			pSci->SendEditor(SCI_AUTOCSHOW, lStart - startPos, (LPARAM)str.c_str());
		}
	}
	else if (ch == '/')
	{
		SStringA clsName = pSci->GetNotePart(lStart - 1);
		SStringA str;
		if (clsName.CompareNoCase("color") == 0)
			str = m_UIResFileMgr.GetColorAutos(_T(""));
		else if (clsName.CompareNoCase("string") == 0)
			str = m_UIResFileMgr.GetStringAutos(_T(""));

		if (!str.IsEmpty())
		{
			pSci->SendEditor(SCI_AUTOCSHOW, lStart - startPos, (LPARAM)(LPCSTR)str);
		}
	}
	else if (ch == '<')
	{//start element
		SStringA strAutoShow = m_editXmlType == XML_LAYOUT? CSysDataMgr::getSingleton().GetCtrlAutos()
			:CSysDataMgr::getSingleton().GetSkinAutos();
		if (!strAutoShow.IsEmpty())
		{
			pSci->SendEditor(SCI_AUTOCSHOW, lStart - startPos, (LPARAM)strAutoShow.c_str());
		}
	}
	else if (ch >= 'a' && ch <= 'z')
	{
		int tagpos = -1;
		SStringA tagname = pSci->GetHtmlTagName(tagpos);
		SStringA strPrefix = pSci->GetRange(lStart-2,lStart-1);

		if (!tagname.IsEmpty() && strPrefix==" " && strPrefix!="\t")
		{//编辑的元素name不为空，而且当前不是编辑属性
			SStringW strTag = S_CA2W(tagname,CP_UTF8);
			SStringA str = m_editXmlType == XML_LAYOUT?CSysDataMgr::getSingleton().GetCtrlAttrAutos(strTag)
				:CSysDataMgr::getSingleton().GetSkinAttrAutos(strTag);
			if (!str.IsEmpty())
			{	// 自动完成字串要进行升充排列, 否则功能不正常
				pSci->SendEditor(SCI_AUTOCSHOW, lStart - startPos, (LPARAM)(LPCSTR)str);
			}
		}
	}
}

void CMainDlg::UpdateEditorToolbar()
{
	switch(m_editXmlType)
	{
	case XML_LAYOUT:
		m_lvWidget->SetVisible(TRUE,TRUE);
		m_lvSkin->SetVisible(FALSE,TRUE);
		break;
	case XML_SKIN:
		m_lvWidget->SetVisible(FALSE,TRUE);
		m_lvSkin->SetVisible(TRUE,TRUE);
		break;
	default:
		m_lvWidget->SetVisible(FALSE,TRUE);
		m_lvSkin->SetVisible(FALSE,TRUE);
		break;
	}
}

void CMainDlg::OnDestroy()
{
	SaveAppCfg();
	SHostWnd::OnDestroy();
}

bool CMainDlg::CheckSave()
{
	if(m_pXmlEdtior->isDirty())
	{
		if(m_bAutoSave) 
		{
			if(!OnBtnSave())
				return false;
		}else
		{
			UINT ret = SMessageBox(m_hWnd,_T("当前文件还没有保存，现在保存吗？"),_T("提示"),MB_YESNOCANCEL);
			if(ret == IDYES)
			{
				if(!OnBtnSave())
					return false;
			}else if(ret == IDCANCEL)
			{
				return false;
			}
		}
	}
	return true;
}

void CMainDlg::UpdateToolbar()
{
	FindChildByID(R.id.toolbar_btn_Open)->EnableWindow(!m_bIsOpen,TRUE);
	FindChildByID(R.id.toolbar_btn_recent)->EnableWindow(!m_bIsOpen,TRUE);
	
	FindChildByID(R.id.toolbar_btn_NewInclude)->EnableWindow(m_bIsOpen,TRUE);
	FindChildByID(R.id.toolbar_btn_NewLayout)->EnableWindow(m_bIsOpen,TRUE);
	FindChildByID(R.id.toolbar_btn_savexml)->EnableWindow(m_bIsOpen,TRUE);
	FindChildByID(R.id.toolbar_btn_resmgr)->EnableWindow(m_bIsOpen,TRUE);
	FindChildByID(R.id.toolbar_btn_Close)->EnableWindow(m_bIsOpen,TRUE);
}
