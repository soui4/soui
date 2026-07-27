#include "stdafx.h"
#include "ChessLayout.h"
#include <sstream>
#include <sdef.h>

#ifdef _UNICODE
#define tstringstream wstringstream
#else
#define tstringstream stringstream
#endif
CChsLytState::CChsLytState(CChessLayout *pLayout,BOOL bUpdate)
{
	m_pLayout=pLayout;
	memset(m_ChessInfo,0,10*9*sizeof(CChessInfo*));
	m_csWinner=CS_NULL;
	if(bUpdate) UpdateState();
}

CChsLytState::~CChsLytState(){
	ClearState();
}

void CChsLytState::ClearState()
{
	m_csWinner=CS_NULL;
	m_ptJiangRed.x=m_ptJiangRed.y=-1;
	m_ptJiangBlack.x=m_ptJiangBlack.y=-1;
	for(int y=0;y<10;y++) for(int x=0;x<9;x++)
	{
		if(!m_ChessInfo[y][x]) continue;
		delete m_ChessInfo[y][x];
		m_ChessInfo[y][x]=NULL;
	}
}

void CChsLytState::Copy(CChsLytState *pState)
{
	m_pLayout=pState->m_pLayout;
	m_ptJiangBlack=pState->m_ptJiangBlack;
	m_ptJiangRed=pState->m_ptJiangRed;
	m_csWinner=pState->m_csWinner;
	for(int y=0;y<10;y++) for(int x=0;x<9;x++)
	{
		if(m_ChessInfo[y][x])
		{
			delete m_ChessInfo[y][x];
			m_ChessInfo[y][x]=NULL;
		}
		if(pState->m_ChessInfo[y][x])	m_ChessInfo[y][x]=new CChessInfo(pState->m_ChessInfo[y][x]);
	}
}

void CChsLytState::UpdateState()
{
	POINT ptDest[MAX_MOVES];
	int   nDest;
	LONG x,y;
	ClearState();
	for(y=0;y<10;y++) for(x=0;x<9;x++)
	{
		if(m_pLayout->m_chesses[y][x]==CHSMAN_NULL) continue;
		//保存将军位置
		if(m_pLayout->m_chesses[y][x]==CHSMAN_RED_JIANG)
			m_ptJiangRed.y=y,m_ptJiangRed.x=x;
		else if(m_pLayout->m_chesses[y][x]==CHSMAN_BLK_JIANG)
			m_ptJiangBlack.y=y,m_ptJiangBlack.x=x;
		//获得每一个棋子的可到达位置
		m_ChessInfo[y][x]=new CChessInfo;
		nDest=m_pLayout->GetPossiableMoves(x,y,ptDest);
		for(int i=0;i<nDest;i++) m_ChessInfo[y][x]->m_lstDest.push_back(ptDest[i]);
	}
	//计算出棋子的保护及攻击位置
	for(y=0;y<10;y++) for(x=0;x<9;x++)
	{
		if(m_pLayout->m_chesses[y][x]==CHSMAN_NULL) continue;
		POINT pt={x,y};
		std::vector<POINT>::iterator p=m_ChessInfo[y][x]->m_lstDest.begin();
		while(p!=m_ChessInfo[y][x]->m_lstDest.end())
		{
			if(m_pLayout->m_chesses[p->y][p->x]!=CHSMAN_NULL) 
			{
				if(CHSMANSIDE(m_pLayout->m_chesses[y][x])==CHSMANSIDE(m_pLayout->m_chesses[p->y][p->x]))
				{
					BOOL bValidProtecter=TRUE;
					if(m_pLayout->m_chesses[y][x]==CHSMAN_RED_PAO || m_pLayout->m_chesses[y][x]==CHSMAN_BLK_PAO)
					{//炮的保护需要特殊处理，如果架炮是对方的棋子并且被保护棋子在该棋子的攻击范围内则保护失效
						//首先找到架炮的棋子坐标
						POINT ptLedge={-1,-1};
						int n1,n2;
						if(p->x==x)
						{
							n1=smin(p->y,y);
							n2=smax(p->y,y);
							for(int i=n1+1;i<=n2-1;i++)
								if(m_pLayout->m_chesses[i][x]!=CHSMAN_NULL)
								{
									ptLedge.x=x,ptLedge.y=i;
									break;
								}
						}else //if(p->y==y)
						{
							n1=smin(p->x,x);
							n2=smax(p->x,x);
							for(int i=n1+1;i<=n2-1;i++)
								if(m_pLayout->m_chesses[y][i]!=CHSMAN_NULL)
								{
									ptLedge.y=y,ptLedge.x=i;
									break;
								}
						}
						assert(ptLedge.x!=-1);
						if(CHSMANSIDE(m_pLayout->m_chesses[ptLedge.y][ptLedge.x])!=CHSMANSIDE(m_pLayout->m_chesses[p->y][p->x]))
						{//发现架炮的棋子是对方的棋子
							std::vector<POINT>::iterator p2=m_ChessInfo[ptLedge.y][ptLedge.x]->m_lstDest.begin();
							while(p2!=m_ChessInfo[ptLedge.y][ptLedge.x]->m_lstDest.end())
							{
								if(p2->x==p->x && p2->y==p->y)
								{
									bValidProtecter=FALSE;
									break;
								}
								p2++;
							}
						}
					}
					if(bValidProtecter)	m_ChessInfo[p->y][p->x]->m_lstProtecter.push_back(pt);
				}else
					m_ChessInfo[p->y][p->x]->m_lstAttacker.push_back(pt);
			}
			p++;
		}
	}
	if(m_ptJiangRed.x==-1)
		m_csWinner=CS_BLACK;
	else if(m_ptJiangBlack.x==-1)
		m_csWinner=CS_RED;
	else
		m_csWinner=CS_NULL;
}

