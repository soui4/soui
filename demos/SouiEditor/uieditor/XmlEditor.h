#pragma once
#include "ScintillaWnd.h"
#include "Scintilla.h"
#include "layout/SouiLayout.h"
#include "layout/SLinearLayout.h"
#include <map>
#include "XmlParser.h"


class CMainDlg;
namespace SOUI
{
	class CXmlEditor
	{
	public:
		CXmlEditor(CMainDlg *pMainHost);
		~CXmlEditor();

		void Init(SWindow * pRoot,CScintillaWnd::IListener *pListener);
		
		void SetProjectPath(const SStringT & strProjPath);

		bool SaveFile();


		bool isValidXml();
		// 打开工程
		BOOL CloseProject();
		
		BOOL LoadXml(SStringT strFileName, SStringT layoutName);   //加载布局

		void SelectCtrlByOrder(int *pOrder,int nLen);

		bool isDirty() const;

		void InsertText(SStringA strText);
	protected:
		//soui消息
		void OnTCSelChanged(IEvtArgs *pEvt);

		EVENT_MAP_BEGIN()
			EVENT_NAME_HANDLER(L"uidesigner_wnd_xmltree",EventTCSelChanged::EventID,OnTCSelChanged)
		EVENT_MAP_BREAK()
	protected:
		void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);
		BOOL OnCopyData(HWND wnd, PCOPYDATASTRUCT pCopyDataStruct);

		void OnTimer(UINT_PTR id);

		BEGIN_MSG_MAP_EX(CXmlEditor)
			MSG_WM_COPYDATA(OnCopyData)
			MSG_WM_COMMAND(OnCommand)
			MSG_WM_TIMER(OnTimer)
		END_MSG_MAP()

	protected:
		void StartPreviewProcess();
		void ReloadLayout();
		bool UpdateXmlStruct(spugi::xml_node xmlNode, HSTREEITEM item,int iSib=0);

	public:
		CMainDlg * m_pMainDlg;

		SSplitWnd * m_xml_editor;
		STreeCtrl *m_treeXmlStruct; //显示xml文档结构的tree控件
		SRealWnd * m_pViewerHost;

		CScintillaWnd *m_pScintillaWnd;	//XML代码编辑窗口
		CXmlParser	   m_xmlParser;

		SStringT m_strProPath;     //C:\demos\MyTest\uires\

		SStringT m_strLayoutName; //当前打开的窗体名  XML_MAINDLG
		SStringT m_strXmlFile; //当前打开的XML文件名  xml\main.xml
		BOOL	 m_bChanged;
		bool	 m_bValidXml;
	};
}