// Upgrade.cpp: implementation of the CUpgrade class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Upgrade.h"
#include <math.h>
#include <sstream>
#include <upgradeProtocol.h>
#include "PropBag.h"
#include <helper/slog.h>
#include "UpgAlgorithm.h"
#define kLogTag "Upgrade_server"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUpgrade::CUpgrade():CGameTable(PLAYER_COUNT)
{
	m_gameState=GS_WAITSTART;
	m_nBankerSeat=-1;		//庄家还没有定
	if(PropBag::getSingletonPtr()->GetProp(PI_RULE_LEVEL_5_10_K))
		m_nLevelA=m_nLevelB = 4;
	else
		m_nLevelA=m_nLevelB= 1;	
	m_nShowMainSeat=-1;		//还没有人亮主。
	m_nMainColor=-1;
	m_pnShowMainInfo=NULL;
	m_nScore=0;
}

CUpgrade::~CUpgrade()
{
	if(m_pnShowMainInfo) free(m_pnShowMainInfo);
}
void CUpgrade::OnGameStart()
{
	CGameTable::OnGameStart();
	//all user are getting ready, game begin
	m_gameState=GS_WAITCARD;
	m_nMainColor=-1;
	if(m_pnShowMainInfo)
	{
		free(m_pnShowMainInfo);
		m_pnShowMainInfo=NULL;
	}
	m_nShowMainSeat=-1;
	for(int i=0;i<PLAYER_COUNT;i++)
	{
		m_clients[i]->m_dwState=0;
        PCLIENTDATA pClientData = (PCLIENTDATA)malloc(sizeof(CLIENTDATA));
        memset(pClientData, 0, sizeof(CLIENTDATA));
        m_clients[i]->m_dwData = (DWORD_PTR)pClientData;
	}
    m_nScore = 0;
    InitCard();
    SendMsg(NULL, GMT_START, NULL, 0); // 通知所有人游戏开始
}

void CUpgrade::OnGameEnd()
{
	CGameTable::OnGameEnd();
	SendMsg(NULL, GMT_END, NULL, 0);
}

void CUpgrade::OnRoundEnd()
{
	CGameTable::OnRoundEnd();
	ClearState();
}

void CUpgrade::InitCard()
{
	int nCards[108];
	CUpgAlgorithm::InitCard(nCards);
	//洗牌
	CUpgAlgorithm::ShuffleCard(nCards,108);
	//保存初始牌数据
	for(int i=0;i<PLAYER_COUNT;i++)
	{//保存玩家牌数据
		PCLIENTDATA pData=(PCLIENTDATA)m_clients[i]->m_dwData;
		ASSERT(pData);
		memcpy(pData->nCards,nCards+25*i,25*sizeof(int));
		pData->nCardCount=25;
	}
	memcpy(m_nCardBottom,nCards+100,8*sizeof(int));
}

BOOL CUpgrade::OnMsg(PCLIENT pClient, DWORD dwType,LPVOID pData,DWORD dwSize)
{
	if(GetState()!=TABLE_STATE_PLAYING) 
		return FALSE;
	switch ( dwType)
	{
	case GMT_UPGRADE_REQCARD:
		OnReqCard(pClient,pData,dwSize);
		break;
	case GMT_UPGRADE_REQSHOWMAIN:
		OnReqShowMain(pClient,pData,dwSize);
		break;
	case GMT_UPGRADE_REQPUTBOTTOM:
		OnReqPutBottom(pClient,pData,dwSize);
		break;
	case GMT_UPGRADE_REQREBELPASS:
		OnReqRebelPass(pClient,pData,dwSize);
		break;
	case GMT_UPGRADE_REQPUTCARD:
		OnReqPutCard(pClient,pData,dwSize);
		break;
	case GMT_UPGRADE_GIFT:
		OnGiftSend(pClient,pData,dwSize);
		break;
	case GMT_UPGRADE_CHAT:
		OnChatSend(pClient,pData,dwSize);
		break;
	}
	return TRUE;
}

