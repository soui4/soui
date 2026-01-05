// JunQiLayout.h: interface for the CJunQiLayout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JUNQILAYOUT_H__E55D8295_49C1_469E_AC9A_973DDE24A976__INCLUDED_)
#define AFX_JUNQILAYOUT_H__E55D8295_49C1_469E_AC9A_973DDE24A976__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "JunqiMap.h"
#include <souicoll.h>
#include <vector>

#define OR_NULL			0		//没有棋


#define OR_SAPPER		1		//工兵
#define OR_PLATOON		2		//排长
#define OR_COMPANY		3		//连长
#define OR_BATTALION	4		//营长
#define OR_COLONEL		5		//团长
#define OR_BRIGADIER	6		//旅长
#define OR_DIVISION		7		//师长
#define OR_ARMY			8		//军长

#define	OR_CHIEF		9		//司令
#define OR_FLAG			10		//军旗
#define OR_BOMB			11		//炸弹
#define OR_MINE			12		//地雷

#define OR_FAKE			0xD		//假棋

//棋子信息
typedef struct tagCHESSMAN
{
	int		nOffRank;
	int     nID;//range from 0 to 24
	POINT	ptCell;
	BOOL	bShow;	//棋子显示开关，单机版研究用
}CHESSMAN,*PCHESSMAN;

//*************************************
//	棋格中的棋子
//	没有棋子时pos=0
//***************************************
typedef struct tagCELLCHESS
{
	SOUI::SPOSITION pos;
	int nSide;	
}CELLCHESS,*PCELLCHESS;

typedef enum LayoutError
{
    LE_OK = 0,
    LE_INVALID_CELL,
    LE_INVALID_FLAG,
    LE_INVALID_MINE,
    LE_INVALID_BOMB,
} LayoutError;

//四个方位
#define SIDE_BOTTOM	0
#define SIDE_LEFT	1
#define SIDE_TOP	2
#define SIDE_RIGHT	3

/*---------------------------------------------------------------------------
 老版本军旗消息定义
---------------------------------------------------------------------------*/
#define PUT_EAT		1	// 吃了目标
#define PUT_BEEAT	2	// 被目标吃
#define PUT_BOMB	3	// 同归于尽
#define PUT_MOVE    4   // 移动棋子

#define RST_NULL	0	//null
#define RST_PEACE	4	// 和棋了		
#define RST_DIE		5	// 消灭了目标军旗	

//---------------------------------------------------------
//	走棋数据结构
//----------------------------------------------------------
typedef struct tagMOVECMINFO
{
	POINT		ptCell1;		//棋子起始位置
	POINT		ptCell2;		//棋子终止位置
	int			nStatus;		//棋子比较结果
	int			nResult;		// 结果状态,RST_NULL,RST_PEACE,RST_DIE
	int			nPathSize;		//走棋线路的长度
	POINT		ptPaths[73];	//走棋线路,最大不可能达到73
}MOVECMINFO;

typedef struct tagADJACENT{
	int nID;
	DWORD dwDis;
}ADJACENT;

class CJunQiLayout  
{
public:
	enum{
		MAT_WIDTH = 17
	};
	
	CJunQiLayout();
    virtual ~CJunQiLayout();	

	/**
	 * 设置游戏属性
	 * @param dwPlayers 游戏人数,只能为2,4
	 * @param dwBombFly 炸弹满天飞
	 * @param dwMineAsBomb 地雷当炸弹标志
	 */
	void SetProperty(DWORD dwPlayers,DWORD dwBombFly,DWORD dwMineAsBomb){ m_dwPlayers=dwPlayers,m_dwBombFly=dwBombFly,m_dwMineAsBomb=dwMineAsBomb;}

public:
    BOOL IsChessShow(POINT ptCell) const;
	int ConvertASide(POINT ptCell);
	BOOL ConvertChess(POINT ptCell);

	/**
	 * 获取某方军旗位置
	 * @param nSide 玩家编号
	 * @return 军旗位置
	 */
	POINT GetFlagPosition(int nSide);

	/**
	 * 初始化布局
	 * @param nMatStone 布局信息
	 * @param nSide 玩家编号, -1表示所有玩家
	 */
	void InitLayout(const int nMatStone[][MAT_WIDTH],int nSide=-1);

	/**
	 * 构造布局
	 * @param nSide 玩家编号
	 * @param nMatStone 布局信息
	 * @return TRUE:成功 FALSE:失败
	 */
	BOOL MakeLayout(int nSide,int nMatStone[][17]) const;

