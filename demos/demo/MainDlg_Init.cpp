/**
* Copyright (C) 2014-2050
* All rights reserved.
*
* @file       MainDlg_Init.cpp
* @brief      CMainDlg 主窗口初始化
* @version    v2.0
* @author     SOUI group
* @date       2014/08/15
*
* Describe    实现 OnInitDialog 及各演示控件的初始化流程,按功能拆分为多个
*             私有 Init 子函数,统一由 OnInitDialog 调用:
*               - InitPageNav      宫格首页导航(MainDlg_Nav.cpp)
*               - InitListCtrl     列表控件演示数据
*               - InitDragDrop     OLE 拖放演示
*               - InitRichEditHost 富文本宿主与 EN_CHANGE 事件
*               - InitListViews    各类列表/树视图适配器绑定
*               - InitMiscCtrls    其余零散控件(区域窗口/路径视图/十六进制编辑器/托盘)
*               - InitSoui3Animation SOUI 3.0 属性动画(MainDlg_Animation.cpp)
*/

#include "stdafx.h"
#include "MainDlg.h"
#include <controls.extend/SHexEdit.h>
#include <controls.extend/SMcListViewEx/SMCListViewEx.h> //adapter.h 依赖 SMcAdapterBaseEx
#include <controls.extend/reole/RichEditOle.h>           //CSmileySource/SetSRicheditOleCallback
#include "SMatrixWindow.h"
#include "skin/SSkinLoader.h"
#include "adapter.h"
#include "trayicon/SShellTray.h"
#include "CAdapter.h"
#include "CDropTarget.h"
#include "skin/SetSkinWnd2.h"

#ifdef _WIN32
//由 MainDlg_RichEdit.cpp 提供:创建 GIF 表情数据源的回调
ISmileySource * CreateSource2();
#endif

#define kLogTag "initdlg"

/**
* @brief      主窗口初始化(相当于传统 MFC 的 OnInitDialog)
* @param      hWnd  宿主窗口句柄
* @return     0 - 继续正常流程
*
* Describe    演示在宿主窗口创建完成后如何查找子控件、绑定适配器与事件。
*             查找子控件的三种常用方式:
*               - FindChildByName2<T>(L"name")   按名字查找并直接返回指定类型
*               - FindChildByID(R.id.xxx)        按整型 ID 查找(名字由 resource.h 生成)
*               - FindChildByID2<T>(R.id.xxx)    按 ID 查找并转换类型
*/
LRESULT CMainDlg::OnInitDialog( HWND hWnd, LPARAM lParam )
{
	SLOGI()<<"OnInitDialog";

#ifdef _WIN32
    //Smiley 表情 COM 组件依赖 Gdiplus,使用前必须初始化
    CSmileySource::GdiplusStartup();
#endif//_WIN32

	m_bLayoutInited=TRUE;

	//左侧控件目录默认选中第一项(多列列表)
	FindChildByID2<SGroupList>(R.id.gl_catalog)->SelectPage(R.id.page_mclistview);

	//radio button 页:演示 STabCtrlHeaderBinder 把一组按钮/单选框绑定到 tab 页头
	STabCtrl *pTabCtrl = FindChildByName2<STabCtrl>(L"tab_radio2");
	{
		m_pTabBinder = new STabCtrlHeaderBinder(pTabCtrl);
		m_pTabBinder->Bind(FindChildByName(L"radio3_1"), 0);
		m_pTabBinder->Bind(FindChildByName(L"radio3_2"), 1);
		m_pTabBinder->Bind(FindChildByName(L"radio3_3"), 2);
		m_pTabBinder->Bind(FindChildByName(L"radio3_4"), 3);
		m_pTabBinder->Bind(FindChildByName(L"radio3_5"), 4);
		m_pTabBinder->Bind(FindChildByName(L"radio3_6"), 5);
		//同一个 tab 可以绑定多组页头控件
		m_pTabBinder2 = new STabCtrlHeaderBinder(pTabCtrl);
		m_pTabBinder->Bind(FindChildByName(L"btn4_1"), 0);
		m_pTabBinder->Bind(FindChildByName(L"btn4_2"), 1);
		m_pTabBinder->Bind(FindChildByName(L"btn4_3"), 2);
		m_pTabBinder->Bind(FindChildByName(L"btn4_4"), 3);
		m_pTabBinder->Bind(FindChildByName(L"btn4_5"), 4);
		m_pTabBinder->Bind(FindChildByName(L"btn4_6"), 5);
	}

	//加载上一次退出时保存的自定义皮肤
	LoadSkin();

	//设置本窗口为磁吸主窗口,其它吸附窗口将吸附到它的边缘
    SetMainWnd(m_hWnd);

	//宫格首页导航初始化(缓存导航栏/页签控件指针)
	InitPageNav();

    //基础控件页:列表控件演示数据
    InitListCtrl();

    //使用 GETSTRING 宏从 string.xml 中取出标题模板并格式化版本号
    SStringW strTitle = SStringW().Format(GETSTRING(R.string.title),SOUI_VER1,SOUI_VER2,SOUI_VER3,SOUI_VER4);
    FindChildByID(R.id.txt_title)->SetWindowText(S_CW2T(GetRoot()->tr(strTitle)));

	//OLE 拖放演示
	InitDragDrop();

	//富文本宿主与 URL 输入框事件
	InitRichEditHost();

	//各类列表/树视图适配器绑定
	InitListViews();

	//其余零散控件初始化
	InitMiscCtrls();

	//SOUI 3.0 属性动画演示(logo 旋转+飘心)
	InitSoui3Animation();

    return 0;
}

