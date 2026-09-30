/**
* Copyright (C) 2014-2050
* All rights reserved.
*
* @file       MainDlg.h
* @brief      SOUI Demo 主窗口
* @version    v2.0
* @author     SOUI group
* @date       2014/08/15
*
* Describe    主窗口实现
*
*             CMainDlg 是整个 demo 的宿主窗口,布局由 uires\xml\dlg_main.xml 描述:
*               - 第 0 页为宫格首页(page_home.xml),以卡片方式导航到各演示分区;
*               - 第 1~7 页为演示分区(基础控件/教程/动画/布局/杂项/Skia/关于),
*                 通过 <include> 引入各自的 page_*.xml 布局;
*               - 页面切换由宫格卡片点击与导航栏返回按钮统一驱动(MainDlg_Nav.cpp)。
*
*             成员函数的实现按功能拆分到多个编译单元,便于维护:
*               MainDlg.cpp        消息映射与窗口生命周期主干
*               MainDlg_Init.cpp   OnInitDialog 及各控件初始化
*               MainDlg_List.cpp   列表/树类控件演示
*               MainDlg_Nav.cpp    宫格首页导航
*               MainDlg_Webkit.cpp 内嵌浏览器(教程页)
*               MainDlg_RichEdit.cpp 富文本/动态创建窗口演示
*               MainDlg_Skin.cpp   换肤演示
*               MainDlg_Animation.cpp 窗口动画/托盘演示
*               MainDlg_Misc.cpp   其余零散演示(菜单/消息框/矩阵变换等)
*/

#pragma once

#include "magnet/MagnetFrame.h"
#include "skin/SDemoSkin.h"
#include <controls.extend/STabCtrlHeaderBinder.h>
#include <helper/SDpiHelper.hpp>
#include "trayicon/SShellTray.h"
#include <SEdit2.h>

extern UINT g_dwSkinChangeMessage;

//demo 内部使用的定时器 ID
#define TIMER_QUIT 1000     //消息框演示:3 秒后强制退出
#define TIMER_SOUI4 1100    //SOUI 3.0 动画:定时投放"飘心"动画

//SListCtrl 演示数据结构,由 InitListCtrl 填充、funCmpare 排序、OnDestory 释放
struct student{
    TCHAR szName[100];
    TCHAR szSex[10];
    int age;
    int score;
};


/**
* @class      CMainDlg
* @brief      主窗口实现
*
* Describe    非模式窗口从SHostWnd派生，模式窗口从SHostDialog派生。
*             同时混入:
*               - CMagnetFrame           磁力吸附(演示窗口吸附到屏幕边缘)
*               - ISetOrLoadSkinHandler  换肤窗口回调(保存/加载皮肤)
*               - IAnimatorListener      属性动画监听(颜色动画演示)
*               - SDpiHandler            DPI 变化自适应
*/
class CMainDlg : public SHostWnd
			   , public CMagnetFrame	//磁力吸附
			   , public ISetOrLoadSkinHandler
               , public IAnimatorListener
               , public IAnimatorUpdateListener
               , public SDpiHandler<CMainDlg>
{
public:

    /**
     * CMainDlg
     * @brief    构造函数
     * Describe  使用uires.idx中定义的maindlg对应的xml布局创建UI
     */
    CMainDlg() : SHostWnd(UIRES.LAYOUT.maindlg),m_bLayoutInited(FALSE)
                 ,m_pMainTab(NULL),m_pContentsTab(NULL),m_pNavTitle(NULL)
    {
    }

protected:
    //////////////////////////////////////////////////////////////////////////
    //  窗口消息响应函数(MainDlg.cpp)
    LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
    void OnDestory();

	void OnClose()
	{
        DestroyWindow();
	}
	void OnMaximize()
	{
		GetNative()->SendMessage(WM_SYSCOMMAND,SC_MAXIMIZE);
	}
	void OnRestore()
	{
		GetNative()->SendMessage(WM_SYSCOMMAND,SC_RESTORE);
	}
	void OnMinimize()
	{
		GetNative()->SendMessage(WM_SYSCOMMAND,SC_MINIMIZE);
	}

	void OnSize(UINT nType, CSize size)
	{
		SetMsgHandled(FALSE);   //这一行很重要，保证消息继续传递给SHostWnd处理，当然也可以用SHostWnd::OnSize(nType,size);代替，但是这里使用的方法更简单，通用
		if(!m_bLayoutInited) return;
		if(nType==SIZE_MAXIMIZED)
		{
			FindChildByID(3)->SetVisible(TRUE);
			FindChildByID(2)->SetVisible(FALSE);
		}else if(nType==SIZE_RESTORED)
		{
			FindChildByID(3)->SetVisible(FALSE);
			FindChildByID(2)->SetVisible(TRUE);
		}
	}

    int OnCreate(LPCREATESTRUCT lpCreateStruct);

    //演示如何在应用层使用定时器(MainDlg_Animation.cpp)
	void OnTimer(UINT_PTR idEvent);

    //DUI菜单响应函数(MainDlg_Misc.cpp)
    void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);

