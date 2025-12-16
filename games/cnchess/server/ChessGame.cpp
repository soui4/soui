#include "stdafx.h"
#include "ChessGame.h"

CChessGame::CChessGame()
    : m_gameState(CGS_WAITING)
    , m_isRedTurn(true)
    , m_moveCount(0)
    , m_playerCount(0)
    , m_roomId(-1)
{
    // 初始化玩家指针
    m_players[0] = nullptr;
    m_players[1] = nullptr;
    
    // 初始化棋盘
    InitializeBoard();
}

CChessGame::~CChessGame()
{
}

void CChessGame::Boardcast(DWORD dwType, LPVOID pData, DWORD dwSize)
{
    // 广播消息给所有玩家
    for (int i = 0; i < 2; i++)
    {
        if (m_players[i])
        {
            SendMsg(m_players[i], dwType, pData, dwSize);
        }
    }
}

BOOL CChessGame::SendMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
    // 发送消息给指定客户端
    // 这里应该调用网络层的发送函数
    return TRUE;
}

BOOL CChessGame::OnMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
    // 处理来自客户端的消息
    return TRUE;
}

void CChessGame::OnRemovePlayer(int seatId)
{
    // 移除玩家
    if (seatId >= 0 && seatId < 2 && m_players[seatId])
    {
        m_players[seatId] = nullptr;
        m_playerCount--;
        
        // 如果玩家都离开了，游戏结束
        if (m_playerCount == 0)
        {
            OnGameEnd();
        }
    }
}

void CChessGame::OnAddPlayer(int seatId, PWSCLIENT pClient)
{
    // 添加玩家
    if (seatId >= 0 && seatId < 2 && !m_players[seatId])
    {
        m_players[seatId] = pClient;
        m_playerCount++;
        
        // 如果两个玩家都加入了，开始游戏
        if (m_playerCount == 2)
        {
            OnGameStart();
        }
    }
}

BOOL CChessGame::OnPlayerLeave(int seatId, PWSCLIENT pClient)
{
    // 玩家离开处理
    if (seatId >= 0 && seatId < 2 && m_players[seatId] == pClient)
    {
        OnRemovePlayer(seatId);
        return TRUE;
    }
    return FALSE;
}

PWSCLIENT* CChessGame::GetClients(int* nCount)
{
    // 返回客户端列表
    if (nCount)
    {
        *nCount = m_playerCount;
    }
    return m_players;
}

int CChessGame::GetPlayerCount() const
{
    return m_playerCount;
}

int CChessGame::GetState() const
{
    return m_gameState;
}

BOOL CChessGame::HasPlayer(int seatId) const
{
    if (seatId >= 0 && seatId < 2)
    {
        return (m_players[seatId] != nullptr);
    }
    return FALSE;
}

int CChessGame::GetEmptySeat() const
{
    // 查找空座位
    for (int i = 0; i < 2; i++)
    {
        if (!m_players[i])
        {
            return i;
        }
    }
    return -1; // 没有空座位
}

void CChessGame::OnGameStart()
{
    // 游戏开始
    m_gameState = CGS_RED_TURN;
    m_isRedTurn = true;
    m_moveCount = 0;
    
    // 初始化棋盘
    InitializeBoard();
    
    // 通知所有玩家游戏开始
    // 这里应该发送游戏开始消息
}

void CChessGame::OnGameEnd()
{
    // 游戏结束
    m_gameState = CGS_GAME_OVER;
    
    // 通知所有玩家游戏结束
    // 这里应该发送游戏结束消息
}

void CChessGame::OnRoundEnd()
{
    // 回合结束，这里可能不需要特别处理
}

void CChessGame::InitializeBoard()
{
    // 初始化棋盘状态
    // 0 表示空位
    // 1-7 表示红方棋子 (帅,士,象,马,车,炮,兵)
    // 8-14 表示黑方棋子 (将,士,象,马,车,炮,卒)
    
    memset(m_board, 0, sizeof(m_board));
    
    // 放置红方棋子
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
    
    // 放置黑方棋子
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

BOOL CChessGame::IsValidMove(int fromX, int fromY, int toX, int toY, bool isRed)
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
    
    // 检查是否是当前玩家的棋子
    bool isRedPiece = (piece >= 1 && piece <= 7);
    if (isRed != isRedPiece)
    {
        return FALSE;
    }
    
    // 检查目标位置是否是己方棋子
    int targetPiece = m_board[toX][toY];
    if (targetPiece != 0)
    {
        bool isTargetRed = (targetPiece >= 1 && targetPiece <= 7);
        if (isRed == isTargetRed)
        {
            return FALSE;
        }
    }
    
    // 这里应该添加每种棋子的具体移动规则
    // 为了简洁，暂时只做基本检查
    return TRUE;
}

BOOL CChessGame::IsInCheck(bool isRed)
{
    // 检查是否被将军
    // 这里需要实现具体的将军检测逻辑
    return FALSE;
}

BOOL CChessGame::IsCheckmate(bool isRed)
{
    // 检查是否被将死
    // 这里需要实现具体的将死检测逻辑
    return FALSE;
}

void CChessGame::MakeMove(int fromX, int fromY, int toX, int toY)
{
    // 执行走棋
    if (m_moveCount < 256)
    {
        // 记录走棋历史
        m_moveHistory[m_moveCount][0] = fromX;
        m_moveHistory[m_moveCount][1] = fromY;
        m_moveHistory[m_moveCount][2] = toX;
        m_moveHistory[m_moveCount][3] = toY;
        m_moveCount++;
    }
    
    // 移动棋子
    m_board[toX][toY] = m_board[fromX][fromY];
    m_board[fromX][fromY] = 0;
    
    // 切换回合
    m_isRedTurn = !m_isRedTurn;
    m_gameState = m_isRedTurn ? CGS_RED_TURN : CGS_BLACK_TURN;
}

void CChessGame::UndoMove()
{
    // 悔棋
    if (m_moveCount > 0)
    {
        m_moveCount--;
        int fromX = m_moveHistory[m_moveCount][0];
        int fromY = m_moveHistory[m_moveCount][1];
        int toX = m_moveHistory[m_moveCount][2];
        int toY = m_moveHistory[m_moveCount][3];
        
        // 恢复棋子位置
        m_board[fromX][fromY] = m_board[toX][toY];
        m_board[toX][toY] = 0;
        
        // 切换回合
        m_isRedTurn = !m_isRedTurn;
        m_gameState = m_isRedTurn ? CGS_RED_TURN : CGS_BLACK_TURN;
    }
}