#pragma once

#include "core/SHostDialog.h"
#include "control/SRichEdit.h"

namespace SOUI
{
	class SDlgNewLayout: public SHostDialog
	{
	public:
		SDlgNewLayout(LPCTSTR pszXmlName, SStringT strProPath,SStringT strFolder=_T("xml"));

		~SDlgNewLayout(void)
		{
		}


		void OnClose();
		void OnBtnDlgOpenFile();

		BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

	protected:
		void OnOK();
		void OnResNameInputNotify(IEvtArgs *e);

		EVENT_MAP_BEGIN()
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_close", OnClose)
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_dlg", OnBtnDlgOpenFile)
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_OK", OnOK)
			EVENT_ID_HANDLER(R.id.new_layout_resname, EventRENotify::EventID, OnResNameInputNotify)
		EVENT_MAP_END2(SHostDialog)

		BEGIN_MSG_MAP_EX(SResMgrDlg)
			MSG_WM_INITDIALOG(OnInitDialog)
			CHAIN_MSG_MAP(SHostDialog)
			REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()

	protected:
		SStringT m_strProPath;
		SStringT m_strFolder;

		SEdit *m_edtName;
		SEdit *m_edtPath;
	public:
		SStringT m_strPath;
		SStringT m_strName;		
	};

}