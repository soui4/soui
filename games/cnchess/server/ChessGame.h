#pragma once

#include "GameTable-i.h"
#include <vector>

// 象棋游戏状态枚举
enum ChessGameState
{
    CGS_WAITING = 0,     // 等待玩家加入
    CGS_RED_TURN,        // 红方行棋
    CGS_BLACK_TURN,      // 黑方行棋
    CGS_GAME_OVER,       // 游戏结束
    CGS_DRAW_REQUESTED,  // 请求和棋
    CGS_UNDO_REQUESTED   // 请求悔棋
};

// 象棋游戏桌子类
class CChessGame : public IGameTable
{
public:
    CChessGame();
    virtual ~CChessGame();

    // IGameTable接口实现
    virtual void Boardcast(DWORD dwType, LPVOID pData, DWORD dwSize) override;
    virtual BOOL SendMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize) override;
    virtual BOOL OnMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize) override;
    virtual void OnRemovePlayer(int seatId) override;
    virtual void OnAddPlayer(int seatId, PWSCLIENT pClient) override;
    virtual BOOL OnPlayerLeave(int seatId, PWSCLIENT pClient) override;
    virtual PWSCLIENT* GetClients(int* nCount) override;
    virtual int GetPlayerCount() const override;
    virtual int GetState() const override;
    virtual BOOL HasPlayer(int seatId) const override;
    virtual int GetEmptySeat() const override;
    virtual void OnGameStart() override;
    virtual void OnGameEnd() override;
    virtual void OnRoundEnd() override;

protected:
    // 游戏逻辑方法
    void InitializeBoard();
    BOOL IsValidMove(int fromX, int fromY, int toX, int toY, bool isRed);
    BOOL IsInCheck(bool isRed);
    BOOL IsCheckmate(bool isRed);
    void MakeMove(int fromX, int fromY, int toX, int toY);
    void UndoMove();

private:
    // 游戏状态数据
    int m_board[9][10];            // 棋盘状态 (9x10)
    ChessGameState m_gameState;    // 游戏状态
    bool m_isRedTurn;              // 是否轮到红方
    int m_moveHistory[256][4];     // 走棋历史记录
    int m_moveCount;               // 走棋步数
    
    // 玩家数据
    PWSCLIENT m_players[2];        // 两位玩家 (红方和黑方)
    int m_playerCount;             // 当前玩家数量
    
    // 游戏参数
    int m_roomId;                  // 房间ID
};