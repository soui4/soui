﻿/**
* Copyright (C) 2014-2050 
* All rights reserved.
* 
* @file       MainDlg.h
* @brief      
* @version    v1.0      
* @author     SOUI group   
* @date       2014/08/15
* 
* Describe    主窗口实现
*/

#pragma once

using namespace SOUI;

#include "magnet/MagnetFrame.h"
#include "skin/SDemoSkin.h"
#include <controls.extend/SMcListViewEx/STabCtrlHeaderBinder.h>
#include <helper/SDpiHelper.hpp>
#include "trayicon/SShellTray.h"

extern UINT g_dwSkinChangeMessage;


/**
* @class      CMainDlg
* @brief      主窗口实现
* 
* Describe    非模式窗口从SHostWnd派生，模式窗口从SHostDialog派生
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
    {
    } 

protected:
    //////////////////////////////////////////////////////////////////////////
    //  Window消息响应函数
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
    
    //演示如何在应用层使用定时器
	void OnTimer(UINT_PTR idEvent);

    //DUI菜单响应函数
    void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);
        
protected:
	virtual void WINAPI onAnimationStart(IValueAnimator * pAnimator){}
	virtual void WINAPI onAnimationRepeat(IValueAnimator * pAnimator){}
	virtual void WINAPI onAnimationEnd(IValueAnimator * pAnimator);
	virtual void WINAPI onAnimationUpdate(IValueAnimator *pAnimator);

protected:
    //virtual void OnSetSkin(int iSkin);

    //////////////////////////////////////////////////////////////////////////
    // SOUI事件处理函数
	//演示屏蔽指定edit控件的右键菜单
	BOOL OnEditMenu(CPoint pt)
	{
		return TRUE;
	}

    //按钮控件的响应
    void OnBtnSelectGIF();
    void OnBtnMenu();

    void OnBtnInsertGif2RE();
    void OnBtnAppendMsg();
    void OnBtnRtfSave();
    void OnBtnRtfOpen();
    
    void OnBtnHideTest();
    void OnBtnMsgBox();

    void OnBtnWebkitGo();
    void OnBtnWebkitBackward();
    void OnBtnWebkitForeward();
    void OnBtnWebkitRefresh();

	void OnBtnLRC();
    
	bool LoadSkin();

	//演示如何使用subscribeEvent来不使用事件映射表实现事件响应
    BOOL OnListHeaderClick(IEvtArgs *pEvt);

        
    void OnChromeTabNew(IEvtArgs *pEvt);

    void OnTabPageRadioSwitch(IEvtArgs *pEvt);
    
    void OnBtnFileWnd();

    void OnUrlReNotify(IEvtArgs *pEvt);
    
    void OnMclvCtxMenu(IEvtArgs *pEvt);
	void OnMclvEventOfPanel(IEvtArgs *pEvt);
	
    //处理模拟菜单中控件的事件
    void OnMenuSliderPos(IEvtArgs *pEvt);
    
    void OnMatrixWindowReNotify(IEvtArgs *pEvt);

    void OnBtnCreateChildren();
    void OnBtnSkin();
	void OnInitListBox();

	void OnBtnTip();

	void OnBtnOpenWrapContent();
	
	void OnCbxInterpolotorChange(IEvtArgs *e);

	void OnEventPath(IEvtArgs *e);

	HRESULT OnSkinChangeMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bHandled);

	void OnInitGroup(IEvtArgs *e);
	void OnInitItem(IEvtArgs *e);
	void OnGroupStateChanged(IEvtArgs *e);
	void OnCtrlPageClick(IEvtArgs *e);

	void OnMcLvHeaderRelayout(IEvtArgs *e);

	//从模板创建子窗口demo
	void OnBtnCreateByTemp();

	void On3dViewRotate(IEvtArgs *e);

	void OnSetPropItemValue();

	void OnToggleLeft(IEvtArgs *e);

	void OnSouiClick();

	void OnSetHostAnimation();
	void OnShellTrayNotify(IEvtArgs * e);
	void OnAnimationStop(IEvtArgs *e);

	void OnGetCaret(IEvtArgs* e);

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

		EVENT_NAME_HANDLER(L"rotate_x",EventSwndStateChanged::EventID,On3dViewRotate)
		EVENT_NAME_HANDLER(L"rotate_y",EventSwndStateChanged::EventID,On3dViewRotate)
		EVENT_NAME_HANDLER(L"rotate_z",EventSwndStateChanged::EventID,On3dViewRotate)
		EVENT_ID_COMMAND(R.id.btn_set_prop_value,OnSetPropItemValue)
		EVENT_NAME_HANDLER(L"tgl_left",EventCmd::EventID,OnToggleLeft)
		EVENT_NAME_COMMAND(L"img_soui",OnSouiClick)
		EVENT_HANDLER(EventSwndAnimationStop::EventID,OnAnimationStop)
		EVENT_NAME_HANDLER(L"ctrl_hk1",EventGetCaret::EventID,OnGetCaret)
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
    void InitListCtrl();
	void InitSoui3Animation();

	virtual bool SaveSkin(SkinType skinType, SkinSaveInf & skinSaveInf);

private:
	BOOL			m_bLayoutInited;/**<UI完成布局标志 */
	STabCtrlHeaderBinder* m_pTabBinder;
	STabCtrlHeaderBinder* m_pTabBinder2;

};
