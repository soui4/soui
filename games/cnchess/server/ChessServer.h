#pragma once

#include "WebSocketGame.h"
#include "ChessGame.h"

// 象棋游戏服务器类
class CChessServer : public CWebSocketGame
{
public:
    CChessServer();
    virtual ~CChessServer();

    // 游戏相关方法
    virtual BOOL GameStart(unsigned short uPort) override;
    virtual void GameStop() override;

protected:
    // 消息处理方法
    virtual BOOL OnMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize) override;
    
    // 客户端消息处理
    BOOL ClientMove(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
    BOOL ClientUndoRequest(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
    BOOL ClientDrawRequest(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
    BOOL ClientSurrender(PWSCLIENT pClient, LPVOID pData, DWORD dwSize);
    
private:
    // 服务器状态
    BOOL m_bRunning;
};