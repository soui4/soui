#pragma once

SNSBEGIN
	struct IFindListener{
		virtual bool OnFind(const SStringT & strText, bool bFindNext, bool bMatchCase, bool bMatchWholeWord) PURE;
		virtual void OnReplace(const SStringT &strText) PURE;
	};

	class CFindDlg : public SHostWnd, public SDpiHandler<CFindDlg>
	{
	public:
		CFindDlg(IFindListener *pFindListener, const SStringT & strFindText);
		~CFindDlg(void);

		SStringT GetFindText();
		void SetFindText(SStringT strFindText);
		
	protected:
		void OnClose();
		void OnFindNext();
		void OnFindPrev();
		void OnReplace();
		
		void OnFind(bool bNext);

		void OnInit(EventArgs *e);
		EVENT_MAP_BEGIN()
			EVENT_HANDLER(EventInit::EventID,OnInit)
			EVENT_ID_COMMAND(R.id.btn_close,OnClose)
			EVENT_ID_COMMAND(R.id.btn_next,OnFindNext)
			EVENT_ID_COMMAND(R.id.btn_prev,OnFindPrev)
			EVENT_ID_COMMAND(R.id.btn_replace,OnReplace)
		EVENT_MAP_END2(SHostWnd)

		BEGIN_MSG_MAP_EX(CFindDlg)
			CHAIN_MSG_MAP(SDpiHandler<CFindDlg>)
			CHAIN_MSG_MAP(SHostWnd)
		END_MSG_MAP()

		virtual void OnFinalMessage(HWND hWnd);

		IFindListener * m_pFindListener;

		SEdit *	m_pEditTarget;
		SEdit * m_pEditReplace;
		SCheckBox * m_pMatchCase;
		SCheckBox * m_pMatchWholeWord;
		SStringT m_strFindText;
	};

SNSEND
