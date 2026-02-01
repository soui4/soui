#pragma once
#include "ResManger.h"

class CSkinViewerDlg : public SHostDialog
{
public:
	CSkinViewerDlg(ResManger *pResMgr);
	~CSkinViewerDlg();

protected:
	BOOL OnInitDialog(HWND hWnd, LPARAM lParam);
	void OnBtnCancel();
	void OnBtnPrev();
	void OnBtnNext();
	void OnSkinTypeSelected(IEvtArgs *e);
	void UpdatePreview();
	void OnChkAutoSize(IEvtArgs *e);

	int GetSkinStates() const;
	
	EVENT_MAP_BEGIN()
		EVENT_ID_COMMAND(R.id.btnCancel, OnBtnCancel)
		EVENT_ID_COMMAND(R.id.btn_prev, OnBtnPrev)
		EVENT_ID_COMMAND(R.id.btn_next, OnBtnNext)
        EVENT_ID_HANDLER(R.id.lv_skin, EventLVSelChanged::EventID, OnSkinTypeSelected)
		EVENT_ID_HANDLER(R.id.chk_auto_size,EventSwndStateChanged::EventID,OnChkAutoSize)
	EVENT_MAP_END2(SHostDialog)
private:
	ResManger *m_pResMgr;
	SListView *m_plvSkins;
	SImageWnd *m_pImgPreviewFull;
	SImageWnd *m_pImgPreviewAuto;
	SButton *m_pBtnPrev;
	SButton *m_pBtnNext;
	SStatic *m_pLblIndex;
	std::vector<SStringT> m_vecSkinTypes;
	int m_nCurrentSkinIndex;
	int m_nCurrentStateIndex;
	BOOL m_bAutoSize;
	BEGIN_MSG_MAP_EX(CSkinViewerDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(SHostDialog)
	END_MSG_MAP()
};
