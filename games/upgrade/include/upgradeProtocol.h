#ifndef _UPGRADEPROTOCOL_H_
#define _UPGRADEPROTOCOL_H_

#include "protocol.h"

#define PLAYER_COUNT	4

//各种属性索引号,只定义客户端关心的索引 PI=property index
enum PropertyIndex{
	PI_TIME_REPORT = 0	,	//反主等待时间
	PI_TIME_PUTBOTTOM	,	//垫底等待时间
	PI_TIME_REBEL		,	//反底等待时间
	PI_TIME_PUTCARD		,	//出牌等待时间
	PI_RULE_REBEL		,	//是否支持反底规则
	PI_RULE_LEVEL_5_10_K,   //只打5，10，K
	PI_RULE_2_MAIN		,	//2是否为常主
};

#pragma pack(push,1)

//请求发牌:无
#define GMT_UPGRADE_REQCARD		(GMT_GAMEBASE+1)
//服务器发牌消息:25张牌
#define GMT_UPGRADE_POSTCARD	(GMT_GAMEBASE+2)
typedef struct tagGAME_UPGRADE_POSTCARD{
	int nCards[25];
}GAME_UPGRADE_POSTCARD;

//请求亮主:座位+亮主张数+牌值
#define GMT_UPGRADE_REQSHOWMAIN	(GMT_GAMEBASE+3)
typedef struct tagGAME_UPGRADE_SHOWMAIN { 
	int index;	//亮主座位
	int nCount;	//亮主张数
	int nCard[2];	//亮主牌值
} GAME_UPGRADE_SHOWMAIN;

//发底牌:庄家座位+8张底牌
#define GMT_UPGRADE_POSTBOTTOM	(GMT_GAMEBASE+5)
typedef struct tagGAME_UPGRADE_POSTBOTTOM{
	int iOwnerIndex;
	int nBottom[8];	
}GAME_UPGRADE_POSTBOTTOM;

//放弃反底
#define GMT_UPGRADE_REQREBELPASS (GMT_GAMEBASE+6)
//请求垫底消息：底牌拥有者座位+底牌
#define GMT_UPGRADE_REQPUTBOTTOM (GMT_GAMEBASE+7)
typedef struct tagGAME_UPGRADE_REQPUTBOTTOM{
	int ownerIndex;
	int nBottom[8];
}GAME_UPGRADE_REQPUTBOTTOM;

//无人亮主时从底牌中抽一张为主花色:牌值
#define GMT_UPGRADE_AUTOCOLOR	(GMT_GAMEBASE+8)
typedef struct tagGAME_UPGRADE_AUTOCOLOR{
	int iBottomIndex;	//底牌索引
	int nCard;	//主花色牌值
}GAME_UPGRADE_AUTOCOLOR;

//开始出牌消息:当前出牌人位置
#define GMT_UPGRADE_STARTPUTCARD (GMT_GAMEBASE+9)
typedef struct tagGAME_UPGRADE_STARTPUTCARD{
	int winnerIndex;
}GAME_UPGRADE_STARTPUTCARD;
//请求出牌:座位+牌张数+牌数据
#define GMT_UPGRADE_REQPUTCARD	(GMT_GAMEBASE+10)
typedef struct tagGAME_UPGRADE_REQPUTCARD{
	int nIndex;
	int nCount;
	int nCard[25];
}GAME_UPGRADE_REQPUTCARD;

//出牌错误:错误号

#define GMT_UPGRADE_PUTCARDERR	(GMT_GAMEBASE+11)
enum{
PCE_SAMECOLOR	=1,
PCE_FOLLOWCOUNT	=2,
PCE_FOLLOWCOLOR	=3,
PCE_FOLLOWTYPE	=4,
PCE_UNKNOWN		=5,
};
typedef struct tagGAME_UPGRADE_PUTCARDERR{
	int errCode;
}GAME_UPGRADE_PUTCARDERR;

//一轮牌结束:牌大方座位
#define GMT_UPGRADE_TURNOVER	(GMT_GAMEBASE+12)
typedef struct tagGAME_UPGRADE_TURNOVER {
    int winnerIndex;
} GAME_UPGRADE_TURNOVER;

//游戏结束：GS_FINISHINFO
#define GMT_UPGRADE_ROUNDEND	(GMT_GAMEBASE+13)
typedef struct tagGS_ROUNDINFO
{
	int	nMaxIndex;	//最后一轮最大的用户座位
	int nBottom[8];	//底牌
	int nMultiple;	//扣底倍数,0代表庄家扣底
	int nScore;		//总得分
	int nLevelA;	//0号位的新级别
	int nLevelB;	//1号位的新级别
	int nNewBankerIndex;//下一轮的庄家
}GAME_UPGRADE_ROUNDEND;

//甩牌失败：座位+扣分数+出的牌+必出的小牌
#define GMT_UPGRADE_THROWFAILED	(GMT_GAMEBASE+14)
typedef struct tagGAME_UPGRADE_THROWFAILED{
	int nIndex;
	int nDiscount;
	int nPutCount;
	int nPutCard[25];
	int nFailedCount;
	int nFailedCard[25];
}GAME_UPGRADE_THROWFAILED;

//动画礼品
#define GMT_UPGRADE_GIFT		(GMT_GAMEBASE+15)
typedef struct tagGS_GIFTIINFO
{
	int nIdxSelf;
	int nIdxTarget;
	char nRepeat;
}GS_GIFTINFO,*PGS_GIFTINFO;

//聊天
#define GMT_UPGRADE_CHAT		(GMT_GAMEBASE+16)
typedef struct tagGS_CHATINFO
{
	int nIdxSelf;
	int nIdxTarget;
	char szBuf[100];
}GS_CHATINFO,*PGS_CHATINFO;

#pragma pack(pop)

#endif//_UPGRADEPROTOCOL_H_