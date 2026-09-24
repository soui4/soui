/**
* Copyright (C) 2014-2050
* All rights reserved.
*
* @file       MainDlg_List.cpp
* @brief      CMainDlg 列表/树类控件事件处理
* @version    v2.0
* @author     SOUI group
* @date       2014/08/15
*
* Describe    "基础控件"页中各类列表控件的交互事件处理:
*               - SListCtrl 表头点击排序(subscribeEvent 订阅)
*               - SMCListView 右键菜单 / 项面板双击 / 表头内嵌复选框布局
*               - SListBox 动态追加条目
*               - SGroupList 分组初始化、展开折叠、目录点击切页
*/

#include "stdafx.h"
#include "MainDlg.h"
#include "helper/SMenu.h"
#include <controls.extend/SMcListViewEx/SMCListViewEx.h>
#include "adapter.h"

#define kLogTag "listdlg"

/**
* @brief      SListCtrl 表头点击排序比较函数
* @param      pCtx  排序列号(由 SortItems 的第二个参数传入)
* @param      p1/p2 参与比较的两个 LVITEM
*
* Describe   演示如何结合 ItemData 实现自定义排序:每个 item 的 dwData
*            中保存 student 指针,按指定列的字段比较大小。
*/
int funCmpare(void* pCtx,const void *p1,const void *p2)
{
    int iCol=*(int*)pCtx;

    const DXLVITEM *plv1=(const DXLVITEM*)p1;
    const DXLVITEM *plv2=(const DXLVITEM*)p2;

    const student *pst1=(const student *)plv1->dwData;
    const student *pst2=(const student *)plv2->dwData;

    switch(iCol)
    {
    case 0://name
        return _tcscmp(pst1->szName,pst2->szName);
    case 1://sex
        return _tcscmp(pst1->szSex,pst2->szSex);
    case 2://age
        return pst1->age-pst2->age;
    case 3://score
        return pst1->score-pst2->score;
    default:
        return 0;
    }
}

/**
* @brief      表头点击事件处理(通过 subscribeEvent 订阅,不走 EVENT_MAP)
* @return     TRUE - 阻断事件继续冒泡
*
* Describe    演示三种事件订阅方式中的"运行时订阅"方式:
*             pHeader->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK, Subscriber(...))。
*             事件对象用 sobj_cast 或直接强转获取附加数据(此处为点击的列号)。
*/
BOOL CMainDlg::OnListHeaderClick(IEvtArgs *pEvtBase)
{
    //事件对象强制转换
    EventHeaderClick *pEvt =(EventHeaderClick*)pEvtBase;
    SHeaderCtrl *pHeader=(SHeaderCtrl*)pEvt->Sender();
    //从表头控件获得列表控件对象
    SListCtrl *pList= (SListCtrl*)pHeader->GetParent();
    //列表数据排序
    SHDITEM hditem;
    hditem.mask=SHDI_ORDER;
    pHeader->GetItem(pEvt->iItem,&hditem);
    pList->SortItems(funCmpare,&hditem.iOrder);
    return true;
}

/**
* @brief      多列列表(mclv_test)右键菜单
*
* Describe    演示虚拟列表的 HitTest:把屏幕坐标转换为点击的列表项并选中,
*             再弹出自定义菜单(SMenu:menu_lv)。
*             菜单命令由 OnCommand 统一处理(nID==100 删除选中项)。
*/
void CMainDlg::OnMclvCtxMenu(IEvtArgs *pEvt)
{
    EventCtxMenu *pEvt2 = sobj_cast<EventCtxMenu>(pEvt);
    POINT pt = pEvt2->pt;

    {
        //选中鼠标点击行
        SMCListView *pListview = sobj_cast<SMCListView>(pEvt2->Sender());
        CPoint pt2 = pt;
        SItemPanel *pItem = pListview->HitTest(pt2);
        if(pItem)
        {
            int iItem = pItem->GetItemIndex();
            pListview->SetSel(iItem);
            SLOGI()<<_T("当前选中行:")<<iItem;
        }

    }
    SMenu menu;
	menu.LoadMenu(_T("smenu:menu_lv"));

    ClientToScreen(&pt);

    menu.TrackPopupMenu(0,pt.x,pt.y,m_hWnd);
}

/**
* @brief      列表项面板事件转发处理(EventOfPanel)
*
* Describe    虚拟列表的每一行都是一个 SItemPanel 面板,面板内部控件的事件
*             会以 EventOfPanel 的形式转发出来,pOrgEvt 中保存原始事件。
*             此处演示响应整行双击(EventItemPanelDbclick)。
*/
void CMainDlg::OnMclvEventOfPanel(IEvtArgs * pEvt)
{
	EventOfPanel *e2 = sobj_cast<EventOfPanel>(pEvt);
	SASSERT(e2);
	if (e2->pOrgEvt->GetID() == EventItemPanelDbclick::EventID)
	{
		EventItemPanelDbclick *e3 = sobj_cast<EventItemPanelDbclick>(e2->pOrgEvt);
		SItemPanel *pSender = sobj_cast<SItemPanel>(e3->Sender());
		SASSERT(pSender);
		int iItem = pSender->GetItemIndex();
		SMessageBox(m_hWnd, SStringT().Format(_T("double click item:%d"), iItem+1), _T("haha"), MB_OK | MB_ICONSTOP);
	}
}

