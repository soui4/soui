/**
* Copyright (C) 2014-2050
* All rights reserved.
*
* @file       MainDlg_Misc.cpp
* @brief      CMainDlg 零散演示功能
* @version    v2.0
* @author     SOUI group
* @date       2014/08/15
*
* Describe    收纳无法归入其他主题文件的功能演示:
*               - 磁吸子窗口(OnBtnLRC)     :把一个 SHostWnd 子窗口吸附到主窗口四边
*               - GIF 播放器(OnBtnSelectGIF):运行期选择 gif 文件并播放
*               - 自绘菜单(OnBtnMenu)     :SMenu 弹出菜单
*               - 消息框(OnBtnMsgBox)     :SMessageBox 的按钮组合与 3 秒强退
*               - 气泡提示(OnBtnTip)      :STipWnd 定点气泡
*               - 控件显隐(OnBtnHideTest) :btn_display 的可见性切换
*               - 矩阵变换(OnMatrixWindowReNotify):SMatrixWindow 的
*                 rotate/skew/scale/translate 四个属性由输入框实时驱动
*               - 3D 视图(On3dViewRotate) / 属性表(OnSetPropItemValue)
*               - 插值器(OnCbxInterpolotorChange) / 路径视图(OnEventPath)
*               - 速度表(OnSpeedInc/OnSpeedDec) / 菜单内控件(OnMenuSliderPos)
*/

#include "stdafx.h"
#include "MainDlg.h"
#include "helper/SMenu.h"
#include <controls.extend/FileHelper.h>
#include <controls.extend/SSpeedMeter.h>
#include "SMatrixWindow.h"

#define kLogTag "miscdlg"

/**
* @class      SSkiaTestWnd
* @brief      演示用宿主子窗口(供 OnBtnLRC 创建)
*
* Describe    重写 OnFinalMessage:窗口销毁流程走到此函数时,原生气已全部清理,
*             此处 delete this 实现对象自管理,调用方 new 之后无需再关心释放。
*/
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

/**
* @brief      磁吸子窗口演示(歌词浮窗效果)
*
* Describe    每点击一次按顺时针轮换一种吸附模式:
*               顶部左对齐 -> 底部左对齐 -> 左侧上对齐 -> 右侧上对齐
*             CMagnetFrame::AddSubWnd 让子窗口跟随主窗口移动/缩放,模拟
*             音乐播放器歌词窗口贴附主界面的效果。
*/
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

/**
* @brief      选择并播放 GIF 文件
*
* Describe    运行期通过标准文件对话框选择 gif,交给 SGifPlayer 播放,
*             演示扩展控件 SGifPlayer 的动态加载能力。
*/
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

/**
* @brief      弹出自绘菜单
*
* Describe    SMenu 从资源包加载菜单模板(SMenu:menu_test)后在光标处弹出,
*             菜单项的命令响应统一走 OnCommand。
*/
void CMainDlg::OnBtnMenu()
{
    CPoint pt;
    GetCursorPos(&pt);

    //使用自绘菜单
    SMenu menu;
	menu.LoadMenu(_T("SMenu:menu_test"));
    menu.TrackPopupMenu(0,pt.x,pt.y,m_hWnd,0,GetScale());
}

/**
* @brief      演示控件的显示/隐藏切换
*
* Describe    第二个参数传 TRUE 表示布局时视同"不存在",会影响空间分配,
*             与只挡住绘制不同,这是 DUI 中控制布局流的标准做法。
*/
void CMainDlg::OnBtnHideTest()
{
    SWindow * pBtn = FindChildByName(L"btn_display");
    if(pBtn) pBtn->SetVisible(!pBtn->IsVisible(TRUE),TRUE);
}

