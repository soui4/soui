#ifndef __GAMEPROTOCOL_H__
#define __GAMEPROTOCOL_H__

#include "protocol.h"

#define PLAYER_COUNT 2

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
	PROPID_EXCHANGABLE,		//可交换颜色
	PROP_SIZE
};


typedef struct tagMSG_MOVE 
{
	POINT ptBegin;	//起点
	POINT ptEnd;	//终点
	BOOL  bLocal;   //本地消息标志
	int	  nSeat;	//走棋人座位号
	DWORD dwRoundTime;//局时,以秒为单位
}MSG_MOVE,*PMSG_MOVE;
//走棋
#define MSG_REQ_MOVE		((GMT_GAMEBASE+1)|MSG_REQ)


//悔棋,发送悔棋方座位
#define MSG_REQ_REGRET		((GMT_GAMEBASE+3)|MSG_REQ)

//走棋超时,发送座位
#define MSG_REQ_TIMEOUT		((GMT_GAMEBASE+9)|MSG_REQ)

//投降请求,发送座位
#define MSG_REQ_SURRENDER	((GMT_GAMEBASE+10)|MSG_REQ)


//求和消息
#define MAX_EXPLAIN	100
typedef struct tagMSG_PEACE
{
	int nSeat;	//求和方
	int nResult;	//求和结果
	char szMsg[MAX_EXPLAIN+1];//求和说明,utf8编码
}MSG_PEACE,*PMSG_PEACE;
#define MSG_REQ_PEACE		((GMT_GAMEBASE+11)|MSG_REQ)
#define MSG_ACK_PEACE		((GMT_GAMEBASE+11)|MSG_ACK)

typedef struct tagMSG_RESULT
{
	int nSeat;	//发送方座位号
	int bWin;	//输赢标志
	int iGameID;//当前游戏ID，客户端在收到SETCOLOR时获得并保存
}MSG_RESULT,*PMSG_RESULT;
//游戏结束,游戏双方在游戏结束时共同发送该消息
#define MSG_REQ_RESULT		((GMT_GAMEBASE+12)|MSG_REQ)


typedef enum tagLC_TYPE{LCT_CATCH=1,LCT_JIANG=2} LC_TYPE;//类型
typedef struct tagMSG_LONGCATCH
{
	int nSeat;		//赖皮方
	LC_TYPE type;//类型
}MSG_LONGCATCH,*PMSG_LONGCATCH;
//报告对方长捉or长将
#define MSG_REQ_LONGCATCH	((GMT_GAMEBASE+13)|MSG_REQ)

typedef struct tagMSG_GAMEOVER
{
	int nWinSeat;	//赢棋方，-1代表和棋
	char szDesc[100];//说明,utf8编码
}MSG_GAMEOVER,*PMSG_GAMEOVER;
//服务器通知客户端游戏结束
#define MSG_ACK_GAMEOVER	((GMT_GAMEBASE+14)|MSG_ACK)

typedef struct tagMSG_EXCHANGE
{
	int nSeat;		//请求交换方
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


typedef struct tagMSG_SETCOLOR
{
	int nRedSeat;//红方座位
	int iGameID;//游戏ID，游戏结束时，由客户端上传结果及ID号,以保证与服务器的ID匹配
}MSG_SETCOLOR,*PMSG_SETCOLOR;
//设置玩家颜色
#define MSG_ACK_SETCOLOR	((GMT_GAMEBASE+21)|MSG_ACK)

//计时消息,每隔5秒计一次
#define TIMETICK_INTER		5
#define MSG_REQ_TIMETICK	((GMT_GAMEBASE+22)|MSG_REQ)

// 玩家养成数值
#define MSG_REQ_GROWVALUE	((GMT_GAMEBASE+23)|MSG_REQ)

typedef struct tagMSG_GROWVALUE
{
	int anValue[ 2 ][ 7 ];
}MSG_GROWVALUE,*PMSG_GROWVALUE;

//滚动消息，消息长度可变。
#define MSG_ACK_TIPMSG		((GMT_GAMEBASE+24)|MSG_ACK)

// 再来一局
#define MSG_REQ_ONCEMORE		((GMT_GAMEBASE+25)|MSG_REQ)
#define MSG_ACK_ONCEMORE		((GMT_GAMEBASE+25)|MSG_ACK)

typedef struct tagMsgReqOnceMore
{
	int nSeat;
	BOOL bCancel;	// 是否取消约战
}MSGREQONCEMORE, *PMSGREQONCEMORE;
typedef struct tagMsgAckOnceMore
{
	int nSeat;
	BOOL bAgree;	// 是否同意
}MSGACKONCEMORE, *PMSGACKONCEMORE;


#endif//__GAMEPROTOCOL_H__