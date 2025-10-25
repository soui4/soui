#ifndef _DLGINPUT_H_
#define _DLGINPUT_H_

#include "core/SHostDialog.h"

namespace SOUI
{
	class SDlgInput: public SHostDialog
	{
	public:
		SDlgInput();

		~SDlgInput(void)
		{

		}


		void OnBtnDlgOpenFile();

		BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);



	protected:
		void OnOK();
		void OnClose();

		EVENT_MAP_BEGIN()

			EVENT_NAME_COMMAND(L"btnOK", OnOK)
			EVENT_NAME_COMMAND(L"btnCancel", OnClose)
			EVENT_MAP_END2(SHostDialog)

			BEGIN_MSG_MAP_EX(SDlgInput)
			MSG_WM_INITDIALOG(OnInitDialog)
			CHAIN_MSG_MAP(SHostDialog)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:

	public:
		SStringT m_strValue;	
		SEdit *m_edt;
	};

}

#endif//_DLGINPUT_H_