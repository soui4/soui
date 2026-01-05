#ifndef __JUNQI_TABLE_H__
#define __JUNQI_TABLE_H__

#include "GameTable-i.h"
#include <helper/obj-ref-impl.hpp>
#include <vector>
#include "Game.h"
#include <junqiProtocol.h>
#include <JunqiLayout.h>
#include <memory>

SNSBEGIN

class CJunQi : public CGameTable
{
	typedef enum JunqiState{
		JQ_STATE_LAYOUT=0,
		JQ_STATE_FIGHTING,
		JQ_STATE_OVER,
	}JunqiState;
public:
    CJunQi(ITableListener* pListener, int nTableId);
    virtual ~CJunQi();

    // IGameTable接口实现
    void Boardcast(DWORD dwType, LPVOID pData, DWORD dwSize) override;
    BOOL SendMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize) override;
    BOOL OnMsg(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize) override;
    void OnGameStart() override;
    void OnGameEnd() override;
    void OnRoundEnd() override{}
    BOOL OnPlayerLeave(int seatId, PWSCLIENT pClient) override;
    void GetPlayerLayoutForSeat(int seatId, BYTE byLayout[13], int nSide);
    void OnAddPlayer(int seatId, PWSCLIENT pClient) override;

  protected:
    // 游戏核心方法
    void BrdcstAckOver(GAMEOVERTYPE nType, int nLossSeat, LPCWSTR pszDesc);

    // 消息处理方法
    BOOL OnPlayerSurrender(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize);
    BOOL OnChessMove(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize);
    BOOL OnReqPeace(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize);
    BOOL OnAckPeace(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize);
    BOOL OnPlayerPass(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize);
	BOOL OnStartFight(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize);
    // 辅助方法
    int GetActiveSeat() const;			// 获取当前活跃玩家座位
    BOOL IsGameWon(int& nWinnerSeat);   // 检查游戏是否有胜者

    void NotifyNextActiveMove();

  protected:
    CJunQiLayout m_layout;				// 棋盘布局
    JunqiState m_jqState;
    // 游戏状态
    GAMEOVERTYPE m_goType;				// 游戏结束类型
    time_t m_timeStart;					// 游戏开始时间
    time_t m_alTime[PLAYER_COUNT];		// 各玩家用时
    int    m_nPassCount[PLAYER_COUNT];	// 各玩家过牌次数
	BYTE   m_byLayout[PLAYER_COUNT][13];// 各玩家布局
    // 棋子移动历史
    std::vector<MSG_MOVE> m_lstMoves;	// 移动历史记录
    int m_nChessMsg;					// 移动消息计数
    
    // 求和相关
    int m_nPeaceSeat;					// 当前求和方座位
    int m_nAckPeacePlayer;				// 同意求和的玩家数量
    UINT m_nPeaceSteps;					// 未吃子步数
    
    // 当前活跃座位
    int m_iActiveIndex;					// 当前活跃座位号
};

SNSEND

#endif // !__JUNQI_TABLE_H__