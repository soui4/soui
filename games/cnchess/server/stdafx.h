// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540001__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers


// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>
// Local Header Files
#define ASSERT	assert


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


#endif//AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540001__INCLUDED_