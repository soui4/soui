/**
* Copyright (C) 2014-2050
* All rights reserved.
*
* @file       MainDlg.cpp
* @brief      CMainDlg 主窗口生命周期与命令主干
* @version    v2.0
* @author     SOUI group
* @date       2014/08/15
*
* Describe    CMainDlg 的实现按功能拆分到多个编译单元(职责见 MainDlg.h 文件头),
*             本文件只保留:
*               - OnCreate  :窗口创建(WM_CREATE),做窗口样式调整;
*               - OnDestory :窗口销毁(WM_DESTROY),集中释放各演示占用的资源;
*               - OnCommand :菜单/控件 WM_COMMAND 命令路由。
*             事件映射表(EVENT_MAP)与消息映射表(BEGIN_MSG_MAP_EX)定义在
*             MainDlg.h 中。
*/

#include "stdafx.h"
#include "MainDlg.h"
#include <controls.extend/reole/RichEditOle.h>
#include "CAdapter.h"

//#define SHOW_AERO //open aero for vista and win7

#ifdef SHOW_AERO
#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")
#endif

/**
* @brief      窗口创建响应(WM_CREATE)
* @param      lpCreateStruct  创建参数
* @return     0 - 继续正常创建流程
*
* Describe    去掉 overlap 风格窗口的默认边框圆角,由布局自行绘制外观;
*             SetMsgHandled(FALSE) 把消息继续交给 SHostWnd 默认处理,
*             这是宿主窗口链式处理消息的固定写法。
*/
int CMainDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
#ifdef SHOW_AERO
    MARGINS mar = {5,5,30,5};
    DwmExtendFrameIntoClientArea ( m_hWnd, &mar );//打开这里可以启用Aero效果
#endif
	ModifyStyle(WS_BORDER, 0);	//去掉overlap风格窗口的默认圆角。
	SetMsgHandled(FALSE);
	return 0;
}

/**
* @brief      窗口销毁响应(WM_DESTROY)
*
* Describe    按初始化的逆序清理各演示资源:
*               1. lc_test 列表项绑定的 student 数据块(InitListCtrl 中 new);
*               2. demoskinbk 换肤背景窗口的当前皮肤状态(持久化到配置文件);
*               3. radio 页的两个 STabCtrlHeaderBinder(普通指针,需手动 delete);
*               4. Smiley 表情 COM 依赖的 Gdiplus(与 OnInitDialog 的 Startup 配对)。
*/
void CMainDlg::OnDestory()
{
    SListCtrl *pList=FindChildByName2<SListCtrl>(L"lc_test");
    if(pList)
    {
        for(int i=0;i<pList->GetItemCount();i++)
        {
            student *pst=(student*) pList->GetItemData(i);
            delete pst;
        }
    }
	SDemoSkin *skin = (SDemoSkin *)GETSKIN(L"demoskinbk",100);
	if (skin)
	{
		skin->SaveSkin();
	}
    SetMsgHandled(FALSE);
	if (m_pTabBinder)
		delete m_pTabBinder;
	if (m_pTabBinder2)
		delete m_pTabBinder2;
#ifdef _WIN32
    CSmileySource::GdiplusShutdown();
#endif //_WIN32
}

/**
* @brief      WM_COMMAND 命令路由
* @param      uNotifyCode  通知码,0 表示菜单命令
* @param      nID          命令 ID
* @param      wndCtl       发送命令的控件句柄
*
* Describe    处理 SMenu:menu_test 弹出菜单与列表右键菜单(smenu:menu_lv)
*             中不带控件句柄的命令项:
*               nID == 6   :菜单的 exit 项,关闭主窗口;
*               nID == 5   :about 项,经 NavigateToPage 动画切到关于分区;
*               nID == 100 :mclv 右键菜单的删除项,移除多列列表当前选中行。
*             其余控件命令已由 MainDlg.h 中的 EVENT_MAP 处理,不经过这里。
*/
void CMainDlg::OnCommand( UINT uNotifyCode, int nID, HWND wndCtl )
{
    if(uNotifyCode==0)
    {
        if(nID==6)
        {//nID==6对应menu_test定义的菜单的exit项。
            PostMessage(WM_CLOSE);
        }else if(nID==5)
        {//about SOUI:经 NavigateToPage 走两级 tab 动画切换到关于分区
            NavigateToPage(8);
		}
		else if(nID==100)
        {//delete item in mclistview
            SMCListView *pListView = FindChildByName2<SMCListView>(L"mclv_test");
            if(pListView)
            {
                int iItem = pListView->GetSel();
                if(iItem!=-1)
                {
                    CTestMcAdapterFix *pAdapter = (CTestMcAdapterFix*)pListView->GetAdapter();
                    pAdapter->DeleteItem(iItem);
                }
            }
        }
    }
}