BOOL CChsLytState::IsJiangJun(CHSSIDE side)
{
	if(side==CS_RED)
		return m_ChessInfo[m_ptJiangRed.y][m_ptJiangRed.x]->m_lstAttacker.size()>0;
	else
		return m_ChessInfo[m_ptJiangBlack.y][m_ptJiangBlack.x]->m_lstAttacker.size()>0;
}

int CChsLytState::GetProtecter(POINT pt)
{
	if(m_pLayout->m_chesses[pt.y][pt.x]==CHSMAN_BLK_JIANG || m_pLayout->m_chesses[pt.y][pt.x]==CHSMAN_RED_JIANG)
		return 0;
	else 
	{
		assert(m_ChessInfo[pt.y][pt.x]);
		return (int)m_ChessInfo[pt.y][pt.x]->m_lstProtecter.size();
	}
}

int CChsLytState::GetAttacter(POINT pt)
{
	assert(m_ChessInfo[pt.y][pt.x]);
	return (int)m_ChessInfo[pt.y][pt.x]->m_lstAttacker.size();
}

CHSSIDE CChsLytState::GetWinner()
{
	if(m_csWinner==CS_NULL)
	{
		BOOL bCanMove=FALSE;
		CHSSIDE csActive=m_pLayout->m_actSide;
		POINT ptFocus;
		for(int y=0;y<10 && !bCanMove;y++) for(int x=0;x<9 && !bCanMove;x++)
		{
			if(CHSMANSIDE(m_pLayout->m_chesses[y][x])==m_pLayout->m_actSide)
			{
				ptFocus.x=x,ptFocus.y=y;
				std::vector<POINT>::iterator p=m_ChessInfo[y][x]->m_lstDest.begin();
				while(p!=m_ChessInfo[y][x]->m_lstDest.end())
				{
					if(CHSMANSIDE(m_pLayout->m_chesses[p->y][p->x])!=csActive)
					{
						MOVESTEP ms=m_pLayout->Move(ptFocus,*p);
						CChsLytState state(m_pLayout);
						state.UpdateState();
						m_pLayout->UndoMove(ms);
						if(!state.IsJiangJun(csActive)) 
						{
							bCanMove=TRUE;
							break;
						}
					}
					p++;
				}
			}
		}
		if(bCanMove) m_csWinner=CS_NEUTRAL;
		else m_csWinner=(CHSSIDE)!m_pLayout->m_actSide;
	}
	return m_csWinner;
}


CChessLayout::CChessLayout(void)
{
}

CChessLayout::~CChessLayout(void)
{
}

