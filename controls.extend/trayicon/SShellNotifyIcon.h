/**
 * Copyright (C) 2014-2050 359501300
 * All rights reserved.
 * 
 * @file       SShellNotifyIcon.h
 * @brief      
 * @version    v1.0      
 * @author     359501300      
 * @date       2014-09-19
 * 
 * Describe     
 */
#pragma once
#include <shellapi.h>
#define WM_ICONNOTIFY (WM_USER + 1101) 

#define ID_TASKBARICON	100

#define MOVE_ICON  20          //20个托盘图标

/**
 * @class      SShellNotifyIcon
 * @brief      ShellNotifyIcon
 * 
 * Describe    ShellNotifyIcon
 */
class SShellNotifyIcon
{
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
	virtual ~SShellNotifyIcon();
	
    void Create(HWND hOwner,HICON hIcon,UINT uFlags = (NIF_ICON | NIF_MESSAGE | NIF_TIP), UINT uCallbackMessage = WM_ICONNOTIFY, UINT uId = ID_TASKBARICON, LPCTSTR szTip=_T("Soui 托盘演示"));

    BOOL Show();

    BOOL Hide();

	void Modify();

	void Shinrk();

	BOOL LoadAnimateIcons(HICON m_hIcon, int num);
protected:
	void IniNotifyIconData(HWND hOwner,HICON hIcon,UINT flags, UINT callbackmsg, UINT ID, LPCTSTR szTip);

	NOTIFYICONDATA nid;

	int nIconNum;

	bool m_bIsIcon;

	HICON m_hIconArray[MOVE_ICON];//托盘区动画图标数组
};

