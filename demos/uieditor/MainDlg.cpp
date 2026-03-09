// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include "helpapi.h"
#include "Dialog/DlgNewLayout.h"
#include "Dialog/DlgAbout.h"
#include "Dialog/DlgInput.h"
#include "SysdataMgr.h"
#include <vector>
#include <algorithm>
#include <helper/SMenu.h>
#include <helper/SMenuEx.h>
#include "SImgCanvas.h"
#include "SkinViewerDlg.h"
#include "Dialog/DlgInsertXmlElement.h"
#include <helper/SFunctor.hpp>
#include <shlwapi.h>
#include <atl.mini/SComCli.h>
#include "FileTreeDragdrop.h"
#include "FileOperationManager.h"
#include "UIResManager.h"

#define kLogTag "maindlg"

#define UIRES_FILE	L"uires.idx"

//----------------------------------------------------------------------------
CMainDlg::CMainDlg() 
: SHostWnd(_T("LAYOUT:XML_MAINWND"))
,m_pXmlEdtior(NULL)
,m_pImageViewer(NULL)
,m_bAutoSave(TRUE)
,m_editXmlType(FT_UNKNOWN)
,m_UIResFileMgr(this)
,m_bIsOpen(FALSE)
,m_pFileOperationManager(NULL)
,m_pUIResManager(NULL)
,m_pToolbarManager(NULL)
{
	m_skinPool.Attach(new SSkinPool());
	SUiDef::getSingleton().PushSkinPool(m_skinPool);
}

CMainDlg::~CMainDlg()
{
	if(m_pXmlEdtior)
	{
		delete m_pXmlEdtior;
	}
	
	if(m_pImageViewer)
	{
		delete m_pImageViewer;
	}
	
	if (m_pFileOperationManager)
	{
		delete m_pFileOperationManager;
	}
	
	if (m_pUIResManager)
	{
		delete m_pUIResManager;
	}
	
	if (m_pToolbarManager)
	{
		delete m_pToolbarManager;
	}
	
	SUiDef::getSingleton().PopSkinPool(m_skinPool);
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	::RegisterDragDrop(hWnd, GetDropTarget());
	// 初始化文件树视图
	m_treeView = FindChildByID2<STreeView>(R.id.workspace_treeview);
	if (m_treeView)
	{
		m_pFileTreeAdapter.Attach(new CFileTreeAdapter(&m_UIResFileMgr));
		m_treeView->SetAdapter(m_pFileTreeAdapter);
		// 初始化路径监视器
		m_pathMonitor.AddListener(this);

		IDropTarget *pDT = new FileTreeDropTarget(m_treeView);
		RegisterDragDrop(m_treeView->GetSwnd(), pDT);
		pDT->Release();
	}
	m_RecentFileMenu.LoadMenu(UIRES.smenu.menu_recent);
	LoadAppCfg();

    FindChildByID(R.id.chk_autosave)->SetCheck(m_bAutoSave);


	//======================================================================
	m_pXmlEdtior = new CXmlEditor(this);
	m_pXmlEdtior->Init(GetRoot());
    m_pImageViewer = new CImageViewer(this);
	m_pImageViewer->Init(GetRoot());

	// 初始化管理类
	m_pFileOperationManager = new CFileOperationManager(m_treeView, m_pFileTreeAdapter, m_hWnd);
	m_pUIResManager = new CUIResManager(&m_UIResFileMgr);
	m_pToolbarManager = new CToolbarManager(m_skinPool);

	// 初始化工具栏
	m_pToolbarManager->InitWidgetToolbar(GetRoot(), m_pXmlEdtior);
	m_pToolbarManager->InitSkinToolbar(GetRoot());

	HRESULT hr = ::RegisterDragDrop(m_hWnd, GetDropTarget());
	RegisterDragDrop(m_treeView->GetSwnd(), new CDropTarget(this));
	SWindow* MainWnd = FindChildByName2<SWindow>("UI_main_caption");
	if (MainWnd)
		RegisterDragDrop(MainWnd->GetSwnd(), new CDropTarget(this));
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

void CMainDlg::OnClose()
{
	// 停止路径监视器
	m_pathMonitor.Stop();
	m_pathMonitor.RemoveListener(this);
	
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
		else if(nID == R.id.menu_file_openproject){
			OnBtnOpen();
		}else if(nID == R.id.menu_file_closeproject){
			OnBtnClose();
		}else if(nID == R.id.menu_file_save_xml){
			OnBtnSave();
		}else if(nID == R.id.menu_file_newlayout){
			OnBtnNewLayout();
		}else if(nID == R.id.menu_file_newinclude){
			OnBtnNewInclude();
        }
        else if (nID == R.id.menu_tool_preview)
        {
			OnBtnPreview();
        }
		else if(nID == R.id.menu_tool_format_xml){
			OnBtnFormatXml();
		}else if(nID == R.id.menu_tool_view_skin){
			 OnBtnViewSkin();
		}else if(nID == R.id.menu_about){
			OnBtnAbout();
		}else if(nID == R.id.menu_file_exit){
			OnClose();
		}else if(nID == R.id.menu_view_property){
			SWindow *pProp = FindChildByID(R.id.property_panel_dock);
			pProp->SetVisible(!pProp->IsVisible(), TRUE);
		}else if(nID == R.id.menu_view_toolbar){
			SWindow *pToolbar = FindChildByID(R.id.tb_main);
			pToolbar->SetVisible(!pToolbar->IsVisible(), TRUE);
		}else if(nID==R.id.menu_view_statusbar){
			SWindow *pStatusbar = FindChildByID(R.id.wnd_status);
			pStatusbar->SetVisible(!pStatusbar->IsVisible(), TRUE);
		}
	}
}
void CMainDlg::OnTvKeyDown(IEvtArgs *e)
{
	EventKeyDown * e2 = sobj_cast<EventKeyDown>(e);
	if (m_pFileOperationManager)
	{
		m_pFileOperationManager->HandleTreeViewKeyboardShortcut(e2->nChar);
	}
}


