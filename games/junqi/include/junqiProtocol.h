//********************************************************
//	联众乱战四国通讯协议
//	黄建雄 2004.6.30
//********************************************************
#ifndef _JUNQIPROTOCAL_H_	
#define _JUNQIPROTOCAL_H_

#include "protocol.h"

#pragma pack(push,1)

#define PLAYER_COUNT	4		// 四人游戏


#define JUNQI_VERMAJOR		2026	// 新版本军棋存盘主版本号
#define	JUNQI_VERMINOR_L2	0		// 新版本布阵图
#define	JUNQI_VERMINOR_S3	0		// 新版本存盘文件的原始信息
#define	JUNQI_VERMINOR_S4	0		// 新版本包含研究信息的存盘文件版本号

//房间属性索引Property Index
#define PROPERTYSIZE	9	//一共9个属性
#define PI_BOMBFLY		0	//炸弹满天飞
#define PI_MINEASBOMB	1	//炸弹雷
#define PI_PEACESTEPS	2	//和棋步数
#define PI_RULE			3	//游戏模式
#define PI_STEPTICK		4	//单步计时
#define PI_PASS			5	//每局可以待机次数
#define PI_AUTOSAVE		6	//自动存盘
#define PI_GETFLASH		7	//拿子闪烁
#define PI_LAYOUTTICK	8	//布阵计时

#define PROPID_BOMBFLY		PI_BOMBFLY
#define PROPID_MINEASBOMB	PI_MINEASBOMB
#define PROPID_PEACESTEPS	PI_PEACESTEPS
#define PROPID_RULE			PI_RULE
#define PROPID_STEPTICK		PI_STEPTICK
#define PROPID_PASS			PI_PASS
#define PROPID_AUTOSAVE		PI_AUTOSAVE
#define PROPID_GETFLASH		PI_GETFLASH
#define PROPID_LAYOUTTICK	PI_LAYOUTTICK

//游戏规则
#define RULE_NOR		0	//四暗
#define RULE_SMS		1	//双明
#define RULE_QMQ		2	//全明


#define MATWIDTH				17		// 棋盘宽高


// 布局结束，可以开始下棋
// -> Server -> 除自己外所有人(包括旁观者)
#define GLID_JQSTARTFIGHT			(GMT_GAMEBASE+0x00000000)
typedef struct tagMSGSTARTFIGHT {
    int iIndex;
	BYTE byLayout[13];
} MSGSTARTFIGHT;

// 拿起棋子的请求
// -> Server -> 广播
#define GLID_JQGETSTONE				(GMT_GAMEBASE+0x00000001)

// 把拿起的棋子放到目标坐标
// -> Server -> 广播
#define GLID_JQPUTSTONE				(GMT_GAMEBASE+0x00000002)

// 全部布局结束消息
// Server -> 广播
#define GLID_JQALLSTARTED			(GMT_GAMEBASE+0x00000003)
typedef struct tagMSGALLSTART
{
	int		iFstIndex;	//第一个走子的玩家的方位
}MSGALLSTART,*PMSGALLSTART;

// 超时 PASS
// -> Server -> 广播
#define MSG_REQ_PASS				(GMT_GAMEBASE+0x00000008)
typedef struct tagMSG_PASS{
	int iIndex;
}MSG_PASS,*PMSG_PASS;

// 一方投降消息
// -> Server -> 广播
#define GLID_JUNQIGIVEUP			(GMT_GAMEBASE+0x00000009)
#define MSG_REQ_SURRENDER GLID_JUNQIGIVEUP

// 请求投降
// GLID_JUNQIGIVEUP
typedef struct
{
	DWORD	dwCurMan;							// 投降方
}GLREQGIVEUP, *PGLREQGIVEUP;
typedef struct tagMSG_SURRENDER{
	int iIndex;
}MSG_SURRENDER,*PMSG_SURRENDER;


//--------------------------------------------------------------------------
//	玩家走棋消息
//--------------------------------------------------------------------------
#define	GLID_JUNQI_MOVECM					(GMT_GAMEBASE + 0x103)
#define MSG_REQ_MOVE GLID_JUNQI_MOVECM
typedef struct tagMSG_MOVE{
	int 	iIndex;	//当前方
	POINT   ptBegin;	//起始位置
	POINT   ptEnd ;		//目标位置
	uint32_t bSapper;//工兵路线
	int     result;	//结果状态
}MSG_MOVE,* PMSG_MOVE;

#define MSG_REQ_PEACE (GMT_GAMEBASE+0x104)
#define MSG_ACK_PEACE (MSG_REQ_PEACE|MSG_ACK)
typedef struct tagMSG_PEACE{
	int iIndex;	//求和方
	int nResult;	//求和结果, 1-agree, 0-disagree
	char szMsg[100];//求和说明,utf8编码
}MSG_PEACE,*PMSG_PEACE;

//服务器通知客户端游戏结束
#define MSG_ACK_GAMEOVER	((GMT_GAMEBASE+105)|MSG_ACK)
typedef struct tagMSG_GAMEOVER
{
	int iWinner;	//赢棋方，-1代表和棋
	BYTE byLayout[4][13];// 各方布局
	char szDesc[100];//说明,utf8编码
}MSG_GAMEOVER,*PMSG_GAMEOVER;

//服务器通知客户端军旗位置
#define MSG_NOTIFY_FLAGPOS (GMT_GAMEBASE+0x106)
typedef struct tagMSG_FLAGPOS
{
	int iIndex;	//方位
	POINT pt;	//位置
}MSG_FLAGPOS,*PMSG_FLAGPOS;

//服务器通知客户端当前激活方
#define MSG_NOTIFY_PLAYER_ACTIVE (GMT_GAMEBASE+0x107)
typedef struct tagMSG_PLAYERACTIVE
{
	int iIndex;	//方位
}MSG_PLAYERACTIVE,*PMSG_PLAYERACTIVE;

#define MSG_NOTIFY_PLAYER_LOSE (GMT_GAMEBASE+0x108)
typedef struct tagMSG_PLAYERLOSE
{
	int iIndex;	//方位
}MSG_PLAYERLOSE,*PMSG_PLAYERLOSE;

#pragma pack(pop)

#endif//_JUNQIPROTOCAL_H_