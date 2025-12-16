#pragma once

#include <souistd.h>
#include <core/SHostWnd.h>
#include <control/SMessageBox.h>
#include <helper/SMenuEx.h>
#include <helper/SMenu.h>
#include <helper/STrayIcon.h>
#include <helper/SHostDialog.h>
#include <helper/findplace.h>
#include <helper/SkinPool.h>
#include <helper/templethelper.hpp>
#include <layout/SLinearLayout.h>
#include <layout/SHBox.h>
#include <layout/SVBox.h>
#include <control/SButton.h>
#include <control/SCheckBox.h>
#include <control/SRadioBox.h>
#include <control/SSliderBar.h>
#include <control/STabCtrl.h>
#include <control/SScrollBar.h>
#include <control/SHeaderCtrl.h>
#include <control/SListCtrl.h>
#include <control/SListBox.h>
#include <control/SComboBox.h>
#include <control/STreeCtrl.h>
#include <control/SMCListView.h>
#include <control/SRichEdit.h>
#include <control/SCalendar.h>
#include <control/SSpinButtonCtrl.h>
#include <control/SActiveX.h>
#include <control/SSplitPane.h>
#include <control/SFlashCtrl.h>
#include <control/SIECtrl.h>
#include <control/SMediaPlayer.h>
#include <control/SChart.h>
#include <control/SWordBreak.h>
#include <gif/SAniImgFrame.h>
#include <gif/SGifPlayer.h>
#include <activex/flash11.tlh>
#include <animator/SInterpolatorImpl.h>
#include <SApp.h>
#include <interface/ws-i.h>
#include <helper/obj-ref-impl.hpp>
#include <com-loader.hpp>

#ifdef _DEBUG
    #define SYS_NAMED_RESOURCE_LUA  _T("soui_named_resource.lua")
#else
    #define SYS_NAMED_RESOURCE_LUA  _T("soui_named_resource.lua")
#endif

using namespace SOUI;