/**
* @brief      列表控件(SListCtrl)与扩展多列列表演示数据初始化
*
* Describe    - SListCtrl:传统报告风格列表,ItemData 中挂 student 结构,
*               表头点击排序事件通过 subscribeEvent 动态订阅(见 OnListHeaderClick);
*             - SMCListViewEx:音乐列表示例,由 SMusicListAdapter 提供数据。
*/
void CMainDlg::InitListCtrl()
{
    //找到列表控件
    SListCtrl *pList=FindChildByName2<SListCtrl>(L"lc_test");
    if(pList)
    {
        //列表控件的唯一子控件即为表头控件
        SWindow *pHeader=pList->GetWindow(GSW_FIRSTCHILD);
        //向表头控件订阅表头点击事件，并把它和 OnListHeaderClick 函数相连。
        pHeader->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK,Subscriber(&CMainDlg::OnListHeaderClick,this));

        //插入 100 行演示数据
        TCHAR szSex[][8]={_T("男"),_T("女"),_T("人妖")};
        for(int i=0;i<100;i++)
        {
            student *pst=new student;
            _stprintf(pst->szName,_T("学生_%d"),i+1);
            _tcscpy(pst->szSex,szSex[rand()%3]);
            pst->age=rand()%30;
            pst->score=rand()%60+40;

            int iItem=pList->InsertItem(i,pst->szName);
            pList->SetItemData(iItem,(LPARAM)pst);
            pList->SetSubItemText(iItem,1,pst->szSex);
            TCHAR szBuf[10];
            _stprintf(szBuf,_T("%d"),pst->age);
            pList->SetSubItemText(iItem,2,szBuf);
            _stprintf(szBuf,_T("%d"),pst->score);
            pList->SetSubItemText(iItem,3,szBuf);
        }
    }

	//扩展多列列表(SMCListViewEx),使用适配器提供音乐数据
	SMCListViewEx *musiclist = FindChildByName2<SMCListViewEx>(_T("musiclist"));
	if (musiclist)
	{
		SMusicListAdapter* musicadapter = new SMusicListAdapter(1, m_hWnd);
		musiclist->SetAdapter(musicadapter);
		//SetAdapter 内部已 AddRef,这里释放一次初始引用,由列表托管生命周期
		musicadapter->Release();
	}
}