void CUpgrade::OnChatSend(PCLIENT pClient, LPVOID pData, DWORD dwSize)
{
	PGS_CHATINFO pChat=(PGS_CHATINFO)pData;
	if(pChat->nIdxSelf==pChat->nIdxTarget)
	{
		for(int i=0;i<PLAYER_COUNT;i++)
		{
			if(pChat->nIdxSelf==i) continue;
			if(m_clients[i])
			{
				SendMsg(m_clients[i],GMT_UPGRADE_CHAT,pData,dwSize);
			}
		}
	}else
	{
		if(m_clients[pChat->nIdxTarget])
		{//转发消息
			SendMsg(m_clients[pChat->nIdxTarget],GMT_UPGRADE_CHAT,pData,dwSize);
		}
	}
}

void CUpgrade::OnGiftSend(PCLIENT pClient, LPVOID pData, DWORD dwSize)
{
	PGS_GIFTINFO pGift=(PGS_GIFTINFO)pData;
	if(pGift->nIdxSelf==pGift->nIdxTarget)
	{
		for(int i=0;i<PLAYER_COUNT;i++)
		{
			if(pGift->nIdxSelf==i) continue;
			if(m_clients[i])
			{
				SendMsg(m_clients[i],GMT_UPGRADE_GIFT,pData,dwSize);
			}
		}
	}else
	{
		if(m_clients[pGift->nIdxTarget])
		{//转发消息
			SendMsg(m_clients[pGift->nIdxTarget],GMT_UPGRADE_GIFT,pData,dwSize);
		}
	}
}

void CUpgrade::OnReqCard(PCLIENT pClient, LPVOID pData, DWORD dwSize)
{
	if(pClient->m_dwState) return;
	pClient->m_dwState=1;
	for(int i=0;i<PLAYER_COUNT;i++)
	{
		if(m_clients[i]->m_dwState==0) return;
	}
	//发牌
	for(int i=0;i<PLAYER_COUNT;i++)
	{
		m_clients[i]->m_dwState=0;	//将状态值重新置0
		PCLIENTDATA pClientData=(PCLIENTDATA)m_clients[i]->m_dwData;
		SendMsg(m_clients[i],GMT_UPGRADE_POSTCARD,pClientData->nCards,25*sizeof(int));
	}
	m_gameState=GS_SHOWMAIN;
	if(m_pnShowMainInfo)
	{
		free(m_pnShowMainInfo);
		m_pnShowMainInfo=NULL;
		m_nShowMainSeat=-1;
	}
}

int CUpgrade::GetLevel()
{
	if(m_nBankerSeat==-1) return m_nLevelA;
	if(m_nBankerSeat%2==0) return m_nLevelA;
	return m_nLevelB;
}

