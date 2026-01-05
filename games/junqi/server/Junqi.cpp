#include "stdafx.h"
#include "Junqi.h"
#include "GameClient.h"
#include "PropBag.h"
#include <algorithm>
#include <string/strcpcvt.h>
#include <helper/slog.h>
#include <time.h>
#define kLogTag "junqi"

SNSBEGIN

CJunQi::CJunQi(ITableListener* pListener, int nTableId)
    : CGameTable(pListener, nTableId, PLAYER_COUNT)
{
	m_jqState = JQ_STATE_LAYOUT;

    // 游戏状态初始化
    m_iActiveIndex = -1;
    m_nChessMsg = 0;
    m_nPeaceSeat = -1;
    m_nAckPeacePlayer = 0;
    m_nPeaceSteps = 0;
    m_goType = GOT_NORMAL;
    m_timeStart = 0;
    memset(m_alTime, 0, sizeof(m_alTime));

	    // 初始化棋盘布局 - 每方25个棋子
    int nMatStone[17][17];
    memset(nMatStone, 0, sizeof(nMatStone));
    
    // TODO: 初始化棋子布局
    // 这里应该从某个配置或预设布局初始化
    
    // 初始化布局
    m_layout.InitLayout(nMatStone);
    m_layout.SetProperty(PLAYER_COUNT,  PropBag::getSingletonPtr()->GetProp(PI_BOMBFLY), PropBag::getSingletonPtr()->GetProp(PI_MINEASBOMB));

}

CJunQi::~CJunQi()
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

void CJunQi::Boardcast(DWORD dwType, LPVOID pData, DWORD dwSize)
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

BOOL CJunQi::SendMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
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

BOOL CJunQi::OnMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
    switch (dwType)
    {
    case MSG_REQ_MOVE:
        return OnChessMove(pClient, dwType, pData, dwSize);
    case MSG_REQ_SURRENDER:
        return OnPlayerSurrender(pClient, dwType, pData, dwSize);
    case MSG_REQ_PASS:
        return OnPlayerPass(pClient, dwType, pData, dwSize);
    case MSG_REQ_PEACE:
        return OnReqPeace(pClient, dwType, pData, dwSize);
    case MSG_ACK_PEACE:
        return OnAckPeace(pClient, dwType, pData, dwSize);
	case GLID_JQSTARTFIGHT:
		return OnStartFight(pClient, dwType, pData, dwSize);
    }
    return TRUE;
}

void CJunQi::OnGameStart()
{
    CGameTable::OnGameStart();
    SLOGI() << "Game start";
    
    
	m_iActiveIndex = rand() % PLAYER_COUNT;
    // 记录游戏开始时间
    m_timeStart = time(NULL);
    
    // 初始化时间统计
    memset(m_alTime, 0, sizeof(m_alTime));
    
    // 初始化游戏状态
    m_nChessMsg = 0;
    m_nPeaceSeat = -1;
    m_nPeaceSteps = 0;
    m_goType = GOT_NORMAL;
    m_lstMoves.clear();
}

void CJunQi::OnGameEnd()
{
    CGameTable::OnGameEnd();
    SLOGI() << "Game end, type: " << (int)m_goType;
}

BOOL CJunQi::OnPlayerLeave(int seatId, PWSCLIENT pClient)
{
    int state = m_state;
	m_clients[seatId] = nullptr;
    if (state == TABLE_STATE_PLAYING)
    {
		m_layout.PlayerLose(seatId);
		MSG_PLAYERLOSE msgPlayerLose;
		msgPlayerLose.iIndex = seatId;
		SendMsg(NULL, MSG_NOTIFY_PLAYER_LOSE, &msgPlayerLose, sizeof(MSG_PLAYERLOSE));
		int iWinner;
		if(IsGameWon(iWinner)){
			BrdcstAckOver(GOT_NORMAL, iWinner, L"游戏结束");
		}else if(seatId == m_iActiveIndex){
			NotifyNextActiveMove();
		}
    }
    return TRUE;
}

/**
 * 获取指定座位的玩家的布局数据
 * @param seatId 座位号
 * @param byLayout 布局数据
 * @param nSide 玩家的阵营
 */
