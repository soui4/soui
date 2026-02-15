#pragma once
#include "propgrid/SPropertyGrid.h"
#include "spugixml/pugixml.hpp"
#include "ScintillaWnd.h"
#include "ResManger.h"

namespace SOUI{
	class DlgInsertXmlElement : public SHostDialog
	{
	public:
		DlgInsertXmlElement(ResManger * resMgr,SXmlNode xmlInitProp,SStringW strNodeName);
		~DlgInsertXmlElement(void);

	public:
		SStringA GetXml() const;
	protected:
		void OnGetValue(IPropertyItem *pItem,BOOL bInsertXml);
		void OnPropToolbarCmd(IEvtArgs *e);

		void OnPropValueChanged(IEvtArgs *e);
		void OnPropItemActive(IEvtArgs *e);
		void OnPropItemButtonClick(IEvtArgs *e);
		void OnPropItemInplaceInit(IEvtArgs *e);
		void OnPropGridItemAutoCompleteFill(IEvtArgs *pEvt);
		void OnPropGridItemAutoCompleteSelected(IEvtArgs *pEvt);
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventPropGridValueChanged::EventID,OnPropValueChanged)
			EVENT_HANDLER(EventPropGridItemActive::EventID,OnPropItemActive)
			EVENT_HANDLER(EventPropGridItemButtonClick::EventID,OnPropItemButtonClick)
			EVENT_HANDLER(EventPropGridItemInplaceInit::EventID,OnPropItemInplaceInit)
			EVENT_HANDLER(EventPropGridItemAutoCompleteFill::EventID,OnPropGridItemAutoCompleteFill)
			EVENT_HANDLER(EventPropGridItemAutoCompleteSelected::EventID,OnPropGridItemAutoCompleteSelected)
			EVENT_HANDLER(EventToolBarCmd::EventID,OnPropToolbarCmd)
		EVENT_MAP_END2(SHostDialog)

		BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
		void OnTimer(UINT_PTR id);
		void OnDestroy();
		void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);

		BEGIN_MSG_MAP_EX(DlgInsertXmlElement)
			MSG_WM_DESTROY(OnDestroy)
			MSG_WM_INITDIALOG(OnInitDialog)
			MSG_WM_TIMER(OnTimer)
			MSG_WM_COMMAND(OnCommand)
			CHAIN_MSG_MAP(SHostDialog)
		END_MSG_MAP()

	private:
		void InitPropGrid(const SStringW &strNodeName,SStringW strParents);

		static BOOL OnEnumPropItem(IPropertyItem *pItem, void* opaque);
		SXmlNode m_xmlInitProp;
		SStringW m_strNodeName;

		SXmlDoc m_xmlDoc;

		SPropertyGrid * m_propgrid;
		CScintillaWnd * m_xmlEditor;
		BOOL			m_bChanged;
		SStringA		m_strXml;
		SToolBar *		m_tbProp;
		ResManger	*   m_resMgr;
		SAutoRefPtr<ISkinObj> m_skin;
		
	};
}

