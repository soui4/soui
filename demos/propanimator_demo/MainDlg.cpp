// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include <helper/SMenuEx.h>
#include <valueAnimator/SPropertyAnimator.h>
#include <layout/SAnchorLayout.h>

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
	, m_pAnimationTarget(NULL)
{
}

CMainDlg::~CMainDlg()
{
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// 获取动画目标窗口和容器
	m_pAnimationTarget = FindChildByName(L"animation_target");
	return 0;
}

//TODO:消息映射
void CMainDlg::OnClose()
{
	SNativeWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}

void CMainDlg::OnWidthAnimation()
{
	if (!m_pAnimationTarget)
		return;

	// 使用Android风格的API创建宽度动画 - 从100dp到300dp
	SLayoutSize values[] = {
		SLayoutSize(100.0f, dp),
		SLayoutSize(300.0f, dp)
	};

	// 使用全局便利函数创建动画器
	IValueAnimator *pAnimator = SPropertyAnimator::ofLayoutSize(
		m_pAnimationTarget, 
		LayoutProperty::WIDTH, 
		values, 
		2
	);

	if (pAnimator)
	{
		pAnimator->setDuration(1000);  // 1秒动画
		pAnimator->start(this);
		pAnimator->Release();
	}
}

void CMainDlg::OnHeightAnimation()
{
	if (!m_pAnimationTarget)
		return;

	// 高度动画 - 从50dp到150dp
	SLayoutSize values[] = {
		SLayoutSize(50.0f, dp),
		SLayoutSize(150.0f, dp)
	};

	IValueAnimator *pAnimator = SPropertyAnimator::ofLayoutSize(
		m_pAnimationTarget, 
		LayoutProperty::HEIGHT, 
		values, 
		2
	);

	if (pAnimator)
	{
		pAnimator->setDuration(800);
		pAnimator->start(this);
		pAnimator->Release();
	}
}

void CMainDlg::OnSizeAnimation()
{
	if (!m_pAnimationTarget)
		return;

	// 同时动画宽度和高度 - 使用PropertyValuesHolder
	SLayoutSize widthValues[] = {
		SLayoutSize(100.0f, dp),
		SLayoutSize(200.0f, dp)
	};
	
	SLayoutSize heightValues[] = {
		SLayoutSize(50.0f, dp),
		SLayoutSize(100.0f, dp)
	};

	// 创建属性值持有者
	IPropertyValuesHolder *pWidthHolder = SPropertyValuesHolder::ofLayoutSize(
		LayoutProperty::WIDTH, widthValues, 2);
	IPropertyValuesHolder *pHeightHolder = SPropertyValuesHolder::ofLayoutSize(
		LayoutProperty::HEIGHT, heightValues, 2);

	IPropertyValuesHolder *holders[] = { pWidthHolder, pHeightHolder };

	// 创建多属性动画器
	IValueAnimator *pAnimator = SPropertyAnimator::ofPropertyValuesHolder(
		m_pAnimationTarget, holders, 2);

	if (pAnimator)
	{
		pAnimator->setDuration(1200);
		pAnimator->start(this);
		pAnimator->Release();
	}
	pWidthHolder->Release();
	pHeightHolder->Release();
}

void CMainDlg::OnWeightAnimation()
{
	if (!m_pAnimationTarget)
		return;
	// 权重动画 - 从0.5到2.0
	float values[] = { 0.5f, 2.0f };

	IValueAnimator *pAnimator = SPropertyAnimator::ofFloat(
		m_pAnimationTarget, 
		LayoutProperty::WEIGHT, 
		values, 
		2
	);

	if (pAnimator)
	{
		pAnimator->setDuration(1500);
		pAnimator->start(this);
		pAnimator->Release();
	}
}