// 文件操作相关方法已移至CFileOperationManager


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
	m_pXmlEdtior->CloseXml();
	m_pImageViewer->Clear();
	m_pFileTreeAdapter->Clear();
	m_UIResFileMgr.CloseProject();	

	m_editXmlType = FT_UNKNOWN;
	UpdateEditorToolbar();

	m_bIsOpen = FALSE;
	return true;
}

//打开工程
BOOL CMainDlg::OpenProject(SStringT strFileName)
{
	SStringT strFile = strFileName;
	int n = strFileName.ReverseFind(TPATH_SLASH);
	m_strUiresPath = strFileName;
	m_strProPath = strFileName.Mid(0, n);
	
	SLOGI() << "open project:" << strFileName.c_str();
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
		
		return FALSE;
	}

	if(!m_UIResFileMgr.OpenProject(m_strUiresPath))
	{
		SLOGW()<<"open project file failed";
		return FALSE;
	}

	m_pXmlEdtior->SetProjectPath(strFileName);

    m_bIsOpen = TRUE;
	
	// 更新文件树的根目录为项目目录
	if (m_pFileTreeAdapter)
	{
		m_pFileTreeAdapter->SetRootPath(m_strProPath);
	}
	
	if (std::find(m_vecRecentFile.begin(), m_vecRecentFile.end(), strFileName) == m_vecRecentFile.end())
	{
		m_vecRecentFile.push_back(strFileName);
		if (m_vecRecentFile.size() > 10)
			m_vecRecentFile.erase(m_vecRecentFile.begin());
		SaveAppCfg();
	}
    return TRUE;
}

void CMainDlg::ReloadWorkspaceUIRes()
{
	if (!m_bIsOpen)
		return;

	m_UIResFileMgr.OpenProject(m_strUiresPath);
}

// 剪贴板相关方法已移至CClipboardManager

BOOL CMainDlg::OnBtnSave()
{
    return m_pXmlEdtior->OnSaveXml();
}

BOOL CMainDlg::NewLayout(const SStringT &strPath, const SStringT &strName)
{
	BOOL bRet = m_UIResFileMgr.NewLayout(strName, strPath);
	if(bRet){
		SStringT strType;
		m_UIResFileMgr.IsLayoutXml(strPath, strType);
		m_pXmlEdtior->LoadXml(strPath, strType);
        FindChildByID(R.id.property_panel_dock)->SetVisible(TRUE, TRUE);
	}
	return bRet;
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
		NewLayout(DlgNewDialog.m_strPath, DlgNewDialog.m_strName);
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
	}
}

