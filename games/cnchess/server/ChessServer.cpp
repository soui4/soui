#include "stdafx.h"
#include "ChessServer.h"
#include "CnChess.h" // 引入中国象棋逻辑类

CChessServer::CChessServer()
    : m_bRunning(FALSE)
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
    // 处理客户端走棋请求
    // 这里需要验证走法合法性并广播给其他客户端
    return TRUE;
}

BOOL CChessServer::ClientUndoRequest(PWSCLIENT pClient, LPVOID pData, DWORD dwSize)
{
    // 处理客户端悔棋请求
    return TRUE;
}

BOOL CChessServer::ClientDrawRequest(PWSCLIENT pClient, LPVOID pData, DWORD dwSize)
{
    // 处理客户端和棋请求
    return TRUE;
}

BOOL CChessServer::ClientSurrender(PWSCLIENT pClient, LPVOID pData, DWORD dwSize)
{
    // 处理客户端认输消息
    return TRUE;
}