//请求亮主
void CUpgrade::OnReqShowMain(PCLIENT pClient, LPVOID pData, DWORD dwSize)
{
	if(dwSize != sizeof(GAME_UPGRADE_SHOWMAIN))
	{
		SLOGW()<<"OnReqShowMain, invalid data size:"<<dwSize;
		return;
	}
	GAME_UPGRADE_SHOWMAIN *pMsg=(GAME_UPGRADE_SHOWMAIN*)pData;
	int nSeat=pMsg->index;
	if(m_gameState==GS_SHOWMAIN)
	{
		PCLIENTDATA pClientData=(PCLIENTDATA)pClient->m_dwData;
		if(!CUpgAlgorithm::ExtractCards(pClientData->nCards,pClientData->nCardCount,pMsg->nCard,pMsg->nCount,TRUE))
		{
			SLOGW()<<"OnReqShowMain, invalid card";
			return ;
		}
		int nLevelCard=GetLevel();

		if(pMsg->nCount==1)
		{
			if(m_pnShowMainInfo) 
				return;
			if(pMsg->nCard[0]>52) 
				return;//大小王不能单张亮主。
			if(pMsg->nCard[0]%13 != nLevelCard) 
				return;
		}else if(pMsg->nCount==2)
		{
			if(pMsg->nCard[0]!=pMsg->nCard[1]) 
				return;//两张牌必须一样
			if(pMsg->nCard[0]<52 && pMsg->nCard[0]%13 != nLevelCard) 
				return;//不是王时必须是级牌
			if(m_pnShowMainInfo && m_pnShowMainInfo->nCount==2 && pMsg->nCard[0]<=m_pnShowMainInfo->nCard[0]) 
				return;//已经有人定主时必须大过
		}else
		{
			return;
		}
		//确定亮主
		if(m_pnShowMainInfo) free(m_pnShowMainInfo);
		m_pnShowMainInfo=(GAME_UPGRADE_SHOWMAIN*)malloc(sizeof(GAME_UPGRADE_SHOWMAIN));
		memcpy(m_pnShowMainInfo,pMsg,sizeof(GAME_UPGRADE_SHOWMAIN));
		m_nShowMainSeat=pMsg->index;
		m_nMainColor= CUpgAlgorithm::GetCardColor(pMsg->nCard[0]);
		//发亮主消息
		SendMsg(NULL,GMT_UPGRADE_REQSHOWMAIN,pData,dwSize);
	}else if(m_gameState==GS_REBELBOTTOM)
	{
		if(pClient->m_dwState==RBLCR_DENY)
			return;
		PCLIENTDATA pClientData=(PCLIENTDATA)pClient->m_dwData;
		if(pMsg->nCount!=2 || pMsg->nCard[0]!=pMsg->nCard[1])
			return;
		int nRblCr=CUpgAlgorithm::GetCardColor(pMsg->nCard[0]);
		if(nRblCr<RBLCR_SKING)
		{
			if(pMsg->nCard[0]%13 != GetLevel()) 
				return;
		}
		if(m_pnShowMainInfo->nCount==2 && pMsg->nCard[0]<=m_pnShowMainInfo->nCard[0]) 
			return;

		if(!CUpgAlgorithm::ExtractCards(pClientData->nCards,25,pMsg->nCard,pMsg->nCount,TRUE))
			return;
		pClient->m_dwState=nRblCr;
		//检查前面的用户是否已经放弃
		for(int i=1;i<PLAYER_COUNT;i++)
		{
			int nSeat=(m_nBottomOwner+i)%PLAYER_COUNT;
			if(nSeat==pClient->m_nIndex) break;
			if(m_clients[nSeat]->m_dwState != RBLCR_PASS
				&& m_clients[nSeat]->m_dwState != RBLCR_DENY)
				return;
		}
		//先广播一个亮主消息
		m_nBottomOwner=nSeat;
		m_nShowMainSeat=nSeat;
		ASSERT(m_pnShowMainInfo);
		free(m_pnShowMainInfo);
		m_nMainColor=nRblCr;
		m_pnShowMainInfo=(GAME_UPGRADE_SHOWMAIN*)malloc(sizeof(GAME_UPGRADE_SHOWMAIN));
		memcpy(m_pnShowMainInfo,pMsg,sizeof(GAME_UPGRADE_SHOWMAIN));
		SendMsg(NULL,GMT_UPGRADE_REQSHOWMAIN,pData,dwSize);
		//发底牌
		memcpy(pClientData->nCards+25,m_nCardBottom,8*sizeof(int));
		pClientData->nCardCount=33;
		int nData[9];
		nData[0]=nSeat;
		memcpy(nData+1,m_nCardBottom,8*sizeof(int));
		SendMsg(NULL,GMT_UPGRADE_POSTBOTTOM,nData,9*sizeof(int));
		m_gameState=GS_PUTBOTTOM2;
	}
}