protected:
    //属性动画监听回调(MainDlg_Animation.cpp)
	virtual void WINAPI onAnimationStart(IValueAnimator * pAnimator){}
	virtual void WINAPI onAnimationRepeat(IValueAnimator * pAnimator){}
	virtual void WINAPI onAnimationEnd(IValueAnimator * pAnimator);
	virtual void WINAPI onAnimationUpdate(IValueAnimator *pAnimator);

protected:
    //////////////////////////////////////////////////////////////////////////
    //  宫格首页导航(MainDlg_Nav.cpp)
    //  两级 tab:tab_main(home/contents)+ contents 页内的 tab_contents(7 分区);
    //  点击卡片/返回按钮由 NavigateToPage 协同两级 tab 完成动画切换。
    void InitPageNav();                 //缓存导航相关子控件指针
    void OnNavCard(IEvtArgs *e);        //宫格卡片点击:切换到对应演示分区
    void OnNavBack();                   //返回按钮点击:回到宫格首页
    void NavigateToPage(int iPage, const wchar_t *pszTitle); //0=回宫格首页;1..7=先无动画切 tab_contents 到目标分区,再动画切 tab_main

    //radio button 页:演示多种 tab 页切换绑定方式
    void OnTabPageRadioSwitch(IEvtArgs *pEvt);

    //////////////////////////////////////////////////////////////////////////
    //  控件/页面事件处理函数(按实现文件归组)
	//演示屏蔽指定edit控件的右键菜单(MainDlg_Misc.cpp)
	BOOL OnEditMenu(CPoint pt)
	{
		return TRUE;
	}

    //基础控件页 - 列表类演示(MainDlg_List.cpp)
    BOOL OnListHeaderClick(IEvtArgs *pEvt);     //演示 subscribeEvent 方式订阅表头点击并排序
	void OnMclvCtxMenu(IEvtArgs *pEvt);         //多列列表右键菜单
	void OnMclvEventOfPanel(IEvtArgs *pEvt);    //列表项面板事件转发(双击)
	void OnMcLvHeaderRelayout(IEvtArgs *e);     //表头内"全选"复选框的跟随布局
	void OnMclvItemSelChanged(IEvtArgs *e);     //mclistview 页单项选中状态事件:实时刷新已选条目数
	void OnLcItemSelChanged(IEvtArgs *e);       //listctrl 页单项选中状态事件:实时刷新已选条目数
	void OnMultiSelToggle(IEvtArgs *e);         //列表页"启用多选"复选框:运行时切换多选(框选)支持
	void OnBandToggle(IEvtArgs *e);             //列表页"启用框选"复选框:运行时切换框选手势(关闭后多选拖动为 fling)
	void OnFullRowSelToggle(IEvtArgs *e);       //treectrl页"整行选中"复选框:运行时切换整行高亮
	void OnInitListBox();                       //动态向 listbox 追加条目
	void OnInitGroup(IEvtArgs *e);              //grouplist 分组项初始化
	void OnInitItem(IEvtArgs *e);               //grouplist 列表项初始化
	void OnGroupStateChanged(IEvtArgs *e);      //分组展开/折叠状态联动
	void OnCtrlPageClick(IEvtArgs *e);          //左侧目录点击切换右侧演示页

    //基础控件页 - 富文本演示(MainDlg_RichEdit.cpp)
    void OnBtnInsertGif2RE();                   //向富文本插入 GIF 表情(OLE)
    void OnBtnAppendMsg();                      //追加格式化消息
    void OnBtnRtfSave();                        //保存为 RTF
    void OnBtnRtfOpen();                        //从 RTF 加载
    void OnGetCaret(IEvtArgs* e);               //自定义富文本光标样式
    void OnBtnFileWnd();                        //从文件创建窗口演示
    void OnBtnCreateChildren();                 //从 XML 字符串动态创建子窗口
    void OnBtnCreateByTemp();                   //从模板创建子窗口
    void OnBtnOpenWrapContent();                //wrap_content 布局演示窗口

    //教程页 - 内嵌浏览器(MainDlg_Webkit.cpp)
    void OnBtnWebkitGo();
	void OnBtnWebkitGo2(IEvtArgs *e){OnBtnWebkitGo();}
    void OnBtnWebkitBackward();
    void OnBtnWebkitForeward();
    void OnBtnWebkitRefresh();
    void OnChromeTabNew(IEvtArgs *pEvt);        //Chrome 风格页签的新建
    void OnUrlReNotify(IEvtArgs *pEvt);         //演示响应 Edit 的 EN_CHANGE

    //杂项/动画页演示(MainDlg_Misc.cpp)
    void OnBtnSelectGIF();
    void OnBtnMenu();
    void OnBtnHideTest();
    void OnBtnMsgBox();
	void OnBtnLRC();
    void OnMatrixWindowReNotify(IEvtArgs *pEvt);//矩阵变换参数输入
    void On3dViewRotate(IEvtArgs *e);           //3D 视图旋转轴选择
    void OnSetPropItemValue();                  //属性表控件设值
	void OnCbxInterpolotorChange(IEvtArgs *e);  //插值器选择联动
	void OnEventPath(IEvtArgs *e);              //路径视图长度统计
	void OnMenuSliderPos(IEvtArgs *pEvt);       //模拟菜单中控件事件
	void OnSpeedDec();                          //速度表减速
	void OnSpeedInc();                          //速度表加速

    //换肤演示(MainDlg_Skin.cpp)
	bool LoadSkin();
	HRESULT OnSkinChangeMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bHandled);
	void OnBtnSkin();

    //动画与托盘演示(MainDlg_Animation.cpp)
	void OnSetHostAnimation();
	void OnShellTrayNotify(IEvtArgs * e);
	void OnAnimationStop(IEvtArgs *e);
	void OnToggleLeft(IEvtArgs *e);
	void OnSouiClick();

	//提示窗口/显隐演示(MainDlg_Misc.cpp)
    void OnBtnTip();

    //UI控件的事件及响应函数映射表
	EVENT_MAP_BEGIN()
		EVENT_ID_HANDLER(R.id.tray_008,EventTrayNotify::EventID,OnShellTrayNotify)
		EVENT_NAME_COMMAND(L"btn_ani_hostwnd", OnSetHostAnimation)
		EVENT_HANDLER(EventPath::EventID,OnEventPath)
		EVENT_ID_HANDLER(R.id.cbx_interpolator,EventCBSelChange::EventID,OnCbxInterpolotorChange)
		EVENT_ID_COMMAND(1, OnClose)
		EVENT_ID_COMMAND(2, OnMaximize)
		EVENT_ID_COMMAND(3, OnRestore)
		EVENT_ID_COMMAND(5, OnMinimize)
		EVENT_ID_COMMAND(R.id.btn_tip,OnBtnTip)
		EVENT_NAME_CONTEXTMENU(L"edit_1140",OnEditMenu)
		EVENT_NAME_COMMAND(L"btn_msgbox",OnBtnMsgBox)

		//<--在新版本的uiresbuilder生成的resource.h中定义了R.id, R.name两个对象，可以使用如下方式来关联变量。
		EVENT_ID_COMMAND(R.id.btnSelectGif,OnBtnSelectGIF)
        EVENT_NAME_COMMAND(R.name.btn_menu,OnBtnMenu)
        EVENT_NAME_COMMAND(R.name.btn_webkit_go,OnBtnWebkitGo)
        EVENT_ID_COMMAND(R.id.btn_createchildren,OnBtnCreateChildren)
		EVENT_ID_COMMAND(R.id.btn_init_listbox,OnInitListBox)
		EVENT_ID_COMMAND(R.id.btn_skin,OnBtnSkin)
		EVENT_ID_COMMAND(R.id.btn_open_wrap_content,OnBtnOpenWrapContent)
        //-->
		EVENT_NAME_COMMAND(L"btn_create_by_temp",OnBtnCreateByTemp)
        EVENT_NAME_COMMAND(L"btn_webkit_back",OnBtnWebkitBackward)
        EVENT_NAME_COMMAND(L"btn_webkit_fore",OnBtnWebkitForeward)
        EVENT_NAME_COMMAND(L"btn_webkit_refresh",OnBtnWebkitRefresh)
		EVENT_NAME_HANDLER(L"edit_url",EventKeyEnter::EventID,OnBtnWebkitGo2)
        EVENT_NAME_COMMAND(L"btn_hidetst",OnBtnHideTest)
        EVENT_NAME_COMMAND(L"btn_insert_gif",OnBtnInsertGif2RE)
        EVENT_NAME_COMMAND(L"btn_append_msg",OnBtnAppendMsg)
        EVENT_NAME_COMMAND(L"btn_richedit_save",OnBtnRtfSave)
        EVENT_NAME_COMMAND(L"btn_richedit_open",OnBtnRtfOpen)
		EVENT_NAME_COMMAND(L"btn_lrc",OnBtnLRC)
        EVENT_NAME_HANDLER(L"chromeTab",EVT_CHROMETAB_NEW,OnChromeTabNew)
        EVENT_NAME_COMMAND(L"btn_filewnd",OnBtnFileWnd)
        EVENT_NAME_HANDLER(L"edit_url",EVT_RE_NOTIFY,OnUrlReNotify)
        EVENT_NAME_HANDLER(L"mclv_test",EVT_CTXMENU,OnMclvCtxMenu)
		EVENT_NAME_HANDLER(L"mclv_test", EventOfPanel::EventID, OnMclvEventOfPanel)
        EVENT_NAME_HANDLER(L"edit_rotate",EVT_RE_NOTIFY,OnMatrixWindowReNotify)
        EVENT_NAME_HANDLER(L"edit_scale",EVT_RE_NOTIFY,OnMatrixWindowReNotify)
        EVENT_NAME_HANDLER(L"edit_skew",EVT_RE_NOTIFY,OnMatrixWindowReNotify)
        EVENT_NAME_HANDLER(L"edit_translate",EVT_RE_NOTIFY,OnMatrixWindowReNotify)

        EVENT_NAME_HANDLER(L"menu_slider",EventSliderPos::EventID,OnMenuSliderPos)
		EVENT_ID_HANDLER(R.id.gl_catalog,EventGroupListInitGroup::EventID,OnInitGroup)
		EVENT_ID_HANDLER(R.id.gl_catalog,EventGroupListInitItem::EventID,OnInitItem)
		EVENT_ID_HANDLER(R.id.gl_catalog,EventGroupStateChanged::EventID,OnGroupStateChanged)
		EVENT_ID_HANDLER(R.id.gl_catalog,EventGroupListItemCheck::EventID,OnCtrlPageClick)
		EVENT_NAME_HANDLER(L"mclv_test_header",EventHeaderRelayout::EventID,OnMcLvHeaderRelayout)
		EVENT_NAME_HANDLER(L"mclv_test",EventItemSelChanged::EventID,OnMclvItemSelChanged)
		EVENT_NAME_HANDLER(L"lc_test",EventItemSelChanged::EventID,OnLcItemSelChanged)
		EVENT_NAME_HANDLER(L"chk_multi_mclv",EventCmd::EventID,OnMultiSelToggle)
		EVENT_NAME_HANDLER(L"chk_multi_lc",EventCmd::EventID,OnMultiSelToggle)
		EVENT_NAME_HANDLER(L"chk_multi_lvfix",EventCmd::EventID,OnMultiSelToggle)
		EVENT_NAME_HANDLER(L"chk_multi_lvflex",EventCmd::EventID,OnMultiSelToggle)
		EVENT_NAME_HANDLER(L"chk_multi_tile",EventCmd::EventID,OnMultiSelToggle)
		EVENT_NAME_HANDLER(L"chk_multi_tv",EventCmd::EventID,OnMultiSelToggle)
		EVENT_NAME_HANDLER(L"chk_multi_tree",EventCmd::EventID,OnMultiSelToggle)
		EVENT_NAME_HANDLER(L"chk_band_mclv",EventCmd::EventID,OnBandToggle)
		EVENT_NAME_HANDLER(L"chk_band_lc",EventCmd::EventID,OnBandToggle)
		EVENT_NAME_HANDLER(L"chk_band_lvfix",EventCmd::EventID,OnBandToggle)
		EVENT_NAME_HANDLER(L"chk_band_lvflex",EventCmd::EventID,OnBandToggle)
		EVENT_NAME_HANDLER(L"chk_band_tile",EventCmd::EventID,OnBandToggle)
		EVENT_NAME_HANDLER(L"chk_band_tv",EventCmd::EventID,OnBandToggle)
		EVENT_NAME_HANDLER(L"chk_band_tree",EventCmd::EventID,OnBandToggle)
		EVENT_NAME_HANDLER(L"chk_fullrow_tree",EventCmd::EventID,OnFullRowSelToggle)

		EVENT_NAME_HANDLER(L"rotate_x",EventSwndStateChanged::EventID,On3dViewRotate)
		EVENT_NAME_HANDLER(L"rotate_y",EventSwndStateChanged::EventID,On3dViewRotate)
		EVENT_NAME_HANDLER(L"rotate_z",EventSwndStateChanged::EventID,On3dViewRotate)
		EVENT_ID_COMMAND(R.id.btn_set_prop_value,OnSetPropItemValue)
		EVENT_NAME_HANDLER(L"tgl_left",EventCmd::EventID,OnToggleLeft)
		EVENT_NAME_COMMAND(L"img_soui",OnSouiClick)
		EVENT_HANDLER(EventSwndAnimationStop::EventID,OnAnimationStop)
		EVENT_NAME_HANDLER(L"ctrl_hk1",EventGetCaret::EventID,OnGetCaret)
		EVENT_NAME_COMMAND(L"btn_speed_dec", OnSpeedDec)
		EVENT_NAME_COMMAND(L"btn_speed_inc", OnSpeedInc)

		//宫格首页导航:卡片点击统一进入 OnNavCard,返回按钮进入 OnNavBack
		EVENT_NAME_HANDLER(L"card_ctrls",EventCmd::EventID,OnNavCard)
		EVENT_NAME_HANDLER(L"card_webkit",EventCmd::EventID,OnNavCard)
		EVENT_NAME_HANDLER(L"card_animator",EventCmd::EventID,OnNavCard)
		EVENT_NAME_HANDLER(L"card_layout",EventCmd::EventID,OnNavCard)
		EVENT_NAME_HANDLER(L"card_misc",EventCmd::EventID,OnNavCard)
		EVENT_NAME_HANDLER(L"card_skia",EventCmd::EventID,OnNavCard)
		EVENT_NAME_HANDLER(L"card_about",EventCmd::EventID,OnNavCard)
		EVENT_NAME_COMMAND(L"btn_back",OnNavBack)
	EVENT_MAP_END2(SHostWnd)

    //HOST消息及响应函数映射表
	BEGIN_MSG_MAP_EX(CMainDlg)
		CHAIN_MSG_MAP(SDpiHandler<CMainDlg>)
		MSG_WM_CREATE(OnCreate)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_DESTROY(OnDestory)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_COMMAND(OnCommand)
		MSG_WM_TIMER(OnTimer)
		MESSAGE_HANDLER(g_dwSkinChangeMessage, OnSkinChangeMessage)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

