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
	struct IShellTrayListener
	{
		virtual void NextFrame() = 0;
		virtual void OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
		virtual void OnTaskbarRecreate() = 0;
	};

	class SShellTrayMsgWnd :public SNativeWnd
	{
	public:
		SShellTrayMsgWnd(IShellTrayListener* shellnotifyicon);
		~SShellTrayMsgWnd(){}
	protected:
		//托盘通知消息处理函数
		LRESULT OnIconNotify(UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnTaskbarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual void OnFinalMessage(HWND hWnd);

		void OnTimer(UINT_PTR nIDEvent);

		//托盘消息处理
		BEGIN_MSG_MAP_EX(SShellTrayMsgWnd)
			MSG_WM_TIMER(OnTimer)
			MESSAGE_HANDLER_EX(MsgTaskbarCreated, OnTaskbarCreated)
			MESSAGE_HANDLER_EX(WM_ICONNOTIFY, OnIconNotify)
			CHAIN_MSG_MAP(SNativeWnd)
		END_MSG_MAP()
	private:
		IShellTrayListener	*m_ShellNotifyIcon;
		UINT                MsgTaskbarCreated;		
	};

	DEF_EVT_EXT(EventTrayNotify,EVT_EXTERNAL_BEGIN+699,{
		UINT uMsg;
		WPARAM wp;
		LPARAM lp;
	});

	class SShellTray :public SWindow, public IShellTrayListener
	{
		DEF_SOBJECT(SWindow, L"shellnotifyicon")
	public:
		/**
		 * SShellNotifyIcon::SShellNotifyIcon
		 * @brief    构造函数
		 *
		 * Describe  构造函数
		 */
		SShellTray();

		/**
		 * SShellNotifyIcon::~SShellNotifyIcon
		 * @brief    析构函数
		 *
		 * Describe  析构函数
		 */
		~SShellTray();

	public:
		BOOL Show();
		BOOL Hide();
		void StartAni();
		void StopAni();
		bool IsRunAni() { return m_bRunAni; }
		void SetDefIconIdx(int iIdx);
	protected:
		void WINAPI OnFinalRelease() override;
		void WINAPI OnInitFinished(THIS_ IXmlNode * xmlNode) override;
	protected:
		void NextFrame() override;
		void OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
		void OnTaskbarRecreate() override;

		void IniNotifyIconData(HWND hOwner, HICON hIcon, UINT flags, LPCTSTR szTip);
	protected:
		HRESULT OnAttrIcons(SStringW strValue, BOOL bLoading);
		HRESULT OnAttrTip(SStringW szTip,BOOL bLoading=FALSE);

		SOUI_ATTRS_BEGIN()
			ATTR_CUSTOM(L"icons", OnAttrIcons)
			ATTR_CUSTOM(L"tip", OnAttrTip)
			ATTR_INT(L"deficoidx", m_iDefIcon, FALSE)
			ATTR_INT(L"startframe", m_iStartFrame, FALSE)
			ATTR_INT(L"duration", m_iDuration,FALSE)
		SOUI_ATTRS_END()
	private:
		SArray<HICON> m_ArrIcon;
		SStringT m_strTip;
		SStringT m_strMenu;
		NOTIFYICONDATA m_nid;
		SShellTrayMsgWnd *m_MsgOnlyWnd;
		//  默认图标索引			开始帧位置		动画间隔时间
		int m_iDefIcon, m_iCurFrame, m_iStartFrame, m_iDuration;
		bool m_bRunAni;
	};
}