static const int kDefaultLayout[10][9] = {
    // 标准棋谱
    { CHSMAN_RED_JU, CHSMAN_RED_MA, CHSMAN_RED_XIANG, CHSMAN_RED_SHI, CHSMAN_RED_JIANG, CHSMAN_RED_SHI, CHSMAN_RED_XIANG, CHSMAN_RED_MA, CHSMAN_RED_JU }, // line1
    { CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL },                              // line2
    { CHSMAN_NULL, CHSMAN_RED_PAO, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_RED_PAO, CHSMAN_NULL },                        // line3
    { CHSMAN_RED_BING, CHSMAN_NULL, CHSMAN_RED_BING, CHSMAN_NULL, CHSMAN_RED_BING, CHSMAN_NULL, CHSMAN_RED_BING, CHSMAN_NULL, CHSMAN_RED_BING },          // line4
    { CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL },                              // line5
    { CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL },                              // line6
    { CHSMAN_BLK_BING, CHSMAN_NULL, CHSMAN_BLK_BING, CHSMAN_NULL, CHSMAN_BLK_BING, CHSMAN_NULL, CHSMAN_BLK_BING, CHSMAN_NULL, CHSMAN_BLK_BING },          // line7
    { CHSMAN_NULL, CHSMAN_BLK_PAO, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_BLK_PAO, CHSMAN_NULL },                        // line8
    { CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL, CHSMAN_NULL },                              // line9
    { CHSMAN_BLK_JU, CHSMAN_BLK_MA, CHSMAN_BLK_XIANG, CHSMAN_BLK_SHI, CHSMAN_BLK_JIANG, CHSMAN_BLK_SHI, CHSMAN_BLK_XIANG, CHSMAN_BLK_MA, CHSMAN_BLK_JU }, // line10
};

void CChessLayout::GetDefaultLayout(int chsLayout[10][9])
{
	memcpy(chsLayout, kDefaultLayout, 10*9*sizeof(int));
}
//初始化棋局
void CChessLayout::InitLayout(const int chsLayout[10][9],CHSSIDE selfSide)
{

	if (chsLayout == NULL)
        chsLayout = kDefaultLayout;
	memcpy(m_chesses,chsLayout,10*9*sizeof(CHESSMAN));
	if(selfSide==CS_BLACK)
	{
		CHSSIDE csBottom=CS_NEUTRAL;
		for(int y=0;y<=2 && csBottom==CS_NEUTRAL;y++)for(int x=3;x<=5 && csBottom==CS_NEUTRAL;x++)
		{
			if(m_chesses[y][x]==CHSMAN_BLK_JIANG)
				csBottom=CS_BLACK;
			else if(m_chesses[y][x]==CHSMAN_RED_JIANG)
				csBottom=CS_RED;
		}
		if(csBottom!=selfSide) CChsMoveGenerator::ConvertLayout(m_chesses);
	}
	//计算出棋子的ID
	int nID=0;
	for(int y=0;y<10;y++)for(int x=0;x<9;x++)
	{
		m_nChsID[y][x]=(m_chesses[y][x]!=CHSMAN_NULL)?(++nID):0;
	}
	m_selfSide=selfSide;
	m_actSide=CS_RED;
	m_nDeadRed=m_nDeadBlack=0;
	memset(m_csDeadRed,0,sizeof(m_csDeadRed));
	memset(m_csDeadBlack,0,sizeof(m_csDeadBlack));
}

void CChessLayout::Copy(CChessLayout *pLayout)
{
	m_selfSide=pLayout->m_selfSide;
	m_actSide=pLayout->m_actSide;
	
	memcpy(m_chesses,pLayout->m_chesses,10*9*sizeof(CHESSMAN));
	memcpy(m_nChsID,pLayout->m_nChsID,10*9*sizeof(int));
	m_nDeadRed=pLayout->m_nDeadRed;
	m_nDeadBlack=pLayout->m_nDeadBlack;
	memcpy(m_csDeadRed,pLayout->m_csDeadRed,m_nDeadRed*sizeof(CHESSMAN));
	memcpy(m_csDeadBlack,pLayout->m_csDeadBlack,m_nDeadBlack*sizeof(CHESSMAN));
}

