#ifndef _MAINDLG_H_
#define _MAINDLG_H_

#include <core/SWnd.h>
#include <control/SCmnCtrl.h>
#include "Dialog/DlgCreatePro.h"
#include "XmlEditor.h"
#include "propgrid/SPropertyGrid.h"
#include "SImageBtnEx.h"
#include "ResManger.h"
#include <helper/SMenuEx.h>
#include <vector>
#include <control/STreeCtrl.h>
#include "DropTarget.h"
#include "SToolBar.h"

#define MenuId_Start  20000


class CWidgetTBAdapter : public SAdapterBase
{
public:
	struct IconInfo{
		int iIcon;
		SStringT strTxt;
		SStringT strTip;
		SStringT strElement;
		SStringA strContent;
	};
	struct IListener{
		virtual void OnInsertWidget(IconInfo *info) = 0;
	};
	SAutoRefPtr<SSkinPool> m_skinPool;
public:
	CWidgetTBAdapter(SXmlNode xmlNode,IListener* pListener):m_pListener(pListener)
	{
		m_skinPool.Attach(new SSkinPool());
		SUiDef::getSingleton().PushSkinPool(m_skinPool);
		SXmlNode xmlIconSkin = xmlNode.child(L"toolbar").child(L"icons");
		ISkinObj *pSkin = SApplication::getSingleton().CreateSkinByName(xmlIconSkin.attribute(L"class_name").as_string(SSkinImgList::GetClassName()));
		if(pSkin)
		{
			pSkin->InitFromXml(&xmlIconSkin);
			m_skinPool->AddSkin(pSkin);
			pSkin->Release();
		}
		SXmlNode xmlWidget = xmlNode.child(L"controls").first_child();
		while(xmlWidget)
		{
			if(xmlWidget.attribute(L"visible").as_bool(true))
			{
				IconInfo info;
				info.iIcon = xmlWidget.attribute(L"icon").as_int(0);
				info.strElement = S_CW2T(xmlWidget.name());
				if(xmlWidget.attribute(L"text"))
					info.strTxt=S_CW2T(xmlWidget.attribute(L"text").as_string());
				else
					info.strTxt = S_CW2T(xmlWidget.name());
				info.strTip = S_CW2T(xmlWidget.attribute(L"tip").as_string());
				SStringW strContent;
				xmlWidget.child(xmlWidget.name()).ToString(&strContent); 
				info.strContent = S_CW2A(strContent,CP_UTF8);
				m_arrIcons.Add(info);
			}
			xmlWidget = xmlWidget.next_sibling();
		}
	}
	~CWidgetTBAdapter(){
		SUiDef::getSingleton().PopSkinPool(m_skinPool);
	}
protected:
	virtual int WINAPI getCount()
	{
		return m_arrIcons.GetCount();
	}

	virtual void WINAPI getView(int position, SItemPanel * pItem, SXmlNode xmlTemplate)
	{
		if(pItem->GetChildrenCount()==0)
		{
			pItem->InitFromXml(&xmlTemplate);
		}
		SImageWnd *pIcon = pItem->FindChildByName2<SImageWnd>(L"item_icon");
		if(pIcon) pIcon->SetIcon(m_arrIcons[position].iIcon);
		SWindow *pTxt = pItem->FindChildByName(L"item_text");
		if(pTxt) pTxt->SetWindowText(m_arrIcons[position].strTxt);
		pItem->GetEventSet()->subscribeEvent(EventItemPanelClickUp::EventID,Subscriber(&CWidgetTBAdapter::OnItemClickUp,this));
	}

	BOOL OnItemClickUp(IEvtArgs *e)
	{
		SItemPanel *pItem = sobj_cast<SItemPanel>(e->Sender());
		int iItem = pItem->GetItemIndex();
		m_pListener->OnInsertWidget(&m_arrIcons[iItem]);
		return true;
	}
private:
	SArray<IconInfo> m_arrIcons;
	IListener * m_pListener;
};


