#ifndef __XML_EDITOR_H__
#define __XML_EDITOR_H__

#include "ScintillaWnd.h"
#include "Scintilla.h"
#include <map>
#include <vector>
#include "XmlParser.h"
#include "designer/DesignWnd.h"
#include <res.mgr/SUiDef.h>
#include <propgrid/SPropertyGrid.h>
#include "monitor/PathMonitor.h"
#include "ResManger.h"
class CMainDlg;
SNSBEGIN

	class CXmlEditor
	{
	public:
		CXmlEditor(CMainDlg *pMainHost);
		~CXmlEditor();

		void Init(SWindow * pRoot,CScintillaWnd::IListener *pListener);
		
		// 打开工程
		void SetProjectPath(const SStringT & strProjPath);
        BOOL LoadXml(SStringT strFileName, SStringT layoutName);   //加载布局
		BOOL CloseXml();
		void Reload(); // 重新加载xml文件
		bool SaveFile();
		bool isValidXml();
		
        void OnSelectedCtrl(const int *pOrder, int nLen);
		void OnUpdateSize(int nWidth, int nHeight);
		void OnUpdatePos(SStringW strPos, BOOL bRemoveSize);
		void SelectCtrlByOrder(const int *pOrder,int nLen,BOOL bSelXml);

		bool isDirty() const;

		void InsertText(SStringA strText);

		BOOL InsertElement(SStringA xmlElement);
		BOOL InsertWidget(SStringA xmlWidget);
        SStringT GetCurrentXml() const
        {
            return m_strXmlFile;
        }
	protected:
		void OnMoveCtrl();
		void UpdatePropGridLayout();
	protected:
		void ClearSelect();
		void InitPropGrid(const SStringW &strNodeName, SStringW strParents);
        // soui消息
        void OnTCSelChanged(IEvtArgs *pEvt);
		void OnPropGridValueChanged(IEvtArgs *pEvt);
		void OnPropGridItemAutoCompleteFill(IEvtArgs *pEvt);
		void OnPropGridItemAutoCompleteSelected(IEvtArgs *pEvt);
		void OnPropGridConvertColor(IEvtArgs *pEvt);
		void OnSaveXml();
		void OnFormatXml();
		EVENT_MAP_BEGIN()
			EVENT_ID_COMMAND(R.id.toolbar_btn_savexml,OnSaveXml)
			EVENT_ID_COMMAND(R.id.toolbar_btn_formatxml,OnFormatXml)
			EVENT_ID_HANDLER(R.id.propgrid_element,EventPropGridValueChanged::EventID,OnPropGridValueChanged)
			EVENT_ID_HANDLER(R.id.propgrid_element,EventPropGridItemAutoCompleteFill::EventID,OnPropGridItemAutoCompleteFill)
			EVENT_ID_HANDLER(R.id.propgrid_element,EventPropGridItemAutoCompleteSelected::EventID,OnPropGridItemAutoCompleteSelected)
			EVENT_ID_HANDLER(R.id.propgrid_element,EventPropGridConvertColor::EventID,OnPropGridConvertColor)
			EVENT_ID_HANDLER(R.id.uidesigner_wnd_xmltree,EventTCSelChanged::EventID,OnTCSelChanged)
		EVENT_MAP_BREAK()
	protected:
		void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);
		LRESULT OnNotify(int idCtrl, LPNMHDR pnmh);
        void OnTimer(UINT_PTR id);

        BEGIN_MSG_MAP_EX(CXmlEditor)
			MSG_WM_COMMAND(OnCommand)
			MSG_WM_NOTIFY(OnNotify)
			MSG_WM_TIMER(OnTimer)
		END_MSG_MAP()

	protected:
		void StartPreviewProcess();
		void UpdateXmlData();
		bool UpdateXmlStruct(spugi::xml_node xmlNode, HSTREEITEM item,int iSib=0);

	public:
		CMainDlg * m_pMainDlg;
		STreeCtrl *m_treeXmlStruct; //显示xml文档结构的tree控件
		SPropertyGrid *m_pPropGrid;

		CScintillaWnd *m_pScintillaWnd;	//XML代码编辑窗口
		CDesignWnd *m_pDesignWnd;
		CXmlParser	   m_xmlParser;

		SStringT m_strProPath;     //C:\demos\MyTest\uires

		SStringT m_strLayoutName; //当前打开的窗体名  XML_MAINDLG
		SStringT m_strXmlFile; //当前打开的XML文件名  xml\main.xml
		BOOL	 m_bChanged;
		BOOL     m_bUpdateDesigner;
		BOOL	 m_bValidXml;	
		int      m_nCaretPos;
		BOOL     m_bSetCaretPos;
		std::vector<int> m_vecSelectOrder;
		SXmlDoc  m_xmlEditing;
		ResManger *m_pResManger;
        SWindow *m_pRoot;
	};

SNSEND

#endif//__XML_EDITOR_H__