/**
* @brief      表头布局完成事件:把"全选"复选框停靠到表头右侧
*
* Describe    演示 EventHeaderRelayout 的用法:表头列宽变化后,手动把内嵌的
*             chk_mclv_sel 复选框移动到第一列(表头)的右侧空白处。
*/
void CMainDlg::OnMcLvHeaderRelayout(IEvtArgs * e)
{
	SHeaderCtrl *pHeader = sobj_cast<SHeaderCtrl>(e->Sender());
	int nItems = pHeader->GetItemCount();
	if (nItems > 1)
	{
		//取第一列的矩形,把复选框垂直居中放到它右侧 5px 处
		CRect rc = pHeader->GetItemRect(pHeader->GetOriItemIndex(0));
		SWindow *pChk = pHeader->FindChildByName(L"chk_mclv_sel");
		SASSERT(pChk);
		CSize szChk ;
		pChk->GetDesiredSize(&szChk,-1,-1);
		CRect rc2(CPoint(rc.right - 5 - szChk.cx, rc.top + (rc.Height()-szChk.cy)/2), szChk);
		if (rc2.right >= rc.right - 5) rc2.right = rc.right - 5;
		pChk->Move(rc2);
	}
}

/**
* @brief      "init listbox"按钮:向列表框动态追加 20 个条目
*
* Describe    演示 SListBox 的 AddString/EnsureVisible/Update 组合用法。
*/
void CMainDlg::OnInitListBox()
{
	SListBox *pLb = FindChildByID2<SListBox>(R.id.lb_test);
	if(pLb)
	{
		int nCount = pLb->GetCount();
		for(int i=0; i< 20; i++)
		{
			int iItem = pLb->AddString(SStringT().Format(_T("new item：%d"),nCount+i));
			pLb->EnsureVisible(iItem);
			pLb->Update();
			Sleep(10);
		}
	}
}

/**
* @brief      grouplist 分组项初始化
*
* Describe    EventGroupListInitGroup 在每个分组创建时触发,用于填充分组
*             模板内的控件内容(标题文本、展开/折叠状态)。
*/
void CMainDlg::OnInitGroup(IEvtArgs *e)
{
	EventGroupListInitGroup *e2 = sobj_cast<EventGroupListInitGroup>(e);
	SToggle *pTgl = e2->pItem->FindChildByID2<SToggle>(R.id.tgl_switch);
	pTgl->SetToggle(!e2->pGroupInfo->bCollapsed);
	e2->pItem->FindChildByID(R.id.txt_label)->SetWindowText(e2->pGroupInfo->strText);
}

/**
* @brief      grouplist 列表项初始化
*
* Describe    EventGroupListInitItem 在每个列表项创建时触发,填充文本与图标,
*             图标索引来自 <data> 节点中 item 的 icon 属性(对应雪碧图分帧)。
*/
void CMainDlg::OnInitItem(IEvtArgs *e)
{
	EventGroupListInitItem *e2 = sobj_cast<EventGroupListInitItem>(e);
	e2->pItem->FindChildByID(R.id.txt_label)->SetWindowText(e2->pItemInfo->strText);
	e2->pItem->FindChildByID2<SImageWnd>(R.id.img_indicator)->SetIcon(e2->pItemInfo->iIcon);
}

/**
* @brief      grouplist 分组展开/折叠状态变化:同步分组项上的 toggle 箭头
*/
void CMainDlg::OnGroupStateChanged(IEvtArgs *e)
{
	EventGroupStateChanged *e2 = sobj_cast<EventGroupStateChanged>(e);
	SToggle *pTgl = e2->pItem->FindChildByID2<SToggle>(R.id.tgl_switch);
	pTgl->SetToggle(!e2->pGroupInfo->bCollapsed);

}

/**
* @brief      左侧控件目录点击:切换右侧 tab_ctrls 到对应演示页
*
* Describe    <data> 节点中各 item 的 id 从 R.id.page_mclistview 起连续分配,
*             页序 = item.id - R.id.page_mclistview,直接作为 SetCurSel 参数。
*/
void CMainDlg::OnCtrlPageClick(IEvtArgs *e)
{
	EventGroupListItemCheck *e2=sobj_cast<EventGroupListItemCheck>(e);
	STabCtrl *pTabOp = FindChildByID2<STabCtrl>(R.id.tab_ctrls);
	int nIndex = e2->pItemInfo->id - R.id.page_mclistview;
	pTabOp->SetCurSel(nIndex);

}