/**
* @brief      OLE 拖放演示初始化
*
* Describe    演示 SOUI 与系统 OLE 拖放的结合:
*               - 对宿主窗口注册 IDropTarget(见 CDropTarget.h 中 GetDropTarget);
*               - 对任意 SOUI 子控件(此处为两个 edit)单独注册拖放目标,
*                 演示控件级别的文件拖放接收。
*/
void CMainDlg::InitDragDrop()
{
    //演示如何在SOUI中的拖放:宿主窗口整体接收拖放
    HRESULT hr=::RegisterDragDrop(m_hWnd,GetDropTarget());

    //两个 edit 分别注册独立的拖放目标,把拖入的文件名显示到输入框里
	{
		SEdit *pEdit1 = FindChildByName2<SEdit>(L"edit_drop_top1");
		if(pEdit1)
		{
			RegisterDragDrop(pEdit1->GetSwnd(),new CTestDropTarget1(pEdit1));
		}
	}
	{
		SEdit *pEdit1 = FindChildByName2<SEdit>(L"edit_drop_top2");
		if(pEdit1)
		{
			RegisterDragDrop(pEdit1->GetSwnd(),new CTestDropTarget1(pEdit1));
		}
	}
}

/**
* @brief      富文本(SChatEdit)宿主与 URL 输入框事件初始化
*
* Describe    - re_gifhost:聊天风格的富文本,演示 RTF 资源加载与 OLE 表情回调;
*             - edit_url:通过 EM_SETEVENTMASK 打开 EN_CHANGE 事件掩码,
*               配合 EVT_RE_NOTIFY 事件(见 OnUrlReNotify)。
*/
void CMainDlg::InitRichEditHost()
{
    //演示 RichEdit 的 OLE 回调:向富文本中插入 GIF 表情需要 SetSRicheditOleCallback
    SRichEdit *pEdit = FindChildByName2<SRichEdit>(L"re_gifhost");
    if(pEdit)
    {
#ifdef _WIN32
        SetSRicheditOleCallback(pEdit,CreateSource2);
#endif
        //演示从资源中加载 RTF 内容(资源注册于 uires.idx 的 <rtf> 节点)
        pEdit->SetAttribute(L"rtf",L"rtf:rtf_test");
    }

    //演示如何响应Edit的EN_CHANGE事件:先打开 ENM_CHANGE 掩码
    SEdit *pEditUrl = FindChildByName2<SEdit>(L"edit_url");
    if(pEditUrl)
    {
        pEditUrl->SSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
    }
}