void CUpgrade::OnReqPutBottom(PCLIENT pClient, LPVOID lpData, DWORD dwSize)
{
	if(m_gameState!=GS_PUTBOTTOM && m_gameState!=GS_PUTBOTTOM2) 
		return;
	if(pClient->m_nIndex!=m_nBottomOwner) 
		return;
	PCLIENTDATA pClientData=(PCLIENTDATA)pClient->m_dwData;
	ASSERT(pClientData->nCardCount==33);
	GAME_UPGRADE_REQPUTBOTTOM *pnData=(GAME_UPGRADE_REQPUTBOTTOM*)lpData;
	if(CUpgAlgorithm::ExtractCards(pClientData->nCards,33,pnData->nBottom,8,FALSE)==FALSE) 
		return;
	pClientData->nCardCount=25;
	memcpy(m_nCardBottom,pnData->nBottom,8*sizeof(int));

	//广播垫底消息
	SendMsg(NULL,GMT_UPGRADE_REQPUTBOTTOM,lpData,dwSize);
	if(m_nShowMainSeat!=-1 && m_nMainColor!=COLOR_BKING)
	{
		for(int i=0;i<PLAYER_COUNT;i++) 
			m_clients[i]->m_dwState=RBLCR_WAIT;
		m_clients[pClient->m_nIndex]->m_dwState=RBLCR_DENY;	
		if(m_gameState==GS_PUTBOTTOM)
			m_clients[m_nShowMainSeat]->m_dwState=RBLCR_DENY;
		m_gameState=GS_REBELBOTTOM;
	}else
	{
		m_gameState=GS_PUTCARD;
		for(int i=0;i<PLAYER_COUNT;i++)
		{
			PCLIENTDATA pClientData=(PCLIENTDATA)m_clients[i]->m_dwData;
			m_clients[i]->m_dwState=0;
		}
		m_clients[m_nBankerSeat]->m_dwState=1;	//该用户可以出牌标志
		m_nTurnFirstSeat=m_nBankerSeat;
		//通知庄家开始出牌
		SLOGI()<<"OnReqPutBottom done, ============================================";
		for(int i=0;i<PLAYER_COUNT;i++){
			std::stringstream ss;
			for(int j=0;j<25;j++){
				ss<<pClientData->nCards[j]<<" ";
			}
			SLOGI() << "player " << i << " : cards=" << ss.str().c_str();
		}
		SendMsg(NULL,GMT_UPGRADE_STARTPUTCARD,&m_nTurnFirstSeat,sizeof(int));
	}
}

