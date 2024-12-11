//////////////////////////////////////////////////////////////////////////
//  Class Name: SShellNotifyIcon
// Description:show ShellNotifyIcon
//     Creator: 359501300
//     Version: 2017.9.11 - 2.0 - Create
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SShellTray.h"

namespace SOUI
{
	SShellTrayMsgWnd::SShellTrayMsgWnd(IShellTrayListener* shellnotifyicon) :m_ShellNotifyIcon(shellnotifyicon)
	{
		MsgTaskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));
		CreateNative(_T("shell_nofity_msg_windows"), 0, 0, 0, 0, 0, 0, HWND_MESSAGE,0, NULL);
	}

	//托盘通知消息处理函数

	LRESULT SShellTrayMsgWnd::OnIconNotify(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		m_ShellNotifyIcon->OnNotify(uMsg,wParam,lParam);
		return 0;
	}

	LRESULT SShellTrayMsgWnd::OnTaskbarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		m_ShellNotifyIcon->OnTaskbarRecreate();
		return 0;
	}

	void SShellTrayMsgWnd::OnFinalMessage(HWND hWnd)
	{
		SNativeWnd::OnFinalMessage(hWnd);
		delete this;
	}

	void SShellTrayMsgWnd::OnTimer(UINT_PTR nIDEvent)
	{
		if(nIDEvent == ANI_TIMER_ID)
			m_ShellNotifyIcon->NextFrame();
	}

	//////////////////////////////////////////////////////////////////////////
	SShellTray::SShellTray() : m_MsgOnlyWnd(NULL), m_iDefIcon(0), m_iStartFrame(1), m_bRunAni(false), m_iDuration(200)
	{
		memset(&m_nid, 0, sizeof(m_nid));
		m_bFloat = TRUE;
		GetEventSet()->addEvent(EVENTID(EventTrayNotify));
	}

	SShellTray::~SShellTray() {}

	void SShellTray::IniNotifyIconData(HWND hOwner, HICON hIcon, UINT flags, LPCTSTR szTip)
	{	
		m_nid.cbSize = sizeof(NOTIFYICONDATA);
		m_nid.hWnd = hOwner;
		m_nid.uID = ID_TASKBARICON;
		m_nid.uCallbackMessage = WM_ICONNOTIFY;
		m_nid.uFlags = flags|NIF_MESSAGE;
		m_nid.hIcon = hIcon;
		_tcscpy_s(m_nid.szTip, szTip);
	}

	void SShellTray::StartAni()
	{
		SASSERT(m_MsgOnlyWnd);

		if (m_MsgOnlyWnd)
		{
			if (m_ArrIcon.GetCount() - (m_iStartFrame + 1) > 2)//至少要两帧吧。。。
			{
				m_nid.uFlags |= NIF_ICON;
				m_nid.uFlags &= ~NIF_INFO;
				m_iCurFrame = m_iStartFrame;
				m_MsgOnlyWnd->SetTimer(ANI_TIMER_ID, m_iDuration);
				m_bRunAni = true;
			}
		}
	}

	void SShellTray::StopAni()
	{
		SASSERT(m_MsgOnlyWnd);
		m_iCurFrame = 0;
		if (m_MsgOnlyWnd&&m_bRunAni)
		{
			m_bRunAni = false;
			m_MsgOnlyWnd->KillTimer(ANI_TIMER_ID);
		}
		m_nid.hIcon = m_ArrIcon[m_iDefIcon];
		Shell_NotifyIcon(NIM_MODIFY, &m_nid);
	}

	void SShellTray::SetDefIconIdx(int iIdx)
	{
		if (iIdx >= m_ArrIcon.GetCount())
			return;
		m_iDefIcon = iIdx;
		if (!m_bRunAni)
		{
			m_nid.uFlags |= NIF_ICON;
			m_nid.uFlags &= ~NIF_INFO;
			m_nid.hIcon = m_ArrIcon[m_iDefIcon];
			Shell_NotifyIcon(NIM_MODIFY, &m_nid);
		}
	}

	void SShellTray::NextFrame()
	{
		if (m_iCurFrame >= m_ArrIcon.GetCount())
			m_iCurFrame = m_iStartFrame;
		m_nid.hIcon = m_ArrIcon[m_iCurFrame++];
		Shell_NotifyIcon(NIM_MODIFY, &m_nid);
	}
	
	HRESULT SShellTray::OnAttrIcons(SStringW strValue, BOOL bLoading)
	{
		for (size_t i = 0; i < m_ArrIcon.GetCount(); i++)
		{
			DestroyIcon(m_ArrIcon[i]);
		}
		m_ArrIcon.RemoveAll();
		SStringWList icoList;
		size_t icoCount = SplitString(strValue, L';', icoList);
		SASSERT(icoCount > 0);
		for (size_t i = 0; i < icoCount; i++)
		{
			HICON ico = LOADICON2(icoList[i]);
			if (ico)
				m_ArrIcon.Add(ico);
		}
		return m_ArrIcon.GetCount() == 0 ? S_FALSE : S_OK;
	}

	BOOL SShellTray::Show()
	{
		//未初使化NotifyIconData
		if (m_nid.cbSize != sizeof(NOTIFYICONDATA))
			return FALSE;
		return Shell_NotifyIcon(NIM_ADD, &m_nid);
	}

	BOOL SShellTray::Hide()
	{
		return Shell_NotifyIcon(NIM_DELETE, &m_nid);
	}

	HRESULT SShellTray::OnAttrTip(SStringW szTip, BOOL bLoading)
	{	
		m_strTip = S_CW2T(GETSTRING(szTip));
		if (!bLoading)
		{
			m_nid.uFlags |= NIF_TIP;
			m_nid.uFlags &= ~NIF_INFO;
			_tcscpy_s(m_nid.szTip, m_strTip);
			Shell_NotifyIcon(NIM_MODIFY, &m_nid);
		}
		return S_FALSE;
	}

	void SShellTray::OnInitFinished(IXmlNode * xmlNode)
	{
		m_MsgOnlyWnd = new SShellTrayMsgWnd(this);
		SASSERT(IsWindow(m_MsgOnlyWnd->m_hWnd));
		SASSERT(m_iDefIcon < m_ArrIcon.GetCount());
		if (m_iDefIcon >= m_ArrIcon.GetCount())
			m_iDefIcon = 0;
		IniNotifyIconData(m_MsgOnlyWnd->m_hWnd, m_ArrIcon.GetCount() == 0 ? GETRESPROVIDER->LoadIcon(_T("ICON_LOGO"), 16) : m_ArrIcon[m_iDefIcon],NIF_ICON|NIF_TIP,m_strTip);
		Show();
	}

	void SShellTray::OnFinalRelease()
	{
		if (m_MsgOnlyWnd)
		{
			Hide();
			for (size_t i = 0; i < m_ArrIcon.GetCount(); i++)
			{
				DestroyIcon(m_ArrIcon[i]);
			}
			m_ArrIcon.RemoveAll();
			m_MsgOnlyWnd->DestroyWindow();
		}
		__super::OnFinalRelease();
	}


	void SShellTray::OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		EventTrayNotify evt(this);
		evt.uMsg = uMsg;
		evt.wp=wParam;
		evt.lp=lParam;
		FireEvent(evt);
	}

	void SShellTray::OnTaskbarRecreate()
	{
		Show();
	}


}