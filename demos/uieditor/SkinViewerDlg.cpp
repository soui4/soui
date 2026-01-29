#include "stdafx.h"
#include "SkinViewerDlg.h"
#include "ResManger.h"

CSkinViewerDlg::CSkinViewerDlg(ResManger *pResMgr)
	: SHostDialog(_T("LAYOUT:dlg_skin_viewer"))
	, m_pResMgr(pResMgr)
	, m_pListBox(NULL)
	, m_pImgPreview(NULL)
	, m_pBtnPrev(NULL)
	, m_pBtnNext(NULL)
	, m_pLblIndex(NULL)
	, m_nCurrentSkinIndex(0)
	, m_nCurrentStateIndex(0)
{}

CSkinViewerDlg::~CSkinViewerDlg()
{}

SStringT CSkinViewerDlg::GetSelectedSkin() const
{
	if(m_pListBox && m_pListBox->GetCurSel() != -1)
	{
		return m_pListBox->GetText(m_pListBox->GetCurSel());
	}
	return _T("");
}

BOOL CSkinViewerDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// 获取控件
	m_pListBox = FindChildByName2<SListBox>(L"NAME_UIDESIGNER_NEW_SKIN_LB");
	m_pImgPreview = FindChildByName2<SImageWnd>(L"img_preview");
	m_pBtnPrev = FindChildByName2<SButton>(L"btn_prev");
	m_pBtnNext = FindChildByName2<SButton>(L"btn_next");
	m_pLblIndex = FindChildByName2<SStatic>(L"lbl_index");

	// 从 ResManager 获取皮肤列表
	if(m_pResMgr)
	{
		m_pResMgr->GetSkinList(m_vecSkinTypes);
	}

	// 填充皮肤类型列表
	if(m_pListBox)
	{
		m_pListBox->DeleteAll();
		for(size_t i = 0; i < m_vecSkinTypes.size(); i++)
		{
			m_pListBox->AddString(m_vecSkinTypes[i]);
		}
		if(!m_vecSkinTypes.empty())
		{
			m_pListBox->SetCurSel(0,TRUE);
		}
	}
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
    EventLBSelChanged *e2 = sobj_cast<EventLBSelChanged>(e);
    m_nCurrentSkinIndex = e2->nNewSel;
    m_nCurrentStateIndex = 0;
    UpdatePreview();
}

void CSkinViewerDlg::UpdatePreview()
{
	if(m_pImgPreview && m_pResMgr && m_nCurrentSkinIndex >= 0 && m_nCurrentSkinIndex < (int)m_vecSkinTypes.size())
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
			m_pImgPreview->SetSkin(pSkin);
			m_pImgPreview->SetIcon(m_nCurrentStateIndex);
			
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
