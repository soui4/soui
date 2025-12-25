// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include <helper/SMenuEx.h>
#include <valueAnimator/SPropertyAnimator.h>
#include <valueAnimator/SAnimatorSet.h>
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
		IPropertyAnimator *pAnimator = SPropertyAnimator::ofPosition(pWnd_left_top, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		//设置权重
		float weights[] = { 1.0f, 5.0f };
		pAnimator->GetPropertyValuesHolderByIndex(0)->SetKeyFrameWeights(weights, ARRAYSIZE(weights));
		m_runningAnimator1 = pAnimator;
		//as m_runningAnimator1 hold a ref, we need release pAnimator
		pAnimator->Release();
		m_runningAnimator1->setDuration(2000);
		m_runningAnimator1->start(this);
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

// ============================================================
// AnimatorSet演示方法
// ============================================================

/**
 * @brief 并行动画演示
 * 
 * 演示多个动画同时运行的效果。创建四个角上的窗口同时执行动画。
 */
void CMainDlg::OnParallelAnimation()
{
	SWindow * pWnd_left_top = FindChildByName(L"set_left_top");
	SWindow * pWnd_right_top = FindChildByName(L"set_right_top");
	SWindow * pWnd_left_bottom = FindChildByName(L"set_left_bottom");
	SWindow * pWnd_right_bottom = FindChildByName(L"set_right_bottom");

	if (!pWnd_left_top || !pWnd_right_top || !pWnd_left_bottom || !pWnd_right_bottom)
		return;

	// 结束之前的动画集
	if (m_runningAnimatorSet)
		m_runningAnimatorSet->end();

	// 创建动画集
	m_runningAnimatorSet.Attach(new SAnimatorSet());
	m_runningAnimatorSet->setPlayMode(PARALLEL);  // 设置为并行模式

	// 获取各窗口的当前位置
	AnchorPos anchorPos[4];
	_GetWindowAnchorPos(pWnd_left_top, anchorPos + 0);
	_GetWindowAnchorPos(pWnd_right_top, anchorPos + 1);
	_GetWindowAnchorPos(pWnd_left_bottom, anchorPos + 2);
	_GetWindowAnchorPos(pWnd_right_bottom, anchorPos + 3);

	// 动画1：左上角窗口向右下移动
	{
		AnchorPos pos[] = { 
			anchorPos[0], 
			anchorPos[3]
		};
		IPropertyAnimator *pAnimator = SPropertyAnimator::ofPosition(pWnd_left_top, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator)
		{
			m_runningAnimatorSet->addAnimator(pAnimator);
			pAnimator->Release();
		}
	}

	// 动画2：右上角窗口向左下移动
	{
		AnchorPos pos[] = { 
			anchorPos[1], 
			anchorPos[2]
		};
		IPropertyAnimator *pAnimator = SPropertyAnimator::ofPosition(pWnd_right_top, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator)
		{
			m_runningAnimatorSet->addAnimator(pAnimator);
			pAnimator->Release();
		}
	}

	// 动画3：左下角窗口向右上移动
	{
		AnchorPos pos[] = {
			anchorPos[2],
			anchorPos[1]
		};
		IPropertyAnimator *pAnimator = SPropertyAnimator::ofPosition(pWnd_left_bottom, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator)
		{
			m_runningAnimatorSet->addAnimator(pAnimator);
			pAnimator->Release();
		}
	}

	// 动画4：右下角窗口向左上移动
	{
		AnchorPos pos[] = { 
			anchorPos[3], 
			anchorPos[0]
		};
		IPropertyAnimator *pAnimator = SPropertyAnimator::ofPosition(pWnd_right_bottom, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator)
		{
			m_runningAnimatorSet->addAnimator(pAnimator);
			pAnimator->Release();
		}
	}

	// 启动动画集
	if (m_runningAnimatorSet)
	{
		m_runningAnimatorSet->start(this);
	}
}

/**
 * @brief 串行动画演示
 * 
 * 演示多个动画按顺序执行的效果。四个窗口按照顺时针方向依次执行动画。
 */
void CMainDlg::OnSequentialAnimation()
{
	SWindow * pWnd_left_top = FindChildByName(L"set_left_top");
	SWindow * pWnd_right_top = FindChildByName(L"set_right_top");
	SWindow * pWnd_left_bottom = FindChildByName(L"set_left_bottom");
	SWindow * pWnd_right_bottom = FindChildByName(L"set_right_bottom");

	if (!pWnd_left_top || !pWnd_right_top || !pWnd_left_bottom || !pWnd_right_bottom)
		return;

	// 结束之前的动画集
	if (m_runningAnimatorSet)
		m_runningAnimatorSet->end();

	// 创建动画集
	m_runningAnimatorSet.Attach(new SAnimatorSet());
	m_runningAnimatorSet->setPlayMode(SEQUENCE);  // 设置为串行模式

	// 获取各窗口的当前位置
	AnchorPos anchorPos[4];
	_GetWindowAnchorPos(pWnd_left_top, anchorPos + 0);
	_GetWindowAnchorPos(pWnd_right_top, anchorPos + 1);
	_GetWindowAnchorPos(pWnd_left_bottom, anchorPos + 2);
	_GetWindowAnchorPos(pWnd_right_bottom, anchorPos + 3);

	// 创建四个动画，按顺序执行
	SAutoRefPtr<IPropertyAnimator> pAnimator1 = NULL;
	SAutoRefPtr<IPropertyAnimator> pAnimator2 = NULL;
	SAutoRefPtr<IPropertyAnimator> pAnimator3 = NULL;
	SAutoRefPtr<IPropertyAnimator> pAnimator4 = NULL;

	AnchorPos centerPos = {4, 0, 0, -0.5f, -0.5f };  // 中心位置
	// 动画1：左上窗口向中心移动
	{
		AnchorPos pos[] = {
			anchorPos[0],
			centerPos
		};
		pAnimator1 = SPropertyAnimator::ofPosition(pWnd_left_top, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator1)
		{
			pAnimator1->setDuration(500);
			m_runningAnimatorSet->addAnimator(pAnimator1);
		}
	}

	// 动画2：右上窗口向中心移动（在动画1后）
	{
		AnchorPos pos[] = {
			anchorPos[1],
			centerPos
		};
		pAnimator2 = SPropertyAnimator::ofPosition(pWnd_right_top, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator2)
		{
			pAnimator2->setDuration(500);
			m_runningAnimatorSet->addAnimatorAfter(pAnimator2, pAnimator1);
		}
	}

	// 动画3：左下窗口向中心移动（在动画2后）
	{
		AnchorPos pos[] = {
			anchorPos[2],
			centerPos
		};
		pAnimator3 = SPropertyAnimator::ofPosition(pWnd_left_bottom, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator3)
		{
			pAnimator3->setDuration(500);
			m_runningAnimatorSet->addAnimatorAfter(pAnimator3, pAnimator2);
		}
	}

	// 动画4：右下窗口向中心移动（在动画3后）
	{
		AnchorPos pos[] = {
			anchorPos[3],
			centerPos
		};
		pAnimator4 = SPropertyAnimator::ofPosition(pWnd_right_bottom, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator4)
		{
			pAnimator4->setDuration(500);
			m_runningAnimatorSet->addAnimatorAfter(pAnimator4, pAnimator3);
		}
	}

	// 启动动画集
	if (m_runningAnimatorSet)
	{
		m_runningAnimatorSet->start(this);
	}
}

/**
 * @brief 复杂动画集演示
 * 
 * 演示混合执行模式：
 * 1. 动画1执行
 * 2. 动画2和3在动画1后同时执行
 * 3. 动画4在动画2后执行
 * 4. 所有动画都返回原位
 */
void CMainDlg::OnComplexAnimatorSetAnimation()
{
	SWindow * pWnd_left_top = FindChildByName(L"set_left_top");
	SWindow * pWnd_right_top = FindChildByName(L"set_right_top");
	SWindow * pWnd_left_bottom = FindChildByName(L"set_left_bottom");
	SWindow * pWnd_right_bottom = FindChildByName(L"set_right_bottom");

	if (!pWnd_left_top || !pWnd_right_top || !pWnd_left_bottom || !pWnd_right_bottom)
		return;

	// 结束之前的动画集
	if (m_runningAnimatorSet)
		m_runningAnimatorSet->end();

	// 创建动画集
	m_runningAnimatorSet.Attach(new SAnimatorSet());

	// 获取各窗口的当前位置
	AnchorPos anchorPos[4];
	_GetWindowAnchorPos(pWnd_left_top, anchorPos + 0);
	_GetWindowAnchorPos(pWnd_right_top, anchorPos + 1);
	_GetWindowAnchorPos(pWnd_left_bottom, anchorPos + 2);
	_GetWindowAnchorPos(pWnd_right_bottom, anchorPos + 3);

	// 创建四个动画
	SAutoRefPtr<IPropertyAnimator> pAnimator1 = NULL;
	SAutoRefPtr<IPropertyAnimator> pAnimator2 = NULL;
	SAutoRefPtr<IPropertyAnimator> pAnimator3 = NULL;
	SAutoRefPtr<IPropertyAnimator> pAnimator4 = NULL;

	// ========== 第一阶段 ==========
	// 动画1：左上窗口向右移动
	{
		AnchorPos toPos = {2, 300, anchorPos[0].y,  -1, -1 };  // 向右移动
		AnchorPos pos[] = {
			anchorPos[0],
			toPos
		};
		pAnimator1 = SPropertyAnimator::ofPosition(pWnd_left_top, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator1)
		{
			pAnimator1->setDuration(600);
			m_runningAnimatorSet->addAnimator(pAnimator1);
		}
	}

	// ========== 第二阶段 ==========
	// 动画2：右上窗口向下移动（在动画1后，与动画3并行）
	{
		AnchorPos toPos = {8, anchorPos[1].x, 300, -1, -1 };  // 向下移动
		AnchorPos pos[] = {
			anchorPos[1],
			toPos
		};
		pAnimator2 = SPropertyAnimator::ofPosition(pWnd_right_top, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator2)
		{
			pAnimator2->setDuration(600);
			m_runningAnimatorSet->addAnimatorAfter(pAnimator2, pAnimator1);
		}
	}

	// 动画3：左下窗口向上移动（在动画1后，与动画2并行）
	{
		AnchorPos toPos = {0, anchorPos[2].x, 100, -1, -1 };  // 向上移动
		AnchorPos pos[] = {
			anchorPos[2],
			toPos
		};
		pAnimator3 = SPropertyAnimator::ofPosition(pWnd_left_bottom, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator3)
		{
			pAnimator3->setDuration(600);
			m_runningAnimatorSet->addAnimatorAfter(pAnimator3, pAnimator1);
		}
	}

	// ========== 第三阶段 ==========
	// 动画4：右下窗口向左移动（在动画2后）
	{
		AnchorPos toPos = {6, 100, anchorPos[3].y, -1, -1 };  // 向左移动
		AnchorPos pos[] = {
			anchorPos[3],
			toPos
		};
		pAnimator4 = SPropertyAnimator::ofPosition(pWnd_right_bottom, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator4)
		{
			pAnimator4->setDuration(600);
			m_runningAnimatorSet->addAnimatorAfter(pAnimator4, pAnimator2);
		}
	}

	// 启动动画集
	if (m_runningAnimatorSet)
	{
		m_runningAnimatorSet->start(this);
	}
}

/**
 * @brief 还原动画集演示窗口的位置
 * 
 * 将所有动画集演示窗口移动到原始位置
 */
void CMainDlg::OnBtnSetRestore()
{
	SWindow * pWnd_left_top = FindChildByName(L"set_left_top");
	SWindow * pWnd_right_top = FindChildByName(L"set_right_top");
	SWindow * pWnd_left_bottom = FindChildByName(L"set_left_bottom");
	SWindow * pWnd_right_bottom = FindChildByName(L"set_right_bottom");

	if (!pWnd_left_top || !pWnd_right_top || !pWnd_left_bottom || !pWnd_right_bottom)
		return;

	// 结束之前的动画集
	if (m_runningAnimatorSet)
		m_runningAnimatorSet->end();

	// 创建动画集
	m_runningAnimatorSet.Attach(new SAnimatorSet());

	// 获取各窗口的当前位置
	AnchorPos anchorPos[4];
	_GetWindowAnchorPos(pWnd_left_top, anchorPos + 0);
	_GetWindowAnchorPos(pWnd_right_top, anchorPos + 1);
	_GetWindowAnchorPos(pWnd_left_bottom, anchorPos + 2);
	_GetWindowAnchorPos(pWnd_right_bottom, anchorPos + 3);

	// 创建四个动画，按顺序执行
	SAutoRefPtr<IPropertyAnimator> pAnimator1 = NULL;
	SAutoRefPtr<IPropertyAnimator> pAnimator2 = NULL;
	SAutoRefPtr<IPropertyAnimator> pAnimator3 = NULL;
	SAutoRefPtr<IPropertyAnimator> pAnimator4 = NULL;

	// 动画1：左上窗口向左上移动
	{
		AnchorPos toPos = {0, 0, 0, 0, 0 };  // 左上
		AnchorPos pos[] = {
			anchorPos[0],
			toPos
		};
		pAnimator1 = SPropertyAnimator::ofPosition(pWnd_left_top, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator1)
		{
			pAnimator1->setDuration(500);
			m_runningAnimatorSet->addAnimator(pAnimator1);
		}
	}

	// 动画2：右上窗口向右上移动
	{
		AnchorPos toPos = {2, 0, 0, -1, 0 };  // 右上
		AnchorPos pos[] = {
			anchorPos[1],
			toPos
		};
		pAnimator2 = SPropertyAnimator::ofPosition(pWnd_right_top, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator2)
		{
			pAnimator2->setDuration(500);
			m_runningAnimatorSet->addAnimatorAfter(pAnimator2, pAnimator1);
		}
	}

	// 动画3：左下窗口向左下移动
	{
		AnchorPos toPos = {6, 0, 0, 0, -1 };  // 左下
		AnchorPos pos[] = {
			anchorPos[2],
			toPos
		};
		pAnimator3 = SPropertyAnimator::ofPosition(pWnd_left_bottom, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator3)
		{
			pAnimator3->setDuration(500);
			m_runningAnimatorSet->addAnimatorAfter(pAnimator3, pAnimator2);
		}
	}

	// 动画4：右下窗口向右下移动
	{
		AnchorPos toPos = {8, 0, 0, -1, -1 };  // 右下
		AnchorPos pos[] = {
			anchorPos[3],
			toPos
		};
		pAnimator4 = SPropertyAnimator::ofPosition(pWnd_right_bottom, LayoutProperty::POSITION, pos, ARRAYSIZE(pos), sizeof(AnchorPos));
		if (pAnimator4)
		{
			pAnimator4->setDuration(500);
			m_runningAnimatorSet->addAnimatorAfter(pAnimator4, pAnimator3);
		}
	}

	// 启动动画集
	if (m_runningAnimatorSet)
	{
		m_runningAnimatorSet->start(this);
	}
}