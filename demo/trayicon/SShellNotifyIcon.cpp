//////////////////////////////////////////////////////////////////////////
//  Class Name: SShellNotifyIcon
// Description:show ShellNotifyIcon
//     Creator: 359501300
//     Version: 2017.9.11 - 2.0 - Create
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SShellNotifyIcon.h"
#include "helper\smenuex.h"
#include "helper\SMenu.h"
#pragma warning(disable:4482) //warning C4482: 使用了非标准扩展: 限定名中使用了枚举“SOUI::MenuType”

namespace SOUI
{
	CShellNotifyHwnd2::CShellNotifyHwnd2(SShellNotifyIcon* shellnotifyicon) :m_ShellNotifyIcon(shellnotifyicon)
	{
		MsgTaskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));
		CreateWindow(_T("shell_nofity_msg_windows"), 0, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL);
	}

	//托盘通知消息处理函数

	LRESULT CShellNotifyHwnd2::OnIconNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL)
	{
		LRESULT bRet = S_FALSE;
		switch (lParam)
		{
		case  WM_RBUTTONDOWN:
			{
				m_ShellNotifyIcon->ShowMenu();
				bRet = S_OK;
			}break;
		case WM_LBUTTONDOWN:
			{
				//if (m_pMainWnd->IsWindowVisible())
				//	m_pMainWnd->ShowWindow(SW_HIDE);
				//else
				//{
				//	m_pMainWnd->ShowWindow(SW_SHOW);
				//	if(m_pMainWnd->IsIconic())
				//	{
				//		m_pMainWnd->SendMessage(WM_SYSCOMMAND,SC_RESTORE);
				//	}

				//	SetForegroundWindow(m_pMainWnd->m_hWnd);
				//}
				bRet = S_OK;
			}break;
		}
		return bRet;
	}

	LRESULT CShellNotifyHwnd2::OnTaskbarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL)
	{
		return m_ShellNotifyIcon->Show();
	}

	void CShellNotifyHwnd2::OnFinalMessage(HWND hWnd)
	{
		__super::OnFinalMessage(hWnd);
		delete this;
	}

	void CShellNotifyHwnd2::OnTimer(UINT_PTR nIDEvent)
	{
		switch (nIDEvent)
		{
		case ANI_TIMER_ID:
			{
				m_ShellNotifyIcon->NextFrame();
			}break;
		default:
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	SShellNotifyIcon::SShellNotifyIcon() :m_menuType(unknow), m_MsgOnlyWnd(NULL), m_iDefIcon(0), m_iStartFrame(1), m_bRunAni(false), m_iDuration(200)
	{
		memset(&m_nid, 0, sizeof(m_nid));
		m_bFloat = TRUE;
	}

	SShellNotifyIcon::~SShellNotifyIcon() {}

	void SShellNotifyIcon::IniNotifyIconData(HWND hOwner, HICON hIcon, UINT flags, LPCTSTR szTip)
	{	
		m_nid.cbSize = sizeof(NOTIFYICONDATA);
		m_nid.hWnd = hOwner;
		m_nid.uID = ID_TASKBARICON;
		m_nid.uCallbackMessage = WM_ICONNOTIFY;
		m_nid.uFlags = flags;
		m_nid.hIcon = hIcon;
		_tcscpy_s(m_nid.szTip, szTip);
	}

	void SShellNotifyIcon::ShowMenu()
	{
		switch (m_menuType)
		{
		case MenuType::menu:
		{
			SMenu tmenu;
			if (tmenu.LoadMenu(m_strMenu))
			{
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(m_MsgOnlyWnd->m_hWnd);
				tmenu.TrackPopupMenu(0, pt.x, pt.y, m_MsgOnlyWnd->m_hWnd);
			}
		}
		break;
		case MenuType::menuex:
		{
			SMenuEx tmenuex;
			if (tmenuex.LoadMenu(m_strMenu))
			{
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(m_MsgOnlyWnd->m_hWnd);
				tmenuex.TrackPopupMenu(0, pt.x, pt.y, m_MsgOnlyWnd->m_hWnd);
			}
		}
		break;
		}
	}

	void SShellNotifyIcon::StartAni()
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

	void SShellNotifyIcon::StopAni()
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

	void SShellNotifyIcon::SetDefIconIdx(int iIdx)
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

	void SShellNotifyIcon::NextFrame()
	{
		if (m_iCurFrame >= m_ArrIcon.GetCount())
			m_iCurFrame = m_iStartFrame;
		m_nid.hIcon = m_ArrIcon[m_iCurFrame++];
		Shell_NotifyIcon(NIM_MODIFY, &m_nid);
	}

	HRESULT SShellNotifyIcon::OnAttrMenu(SStringW strValue, BOOL bLoading)
	{
		SXmlDoc xmlDoc;
		if (SApplication::getSingleton().LoadXmlDocment(xmlDoc, S_CW2T(strValue)))
		{
			if (wcscmp(xmlDoc.root().first_child().name(), L"menu") == 0)
				m_menuType = MenuType::menu;
			else if ((wcscmp(xmlDoc.root().first_child().name(),L"menuRoot") == 0) || (wcscmp(xmlDoc.root().first_child().name(), L"menuItem") == 0))
				m_menuType = MenuType::menuex;
			else m_menuType = MenuType::unknow;

			if (MenuType::unknow != m_menuType)
				m_strMenu = S_CW2T(strValue);
			return S_OK;
		}
		else return S_FALSE;
	}
	
	HRESULT SShellNotifyIcon::OnAttrIcons(SStringW strValue, BOOL bLoading)
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

	BOOL SShellNotifyIcon::Show()
	{
		//未初使化NotifyIconData
		if (m_nid.cbSize != sizeof(NOTIFYICONDATA))
			return FALSE;
		return Shell_NotifyIcon(NIM_ADD, &m_nid);
	}

	BOOL SShellNotifyIcon::Hide()
	{
		return Shell_NotifyIcon(NIM_DELETE, &m_nid);
	}

	HRESULT SShellNotifyIcon::OnAttrTip(SStringW szTip, BOOL bLoading)
	{	
		if (!szTip.IsEmpty())
		{
			m_strTip = S_CW2T(GETSTRING(szTip));
			if (!bLoading)
			{
				m_nid.uFlags |= NIF_TIP;
				m_nid.uFlags &= ~NIF_INFO;
				_tcscpy_s(m_nid.szTip, m_strTip);
				Shell_NotifyIcon(NIM_MODIFY, &m_nid);
			}
			return S_OK;
		}
		return S_FALSE;
	}

	BOOL SShellNotifyIcon::CreateChildren(SXmlNode xmlNode)
	{
		m_MsgOnlyWnd = new CShellNotifyHwnd2(this);
		SASSERT(IsWindow(m_MsgOnlyWnd->m_hWnd));
		SASSERT(m_iDefIcon < m_ArrIcon.GetCount());
		if (m_iDefIcon >= m_ArrIcon.GetCount())
			m_iDefIcon = 0;
		IniNotifyIconData(m_MsgOnlyWnd->m_hWnd, m_ArrIcon.GetCount() == 0 ? GETRESPROVIDER->LoadIcon(_T("ICON_LOGO"), 16) : m_ArrIcon[m_iDefIcon],m_strTip);
		return Show();
	}

	void SShellNotifyIcon::OnFinalRelease()
	{
		if (m_MsgOnlyWnd)
		{
			Hide();
			for (size_t i = 0; i < m_ArrIcon.GetCount(); i++)
			{
				DestroyIcon(m_ArrIcon[i]);
			}
			m_ArrIcon.RemoveAll();
			m_MsgOnlyWnd->PostMessage(WM_CLOSE);
		}
	}

	BOOL SShellNotifyIcon::ShowNotify(LPCTSTR szMsg, LPCTSTR szTitle /*= NULL*/)
	{
		if (szMsg)
		{
			_tcscpy_s(m_nid.szInfo, szMsg);
			m_nid.uFlags |= NIF_INFO;
			m_nid.uTimeout = 1000;
		}
		if (szTitle)
		{
			_tcscpy_s(m_nid.szInfoTitle, szTitle);
		}
		return Shell_NotifyIcon(NIM_MODIFY, &m_nid);
	}


}