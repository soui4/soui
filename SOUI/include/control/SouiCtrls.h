#ifndef __SOUICTRLS__H__
#define __SOUICTRLS__H__
#include <core/SPanel.h>
#include <control/SCmnCtrl.h>
#include <control/SCaption.h>
#include <control/SRealWnd.h>
#include <control/STabCtrl.h>
#include <control/SEdit.h>
#include <control/SComboBox.h>
#include <control/SComboView.h>
#include <control/SSliderBar.h>
#include <control/SSplitWnd.h>
#include <control/STreeCtrl.h>
#include <control/SScrollbar.h>
#include <control/SHeaderCtrl.h>
#include <control/SListCtrl.h>
#include <control/SListbox.h>
#include <control/SHotKeyCtrl.h>
#include <control/SCalendar.h>
#include <control/SSpinButtonCtrl.h>
#include <control/SListView.h>
#include <control/SMCListView.h>
#include <control/STileView.h>
#include <control/STreeView.h>
#include <control/SDateTimePicker.h>
#include <control/SFrame.h>
#include <control/SStackView.h>
#include <control/SRichEdit.h>
#include <control/SMenuBar.h>
#include <control/SSwitch.h>
#include <control/SSearchDropdownList.h>
#include <control/SToolBar.h>
#include <control/SDockBar.h>
#include <control/SDockFloatWnd.h>
#include <control/SRebar.h>
#if defined(_WIN32) && !defined(FREERTOS_HOST)
// SActiveX.h needs the system <mshtmhst.h> (no swinx shim); the freertos
// simulator builds without the ActiveX hosting path.
#include <control/SActiveX.h>
#endif /**< _WIN32 */

#endif /**< __SOUICTRLS__H__ */