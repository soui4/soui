#include "stdafx.h"
#include "CnChess.h"
#include "GameClient.h"
#include <algorithm>
#include <helper/slog.h>
#define kLogTag "CCnChess"

CCnChess::CCnChess()
{
    m_clients.assign(2, nullptr); // 象棋只有两个玩家：红方和黑方
    m_state = CS_WAITSTART;
    
    // 初始化棋盘
    InitBoard();
    
    m_bRedTurn = TRUE; // 红方先行
    m_nMoveCount = 0;
    memset(m_moveHistory, 0, sizeof(m_moveHistory));
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
    // 象棋游戏的消息处理逻辑将在这里实现
    // 这里暂时留空，后续根据具体协议实现
    return TRUE;
}

PWSCLIENT* CCnChess::GetClients(int* nCount)
{
    *nCount = m_clients.size();
    return m_clients.data();
}

int CCnChess::GetState() const
{
    return m_state;
}

int CCnChess::GetPlayerCount() const
{
    return std::count_if(m_clients.begin(), m_clients.end(), [](PWSCLIENT pClient) {return pClient != NULL; });
}

void CCnChess::OnRemovePlayer(int seatId)
{
    SLOGI() << "remove player, seat id:" << seatId;
    if (seatId >= 0 && seatId < (int)m_clients.size()) {
        PWSCLIENT pClient = m_clients[seatId];
        if (!pClient) {
            SLOGW() << "player is null";
            return;
        }
        if (m_state == CS_RED_MOVE || m_state == CS_BLACK_MOVE) {
            OnPlayerLeave(seatId, pClient);
        }
        m_clients[seatId] = nullptr;
        delete pClient;
    }
}

void CCnChess::OnAddPlayer(int seatId, PWSCLIENT pClient)
{
    SLOGI() << "add player, seat id:" << seatId;
    if (seatId >= 0 && seatId < (int)m_clients.size() && m_clients[seatId] == NULL) {
        m_clients[seatId] = pClient;
    }
}

BOOL CCnChess::OnPlayerLeave(int seatId, PWSCLIENT pClient)
{
    SLOGI() << "player leave, seat id:" << seatId;
    // 玩家离开时的处理逻辑
    return TRUE;
}

BOOL CCnChess::HasPlayer(int seatId) const
{
    if (seatId >= 0 && seatId < (int)m_clients.size())
        return m_clients[seatId] != NULL;
    return FALSE;
}

int CCnChess::GetEmptySeat() const
{
    for (int i = 0; i < (int)m_clients.size(); i++)
    {
        if (m_clients[i] == NULL)
            return i;
    }
    return -1;
}

void CCnChess::OnGameStart()
{
    SLOGI() << "game start";
    m_state = CS_RED_MOVE;
    
    // 初始化棋盘
    InitBoard();
    
    m_bRedTurn = TRUE;
    m_nMoveCount = 0;
    memset(m_moveHistory, 0, sizeof(m_moveHistory));
    
    // 通知所有玩家游戏开始
    // 这里应该发送游戏开始的消息
}

void CCnChess::OnGameEnd()
{
    SLOGI() << "game end";
    m_state = CS_GAME_OVER;
    
    // 通知所有玩家游戏结束
    // 这里应该发送游戏结束的消息
}

void CCnChess::OnRoundEnd()
{
    // 象棋没有"回合"的概念，所以这个方法可以留空或者用于其他用途
}

void CCnChess::InitBoard()
{
    // 初始化棋盘状态
    // 0 表示空位
    // 1-7 表示红方棋子 (帅,士,象,马,车,炮,兵)
    // 8-14 表示黑方棋子 (将,士,象,马,车,炮,卒)
    
    memset(m_board, 0, sizeof(m_board));
    
    // 放置红方棋子 (底部)
    m_board[0][0] = 5; // 车
    m_board[1][0] = 4; // 马
    m_board[2][0] = 3; // 象
    m_board[3][0] = 2; // 士
    m_board[4][0] = 1; // 帅
    m_board[5][0] = 2; // 士
    m_board[6][0] = 3; // 象
    m_board[7][0] = 4; // 马
    m_board[8][0] = 5; // 车
    
    m_board[1][2] = 6; // 炮
    m_board[7][2] = 6; // 炮
    
    for (int i = 0; i < 9; i += 2)
    {
        m_board[i][3] = 7; // 兵
    }
    
    // 放置黑方棋子 (顶部)
    m_board[0][9] = 12; // 车
    m_board[1][9] = 11; // 马
    m_board[2][9] = 10; // 象
    m_board[3][9] = 9;  // 士
    m_board[4][9] = 8;  // 将
    m_board[5][9] = 9;  // 士
    m_board[6][9] = 10; // 象
    m_board[7][9] = 11; // 马
    m_board[8][9] = 12; // 车
    
    m_board[1][7] = 13; // 炮
    m_board[7][7] = 13; // 炮
    
    for (int i = 0; i < 9; i += 2)
    {
        m_board[i][6] = 14; // 卒
    }
}

BOOL CCnChess::IsValidMove(int fromX, int fromY, int toX, int toY)
{
    // 验证走棋是否合法
    // 这里需要实现具体的象棋规则
    
    // 检查坐标是否在棋盘范围内
    if (fromX < 0 || fromX >= 9 || fromY < 0 || fromY >= 10 ||
        toX < 0 || toX >= 9 || toY < 0 || toY >= 10)
    {
        return FALSE;
    }
    
    // 检查起始位置是否有棋子
    int piece = m_board[fromX][fromY];
    if (piece == 0)
    {
        return FALSE;
    }
    
    // 检查是否是当前玩家的回合
    BOOL isRedPiece = (piece >= 1 && piece <= 7);
    if (m_bRedTurn != isRedPiece)
    {
        return FALSE;
    }
    
    // 检查目标位置是否是己方棋子
    int targetPiece = m_board[toX][toY];
    if (targetPiece != 0)
    {
        BOOL isTargetRed = (targetPiece >= 1 && targetPiece <= 7);
        if (isRedPiece == isTargetRed)
        {
            return FALSE;
        }
    }
    
    // 这里应该添加每种棋子的具体移动规则
    // 为了简洁，暂时只做基本检查
    return TRUE;
}

void CCnChess::MakeMove(int fromX, int fromY, int toX, int toY)
{
    // 执行走棋
    if (m_nMoveCount < 256)
    {
        // 记录走棋历史
        m_moveHistory[m_nMoveCount][0] = fromX;
        m_moveHistory[m_nMoveCount][1] = fromY;
        m_moveHistory[m_nMoveCount][2] = toX;
        m_moveHistory[m_nMoveCount][3] = toY;
        m_nMoveCount++;
    }
    
    // 移动棋子
    m_board[toX][toY] = m_board[fromX][fromY];
    m_board[fromX][fromY] = 0;
    
    // 切换回合
    m_bRedTurn = !m_bRedTurn;
    m_state = m_bRedTurn ? CS_RED_MOVE : CS_BLACK_MOVE;
}

BOOL CCnChess::IsInCheck(BOOL bRed)
{
    // 检查是否被将军
    // 这里需要实现具体的将军检测逻辑
    return FALSE;
}

BOOL CCnChess::IsCheckmate(BOOL bRed)
{
    // 检查是否被将死
    // 这里需要实现具体的将死检测逻辑
    return FALSE;
}