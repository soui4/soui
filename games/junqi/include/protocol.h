#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <stdint.h>

#define MAX_PROP			20	//服务器配置文件的属性最大值

enum tagSEXTYPE{SEX_MALE=0,SEX_FEMALE,SEX_SECRET};

typedef enum tagTableState
{
    TABLE_STATE_WAIT = 0,
    TABLE_STATE_PLAYING,
    TABLE_STATE_FINISH,
}TableState;


typedef enum tagGAMEOVERTYPE{
	GOT_NORMAL=0,	//正常结束
	GOT_NETBREAK,	//断线
	GOT_SURRENDER,	//投降
	GOT_PEACE,		//求和
	GOT_TIMEOUT,	//超时
	GOT_CHEAT,		//作弊
	GOT_UNDEFINED,	//非正常结束
}GAMEOVERTYPE;


#pragma pack(push,1)


typedef struct tagGAMEMSG
{
	DWORD	dwLen;		//后面的数据长度，不包括本身
	DWORD	dwType;
	BYTE	byData[1];
}GAMEMSG,*PGAMEMSG;
#define MAX_MSGSIZE			(1<<18)		//消息数据最大值，不能超过256K

#define MAX_USERNAME_LEN 50

typedef struct tagGS_USERINFO
{
	uint32_t uid;	//用户ID, login时分配
	char szName[MAX_USERNAME_LEN];//玩家名称,utf8
	int	nSex;		//性别
}GS_USERINFO;


//用户登录请求，上传用户信息及头像数据
#define GMT_LOGIN_REQ			100
typedef struct tagGAME_LOGIN_REQ : GS_USERINFO{
	DWORD dwLen;
	BYTE byData[1];
}GAME_LOGIN_REQ;

//游戏登录应答
#define GMT_LOGIN_ACK			101
typedef struct tagGAME_LOGIN_ACK
{
	uint32_t uid;	//用户ID
	uint32_t errCode;	//错误码
	DWORD    dwProps[MAX_PROP];
}GAME_LOGIN_ACK;

#define ERR_SUCCESS			0
#define ERR_USER_EXIST		1

typedef struct SeatID
{
	int nTableId;	//游戏桌ID
	int nSeat;		//游戏座位
}SeatID;


//用户在线消息
#define GMT_ONLINE			102
typedef struct tagGAME_ONLINE
{
	GS_USERINFO	stUserInfo;//用户信息
}GAME_ONLINE;

//用户坐下消息
#define GMT_SEATDOWN_REQ		103
typedef SeatID GAME_SEATDOWN_REQ;
//用户坐下消息ACK, 只发送给请求的用户
#define GMT_SEATDOWN_ACK		104
typedef struct tagGAME_SEATDOWN_ACK {
    int nTableId; // 游戏桌ID
    int nSeat;    // 游戏座位
	uint32_t bSuccess;
}GAME_SEATDOWN_ACK;

#define GMT_GETUP_REQ			105
typedef SeatID GAME_GETUP_REQ;

#define GMT_TABLE_INFO		106

typedef struct tagSEAT_INFO
{
    int nIndex;             // 游戏座位索引
	uint32_t bReady;			// 是否已准备
	GS_USERINFO stUserInfo;	//用户信息
}SEAT_INFO;
typedef struct tagGAME_TABLE_INFO
{
	int nTableId;	//游戏桌ID
	int nPlayers;	//人数
	int tableState;//TableState
	SEAT_INFO seatInfo[1];
}GAME_TABLE_INFO;

#define GMT_ROOM_INFO			107
typedef struct tagGAME_ROOM_INFO
{
	int nTableCount;	//游戏桌数量
	GAME_TABLE_INFO tableInfo[1];
}GAME_ROOM_INFO;

//请求头像
#define GMT_AVATAR_REQ			108
typedef struct tagGAME_AVATAR_REQ
{
	int nTableId;	//游戏桌ID
	uint32_t uid;	//用户ID
}GAME_AVATAR_REQ;
//返回头像数据
#define GMT_AVATAR_ACK			109
typedef struct tagGAME_AVATAR_ACK
{
	uint32_t uid;	//用户ID
	DWORD dwLen;	//后面的数据长度，不包括本身
	BYTE byData[1];
}GAME_AVATAR_ACK;

//玩家就绪
#define GMT_READY				110


//退出游戏
#define GMT_LOGOFF			111
//开始游戏, 告诉所有玩家开始游戏, 不同游戏使用不同的消息结构体
#define GMT_START			112
//结束游戏
#define GMT_END				113
//连接服务器
#define GMT_CONNECT			114
//服务器Socket关闭
#define GMT_SOCKCLOSE		115

//游戏桌内通知消息, utf8
#define GMT_GAME_MSG			116

//其它游戏的消息ID从GMT_GAMEBASE+1开始
#define	GMT_GAMEBASE		1000

#define MSG_REQ 0x00000000
#define MSG_ACK 0x10000000

#pragma pack(pop)

#endif//_PROTOCOL_H_