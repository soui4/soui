#ifndef _DLGNEWCUSTOM_H_
#define _DLGNEWCUSTOM_H_

#include "core/SHostDialog.h"

SNSBEGIN
	class SDlgNewCustom: public SHostDialog
	{
	public:
		SDlgNewCustom();

		~SDlgNewCustom(void)
		{

		}

		BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

	protected:
		void OnOK();
		void OnClose();

		EVENT_MAP_BEGIN()
			EVENT_ID_COMMAND(IDOK, OnOK)
			EVENT_ID_COMMAND(IDCANCEL, OnClose)
		EVENT_MAP_END2(SHostDialog)

		BEGIN_MSG_MAP_EX(SDlgNewCustom)
			MSG_WM_INITDIALOG(OnInitDialog)
			CHAIN_MSG_MAP(SHostDialog)
			REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()

	public:
		SStringT m_strTitle;
		SEdit *m_edtName;
		SEdit *m_edtParent;

		SStringT m_strName;
		SStringT m_strParent;
	};

SNSEND

#endif//_DLGNEWCUSTOM_H_