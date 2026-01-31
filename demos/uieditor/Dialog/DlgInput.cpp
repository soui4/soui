#include "stdafx.h"
#include "DlgInput.h"

namespace SOUI
{

	SDlgInput::SDlgInput():SHostDialog(_T("LAYOUT:DLG_INPUT"))
	{
		m_strTitle = _T("输入对话框");
	}

	//TODO:消息映射
	void SDlgInput::OnClose()
	{
		SHostDialog::OnCancel();
	}

	void SDlgInput::OnOK()
	{
		m_strValue = m_edt->GetWindowText();
		SHostDialog::OnOK();
	}

	BOOL SDlgInput::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
	{
		m_edt = FindChildByName2<SEdit>(L"edtInput");
		m_edt->SetWindowText(m_strValue);
		SWindow * txtTitle = FindChildByName(L"txt_title");
		txtTitle->SetWindowText(m_strTitle);
		return TRUE;
	}

}


