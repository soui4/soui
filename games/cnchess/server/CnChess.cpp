#include "stdafx.h"
#include "CnChess.h"
#include "GameClient.h"
#include "PropBag.h"
#include <algorithm>
#include <string/strcpcvt.h>
#include <helper/slog.h>
#include <time.h>
#define kLogTag "CCnChess"

SNSBEGIN

CCnChess::CCnChess(ITableListener* pListener, int nTableId):CGameTable(pListener, nTableId, PLAYER_COUNT)
{
    m_nRedIndex = rand()%PLAYER_COUNT;
    memcpy(m_dwProps, PropBag::getSingletonPtr()->m_dwProps, sizeof(DWORD) * PROP_SIZE);
}

CCnChess::~CCnChess()
{
    for (auto& client : m_clients)
    {
        if (client != nullptr)
        {
            delete client;
            client = nullptr;
        }
    }
}

void CCnChess::Boardcast(DWORD dwType, LPVOID pData, DWORD dwSize)
{
    for (auto& client : m_clients)
    {
        if (client == nullptr) {
            continue;
        }
        if (!client->m_pConn)
        {
            continue;
        }
        client->m_pConn->sendBinary2(dwType, pData, dwSize);
    }
}

BOOL CCnChess::SendMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
    if (!pClient)
    {
        Boardcast(dwType, pData, dwSize);
        return TRUE;
    }
    if (!pClient->m_pConn)
    {
        return FALSE;
    }
    pClient->m_pConn->sendBinary2(dwType, pData, dwSize);
    return TRUE;
}

BOOL CCnChess::OnMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
    switch (dwType)
    {
    case MSG_REQ_MOVE:
        return OnChessMove(pClient, dwType, pData, dwSize);
    case MSG_REQ_SURRENDER:
        return OnPlayerSurrender(pClient, dwType, pData, dwSize);
    case MSG_REQ_TIMEOUT:
        return OnPlayerTimeout(pClient, dwType, pData, dwSize);
    case MSG_REQ_REGRET:
        return OnRegret(pClient, dwType, pData, dwSize);
    case MSG_REQ_PEACE:
        return OnReqPeace(pClient, dwType, pData, dwSize);
    case MSG_ACK_PEACE:
        return OnAckPeace(pClient, dwType, pData, dwSize);
    case MSG_REQ_LONGCATCH:
        return OnReqLongCatch(pClient, dwType, pData, dwSize);
	case MSG_REQ_RESULT:
        return OnResult(pClient, dwType, pData, dwSize);
    }
    return TRUE;
}

void CCnChess::OnGameStart()
{
    CGameTable::OnGameStart();
    SLOGI() << "game start";
    //游戏开始前的初始化逻辑
    m_layout.InitLayout(NULL,CS_RED);
    m_dwStartTime = time(NULL);
    MSG_INIT msg;
    memcpy(msg.layout,m_layout.m_chesses,sizeof(msg.layout));
    msg.iRedIndex = m_nRedIndex;

    m_nChsPassable = m_nLeftPassable = 22;
    m_regretRec[0].nLeft=m_regretRec[1].nLeft=m_dwProps[PROPID_REGRET];
    m_regretRec[0].iMoveStep=m_regretRec[1].iMoveStep=-1;
    m_nRegretSteps[0]=m_nRegretSteps[1]=0;
    m_nPeaceSeat=-1;
    m_nPeaceSteps=0;
    m_nChessMsg=0;
    m_goType = GOT_NORMAL;
    m_lstMoves.clear();

    SendMsg(NULL, GMT_START, &msg, sizeof(msg));
}

void CCnChess::OnGameEnd()
{
    CGameTable::OnGameEnd();
    m_nRedIndex = (m_nRedIndex+1)%2;
}

BOOL CCnChess::OnPlayerLeave(int seatId, PWSCLIENT pClient)
{
	CGameTable::OnPlayerLeave(seatId, pClient);
	BrdcstAckOver(GOT_NETBREAK, seatId, L"断线");
    return TRUE;
}

int CCnChess::GetActiveSeat() const
{
    CHSSIDE activeSide = m_layout.m_actSide;
    if(activeSide == CS_RED){
        return GetRedSeat();
    }else{
        return (GetRedSeat()+1)%2;
    }
    return 0;
}
//用户投降
BOOL CCnChess::OnPlayerSurrender(PWSCLIENT pClient,DWORD dwType,LPVOID pData , DWORD dwSize)
{
    SLOGI() << "OnPlayerSurrender";
    PMSG_SURRENDER pSurrender = (PMSG_SURRENDER)pData;
	if(pSurrender->iIndex!=pClient->m_nIndex || m_state!=TABLE_STATE_PLAYING)
        return FALSE;
    BrdcstAckOver(GOT_SURRENDER, pSurrender->iIndex, L"投降");
	return TRUE;
}