	/**
	 * 为指定方初始化布局
	 * @param nSide 玩家编号
	 * @param byLayout[13] 布局信息
	 */
	BOOL InitLayout(int nSide,const BYTE byLayout[13]);

	/**
	 * 为指定方初始化布局
	 * @param nSide 玩家编号
	 * @param nMat 布局信息
	 */
	BOOL InitLayout(int nSide,const int nMat[6][5]);

	/**
	 * 为指定方构造一个布阵图
	 * @param nSide 玩家编号
	 * @param byLayout[13] 布阵图信息
	 * @return TRUE:成功 FALSE:失败
	 */
    BOOL MakeLayout(int nSide, BYTE byLayout[13]) const;

	/**
	 * 清空布局
	 * @param nSide 玩家编号
	 * @return TRUE:成功 FALSE:失败
	 */
	BOOL ClearLayout(int nSide);

	/**
	 * 判断是否是工兵
	 * @param ptCell 棋子位置
	 * @return TRUE:是工兵 FALSE:不是工兵
	 */
	BOOL IsSapper(POINT ptCell) const;

	/**
	 * 判断是否可以移动
	 * @param ptFrom 起始位置
	 * @param ptTo 目标位置
	 * @param nSide 玩家编号
	 * @return TRUE:可以移动 FALSE:不可以移动
	 */
	BOOL IsValidMove(POINT ptFrom,POINT ptTo,int nSide) const;

	/**
	 * 获取某方某棋子信息
	 * @param ptCell 棋子位置
	 * @return 棋子信息
	 */
	CHESSMAN GetCellChessMan(POINT ptCell) const;

	/**
	 * 获取棋子信息
	 * @param ptCell 棋子位置
	 * @return 棋子信息
	 */
    CELLCHESS GetCellChess(POINT ptCell) const;

	/**
	 * 判断一个棋子是否可以自由移动
	 * @param ptCell 棋子位置
	 * @return TRUE:可以自由移动 FALSE:不可以自由移动
	 */
	BOOL IsChessmanFree(POINT ptCell) const;

	/**
	 * 获取某方podium的方位
	 * @param ptPodium podium位置
	 * @return 方位
	 */
	int GetPodiumSide(POINT ptPodium) const;

	/**
	 * 判断某方是否还有空闲的棋子
	 * @param nSide 玩家编号
	 * @return TRUE:有空闲棋子 FALSE:无空闲棋子
	 */
	BOOL HasFreeChessman(int nSide) const;

	/**
	 * 删除布局
	 */
	void DeleteLayout();

	/**
	 * 玩家战败，将该玩家的棋子从棋盘上拿掉
	 * @param nSide 玩家编号
 	*/
	void PlayerLose(int nSide);


	/**
	 * 判断某方是否已输
	 * @param nSide 玩家编号
	 * @return TRUE:已输 FALSE:未输
	 */
	BOOL IsPlayerLost(int nSide) const;

	/**
	 * 翻开棋子
	 * @param ptCell 棋子位置
	 * @param nOffRank 棋子rank
	 */
	int ViewChessman(POINT ptCell,int nOffRank);

	/**
	 * 获取棋子比较结果
	 * @param ptCell1 起始位置
	 * @param ptCell2 目标位置
	 * @return 棋子比较结果
	 */
	int GetCmpResult(POINT ptCell1,POINT ptCell2) const;

	/**
	 * 移动棋子for 服务器
	 * @param ptCell1 起始位置
	 * @param ptCell2 目标位置
	 * @return 移动结果
	 */
	int MoveChessmanServer(POINT ptCell1,POINT ptCell2);

	/**
	 * 移动棋子for 客户端
	 * @param ptCell1 起始位置
	 * @param ptCell2 目标位置
	 * @param nResult 移动结果
	 * @note 根据移动结果修改棋盘布局
	 */
	void MoveChessmanClient(POINT ptCell1,POINT ptCell2,int nResult);

	/**
	 * 获取棋子rank
	 * @param ptCell 棋子位置
	 * @return 棋子rank
	 */
	int GetCellChessOffRank(POINT ptCell) const;

	/**
	 * 在指定位置放置一颗棋子
	 * @param ptCell 棋子位置
	 * @param nOffRank 棋子rank
	 * @param nSide 棋子所属方位
	 * @param bShow 是否显示
	 */
	void SetCellChessman(POINT ptCell, int nOffRank, int nSide,BOOL bShow=TRUE);