void CJunQi::GetPlayerLayoutForSeat(int seatId, BYTE byLayout[13], int nSide)
{
	int rule = PropBag::getSingletonPtr()->GetProp(PI_RULE);
	if(rule == RULE_QMQ || (rule == RULE_SMS && seatId % 2 == nSide % 2)){
		m_layout.MakeLayout(nSide,byLayout);
	}else{
		memset(byLayout, 0xdd, 13);
	}
}
void CJunQi::OnAddPlayer(int seatId, PWSCLIENT pClient)
{
    CGameTable::OnAddPlayer(seatId,pClient);
	for(int i = 0; i < PLAYER_COUNT; i++){
		if(i != seatId && m_clients[i] &&  m_clients[i]->m_dwState == 1){
			//send ready message to new player
			MSGSTARTFIGHT msgLayout;
			msgLayout.iIndex = i;
			GetPlayerLayoutForSeat(seatId, msgLayout.byLayout, i);
			SendMsg(pClient, GLID_JQSTARTFIGHT, &msgLayout, sizeof(MSGSTARTFIGHT));
		}
	}
}
int CJunQi::GetActiveSeat() const
{
	return m_iActiveIndex;
}

BOOL CJunQi::IsGameWon(int& nWinnerSeat)
{
    int nLosers = 0;
    for (int i = 0; i < PLAYER_COUNT; i++)
    {
        if (m_layout.IsPlayerLost(i))
        {
            nLosers++;
        }
    }
    
    if (nLosers == 3)
    {// 如果有3方失败，则剩下的1方赢
        for (int i = 0; i < PLAYER_COUNT; i++)
        {
            if (!m_layout.IsPlayerLost(i))
            {
                nWinnerSeat = i;
                return TRUE;
            }
        }
    }else if(nLosers == 2)
	{// 如果有2方失败，则检查2家是不是对家
		int losers[2];
		int n = 0;
		for (int i = 0; i < PLAYER_COUNT; i++)
        {
            if (m_layout.IsPlayerLost(i))
            {
                losers[n++] = i;
            }
        }
		if(losers[0] %2 == losers[1]%2){
			nWinnerSeat = (losers[0]+1)%PLAYER_COUNT;
			return TRUE;
		}
	}
    
    return FALSE;
}

void CJunQi::NotifyNextActiveMove(){
	int iActiveIndex = m_iActiveIndex;
	for(int i=0;i<PLAYER_COUNT;i++){
		iActiveIndex = (iActiveIndex + 1) % PLAYER_COUNT;
		if(!m_layout.IsPlayerLost(iActiveIndex)){
			break;
		}
	}
	SASSERT(iActiveIndex != m_iActiveIndex);
	m_iActiveIndex = iActiveIndex;
	MSG_PLAYERACTIVE msg;
	msg.iIndex = m_iActiveIndex;
	SendMsg(NULL,MSG_NOTIFY_PLAYER_ACTIVE,&msg,sizeof(msg));
}