static BOOL PtEqual(POINT pt1,POINT pt2)
{
	return pt1.x==pt2.x && pt1.y==pt2.y;
}


//服务器主动通知用户游戏结束
void CCnChess::BrdcstAckOver(GAMEOVERTYPE nType,int nLossSeat, LPCWSTR pszDesc)
{
	MSG_GAMEOVER msg;
	msg.iWinner=(nLossSeat==-1)?-1:((nLossSeat+1)%2);
    SStringA strDesc = S_CW2A(pszDesc, CP_UTF8);
	strcpy(msg.szDesc,strDesc.c_str());

    SendMsg(NULL, MSG_ACK_GAMEOVER, &msg, sizeof(MSG_GAMEOVER));
	m_goType=nType;
    EndGame(nType, pszDesc);
}
//走棋消息
BOOL CCnChess::OnChessMove(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
	PMSG_MOVE pMsgMove=(PMSG_MOVE)(pData);
    int iIndexOrder = pClient->m_nIndex;
	if(iIndexOrder!=pMsgMove->iIndex || TABLE_STATE_PLAYING != m_state) 
        return FALSE;
    if(iIndexOrder != GetActiveSeat())
    {
        SLOGW() << "Invalide move Seat="<<iIndexOrder;
        return FALSE;
    }

	if(pMsgMove->ptBegin.x<0 || pMsgMove->ptBegin.x>8
		||pMsgMove->ptBegin.y<0 || pMsgMove->ptBegin.y>9
		||pMsgMove->ptEnd.x<0 || pMsgMove->ptEnd.x>8
		||pMsgMove->ptEnd.y<0 || pMsgMove->ptEnd.y>9
		)
	{
		SLOGW() << "Invalide move msg. seat="<<iIndexOrder; 
		return FALSE;
	}
	//效验走棋合法性
	if(!m_layout.IsValidMove(pMsgMove->ptBegin,pMsgMove->ptEnd))
	{
		PWSCLIENT pPlayer=GetPlayer(iIndexOrder);
		SLOGW()<<L"走棋验证失败,nickname="<< S_CA2W(pPlayer->m_userInfo.szName,CP_UTF8).c_str();
        BrdcstAckOver(GOT_CHEAT,iIndexOrder,L"作弊！");
		return TRUE;
	}
    time_t now = time(NULL);
	time_t  tSpan=now-m_dwStartTime;
	m_alTime[iIndexOrder]+=tSpan;
	if(abs((int)m_alTime[iIndexOrder]-(int)pMsgMove->dwRoundTime)<2)
		m_alTime[iIndexOrder]=pMsgMove->dwRoundTime;
	else
		pMsgMove->dwRoundTime=m_alTime[iIndexOrder];
	m_dwStartTime=now;

	MOVESTEP mstep=m_layout.Move(pMsgMove->ptBegin,pMsgMove->ptEnd);
	m_lstMoves.push_back(mstep);
	SendMsg(NULL, dwType, pData, dwSize); //广播消息给所有用户，包括自己。自己也会收到自己发出的消息。
	m_nChessMsg++;
	//累计未吃子步数
	if(mstep.enemy==CHSMAN_NULL)
		m_nPeaceSteps++;
	else
	{
		m_nPeaceSteps=0;
		int nChs=mstep.enemy%7;
		if((nChs>=CHSMAN_RED_JU && nChs<=CHSMAN_RED_PAO) || nChs==CHSMAN_RED_BING)
			m_nLeftPassable--;
	}
	if(m_nPeaceSteps==m_dwProps[PROPID_MAX_STP_PEACE])
	{//通知和棋
        SStringW strMsg = SStringW(L"[%d]未吃子，自动判和！",m_dwProps[PROPID_MAX_STP_PEACE]);
		BrdcstAckOver(GOT_PEACE,-1,strMsg);
	}else if(m_nLeftPassable==0)
	{//双方无可过河子
		WCHAR szMsg[100]=L"双方无可过河子，自动判和！";
		BrdcstAckOver(GOT_PEACE,-1,szMsg);
	}
	return TRUE;
}


