#ifndef _MAINDLG_H_
#define _MAINDLG_H_

#include <core/SWnd.h>
#include <souicoll.h>
#include <control/SCmnCtrl.h>
#include "XmlEditor.h"
#include "ImageViewer.h"
#include "propgrid/SPropertyGrid.h"
#include "SImageBtnEx.h"
#include "ResManger.h"
#include <helper/SMenuEx.h>
#include <vector>
#include <control/STreeCtrl.h>
#include <control/SToolBar.h>
#include <algorithm>
#include "FileTreeAdapter.h"
#include "monitor/PathMonitor.h"
#include "FileOperationManager.h"
#include "ClipboardManager.h"
#include "UIResManager.h"
#include "ToolbarManager.h"
#define MenuId_Start  20000

class CMainDlg : public SHostWnd, CPathMonitor::IListener
{
public:
	CMainDlg();
	~CMainDlg();
	
 public:
	void OnNewCustomExtend(bool bSkin, SStringT strName = _T(""));
	BOOL OnDrop(LPCTSTR pszName);
  protected:
	// CPathMonitor::IListener接口实现
    void OnFileChanged(LPCTSTR pszFile, CPathMonitor::Flag nFlag) override;
    void _OnFileChanged(tstring &pszFile, CPathMonitor::Flag nFlag);

  protected:
	void LoadAppCfg();

	void SaveAppCfg();

	BOOL OpenProject(SStringT strFileName);
	void ReloadWorkspaceUIRes();
	bool CloseProject();

	bool CheckSave();

	BOOL NewLayout(const SStringT& strPath, const SStringT& strName);

protected:
	//soui消息
	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnBtnOpen(); //打开工程
	void OnBtnClose();
	BOOL OnBtnSave(); //保存布局
	void OnBtnNewLayout(); //新建Dialog
	void OnBtnNewInclude(); //新建Include
	void OnBtnAbout();
	void OnBtnViewSkin();
	void OnBtnFormatXml();
	void OnBtnPreview(); 
	void OnBtnRecentFile(IEvtArgs *e);
    void OnTvEventOfPanel(IEvtArgs *pEvtBase);
	void OnTvKeyDown(IEvtArgs *e);
	void OnTbWidgetClick(IEvtArgs *e);
	void OnTbSkinClick(IEvtArgs *e);
	void OnTbMainClick(IEvtArgs *e);
	void OnMenuSelect(IEvtArgs *e);
	void OnUpdateCmdUI(IEvtArgs *e);
    void OnAutoCheck(IEvtArgs *e);
    void OnUpdateCmdTip(IEvtArgs *e);
	void UpdateEditorToolbar();
	EVENT_MAP_BEGIN()
		if(m_pXmlEdtior) CHAIN_EVENT_MAP_MEMBER((*m_pXmlEdtior))
		if(m_pImageViewer) CHAIN_EVENT_MAP_MEMBER((*m_pImageViewer))
		EVENT_ID_COMMAND(R.id.btn_close, OnClose)
        EVENT_ID_COMMAND(R.id.btn_min, OnMinimize)
        EVENT_ID_COMMAND(R.id.btn_max, OnMaximize)
        EVENT_ID_COMMAND(R.id.btn_restore, OnRestore)
		EVENT_ID_COMMAND(R.id.btn_help, OnBtnAbout)
        EVENT_ID_HANDLER(R.id.chk_autosave, EventSwndStateChanged::EventID, OnAutoCheck)
		EVENT_ID_HANDLER(R.id.workspace_treeview,EventOfPanel::EventID,OnTvEventOfPanel)
		EVENT_ID_HANDLER(R.id.workspace_treeview,EventKeyDown::EventID,OnTvKeyDown)
		EVENT_ID_HANDLER(R.id.tb_widget,EventToolBarCmd::EventID,OnTbWidgetClick)
		EVENT_ID_HANDLER(R.id.tb_skin,EventToolBarCmd::EventID,OnTbSkinClick)
		EVENT_ID_HANDLER(R.id.tb_main,EventToolBarCmd::EventID,OnTbMainClick)
		EVENT_ID_HANDLER(R.id.main_menu,EventSelectMenu::EventID,OnMenuSelect)
        EVENT_ID_HANDLER(R.id.tb_main,EventUpdateCmdTip::EventID,OnUpdateCmdTip)
		EVENT_ID_HANDLER(R.id.tb_widget,EventUpdateCmdTip::EventID,OnUpdateCmdTip)
		EVENT_ID_HANDLER(R.id.tb_skin,EventUpdateCmdTip::EventID,OnUpdateCmdTip)
        EVENT_ID_HANDLER(ROOT_ID, EventUpdateCmdTip::EventID, OnUpdateCmdTip)
		EVENT_ID_HANDLER(R.id.tb_main,EventUpdateCmdUI::EventID,OnUpdateCmdUI)
		EVENT_ID_HANDLER(ROOT_ID,EventUpdateCmdUI::EventID,OnUpdateCmdUI)
	EVENT_MAP_END2(SHostWnd)

protected:
	//HostWnd真实窗口消息处理
	void OnSize(UINT nType, CSize size);
	void OnShowWindow(BOOL bShow, UINT nStatus);
	void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
	void OnDestroy();
	BEGIN_MSG_MAP_EX(CMainDlg)
		if(m_pXmlEdtior) CHAIN_MSG_MAP_MEMBER((*m_pXmlEdtior))
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_COMMAND(OnCommand)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

private:
	std::vector<SStringT> m_vecRecentFile;

public:
	SMenuEx		m_RecentFileMenu;

	STreeView *m_treeView;          //文件树视图
	SAutoRefPtr<CFileTreeAdapter> m_pFileTreeAdapter; //文件树适配器
	SStringT  m_cmdWorkspaceFile;	//命令行要打开的工程文件
	
	// 文件操作相关
	CPathMonitor m_pathMonitor;     // 路径监视器

	// 管理类
	CFileOperationManager* m_pFileOperationManager;
	CUIResManager* m_pUIResManager;
	CToolbarManager* m_pToolbarManager;

	ResManger m_UIResFileMgr;    // 管理编辑的UI文件资源

	CXmlEditor *m_pXmlEdtior;
	CImageViewer *m_pImageViewer;
	int	m_editXmlType;

	SStringT m_strUiresPath;    //uires.idx 的全路径
	SStringT m_strProPath;
	
	BOOL	m_bIsOpen;  //工程是否打开
	bool	m_bAutoSave;
	CPoint  	m_tvClickPt;

	SAutoRefPtr<SSkinPool> m_skinPool;
};
#endif//_MAINDLG_H_