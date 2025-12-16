#include "stdafx.h"
#include "ChessServer.h"
#include "CnChess.h" // 引入中国象棋逻辑类

CChessServer::CChessServer()
    : m_bRunning(FALSE), m_ChessGame()
{
}

// 假设在头文件中已声明: CCnChess m_ChessGame;

CChessServer::~CChessServer()
{
}

BOOL CChessServer::GameStart(unsigned short uPort)
{
    m_bRunning = TRUE;
    return CWebSocketGame::GameStart(uPort);
}

void CChessServer::GameStop()
{
    m_bRunning = FALSE;
    CWebSocketGame::GameStop();
}

BOOL CChessServer::OnMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize)
{
    switch (dwType)
    {
    case CHESS_MSG_MOVE: // 处理走棋消息
        return ClientMove(pClient, pData, dwSize);
        
    case CHESS_MSG_UNDO_REQUEST: // 处理悔棋请求
        return ClientUndoRequest(pClient, pData, dwSize);
        
    case CHESS_MSG_DRAW_REQUEST: // 处理和棋请求
        return ClientDrawRequest(pClient, pData, dwSize);
        
    case CHESS_MSG_SURRENDER: // 处理认输消息
        return ClientSurrender(pClient, pData, dwSize);
        
    default:
        // 调用父类的消息处理
        return CWebSocketGame::OnMsg(pClient, dwType, pData, dwSize);
    }
}

BOOL CChessServer::ClientMove(PWSCLIENT pClient, LPVOID pData, DWORD dwSize)
{
    if (dwSize != sizeof(MoveData)) // 假设 MoveData 是走法数据结构
        return FALSE;

    MoveData* pMove = static_cast<MoveData*>(pData);
    
    // 使用 CCnChess 验证并执行走法
    if (!m_ChessGame.DoMove(*pMove))
        return FALSE; // 走法不合法

    // 广播走法给所有客户端
    BroadcastMessage(CHESS_MSG_MOVE, pMove, dwSize);

    return TRUE;
}

BOOL CChessServer::ClientUndoRequest(PWSCLIENT pClient, LPVOID pData, DWORD dwSize)
{
    // 可以记录请求，等待双方同意
    // 此处简化为直接尝试悔棋（实际应用需协商）
    if (m_ChessGame.CanUndo())
    {
        m_ChessGame.UndoMove();
        BroadcastMessage(CHESS_MSG_UNDO_REQUEST, nullptr, 0); // 通知悔棋成功
    }
    return TRUE;
}

BOOL CChessServer::ClientDrawRequest(PWSCLIENT pClient, LPVOID pData, DWORD dwSize)
{
    // 通常需要对方同意，此处仅广播请求
    BroadcastMessage(CHESS_MSG_DRAW_REQUEST, nullptr, 0);
    return TRUE;
}

BOOL CChessServer::ClientSurrender(PWSCLIENT pClient, LPVOID pData, DWORD dwSize)
{
    m_ChessGame.SetGameOver(); // 标记游戏结束
    BroadcastMessage(CHESS_MSG_SURRENDER, nullptr, 0); // 广播认输
    return TRUE;
}