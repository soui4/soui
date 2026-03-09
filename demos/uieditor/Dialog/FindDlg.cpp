#include "stdafx.h"
#include "FindDlg.h"

SNSBEGIN

	CFindDlg::CFindDlg(IFindListener *pFindListener, const SStringT & strFindText)
		:SHostWnd(UIRES.LAYOUT.dlg_find)
		,m_pFindListener(pFindListener)
	{
		m_strFindText = strFindText;
	}

	CFindDlg::~CFindDlg(void)
	{
	}

	void CFindDlg::OnFinalMessage(HWND hWnd)
	{
		SHostWnd::OnFinalMessage(hWnd);
		delete this;
	}

	void CFindDlg::OnClose()
	{
		SNativeWnd::ShowWindow(SW_HIDE);
	}

	void CFindDlg::OnFindNext()
	{
		OnFind(true);
	}

	void CFindDlg::OnFindPrev()
	{
		OnFind(false);
	}

	void CFindDlg::OnFind(bool bNext)
	{
		if(!m_pFindListener) return;
		SStringT strText = m_pEditTarget->GetWindowText();
		if(strText.IsEmpty()) return;
		bool bMatchCase = !!m_pMatchCase->IsChecked();
		bool bMatchWholeWord = !!m_pMatchWholeWord->IsChecked();
		m_pFindListener->OnFind(strText,bNext,bMatchCase,bMatchWholeWord);
	}

	void CFindDlg::OnInit(EventArgs *e)
	{
		m_pEditTarget = FindChildByID2<SEdit>(R.id.edit_search);
		m_pMatchCase = FindChildByID2<SCheckBox>(R.id.chk_match_case);
		m_pMatchWholeWord = FindChildByID2<SCheckBox>(R.id.chk_match_whole_word);
		m_pEditTarget->SetFocus();
		m_pEditReplace = FindChildByID2<SEdit>(R.id.edit_replace);
		m_pEditTarget->SetWindowText(m_strFindText);
	}

	void CFindDlg::OnReplace()
	{
		if(!m_pFindListener) return;
		SStringT strText = m_pEditReplace->GetWindowText();
		m_pFindListener->OnReplace(strText);
		OnFind(true);
	}

	void CFindDlg::SetFindText(SStringT strFindText)
	{
		m_strFindText = strFindText;
		if(m_pEditTarget)
			m_pEditTarget->SetWindowText(m_strFindText);
	}

	SStringT CFindDlg::GetFindText()
	{
		if(m_pEditTarget)
			m_strFindText = m_pEditTarget->GetWindowText();
		return m_strFindText;
	}
SNSEND
