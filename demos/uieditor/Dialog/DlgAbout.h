#ifndef _DLG_ABOUT_H_
#define _DLG_ABOUT_H_

#include "core/SHostDialog.h"

namespace SOUI
{
	class SDlgAbout: public SHostDialog
	{
	public:
		SDlgAbout();

		~SDlgAbout(void)
		{

		}

		void OnClose();
		void OnBtnDlgOpenFile();

		BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
	protected:
		void OnOK();

		EVENT_MAP_BEGIN()

			EVENT_ID_COMMAND(IDOK,OnOK)
			EVENT_MAP_END2(SHostDialog)

			BEGIN_MSG_MAP_EX(SDlgAbout)
			MSG_WM_INITDIALOG(OnInitDialog)
			CHAIN_MSG_MAP(SHostDialog)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()
	};

}

#endif//_DLG_ABOUT_H_