class CSkinTBAdapter : public SAdapterBase
{
	SAutoRefPtr<SSkinPool> m_skinPool;
public:
	struct IconInfo{
		int iIcon;
		SStringT strTxt;
		SStringT strElement;
		SStringT strTip;
	};

	struct IListener
	{
		virtual void OnInertSkin(IconInfo * info) = 0;
	};
public:
	~CSkinTBAdapter(){
		SUiDef::getSingleton().PopSkinPool(m_skinPool);
	}
	CSkinTBAdapter(SXmlNode xmlNode, IListener * pListener):m_pListener(pListener)
	{
		m_skinPool.Attach(new SSkinPool());
		SUiDef::getSingleton().PushSkinPool(m_skinPool);
		SXmlNode xmlIconSkin = xmlNode.child(L"toolbar").child(L"icons");
		ISkinObj *pSkin = SApplication::getSingleton().CreateSkinByName(xmlIconSkin.attribute(L"class_name").as_string(SSkinImgList::GetClassName()));
		if(pSkin)
		{
			pSkin->InitFromXml(&xmlIconSkin);
			m_skinPool->AddSkin(pSkin);
			pSkin->Release();
		}
		SXmlNode xmlSkin = xmlNode.child(L"skins").first_child();
		while(xmlSkin)
		{
			if(xmlSkin.attribute(L"visible").as_bool(true))
			{
				IconInfo info;
				info.iIcon = xmlSkin.attribute(L"icon").as_int(0);
				info.strElement = S_CW2T(xmlSkin.name());
				if(xmlSkin.attribute(L"text"))
					info.strTxt=S_CW2T(xmlSkin.attribute(L"text").as_string());
				else
					info.strTxt = S_CW2T(xmlSkin.name());
				info.strTip = S_CW2T(xmlSkin.attribute(L"tip").as_string());
				m_arrIcons.Add(info);
			}
			xmlSkin = xmlSkin.next_sibling();
		}
	}
protected:
	virtual int WINAPI getCount()
	{
		return m_arrIcons.GetCount();
	}

	virtual void WINAPI getView(int position, SItemPanel * pItem, SXmlNode xmlTemplate)
	{
		if(pItem->GetChildrenCount()==0)
		{
			pItem->InitFromXml(&xmlTemplate);
		}
		SImageWnd *pIcon = pItem->FindChildByName2<SImageWnd>(L"item_icon");
		if(pIcon) pIcon->SetIcon(m_arrIcons[position].iIcon);
		SWindow *pTxt = pItem->FindChildByName(L"item_text");
		if(pTxt) pTxt->SetWindowText(m_arrIcons[position].strTxt);
		pItem->GetEventSet()->subscribeEvent(EventItemPanelClickUp::EventID,Subscriber(&CSkinTBAdapter::OnItemClickUp,this));
	}

	BOOL OnItemClickUp(IEvtArgs *e)
	{
		SItemPanel *pItem = sobj_cast<SItemPanel>(e->Sender());
		int iItem = pItem->GetItemIndex();
		m_pListener->OnInertSkin(&m_arrIcons[iItem]);
		return true;
	}
private:
	SArray<IconInfo> m_arrIcons;
	IListener * m_pListener;
};


class CMainDlg : public SHostWnd, STreeCtrl::IListener,CScintillaWnd::IListener,public CDropTarget::IDropListener, CWidgetTBAdapter::IListener, CSkinTBAdapter::IListener
{
	enum XmlType{
		XML_LAYOUT,
		XML_SKIN,
		XML_UNKNOWN,
	};
public:
	CMainDlg();
	~CMainDlg();

protected:
	void LoadAppCfg();

	void SaveAppCfg();

	void OpenProject(SStringT strFileName);
	void ReloadWorkspaceUIRes();
	bool CloseProject();

