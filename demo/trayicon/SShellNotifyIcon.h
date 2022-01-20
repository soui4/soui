/**
 * Copyright (C) 2014-2050 359501300
 * All rights reserved.
 *
 * @file       SShellNotifyIcon.h
 * @brief
 * @version    v2.0
 * @author     359501300
 * @date       2017-09-11
 *
 * Describe
 */
#pragma once
#include <ShellAPI.h>
#include <core/SWnd.h>
#include <core/SNativeWnd.h>
 /**
  * @class      SShellNotifyIcon
  * @brief      ShellNotifyIcon
  *
  * Describe    ShellNotifyIcon
  */
#define WM_ICONNOTIFY (WM_USER+1111)
#ifndef ID_TASKBARICON
#define ID_TASKBARICON	100
#endif // !ID_TASKBARICON
#define ANI_TIMER_ID 8

namespace SOUI
{
	enum MenuType
	{
		menu, menuex, unknow
	};

	class SShellNotifyIcon;
	class CShellNotifyHwnd2 :public SNativeWnd
	{
	public:
		CShellNotifyHwnd2(SShellNotifyIcon* shellnotifyicon);
		~CShellNotifyHwnd2(){}
	protected:
		//托盘通知消息处理函数
		LRESULT OnIconNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL/* bHandled*/);
		LRESULT OnTaskbarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL);
		virtual void OnFinalMessage(HWND hWnd);
		void OnTimer(UINT_PTR nIDEvent);

		BEGIN_MSG_MAP_EX(CShellNotifyHwnd2)
			//托盘消息处理
			MESSAGE_HANDLER(MsgTaskbarCreated, OnTaskbarCreated)
			MSG_WM_TIMER(OnTimer)
			MESSAGE_HANDLER(WM_ICONNOTIFY, OnIconNotify)
			CHAIN_MSG_MAP(SNativeWnd)
		END_MSG_MAP()
	private:
		SShellNotifyIcon	*m_ShellNotifyIcon;
		UINT                MsgTaskbarCreated;		
	};

	class SShellNotifyIcon :public SWindow
	{
		SOUI_CLASS_NAME(SWindow, L"shellnotifyicon")
		friend class CShellNotifyHwnd2;
	public:
		/**
		 * SShellNotifyIcon::SShellNotifyIcon
		 * @brief    构造函数
		 *
		 * Describe  构造函数
		 */
		SShellNotifyIcon();

		/**
		 * SShellNotifyIcon::~SShellNotifyIcon
		 * @brief    析构函数
		 *
		 * Describe  析构函数
		 */
		~SShellNotifyIcon();

		BOOL Show();

		BOOL Hide();
		BOOL ShowNotify(LPCTSTR szMsg, LPCTSTR szTitle = NULL);
		void ShowMenu();
		void StartAni();
		void StopAni();
		bool IsRunAni() { return m_bRunAni; }
		void SetDefIconIdx(int iIdx);

	protected:
		virtual void WINAPI OnFinalRelease();
	protected:
		void NextFrame();
		virtual BOOL CreateChildren(SXmlNode xmlNode);
		void IniNotifyIconData(HWND hOwner, HICON hIcon, UINT flags, LPCTSTR szTip);
	protected:
		HRESULT OnAttrMenu(SStringW strValue, BOOL bLoading);
		HRESULT OnAttrIcons(SStringW strValue, BOOL bLoading);
		HRESULT OnAttrTip(SStringW szTip,BOOL bLoading=FALSE);

		SOUI_ATTRS_BEGIN()
			ATTR_CUSTOM(L"icons", OnAttrIcons)
			ATTR_CUSTOM(L"tip", OnAttrTip)
			ATTR_CUSTOM(L"menu", OnAttrMenu)
			ATTR_INT(L"deficoidx", m_iDefIcon, FALSE)
			ATTR_INT(L"startframe", m_iStartFrame, FALSE)
			ATTR_INT(L"duration", m_iDuration,FALSE)
		SOUI_ATTRS_END()
	private:
		SArray<HICON> m_ArrIcon;
		SStringT m_strTip;
		SStringT m_strMenu;
		MenuType m_menuType;
		NOTIFYICONDATA m_nid;
		CShellNotifyHwnd2 *m_MsgOnlyWnd;
		//  默认图标索引			开始帧位置		动画间隔时间
		int m_iDefIcon, m_iCurFrame, m_iStartFrame, m_iDuration;
		bool m_bRunAni;
	};
}