void CMainDlg::OnBtnAbout()
{
	SDlgAbout dlg;
	dlg.DoModal(m_hWnd);
}

void CMainDlg::OnBtnRecentFile(IEvtArgs *e)
{
	SToolBar *pToolBar = sobj_cast<SToolBar>(e->Sender());
	CRect rcItem = pToolBar->GetItemRect(pToolBar->CommandToIndex(R.id.tb_main_openproject));
	ClientToScreen2(&rcItem);
	m_RecentFileMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_TOPALIGN, rcItem.left, rcItem.bottom, m_hWnd);
}

void CMainDlg::OnBtnFormatXml()
{
	m_pXmlEdtior->OnFormatXml();
}

void CMainDlg::OnTvEventOfPanel(IEvtArgs *e)
{
	enum{
		menu_new_xml = 190,
		menu_new_folder = 191,
		menu_new_layout = 192,
		menu_new_include = 193,
		menu_copy = 200,
		menu_cut = 201,
		menu_paste = 202,
		menu_delete = 203,
		menu_rename = 204,
		menu_explorer = 205,
		menu_copy_path = 206,
		menu_add_to_uires = 207,
		menu_add_skin = 208,
	};

	EventOfPanel *e2 = sobj_cast<EventOfPanel>(e);
	if(!e2) return;
	if(e2->pOrgEvt->GetID()==EventItemPanelRclick::EventID)
	{
		EventItemPanelRclick *pEvt = sobj_cast<EventItemPanelRclick>(e2->pOrgEvt);
		SMenuEx menu;
		menu.LoadMenu(UIRES.smenu.menu_file_ctx);
		SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pEvt->sender);
		HSTREEITEM hItem = (HSTREEITEM)pItemPanel->GetItemIndex();
		CPoint pt(GET_X_LPARAM(pEvt->lParam), GET_Y_LPARAM(pEvt->lParam));
		CRect rc(pt, CSize());
		pItemPanel->FrameToHost(&rc);
		ClientToScreen2(rc);
		const FileItemData& itemData = m_pFileTreeAdapter->GetItemData(hItem);
		if(!itemData.bIsDir){
			menu.EnableMenuItem(menu_new_xml, MF_DISABLED);
			menu.EnableMenuItem(menu_new_folder, MF_DISABLED);
			menu.EnableMenuItem(menu_new_layout, MF_DISABLED);
			menu.EnableMenuItem(menu_new_include, MF_DISABLED);
		}
	
		// 根据 clipboard 状态更新菜单项
		menu.EnableMenuItem(menu_paste, m_pFileOperationManager->HasClipboardData()?MF_ENABLED:MF_DISABLED);
	
		int nId = menu.TrackPopupMenu(TPM_RETURNCMD, rc.left, rc.top, m_hWnd);
		SLOGI()<<"menu id:"<<nId;
		if(nId>0)
		{
			switch(nId)
			{
				case menu_new_xml://new Xml
				{
					if (m_strProPath.IsEmpty())
					{
						SLOGW() << "请先打开工程";
						return;
					}
					
					const FileItemData& itemData = m_pFileTreeAdapter->GetItemData(hItem);
					SStringT strDir = itemData.bIsDir ? itemData.strPath : itemData.strPath.Left(itemData.strPath.ReverseFind(_T('/')));
					SDlgInput dlgInput;
					dlgInput.m_strTitle = _T("请输入文件名,不含扩展名");
					dlgInput.m_strValue = _T("NewXml");
					if (IDOK == dlgInput.DoModal(m_hWnd) && !dlgInput.m_strValue.IsEmpty())
					{
						SStringT strNewName = CFileTreeAdapter::GenerateAvailableName(strDir, dlgInput.m_strValue, _T("xml"));
						m_pFileTreeAdapter->CreateItem(hItem, strNewName, FALSE);
					}
				}
				break;
				case menu_new_folder://new folder
				{
					if (m_strProPath.IsEmpty())
					{
						SLOGW() << "请先打开工程";
						return;
					}
					
					SStringT strDir = itemData.bIsDir ? itemData.strPath : itemData.strPath.Left(itemData.strPath.ReverseFind(_T('/')));
					SDlgInput dlgInput;
					dlgInput.m_strTitle = _T("请输入文件夹名");
					dlgInput.m_strValue = _T("NewFolder");
					if (IDOK == dlgInput.DoModal(m_hWnd) && !dlgInput.m_strValue.IsEmpty())
					{
						SStringT strNewName = CFileTreeAdapter::GenerateAvailableName(strDir, dlgInput.m_strValue, _T(""));
						m_pFileTreeAdapter->CreateItem(hItem, strNewName, TRUE);
					}
				}
				break;
				case menu_new_layout://new layout
				{
					if (m_strProPath.IsEmpty())
					{
						SLOGW() << "请先打开工程";
						return;
					}
					
					const FileItemData& itemData = m_pFileTreeAdapter->GetItemData(hItem);
					SASSERT(itemData.bIsDir);
					SDlgNewLayout DlgNewDialog(_T("layout:UIDESIGNER_XML_NEW_LAYOUT"), m_strProPath, itemData.strName);
					if (IDOK == DlgNewDialog.DoModal(m_hWnd))
					{
						CopyFile(CSysDataMgr::getSingleton().GetConfigDir() + _T("/LayoutTmpl/Dialog.xml"), DlgNewDialog.m_strPath, FALSE);
						NewLayout(DlgNewDialog.m_strPath, DlgNewDialog.m_strName);
					}
				}
				break;
			case menu_new_include://new include
				{
					if (m_strProPath.IsEmpty())
					{
						SLOGW() << "请先打开工程";
						return;
					}
					
					const FileItemData& itemData = m_pFileTreeAdapter->GetItemData(hItem);
					SASSERT(itemData.bIsDir);
					SDlgNewLayout DlgNewDialog(_T("layout:UIDESIGNER_XML_NEW_LAYOUT"), m_strProPath, itemData.strName);
					if (IDOK == DlgNewDialog.DoModal(m_hWnd))
					{
						CopyFile(CSysDataMgr::getSingleton().GetConfigDir() + _T("/LayoutTmpl/Include.xml"), DlgNewDialog.m_strPath, FALSE);
						NewLayout(DlgNewDialog.m_strPath, DlgNewDialog.m_strName);
					}
					break;
				}
			case menu_copy://复制
			{
				m_pFileOperationManager->OnFileCopy();
			}
			break;
		case menu_cut://剪切
			{
				m_pFileOperationManager->OnFileCut();
			}
			break;
            case menu_paste: // 粘贴
            {
                m_pFileOperationManager->OnFilePaste(hItem);
            }
            break;
            case menu_delete: // 删除
            {
                m_pFileOperationManager->OnFileDelete(hItem);
            }
            break;
            case menu_rename: // 重命名
                m_pFileOperationManager->OnFileRename(hItem);
                break;
            case menu_explorer: // reveal in explorer
            {
                const FileItemData &itemData = m_pFileTreeAdapter->GetItemData(hItem);
                SStringT strPath = itemData.strPath;
                strPath.ReplaceChar(_T('/'), PATH_SLASH);
                ShellExecute(0, _T("open"), _T("explorer.exe"), _T("/select,") + strPath, NULL, SW_SHOW);
                SLOGI() << "Reveal in explorer: " << strPath.c_str();
            }
            break;
			case menu_copy_path:
			{
				const FileItemData &itemData = m_pFileTreeAdapter->GetItemData(hItem);
				SStringW strPath = S_CT2W(itemData.strPath);
				HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (strPath.GetLength() + 1) * sizeof(wchar_t));
				if(hMem)
				{
					wchar_t *pMem = (wchar_t *)GlobalLock(hMem);
					wcsncpy(pMem, strPath, strPath.GetLength()+1);
					GlobalUnlock(hMem);
					OpenClipboard(m_hWnd);
					EmptyClipboard();
					SetClipboardData(CF_UNICODETEXT, hMem);
					CloseClipboard();
					GlobalFree(hMem);
				}
			}
			break;
            case menu_add_to_uires: // add to uires.idx
                m_pUIResManager->AddUires(hItem, m_treeView, m_pFileTreeAdapter);
                break;
            case menu_add_skin:
                m_pUIResManager->AddUires(hItem, m_treeView, m_pFileTreeAdapter); // add to uires.idx first
                m_pUIResManager->AddSkin(hItem, m_treeView, m_pFileTreeAdapter);
                break;
            }
        }
    }
	else if(e2->pOrgEvt->GetID()==EventItemPanelDbclick::EventID)
	{
		EventItemPanelDbclick *pEvt = sobj_cast<EventItemPanelDbclick>(e2->pOrgEvt);
		SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pEvt->Sender());
		HSTREEITEM hItem = (HSTREEITEM)pItemPanel->GetItemIndex();
		const FileItemData& itemData = m_pFileTreeAdapter->GetItemData(hItem);
		SLOGI()<<"File:"<<itemData.strPath.c_str();

		if(!CheckSave())
			return;
		if(itemData.bIsDir)
		{
			m_pFileTreeAdapter->ExpandItem(hItem, TVC_TOGGLE);
			return;
		}
		int type = m_UIResFileMgr.GetFileType(itemData.strPath);
		SStringT strPath = itemData.strPath;
		SStringT layoutId;
		if(type == FT_LAYOUT_XML){
			m_UIResFileMgr.IsLayoutXml(strPath, layoutId);
		}
		STabCtrl *pTab = FindChildByID2<STabCtrl>(R.id.tab_editor);
		if(type != FT_IMAGE){
			pTab->SetCurSel(0);
			m_editXmlType = type;
			m_pXmlEdtior->LoadXml(strPath, layoutId);
            FindChildByID(R.id.property_panel_dock)->SetVisible(TRUE, TRUE);
			UpdateEditorToolbar();
		}else{
			// 打开图片查看器
			pTab->SetCurSel(1);
			if(m_pImageViewer)
			{
				m_pImageViewer->LoadImage(strPath);
			}
		}
	}
	else if(e2->pOrgEvt->GetID()==EventMouseClick::EventID)
	{
		EventMouseClick *pEvt = sobj_cast<EventMouseClick>(e2->pOrgEvt);
		if(pEvt->clickId==MOUSE_LBTN_DOWN){
			m_tvClickPt = pEvt->pt;
			//		SLOGI()<<"Mouse click:"<<pEvt->pt.x<<","<<pEvt->pt.y;
		}else if(pEvt->clickId == MOUSE_LBTN_UP){
			m_tvClickPt = CPoint(-1, -1);
			//		SLOGI()<<"Mouse up:"<<pEvt->pt.x<<","<<pEvt->pt.y;
		}
    }
    else if (m_tvClickPt.x!=-1 && e2->pOrgEvt->GetID() == EventSwndMouseMove::EventID)
    {
		EventSwndMouseMove *pEvt = sobj_cast<EventSwndMouseMove>(e2->pOrgEvt);
		CPoint pt = pEvt->pt;
		// 检查是否开始拖动（移动距离>2像素且按住左键）
		if((pEvt->nFlags & MK_LBUTTON) && (abs(m_tvClickPt.x-pt.x) > 2 || abs(m_tvClickPt.y-pt.y) > 2))
		{
            SWindow *pSender = sobj_cast<SWindow>(pEvt->sender);
            SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pSender->GetRoot());
			if(!pItemPanel) return;
			
			HSTREEITEM hItem = (HSTREEITEM)pItemPanel->GetItemIndex();
			SWND swndCapture = pSender->GetCapture();
		    SWindow *pCapture = SWindowMgr::GetWindow(swndCapture); 
		    if (pCapture)
		    {
		        pCapture->ReleaseCapture();
		    }
			
			// 执行拖放操作
			m_pFileOperationManager->StartDrag(hItem, m_tvClickPt);
			
		    pCapture = SWindowMgr::GetWindow(swndCapture); 
			if (pCapture)
		    {//restore item state.
		        pCapture->SetCapture();
		        CPoint pt;
		        GetCursorPos(&pt);
		        ScreenToClient(&pt);
		        SendMessage(WM_LBUTTONUP, 0, MAKELPARAM(pt.x, pt.y));
		        SendMessage(WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
		    }
			
			m_tvClickPt = CPoint(-1, -1);
		}
	}
}

