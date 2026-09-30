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
*               - mclistview 页单项选中状态事件:实时显示已选条目数
*               - listctrl 页单项选中状态事件:实时显示已选条目数
*               - 各列表页"启用多选"复选框:运行时切换多选(关闭后可测试 fling)
*               - 各列表页"启用框选"复选框:运行时切换框选手势
*               - treectrl页"整行选中"复选框:运行时切换整行高亮(fullRowSel)
*               - SListBox 动态追加条目
*               - SGroupList 分组初始化、展开折叠、目录点击切页
*/

#include "stdafx.h"
#include "MainDlg.h"
#include "helper/SMenu.h"

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
* @brief      mclistview 页单项选中状态事件:实时显示已选条目数
*
* Describe    EventItemSelChanged 由 AddSelItem/RemoveSelItem 在条目选中状态
*             真实翻转时逐条发出(框选期间也实时触发),此处据此刷新页头文本。
*/
void CMainDlg::OnMclvItemSelChanged(IEvtArgs *e)
{
    SMCListView *pList = sobj_cast<SMCListView>(e->Sender());
    if (!pList)
        return;
    SWindow *pText = FindChildByName(L"txt_mclv_selcount");
    if (pText)
    {
        pText->SetWindowText(SStringT().Format(_T("已选 %d 项"), pList->GetSelItemCount()));
    }
}

/**
* @brief      listctrl 页单项选中状态事件:实时显示已选条目数
*
* Describe    与 OnMclvItemSelChanged 同理:SListCtrl 的点击/框选/键盘选择
*             翻转条目选中状态时逐条发出 EventItemSelChanged,据此刷新页头文本。
*/
void CMainDlg::OnLcItemSelChanged(IEvtArgs *e)
{
    SListCtrl *pList = sobj_cast<SListCtrl>(e->Sender());
    if (!pList)
        return;
    SWindow *pText = FindChildByName(L"txt_lc_selcount");
    if (pText)
    {
        pText->SetWindowText(SStringT().Format(_T("已选 %d 项"), pList->GetSelItemCount()));
    }
}

/**
* @brief      列表页"启用多选"复选框:运行时切换各列表控件的多选(框选)支持
*
* Describe    multiSel 开启时左键拖动为框选多选,关闭后拖动恢复为滚动/fling,
*             用于对比测试两种手势。按控件具体类型分流:
*             SListCtrl 走 EnableMultiSelection,其余走各自的 SetMultiSel。
*/
void CMainDlg::OnMultiSelToggle(IEvtArgs *e)
{
    SWindow *pChk = sobj_cast<SWindow>(e->Sender());
    if (!pChk)
        return;
    BOOL bOn = pChk->IsChecked();
    SStringW strChk = pChk->GetName();

    struct Binding
    {
        LPCWSTR pszChk;
        LPCWSTR pszList;
        int     nKind; // 0: SMCListView 1: SListCtrl 2: SListView 3: STileView 4: STreeView 5: STreeCtrl
    };
    static const Binding kBindings[] = {
        { L"chk_multi_mclv",   L"mclv_test",        0 },
        { L"chk_multi_lc",     L"lc_test",          1 },
        { L"chk_multi_lvfix",  L"lv_test_fix_horz", 2 },
        { L"chk_multi_lvfix",  L"lv_test_fix",      2 },
        { L"chk_multi_lvflex", L"lv_test_flex",     2 },
        { L"chk_multi_tile",   L"lv_test_tile",     3 },
        { L"chk_multi_tv",     L"room_tv",          4 },
        { L"chk_multi_tree",   L"mytree",           5 },
    };

    for (int i = 0; i < (int)(sizeof(kBindings) / sizeof(kBindings[0])); i++)
    {
        if (strChk != kBindings[i].pszChk)
            continue;
        SWindow *pList = FindChildByName(kBindings[i].pszList);
        if (!pList)
            continue;
        switch (kBindings[i].nKind)
        {
        case 0:
            if (SMCListView *pView = sobj_cast<SMCListView>(pList))
                pView->SetMultiSel(bOn);
            break;
        case 1:
            if (SListCtrl *pLc = sobj_cast<SListCtrl>(pList))
                pLc->EnableMultiSelection(bOn);
            break;
        case 2:
            if (SListView *pLv = sobj_cast<SListView>(pList))
                pLv->SetMultiSel(bOn);
            break;
        case 3:
            if (STileView *pTv = sobj_cast<STileView>(pList))
                pTv->SetMultiSel(bOn);
            break;
        case 4:
            if (STreeView *pTv = sobj_cast<STreeView>(pList))
                pTv->SetMultiSel(bOn);
            break;
        case 5:
            if (STreeCtrl *pTree = sobj_cast<STreeCtrl>(pList))
                pTree->EnableMultiSelection(bOn);
            break;
        }
        SLOGI() << (bOn ? _T("enable") : _T("disable")) << _T(" multiSel: ") << kBindings[i].pszList;
    }
}