	/**
	 * 交换两个棋子的位置
	 * @param ptCell1 棋子1位置
	 * @param ptCell2 棋子2位置
	 * @return TRUE:成功 FALSE:失败
	 */
	BOOL ExchangeChessman(POINT ptCell1,POINT ptCell2);


	/**
	 * 拿掉一颗棋子
	 * @param ptCell 棋子位置
	 * @return 删除的棋子rank
	 */
	int RemoveChessman(POINT ptCell);

	/**
	 * 获取某方的棋子列表
	 * @param nSide 玩家编号
	 * @return 棋子列表
 	*/
	const SOUI::SList<CHESSMAN> * GetTroopList(int nSide) const;

	/**
	 * 判断目标格子是否对某方有效
	 * @param nSide 玩家编号
	 * @param ptCell 目标格子
	 * @return TRUE:有效 FALSE:无效
 	*/
	BOOL IsValidCell(int nSide,POINT ptCell) const;

	/**
	 * 获取棋子可移动范围
	 * @param ptCell 棋子位置
	 * @param pLstCell[out] 可移动范围
	 */
	int GetChessmanTravelSpace(POINT ptCell, SOUI::SList<POINT> *pLstCell) const;
	int GetChessmanTravelSpace(POINT ptCell, std::vector<POINT> *pLstCell) const;

	/**
	 * 获取棋子移动路径
	 * @param bSapper 是否为工兵
	 * @param ptCell1 起始位置
	 * @param ptCell2 目标位置
	 * @param pLstPath[out] 路径
	 */
	void GetMoveCMPath(BOOL bSapper,POINT ptCell1, POINT ptCell2, SOUI::SList<POINT> *pLstPath) const;
	void GetMoveCMPath(BOOL bSapper,POINT ptCell1, POINT ptCell2, std::vector<POINT> *pLstPath) const;

	/**
	 * 获取地图对象
	 * @return 地图对象
	 */
	CJunQiMap* GetJunQiMap(){return &m_junQiMap;}

public:
	/**
	 * 判断目标格子对于指定的棋子是否有效
	 * @param ptCell 目标格子
	 * @param nRank 棋子rank
	 * @return 0:有效 1:无效格子 2:无效军旗 3:无效地雷 4:无效炸弹 
	 * @note 只以布局阶段使用
 	*/
	virtual LayoutError IsValidCellForChessman(POINT ptCell, int nRank) const;

	/**
	 * 比较两个棋子的大小
	 * @param nOffRank1 棋子1rank
	 * @param nOffRank2 棋子2rank
	 * @return 比较结果
	 */
	virtual int ChessmanCompare(int nOffRank1,int nOffRank2) const;
private:
	CJunQiMap m_junQiMap;
	SOUI::SList<CHESSMAN> m_lstTroops[4];
	mutable CELLCHESS	m_cellChess[17][17];
private:
	void ChangeChessmanPos(POINT ptChess,POINT ptCell);

	SOUI::SList<CHESSMAN> * GetTroopList2(int nSide);

	BOOL FindShortestPath(
					  ADJACENT *adjacent[AC_RWSSIZE], 
					  int		nAdjCount[AC_RWSSIZE],
					  int nBeginID, 
					  int nEndID,
					  SOUI::SList<POINT> *pLstPath
					  ) const;
	BOOL FindShortestPath(DWORD adjMatrix[][AC_RWSSIZE], int nBegin, int nEnd,SOUI::SList<POINT> *pLstPath) const;
	void FindSapperPath(POINT ptBegin,POINT ptEnd,SOUI::SList<POINT> *pLstPath) const;
	int GetSapperTravelSpace(POINT ptCell,SOUI::SList<POINT> *pLstCell) const;
	void GetSapperTravelSpaceRecursion(int nSide,POINT ptCell,SOUI::SList<POINT> *pLstCell,BYTE *pbyFlagMetrix) const;
	BOOL AddCellDistinct(SOUI::SList<POINT> *pLstCell,POINT ptCell) const;

	DWORD			m_dwBombFly;	//炸弹满天飞
	DWORD			m_dwMineAsBomb;	//地雷当炸弹标志
	DWORD			m_dwPlayers;	//游戏人数,只能为2,4
protected:
	BOOL IsValidMoveDest(int nSide,POINT ptDestCell) const;
};

#endif // !defined(AFX_JUNQILAYOUT_H__E55D8295_49C1_469E_AC9A_973DDE24A976__INCLUDED_)