void CUpgrade::OnReqRebelPass(PCLIENT pClient, LPVOID lpData, DWORD dwSize)
{
	if(m_gameState==GS_REBELBOTTOM)
	{//反底阶段的pass
		if(pClient->m_dwState!=RBLCR_WAIT) 
			return;
		pClient->m_dwState=RBLCR_PASS;
		//检查是否全部Pass
		for(int i=0;i<PLAYER_COUNT;i++)
		{//查找下一个请求反底的人
			int nSeat=(i+m_nShowMainSeat)%PLAYER_COUNT;	
			if(m_clients[nSeat]->m_dwState==RBLCR_WAIT) 
				return;
			if(m_clients[nSeat]->m_dwState!=RBLCR_PASS &&
				m_clients[nSeat]->m_dwState!=RBLCR_DENY)
			{//找到下一个反底的用户，给他发底牌
				GAME_UPGRADE_SHOWMAIN msg;
				msg.index=nSeat;
				msg.nCount=2;
				msg.nCard[0]=msg.nCard[1]= CUpgAlgorithm::BuildCard(m_clients[nSeat]->m_dwState,GetLevel());
				//广播亮主信息
				m_nMainColor=m_clients[nSeat]->m_dwState;
				ASSERT(m_pnShowMainInfo);
				free(m_pnShowMainInfo);
				m_pnShowMainInfo=(GAME_UPGRADE_SHOWMAIN*)malloc(sizeof(GAME_UPGRADE_SHOWMAIN));
				memcpy(m_pnShowMainInfo,&msg,sizeof(msg));
				m_nShowMainSeat=nSeat;
				SendMsg(NULL,GMT_UPGRADE_REQSHOWMAIN,&msg,sizeof(msg));
				//广播发底牌信息
				m_nBottomOwner=nSeat;
				PCLIENTDATA pClientData=(PCLIENTDATA)m_clients[nSeat]->m_dwData;
				memcpy(pClientData->nCards+25,m_nCardBottom,8*sizeof(int));
				pClientData->nCardCount=33;
				GAME_UPGRADE_POSTBOTTOM msg2;
				msg2.iOwnerIndex=nSeat;
				memcpy(msg2.nBottom,m_nCardBottom,8*sizeof(int));
				SendMsg(NULL,GMT_UPGRADE_POSTBOTTOM,&msg2,sizeof(msg2));
				m_gameState=GS_PUTBOTTOM;
				return;
			}
		}
		//开始出牌
		m_gameState=GS_PUTCARD;
		for(int i=0;i<PLAYER_COUNT;i++)
		{
			PCLIENTDATA pClientData=(PCLIENTDATA)m_clients[i]->m_dwData;
			m_clients[i]->m_dwState=0;
		}
		m_clients[m_nBankerSeat]->m_dwState=1;	//该用户可以出牌标志
		m_nTurnFirstSeat=m_nBankerSeat;

		SendMsg(NULL,GMT_UPGRADE_STARTPUTCARD,&m_nTurnFirstSeat,sizeof(int));
	}else if(m_gameState==GS_SHOWMAIN)
	{//亮主阶段的Pass
		pClient->m_dwState=1;
		for(int i=0;i<PLAYER_COUNT;i++)
		{
			ASSERT(m_clients[i]);
			if(m_clients[i]->m_dwState==0) return;
		}
		if(m_nBankerSeat==-1 && m_nShowMainSeat==-1)
		{//游戏重新开始
			m_gameState=GS_WAITCARD;
			for(int i=0;i<PLAYER_COUNT;i++) m_clients[i]->m_dwState=0;
			InitCard();
			SendMsg(NULL,GMT_START,NULL,0);
		}else
		{
			if(m_nBankerSeat==-1)
			{
				m_nBankerSeat=m_nShowMainSeat;
			}else if(m_nShowMainSeat==-1)
			{//没人亮主，自动抽一张牌
				GAME_UPGRADE_AUTOCOLOR req;
				req.iBottomIndex = rand()%8;
				req.nCard=m_nCardBottom[req.iBottomIndex];
				m_nMainColor=CUpgAlgorithm::GetCardColor(req.nCard);
				SendMsg(NULL,GMT_UPGRADE_AUTOCOLOR,&req,sizeof(req));
			}
			//将底牌加入到庄家手中
			PCLIENTDATA pClientData=(PCLIENTDATA)m_clients[m_nBankerSeat]->m_dwData;
			memcpy(pClientData->nCards+25,m_nCardBottom,8*sizeof(int));
			pClientData->nCardCount+=8;
			//发底牌
			int nData[9];
			nData[0]=m_nBankerSeat;
			memcpy(nData+1,m_nCardBottom,8*sizeof(int));
			SendMsg(NULL,GMT_UPGRADE_POSTBOTTOM,nData,9*sizeof(int));
			m_nBottomOwner=m_nBankerSeat;
			//进入垫底阶段
			m_gameState=GS_PUTBOTTOM;
		}
	}
}


