#pragma once
#include "ResManger.h"

class CSkinViewerDlg : public SHostDialog
{
public:
	CSkinViewerDlg(ResManger *pResMgr);
	~CSkinViewerDlg();

	SStringT GetSelectedSkin() const;

protected:
	BOOL OnInitDialog(HWND hWnd, LPARAM lParam);
	void OnBtnCancel();
	void OnBtnPrev();
	void OnBtnNext();
	void OnSkinTypeSelected(IEvtArgs *e);
	void UpdatePreview();

	int GetSkinStates() const;
	
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btnCancel", OnBtnCancel)
		EVENT_NAME_COMMAND(L"btn_prev", OnBtnPrev)
		EVENT_NAME_COMMAND(L"btn_next", OnBtnNext)
        EVENT_NAME_HANDLER(L"NAME_UIDESIGNER_NEW_SKIN_LB", EventLBSelChanged::EventID, OnSkinTypeSelected)
	EVENT_MAP_END2(SHostDialog)
private:
	ResManger *m_pResMgr;
	SListBox *m_pListBox;
	SImageWnd *m_pImgPreview;
	SButton *m_pBtnPrev;
	SButton *m_pBtnNext;
	SStatic *m_pLblIndex;
	std::vector<SStringT> m_vecSkinTypes;
	int m_nCurrentSkinIndex;
	int m_nCurrentStateIndex;

	BEGIN_MSG_MAP_EX(CSkinViewerDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(SHostDialog)
	END_MSG_MAP()
};
