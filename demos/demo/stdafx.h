// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#ifndef __STDAFX_H_
#define __STDAFX_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#define OUTLOG_LEVEL	2	//large than 1 to disable OutputDebugString from soui system log when not use log4z module.
#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/SMessageBox.h>
#include <control/SouiCtrls.h>
#include <res.mgr/SObjDefAttr.h>
#include <event/SNotifyCenter.h>

#include "resource.h"	//APP资源
#define R_IN_CPP
#include "res/resource.h"

#include <controls.extend/SWkeWebkit.h>
#include <controls.extend/gif/SSkinAni.h>
#include <controls.extend/gif/SGifPlayer.h>
#include <controls.extend/SIPAddressCtrl.h>
#include <controls.extend/SImageMaskWnd.h>
#include <controls.extend/SRatingBar.h>
#include <controls.extend/SChromeTabCtrl.h>
#include <controls.extend/smiley/SSmileyCtrl.h>
#include <controls.extend/SChatEdit.h>
#include <controls.extend/SFreeMoveWindow.h>
#include <controls.extend/TipWnd.h>
#include <controls.extend/SProgressRing.h>
#include <controls.extend/SGroupList.h>
#include <controls.extend/SAniWindow.h>
#include <controls.extend/propgrid/SPropertyGrid.h>
#include <controls.extend/SCheckBox2.h>

#include "SInterpolatorView.h"
#include "SPathView.h"

using namespace SOUI;

#endif//__STDAFX_H_