//悔棋
BOOL CCnChess::OnRegret(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
    PMSG_REGRET pRegret = (PMSG_REGRET)pData;
	int iIndex=pRegret->iIndex;
    int iIndexOrder = pClient->m_nIndex;
	if(TABLE_STATE_PLAYING != m_state || iIndex!=iIndexOrder) return FALSE;
	if(m_regretRec[iIndex].nLeft<=0) return FALSE;
	//验证悔棋有效性
	CHSSIDE cs=(iIndex==GetRedSeat())?CS_RED:CS_BLACK;
	UINT nSteps=(m_layout.m_actSide==cs)?2:1;
	if(m_lstMoves.size()<nSteps) return FALSE;
	if((int)(m_lstMoves.size()-nSteps)<=m_regretRec[iIndex].iMoveStep)
	{//控制连续悔棋
        PWSCLIENT pPlayer=GetPlayer(iIndexOrder);
        NotifyClient(pPlayer, L"不能连续悔棋！");
		return FALSE;
	}
	for(UINT i=0;i<nSteps;i++)
	{
		MOVESTEP ms=m_lstMoves.back();
		m_layout.UndoMove(ms);
		m_lstMoves.pop_back();
	}
	m_nRegretSteps[iIndex]+=nSteps;

	m_regretRec[iIndex].nLeft--;
	m_regretRec[iIndex].iMoveStep=m_lstMoves.size();
	m_nChessMsg++;
	SendMsg(NULL, dwType, pData, dwSize); //广播消息给所有用户，包括自己。自己也会收到自己发出的消息。
	return TRUE;
}

BOOL CCnChess::OnResult(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
	PMSG_RESULT pMsgResult=(PMSG_RESULT)pData;
    int iIndexOrder = pClient->m_nIndex;
	if(pMsgResult->iIndex!=iIndexOrder || TABLE_STATE_PLAYING != m_state)
		return FALSE;
	if(!pMsgResult->bWin)
	{
		m_state = TABLE_STATE_FINISH;
		BrdcstAckOver(GOT_NORMAL,iIndexOrder,L"游戏结束");
	}
    return 0;
}

//用户求和
BOOL CCnChess::OnReqPeace(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
	PMSG_PEACE pPeace=(PMSG_PEACE)pData;
	if(m_lstMoves.size()<m_dwProps[PROPID_MIN_STEPS]) 
    {
		SStringW strMsg = SStringW().Format(L"不能在%d步内求和！",m_dwProps[PROPID_MIN_STEPS]);
        NotifyClient(pClient, strMsg);
        return FALSE;
	}    
    int iIndexOrder = pClient->m_nIndex;	//求和方座位号
	if(pPeace->iIndex!=iIndexOrder || TABLE_STATE_PLAYING != m_state) 
        return FALSE;
	if(m_nPeaceSeat==iIndexOrder && m_nChessMsg-m_nReqPeaceSteps<m_dwProps[PROPID_MIN_STEPS]){
        SStringW strMsg = SStringW().Format(L"不能在规定的最小步数内重复求和！(%d步)",m_dwProps[PROPID_MIN_STEPS]);
        NotifyClient(pClient, strMsg);
        return FALSE;
    }
	m_nPeaceSeat=iIndexOrder;
    m_nReqPeaceSteps = m_nChessMsg;
	SendMsg(NULL, dwType, pData, dwSize); //广播消息给所有用户，包括自己。自己也会收到自己发出的消息。	return TRUE;
	return TRUE;
}

//另一个用户对求和的回应
BOOL CCnChess::OnAckPeace(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
	if(m_nPeaceSeat==-1) 
        return FALSE;
	PMSG_PEACE pPeace=(PMSG_PEACE)pData;	//求和方座位号
	if(pPeace->iIndex!=m_nPeaceSeat) 
        return FALSE;	//求和方座位号不匹配，拒绝消息。
	int iIndexOrder = pClient->m_nIndex;	//回应方座位号
	//求和成功,游戏结束
	if(pPeace->nResult)
	{
        BrdcstAckOver(GOT_NORMAL,-1,L"和棋");
	}else{
        SendMsg(NULL, dwType, pData, dwSize);
    }
	return TRUE;
}


//用户超时
BOOL CCnChess::OnPlayerTimeout(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
    PMSG_TIMEOUT pTimeout = (PMSG_TIMEOUT)pData;
    int iIndexOrder = pClient->m_nIndex;
    if(pTimeout->iIndex != GetActiveSeat())
        return FALSE;
    time_t tSpan=time(NULL)-m_dwStartTime;
    if(tSpan<m_dwProps[PROPID_TIME_STEP]-1)
        return FALSE;

	PWSCLIENT pCheatPlayer=GetPlayer(pTimeout->iIndex);
    SOUI::SStringW strMsg = SStringW().Format(L"[%s]走棋超时，判负！", S_CA2W(pCheatPlayer->m_userInfo.szName,CP_UTF8).c_str());
	BrdcstAckOver(GOT_TIMEOUT,pTimeout->iIndex,strMsg);
	return TRUE;
}