void CUpgrade::OnReqPutCard(PCLIENT pClient, LPVOID lpData, DWORD dwSize)
{
	if(m_gameState!=GS_PUTCARD) return;
	if(pClient->m_dwState!=1) return;
	PCLIENTDATA pClientData=(PCLIENTDATA)pClient->m_dwData;
	GAME_UPGRADE_REQPUTCARD *pReq=(GAME_UPGRADE_REQPUTCARD*)lpData;
	if(!CUpgAlgorithm::ExtractCards(pClientData->nCards,pClientData->nCardCount,pReq->nCard,pReq->nCount,TRUE)){
		SLOGW()<<"Invalid card"<<pClient->m_nIndex;
		return;
	}
	int cErrorCode=0;
	bool is2ConstMain = PropBag::getSingletonPtr()->GetProp(PI_RULE_2_MAIN);
	if(pClient->m_nIndex==m_nTurnFirstSeat)
	{// 首家出牌
		PutCardResult result = CUpgAlgorithm::ValidateFirstPlay(pReq->nCard,pReq->nCount, m_nMainColor, GetLevel(), is2ConstMain);
		if (result != PUT_CARD_RESULT_SUCCESS)
		{
			cErrorCode = (int)result;
			SendMsg(pClient,GMT_UPGRADE_PUTCARDERR,&cErrorCode,sizeof(int));
			return;
		}
		int cardType = CUpgAlgorithm::GetCardType(pReq->nCard, pReq->nCount, m_nMainColor, GetLevel(), is2ConstMain);
		BOOL isThrow = FALSE;
		if(cardType == CARD_TYPE_MIX)
			isThrow = TRUE;
		if(cardType == CARD_TYPE_SINGLE && pReq->nCount>1)
			isThrow = TRUE;
		if(cardType == CARD_TYPE_PAIR && pReq->nCount>2)
			isThrow = TRUE;
		if(isThrow){
			//step2:检查甩牌是否成功:在4家手中检查是否有更大的牌,从最大的牌型开始查找
			// 首先判断首家出牌是主牌还是副牌
			bool bIsMainColor = (CUpgAlgorithm::GetMainValue(pReq->nCard[0], m_nMainColor, GetLevel(),is2ConstMain) >= 0);
			// 如果是主牌，传递COLOR_MAIN标识；如果是副牌，传递具体花色
			int nColor = bIsMainColor ? 10 : CUpgAlgorithm::GetCardColor(pReq->nCard[0]);  // COLOR_MAIN = 10
            const int *pHandCards[PLAYER_COUNT - 1];
			int nCardCount[PLAYER_COUNT-1];
			for(int i=0;i<PLAYER_COUNT-1;i++)
			{
				int iSeat=(i+m_nTurnFirstSeat+1)%PLAYER_COUNT;
				PCLIENTDATA tmp=(PCLIENTDATA)m_clients[iSeat]->m_dwData;
                pHandCards[i] = tmp->nCards;
                nCardCount[i] = tmp->nCardCount;
			}
			int nFailedCard[26]={0};
            int nFailedCount = CUpgAlgorithm::CheckThrowFailed(pReq->nCard, pReq->nCount, pHandCards, nCardCount, m_nMainColor, GetLevel(), is2ConstMain, nFailedCard);
			if(nFailedCount)
			{//甩牌失败,强制出小
				//print 4 player cards to log
				for(int i=0;i<PLAYER_COUNT;i++){
					PCLIENTDATA pClientData=(PCLIENTDATA)m_clients[i]->m_dwData;
					std::stringstream ss;
					for(int j=0;j<pClientData->nCardCount;j++){
						ss<<pClientData->nCards[j]<<" ";
					}
					SLOGI() << "player " << i << " : cards=" << ss.str().c_str();
				}
				//print throw failed cards
				{
					std::stringstream ss;
					for(int i=0;i<pReq->nCount;i++){
						ss<<pReq->nCard[i]<<" ";
					}
					SLOGI() << "throw cards: " << ss.str().c_str();
					for(int i=0;i<nFailedCount;i++){
						ss<<nFailedCard[i]<<" ";
					}
					SLOGI() << "failed and put cards: " << ss.str().c_str();
				}
				GAME_UPGRADE_THROWFAILED ack;
				ack.nIndex=pClient->m_nIndex;
				ack.nPutCount=pReq->nCount;
				ack.nDiscount=ack.nPutCount*5;
				memcpy(ack.nPutCard,pReq->nCard,ack.nPutCount*sizeof(int));
				ack.nFailedCount = nFailedCount;
				memcpy(ack.nFailedCard,nFailedCard,ack.nFailedCount*sizeof(int));
				//将必出牌信息加出到出牌数据中
				CUpgAlgorithm::ExtractCards(pClientData->nCards, pClientData->nCardCount, nFailedCard, nFailedCount, FALSE);
				pClientData->nCardCount -= nFailedCount;

				{
					std::stringstream ss;
					for(int i=0;i<pClientData->nCardCount;i++){
						ss<<pClientData->nCards[i]<<" ";
					}
				}
				
				memcpy(pClientData->nCardPut+1, nFailedCard, nFailedCount * sizeof(int));
				pClientData->nCardPut[0] = nFailedCount;
				pClient->m_dwState=0;
				int nNextSeat=(pClient->m_nIndex+1)%PLAYER_COUNT;
				m_clients[nNextSeat]->m_dwState=1;
				if(pClient->m_nIndex%2 != m_nBankerSeat%2)
					m_nScore-=ack.nDiscount;
				else
					m_nScore+=ack.nDiscount;
				SendMsg(NULL,GMT_UPGRADE_THROWFAILED,&ack,sizeof(ack));
				return;
			}
		}
	}else
	{//跟牌
		PCLIENTDATA pFirstData=(PCLIENTDATA)m_clients[m_nTurnFirstSeat]->m_dwData;
		//step1:检查牌张数
		if(pReq->nCount!=pFirstData->nCardPut[0])
		{
			cErrorCode=PCE_FOLLOWCOUNT;
			SendMsg(pClient,GMT_UPGRADE_PUTCARDERR,&cErrorCode,sizeof(int));
			return;
		}
		PutCardResult result = CUpgAlgorithm::ValidateFollowPlay(pFirstData->nCardPut+1, pFirstData->nCardPut[0], 
			pReq->nCard, pReq->nCount, 
			pClientData->nCards, pClientData->nCardCount, 
			m_nMainColor, GetLevel(), PropBag::getSingletonPtr()->GetProp(PI_RULE_2_MAIN)); 
		if (result != PUT_CARD_RESULT_SUCCESS)
		{
			cErrorCode = (int)result;
			SendMsg(pClient,GMT_UPGRADE_PUTCARDERR,&cErrorCode,sizeof(int));
			return;
		}
	}

	//广播出牌消息
	CUpgAlgorithm::ExtractCards(pClientData->nCards,pClientData->nCardCount,pReq->nCard,pReq->nCount,FALSE);
	pClientData->nCardCount-=pReq->nCount;
	std::stringstream ss;
	for(int i=0;i<pReq->nCount;i++){
		ss<<pReq->nCard[i]<<" ";
	}
	SLOGI() << "player " << pClient->m_nIndex << " put cards: " << ss.str().c_str();

	SendMsg(NULL,GMT_UPGRADE_REQPUTCARD,lpData,dwSize);
	pClientData->nCardPut[0]=pReq->nCount;
	memcpy(pClientData->nCardPut+1,pReq->nCard,pReq->nCount*sizeof(int));
	pClient->m_dwState=0;
	int nNextSeat=(pClient->m_nIndex+1)%PLAYER_COUNT;
	if(nNextSeat!=m_nTurnFirstSeat)
	{//设置下一个人出牌标志
		m_clients[nNextSeat]->m_dwState=1;
	}else
	{//一轮牌出完

		const int * pCardsPlayed[PLAYER_COUNT];
		int nCardsCount[PLAYER_COUNT];
		for(int i=0;i<PLAYER_COUNT;i++)
		{
			PCLIENTDATA pClientData=(PCLIENTDATA)m_clients[i]->m_dwData;
			pCardsPlayed[i]=pClientData->nCardPut+1;
			nCardsCount[i]=pClientData->nCardPut[0];
		}
		int nWinner=CUpgAlgorithm::DetermineWinner(pCardsPlayed,nCardsCount,m_nMainColor,GetLevel(),is2ConstMain,m_nTurnFirstSeat);
		if((nWinner%2) != (m_nBankerSeat%2))
		{//闲家大，闲家检分
			for(int i=0;i<PLAYER_COUNT;i++)
			{
				for(int j=0;j<nCardsCount[i];j++)
				{
					int nValue=pCardsPlayed[i][j]%13;
					if(nValue==4)
						m_nScore+=5;
					else if(nValue==9 || nValue==12)
						m_nScore+=10;
				}
			}
		}
		pClientData=(PCLIENTDATA)m_clients[nWinner]->m_dwData;
		if(pClientData->nCardCount)
		{//还有牌，通知出牌
			for(int i=0;i<PLAYER_COUNT;i++)
			{//清除本轮数据
				pClientData=(PCLIENTDATA)m_clients[i]->m_dwData;
				pClientData->nCardPut[0]=0;
				m_clients[i]->m_dwState=0;
			}
			m_clients[nWinner]->m_dwState=1;
			m_nTurnFirstSeat=nWinner;
			SendMsg(NULL,GMT_UPGRADE_TURNOVER,&nWinner,sizeof(int));
		}else
		{//游戏结束,下发底牌、翻倍、及总得分
			GAME_UPGRADE_ROUNDEND gsri;
			gsri.nMaxIndex=nWinner;
			memcpy(gsri.nBottom,m_nCardBottom,8*sizeof(int));
			if((nWinner%2) != (m_nBankerSeat%2))
			{//闲家扣底
				CARDTYPE cardType = (CARDTYPE)CUpgAlgorithm::GetCardType(pClientData->nCardPut+1, pClientData->nCardPut[0], m_nMainColor, GetLevel(), is2ConstMain);
				if(cardType == CARD_TYPE_PAIR)
					gsri.nMultiple = 2;
				else if(cardType == CARD_TYPE_TRACTOR)
					gsri.nMultiple = (int)pow(2.0f,pClientData->nCardPut[0]/2);
				int nBtmScore=0;
				for(int i=0;i<8;i++)
				{
					int nValue=m_nCardBottom[i]%13;
					if(nValue==4)
						nBtmScore+=5;
					else if(nValue==9 || nValue==12)
						nBtmScore+=10;
				}
				gsri.nScore=m_nScore+nBtmScore*gsri.nMultiple;
			}else
			{
				gsri.nMultiple=0;
				gsri.nScore=m_nScore;
			}
			//根据得分计算下一轮的级别
			if(gsri.nScore>=80)
			{//下庄
				if(m_nBankerSeat%2==0)
				{
					m_nLevelB = NextLevel(m_nLevelB, (gsri.nScore-80)/40);
				}else
				{
					m_nLevelA = NextLevel(m_nLevelA, (gsri.nScore-80)/40);
				}
				m_nBankerSeat=(m_nBankerSeat+1)%PLAYER_COUNT;
			}else
			{//过庄
				int nUpLevel = 0;
				if(gsri.nScore==0)
				{
					nUpLevel=3;
				} 
				else if(gsri.nScore<40)
				{
					nUpLevel=2; 
				}else
				{
					nUpLevel=1;
				}
				if(m_nBankerSeat%2==0)
				{
					m_nLevelA = NextLevel(m_nLevelA, nUpLevel);
				}else
				{
					m_nLevelB = NextLevel(m_nLevelB, nUpLevel);
				}
				m_nBankerSeat=(m_nBankerSeat+2)%PLAYER_COUNT;
			}
			gsri.nLevelA=m_nLevelA;
			gsri.nLevelB=m_nLevelB;
			gsri.nNewBankerIndex=m_nBankerSeat;
			//广播游戏结束消息
			SendMsg(NULL,GMT_UPGRADE_ROUNDEND,&gsri,sizeof(GAME_UPGRADE_ROUNDEND));
			OnRoundEnd();
		}
	}
}