BOOL CMainDlg::OnDrop(LPCTSTR pszName)
{
	SStringT strName(pszName);
	if(!strName.EndsWith(_T("idx")))
		return FALSE;
	OpenProject(pszName);
	return TRUE;
}

void CMainDlg::OnAutoCheck(IEvtArgs *e)
{
	EventSwndStateChanged *e2=sobj_cast<EventSwndStateChanged>(e);
	if(EventSwndStateChanged_CheckState(e2,WndState_Check))
	{
		m_bAutoSave = e2->dwNewState&WndState_Check;
	}
}
void CMainDlg::UpdateEditorToolbar()
{
    m_pToolbarManager->UpdateEditorToolbar(m_editXmlType);
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

void CMainDlg::OnFileChanged(LPCTSTR pszFile, CPathMonitor::Flag nFlag)
{
    STaskHelper::post(GetMsgLoop(), this, &CMainDlg::_OnFileChanged, tstring(pszFile), nFlag);
}

void CMainDlg::_OnFileChanged(tstring& strFile, CPathMonitor::Flag nFlag)
{
    m_UIResFileMgr.OnFileChanged(strFile, nFlag);
    SStringT strFilePath = strFile.c_str();
    switch (nFlag)
    {
    case CPathMonitor::FILE_DELETED:
        m_pFileTreeAdapter->RemoveItemByPath(strFilePath);
        if (m_pXmlEdtior->GetCurrentXml() == strFilePath)
        {
            m_pXmlEdtior->CloseXml();
			m_editXmlType = FT_UNKNOWN;
			UpdateEditorToolbar();
		}
        break;
        
    case CPathMonitor::FILE_CREATED:
        // 增加文件/夹：如果上一级目录已经展开，则直接增加这个项，否则不处理
        m_pFileTreeAdapter->AddItemByPath(strFilePath);
        break;
    case CPathMonitor::FILE_MODIFIED:
        if (m_pXmlEdtior->GetCurrentXml() == strFilePath)
        {
            m_pXmlEdtior->Reload();
        }
        break;
    }
}

void CMainDlg::OnBtnViewSkin()
{
	CSkinViewerDlg dlg(&m_UIResFileMgr);
	dlg.DoModal();
}

void CMainDlg::OnBtnPreview()
{
	m_pXmlEdtior->StartPreviewProcess();
}

void CMainDlg::OnTbWidgetClick(IEvtArgs *e)
{
	EventToolBarCmd *e2=sobj_cast<EventToolBarCmd>(e);
	ToolBarItem item;
	m_pToolbarManager->GetWidgetToolbar()->GetItemInfo(e2->iItem,&item);
    SStringW strContent = m_pToolbarManager->GetWidgetContent(item.strText);
    if (!strContent.IsEmpty())
    {
        m_pXmlEdtior->InsertWidget(S_CW2A(strContent, CP_UTF8));
	}
}

void CMainDlg::OnTbSkinClick(IEvtArgs *e)
{
	EventToolBarCmd *e2=sobj_cast<EventToolBarCmd>(e);
	ToolBarItem item;
    m_pToolbarManager->GetSkinToolbar()->GetItemInfo(e2->iItem, &item);
    DlgInsertXmlElement dlg(&m_UIResFileMgr, CSysDataMgr::getSingleton().getSkinDefNode().child(L"skins"), S_CT2W(item.strText));
	if(IDOK==dlg.DoModal())
	{
		m_pXmlEdtior->InsertElement(dlg.GetXml());
	}
}

void CMainDlg::OnUpdateCmdUI(IEvtArgs *e){
	EventUpdateCmdUI *e2=sobj_cast<EventUpdateCmdUI>(e);
	if(e2->nCmdId == R.id.tb_main_openproject){
		e2->bEnable = !m_bIsOpen;
	}else if(e2->nCmdId == R.id.tb_main_closeproject){
		e2->bEnable = m_bIsOpen;
	}else if(e2->nCmdId == R.id.tb_main_savexml){
		e2->bEnable = m_bIsOpen && m_pXmlEdtior->isDirty();
	}else if(e2->nCmdId == R.id.tb_main_viewskin || e2->nCmdId == R.id.menu_tool_view_skin){
		e2->bEnable = m_bIsOpen;
	}else if(e2->nCmdId == R.id.tb_main_formatxml || e2->nCmdId == R.id.menu_tool_format_xml){
		e2->bEnable = m_bIsOpen && (m_editXmlType != FT_UNKNOWN);
	}else if(e2->nCmdId == R.id.tb_main_newinclude || e2->nCmdId == R.id.tb_main_newlayout){
		e2->bEnable = m_bIsOpen;
    }
    else if (e2->nCmdId == R.id.menu_tool_preview)
    {
		e2->bEnable = m_bIsOpen && (m_editXmlType == FT_LAYOUT_XML);
	}else if(e2->nCmdId == R.id.menu_view_property){
		e2->bEnable = m_bIsOpen && m_editXmlType == FT_LAYOUT_XML;
		e2->bChecked = FindChildByID(R.id.property_panel_dock)->IsVisible();
	}else if(e2->nCmdId == R.id.menu_view_toolbar){
		e2->bChecked = FindChildByID(R.id.tb_main)->IsVisible();
	}else if(e2->nCmdId == R.id.menu_view_statusbar){
		e2->bChecked = FindChildByID(R.id.wnd_status)->IsVisible();
	}
}

void CMainDlg::OnTbMainClick(IEvtArgs *e)
{
	EventToolBarCmd *e2=sobj_cast<EventToolBarCmd>(e);
	if(e2->nCmdId == R.id.tb_main_openproject){
		if(e2->bDropDown){
			OnBtnRecentFile(e);
		}else{
			OnBtnOpen();
		}
	}else if(e2->nCmdId == R.id.tb_main_closeproject){ 
		OnBtnClose();
	}else if(e2->nCmdId == R.id.tb_main_savexml){
		OnBtnSave();
	}else if(e2->nCmdId == R.id.tb_main_viewskin){
		OnBtnViewSkin();
	}else if(e2->nCmdId == R.id.tb_main_formatxml){
		OnBtnFormatXml();
	}else if(e2->nCmdId == R.id.tb_main_about){
		OnBtnAbout();
	}else if(e2->nCmdId == R.id.tb_main_newlayout){
		OnBtnNewLayout();
	}else if(e2->nCmdId == R.id.tb_main_newinclude){
		OnBtnNewInclude();
	}
}

void CMainDlg::OnMenuSelect(IEvtArgs *e){
	EventSelectMenu *e2 = sobj_cast<EventSelectMenu>(e);
	OnCommand(0,e2->nMenuId,NULL);
}

void CMainDlg::OnUpdateCmdTip(IEvtArgs *e)
{
    SWindow *pTxtStatus = FindChildByID(R.id.txt_status);
    EventUpdateCmdTip *e2 = sobj_cast<EventUpdateCmdTip>(e);
    if (e2->strTip)
    {
        pTxtStatus->SetWindowText(SStringT(e2->strTip));
	}
	else if (e2->nCmdId >= 0) {
        if (e2->nCmdId == R.id.menu_file_openproject)
        {
			SStringT strTip = S_CW2T(GETSTRING(R.string.open_prject));
			pTxtStatus->SetWindowText(strTip);
		}
		else if (e2->nCmdId == R.id.menu_file_closeproject)
		{
			SStringT strTip = S_CW2T(GETSTRING(R.string.close_prject));
			pTxtStatus->SetWindowText(strTip);
		}
		else if (e2->nCmdId == R.id.menu_file_exit)
		{
			SStringT strTip = S_CW2T(GETSTRING(R.string.exit));
			pTxtStatus->SetWindowText(strTip);
		}
		else if (e2->nCmdId == R.id.menu_tool_view_skin)
		{
			SStringT strTip = S_CW2T(GETSTRING(R.string.view_skin));
			pTxtStatus->SetWindowText(strTip);
		}
		 else if (e2->nCmdId == R.id.menu_tool_format_xml)
		 {
			 SStringT strTip = S_CW2T(GETSTRING(R.string.format_xml));
             pTxtStatus->SetWindowText(strTip);
        }
	}else
    {
        SStringT strDefaultTip = S_CW2T(GETSTRING(R.string.idlemsg));
        pTxtStatus->SetWindowText(strDefaultTip);
    }
}