CHSSIDE CCnChess::GetPlayerColor(int iIndex)
{
	if((iIndex%2)==GetRedSeat()) return CS_RED;
	else return CS_BLACK;
}

//用户投诉对手在长捉
BOOL CCnChess::OnReqLongCatch(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
	PMSG_LONGCATCH pLongCatch=(PMSG_LONGCATCH)pData;
    int iIndexOrder = pClient->m_nIndex;
	if(pLongCatch->iIndex==iIndexOrder || TABLE_STATE_PLAYING != m_state) 
        return FALSE;
	if(m_layout.m_actSide!=GetPlayerColor(iIndexOrder)) 
        return FALSE;//只能是当前操作方可以上传该消息
	CHESSMAN chsEnemy=CHSMAN_NULL;
	UINT nRepeatMove=GetFarthestRepeat(chsEnemy);
	if((pLongCatch->type==LCT_JIANG && (chsEnemy==CHSMAN_RED_JIANG || chsEnemy==CHSMAN_BLK_JIANG) && nRepeatMove>=m_dwProps[PROPID_MAX_STP_JIANG])
	||(pLongCatch->type==LCT_CATCH && chsEnemy!=CHSMAN_NULL && nRepeatMove>=m_dwProps[PROPID_MAX_STP_CATCH]))
	{//检测通过
		SendMsg(NULL, dwType, pData, dwSize); //广播消息给所有用户，包括自己。自己也会收到自己发出的消息。	return TRUE;
		m_goType=GOT_CHEAT;
        BrdcstAckOver(GOT_CHEAT,pLongCatch->iIndex,L"长捉");
		return TRUE;
	}else
	{
        BrdcstAckOver(GOT_CHEAT,iIndexOrder,L"投诉失败！");
		return FALSE;
	}
}

//计算当前步最远的一次重复走棋，在计算长捉时使用
UINT CCnChess::GetFarthestRepeat(CHESSMAN & chsEnemy)
{
	UINT nRet=0,nSteps=0;
	int	nChsID=-1,nChsEnemy=-1;
	POINT ptLastDest={-1,-1};
	POINT ptEnemy={-1,-1};

	CChessLayout layout;
	layout.Copy(&m_layout);
	CChsLytState state(&layout);
	CHSSIDE actSide=(CHSSIDE)!layout.m_actSide;

	chsEnemy=CHSMAN_NULL;

	std::vector<MOVESTEP>::iterator p=m_lstMoves.end();
	while(p!=m_lstMoves.begin())
	{
		p--;
		if(CHSMANSIDE(layout.m_chesses[p->pt2.y][p->pt2.x])==actSide)
		{
			state.UpdateState();
			nSteps++;
			//找到攻击的无根子数量
			CChsLytState::CChessInfo *pChessInfo=state.GetChessInfo(p->pt2);
			int nNoRootChs=0;
			POINT ptDest;
			std::vector<POINT>::iterator p2=pChessInfo->m_lstDest.begin();
			while(p2!=pChessInfo->m_lstDest.end())
			{
				if(CHSMANSIDE(layout.m_chesses[p2->y][p2->x])==layout.m_actSide
					&& state.GetProtecter(*p2)==0
					&& state.GetAttacter(*p2)==1)
				{
					nNoRootChs++;
					ptDest=*p2;
					if(nNoRootChs>1) break;
				}
				p2++;
			}
			if(nNoRootChs!=1) break;

			if(nChsID==-1)
			{//最后一步
				nChsID=layout.m_nChsID[p->pt2.y][p->pt2.x];
				nChsEnemy=layout.m_nChsID[ptDest.y][ptDest.x];
				ptLastDest=p->pt2;
				ptEnemy=ptDest;
				chsEnemy=layout.m_chesses[ptDest.y][ptDest.x];
			}else
			{
				//攻击及被攻击的棋子不是原棋或者退出计算
				if(nChsID!=layout.m_nChsID[p->pt2.y][p->pt2.x]
				|| nChsEnemy!=layout.m_nChsID[ptDest.y][ptDest.x] )
					break;
				if(p->pt2.x==ptLastDest.x && p->pt2.y==ptLastDest.y)
					nRet=nSteps;//发现重复
			}
		}else
		{
			//移动的棋子不是被攻击棋子
			if(layout.m_nChsID[p->pt2.y][p->pt2.x]!=nChsEnemy)
				break;
		}
		layout.UndoMove(*p);
	}
	return nRet;
}

SNSEND
