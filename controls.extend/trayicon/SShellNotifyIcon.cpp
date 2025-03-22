//////////////////////////////////////////////////////////////////////////
//  Class Name: SShellNotifyIcon
// Description:show ShellNotifyIcon
//     Creator: 359501300
//     Version: 2014.9.19 - 1.0 - Create
//    modifier: xiaoke 1035144170
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SShellNotifyIcon.h"

	SShellNotifyIcon::SShellNotifyIcon()
	{
		memset(&nid,0,sizeof(nid));
		nIconNum=0;
		m_bIsIcon=false;
	}
	
	SShellNotifyIcon::~SShellNotifyIcon()
	{
		Hide();
	}
	
	void SShellNotifyIcon::Create(HWND hOwner,HICON hIcon,UINT uFlags, UINT uCallbackMessage, UINT uId, LPCTSTR szTip)
	{
		IniNotifyIconData(hOwner,hIcon,uFlags, uCallbackMessage, uId, szTip);
	}
	

	BOOL SShellNotifyIcon::Show()
	{
		//未初使化NotifyIconData
		if (nid.cbSize != sizeof(NOTIFYICONDATA))
			return FALSE;
		return Shell_NotifyIcon(NIM_ADD, &nid);
	}
	
	BOOL SShellNotifyIcon::Hide()
	{
		return Shell_NotifyIcon(NIM_DELETE, &nid);
	}

	void SShellNotifyIcon::Modify()
	{
		//这样写破坏了原始类接口的统一（可优化）
		if (nIconNum<MOVE_ICON)
		{
			nid.hIcon=m_hIconArray[nIconNum];
			int m = Shell_NotifyIcon(NIM_MODIFY,&nid);
			nIconNum++;

		}else
		{
			nIconNum=1;
		}
		//return Shell_NotifyIcon(NIM_MODIFY, &nid);

	}

	void SShellNotifyIcon::Shinrk()
	{
		if(m_bIsIcon)
		{
			nid.hIcon = NULL/*hicon*/;
			int m = Shell_NotifyIcon(NIM_MODIFY,&nid);
			m_bIsIcon = false;
		}
		else
		{
			nid.hIcon = m_hIconArray[20];
			int m = Shell_NotifyIcon(NIM_MODIFY,&nid);
			m_bIsIcon = true;
		}
	}

	BOOL SShellNotifyIcon::LoadAnimateIcons(HICON m_hIcon,int num)
	{
		//加载20托盘图标（需优化时不应该这么写）
		if (num<MOVE_ICON)
		{
			m_hIconArray[num] =m_hIcon;
		}
		return TRUE;
	}
	
	void SShellNotifyIcon::IniNotifyIconData(HWND hOwner,HICON hIcon,UINT flags, UINT callbackmsg, UINT ID, LPCTSTR szTip)
	{
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = hOwner;
		nid.uID = ID;
		nid.uFlags = flags;//NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage = callbackmsg;
		nid.hIcon = hIcon;
		m_hIconArray[20]=hIcon;//保存留闪烁用
		_tcscpy(nid.szTip, szTip);
	}