// 移动一步
MOVESTEP CChessLayout::Move(POINT pt1, POINT pt2)
{
	MOVESTEP moveStep={pt1,pt2,m_chesses[pt2.y][pt2.x],m_nChsID[pt2.y][pt2.x]};
	assert(CHSMANSIDE(m_chesses[pt1.y][pt1.x])==m_actSide);
	switch(CHSMANSIDE(moveStep.enemy))
	{
	case CS_RED:m_csDeadRed[m_nDeadRed++]=moveStep.enemy;break;
	case CS_BLACK:m_csDeadBlack[m_nDeadBlack++]=moveStep.enemy;break;
	}
	m_chesses[pt2.y][pt2.x]=m_chesses[pt1.y][pt1.x];
	m_chesses[pt1.y][pt1.x]=CHSMAN_NULL;
	m_nChsID[pt2.y][pt2.x]=m_nChsID[pt1.y][pt1.x];
	m_nChsID[pt1.y][pt1.x]=0;
	m_actSide=(CHSSIDE)!m_actSide;
	return moveStep;
}

BOOL CChessLayout::UndoMove(const MOVESTEP & moveStep)
{
	if(m_chesses[moveStep.pt2.y][moveStep.pt2.x]==CHSMAN_NULL) return FALSE;
	m_chesses[moveStep.pt1.y][moveStep.pt1.x]=m_chesses[moveStep.pt2.y][moveStep.pt2.x];
	m_chesses[moveStep.pt2.y][moveStep.pt2.x]=moveStep.enemy;
	m_nChsID[moveStep.pt1.y][moveStep.pt1.x]=m_nChsID[moveStep.pt2.y][moveStep.pt2.x];
	m_nChsID[moveStep.pt2.y][moveStep.pt2.x]=moveStep.nEnemyID;
	m_actSide=(CHSSIDE)!m_actSide;
	switch(CHSMANSIDE(moveStep.enemy))
	{
	case CS_RED:m_nDeadRed--;break;
	case CS_BLACK:m_nDeadBlack--;break;
	}
	return TRUE;
}

int  CChessLayout::GetPossiableMoves(POINT ptFocus,POINT ptMoves[MAX_MOVES])
{
	return CChsMoveGenerator::GetPossiableMoves(m_chesses,m_selfSide==CS_RED,ptFocus,ptMoves);
}

int  CChessLayout::GetPossiableMoves(int x,int y,POINT ptMoves[MAX_MOVES])
{
	POINT pt={x,y};
	return CChsMoveGenerator::GetPossiableMoves(m_chesses,m_selfSide==CS_RED,pt,ptMoves);
}

//在走棋描述中需要使用到的字符串常量
static const LPCTSTR kszPosCn[]={_T(""),_T("一"),_T("二"),_T("三"),_T("四"),_T("五"),_T("六"),_T("七"),_T("八"),_T("九")};
static const LPCTSTR kszPosArabic[]={_T("0"),_T("1"),_T("2"),_T("3"),_T("4"),_T("5"),_T("6"),_T("7"),_T("8"),_T("9")};
static const LPCTSTR kszRedChess[]={_T("帥"),_T("車"),_T("馬"),_T("炮"),_T("仕"),_T("相"),_T("兵")}; 
static const LPCTSTR kszBlackChess[]={_T("将"),_T("車"),_T("馬"),_T("炮"),_T("士"),_T("象"),_T("卒")}; 

//找到与当前走子在同一列的相同棋子的位置
int CChessLayout::SameLineSameChess(CHESSMAN chsLayout[10][9],POINT pt1,POINT pt2,CHESSMAN chsMove)
{
	for(int i=0;i<10;i++)
	{
		if(i==pt1.y || (pt2.x==pt1.x && i==pt2.y)) continue;
		if(chsLayout[i][pt1.x]==chsMove) return i;
	}
	return -1;
}

