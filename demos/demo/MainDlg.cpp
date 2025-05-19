﻿// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include "helper/SMenu.h"
#include <controls.extend/FileHelper.h>
#include <controls.extend/SChatEdit.h>
#include <controls.extend/reole/RichEditOle.h>
#include <controls.extend/SHexEdit.h>
#include "SMatrixWindow.h"
#include "FormatMsgDlg.h"
#include <math.h>
#include <valueAnimator/SValueAnimator.h>
#include <helper/SMenuEx.h>
#include <helper/SDIBHelper.h>

#pragma warning(disable:4192)

#include "skin/SSkinLoader.h"
//#define SHOW_AERO //open aero for vista and win7

#define TIMER_QUIT 1000
#define TIMER_SOUI4 1100

#ifdef SHOW_AERO
#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")
#endif

#include <shellapi.h>
#include "skin/SetSkinWnd2.h"
#include <controls.extend/SMcListViewEx/SMCListViewEx.h>
#include "adapter.h"
#include "trayicon/SShellTray.h"
#include "CAdapter.h"
#include "CDropTarget.h"

#ifdef _WIN32
#define SKIN_CFG _T("\\themes\\skin_config.xml")
#else
#define SKIN_CFG _T("/themes/skin_config.xml")
#endif//_WIN32
#define kLogTag "maindlg"

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


struct student{
    TCHAR szName[100];
    TCHAR szSex[10];
    int age;
    int score;
};

//init listctrl
void CMainDlg::InitListCtrl()
{
    //找到列表控件
    SListCtrl *pList=FindChildByName2<SListCtrl>(L"lc_test");
    if(pList)
    {
        //列表控件的唯一子控件即为表头控件
        SWindow *pHeader=pList->GetWindow(GSW_FIRSTCHILD);
        //向表头控件订阅表明点击事件，并把它和OnListHeaderClick函数相连。
        pHeader->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK,Subscriber(&CMainDlg::OnListHeaderClick,this));

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
	SMCListViewEx *musiclist = FindChildByName2<SMCListViewEx>(_T("musiclist"));
	if (musiclist)
	{
		SMusicListAdapter* musicadapter = new SMusicListAdapter(1, m_hWnd);
		musiclist->SetAdapter(musicadapter);
        musicadapter->Release();
	}
}

void SaveSkinInf2File(SkinType skinType, SkinSaveInf &skinSaveInf)
{
	SXmlDoc docSave;
	SXmlNode rootNode = docSave.root().append_child(L"DEMO_SKIN_CONFIG");
	SXmlNode childSkinType = rootNode.append_child(L"skinInf");
	childSkinType.append_attribute(L"type").set_value(skinType);
	SStringT strSkinConfigPath = SApplication::getSingleton().GetAppDir() + SKIN_CFG;
	switch (skinType)
	{
	case color://纯色只有SkinSaveInf的color有效
		childSkinType.append_attribute(L"color").set_value((int)skinSaveInf.color);
		break;				
	case sys://此处为系统皮肤，只需要给文件路径和margin
		{
			childSkinType.append_attribute(L"skin_path").set_value(skinSaveInf.filepath.c_str());
			SStringW margin;
			margin.Format(L"%d,%d,%d,%d", skinSaveInf.margin.left, skinSaveInf.margin.top, skinSaveInf.margin.right, skinSaveInf.margin.bottom);
			childSkinType.append_attribute(L"skin_margin").set_value(margin);
		}
		break;
	case builtin:
	default:
		break;
	}
	docSave.save_file(strSkinConfigPath);
}

bool CMainDlg::SaveSkin(SkinType skinType, SkinSaveInf &skinSaveInf)
{
	HRESULT hr = S_OK;
	SaveSkinInf2File(skinType, skinSaveInf);
	return hr == S_OK;
}

void LoadSkinFormXml(SDemoSkin *skin, SkinType *skinType, SkinLoadInf *skininf)
{
	SStringT strSkinConfigPath = SApplication::getSingleton().GetAppDir() + SKIN_CFG;

	SXmlDoc docLoad;
	bool bLoad = docLoad.load_file(strSkinConfigPath);
	if (bLoad)
	{
		SXmlNode skinInf = docLoad.root().child(L"DEMO_SKIN_CONFIG").child(L"skinInf");
		*skinType = (SkinType)skinInf.attribute(L"type").as_int();
		switch (*skinType)
		{
			//纯色只有SkinSaveInf的color有效
		case color:
			skininf->color = skinInf.attribute(L"color").as_int();
			break;			
			//此处为系统皮肤，只需要给文件路径和margin
		case sys:
			skininf->filepath = skinInf.attribute(L"skin_path").as_string();
			int v1 = 0, v2 = 0, v3 = 0, v4 = 0;
			swscanf(skinInf.attribute(L"skin_margin").as_string(), L"%d,%d,%d,%d", &v1, &v2, &v3, &v4);
			skininf->margin.left = v1;
			skininf->margin.top = v2;
			skininf->margin.right = v3;
			skininf->margin.bottom = v4;
			break;
		}
	}
}
bool CMainDlg::LoadSkin()
{
	SDemoSkin *skin = (SDemoSkin *)GETSKIN(L"demoskinbk",GetScale());
	if (skin)
	{
		SkinLoadInf loadInf;
		SkinType type;
		LoadSkinFormXml(skin, &type, &loadInf);
		skin->SetHander(this);
		return skin->LoadSkin(type, loadInf);
	}
	return false;
}


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