/**
* @brief      消息框演示
*
* Describe    依次弹出三种按钮组合的消息框;第三个弹出前启动 TIMER_QUIT
*             3 秒定时器,若用户保持消息框不关闭,应用将被强制退出
*             (见 MainDlg_Animation.cpp 的 OnTimer),演示 SMessageBox 的
*             "无父窗口"静态用法与定时器配合的自动退出策略。
*/
void CMainDlg::OnBtnMsgBox()
{
    SMessageBox(0,_T("this is a message box"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
    SMessageBox(0,_T("this message box includes two buttons"),_T("haha"),MB_YESNO|MB_ICONQUESTION);

	SNativeWnd::SetTimer(TIMER_QUIT,3000,NULL);//3S后退出APP
    SMessageBox(0,_T("this message box includes three buttons. \nthe app will quit after 3 seconds if you keep the msgbox open!"),_T("Alarm"),MB_ABORTRETRYIGNORE|MB_ICONSTOP);
	SNativeWnd::KillTimer(TIMER_QUIT);
}

/**
* @brief      在按钮右上角弹出气泡提示
*
* Describe    STipWnd::ShowTip 是静态接口,在指定屏幕坐标处显示多行文字气泡,
*             AT_LEFT_BOTTOM 表示气泡左下角对齐锚点;缩放参数保证高分屏下
*             气泡大小随界面缩放一致。
*/
void CMainDlg::OnBtnTip()
{
	SWindow *pBtn = FindChildByID(R.id.btn_tip);
	if (pBtn)
	{
		CRect rc = pBtn->GetWindowRect();
		ClientToScreen2(&rc);
		STipWnd::ShowTip(rc.right, rc.top, STipWnd::AT_LEFT_BOTTOM, _T("欢迎使用SOUI!\n如果有好的demo欢迎发送截图给作者，SOUI2基于MIT协议,SOUI4使用自定义协议,商用收费!\n启程软件"),GetRoot()->GetScale());
	}
}

/**
* @brief      矩阵变换演示:输入框联动 SMatrixWindow 属性
* @param      e  富文本通知事件(EVT_RE_NOTIFY)
*
* Describe    四个输入框 edit_rotate/edit_skew/edit_scale/edit_translate 的
*             EN_CHANGE 都路由到本函数,依据事件来源控件名将输入值写入
*             matrix_test 的对应属性,实时观察矩阵变换效果。
*/
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

/**
* @brief      3D 视图旋转轴切换
* @param      e  三个单选按钮(rotate_x/y/z)的状态变化事件
*
* Describe    将选中按钮的名字作为 rotateDir 属性值写入 3d_test,
*             演示"控件名即参数"的简化事件处理模式。
*/
void CMainDlg::On3dViewRotate(IEvtArgs *e)
{
	EventSwndStateChanged *e2 = sobj_cast<EventSwndStateChanged>(e);
	if(EventSwndStateChanged_CheckState(e2,WndState_Check))
	{
		SWindow *p3dView = FindChildByName("3d_test");
		if(p3dView) p3dView->SetAttribute(L"rotateDir",e2->sender->GetName());
	}
}

/**
* @brief      属性表控件设值演示
*
* Describe    从三个输入框取目标对象名/属性名/属性值,调用
*             SPropertyGrid::SetItemAttribute 修改属性表条目,
*             目标条目不存在时给出错误提示。
*/
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

/**
* @brief      插值器下拉框联动演示
* @param      e  SComboBox 的选中变化事件(EventCBSelChange)
*
* Describe    从选中的文本中截出插值器名(形如"1.AccelerateInterpolator"),
*             用 CREATEINTERPOLATOR 工厂宏创建插值器实例并交给
*             SInterpolatorView 播放,直观对比各插值曲线的差异。
*/
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
            SSLOGI() << "CREATEINTERPOLATOR " << str.c_str() << " succeed!";
        }
        else
        {
            SSLOGW() << "CREATEINTERPOLATOR " << str.c_str() << " failed!";
        }
	}
}

/**
* @brief      路径视图测量事件
* @param      e  EventPath 事件,fLength 为路径总长度
*
* Describe    把 SPathView 实时计算出的路径长度刷新到文本控件,
*             演示视图类控件通过自定义事件向外抛数据的方式。
*/
void CMainDlg::OnEventPath(IEvtArgs *e)
{
	EventPath * e2 = sobj_cast<EventPath>(e);
	SStringT strLen = SStringT().Format(_T("%.2f"),e2->fLength);
	FindChildByID(R.id.txt_path_length)->SetWindowText(strLen);
}

/**
* @brief      速度表指针递增
*
* Describe    随机增加 10~59,超出量程由 SSpeedMeter 内部按环形处理。
*/
void CMainDlg::OnSpeedInc()
{
	SSpeedMeter *pSpeedMeter=FindChildByName2<SSpeedMeter>("speed_ctrl");
	if(pSpeedMeter){
		pSpeedMeter->SetValue(pSpeedMeter->GetValue()+rand()%50+10);
	}
}

/**
* @brief      速度表指针递减
*
* Describe    随机减少 10~59,与 OnSpeedInc 对称。
*/
void CMainDlg::OnSpeedDec()
{
	SSpeedMeter *pSpeedMeter=FindChildByName2<SSpeedMeter>("speed_ctrl");
	if(pSpeedMeter){
		pSpeedMeter->SetValue(pSpeedMeter->GetValue()-rand()%50-10);
	}
}

/**
* @brief      菜单内滑杆位置变化
* @param      e  EventSliderPos 事件,来自菜单中的 SSliderBar
*
* Describe    注意:菜单是独立的宿主,这里不能用 this->FindChildByXXX 查找
*             菜单里的控件,必须从事件 Sender 沿控件树向上定位
*             (pSlider->GetParent()->FindChildByName)。
*/
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