//获得走棋描述
void CChessLayout::GetMoveDescription(MOVESTEP mstep,TCHAR szDesc[100])
{
	static const LPCTSTR kszMove[]={
		_T("进"),
		_T("平"),
		_T("退"),
		_T("左"),
		_T("右"),
		_T("前"),
		_T("后"),
	};
	enum MoveType
	{
		MT_IN=0,
		MT_EQUAL,
		MT_OUT,
		MT_LEFT,
		MT_RIGHT,
		MT_FRONT,
		MT_BACK,
	};

	const LPCTSTR *pszPos=kszPosCn;
	const LPCTSTR *pszChess=kszRedChess;
	POINT pt1=mstep.pt1,pt2=mstep.pt2;
	CHESSMAN chsMove=m_chesses[pt2.y][pt2.x];
	CHESSMAN tmpLayout[10][9];
	memcpy(tmpLayout,m_chesses,10*9*sizeof(CHESSMAN));
	
	if(m_selfSide==CS_BLACK)
	{//将棋盘调整一下方向,保证红在下黑在上
		CChsMoveGenerator::ConvertLayout(tmpLayout);
		pt1.x=8-pt1.x;
		pt1.y=9-pt1.y;
		pt2.x=8-pt2.x;
		pt2.y=9-pt2.y;
	}
	if(CHSMANSIDE(chsMove)==CS_BLACK)
	{//黑棋走子,先调整方向
		CChsMoveGenerator::ConvertLayout(tmpLayout);
		CChsMoveGenerator::ExchangeColor(tmpLayout);
		pt1.x=8-pt1.x;
		pt1.y=9-pt1.y;
		pt2.x=8-pt2.x;
		pt2.y=9-pt2.y;
		chsMove=(CHESSMAN)(chsMove-7);//变成红棋
		pszPos=kszPosArabic;
		pszChess=kszBlackChess;
	}
	int iSameChs=SameLineSameChess(tmpLayout,pt1,pt2,chsMove);
	
	std::tstringstream ss;
	switch(chsMove)
	{
	case CHSMAN_RED_JIANG://红帅
		if(mstep.pt1.x==mstep.pt2.x)
		{
			if(mstep.pt1.y<mstep.pt2.y)
				ss << pszChess[chsMove] << pszPos[9-mstep.pt1.x] << kszMove[MT_IN] << pszPos[mstep.pt2.y-mstep.pt1.y];
			else
				ss << pszChess[chsMove] << pszPos[9-mstep.pt1.x] << kszMove[MT_OUT] << pszPos[mstep.pt1.y-mstep.pt2.y];
		}
		else
			ss << pszChess[chsMove] << pszPos[9-mstep.pt1.x] << kszMove[MT_EQUAL] << pszPos[9-mstep.pt2.x];
		
		break;

	case CHSMAN_RED_JU://红车
	case CHSMAN_RED_PAO://红炮
		if(mstep.pt1.y>mstep.pt2.y)
		{//退
			if(iSameChs!=-1)
				ss << (iSameChs<mstep.pt1.y?kszMove[MT_FRONT]:kszMove[MT_BACK]) << pszChess[chsMove] << kszMove[MT_OUT] << pszPos[mstep.pt1.y-mstep.pt2.y];
			else
				ss << pszChess[chsMove] << pszPos[9-mstep.pt1.x] << kszMove[MT_OUT] << pszPos[mstep.pt1.y-mstep.pt2.y];
		}
		else if(mstep.pt1.y<mstep.pt2.y)
		{//进
			if(iSameChs!=-1)
				ss << (iSameChs<mstep.pt1.y?kszMove[MT_FRONT]:kszMove[MT_BACK]) << pszChess[chsMove] << kszMove[MT_IN] << pszPos[mstep.pt2.y-mstep.pt1.y];
			else
				ss << pszChess[chsMove] << pszPos[9-mstep.pt1.x] << kszMove[MT_IN] << pszPos[mstep.pt2.y-mstep.pt1.y];
		}else
		{//平
			if(iSameChs!=-1)
				ss << (iSameChs<mstep.pt1.y?kszMove[MT_FRONT]:kszMove[MT_BACK]) << pszChess[chsMove] << kszMove[MT_EQUAL] << pszPos[9-mstep.pt2.x];
			else
				ss << pszChess[chsMove] << pszPos[9-mstep.pt1.x] << kszMove[MT_EQUAL] << pszPos[9-mstep.pt2.x];
		}
		break;

	case CHSMAN_RED_MA://红马
	case CHSMAN_RED_SHI://红士
	case CHSMAN_RED_XIANG://红相
		if(iSameChs!=-1)
			ss << (iSameChs<mstep.pt1.y?kszMove[MT_FRONT]:kszMove[MT_BACK]) << pszChess[chsMove] << (mstep.pt1.y<mstep.pt2.y?kszMove[MT_IN]:kszMove[MT_OUT]) << pszPos[9-mstep.pt2.x];
		else
			ss << pszChess[chsMove] << pszPos[9-mstep.pt1.x] << (mstep.pt1.y<mstep.pt2.y?kszMove[MT_IN]:kszMove[MT_OUT]) << pszPos[9-mstep.pt2.x];
		
		break;

	case CHSMAN_RED_BING://红兵
		{
			int nCount=0;
			int nPos[5]={-1,-1,-1,-1,-1};
			for(int i=0;i<10;i++)
				if(tmpLayout[i][mstep.pt1.x]==CHSMAN_RED_BING && i!=mstep.pt1.y && i!=mstep.pt2.y)
				{
					nPos[nCount++]=i;
				}

				if(nCount==0)
				{//没有同列的兵
					if(mstep.pt1.y==mstep.pt2.y)
						ss << pszChess[chsMove] << pszPos[9-mstep.pt1.x] << kszMove[MT_EQUAL] << pszPos[9-mstep.pt2.x];
					else
						ss << pszChess[chsMove] << pszPos[9-mstep.pt1.x] << kszMove[MT_IN] << pszPos[1];
				}else if(nCount==1)
				{//有一颗同列的兵
					if(mstep.pt1.y==mstep.pt2.y)
						ss << (mstep.pt1.y>nPos[1]?kszMove[MT_FRONT]:kszMove[MT_BACK]) << pszChess[chsMove] << kszMove[MT_EQUAL] << pszPos[9-mstep.pt2.x];
					else
						ss << (mstep.pt1.y>nPos[1]?kszMove[MT_FRONT]:kszMove[MT_BACK]) << pszChess[chsMove] << kszMove[MT_IN] << pszPos[1];
				}else//if(nCount>1)
				{//多个兵在同一列
					int j=0;
					for(int i=0;i<5;i++)
						if(nPos[i]==-1)
							break;
						else if(nPos[i]>mstep.pt1.y)
							break;
						else
							j++;
						if(mstep.pt1.y==mstep.pt2.y)
							ss << pszPos[j] << pszChess[chsMove] << kszMove[MT_EQUAL] << pszPos[mstep.pt2.x];
						else
							ss << pszPos[j] << pszChess[chsMove] << kszMove[MT_IN] << pszPos[1];
				}
				
		}
		break;

	default:
		break;
	}
	_tcscpy(szDesc, ss.str().c_str());
}