void CMainDlg::OnMultiPropertyAnimation()
{
	if (!m_pAnimationTarget)
		return;

	// 复杂的多属性关键帧动画 - 宽度、高度、权重同时动画
	SLayoutSize widthValues[] = {
		SLayoutSize(80.0f, dp),
		SLayoutSize(160.0f, dp),
		SLayoutSize(120.0f, dp)  // 关键帧动画
	};
	
	SLayoutSize heightValues[] = {
		SLayoutSize(40.0f, dp),
		SLayoutSize(120.0f, dp),
		SLayoutSize(80.0f, dp)
	};

	float weightValues[] = { 1.0f, 3.0f, 1.5f };

	// 创建多个属性值持有者
	IPropertyValuesHolder *pWidthHolder = SPropertyValuesHolder::ofLayoutSize(
		LayoutProperty::WIDTH, widthValues, 3);
	IPropertyValuesHolder *pHeightHolder = SPropertyValuesHolder::ofLayoutSize(
		LayoutProperty::HEIGHT, heightValues, 3);
	IPropertyValuesHolder *pWeightHolder = SPropertyValuesHolder::ofFloat(
		LayoutProperty::WEIGHT, weightValues, 3);

	IPropertyValuesHolder *holders[] = { pWidthHolder, pHeightHolder, pWeightHolder };

	// 创建复杂多属性动画器
	IValueAnimator *pAnimator = SPropertyAnimator::ofPropertyValuesHolder(
		m_pAnimationTarget, holders, 3);

	if (pAnimator)
	{
		pAnimator->setDuration(2000);  // 2秒动画
		pAnimator->start(m_pAnimationTarget->GetContainer());
	}
	pWidthHolder->Release();
	pHeightHolder->Release();
	pWeightHolder->Release();
}

static BOOL _GetWindowAnchorPos(IWindow *pWnd, AnchorPos *pAnchorPos)
{
	ILayoutParam *pLayoutParam = pWnd->GetLayoutParam();
	if (!pLayoutParam->IsClass(SAnchorLayoutParam::GetClassName()))
		return FALSE;
	SAnchorLayoutParamStruct *pAnchorLayoutParam = (SAnchorLayoutParamStruct *)pLayoutParam->GetRawData();
	*pAnchorPos = pAnchorLayoutParam->pos;
	return TRUE;
}

void CMainDlg::OnBtnAnimation()
{
	SWindow * pWnd_left_top = FindChildByName(L"win_left_top");
	SWindow * pWnd_right_top = FindChildByName(L"win_right_top");
	SWindow * pWnd_left_bottom = FindChildByName(L"win_left_bottom");
	SWindow * pWnd_right_bottom = FindChildByName(L"win_right_bottom");
	SWindow * pWnd_center = FindChildByName(L"win_center");

	AnchorPos anchorPos[5];
	_GetWindowAnchorPos(pWnd_left_top, anchorPos+0);
	_GetWindowAnchorPos(pWnd_right_top, anchorPos+1);
	_GetWindowAnchorPos(pWnd_left_bottom, anchorPos+2);
	_GetWindowAnchorPos(pWnd_right_bottom, anchorPos+3);
	_GetWindowAnchorPos(pWnd_center, anchorPos+4);

	if(1){
		//animate pWnd_left_top to pWnd_left_bottom
		AnchorPos pos[] = {
			anchorPos[0],
			anchorPos[4],
			anchorPos[2]
		};
		if(m_runningAnimator1) 
			m_runningAnimator1->end();
        m_runningAnimator1.Attach(SPropertyAnimator::ofPosition(pWnd_left_top, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos)));
		if (m_runningAnimator1)
		{
			m_runningAnimator1->setDuration(2000);
			m_runningAnimator1->start(this);
		}
	}
	if(1){
		//animate pWnd_left_bottom  to pWnd_left_top
		AnchorPos pos[] = {
			anchorPos[2],
			anchorPos[4],
			anchorPos[0]
		};
		if(m_runningAnimator2) 
			m_runningAnimator2->end();		
        m_runningAnimator2.Attach(SPropertyAnimator::ofPosition(pWnd_left_bottom, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos)));
		if (m_runningAnimator2)
		{
			m_runningAnimator2->setDuration(2000);
			m_runningAnimator2->start(this);
		}
	}
}