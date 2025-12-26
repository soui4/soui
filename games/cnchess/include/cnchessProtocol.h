#ifndef __GAMEPROTOCOL_H__
#define __GAMEPROTOCOL_H__

#include "protocol.h"

#define PLAYER_COUNT 2

#pragma pack(push,1)
// 象棋游戏消息类型定义

//属性定义
enum tagPROPID
{
	PROPID_REGRET=0,	//悔棋次数
	PROPID_TIME_ROUND,	//局时
	PROPID_TIME_STEP,	//步时,步时与读秒都为0时为定时房间
	PROPID_MAX_STP_JIANG,	//长将步限
	PROPID_MAX_STP_CATCH,	//长捉步限
	PROPID_MAX_STP_PEACE,	//最多不吃子步数
	PROPID_MIN_STEPS,		//每局最小步数，以内不能求和及认输
	PROP_SIZE
};

//初始化消息, GMT_START data
typedef struct tagMSG_INIT
{
    int iRedIndex; // 红方座位
	int layout[10][9];//布局
}MSG_INIT,*PMSG_INIT;



typedef struct tagMSG_MOVE 
{
	POINT ptBegin;	//起点
	POINT ptEnd;	//终点
	uint32_t  bLocal;   //本地消息标志
	int	  iIndex;	//走棋人座位号
	DWORD dwRoundTime;//局时,以秒为单位
}MSG_MOVE,*PMSG_MOVE;
//走棋
#define MSG_REQ_MOVE		((GMT_GAMEBASE+1)|MSG_REQ)


//悔棋,发送悔棋方座位
#define MSG_REQ_REGRET		((GMT_GAMEBASE+3)|MSG_REQ)
typedef struct tagMSG_REGRET
{
	int iIndex;	//悔棋方座位
}MSG_REGRET,*PMSG_REGRET;

//走棋超时,发送座位
#define MSG_REQ_TIMEOUT		((GMT_GAMEBASE+9)|MSG_REQ)
typedef struct tagMSG_TIMEOUT
{
	int iIndex;	//走棋方座位
}MSG_TIMEOUT,*PMSG_TIMEOUT;

//投降请求,发送座位
#define MSG_REQ_SURRENDER	((GMT_GAMEBASE+10)|MSG_REQ)
typedef struct tagMSG_SURRENDER
{
	int iIndex;	//走棋方座位
}MSG_SURRENDER,*PMSG_SURRENDER;

//求和消息
#define MAX_EXPLAIN	100
typedef struct tagMSG_PEACE
{
	int iIndex;	//求和方
	int nResult;	//求和结果, 1-agree, 0-disagree
	char szMsg[MAX_EXPLAIN+1];//求和说明,utf8编码
}MSG_PEACE,*PMSG_PEACE;
#define MSG_REQ_PEACE		((GMT_GAMEBASE+11)|MSG_REQ)
#define MSG_ACK_PEACE		((GMT_GAMEBASE+11)|MSG_ACK)

typedef struct tagMSG_RESULT
{
	int iIndex;	//发送方座位号
	int bWin;	//输赢标志
}MSG_RESULT,*PMSG_RESULT;
//游戏结束,游戏双方在游戏结束时共同发送该消息
#define MSG_REQ_RESULT		((GMT_GAMEBASE+12)|MSG_REQ)


typedef enum tagLC_TYPE{LCT_CATCH=1,LCT_JIANG=2} LC_TYPE;//类型
typedef struct tagMSG_LONGCATCH
{
	int iIndex;		//赖皮方
	LC_TYPE type;//类型
}MSG_LONGCATCH,*PMSG_LONGCATCH;
//报告对方长捉or长将
#define MSG_REQ_LONGCATCH	((GMT_GAMEBASE+13)|MSG_REQ)

typedef struct tagMSG_GAMEOVER
{
	int iWinner;	//赢棋方，-1代表和棋
	char szDesc[100];//说明,utf8编码
}MSG_GAMEOVER,*PMSG_GAMEOVER;
//服务器通知客户端游戏结束
#define MSG_ACK_GAMEOVER	((GMT_GAMEBASE+14)|MSG_ACK)

typedef struct tagMSG_EXCHANGE
{
	int iIndex;		//请求交换方
	int nResult;	//用户回应
	char szMsg[MAX_EXPLAIN+1];//请求说明,utf8编码
}MSG_EXCHANGE,*PMSG_EXCHANGE;
//交换颜色
#define MSG_REQ_EXCHANGE		((GMT_GAMEBASE+15)|MSG_REQ)
#define MSG_ACK_EXCHANGE		((GMT_GAMEBASE+15)|MSG_ACK)

typedef struct tagMSG_PROTOCOL
{
	int	nTimeRound;		//总时间
	int nTimeStep;		//单步时间
	int nTimeExtra;		//读秒
	int nTimeMakeup;	//每步加时
}MSG_PROTOCOL,*PMSG_PROTOCOL;
//协商时间设置
#define MSG_REQ_PROTOCOL	((GMT_GAMEBASE+20)|MSG_REQ)

#pragma pack(pop)

#endif//__GAMEPROTOCOL_H__