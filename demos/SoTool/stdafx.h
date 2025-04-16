// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/SMessageBox.h>
#include <control/SouiCtrls.h>
#include <res.mgr/SObjDefAttr.h>
#include <helper/SMenu.h>
#include <helper/SMenuEx.h>


#include "resource.h"

#include "SEdit2.h"
#include "SImgCanvas.h"
#include "SFolderList.h"

using namespace SOUI;


#define IIF_STATE2(the_state, normal_value, hover_value) \
    (((the_state) & SOUI::WndState_Hover) ? (hover_value) : (normal_value))

#define IIF_STATE3(the_state, normal_value, hover_value, pushdown_value) \
    (((the_state) & (SOUI::WndState_PushDown|SOUI::WndState_Check)) ? (pushdown_value) : IIF_STATE2(the_state, normal_value, hover_value))

#define IIF_STATE4(the_state, normal_value, hover_value, pushdown_value, disable_value) \
    (((the_state) & SOUI::WndState_Disable) ? (disable_value) : IIF_STATE3(the_state, normal_value, hover_value, pushdown_value))