protected:
    //////////////////////////////////////////////////////////////////////////
    //  辅助函数
    void InitListCtrl();            //列表控件演示数据初始化(MainDlg_Init.cpp)
	void InitSoui3Animation();      //SOUI 3.0 动画初始化(MainDlg_Animation.cpp)
	void InitDragDrop();            //OLE 拖放演示初始化(MainDlg_Init.cpp)
	void InitRichEditHost();        //富文本宿主初始化(MainDlg_Init.cpp)
	void InitListViews();           //各类 ListView/TreeView 适配器绑定(MainDlg_Init.cpp)
	void InitMiscCtrls();           //其余零散控件初始化(MainDlg_Init.cpp)

	virtual bool SaveSkin(SkinType skinType, SkinSaveInf & skinSaveInf);

private:
	BOOL			m_bLayoutInited;/**<UI完成布局标志 */
	STabCtrlHeaderBinder* m_pTabBinder;
	STabCtrlHeaderBinder* m_pTabBinder2;

	//宫格首页导航相关控件缓存(MainDlg_Nav.cpp 中维护)
	STabCtrl *  m_pMainTab;     /**<主页面容器 tab_main:0=home 宫格首页,1=contents */
	STabCtrl *  m_pContentsTab; /**<分区页容器 tab_contents,位于 contents 页内 */
	SWindow *   m_pNavTitle;    /**<导航栏页面标题 txt_nav_title */

};
