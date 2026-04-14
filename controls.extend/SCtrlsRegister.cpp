#include "stdafx.h"
#include "SCtrlsRegister.h"
#include "STabCtrl2.h"
#include "SButtonEx.h"
#include "SImageMaskWnd.h"
#include "SFreeMoveWindow.h"
#include "SVscrollbar.h"
#include "SSkinNewScrollBar.h"
#include "gif/SSkinAni.h"
#include "gif/SGifPlayer.h"
#include "SScrollText.h"
#include "ExtendSkins.h"
#include "SRatingBar.h"
#include "SListCtrlEx.h"
#include "SIPAddressCtrl.h"
#include "STurn3DView.h"
#include "SRadioBox2.h"
#include "SMcListViewEx/SHeaderCtrlEx.h"
#include "SMcListViewEx/SMCListViewEx.h"
#include "SGroupList.h"
#include "SChromeTabCtrl.h"
#include "SScrollText.h"
#include "SHexEdit.h"
#include "hellocharts/SHelloCharts.h"
#include "propgrid/SPropertyGrid.h"
#include "gridctrl/SGridCtrl.h"
#include "SLightButton.h"
#include "SAniWindow.h"
#include "SCheckBox2.h"
#include "SWaveProgress.h"
#include "SImageBtnEx.h"
#include "SImgCanvas.h"
#include "SEdit2.h"
#include "SVerificationCodeEdit.h"
#include "SChatEdit.h"
#include "SRoundImage.h"
#include "SRoundWnd.h"
#include "SImageKnob.h"
#include "SFadeFrame.h"
#include "SCalendar2.h"
#include "SColorizeText.h"
#include "SLrcView.h"
#ifdef _WIN32
#include "SWkeWebkit.h"
#include "SStaticGdip.h"
#include "SIECtrl.h"
#endif//_WIN32
SNSBEGIN

void SCtrlsRegister::RegisterCtrls(SApplication *theApp)
{
	theApp->RegisterWindowClass<STabPage2>();//STabPage2
	theApp->RegisterWindowClass<STabCtrl2>();//STabCtrl2
	theApp->RegisterWindowClass<SHeaderCtrlEx>();//STabCtrl2
	theApp->RegisterWindowClass<SMCListViewEx>();
	theApp->RegisterWindowClass<SChromeTabCtrl>();//ChromeTabCtrl
	theApp->RegisterWindowClass<SImageMaskWnd>();//SImageMaskWnd
	theApp->RegisterWindowClass<SButtonEx>();
	theApp->RegisterWindowClass<SGifPlayer>();//gif
	theApp->RegisterWindowClass<SRatingBar>();
	theApp->RegisterWindowClass<SIPAddressCtrl>();
	theApp->RegisterWindowClass<STurn3dView>();
	theApp->RegisterWindowClass<SRadioBox2>();
	theApp->RegisterWindowClass<SGroupList>();
	theApp->RegisterWindowClass<SScrollText>();
	theApp->RegisterWindowClass<SHexEdit>();
	theApp->RegisterWindowClass<SPropertyGrid>();
	theApp->RegisterWindowClass<SGridCtrl>();
	theApp->RegisterWindowClass<SLightButton>();
	theApp->RegisterWindowClass<SAniWindow>();
	theApp->RegisterWindowClass<SCheckBox2>();
	theApp->RegisterWindowClass<SWaveProgress>();
	theApp->RegisterWindowClass<SImageBtnEx>();
	theApp->RegisterWindowClass<SImgCanvas>();
	theApp->RegisterWindowClass<SEdit2>();
	theApp->RegisterWindowClass<SVerificationCodeEdit>();//SVerificationCodeEdit

    theApp->RegisterWindowClass<SChatEdit>();      // 注册ChatEdit
    theApp->RegisterWindowClass<SRoundImage>();
    theApp->RegisterWindowClass<SRoundWnd>();
    theApp->RegisterWindowClass<SImageKnob>();
    theApp->RegisterWindowClass<SFadeFrame>(); // 注册渐显隐动画控件
    theApp->RegisterWindowClass<SCalendar2>(); // 注册SCalendar2

	theApp->RegisterWindowClass<SColorizeText>();
	theApp->RegisterWindowClass<SLrcView>();
#ifdef _WIN32
    theApp->RegisterWindowClass<SWkeWebkit>(); // 注册WKE浏览器
    theApp->RegisterWindowClass<SStaticGdip>();
#ifndef __MINGW32__
    theApp->RegisterWindowClass<SIECtrl>(); // 注册IECtrl
#endif//__MINGW32__
#endif// _WIN32
	// Register HelloCharts controls
	SHelloChartsRegister::RegisterControls(theApp);

	//extened skins
	theApp->RegisterSkinClass<SColorMask>();
	theApp->RegisterSkinClass<SSkinVScrollbar>();
	theApp->RegisterSkinClass<SSkinNewScrollbar>();
	theApp->RegisterSkinClass<SSkinAni>();
}


SNSEND