// 处理棋子移动
BOOL CJunQi::OnChessMove(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
    if (m_state != TABLE_STATE_PLAYING)
        return FALSE;
    
    PMSG_MOVE pMsgMove = (PMSG_MOVE)(pData);
    int iIndexOrder = pClient->m_nIndex;
    
    // 检查是否是当前玩家的回合
    if (iIndexOrder != GetActiveSeat())
    {
        SLOGW() << "Invalid move - not player's turn. Seat=" << iIndexOrder;
        return FALSE;
    }
	// 使用JunqiLayout的方法验证移动是否有效
	BOOL bValid = m_layout.IsValidMove(pMsgMove->ptBegin, pMsgMove->ptEnd, iIndexOrder);
	if(!bValid)
	{
		SLOGW() << "Invalid move. Seat=" << iIndexOrder;
		BrdcstAckOver(GOT_CHEAT, iIndexOrder, L"作弊！");
		return TRUE;
	}
	int nRank1 = m_layout.GetCellChessOffRank(pMsgMove->ptBegin);
	int nRank2 = m_layout.GetCellChessOffRank(pMsgMove->ptEnd);
	int nSide2 = m_layout.GetCellChess(pMsgMove->ptEnd).nSide;
	int result = m_layout.MoveChessmanServer(pMsgMove->ptBegin, pMsgMove->ptEnd);
	pMsgMove->result = result;
    // 广播移动消息给所有玩家
    SendMsg(NULL, dwType, pData, dwSize);

	if(result == PUT_BOMB){
		// 如何司令死亡，则报告军旗位置
		if(nRank1 == OR_CHIEF){
			MSG_FLAGPOS msgFlagPos;
			msgFlagPos.iIndex = iIndexOrder;
			msgFlagPos.pt = m_layout.GetFlagPosition(iIndexOrder);
			SendMsg(NULL, MSG_NOTIFY_FLAGPOS, &msgFlagPos, sizeof(MSG_FLAGPOS));
		}
		if(nRank2 == OR_CHIEF){
			MSG_FLAGPOS msgFlagPos;
			msgFlagPos.iIndex = nSide2;
			msgFlagPos.pt = m_layout.GetFlagPosition(nSide2);
			SendMsg(NULL, MSG_NOTIFY_FLAGPOS, &msgFlagPos, sizeof(MSG_FLAGPOS));
		}
	}
	if(nRank2 == OR_FLAG){
		m_layout.PlayerLose(nSide2);
		MSG_PLAYERLOSE msgPlayerLose;
		msgPlayerLose.iIndex = nSide2;
		SendMsg(NULL, MSG_NOTIFY_PLAYER_LOSE, &msgPlayerLose, sizeof(MSG_PLAYERLOSE));
	}
    // 检查游戏是否结束
    int nWinnerSeat = -1;
    if (IsGameWon(nWinnerSeat))
    {
        BrdcstAckOver(GOT_NORMAL, nWinnerSeat, L"游戏结束");
        return TRUE;
    }
	// 更新未吃子步数
    if(result == PUT_MOVE)
		m_nPeaceSteps++;
	else
		m_nPeaceSteps = 0;
    // 检查是否自动判和（未吃子步数过多）
    if (m_nPeaceSteps >= PropBag::getSingletonPtr()->GetProp(PI_PEACESTEPS))
    {
        BrdcstAckOver(GOT_PEACE, -1, L"连续未吃子，自动判和");
        return TRUE;
    }
	NotifyNextActiveMove();
	
    return TRUE;
}

// 用户投降
BOOL CJunQi::OnPlayerSurrender(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
    SLOGI() << "OnPlayerSurrender";
    if (m_state != TABLE_STATE_PLAYING)
        return FALSE;
	MSG_SURRENDER *pSurrender = (MSG_SURRENDER *)pData;
	m_layout.PlayerLose(pSurrender->iIndex);
	SendMsg(NULL, dwType, pData, dwSize);

	// 检查游戏是否结束
    int nWinnerSeat = -1;
    if (IsGameWon(nWinnerSeat))
    {
        BrdcstAckOver(GOT_NORMAL, nWinnerSeat, L"游戏结束");
    }else if(pSurrender->iIndex == m_iActiveIndex){
		NotifyNextActiveMove();
	}
    return TRUE;
}

// 求和请求
BOOL CJunQi::OnReqPeace(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
    if (m_state != TABLE_STATE_PLAYING)
        return FALSE;
    
    PMSG_PEACE pPeace = (PMSG_PEACE)pData;
    int iIndexOrder = pClient->m_nIndex;
    
    if (pPeace->iIndex != iIndexOrder)
        return FALSE;
    
    // 最少走10步才能求和
    if (m_lstMoves.size() < PropBag::getSingletonPtr()->GetProp(PROPID_PEACESTEPS))
    {
        NotifyClient(pClient, L"游戏未进行足够的步数，不能求和！");
        return FALSE;
    }
    
    m_nPeaceSeat = iIndexOrder;
    m_nAckPeacePlayer = 0;
    
    SendMsg(NULL, dwType, pData, dwSize);
    return TRUE;
}

// 对求和的回应
BOOL CJunQi::OnAckPeace(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
    if (m_nPeaceSeat == -1)
        return FALSE;
    
    PMSG_PEACE pPeace = (PMSG_PEACE)pData;
    int iIndexOrder = pClient->m_nIndex;
    
    if (pPeace->iIndex == m_nPeaceSeat)
        return TRUE;  // 自己不能回应自己的求和请求
    // 求和成功
    if (pPeace->nResult == 1)
    {
	    m_nAckPeacePlayer++;
		if(m_nAckPeacePlayer == PLAYER_COUNT-1){
			m_nPeaceSeat = -1;
			m_nAckPeacePlayer = 0;
        	BrdcstAckOver(GOT_PEACE, -1, L"和棋");
		}
    }
    else
    {
        m_nPeaceSeat = -1;
		m_nAckPeacePlayer = 0;
        SendMsg(NULL, dwType, pData, dwSize);
    }
    
    return TRUE;
}