int CUpgrade::NextLevel(int nCurLevel,int nUpLevel)
{
	if(PropBag::getSingletonPtr()->GetProp(PI_RULE_LEVEL_5_10_K))
	{
		static const int nLevel5[3]={4,9,12};
		int i=0;
		for(i=0;i<3;i++)
		{
			if(nLevel5[i]==nCurLevel) break;
		}
		SASSERT(i<3);
		i=(i+nUpLevel)%3;
		return nLevel5[i];
	}else
	{ 
		return (nCurLevel+nUpLevel)%13;
	}
}

void CUpgrade::ClearState()
{
	if(m_gameState==GS_WAITSTART) return;
	m_gameState=GS_WAITSTART;
	//清除游戏状态数据
	m_nShowMainSeat=-1;
	m_nMainColor=-1;
	if(m_pnShowMainInfo)
	{
		free(m_pnShowMainInfo);
		m_pnShowMainInfo=NULL;
	}
	m_nBottomOwner=-1;
	m_nScore=0;
	for(int i=0;i<PLAYER_COUNT;i++)
	{//清除本轮数据
		if(!m_clients[i]) continue;
		PCLIENTDATA pClientData=(PCLIENTDATA)m_clients[i]->m_dwData;
		pClientData->nCardPut[0]=0;
		m_clients[i]->m_dwState=0;
		m_clients[i]->m_bReady = FALSE;
	}
}