BOOL CChessLayout::IsValidMove(POINT pt1 , POINT pt2,CChsLytState *pLytState)
{
	BOOL bRet=FALSE;
	BOOL bStateOwner=(pLytState==NULL);
	if(CHSMANSIDE(m_chesses[pt1.y][pt1.x])!=m_actSide) return FALSE;
	if(bStateOwner) pLytState=new CChsLytState(this,TRUE);
	std::vector<POINT> *pLstDest=&pLytState->m_ChessInfo[pt1.y][pt1.x]->m_lstDest;
	std::vector<POINT>::iterator p=pLstDest->begin();
	while(p!=pLstDest->end())
	{
		if(p->x==pt2.x && p->y==pt2.y 
			&& CHSMANSIDE(m_chesses[pt1.y][pt1.x])!=CHSMANSIDE(m_chesses[pt2.y][pt2.x]))
		{
			bRet=TRUE;
			break;
		}
		p++;
	}
	if(bStateOwner) delete pLytState;
	return bRet;
}

void CChessLayout::ExchangeSide(void)
{
	for(int y=0;y<5;y++) for(int x=0;x<9;x++)
	{
		CHESSMAN chs=m_chesses[y][x];
		m_chesses[y][x]=m_chesses[9-y][8-x];
		m_chesses[9-y][8-x]=chs;
		int nID=m_nChsID[y][x];
		m_nChsID[y][x]=m_nChsID[9-y][8-x];
		m_nChsID[9-y][8-x]=nID;
	}
	CHESSMAN chsBuf[16];
	memcpy(chsBuf,m_csDeadBlack,16*sizeof(CHESSMAN));
	memcpy(m_csDeadBlack,m_csDeadRed,16*sizeof(CHESSMAN));
	memcpy(m_csDeadRed,chsBuf,16*sizeof(CHESSMAN));
	int nTmp=m_nDeadBlack;
	m_nDeadBlack=m_nDeadRed;
	m_nDeadRed=nTmp;
	m_selfSide=(CHSSIDE)!m_selfSide;
}

int CChessLayout::GetAllChess(POINT ptChs[16],CHSSIDE side)
{
	int nCount=0;
	for(int y=0;y<10;y++) for(int x=0;x<9;x++)
	{
		if(CHSMANSIDE(m_chesses[y][x])==side)
		{
			ptChs[nCount].x=x,ptChs[nCount].y=y;
			nCount++;
		}
	}
	return nCount;
}