//表头点击事件处理函数
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
	SDemoSkin *skin = (SDemoSkin *)GETSKIN(L"demoskinbk",GetScale());
	if (skin)
	{
		skin->SaveSkin();
	}
    SetMsgHandled(FALSE);
	if (m_pTabBinder)
		delete m_pTabBinder;
	if (m_pTabBinder2)
		delete m_pTabBinder2;
}


#ifdef _WIN32
class CSmileySource2 : public CSmileySource
{
public:
    CSmileySource2(){}

protected:
    //获对ID对应的图片路径
    virtual SStringW ImageID2Path(UINT nID)
    {
        return SStringW().Format(L"./gif/%d.gif",nID);
    }
};

//Richedit中插入表情使用的回调函数。
ISmileySource * CreateSource2()
{
    return  new CSmileySource2;
}
#endif

HRESULT CMainDlg::OnSkinChangeMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bHandled)
{
	STabCtrl *pMainTab = FindChildByName2<STabCtrl>("tab_main");
	if(GETSKIN(L"tab_icon",100))
	{
		pMainTab->SetAttribute(L"iconSkin",L"tab_icon");
	}else
	{
		pMainTab->SetAttribute(L"iconSkin",L"skin_page_icons");
	}
	FindChildByID(9527)->Invalidate();
	SDemoSkin *skin = (SDemoSkin *) GETSKIN(L"demoskinbk",GetScale());

	DWORD tm1=GetTickCount();
	COLORREF crTheme = skin->GetThemeColor();
	if (crTheme != CR_INVALID)
		GetRoot()->DoColorize(crTheme | 0xff000000);
	else
		GetRoot()->DoColorize(0);

	SLOGI()<<"DoColorize spend "<<GetTickCount()-tm1<<" ms";
	return S_OK;
}