/**
* @brief      列表页"启用框选"复选框:运行时切换框选手势(bandEnable)
*
* Describe    框选与多选解耦:框选开启时(且多选开启)左键拖动优先进入框选;
*             关闭后即使多选开启,拖动也走滚动/fling。手势开关在 SPanel 上,
*             各控件通用 EnableBandSel,无需按类型分流。
*/
void CMainDlg::OnBandToggle(IEvtArgs *e)
{
    SWindow *pChk = sobj_cast<SWindow>(e->Sender());
    if (!pChk)
        return;
    BOOL bOn = pChk->IsChecked();
    SStringW strChk = pChk->GetName();

    struct Binding
    {
        LPCWSTR pszChk;
        LPCWSTR pszList;
    };
    static const Binding kBindings[] = {
        { L"chk_band_mclv",   L"mclv_test" },
        { L"chk_band_lc",     L"lc_test" },
        { L"chk_band_lvfix",  L"lv_test_fix_horz" },
        { L"chk_band_lvfix",  L"lv_test_fix" },
        { L"chk_band_lvflex", L"lv_test_flex" },
        { L"chk_band_tile",   L"lv_test_tile" },
        { L"chk_band_tv",     L"room_tv" },
        { L"chk_band_tree",   L"mytree" },
    };

    for (int i = 0; i < (int)(sizeof(kBindings) / sizeof(kBindings[0])); i++)
    {
        if (strChk != kBindings[i].pszChk)
            continue;
        SWindow *pList = FindChildByName(kBindings[i].pszList);
        SPanel *pPanel = pList ? sobj_cast<SPanel>(pList) : NULL;
        if (pPanel)
        {
            pPanel->EnableBandSel(bOn);
            SLOGI() << (bOn ? _T("enable") : _T("disable")) << _T(" bandSel: ") << kBindings[i].pszList;
        }
    }
}

/**
* @brief      treectrl页"整行选中"复选框:运行时切换整行高亮
*
* Describe    开启时选中高亮覆盖整个条目行(含树线缩进区),点击行内任意位置
*             选中;关闭后只高亮文本部分,点击文字后方视为空白点击(清除选择)。
*             对应 STreeCtrl::EnableFullRowSel / XML 属性 fullRowSel。
*/
void CMainDlg::OnFullRowSelToggle(IEvtArgs *e)
{
    SWindow *pChk = sobj_cast<SWindow>(e->Sender());
    if (!pChk)
        return;
    SStringW strChk = pChk->GetName();
    if (strChk != L"chk_fullrow_tree")
        return;

    BOOL bOn = pChk->IsChecked();
    SWindow *pTree = FindChildByName(L"mytree");
    STreeCtrl *pTc = pTree ? sobj_cast<STreeCtrl>(pTree) : NULL;
    if (pTc)
    {
        pTc->EnableFullRowSel(bOn);
        SLOGI() << (bOn ? _T("enable") : _T("disable")) << _T(" fullRowSel: mytree");
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
