#include "stdafx.h"
#include "SkinViewerDlg.h"
#include "ResManger.h"
#include "AutoCompleteAdapter.h"
CSkinViewerDlg::CSkinViewerDlg(ResManger *pResMgr)
	: SHostDialog(_T("LAYOUT:dlg_skin_viewer"))
	, m_pResMgr(pResMgr)
	, m_pImgPreviewFull(NULL)
	, m_pBtnPrev(NULL)
	, m_pBtnNext(NULL)
	, m_pLblIndex(NULL)
	, m_nCurrentSkinIndex(0)
	, m_nCurrentStateIndex(0)
{}

CSkinViewerDlg::~CSkinViewerDlg()
{}

BOOL CSkinViewerDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// 获取控件
	m_plvSkins = FindChildByID2<SListView>(R.id.lv_skin);
	m_pImgPreviewFull = FindChildByID2<SImageWnd>(R.id.img_preview_full);
	m_pImgPreviewAuto = FindChildByID2<SImageWnd>(R.id.img_preview_auto);
	m_pBtnPrev = FindChildByID2<SButton>(R.id.btn_prev);
	m_pBtnNext = FindChildByID2<SButton>(R.id.btn_next);
	m_pLblIndex = FindChildByID2<SStatic>(R.id.lbl_index);

	m_bAutoSize = FindChildByID(R.id.chk_auto_size)->IsChecked();
	m_pImgPreviewFull->SetVisible(!m_bAutoSize);
	m_pImgPreviewAuto->SetVisible(m_bAutoSize);
	// 从 ResManager 获取皮肤列表
	if(m_pResMgr)
	{
		m_pResMgr->GetSkinList(m_vecSkinTypes);
	}
    CAutoCompleteSkinAdapter *pAdapter = new CAutoCompleteSkinAdapter;
    for (int i = 0; i < m_vecSkinTypes.size(); i++)
    {
        ISkinObj *pSkin = m_pResMgr->GetSkin(S_CT2W(m_vecSkinTypes[i]), 100);
        pAdapter->AddSkin(m_vecSkinTypes[i], pSkin);
    }
    m_plvSkins->SetAdapter(pAdapter);
	pAdapter->Release();
	m_plvSkins->SetSel(0,TRUE);
	return TRUE;
}

void CSkinViewerDlg::OnBtnCancel()
{
	EndDialog(IDCANCEL);
}

int CSkinViewerDlg::GetSkinStates() const
{
	if(m_pResMgr && m_nCurrentSkinIndex >= 0 && m_nCurrentSkinIndex < (int)m_vecSkinTypes.size())
	{
		// 获取当前选中的皮肤名称
		SStringT strSkinName = m_vecSkinTypes[m_nCurrentSkinIndex];
		
		// 通过 ResManager 获取 ISkinObj 对象
		ISkinObj *pSkin = m_pResMgr->GetSkin(S_CT2W(strSkinName), 100);
		
		if(pSkin)
		{
			return pSkin->GetStates();
		}
	}
	return 0;
}

void CSkinViewerDlg::OnBtnPrev()
{
	if(m_nCurrentStateIndex > 0)
	{
		m_nCurrentStateIndex--;
		UpdatePreview();
	}
}

void CSkinViewerDlg::OnBtnNext()
{
	m_nCurrentStateIndex++;
	UpdatePreview();
}

void CSkinViewerDlg::OnSkinTypeSelected(IEvtArgs *e)
{
    EventLVSelChanged *e2 = sobj_cast<EventLVSelChanged>(e);
    m_nCurrentSkinIndex = e2->iNewSel;
    m_nCurrentStateIndex = 0;
    UpdatePreview();
}

void CSkinViewerDlg::UpdatePreview()
{
	if(m_pImgPreviewFull && m_pResMgr && m_nCurrentSkinIndex >= 0 && m_nCurrentSkinIndex < (int)m_vecSkinTypes.size())
	{
		// 获取当前选中的皮肤名称
		SStringT strSkinName = m_vecSkinTypes[m_nCurrentSkinIndex];
		
		// 通过 ResManager 获取 ISkinObj 对象
		ISkinObj *pSkin = m_pResMgr->GetSkin(S_CT2W(strSkinName), 100);
		
		if(pSkin)
		{
			// 获取皮肤的状态数量
			int nStates = pSkin->GetStates();
			
			// 限制状态索引范围
			if(m_nCurrentStateIndex < 0) m_nCurrentStateIndex = 0;
			if(m_nCurrentStateIndex >= nStates) m_nCurrentStateIndex = nStates - 1;
			
			// 设置图片控件显示指定状态的皮肤
			m_pImgPreviewFull->SetSkin(pSkin);
			m_pImgPreviewFull->SetIcon(m_nCurrentStateIndex);
			m_pImgPreviewAuto->SetSkin(pSkin);
			m_pImgPreviewAuto->SetIcon(m_nCurrentStateIndex);
			// 更新索引标签
			if(m_pLblIndex)
			{
				m_pLblIndex->SetWindowText(SStringT().Format(_T("%d/%d"), m_nCurrentStateIndex + 1, nStates));
			}
			
			// 更新按钮状态
			if(m_pBtnPrev && m_pBtnNext)
			{
				m_pBtnPrev->EnableWindow(m_nCurrentStateIndex > 0);
				m_pBtnNext->EnableWindow(m_nCurrentStateIndex < nStates - 1);
			}
		}
	}
}

void CSkinViewerDlg::OnChkAutoSize(IEvtArgs *e)
{
	EventSwndStateChanged *e2 = sobj_cast<EventSwndStateChanged>(e);
	if(EventSwndStateChanged_CheckState(e2,WndState_Check))
	{
		m_bAutoSize = e2->dwNewState&WndState_Check;
		m_pImgPreviewFull->SetVisible(!m_bAutoSize,TRUE);
		m_pImgPreviewAuto->SetVisible(m_bAutoSize,TRUE);
	}
}
