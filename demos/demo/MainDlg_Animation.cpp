/**
* Copyright (C) 2014-2050
* All rights reserved.
*
* @file       MainDlg_Animation.cpp
* @brief      CMainDlg 动画与托盘演示
* @version    v2.0
* @author     SOUI group
* @date       2014/08/15
*
* Describe    集中实现与动画、托盘图标相关的成员函数,展示 SOUI 的三种动画形态:
*               - 补间动画 IAnimation :由 XML(anim:xxx)定义,SetAnimation 应用到
*                 任意 SWindow,支持 setStartOffset 随机延迟启动;
*               - 属性动画 IValueAnimator :由 XML(valueAni:xxx)定义,值本身不绑定
*                 控件,需要监听者(onAnimationUpdate/onAnimationEnd)自行消费;
*               - 宿主动画 StartHostAnimation :对整个宿主窗口(原生窗口)做动画。
*             另包含托盘图标(SShellTray)的消息响应演示。
*/

#include "stdafx.h"
#include "MainDlg.h"
#include <valueAnimator/SValueAnimator.h>
#include <helper/SMenuEx.h>

#define kLogTag "anidlg"

/**
* @brief      SOUI 3.0 动画初始化
*
* Describe    1. 给 logo 图片(img_soui)挂载 anim:rotate 定义的旋转补间动画;
*             2. 启动 1 秒周期定时器 TIMER_SOUI4,周期性向 wnd_ani_host 投放
*                "飘心"动画(见 OnTimer)。
*             动画 XML 由 SApplication::LoadAnimation 从资源包的 anim 分区加载,
*             同一个 IAnimation 对象只允许挂到一个窗口,用完需 Release。
*/
void CMainDlg::InitSoui3Animation()
{
	SWindow *pWnd = FindChildByName(L"img_soui");
	if (pWnd)
	{
		IAnimation *pAni = SApplication::getSingletonPtr()->LoadAnimation(_T("anim:rotate"));
		if(pAni)
		{
			pWnd->SetAnimation(pAni);
			pAni->Release();
		}
	}
	SNativeWnd::SetTimer(TIMER_SOUI4,1000);	//start timer.
}

/**
* @brief      定时器响应
* @param      idEvent  触发的定时器 ID
*
* Describe    TIMER_QUIT  :消息框演示中 3 秒无人干预时强制退出整个应用;
*             TIMER_SOUI4 :每次触发都在 wnd_ani_host 末尾创建一个爱心子窗口,
*                          挂上 anim:love 动画并随机延迟 0~100ms 启动。
*             爱心窗口通过 SetUserData(TIMER_SOUI4) 打标记,动画自然结束时会
*             触发 EventSwndAnimationStop,由 OnAnimationStop 据此将其销毁,
*             从而避免窗口数量无限增长。
*/
void CMainDlg::OnTimer(UINT_PTR idEvent)
{
	SetMsgHandled(FALSE);
	if(idEvent==TIMER_QUIT)
	{
		SNativeWnd::KillTimer(idEvent);
		PostQuitMessage(-3);
	}else if(idEvent == TIMER_SOUI4)
	{
		SWindow *pAniHost = FindChildByName(L"wnd_ani_host");
		//宿主不可见(比如演示页不在前台)时跳过投放,定时器照常计时
		if (pAniHost && pAniHost->IsVisible(TRUE))
		{
			IAnimation *pAni = SApplication::getSingletonPtr()->LoadAnimation(_T("anim:love"));
			if(pAni)
			{
				//xml_love 中描述了一个爱心图片窗口,以 include 方式动态创建
				const WCHAR * kLoveXml= L"<include src=\"LAYOUT:xml_love\"/>";
				BOOL bLoad= pAniHost->CreateChildrenFromXml(kLoveXml);
				if(bLoad)
				{
					SWindow *pLove = pAniHost->GetWindow(GSW_LASTCHILD);
					pAniHost->UpdateLayout();
					//标记该窗口由 TIMER_SOUI4 投放,动画结束后自动销毁
					pLove->SetUserData(TIMER_SOUI4);
					pAni->setStartOffset(rand()%100);//random delay max to 100 ms to play the animation.
					pLove->SetAnimation(pAni);
				}
				pAni->Release();
			}
		}
	}
}

/**
* @brief      补间动画播放结束事件
* @param      e  EventSwndAnimationStop 事件
*
* Describe    只有带 TIMER_SOUI4 标记的窗口(OnTimer 投放的爱心)才自动销毁,
*             其余窗口(如 img_soui 的旋转)动画结束后保持最终状态。
*/
void CMainDlg::OnAnimationStop(IEvtArgs *e)
{
	SWindow *pSender = sobj_cast<SWindow>(e->Sender());
	if(pSender && pSender->GetUserData()==TIMER_SOUI4){
		pSender->Destroy();
	}
}

