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
,m_uClipboardFormat(0)
{
	// 注册自定义 clipboard format
	m_uClipboardFormat = RegisterClipboardFormat(_T("SOUI_UIEDITOR_FILE_ITEMS"));
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
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	::RegisterDragDrop(hWnd, GetDropTarget());

	m_btn_recentFile = FindChildByName2<SButton>(L"toolbar_btn_recent");
	
	m_tabWorkspace = FindChildByName2<STabCtrl>(L"workspace_tab");

	m_lvSkin = FindChildByName2<SListView>(L"lv_tb_skin");
	m_lvWidget = FindChildByName2<SListView>(L"lv_tb_widget");

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
	m_pXmlEdtior->Init(GetRoot(),this);
    m_pImageViewer = new CImageViewer(this);
	m_pImageViewer->Init(GetRoot());

	{//init control listbox
		CWidgetTBAdapter * pAdapter = new CWidgetTBAdapter(CSysDataMgr::getSingleton().getCtrlDefNode(),this);
		m_lvWidget->SetAdapter(pAdapter);
		pAdapter->Release();
	}
	{//init skin listbox
		CSkinTBAdapter * pAdapter = new CSkinTBAdapter(CSysDataMgr::getSingleton().getSkinDefNode(),this);
		m_lvSkin->SetAdapter(pAdapter);
		pAdapter->Release();
	}


	HRESULT hr = ::RegisterDragDrop(m_hWnd, GetDropTarget());
	RegisterDragDrop(m_treeView->GetSwnd(), new CDropTarget(this));
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
std::vector<HSTREEITEM> CMainDlg::GetSelectedItems(HSTREEITEM hDefaultItem /*= NULL*/)
{
	std::vector<HSTREEITEM> selectedItems;
	int selCount = m_treeView->GetSelItemCount();
	if(selCount > 0)
	{
		HSTREEITEM *pItems = new HSTREEITEM[selCount];
		m_treeView->GetSelItems(pItems, selCount);
		for(int i = 0; i < selCount; i++)
		{
			selectedItems.push_back(pItems[i]);
		}
		delete []pItems;
	}
	else if(hDefaultItem)
	{
		selectedItems.push_back(hDefaultItem);
	}
	return selectedItems;
}

void CMainDlg::OnTvKeyDown(IEvtArgs *e)
{
	EventKeyDown * e2 = sobj_cast<EventKeyDown>(e);
	HandleTreeViewKeyboardShortcut(e2->nChar);
}

BOOL CMainDlg::HandleTreeViewKeyboardShortcut(UINT nChar)
{
	if(!m_treeView || !m_bIsOpen)
		return FALSE;

	BOOL bCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	HSTREEITEM hItem = m_treeView->GetSel();
	if(bCtrlPressed)
	{
		if(nChar == _T('C') || nChar == _T('c'))
		{
			OnFileCopy();
			return TRUE;
		}
		else if(nChar == _T('V') || nChar == _T('v'))
		{
			if(hItem == ITEM_NULL)
				return FALSE;
			
			OnFilePaste(hItem);
			return TRUE;
		}
		else if(nChar == _T('X') || nChar == _T('x'))
		{
			OnFileCut();
			return TRUE;
		}
    }
    else if (nChar == VK_DELETE || nChar == VK_BACK)
    {
        OnFileDelete(ITEM_NULL);
        return TRUE;
    }
    else if (nChar == VK_F2)
    {
        if (hItem == ITEM_NULL)
            return FALSE;
        OnFileRename(hItem);
        return TRUE;
    }
	return FALSE;
}


enum {
	file_op_copy=1,
	file_op_cut=2,
};
void CMainDlg::OnFileCopy()
{
	std::vector<HSTREEITEM> selectedItems = GetSelectedItems();
	if(!selectedItems.empty())
	{
		// 清除之前的剪切状态
		m_pFileTreeAdapter->ClearAllCutStates();
		SerializeItemsToClipboard(selectedItems, file_op_copy);
	}
}

void CMainDlg::OnFileCut()
{
	std::vector<HSTREEITEM> selectedItems = GetSelectedItems();
	if(!selectedItems.empty())
	{
		// 清除之前的剪切状态
		m_pFileTreeAdapter->ClearAllCutStates();
		// 标记当前选中项为剪切状态
		for(std::vector<HSTREEITEM>::iterator it = selectedItems.begin();it!= selectedItems.end();it++)
		{
			m_pFileTreeAdapter->SetItemCutState(*it, TRUE);
		}
		SerializeItemsToClipboard(selectedItems, file_op_cut);
	}
}

void CMainDlg::OnFileDelete(HSTREEITEM hItem)
{
    std::vector<HSTREEITEM> itemsToDelete = GetSelectedItems(hItem);
    if (!itemsToDelete.empty())
    {
        SStringT strMsg = _T("确定要删除") + SStringT().Format(_T("%d"), itemsToDelete.size()) + _T("个项目吗？");
        if (SMessageBox(m_hWnd, strMsg, _T("提示"), MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            for (size_t i = 0; i < itemsToDelete.size(); i++)
            {
                m_pFileTreeAdapter->RemoveItem(itemsToDelete[i]);
            }
        }
    }
}

void CMainDlg::OnFileRename(HSTREEITEM hItem)
{
    const FileItemData &itemData = m_pFileTreeAdapter->GetItemData(hItem);
    SDlgInput dlgInput;
    dlgInput.m_strTitle = _T("请输入新名称");
    dlgInput.m_strValue = itemData.strName;
    if (IDOK == dlgInput.DoModal(m_hWnd) && !dlgInput.m_strValue.IsEmpty())
    {
        ::MoveFile(itemData.strPath, itemData.strPath.Left(itemData.strPath.ReverseFind(_T('/')) + 1) + dlgInput.m_strValue);
    }
}

void CMainDlg::OnFilePaste()
{
	HSTREEITEM hItem = m_treeView->GetSel();
	if(hItem)
	{
		OnFilePaste(hItem);
	}
}

void CMainDlg::OnFilePaste(HSTREEITEM hItem)
{
	std::vector<SStringT> vecClipboardItems;
	int nClipboardOperation = 0;
	BOOL bFromClipboard = DeserializeItemsFromClipboard(vecClipboardItems, nClipboardOperation);
	
	if(bFromClipboard && !vecClipboardItems.empty() && nClipboardOperation > 0)
	{
		const FileItemData& destItemData = m_pFileTreeAdapter->GetItemData(hItem);
		SStringT strDestPath = destItemData.bIsDir ? destItemData.strPath : destItemData.strPath.Left(destItemData.strPath.ReverseFind(_T('/')));
		
		for(size_t i = 0; i < vecClipboardItems.size(); i++)
		{
			SStringT strSrcPath = vecClipboardItems[i];
			int nPos = strSrcPath.ReverseFind(_T('/'));
			SStringT strFileName = strSrcPath.Mid(nPos + 1);
			SStringT strDestFilePath = strDestPath + _T('/') + strFileName;
			
			if(nClipboardOperation == 1)
			{
				if(::PathFileExists(strDestFilePath))
				{
					SStringT strBaseName = strFileName.Left(strFileName.ReverseFind(_T('.')));
					SStringT strExt = strFileName.Mid(strFileName.ReverseFind(_T('.')));
					int nIndex = 1;
					while(::PathFileExists(strDestPath + _T('/') + strBaseName + _T('(') + SStringT().Format(_T("%d"), nIndex) + _T(')') + strExt))
					{
						nIndex++;
					}
					strDestFilePath = strDestPath + _T('/') + strBaseName + _T('(') + SStringT().Format(_T("%d"), nIndex) + _T(')') + strExt;
				}
				
				if(::PathIsDirectory(strSrcPath))
				{
					SHFILEOPSTRUCT fos = {0};
					fos.wFunc = FO_COPY;
					fos.pFrom = strSrcPath + _T('\0');
					fos.pTo = strDestFilePath + _T('\0');
					fos.fFlags = FOF_NOCONFIRMMKDIR | FOF_SILENT | FOF_NOERRORUI;
					::SHFileOperation(&fos);
				}
				else
				{
					::CopyFile(strSrcPath, strDestFilePath, FALSE);
				}
			}
			else if(nClipboardOperation == file_op_cut)
			{
				if(::PathIsDirectory(strSrcPath))
				{
					SHFILEOPSTRUCT fos = {0};
					fos.wFunc = FO_MOVE;
					fos.pFrom = strSrcPath + _T('\0');
					fos.pTo = strDestPath + _T('\0');
					fos.fFlags = FOF_NOCONFIRMMKDIR | FOF_SILENT | FOF_NOERRORUI;
					::SHFileOperation(&fos);
				}
				else
				{
					::MoveFile(strSrcPath, strDestFilePath);
				}
			}
		}
		// 清除所有剪切状态
		m_pFileTreeAdapter->ClearAllCutStates();
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
	m_pXmlEdtior->CloseXml();
	m_pImageViewer->Clear();
	m_pFileTreeAdapter->Clear();
	m_UIResFileMgr.CloseProject();	

	m_editXmlType = FT_UNKNOWN;
	UpdateEditorToolbar();

	m_bIsOpen = FALSE;
	UpdateToolbar();
	return true;
}

//打开工程
void CMainDlg::OpenProject(SStringT strFileName)
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
		
		return;
	}

	if(!m_UIResFileMgr.OpenProject(m_strUiresPath))
	{
		SLOGW()<<"open project file failed";
		return;
	}

	m_strUIResFile = strFileName;
	m_pXmlEdtior->SetProjectPath(strFileName);

    m_bIsOpen = TRUE;
	
	// 更新文件树的根目录为项目目录
	if (m_pFileTreeAdapter)
	{
		m_pFileTreeAdapter->SetRootPath(m_strProPath);
	}
	
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

// 序列化项目到 clipboard
void CMainDlg::SerializeItemsToClipboard(const std::vector<HSTREEITEM>& items, int nOperation)
{
	if(items.empty())
		return;

	// 准备数据：操作类型 + 项目数量 + 项目路径列表（UTF-8）
	std::vector<SStringA> vecItemPathsUtf8;
	for(size_t i = 0; i < items.size(); i++)
	{
		const FileItemData& itemData = m_pFileTreeAdapter->GetItemData(items[i]);
		vecItemPathsUtf8.push_back(S_CT2A(itemData.strPath));
	}

	// 计算所需缓冲区大小
	std::stringstream ss;
	int nCount = vecItemPathsUtf8.size();
	for(size_t i = 0; i < vecItemPathsUtf8.size(); i++)
	{
		ss<< vecItemPathsUtf8[i].c_str();
		ss<< '\0';
	}
	ss<< '\0';

	// 分配内存
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, ss.str().size() + sizeof(int) * 2);
	if(hGlobal)
	{
		LPVOID lpData = GlobalLock(hGlobal);
		if(lpData)
		{
			BYTE* pBuffer = (BYTE*)lpData;
			
			// 写入操作类型
			*((int*)pBuffer) = nOperation;
			pBuffer += sizeof(int);
			
			// 写入项目数量
			*((int*)pBuffer) = nCount;
			pBuffer += sizeof(int);
			// 写入项目路径（UTF-8）
			memcpy(pBuffer, ss.str().c_str(), ss.str().size());
			
			GlobalUnlock(hGlobal);
			
			// 设置到 clipboard
			if(OpenClipboard(m_hWnd))
			{
				EmptyClipboard();
				SetClipboardData(m_uClipboardFormat, hGlobal);
				CloseClipboard();
			}
			else
			{
				GlobalFree(hGlobal);
			}
		}
		else
		{
			GlobalFree(hGlobal);
		}
	}
	hGlobal = GlobalAlloc(GMEM_MOVEABLE, ss.str().size() + sizeof(DROPFILES));
	if(hGlobal)
	{
		LPVOID lpData = GlobalLock(hGlobal);
		if(lpData)
		{
			BYTE* pBuffer = (BYTE*)lpData;
			DROPFILES* pDropFiles = (DROPFILES*)pBuffer;
			pDropFiles->fNC = FALSE;
			pDropFiles->fWide = FALSE;
			pDropFiles->pFiles = sizeof(DROPFILES);
			pBuffer += sizeof(DROPFILES);
			memcpy(pBuffer, ss.str().c_str(), ss.str().size());
			GlobalUnlock(hGlobal);
			
			// 设置到 clipboard
			if(OpenClipboard(m_hWnd))
			{
				EmptyClipboard();
				SetClipboardData(CF_HDROP, hGlobal);
				CloseClipboard();
			}
			else
			{
				GlobalFree(hGlobal);
			}
		}
		else
		{
			GlobalFree(hGlobal);
		}
	}
}

// 从 CF_HDROP 读取文件路径
// 检查 clipboard 是否有数据
BOOL CMainDlg::HasClipboardData()
{
	BOOL bHasData = FALSE;
	
	if(IsClipboardFormatAvailable(m_uClipboardFormat))
	{
		if(OpenClipboard(m_hWnd))
		{
			HGLOBAL hGlobal = (HGLOBAL)GetClipboardData(m_uClipboardFormat);
			if(hGlobal)
			{
				LPVOID lpData = GlobalLock(hGlobal);
				if(lpData)
				{
					BYTE* pBuffer = (BYTE*)lpData;
					int nOperation = *((int*)pBuffer);
					int nCount = *((int*)(pBuffer + sizeof(int)));
					bHasData = (nOperation > 0 && nCount > 0);
					GlobalUnlock(hGlobal);
				}
			}
			CloseClipboard();
		}
	}
	
	if(!bHasData && IsClipboardFormatAvailable(CF_HDROP))
	{
		bHasData = TRUE;
	}
	
	return bHasData;
}

// 从 clipboard 反序列化项目
BOOL CMainDlg::DeserializeItemsFromClipboard(std::vector<SStringT>& vecItemPaths, int& nOperation)
{
	vecItemPaths.clear();

	// 首先尝试读取自定义格式
	if(IsClipboardFormatAvailable(m_uClipboardFormat))
	{
		if(OpenClipboard(m_hWnd))
		{
			HGLOBAL hGlobal = (HGLOBAL)GetClipboardData(m_uClipboardFormat);
			if(hGlobal)
			{
				LPVOID lpData = GlobalLock(hGlobal);
				if(lpData)
				{
					BYTE* pBuffer = (BYTE*)lpData;
					
					// 读取操作类型
					nOperation = *((int*)pBuffer);
					pBuffer += sizeof(int);
					
					// 读取项目数量
					int nCount = *((int*)pBuffer);
					pBuffer += sizeof(int);
					
					// 读取项目路径（UTF-8）
					for(int i = 0; i < nCount; i++)
					{
						// 读取 UTF-8 字符串
						char* szUtf8Path = (char*)pBuffer;
						int nLen = strlen(szUtf8Path) + 1;
						// 转换为宽字符串
						SStringT strPath = S_CA2T(szUtf8Path, CP_UTF8);
						vecItemPaths.push_back(strPath);
						pBuffer += nLen;
					}
					
					GlobalUnlock(hGlobal);
					CloseClipboard();
					return !vecItemPaths.empty();
				}
			}
			CloseClipboard();
		}
	}

	// 如果自定义格式失败，尝试读取系统资源管理器的 CF_HDROP 格式
	if(IsClipboardFormatAvailable(CF_HDROP))
	{
		if(OpenClipboard(m_hWnd))
		{
			HGLOBAL hGlobal = (HGLOBAL)GetClipboardData(CF_HDROP);
			if(hGlobal)
			{
				HDROP hDrop = (HDROP)GlobalLock(hGlobal);
				if(hDrop)
				{
					UINT nFileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
					for(UINT i = 0; i < nFileCount; i++)
					{
						TCHAR szFilePath[MAX_PATH];
						if(DragQueryFile(hDrop, i, szFilePath, MAX_PATH))
						{
							vecItemPaths.push_back(szFilePath);
						}
					}
					GlobalUnlock(hGlobal);
					CloseClipboard();
					if(!vecItemPaths.empty())
					{
						nOperation = file_op_copy; // 资源管理器复制默认为复制操作
						return TRUE;
					}
				}
			}
			CloseClipboard();
		}
	}

	return FALSE;
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

BOOL CMainDlg::NewLayout(const SStringT &strPath, const SStringT &strName)
{
	BOOL bRet = m_UIResFileMgr.NewLayout(strName, strPath);
	if(bRet){
		SStringT strType;
		m_UIResFileMgr.IsLayoutXml(strPath, strType);
		m_pXmlEdtior->LoadXml(strPath, strType);
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

void CMainDlg::OnBtnRecentFile()
{
	CRect rect = m_btn_recentFile->GetWindowRect();
	ClientToScreen2(&rect);

	m_RecentFileMenu.TrackPopupMenu(0, rect.left, rect.bottom, m_hWnd);
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
		menu.EnableMenuItem(menu_paste, HasClipboardData()?MF_ENABLED:MF_DISABLED);
	
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
				std::vector<HSTREEITEM> selectedItems = GetSelectedItems(hItem);
				if(!selectedItems.empty())
				{
					SerializeItemsToClipboard(selectedItems, file_op_copy);
				}
			}
			break;
		case menu_cut://剪切
			{
				std::vector<HSTREEITEM> selectedItems = GetSelectedItems(hItem);
				if(!selectedItems.empty())
				{
					SerializeItemsToClipboard(selectedItems, file_op_cut);
				}
			}
			break;
            case menu_paste: // 粘贴
            {
                OnFilePaste(hItem);
            }
            break;
            case menu_delete: // 删除
            {
                OnFileDelete(hItem);
            }
            break;
            case menu_rename: // 重命名
                OnFileRename(hItem);
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
                addUires(hItem);
                break;
            case menu_add_skin:
                addUires(hItem); // add to uires.idx first
                addSkin(hItem);
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
//			SLOGI()<<"Mouse click:"<<pEvt->pt.x<<","<<pEvt->pt.y;
		}else if(pEvt->clickId == MOUSE_LBTN_UP){
			m_tvClickPt = CPoint(-1, -1);
//			SLOGI()<<"Mouse up:"<<pEvt->pt.x<<","<<pEvt->pt.y;
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
			std::vector<HSTREEITEM> selectedItems = GetSelectedItems(hItem);
			if(selectedItems.empty())
			{
				m_tvClickPt = CPoint(-1, -1);
				return;
			}

			// 收集所有选中项目的文件路径信息
			std::vector<SStringW> filePaths;
			int totalPathLength = 0;
			
			for(size_t i = 0; i < selectedItems.size(); i++)
			{
				const FileItemData &itemData = m_pFileTreeAdapter->GetItemData(selectedItems[i]);
                SStringW strPathW = S_CT2W(itemData.strPath);
                filePaths.push_back(strPathW);
                totalPathLength += strPathW.GetLength() + 1; // +1 用于宽字符终止符
			}

			// 创建 DROPFILES 格式的全局内存，用于支持标准文件拖放
			HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, sizeof(DROPFILES) + totalPathLength * sizeof(wchar_t) + sizeof(wchar_t));
			if(!hGlobal)
			{
				m_tvClickPt = CPoint(-1, -1);
				return;
			}

			DROPFILES *pDropFiles = (DROPFILES *)GlobalLock(hGlobal);
			if(!pDropFiles)
			{
				GlobalFree(hGlobal);
				m_tvClickPt = CPoint(-1, -1);
				return;
			}

			// 初始化 DROPFILES 结构
			pDropFiles->pFiles = sizeof(DROPFILES);
			pDropFiles->pt.x = m_tvClickPt.x;
			pDropFiles->pt.y = m_tvClickPt.y;
			pDropFiles->fNC = FALSE;
			pDropFiles->fWide = TRUE;

			// 复制所有选中项目的路径到全局内存
			wchar_t *pPath = (wchar_t *)((BYTE *)pDropFiles + sizeof(DROPFILES));
			for(size_t i = 0; i < filePaths.size(); i++)
			{
				SStringW strPathW =filePaths[i];
				int pathLength = strPathW.GetLength() + 1;
				wcscpy_s(pPath, pathLength, strPathW.c_str());
				pPath += pathLength;
			}
			// 添加最终的空终止符
			*pPath = L'\0';

			GlobalUnlock(hGlobal);

			// 创建数据对象和拖放源
			SComPtr<IDataObject> pDataObj(new FileTreeDataSource(hGlobal, TRUE), FALSE);
            SComPtr<FileTreeDropSource> pDropSource(new FileTreeDropSource(), FALSE);
			
			if(!pDataObj || !pDropSource)
			{
				GlobalFree(hGlobal);
				m_tvClickPt = CPoint(-1, -1);
				return;
			}
            HBITMAP hDragBitmap = FileTreeDropSource::CreateDragBitmap(filePaths);
            pDropSource->SetDragImage(hDragBitmap, CPoint(-10, -10));
            SWND swndCapture = pSender->GetCapture();
            SWindow *pCapture = SWindowMgr::GetWindow(swndCapture); 
            if (pCapture)
            {
                pCapture->ReleaseCapture();
            }
			// 执行拖放操作
			DWORD dwEffect = DROPEFFECT_MOVE;
            SLOGI() << "Before DoDragDrop";
			HRESULT hr = ::DoDragDrop(pDataObj, pDropSource, DROPEFFECT_MOVE, &dwEffect);
			SLOGI()<<"DoDragDrop hr="<<hr<<" dwEffect="<<dwEffect;
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
			// 拖放操作完成后才删除位图
			if (hDragBitmap)
			{
				DeleteObject(hDragBitmap);
			}
			
			m_tvClickPt = CPoint(-1, -1);
		}
	}
}

