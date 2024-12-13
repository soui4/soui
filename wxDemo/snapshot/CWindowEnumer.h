#pragma once

#include <vector>

class CWindowEnumer
{
	CWindowEnumer();
	~CWindowEnumer();
	static std::vector<CRect> rcList;
public:
	//获取所有项层窗口并获取矩形
	static void EnumAllTopWindow()
	{
		//获取屏幕上所有的顶层窗口,每发现一个窗口就调用回调函数一次
		HWND hWnd = GetDesktopWindow();        //得到桌面窗口
		hWnd = GetWindow(hWnd, GW_CHILD);        //得到屏幕上第一个子窗口
		rcList.clear();
		while (hWnd != NULL)                    //循环得到所有的子窗口
		{
			if (IsWindowVisible(hWnd))  //判断是否顶层窗口并且可见
			{
				CRect WindowRc;
				GetWindowRect(hWnd, &WindowRc);
				if (!WindowRc.IsRectEmpty())
					rcList.push_back(WindowRc);
			}
			hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
		}
	}
	static CRect GetPointInRect(POINT pt)
	{/*
		for (auto &rc:rcList)
		{
			if (rc.PtInRect(pt))
				return rc;
		}*/
		for (std::vector<CRect>::const_iterator ite = rcList.begin(); ite != rcList.end(); ite++)
		{
			if (ite->PtInRect(pt))
				return *ite;
		}

		return CRect();
	}
};

