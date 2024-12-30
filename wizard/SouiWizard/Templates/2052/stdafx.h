// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

#define  _CRT_SECURE_NO_WARNINGS
[!if DYNAMIC_SOUI]
#define	 DLL_SOUI
[!endif]	
#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/SMessageBox.h>
#include <control/SouiCtrls.h>
#include <res.mgr/SObjDefAttr.h>
#include <com-cfg.h>
#include <SouiFactory.h>
[!if CHECKBOX_SHELLNOTIFYICON]
#include "trayicon/SShellTray.h"
[!endif]
#include "resource.h"
#define R_IN_CPP	//定义这个开关来
#include "res\resource.h"
using namespace SOUI;

