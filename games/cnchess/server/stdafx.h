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
#include <helper/slog.h>

// 象棋游戏消息类型定义
#define CHESS_MSG_MOVE          1001  // 走棋消息
#define CHESS_MSG_UNDO_REQUEST  1002  // 悔棋请求
#define CHESS_MSG_DRAW_REQUEST  1003  // 和棋请求
#define CHESS_MSG_SURRENDER     1004  // 认输消息
#define CHESS_MSG_CHAT          1005  // 聊天消息
#define CHESS_MSG_JOIN_ROOM     1006  // 加入房间
#define CHESS_MSG_LEAVE_ROOM    1007  // 离开房间
#define CHESS_MSG_READY         1008  // 准备就绪
#define CHESS_MSG_GAME_START    1009  // 游戏开始
#define CHESS_MSG_GAME_END      1010  // 游戏结束

using namespace SOUI;