/**
* @brief      各类列表/树视图适配器绑定
*
* Describe    SOUI 的高性能虚拟列表均通过 Adapter 提供数据:
*               - SListView(固定行高 lv_test_fix / 水平列表 lv_test_fix_horz)
*               - SListView(可变行高 lv_test_flex)
*               - SMCListView(多列虚拟列表 mclv_test)
*               - STileView(宫格列表 lv_test_tile)
*               - STreeView(树形列表 room_tv)
*             SetAdapter 会增加适配器引用计数,初始化后释放一次由控件托管。
*/
void CMainDlg::InitListViews()
{
    //行高固定的列表
    SListView *pLstViewFix = FindChildByName2<SListView>("lv_test_fix");
    if(pLstViewFix)
    {
        ILvAdapter *pAdapter = new CTestAdapterFix;
        pLstViewFix->SetAdapter(pAdapter);
        pAdapter->Release();
    }

	//水平方向的固定行高列表
	SListView *pLstViewFixHorz = FindChildByName2<SListView>("lv_test_fix_horz");
	if(pLstViewFixHorz)
	{
		ILvAdapter *pAdapter = new CTestAdapterFixHorz;
		pLstViewFixHorz->SetAdapter(pAdapter);
		pAdapter->Release();
	}

    //行高可变的列表
    SListView *pLstViewFlex = FindChildByName2<SListView>("lv_test_flex");
    if(pLstViewFlex)
    {
        ILvAdapter *pAdapter = new CTestAdapterFlex;
        pLstViewFlex->SetAdapter(pAdapter);
        pAdapter->Release();
    }

    //多列listview
    SMCListView * pMcListView = FindChildByName2<SMCListView>("mclv_test");
    if(pMcListView)
    {
        IMcAdapter *pAdapter = new CTestMcAdapterFix;
        pMcListView->SetAdapter(pAdapter);
        pAdapter->Release();
    }

    //tileView
    STileView *pTileView = FindChildByName2<STileView>("lv_test_tile");
    if(pTileView)
    {
        CTestTileAdapter *pAdapter = new CTestTileAdapter;
        pTileView->SetAdapter(pAdapter);
        pAdapter->Release();
    }

	//treeview
	STreeView * pTreeView = FindChildByName2<STreeView>("room_tv");
	if (pTreeView)
	{
		CTreeViewAdapter * pTreeViewAdapter = new CTreeViewAdapter;
		pTreeView->SetAdapter(pTreeViewAdapter);
		pTreeViewAdapter->Release();
	}
}

/**
* @brief      其余零散演示控件初始化
*
* Describe    - 托盘图标动画启动;
*             - SPathView 随机路径点(演示自定义 Swnd 绘制与 EventPath);
*             - SetWindowRgn 演示(圆形区域窗口,隐藏控件需先逐层请求布局);
*             - SHexEdit 十六进制编辑器填充 0~127 演示数据。
*/
void CMainDlg::InitMiscCtrls()
{
	//托盘图标:启动帧动画(在线/离线/loading 图标轮播)
	FindChildByID2<SShellTray>(R.id.tray_008)->StartAni();

	//路径视图:随机生成 10 个路径点,EventPath 事件回传路径总长度
	SPathView *pPathView = FindChildByName2<SPathView>("pv_test");
	if(pPathView)
	{
		POINT pts[10];
		for(int i=0;i<ARRAYSIZE(pts);i++)
		{
			pts[i].x = rand()%500;
			pts[i].y = rand()%300;
		}
		pPathView->AddPoint(pts,ARRAYSIZE(pts));
	}

    //演示SetWindowRgn用法:把一个窗口裁剪为圆形
    SWindow *pWndRgn = FindChildByName(L"wnd_rgn");
    if(pWndRgn)
    {
		//性能优化后，隐藏窗口不能直接获取位置，这里先沿父链逐层主动请求布局。
		SList<SWindow*> pps;
		SWindow *p = pWndRgn->GetParent();
		while (!p->IsVisible(TRUE))
		{
			pps.AddHead(p);
			p = p->GetParent();
		}
		SPOSITION pos = pps.GetHeadPosition();
		while (pos)
		{
			SWindow *p = pps.GetNext(pos);
			p->UpdateChildrenPosition();
		}

		//以窗口矩形创建椭圆区域,SWindow 将窗口左上角定义为 Rgn 的原点
        SAutoRefPtr<IRegionS> pRgn;
        GETRENDERFACTORY->CreateRegion(&pRgn);
		CRect rc=pWndRgn->GetWindowRect();
		rc.MoveToXY(0,0);
		pRgn->CombineEllipse(&rc,RGN_COPY);
        pWndRgn->SetWindowRgn(pRgn,TRUE);
    }

	//十六进制编辑器:填充 0~127 的演示数据
	BYTE hexData[128] = {0};
	for (int i=0; i<sizeof(hexData); ++i)
	{
		hexData[i] = i;
	}
	SHexEdit* hexEdit = FindChildByName2<SHexEdit>("ctrl_hexedit");
	if(hexEdit)
	{
		hexEdit->SetData(hexData, sizeof(hexData));
	}
}
