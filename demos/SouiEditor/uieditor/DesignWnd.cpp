// ScintillaWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "DesignWnd.h"
#include <shellapi.h>
#pragma comment(lib,"Msimg32.lib")

//////////////////////////////////////////////////////////////////////////


CDesignWnd::CDesignWnd()
{
}

CDesignWnd::~CDesignWnd()
{
}


void CDesignWnd::RefreshDesignLayout()
{
	HWND hChild = ::GetWindow(m_hWnd,GW_CHILD);
	if ( hChild )
	{
		CRect rc;
		SNativeWnd::GetClientRect(&rc);
		::SetWindowPos(hChild,NULL,0,0,rc.Width(),rc.Height(),SWP_NOZORDER);
	}else
	{
		SNativeWnd::Invalidate();
	}
}

void CDesignWnd::OnSize(UINT nType, CSize size)
{
	RefreshDesignLayout();
	SetMsgHandled(FALSE);
}