void CMainDlg::addUires(HSTREEITEM hItem)
{
        BOOL bUpdated = FALSE;
        int selCount = m_treeView->GetSelItemCount();
        if (selCount)
        {
            HSTREEITEM *pItems = new HSTREEITEM[selCount];
            m_treeView->GetSelItems(pItems, selCount);
            // support multi select including sub directory
            for (int i = 0; i < selCount; i++)
            {
                const FileItemData &itemData = m_pFileTreeAdapter->GetItemData(pItems[i]);

                if (itemData.bIsDir)
                {
                    // 如果是目录，递归添加目录中的所有文件
                    AddFilesInDirectoryToUIRes(itemData.strPath);
                    bUpdated = TRUE;
                }
                else
                {
                    // 如果是文件，直接添加到UIRes
                    if (m_UIResFileMgr.AddToUIRes(itemData.strPath))
                        bUpdated = TRUE;
                }
            }
            delete[] pItems;
        }
        else
        {
            const FileItemData &itemData = m_pFileTreeAdapter->GetItemData(hItem);
            bUpdated = m_UIResFileMgr.AddToUIRes(itemData.strPath);
        }
        if (bUpdated && m_editXmlType == FT_INDEX_XML)
        { // 如果正在编辑uires.idx
            m_pXmlEdtior->LoadXml(m_strUIResFile, _T(""));
        }
}