LRESULT CMainDlg::OnInitDialog( HWND hWnd, LPARAM lParam )
{
	SLOGI()<<"OnInitDialog";

	m_bLayoutInited=TRUE;
	FindChildByID2<SGroupList>(R.id.gl_catalog)->SelectPage(R.id.page_mclistview);

	STabCtrl *pTabCtrl = FindChildByName2<STabCtrl>(L"tab_radio2");
	{
		m_pTabBinder = new STabCtrlHeaderBinder(pTabCtrl);
		m_pTabBinder->Bind(FindChildByName(L"radio3_1"), 0);
		m_pTabBinder->Bind(FindChildByName(L"radio3_2"), 1);
		m_pTabBinder->Bind(FindChildByName(L"radio3_3"), 2);
		m_pTabBinder->Bind(FindChildByName(L"radio3_4"), 3);
		m_pTabBinder->Bind(FindChildByName(L"radio3_5"), 4);
		m_pTabBinder->Bind(FindChildByName(L"radio3_6"), 5);
		m_pTabBinder2 = new STabCtrlHeaderBinder(pTabCtrl);
		m_pTabBinder->Bind(FindChildByName(L"btn4_1"), 0);
		m_pTabBinder->Bind(FindChildByName(L"btn4_2"), 1);
		m_pTabBinder->Bind(FindChildByName(L"btn4_3"), 2);
		m_pTabBinder->Bind(FindChildByName(L"btn4_4"), 3);
		m_pTabBinder->Bind(FindChildByName(L"btn4_5"), 4);
		m_pTabBinder->Bind(FindChildByName(L"btn4_6"), 5);
	}
	LoadSkin();
    //设置为磁吸主窗口
    SetMainWnd(m_hWnd);
    
    InitListCtrl();

    //设置标题
    SStringW strTitle = SStringW().Format(GETSTRING(R.string.title),SOUI_VER1,SOUI_VER2,SOUI_VER3,SOUI_VER4);
    FindChildByID(R.id.txt_title)->SetWindowText(S_CW2T(GetRoot()->tr(strTitle)));
    
    //演示在SOUI中的拖放
    HRESULT hr=::RegisterDragDrop(m_hWnd,GetDropTarget());
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
    SRichEdit *pEdit = FindChildByName2<SRichEdit>(L"re_gifhost");
    if(pEdit)
    {
#ifdef _WIN32
        SetSRicheditOleCallback(pEdit,CreateSource2);
#endif
        pEdit->SetAttribute(L"rtf",L"rtf:rtf_test");
    }

    //演示如何响应Edit的EN_CHANGE事件
    SEdit *pEditUrl = FindChildByName2<SEdit>(L"edit_url");
    if(pEditUrl)
    {
        pEditUrl->SSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
    }
    
    //演示SetWindowRgn用法
    SWindow *pWndRgn = FindChildByName(L"wnd_rgn");
    if(pWndRgn)
    {
		//性能优化后，隐藏窗口不能直接获取位置，这里先主动请求布局。
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


        SAutoRefPtr<IRegionS> pRgn;
        GETRENDERFACTORY->CreateRegion(&pRgn);
		CRect rc=pWndRgn->GetWindowRect();
		rc.MoveToXY(0,0);//注意：SWindow将窗口的左上角定义为Rgn的原点。
		pRgn->CombineEllipse(&rc,RGN_COPY);
        pWndRgn->SetWindowRgn(pRgn,TRUE);
    }
    
    //行高固定的列表
    SListView *pLstViewFix = FindChildByName2<SListView>("lv_test_fix");
    if(pLstViewFix)
    {
        ILvAdapter *pAdapter = new CTestAdapterFix;
        pLstViewFix->SetAdapter(pAdapter);
        pAdapter->Release();
    }

	//行高固定的列表
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

	FindChildByID2<SShellTray>(R.id.tray_008)->StartAni();

	SPathView *pPathView = FindChildByName2<SPathView>("pv_test");
	if(pPathView)
	{
		CRect rc=pPathView->GetClientRect();
		POINT pts[10];
		for(int i=0;i<ARRAYSIZE(pts);i++)
		{
			pts[i].x = rand()%500;
			pts[i].y = rand()%300;
		}
		pPathView->AddPoint(pts,ARRAYSIZE(pts));
	}

	//init soui 3.0 animation.
	InitSoui3Animation();

	BYTE hexData[128] = {0};
	for (int i=0; i<sizeof(hexData); ++i)
	{
		hexData[i] = i;
	}
	SHexEdit* hexEdit = FindChildByName2<SHexEdit>("ctrl_hexedit");
	hexEdit->SetData(hexData, sizeof(hexData));

    return 0;
}

void CMainDlg::OnBtnWebkitGo()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        SEdit *pEdit=FindChildByName2<SEdit>(L"edit_url");
        SStringT strUrl=pEdit->GetWindowText();
        pWebkit->SetAttribute(L"url",S_CT2W(strUrl),FALSE);
    }
}

void CMainDlg::OnBtnWebkitBackward()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        pWebkit->GetWebView()->goBack();
    }
}

void CMainDlg::OnBtnWebkitForeward()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        pWebkit->GetWebView()->goForward();
    }
}

void CMainDlg::OnBtnWebkitRefresh()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        pWebkit->GetWebView()->reload();
    }
}

void CMainDlg::OnBtnSelectGIF()
{
#ifdef _WIN32
    SGifPlayer *pGifPlayer = FindChildByName2<SGifPlayer>(L"giftest");
    if(pGifPlayer)
    {
        CFileDialogEx openDlg(TRUE,_T("gif"),0,6,_T("gif files(*.gif)\0*.gif\0All files (*.*)\0*.*\0\0"));
        if(openDlg.DoModal()==IDOK)
            pGifPlayer->PlayGifFile(openDlg.m_szFileName);
    }
#endif
}

void CMainDlg::OnBtnMenu()
{
    CPoint pt;
    GetCursorPos(&pt);

    //使用自绘菜单
    SMenu menu;
	menu.LoadMenu(_T("SMenu:menu_test"));
    menu.TrackPopupMenu(0,pt.x,pt.y,m_hWnd);
}

