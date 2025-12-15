// Upgrade.h: interface for the CUpgrade class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UPGRADE_H__F1D4B8AF_9823_45C7_A020_26BCD90E8BB1__INCLUDED_)
#define AFX_UPGRADE_H__F1D4B8AF_9823_45C7_A020_26BCD90E8BB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WebSocketGame.h"
#include "Game.h"
#include <upgradeProtocol.h>

typedef struct tagCLIENTDATA
{
	int nCardCount;//牌数
	int nCards[25+8];//手中牌
    int nCardPut[26];   // 本轮出牌数据,第0个位置保存出牌数量
}CLIENTDATA,*PCLIENTDATA;

typedef enum tagGAMESTATE
{
	GS_WAITSTART=0,	//等待用户开始
	GS_WAITCARD,	//等待请求发牌
	GS_SHOWMAIN,	//亮主阶段
	GS_PUTBOTTOM,	//垫底阶段
	GS_REBELBOTTOM,	//反底阶段
	GS_PUTBOTTOM2,	//反底后的垫底阶段
	GS_PUTCARD,		//出牌阶段
}GAMESTATE;

enum tagREBELCOLOR
{
	RBLCR_DIAMOND=0,	//方块
	RBLCR_CLUB,			//梅花
	RBLCR_HEART,		//红桃
	RBLCR_SPADE,		//黑桃
	RBLCR_SKING,		//小王
	RBLCR_BKING,		//大王
	RBLCR_DENY,			//不能反
	RBLCR_WAIT,			//正在等待
	RBLCR_PASS,			//放弃
};


class CUpgrade: public CGameTable
{
public:
	CUpgrade();
	virtual ~CUpgrade();
  public:
	virtual void OnGameStart() override;
	virtual void OnGameEnd() override;
	virtual void OnRoundEnd() override;
protected:
	void ClearState();
	void InitCard();
	int  NextLevel(int nCurLevel,int nUpLevel);
	int GetLevel();

	void OnReqPutCard(PCLIENT pClient,LPVOID lpData,DWORD dwSize);
	void OnReqRebelPass(PCLIENT pClient,LPVOID lpData,DWORD dwSize);
	void OnReqPutBottom(PCLIENT pClient,LPVOID lpData,DWORD dwSize);
	void OnReqShowMain(PCLIENT pClient,LPVOID pData,DWORD dwSize);
	void OnReqCard(PCLIENT pClient,LPVOID pData,DWORD dwSize);
	void OnGiftSend(PCLIENT pClient,LPVOID pData , DWORD dwSize);
	void OnChatSend(PCLIENT pClient,LPVOID pData , DWORD dwSize);

	virtual BOOL OnMsg(PCLIENT pClient,DWORD dwType,LPVOID pData , DWORD dwSize) override;
	
	int		m_nCardBottom[8];				//底牌
	GAMESTATE	m_gameState;				//游戏状态
	int		m_nLevelA,m_nLevelB;			//0号和1号位两家的级牌
	int		m_nBankerSeat;					//庄家座位
	GAME_UPGRADE_SHOWMAIN	*	m_pnShowMainInfo;				//当前的亮主信息
	int		m_nShowMainSeat;				//当前亮主方
	int		m_nBottomOwner;					//底牌拥有方
	int		m_nMainColor;					//主花色
	int		m_nTurnFirstSeat;				//本轮首先出牌方
	int		m_nScore;						//闲家得分
};

#endif // !defined(AFX_UPGRADE_H__F1D4B8AF_9823_45C7_A020_26BCD90E8BB1__INCLUDED_)
