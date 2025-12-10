// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <valueAnimator/SPropertyAnimator.h>

class CMainDlg : public SHostWnd
			   , public SDpiHandler<CMainDlg>
{
public:
	CMainDlg();
	~CMainDlg();

	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnSize(UINT nType, CSize size);
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

	// 布局动画演示方法
	void OnWidthAnimation();
	void OnHeightAnimation();
	void OnSizeAnimation();
	void OnWeightAnimation();
	void OnMultiPropertyAnimation();
	void OnBtnAnimation();

protected:
	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
		
		// 布局动画事件
		EVENT_NAME_COMMAND(L"btn_width_anim", OnWidthAnimation)
		EVENT_NAME_COMMAND(L"btn_height_anim", OnHeightAnimation)
		EVENT_NAME_COMMAND(L"btn_size_anim", OnSizeAnimation)
		EVENT_NAME_COMMAND(L"btn_weight_anim", OnWeightAnimation)
		EVENT_NAME_COMMAND(L"btn_multi_anim", OnMultiPropertyAnimation)
		EVENT_NAME_COMMAND(L"btn_ani", OnBtnAnimation)
	EVENT_MAP_END2(SHostWnd)
		
	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CMainDlg)
		CHAIN_MSG_MAP(SDpiHandler<CMainDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

private:
	SWindow *m_pAnimationTarget;  // 动画目标窗口
	SAutoRefPtr<IValueAnimator> m_runningAnimator1;// 正在运行的动画
	SAutoRefPtr<IValueAnimator> m_runningAnimator2;// 正在运行的动画
};
