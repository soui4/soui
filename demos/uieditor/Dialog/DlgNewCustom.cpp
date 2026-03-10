#include "stdafx.h"
#include "DlgNewCustom.h"

SNSBEGIN

	SDlgNewCustom::SDlgNewCustom():SHostDialog(_T("LAYOUT:DLG_NEW_CUSTOM"))
	{
		m_strTitle = _T("新建扩展对象");
	}
	void SDlgNewCustom::OnClose()
	{
		SHostDialog::OnCancel();
	}

	void SDlgNewCustom::OnOK()
	{
		m_strName = m_edtName->GetWindowText();
		m_strParent = m_edtParent->GetWindowText();
		SHostDialog::OnOK();
	}

	BOOL SDlgNewCustom::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
	{
		m_edtName = FindChildByName2<SEdit>(L"edt_custom_name");
		m_edtParent = FindChildByName2<SEdit>(L"edt_custom_parent");
		SWindow * txtTitle = FindChildByName(L"txt_title");
		txtTitle->SetWindowText(m_strTitle);
        m_edtName->SetWindowText(m_strName);
        m_edtParent->SetWindowText(m_strParent);
		return TRUE;
	}

SNSEND