void CMainDlg::addSkin(HSTREEITEM hItem)
{
    BOOL bUpdated = FALSE;
    int selCount = m_treeView->GetSelItemCount();
    std::vector<FileItemData> selectedItems;

    if (selCount)
    {
        HSTREEITEM *pItems = new HSTREEITEM[selCount];
        m_treeView->GetSelItems(pItems, selCount);
        for (int i = 0; i < selCount; i++)
        {
            const FileItemData &itemData = m_pFileTreeAdapter->GetItemData(pItems[i]);
            if (!itemData.bIsDir && m_UIResFileMgr.GetFileType(itemData.strPath) == FT_IMAGE)
            {
                selectedItems.push_back(itemData);
            }
        }
        delete[] pItems;
    }
    else
    {
        const FileItemData &itemData = m_pFileTreeAdapter->GetItemData(hItem);
        if (!itemData.bIsDir && m_UIResFileMgr.GetFileType(itemData.strPath) == FT_IMAGE)
        {
            selectedItems.push_back(itemData);
        }
    }

    if (!selectedItems.empty())
    {
        // 加载skin.xml文件
        pugi::xml_document &xmlDocSkin = m_UIResFileMgr.m_xmlDocSkin;
        pugi::xml_node xmlSkinRoot = xmlDocSkin.child(L"skins");
        if (!xmlSkinRoot)
        {
            xmlSkinRoot = xmlDocSkin.append_child(L"skins");
        }

        // 遍历选中的图片文件
        for (std::vector<FileItemData>::iterator it = selectedItems.begin(); it != selectedItems.end(); ++it)
        {
			FileItemData &itemData = *it;
            wchar_t szName[MAX_PATH], szExt[50];
			SStringW strPath = S_CT2W(itemData.strPath);
            _wsplitpath(strPath.c_str(), NULL, NULL, szName, szExt);

            SStringW strName = szName;

            // 解析文件名中的配置信息
            int nStates = 1, left = -1, top = -1, right = -1, bottom = -1;
            int nValues = 0;
            int nColorize = 1, nAutoFit = 1, nTile = 0, nVertical = 0, nFilter = 0;

            wchar_t* p = wcsrchr(szName, L'[');
            if (p)
            {
                *p = 0;
                strName = szName;
                nValues = swscanf(p + 1, L"%d{%d,%d,%d,%d}]", &nStates, &left, &top, &right, &bottom);

                const wchar_t* pszFind = wcsstr(p + 1, L"{ec=");
                if (pszFind) nColorize = _wtoi(pszFind + 4);
                pszFind = wcsstr(p + 1, L"{fit=");
                if (pszFind) nAutoFit = _wtoi(pszFind + 5);
                pszFind = wcsstr(p + 1, L"{filter=");
                if (pszFind) nFilter = _wtoi(pszFind + 8);
                pszFind = wcsstr(p + 1, L"{tile=");
                if (pszFind) nTile = _wtoi(pszFind + 6);
                pszFind = wcsstr(p + 1, L"{vert=");
                if (pszFind) nVertical = _wtoi(pszFind + 6);
            }

            // 从uires.idx中查询src路径
            SStringW strSrc;
            pugi::xml_node xmlNodeUiRes = m_UIResFileMgr.m_xmlDocUiRes.child(L"uires");
            if (xmlNodeUiRes)
            {
                pugi::xml_node fileNode = xmlNodeUiRes.child(strName.c_str());
                if (fileNode)
                {
                    SStringW strPath = fileNode.attribute(L"path").as_string();
                    wchar_t szPathName[MAX_PATH], szPathExt[50];
                    _wsplitpath(strPath, NULL, NULL, szPathName, szPathExt);
                    SStringW strPathName = szPathName;
                    // 构建src路径，格式为：section:name
                    pugi::xml_node parentNode = fileNode.parent();
                    if (parentNode && wcscmp(parentNode.name(), L"uires") != 0)
                    {
                        strSrc = parentNode.name();
                        strSrc += L":";
                        strSrc += strName;
                    }
                    else
                    {
                        strSrc = L"";
                        strSrc += L":";
                        strSrc += strName;
                    }
                }
            }

            if (!strSrc.IsEmpty())
            {
                // 检查是否已存在同名皮肤
                pugi::xml_node existingSkin = xmlSkinRoot.child(strName.c_str());
                if (existingSkin)
                {
                    xmlSkinRoot.remove_child(existingSkin);
                }

                // 根据解析结果添加相应的皮肤配置
                pugi::xml_node newSkin;
                if (strPath.EndsWith(L".9.png"))
                {
                    // Android .9.png 格式
                    newSkin = xmlSkinRoot.append_child(L"imgframe2");
                }
                else if (nValues == 0 || nValues == 1)
                {
                    // imglist
                    newSkin = xmlSkinRoot.append_child(L"imglist");
                }
                else if (nValues == 3 || nValues == 5)
                {
                    // imgframe
                    newSkin = xmlSkinRoot.append_child(L"imgframe");
					SStringW strMargin;
					strMargin.Format(L"%d,%d,%d,%d", left, top, right == -1 ? left : right, bottom == -1 ? top : bottom);
                    newSkin.append_attribute(L"margin") = strMargin;
                }

                if (newSkin)
                {
                    newSkin.append_attribute(L"name") = strName;
                    newSkin.append_attribute(L"src") = strSrc;
                    newSkin.append_attribute(L"states") = nStates;

                    // 设置各种可选属性
                    if (nColorize == 0) newSkin.append_attribute(L"enableColorize") = 0;
                    if (nAutoFit == 0) newSkin.append_attribute(L"autoFit") = 0;
                    if (nTile != 0) newSkin.append_attribute(L"tile") = 1;
                    if (nVertical != 0) newSkin.append_attribute(L"vertical") = 1;
                    switch (nFilter)
                    {
                    case 1: newSkin.append_attribute(L"filterLevel") = L"low"; break;
                    case 2: newSkin.append_attribute(L"filterLevel") = L"midium"; break;
                    case 3: newSkin.append_attribute(L"filterLevel") = L"high"; break;
                    }

                    bUpdated = TRUE;
                }
            }
        }

        if (bUpdated)
        {
            // 保存skin.xml文件
            xmlDocSkin.save_file(m_UIResFileMgr.m_strSkinFile, L"\t", pugi::format_default, pugi::encoding_utf8);
            if (m_editXmlType == FT_SKIN)
            {
                // 如果正在编辑skin.xml，重新加载
                m_pXmlEdtior->LoadXml(m_UIResFileMgr.m_strSkinFile, _T(""));
            }
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

void CMainDlg::OnInsertWidget(CWidgetTBAdapter::IconInfo *info)
{
	m_pXmlEdtior->InsertWidget(info->strContent);
	m_lvWidget->SetSel(-1,FALSE);
}

void CMainDlg::OnInertSkin(CSkinTBAdapter::IconInfo * info)
{
	m_lvSkin->SetSel(-1,FALSE);
	DlgInsertXmlElement dlg(&m_UIResFileMgr,CSysDataMgr::getSingleton().getSkinDefNode().child(L"skins"),S_CT2W(info->strElement));
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
	if(m_editXmlType == FT_UNKNOWN)
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
		SStringA strAutoShow;
		if(m_editXmlType== FT_LAYOUT_XML)
			strAutoShow = CSysDataMgr::getSingleton().GetCtrlAutos();
		if(m_editXmlType== FT_SKIN)
			strAutoShow = CSysDataMgr::getSingleton().GetSkinAutos();
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
			
			SStringA str;
			if(m_editXmlType== FT_LAYOUT_XML)
				str = CSysDataMgr::getSingleton().GetCtrlAttrAutos(strTag);
			if(m_editXmlType== FT_SKIN)
				str = CSysDataMgr::getSingleton().GetSkinAttrAutos(strTag);
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
	case FT_LAYOUT_XML:
		m_lvWidget->SetVisible(TRUE,TRUE);
		m_lvSkin->SetVisible(FALSE,TRUE);
		break;
	case FT_SKIN:
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
	FindChildByID(R.id.toolbar_btn_viewskin)->EnableWindow(m_bIsOpen,TRUE);
	FindChildByID(R.id.toolbar_btn_Close)->EnableWindow(m_bIsOpen,TRUE);
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

// 递归添加目录中的所有文件到UIRes
void CMainDlg::AddFilesInDirectoryToUIRes(const SStringT& dirPath)
{
    WIN32_FIND_DATA findData;
    HANDLE hFind;
    SStringT searchPath = dirPath;
    searchPath += _T("/*.*");

    hFind = FindFirstFile(searchPath, &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // 是子目录，跳过"."和".."目录
                if (_tcscmp(findData.cFileName, _T(".")) != 0 &&
                    _tcscmp(findData.cFileName, _T("..")) != 0)
                {
                    SStringT subDirPath = dirPath;
                    subDirPath += _T("/");
                    subDirPath += findData.cFileName;
                    AddFilesInDirectoryToUIRes(subDirPath); // 递归处理子目录
                }
            }
            else
            {
                // 是文件，添加到UIRes
                SStringT filePath = dirPath;
                filePath += _T("/");
                filePath += findData.cFileName;
                m_UIResFileMgr.AddToUIRes(filePath);
            }
        } while (FindNextFile(hFind, &findData));

        FindClose(hFind);
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