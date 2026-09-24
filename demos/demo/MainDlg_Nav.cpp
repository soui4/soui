/**
* Copyright (C) 2014-2050
* All rights reserved.
*
* @file       MainDlg_Nav.cpp
* @brief      CMainDlg 宫格首页导航
* @version    v2.0
* @author     SOUI group
* @date       2014/08/15
*
* Describe    宫格首页(page_home.xml)的页面切换逻辑(两级 tab 结构):
*               - tab_main 仅两页:0=home(宫格首页),1=contents(导航栏+分区容器);
*               - 卡片点击 -> OnNavCard -> NavigateToPage(1..7):先将 tab_contents
*                 无动画切到目标分区,再由 tab_main 以滑动动画进入 contents 页;
*               - 返回按钮 -> OnNavBack   -> NavigateToPage(0):tab_main 滑动动画回 home;
*               - 导航栏(nav_bar)位于 contents 页内部,显隐随 tab_main 页切换
*                 自动完成,无需代码联动;标题文本由 NavigateToPage 设置。
*/

#include "stdafx.h"
#include "MainDlg.h"

#define kLogTag "navdlg"

/**
* @brief      导航初始化:缓存导航相关控件指针
*
* Describe    OnInitDialog 阶段调用。控件指针缓存到成员变量,
*             避免每次导航都按名字查找。
*/
void CMainDlg::InitPageNav()
{
	m_pMainTab     = FindChildByName2<STabCtrl>(L"tab_main");
	m_pContentsTab = FindChildByName2<STabCtrl>(L"tab_contents");
	m_pNavTitle    = FindChildByName(L"txt_nav_title");
}

/**
* @brief      宫格卡片点击的统一入口
*
* Describe    7 张卡片的事件在 EVENT_MAP 中统一绑定到本函数,
*             通过 sender 的 name 查映射表得到目标页序与标题。
*             卡片 name 与 dlg_main.xml 中分区页序的对应关系
*             (NavigateToPage 内部将其映射为 tab_contents 的页序):
*               card_ctrls=1 基础控件  card_webkit=2 教程  card_animator=3 动画
*               card_layout=4 布局     card_misc=5 杂项    card_skia=6 Skia
*               card_about=7 关于
*/
void CMainDlg::OnNavCard(IEvtArgs *e)
{
	SWindow *pSender = sobj_cast<SWindow>(e->Sender());
	SASSERT(pSender);
	const SStringW strName = pSender->GetName();

	struct NavItem { const wchar_t *pszCard; int iPage; const wchar_t *pszTitle; };
	static const NavItem kNavMap[] = {
		{ L"card_ctrls",    1, L"基础控件" },
		{ L"card_webkit",   2, L"教程" },
		{ L"card_animator", 3, L"动画" },
		{ L"card_layout",   4, L"布局" },
		{ L"card_misc",     5, L"杂项" },
		{ L"card_skia",     6, L"Skia" },
		{ L"card_about",    7, L"关于" },
	};
	for(int i=0;i<ARRAYSIZE(kNavMap);i++)
	{
		if(strName.CompareNoCase(kNavMap[i].pszCard)==0)
		{
			NavigateToPage(kNavMap[i].iPage,kNavMap[i].pszTitle);
			return;
		}
	}
	SLOGW()<<"unknown card clicked: "<<strName.c_str();
}

/**
* @brief      导航栏返回按钮:回到宫格首页
*/
void CMainDlg::OnNavBack()
{
	NavigateToPage(0,L"");
}

/**
* @brief      执行页面切换(两级 tab 协同)
* @param      iPage     0=宫格首页;1..7=第 iPage 个演示分区
* @param      pszTitle  导航栏显示的标题(home 页不使用)
*
* Describe    进入分区:先将 tab_contents 切到目标分区页,再让 tab_main
*             动画切入 contents 页,滑入时即显示目标分区。tab_contents
*             的切换无需主动禁用动画:tab_main 停在 home 页时 contents
*             页不可见,STabCtrl::SetCurSel 对不可见容器直接显隐,不创建
*             滑动动画。返回首页时仅 tab_main 动画回 home,contents 保留
*             原分区,下次进入再按需切换。
*             演示 SetCurSel 的整型页序用法;对比 OnCommand 中按页 title
*             跳转的字符串用法(两种方式 SOUI 均支持)。
*/
void CMainDlg::NavigateToPage(int iPage, const wchar_t *pszTitle)
{
	if(m_pMainTab == NULL)
		return;
	if(iPage > 0)
	{
		if(m_pContentsTab && m_pContentsTab->GetCurSel() != iPage-1)
			m_pContentsTab->SetCurSel(iPage-1);
		if(m_pNavTitle && pszTitle && pszTitle[0])
		{
			m_pNavTitle->SetWindowText(S_CW2T(SStringW(pszTitle)));
		}
		if(m_pMainTab->GetCurSel() != 1)
			m_pMainTab->SetCurSel(1);
	}
	else
	{
		if(m_pMainTab->GetCurSel() != 0)
			m_pMainTab->SetCurSel(0);
	}
}

/**
* @brief      radio button 页:点击带 Check 状态的控件切换 tab_radio2 页
*
* Describe    演示 EventSwndStateChanged 的状态位检查方式:
*             通过 id 与 10000 的差值计算目标页序(与布局中的控件 id 约定配套)。
*             该页同时演示了 STabCtrlHeaderBinder 的自动绑定方式(见 OnInitDialog),
*             本函数为手动切换的备选实现。
*/
void CMainDlg::OnTabPageRadioSwitch(IEvtArgs *pEvt)
{
	EventSwndStateChanged *pEvt2 = sobj_cast<EventSwndStateChanged>(pEvt);
	if(EventSwndStateChanged_CheckState(pEvt2,WndState_Check) && (pEvt2->dwNewState & WndState_Check))
	{
		int id= pEvt->IdFrom();
		STabCtrl *pTab =FindChildByName2<STabCtrl>(L"tab_radio2");
		if(pTab) pTab->SetCurSel(id-10000);
	}
}
