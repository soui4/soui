#pragma once
#include "Chessman.h"
#include <vector>


class CChessLayout;

class CChsLytState
{
	friend class CChessLayout;
public:
	class CChessInfo
	{
	public:
		CChessInfo()
		{}
		CChessInfo(CChessInfo *pChsInfo)
			:m_lstDest(pChsInfo->m_lstDest),m_lstProtecter(pChsInfo->m_lstProtecter),m_lstAttacker(pChsInfo->m_lstAttacker)
		{}
		std::vector<POINT> m_lstDest;		//目标位置
		std::vector<POINT> m_lstProtecter;	//保护者
		std::vector<POINT> m_lstAttacker;	//攻击者
	};
	CChsLytState(CChessLayout *pLayout,BOOL bUpdate=FALSE);
	~CChsLytState();
	BOOL IsJiangJun(CHSSIDE side);
	//获得当前走棋方将军的位置
	POINT GetJiangJunPosition(CHSSIDE side){return side==CS_RED?m_ptJiangRed:m_ptJiangBlack;}
	//计算出当前局面的赢家
	CHSSIDE GetWinner();
	//获得目标棋子的被保护计数，如果目标棋子为将则返回0
	int GetProtecter(POINT pt);
	//获得目标棋子的被攻击计数
	int GetAttacter(POINT pt);
	//获得棋子状态
	CChessInfo *GetChessInfo(POINT pt){return m_ChessInfo[pt.y][pt.x];}
	//更新状态
	void UpdateState();
	//创建一个状态备分
	void Copy(CChsLytState *pState);
protected:
	void ClearState();
	//游戏状态数据
	CChessInfo * m_ChessInfo[10][9];
	POINT		 m_ptJiangRed,m_ptJiangBlack;	//双方将军位置
	CHSSIDE		 m_csWinner;
	CChessLayout *m_pLayout;
};

typedef struct tagMOVESTEP
{
	POINT pt1; //走子位置
	POINT pt2; //目标位置
	CHESSMAN enemy;	//目标棋子
	int		 nEnemyID;//目标棋子的ID
}MOVESTEP,*PMOVESTEP;

//!棋局布局
class CChessLayout
{
  public:
	CChessLayout();
	virtual ~CChessLayout(void);
    static void GetDefaultLayout(int chsLayout[10][9]);

	//初始化棋局
	//CHESSMAN chsLayout[10][9]:布局
	//CHSSIDE selfSide:自己的颜色
	void InitLayout(const int chsLayout[10][9],CHSSIDE selfSide);
	void Copy(CChessLayout *pLayout);
	//获得指定棋子的移动范围
	int  GetPossiableMoves(POINT ptFocus,POINT ptMoves[MAX_MOVES]);
	int  GetPossiableMoves(int x,int y,POINT ptMoves[MAX_MOVES]);
	// 移动一步
	MOVESTEP Move(POINT pt1, POINT pt2);
	//回退一步走棋
	BOOL UndoMove(const MOVESTEP & moveStep);
	//获得当前步被移动的棋子
	CHESSMAN GetMoveChess(const MOVESTEP & moveStep){return m_chesses[moveStep.pt2.y][moveStep.pt2.x];}
	//获得走棋描述
	//MOVESTEP mstep当前走棋步
	//char szDesc[100]:走棋描述
	void GetMoveDescription(MOVESTEP mstep,TCHAR szDesc[100]);
	//判断自己的是不是红色
	BOOL IsRedTurn() const {return m_actSide==CS_RED;}
	BOOL IsRedSide() const {return m_selfSide==CS_RED;}
	//检测走棋的有效性
	BOOL IsValidMove(POINT pt1 , POINT pt2,CChsLytState *pLytState=NULL);
	//交换颜色
	void ExchangeSide(void);
	void SetActiveSide(CHSSIDE side){m_actSide=side;}

	int GetAllChess(POINT ptChs[16],CHSSIDE side);
	
	//!当前行棋方
	CHSSIDE m_actSide;
	//!己方(下方）颜色
	CHSSIDE m_selfSide;

	//!游戏状态数据
	//!棋子当前布局
	CHESSMAN	m_chesses[10][9];
	//记录每一个棋子的ID，以便在长捉中区别相同棋的不同棋子
	int			m_nChsID[10][9];
	//!死棋数据
	CHESSMAN	m_csDeadRed[16],m_csDeadBlack[16];
	int			m_nDeadRed,m_nDeadBlack;
protected:
	int SameLineSameChess(CHESSMAN chsLayout[10][9],POINT pt1,POINT pt2,CHESSMAN chsMove);
};