// 超时处理
BOOL CJunQi::OnPlayerPass(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
    PMSG_PASS pTimeout = (PMSG_PASS)pData;
    int iIndexOrder = pClient->m_nIndex;
    if(iIndexOrder != pTimeout->iIndex)
        return FALSE;
    if (pTimeout->iIndex != GetActiveSeat())
        return FALSE;
    
    if (m_state != TABLE_STATE_PLAYING)
        return FALSE;
    m_nPassCount[iIndexOrder]++;
	if(m_nPassCount[iIndexOrder] >=  PropBag::getSingletonPtr()->GetProp(PI_PASS)){
		m_layout.PlayerLose(iIndexOrder);
		MSG_PLAYERLOSE msgPlayerLose;
		msgPlayerLose.iIndex = iIndexOrder;
		SendMsg(NULL, MSG_NOTIFY_PLAYER_LOSE, &msgPlayerLose, sizeof(MSG_PLAYERLOSE));
		int iWinerSeat = -1;
		if(IsGameWon(iWinerSeat)){
			BrdcstAckOver(GOT_NORMAL, iWinerSeat, L"游戏结束");
			return TRUE;
		}
	}
	NotifyNextActiveMove();
    return TRUE;
}

BOOL CJunQi::OnStartFight(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
	if (m_jqState != JQ_STATE_LAYOUT)
		return FALSE;
	SASSERT(dwSize>=sizeof(MSGSTARTFIGHT));	// 布局数据不合法，直接返回失败
	MSGSTARTFIGHT *pStart = (MSGSTARTFIGHT *)pData;
	int iIndexOrder = pStart->iIndex;
    if (!m_layout.InitLayout(pStart->iIndex, pStart->byLayout))
    {
        SLOGW() << "!!init layout failed! index=" << pStart->iIndex;
        return FALSE;
    }
	// 记录布局
	memcpy(m_byLayout[iIndexOrder], pStart->byLayout, 13);

	pClient->m_dwState = 1;
    int rule = PropBag::getSingletonPtr()->GetProp(PI_RULE);
    MSGSTARTFIGHT msgFake;
    msgFake.iIndex = iIndexOrder;
    memset(msgFake.byLayout, 0xdd, 13);
    for (int i = 0; i < PLAYER_COUNT; i++)
    {
        if (m_clients[i] && i != iIndexOrder)
        {
            if (rule == RULE_SMS && i % 2 == iIndexOrder % 2 || rule == RULE_QMQ)
            {
                SendMsg(m_clients[i], GLID_JQSTARTFIGHT, pData, dwSize);
            }
            else
            {
                SendMsg(m_clients[i], GLID_JQSTARTFIGHT, &msgFake, sizeof(MSGSTARTFIGHT)); // 发送假数据，让客户端显示布局
            }
        }
    }
    BOOL bAllReady = TRUE;
	for(int i=0;i<PLAYER_COUNT;i++){
		if(!m_clients[i] || m_clients[i]->m_dwState == 0){
			bAllReady = FALSE;
			break;
		}
	}
	if(bAllReady){
		m_jqState = JQ_STATE_FIGHTING;
		for(int i=0;i<PLAYER_COUNT;i++){
			if(m_clients[i]){
				m_clients[i]->m_dwState = 0;
			}
			m_nPassCount[i]=0;
		}
		m_iActiveIndex = rand() % PLAYER_COUNT;	// 随机选择一个玩家先手
		MSGALLSTART msgStart;
		msgStart.iFstIndex = m_iActiveIndex;
		SendMsg(NULL, GLID_JQALLSTARTED, &msgStart, sizeof(msgStart));
	}
	return TRUE;
}

void CJunQi::BrdcstAckOver(GAMEOVERTYPE nType, int nLossSeat, LPCWSTR pszDesc)
{
    SLOGI() << "BrdcstAckOver: type=" << (int)nType << " lossSeat=" << nLossSeat;
    
    MSG_GAMEOVER msg;
	memcpy(msg.byLayout, m_byLayout, sizeof(msg.byLayout));
    msg.iWinner = (nLossSeat == -1) ? -1 : nLossSeat;
    SStringA strDesc = S_CW2A(pszDesc, CP_UTF8);
    strcpy_s(msg.szDesc, sizeof(msg.szDesc), strDesc.c_str());
    
    SendMsg(NULL, MSG_ACK_GAMEOVER, &msg, sizeof(MSG_GAMEOVER));
    m_goType = nType;
    EndGame(nType, pszDesc);
}


SNSEND
