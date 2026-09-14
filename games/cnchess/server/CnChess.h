#ifndef __CNCHESSTABLE_H__
#define __CNCHESSTABLE_H__

#include "GameTable-i.h"
#include <helper/obj-ref-impl.hpp>
#include <vector>
#include "Game.h"
#include <cnchessProtocol.h>
#include <ChessLayout.h>
#include <memory>

SNSBEGIN

//悔棋记录
typedef struct tagREGRETREC
{
	int nLeft;		//剩余次数
	int iMoveStep;	//上次悔棋时消息序号
}REGRETREC;	


class CCnChess : public CGameTable
{
public:
    CCnChess(ITableListener* pListener, int nTableId);
    virtual ~CCnChess();

    // IGameTable接口实现
    void Boardcast(DWORD dwType, LPVOID pData, DWORD dwSize) override;
    BOOL SendMsg(PWSCLIENT PWSCLIENT, DWORD dwType, LPVOID pData, DWORD dwSize) override;
    BOOL OnMsg(PWSCLIENT PWSCLIENT, DWORD dwType, LPVOID pData, DWORD dwSize) override;
    void OnGameStart() override;
    void OnGameEnd() override;
    void OnRoundEnd() override{}
    BOOL OnPlayerLeave(int seatId, PWSCLIENT pClient) override;
    // 将指定座位标记为机器人并记录智力等级(1=初级,2=中级,3=高级)
    void SetupRobot(int seatId, int nLevel) override;
    // 配置残局桌: 进入残局时设置自定义布局(布局始终以红方为下方向, 与正式棋盘一致)
    void ConfigureEndgame(int nEndgameId, const int layout[10][9]);
    // 应用机器人走子(桌子A.I.搜索结果经线程池异步回到主线程后调用; generation 用于丢弃过期结果)
    void ApplyRobotMove(const MOVESTEP &best, int seatId, int generation);
protected:
    // 象棋游戏相关方法
    void BrdcstAckOver(GAMEOVERTYPE nType, int nLossSeat, LPCWSTR pszDesc);

	BOOL OnPlayerSurrender(PWSCLIENT pClient,DWORD dwType,LPVOID pData , DWORD dwSize);
    BOOL OnChessMove(PWSCLIENT pClient, DWORD dwType, LPVOID pData, DWORD dwSize);
    BOOL OnRegret(PWSCLIENT pClient,DWORD dwType,LPVOID pData , DWORD dwSize);
	BOOL OnResult(PWSCLIENT pClient,DWORD dwType,LPVOID pData , DWORD dwSize);
	BOOL OnReqPeace(PWSCLIENT pClient,DWORD dwType,LPVOID pData , DWORD dwSize);
	BOOL OnAckPeace(PWSCLIENT pClient,DWORD dwType,LPVOID pData , DWORD dwSize);
	BOOL OnReqLongCatch(PWSCLIENT pClient,DWORD dwType,LPVOID pData , DWORD dwSize);
	BOOL OnPlayerTimeout(PWSCLIENT pClient,DWORD dwType,LPVOID pData , DWORD dwSize);

	CHSSIDE GetPlayerColor(int nSeat);
	int GetRedSeat() const {
        return m_nRedIndex;
    }
	UINT GetFarthestRepeat(CHESSMAN & chsEnemy);
    int GetActiveSeat() const;

	// 机器人相关辅助
	bool HasRobot() const;              // 桌上是否有机器人
	void RobotMakeMove(int seatId);     // 让机器人思考并走一步
	void CheckServerOver();             // 服务端将死/困毙检测(仅机器人对局)
	void TriggerIfRobotTurn();          // 当前行棋方为机器人时触发走棋
protected:
	DWORD	m_dwProps[PROP_SIZE];//游戏室属性

	REGRETREC	m_regretRec[PLAYER_COUNT];
	int		m_nRegretSteps[PLAYER_COUNT];		//悔棋步数
	int		m_nPeaceSeat;		//当前求和方的座位号
    int     m_nReqPeaceSteps;   //求和请求步数
	UINT	m_nPeaceSteps;		//未吃子步数		
	int		m_nChessMsg;						//当前走棋消息计数，与layout中保留的当前步不一样，该变量在一局游戏中不断递增，以识别定时器消息
	CChessLayout	m_layout;				//红方的棋子状态
	GAMEOVERTYPE m_goType;			//游戏结束类型
	time_t	m_timeStart;				//一局游戏开始时间

	std::vector<MOVESTEP> m_lstMoves;			//走棋历史
	int		m_nChsPassable;				//总可过河子数量，一般为22子，打残局时不一样。
	int		m_nLeftPassable;			//当前可河子数量。
	int		m_nRedIndex;					//红方的座位号

	time_t m_alTime[ PLAYER_COUNT ];
	time_t m_dwStartTime; 

	bool	m_bRobot[PLAYER_COUNT];		// 座位是否为机器人
	int		m_nRobotLevel[PLAYER_COUNT];	// 机器人智力等级(ROBOT_LEVEL_*)

	// 残局桌相关
	bool	m_bEndgame;					// 本桌是否为残局桌
	int		m_nEndgameId;				// 残局ID
	int		m_nEndgameLayout[10][9];	// 残局布局(正红方向)

};

SNSEND

#endif // !__CNCHESSTABLE_H__