//演示如何响应菜单事件
void CMainDlg::OnCommand( UINT uNotifyCode, int nID, HWND wndCtl )
{
    if(uNotifyCode==0)
    {
        if(nID==6)
        {//nID==6对应menu_test定义的菜单的exit项。
            PostMessage(WM_CLOSE);
        }else if(nID==5)
        {//about SOUI
            STabCtrl *pTabCtrl = FindChildByName2<STabCtrl>(L"tab_main");
            if(pTabCtrl) pTabCtrl->SetCurSel(_T("about"));
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


void CMainDlg::OnBtnHideTest()
{
    SWindow * pBtn = FindChildByName(L"btn_display");
    if(pBtn) pBtn->SetVisible(!pBtn->IsVisible(TRUE),TRUE);
}


void CMainDlg::OnBtnInsertGif2RE()
{
    SRichEdit *pEdit = FindChildByName2<SRichEdit>(L"re_gifhost");
    if(pEdit)
    {
        CFileDialogEx openDlg(TRUE,_T("gif"),0,6,_T("gif files(*.gif)\0*.gif\0All files (*.*)\0*.*\0\0"));
        if(openDlg.DoModal()==IDOK)
        {
#if defined(_WIN32) && !defined(_ARM64_) && !defined(_ARM_)
            ISmileySource *pSource = new CSmileySource2;
            HRESULT hr=pSource->LoadFromFile(S_CT2W(openDlg.m_szFileName));
            if(SUCCEEDED(hr))
            {
                SComPtr<ISmileyCtrl> pSmiley;
                hr=::CoCreateInstance(CLSID_SSmileyCtrl,NULL,CLSCTX_INPROC,__uuidof(ISmileyCtrl),(LPVOID*)&pSmiley); 
                if(SUCCEEDED(hr))
                {
                    pSmiley->SetSource(pSource);
                    SComPtr<IRichEditOle> ole;
                    pEdit->SSendMessage(EM_GETOLEINTERFACE,0,(LPARAM)&ole);
                    pSmiley->Insert2Richedit(ole);
                }else
                {
                    UINT uRet = SMessageBox(m_hWnd,_T("可能是因为没有向系统注册表情COM模块。\\n现在注册吗?"),_T("创建表情OLE对象失败"),MB_YESNO|MB_ICONSTOP);
                    if(uRet == IDYES)
                    {
                        HMODULE hMod = LoadLibrary(_T("sosmiley.dll"));
                        if(hMod)
                        {
                            typedef HRESULT (STDAPICALLTYPE *DllRegisterServerPtr)();
                            DllRegisterServerPtr funRegDll = (DllRegisterServerPtr)GetProcAddress(hMod,"DllRegisterServer");
                            if(funRegDll)
                            {
                                HRESULT hr=funRegDll();
                                if(FAILED(hr))
                                {
                                    SMessageBox(m_hWnd,_T("请使用管理员权限运行模块注册程序"),_T("注册表情COM失败"),MB_OK|MB_ICONSTOP);
                                }else
                                {
                                    SMessageBox(m_hWnd,_T("请重试"),_T("注册成功"),MB_OK|MB_ICONINFORMATION);
                                }
                            }
                            FreeLibrary(hMod);
                        }else
                        {
                            SMessageBox(m_hWnd,_T("没有找到表情COM模块[sosmiley.dll]。\\n现在注册吗"),_T("错误"),MB_OK|MB_ICONSTOP);
                        }
                    }
                }
            }else
            {
                SMessageBox(m_hWnd,_T("加载表情失败"),_T("错误"),MB_OK|MB_ICONSTOP);
            }
            pSource->Release();
#endif
        }
    }
}

void CMainDlg::OnBtnAppendMsg()
{
    SChatEdit *pEdit = FindChildByName2<SChatEdit>(L"re_gifhost");
    if(pEdit)
    {
        CFormatMsgDlg formatMsgDlg;
        if(formatMsgDlg.DoModal()==IDOK)
        {
            for(int i=0;i<formatMsgDlg.m_nRepeat;i++)
                pEdit->AppendFormatText(S_CT2W(SStringT().Format(_T("line:%d "),i) + formatMsgDlg.m_strMsg));
        }
    }
}

void CMainDlg::OnBtnMsgBox()
{
    SMessageBox(0,_T("this is a message box"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
    SMessageBox(0,_T("this message box includes two buttons"),_T("haha"),MB_YESNO|MB_ICONQUESTION);

	SNativeWnd::SetTimer(TIMER_QUIT,3000,NULL);//3S后退出APP
    SMessageBox(0,_T("this message box includes three buttons. \nthe app will quit after 3 seconds if you keep the msgbox open!"),_T("Alarm"),MB_ABORTRETRYIGNORE|MB_ICONSTOP);
	SNativeWnd::KillTimer(TIMER_QUIT);
}

class SSkiaTestWnd : public SHostWnd
{
public:
	SSkiaTestWnd(LPCTSTR pszResName = NULL):SHostWnd(pszResName){}


protected:

	void OnFinalMessage(HWND hWnd){ 
	    //演示OnFinalMessage用法,下面new出来的不需要显示调用delete
	    SHostWnd::OnFinalMessage(hWnd);
	    delete this;
	}  
};

void CMainDlg::OnBtnLRC()
{	
    static int s_Count = 0;
    
    SSkiaTestWnd* pHostWnd = new SSkiaTestWnd(_T("layout:dlg_skiatext"));
    pHostWnd->CreateEx(m_hWnd,WS_POPUP,0,0,0,0,0);
    
    //选择一种吸附模式
    CMagnetFrame::ATTACHMODE am;
    CMagnetFrame::ATTACHALIGN aa;
    switch(s_Count++ %4)
    {
    case 0:am = AM_TOP,aa = AA_LEFT;break;
    case 1:am = AM_BOTTOM, aa=AA_LEFT;break;
    case 2:am = AM_LEFT, aa=AA_TOP;break;
    case 3:am = AM_RIGHT,aa=AA_TOP;break;
    }
    AddSubWnd(pHostWnd->m_hWnd, am,aa);
    pHostWnd->ShowWindow(SW_SHOW);
}

void CMainDlg::OnTabPageRadioSwitch(IEvtArgs *pEvt)
{
    EventSwndStateChanged *pEvt2 = sobj_cast<EventSwndStateChanged>(pEvt);
    if(EventSwndStateChanged_CheckState(pEvt2,WndState_Check) && (pEvt2->dwNewState & WndState_Check))
    {
        int id= pEvt->IdFrom();
        STabCtrl *pTab =FindChildByName2<STabCtrl>(L"tab_radio2");
        if(pTab) pTab->SetCurSel(id-10000);
    }
}

void CMainDlg::OnBtnRtfSave()
{
    SRichEdit *pEdit = FindChildByName2<SRichEdit>(L"re_gifhost");
    if(pEdit)
    {
        CFileDialogEx openDlg(FALSE,_T("rtf"),_T("soui_richedit"),6,_T("rtf files(*.rtf)\0*.rtf\0All files (*.*)\0*.*\0\0"));
        if(openDlg.DoModal()==IDOK)
        {
            pEdit->SaveRtf(openDlg.m_szFileName);
        }
    }
}

void CMainDlg::OnBtnRtfOpen()
{
    SRichEdit *pEdit = FindChildByName2<SRichEdit>(L"re_gifhost");
    if(pEdit)
    {
        CFileDialogEx openDlg(TRUE,_T("rtf"),0,6,_T("rtf files(*.rtf)\0*.rtf\0All files (*.*)\0*.*\0\0"));
        if(openDlg.DoModal()==IDOK)
        {
            pEdit->LoadRtf(openDlg.m_szFileName);
        }
    }
}


void CMainDlg::OnChromeTabNew( IEvtArgs *pEvt )
{
    static int iPage = 0;
    EventChromeTabNew *pEvtTabNew = (EventChromeTabNew*)pEvt;

    SStringT strTitle = SStringT().Format(_T("新建窗口 %d"),++iPage);
    pEvtTabNew->pNewTab->SetWindowText(strTitle);
    pEvtTabNew->pNewTab->SetAttribute(L"tip",S_CT2W(strTitle));
}

//演示如何使用文件资源创建窗口
void CMainDlg::OnBtnFileWnd()
{
    //由于资源中使用了相对路径，需要将当前路径指定到资源所在位置
    SStringT strCurDir = SApplication::getSingleton().GetAppDir();
	#ifdef _WIN32
    strCurDir += _T("\\filewnd");
	#else
    strCurDir += _T("/filewnd");
#endif //_WIN32
    SetCurrentDirectory(strCurDir);
    if(GetFileAttributes(_T("test.xml"))==INVALID_FILE_ATTRIBUTES)
    {
        SMessageBox(m_hWnd,_T("没有找到资源文件！"),_T("错误"),MB_OK|MB_ICONSTOP);
        return ;
    }
    SHostDialog fileDlg(_T("file:test.xml"));
    fileDlg.DoModal(m_hWnd);
}

//演示如何响应Edit的EN_CHANGE事件
void CMainDlg::OnUrlReNotify(IEvtArgs *pEvt)
{
    EventRENotify *pEvt2 = sobj_cast<EventRENotify>(pEvt);
    SLOGFMTD(_T("OnUrlReNotify,iNotify = %d"),pEvt2->iNotify);
    if(pEvt2->iNotify == EN_CHANGE)
    {
        SLOGFMTD(_T("OnUrlReNotify,iNotify = EN_CHANGE"));    
    }
}

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

//处理模拟菜单中控件的事件
void CMainDlg::OnMenuSliderPos(IEvtArgs *pEvt)
{
    EventSliderPos *pEvt2 = sobj_cast<EventSliderPos>(pEvt);
    SASSERT(pEvt2);
    SSliderBar * pSlider = sobj_cast<SSliderBar>(pEvt->Sender());
    SASSERT(pSlider);
    //注意此处不能调用this->FindChildByXXX，因为pEvt是菜单中的对象，和this不是一个host
    SWindow *pText = pSlider->GetParent()->FindChildByName(L"menu_text");
    SASSERT(pText);
    pText->SetWindowText(SStringT().Format(_T("%d"),pEvt2->nPos));
}


void CMainDlg::OnMatrixWindowReNotify(IEvtArgs *pEvt)
{
    EventRENotify *pEvt2 = sobj_cast<EventRENotify>(pEvt);
    SASSERT(pEvt2);
    if(pEvt2->iNotify != EN_CHANGE)
        return;
    SEdit *pEdit = sobj_cast<SEdit>(pEvt->Sender());
    SASSERT(pEdit);
    
    SStringW strValue = S_CT2W(pEdit->GetWindowText());
    
    SWindow *pMatrixWnd = FindChildByName(L"matrix_test");
    SASSERT(pMatrixWnd);
    
    if(SStringW(L"edit_rotate") == pEvt->NameFrom())
    {
        pMatrixWnd->SetAttribute(L"rotate",strValue);
    }else if(SStringW(L"edit_skew") == pEvt->NameFrom())
    {
        pMatrixWnd->SetAttribute(L"skew",strValue);
    }else if(SStringW(L"edit_scale") == pEvt->NameFrom())
    {
        pMatrixWnd->SetAttribute(L"scale",strValue);
    }else if(SStringW(L"edit_translate") == pEvt->NameFrom())
    {
        pMatrixWnd->SetAttribute(L"translate",strValue);
    }
}

//演示从XML字符串动态创建子窗口，使用subscribeEvent来响应创建出来的控件的事件，这里不做演示
void CMainDlg::OnBtnCreateChildren()
{
    SRichEdit *pEdit = FindChildByID2<SRichEdit>(R.id.edit_xml);
    SASSERT(pEdit);
    SStringW strXml = S_CT2W(pEdit->GetWindowText());
    SWindow *pContainer = FindChildByID(R.id.wnd_container);
    SASSERT(pContainer);
    //remove all children at first.
    SWindow *pChild = pContainer->GetWindow(GSW_FIRSTCHILD);
    while(pChild)
    {
        SWindow *pNext = pChild->GetWindow(GSW_NEXTSIBLING);
        pChild->Destroy();
        pChild = pNext;
    }
    //using SWindow::CreateChildren to Create Children described in the input xml string.
    pContainer->CreateChildrenFromXml(strXml);
}

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

void CMainDlg::OnBtnSkin()
{
	CSetSkinWnd pSetSkinWnd;
	pSetSkinWnd.DoModal(m_hWnd);
}

void CMainDlg::OnBtnTip()
{
	SWindow *pBtn = FindChildByID(R.id.btn_tip);
	if (pBtn)
	{
		CRect rc = pBtn->GetWindowRect();
		ClientToScreen2(&rc);
		STipWnd::ShowTip(rc.right, rc.top, STipWnd::AT_LEFT_BOTTOM, _T("欢迎使用SOUI!\n如果有好的demo欢迎发送截图给作者，SOUI2基于MIT协议,SOUI4使用自定义协议,商用收费!\n启程软件"));
	}
}

void CMainDlg::OnBtnOpenWrapContent()
{
	SHostDialog dlgWrapContent(_T("layout:dlg_wrap_content"));
	dlgWrapContent.DoModal(m_hWnd);
}

void CMainDlg::OnTimer(UINT_PTR idEvent)
{
	SetMsgHandled(FALSE);
	if(idEvent==TIMER_QUIT)
	{
		SNativeWnd::KillTimer(idEvent);
		PostQuitMessage(-3);
	}else if(idEvent == TIMER_SOUI4)
	{
		SWindow *pAniHost = FindChildByName(L"wnd_ani_host");
		if (pAniHost && pAniHost->IsVisible(TRUE))
		{
			IAnimation *pAni = SApplication::getSingletonPtr()->LoadAnimation(_T("anim:love"));
			if(pAni)
			{
				const WCHAR * kLoveXml= L"<include src=\"LAYOUT:xml_love\"/>";
				BOOL bLoad= pAniHost->CreateChildrenFromXml(kLoveXml);
				if(bLoad)
				{
					SWindow *pLove = pAniHost->GetWindow(GSW_LASTCHILD);
					pAniHost->UpdateLayout();
					pLove->SetUserData(TIMER_SOUI4);
					pAni->setStartOffset(rand()%100);//random delay max to 100 ms to play the animation.
					pLove->SetAnimation(pAni);
				}
				pAni->Release();
			}
		}
	}
}

void CMainDlg::OnCbxInterpolotorChange(IEvtArgs *e)
{
	EventCBSelChange *e2=sobj_cast<EventCBSelChange>(e);
	SComboBox *pCbx = sobj_cast<SComboBox>(e2->Sender());
	if(e2->nCurSel!=-1)
	{
		SStringT str = pCbx->GetLBText(e2->nCurSel);
		str=str.Mid(1,str.GetLength()-1-strlen("Interpolator"));
		IInterpolator * pInterpolator = CREATEINTERPOLATOR(S_CT2W(str));
		if(pInterpolator)
		{
			SInterpolatorView *pView = FindChildByID2<SInterpolatorView>(R.id.view_interpolator);
			pView->SetInterpolator(pInterpolator);
			pInterpolator->Release();
		}
	}
}

void CMainDlg::OnEventPath(IEvtArgs *e)
{
	EventPath * e2 = sobj_cast<EventPath>(e);
	SStringT strLen = SStringT().Format(_T("%.2f"),e2->fLength);
	FindChildByID(R.id.txt_path_length)->SetWindowText(strLen);
}

void CMainDlg::OnInitGroup(IEvtArgs *e)
{
	EventGroupListInitGroup *e2 = sobj_cast<EventGroupListInitGroup>(e);
	SToggle *pTgl = e2->pItem->FindChildByID2<SToggle>(R.id.tgl_switch);
	pTgl->SetToggle(!e2->pGroupInfo->bCollapsed);
	e2->pItem->FindChildByID(R.id.txt_label)->SetWindowText(e2->pGroupInfo->strText);
}

void CMainDlg::OnInitItem(IEvtArgs *e)
{
	EventGroupListInitItem *e2 = sobj_cast<EventGroupListInitItem>(e);
	e2->pItem->FindChildByID(R.id.txt_label)->SetWindowText(e2->pItemInfo->strText);
	e2->pItem->FindChildByID2<SImageWnd>(R.id.img_indicator)->SetIcon(e2->pItemInfo->iIcon);
}

void CMainDlg::OnGroupStateChanged(IEvtArgs *e)
{
	EventGroupStateChanged *e2 = sobj_cast<EventGroupStateChanged>(e);
	SToggle *pTgl = e2->pItem->FindChildByID2<SToggle>(R.id.tgl_switch);
	pTgl->SetToggle(!e2->pGroupInfo->bCollapsed);

}


void CMainDlg::OnCtrlPageClick(IEvtArgs *e)
{
	EventGroupListItemCheck *e2=sobj_cast<EventGroupListItemCheck>(e);
	STabCtrl *pTabOp = FindChildByID2<STabCtrl>(R.id.tab_ctrls);
	int nIndex = e2->pItemInfo->id - R.id.page_mclistview;
	pTabOp->SetCurSel(nIndex);

}

void CMainDlg::OnMcLvHeaderRelayout(IEvtArgs * e)
{
	SHeaderCtrl *pHeader = sobj_cast<SHeaderCtrl>(e->Sender());
	int nItems = pHeader->GetItemCount();
	if (nItems > 1)
	{
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

void CMainDlg::OnBtnCreateByTemp()
{
	SWindow *pContainer = FindChildByName(L"wnd_temp_host");
	SWindow *pInput = FindChildByName(L"re_temp_input");
	if(pContainer && pInput)
	{
		SStringT strInput = pInput->GetWindowText();
		pContainer->CreateChildrenFromXml(S_CT2W(strInput));
	}
}

void CMainDlg::On3dViewRotate(IEvtArgs *e)
{
	EventSwndStateChanged *e2 = sobj_cast<EventSwndStateChanged>(e);
	if(EventSwndStateChanged_CheckState(e2,WndState_Check))
	{
		SWindow *p3dView = FindChildByName("3d_test");
		if(p3dView) p3dView->SetAttribute(L"rotateDir",e2->sender->GetName());
	}
}

void CMainDlg::OnSetPropItemValue()
{
	SPropertyGrid * pPropGrid = FindChildByID2<SPropertyGrid>(R.id.prop_test);
	SASSERT(pPropGrid);

	SStringW strTarget = S_CT2W(FindChildByID(R.id.prop_target)->GetWindowText());
	SStringW strProp = S_CT2W(FindChildByID(R.id.prop_prop)->GetWindowText());
	SStringW strValue = S_CT2W(FindChildByID(R.id.prop_value)->GetWindowText());

	IPropertyItem *pItem = pPropGrid->FindItemByName(strTarget);
	if(pItem)
	{
		pPropGrid->SetItemAttribute(pItem,strProp,strValue);
	}else
	{
		SMessageBox(m_hWnd,_T("target item not found!"),_T("error"),MB_OK|MB_ICONSTOP);
	}
}

void CMainDlg::InitSoui3Animation()
{
	SWindow *pWnd = FindChildByName(L"img_soui");
	if (pWnd)
	{
		IAnimation *pAni = SApplication::getSingletonPtr()->LoadAnimation(_T("anim:rotate"));
		if(pAni)
		{
			pWnd->SetAnimation(pAni);
			pAni->Release();
		}
	}
	SNativeWnd::SetTimer(TIMER_SOUI4,1000);	//start timer.
}


void CMainDlg::OnAnimationStop(IEvtArgs *e)
{
	SWindow *pSender = sobj_cast<SWindow>(e->Sender());
	if(pSender && pSender->GetUserData()==TIMER_SOUI4){
		pSender->Destroy();
	}
}


void CMainDlg::OnToggleLeft(IEvtArgs *e)
{
	SToggle *pToggle = sobj_cast<SToggle>(e->Sender());
	SASSERT(pToggle);
	SWindow *pWnd = FindChildByName(L"pane_left");
	if(!pWnd)
		return;
	if(pToggle->GetToggle())
	{
		IAnimation *pAni = SApplication::getSingletonPtr()->LoadAnimation(_T("anim:slide_show"));
		if(pAni)
		{
			pWnd->SetAnimation(pAni);
			pAni->Release();
		}
	}else
	{
		IAnimation *pAni = SApplication::getSingletonPtr()->LoadAnimation(_T("anim:slide_hide"));
		if(pAni)
		{
			pWnd->SetAnimation(pAni);
			pAni->Release();
		}
	}
}

void CMainDlg::OnSouiClick()
{
	IValueAnimator * pAni = SApplication::getSingletonPtr()->LoadValueAnimator(_T("valueAni:colorAni"));
	if(pAni)
	{
		pAni->addListener(this);
		pAni->addUpdateListener(this);
		pAni->start(this);
	}
}

void CMainDlg::OnSetHostAnimation()
{
	IAnimation *pAni = SApplication::getSingletonPtr()->LoadAnimation(_T("anim:anihost"));
	if (pAni)
	{
		StartHostAnimation(pAni);
		pAni->Release();
	}
}

void CMainDlg::onAnimationEnd(IValueAnimator * pAnimator)
{
	SWindow *pTst = FindChildByName(L"tree_test");
	if(pTst)
	{
		pTst->SetAttribute(L"colorBkgnd",L"RGBA(255,255,255,0)");//set invalid colorBkgnd
		pAnimator->Release();
	}
}

void CMainDlg::onAnimationUpdate(IValueAnimator *pAnimator)
{
	SWindow *pTst = FindChildByName(L"tree_test");
	if(pTst)
	{
		SColorAnimator *ani = sobj_cast<SColorAnimator>(pAnimator);
		if(ani)
		{
			SStringW strColor;
			SColor cr(ani->getValue());
			strColor.Format(L"RGBA(%d,%d,%d,%d)",cr.r,cr.g,cr.b,cr.a);
			pTst->SetAttribute(L"colorBkgnd",strColor);
		}
	}
}

void CMainDlg::OnShellTrayNotify(IEvtArgs * e)
{
	EventTrayNotify *e2 = sobj_cast<EventTrayNotify>(e);
	SShellTray *pTray = sobj_cast<SShellTray>(e->Sender());
	switch(e2->lp)
	{
	case WM_LBUTTONDOWN:
		if(IsWindowVisible())
		{
			ShowWindow(SW_HIDE);
		}else
		{
			ShowWindow(SW_SHOW);
			if(IsIconic())
			{
				OnRestore();
			}
		}
		break;
	case WM_RBUTTONDOWN:
		{
			SMenuEx tmenuex;
			if (tmenuex.LoadMenu(_T("SMENUEX:menu_tray")))
			{
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(m_hWnd);
				tmenuex.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
			}
		}
		break;
	}
}

void CMainDlg::OnGetCaret(IEvtArgs* e)
{
	EventGetCaret *e2=sobj_cast<EventGetCaret>(e);
	e2->strCaret->Assign(L"<caret color=\"rgb(0,0,255)\" animate=\"true\" fadeTime=\"20\" showTime=\"10\" interpolator=\"Accelerate\"/>");
}
