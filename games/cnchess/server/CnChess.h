#ifndef __CNCHESSTABLE_H__
#define __CNCHESSTABLE_H__

#include "GameTable-i.h"
#include <helper/obj-ref-impl.hpp>
#include <vector>
#include "Game.h"
#include <cnchessProtocol.h>

SNSBEGIN

typedef class GameClient WSCLIENT, *PWSCLIENT;

// 象棋游戏状态
enum tagChessState
{
    CS_WAITSTART = 0,   // 等待开始
    CS_RED_MOVE,        // 红方走棋
    CS_BLACK_MOVE,      // 黑方走棋
    CS_GAME_OVER,       // 游戏结束
    CS_DRAW_REQUESTED,  // 请求和棋
    CS_UNDO_REQUESTED   // 请求悔棋
};

typedef tagChessState ChessState;

class CCnChess : public CGameTable
{
public:
    CCnChess();
    virtual ~CCnChess();

    // IGameTable接口实现
    void Boardcast(DWORD dwType, LPVOID pData, DWORD dwSize) override;
    BOOL SendMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize) override;
    BOOL OnMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize) override;
    void OnRemovePlayer(int seatId) override;
    void OnAddPlayer(int seatId, PWSCLIENT pClient) override;
    BOOL OnPlayerLeave(int seatId, PWSCLIENT pClient) override;
    PWSCLIENT* GetClients(int* nCount) override;
    int GetPlayerCount() const override;
    int GetState() const override;
    BOOL HasPlayer(int seatId) const override;
    int GetEmptySeat() const override;
    void OnGameStart() override;
    void OnGameEnd() override;
    void OnRoundEnd() override;

protected:
    // 象棋游戏相关方法
    void InitBoard();
    BOOL IsValidMove(int fromX, int fromY, int toX, int toY);
    void MakeMove(int fromX, int fromY, int toX, int toY);
    BOOL IsInCheck(BOOL bRed);
    BOOL IsCheckmate(BOOL bRed);

protected:
    ChessState m_state;

    // 象棋游戏数据
    int m_board[9][10];     // 棋盘 (x: 0-8, y: 0-9)
    BOOL m_bRedTurn;        // TRUE表示轮到红方
    int m_nMoveCount;       // 走棋步数
    int m_moveHistory[256][4]; // 走棋历史记录 [fromX, fromY, toX, toY]
};

SNSEND

#endif // !__CNCHESSTABLE_H__