	void UpdateEditorToolbar();
	void UpdateToolbar();
	bool CheckSave();
protected:
	void onScintillaSave(CScintillaWnd *pSci,LPCTSTR pszFileName) override;
	void onScintillaAutoComplete(CScintillaWnd *pSci,char c) override;
protected:
	BOOL OnDrop(LPCTSTR pszName) override;
	void OnInsertWidget(CWidgetTBAdapter::IconInfo *info) override;
	void OnInertSkin(CSkinTBAdapter::IconInfo * info) override;
	void OnInsertItem(STreeCtrl *pTreeCtrl,HSTREEITEM hItem) override{}
	void OnDeleteItem(STreeCtrl *pTreeCtrl,HSTREEITEM hItem,LPARAM lParam) override;
protected:
	//soui消息
	bool OnTreeproContextMenu(CPoint pt);
	void OnAutoCheck(IEvtArgs *e);
	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnBtnOpen(); //打开工程
	void OnBtnClose();
	BOOL OnBtnSave(); //保存布局
	void OnBtnNewLayout(); //新建Dialog
	void OnBtnNewInclude(); //新建Include
	void OnBtnResMgr();
	void OnBtnAbout();
	void OnBtnRecentFile();
	void OnTreeItemDbClick(IEvtArgs *pEvtBase);
	void OnWorkspaceXMLDbClick(IEvtArgs *pEvtBase);
	EVENT_MAP_BEGIN()
		if(m_pXmlEdtior) CHAIN_EVENT_MAP_MEMBER((*m_pXmlEdtior))
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)

		EVENT_ID_COMMAND(R.id.toolbar_btn_Open, OnBtnOpen)
		EVENT_ID_COMMAND(R.id.toolbar_btn_Close, OnBtnClose)
		
		EVENT_ID_COMMAND(R.id.toolbar_btn_NewLayout, OnBtnNewLayout)
		EVENT_ID_COMMAND(R.id.toolbar_btn_NewInclude, OnBtnNewInclude)
		EVENT_ID_COMMAND(R.id.toolbar_btn_savexml, OnBtnSave)
		EVENT_ID_COMMAND(R.id.btn_help, OnBtnAbout)
		EVENT_ID_COMMAND(R.id.toolbar_btn_recent, OnBtnRecentFile)
		
		EVENT_ID_COMMAND(R.id.toolbar_btn_resmgr, OnBtnResMgr)
		EVENT_ID_CONTEXTMENU(R.id.workspace_tree, OnTreeproContextMenu)
		EVENT_ID_HANDLER(R.id.workspace_tree,EVT_TC_DBCLICK,OnTreeItemDbClick)
		EVENT_ID_HANDLER(R.id.workspace_xmlfile_lb,EVT_LB_DBCLICK,OnWorkspaceXMLDbClick)
		EVENT_ID_HANDLER(R.id.chk_autosave,EventSwndStateChanged::EventID,OnAutoCheck)
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
	std::vector<SStringT>	m_vecRecentFile;

public:
	SStringT		   m_strUIResFile;

	SMenuEx			m_RecentFileMenu;
	SButton*		m_btn_recentFile;

	STreeCtrl * m_treePro;			//工程Layout列表
	SListBox* m_lbWorkSpaceXml;		//工程中的XML文件列表

	SStringT  m_cmdWorkspaceFile;	//命令行要打开的工程文件

	ResManger m_UIResFileMgr;	// 管理编辑的UI文件资源

	CXmlEditor *m_pXmlEdtior;
	XmlType		m_editXmlType;
	
	SListView * m_lvSkin;
	SListView * m_lvWidget;

	SStringT m_strUiresPath;	//uires.idx 的全路径
	SStringT m_strProPath;

	STabCtrl *m_tabWorkspace;
	
	BOOL		m_bIsOpen;  //工程是否打开
	bool		m_bAutoSave;
};
#endif//_MAINDLG_H_