/**
* @brief      左侧面板开关联动演示
* @param      e  SToggle 的点击事件
*
* Describe    根据开关状态给 pane_left 挂载 slide_show / slide_hide 两个平移动画,
*             演示"同一控件在不同状态下复用不同动画资源"的典型用法。
*/
void CMainDlg::OnToggleLeft(IEvtArgs *e)
{
	SToggle *pToggle = sobj_cast<SToggle>(e->Sender());
	SASSERT(pToggle);
	SWindow *pWnd = FindChildByName(L"pane_left");
	if(!pWnd)
		return;
	if(pToggle->GetToggle())
	{
		IAnimation *pAni = SApplication::getSingletonPtr()->LoadAnimation(_T("anim:slide_show"));
		if(pAni)
		{
			pWnd->SetAnimation(pAni);
			pAni->Release();
		}
	}else
	{
		IAnimation *pAni = SApplication::getSingletonPtr()->LoadAnimation(_T("anim:slide_hide"));
		if(pAni)
		{
			pWnd->SetAnimation(pAni);
			pAni->Release();
		}
	}
}

/**
* @brief      点击 SOUI logo 启动颜色属性动画
*
* Describe    属性动画与补间动画不同:它只产生"值",不直接作用于控件。
*             这里加载 valueAni:colorAni 后,由宿主窗口自身充当监听者
*             (CMainDlg 混入了 IAnimatorListener/IAnimatorUpdateListener),
*             在 onAnimationUpdate 中把当前颜色应用到 tree_test 的背景。
*/
void CMainDlg::OnSouiClick()
{
	IValueAnimator * pAni = SApplication::getSingletonPtr()->LoadValueAnimator(_T("valueAni:colorAni"));
	if(pAni)
	{
		pAni->addListener(this);
		pAni->addUpdateListener(this);
		pAni->start(this);
	}
}

/**
* @brief      启动宿主窗口级动画
*
* Describe    StartHostAnimation 对整个原生窗口(而非某个 SWindow)做动画,
*             anim:anihost 中定义了位移+透明度组合,演示 DUI 窗口整体的入场效果。
*/
void CMainDlg::OnSetHostAnimation()
{
	IAnimation *pAni = SApplication::getSingletonPtr()->LoadAnimation(_T("anim:anihost"));
	if (pAni)
	{
		StartHostAnimation(pAni);
		pAni->Release();
	}
}

/**
* @brief      属性动画结束回调(IAnimatorListener)
* @param      pAnimator  结束的动画器
*
* Describe    把 tree_test 的背景恢复为无效色(全透明),撤销动画期间写入的颜色;
*             注意动画器在 start 时被引用计数+1,结束时必须 Release 归还。
*/
void CMainDlg::onAnimationEnd(IValueAnimator * pAnimator)
{
	SWindow *pTst = FindChildByName(L"tree_test");
	if(pTst)
	{
		pTst->SetAttribute(L"colorBkgnd",L"RGBA(255,255,255,0)");//set invalid colorBkgnd
		pAnimator->Release();
	}
}

/**
* @brief      属性动画每帧更新回调(IAnimatorUpdateListener)
* @param      pAnimator  正在更新的动画器
*
* Describe    将 SColorAnimator 的当前值格式化为 RGBA 字符串写入 tree_test 背景,
*             实现"背景色渐变"效果。sobj_cast 判定动画器具体类型,非颜色动画
*             直接忽略。
*/
void CMainDlg::onAnimationUpdate(IValueAnimator *pAnimator)
{
	SWindow *pTst = FindChildByName(L"tree_test");
	if(pTst)
	{
		SColorAnimator *ani = sobj_cast<SColorAnimator>(pAnimator);
		if(ani)
		{
			SStringW strColor;
			SColor cr(ani->getValue());
			strColor.Format(L"RGBA(%d,%d,%d,%d)",cr.r,cr.g,cr.b,cr.a);
			pTst->SetAttribute(L"colorBkgnd",strColor);
		}
	}
}

/**
* @brief      托盘图标消息响应
* @param      e  EventTrayNotify 事件,lp 字段为底层鼠标消息
*
* Describe    左键按下:切换主窗口显示/隐藏,最小化状态下先恢复;
*             右键按下:弹出 SMENUEX 自绘菜单(menu_tray)。
*             托盘对象的启动动画在 OnInitDialog 中通过 SShellTray::StartAni 完成。
*/
void CMainDlg::OnShellTrayNotify(IEvtArgs * e)
{
	EventTrayNotify *e2 = sobj_cast<EventTrayNotify>(e);
	SShellTray *pTray = sobj_cast<SShellTray>(e->Sender());
	switch(e2->lp)
	{
	case WM_LBUTTONDOWN:
		if(IsWindowVisible())
		{
			ShowWindow(SW_HIDE);
		}else
		{
			ShowWindow(SW_SHOW);
			if(IsIconic())
			{
				OnRestore();
			}
		}
		break;
	case WM_RBUTTONDOWN:
		{
			//SMENUEX 是增强版自绘菜单,支持在菜单项中放置任意控件
			SMenuEx tmenuex;
			if (tmenuex.LoadMenu(_T("SMENUEX:menu_tray")))
			{
				POINT pt;
				GetCursorPos(&pt);
				//托盘消息到达时宿主可能不是前台窗口,先置前台保证菜单能正常收起
				SetForegroundWindow(m_hWnd);
				tmenuex.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
			}
		}
		break;
	}
}
