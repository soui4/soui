// JunQiLayout.cpp: implementation of the CJunQiLayout class.
// 管理棋子布局
//////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <assert.h>
#include "JunqiLayout.h"

#define ASSERT(x) assert(x)

using namespace SOUI;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//8个大本营的位置
static const POINT kJQFlagPos[4][2]=
{
	{{7,0},{9,0}},
	{{0,7},{0,9}},
	{{7,16},{9,16}},
	{{16,7},{16,9}},
};

CJunQiLayout::CJunQiLayout()
    : m_dwBombFly(FALSE)
    , m_dwMineAsBomb(FALSE)
    , m_dwPlayers(4)
{
	memset(m_cellChess,0,sizeof(CELLCHESS)*17*17);
}

CJunQiLayout::~CJunQiLayout()
{
}

//********************************************************************
//	获取指定位置的棋子可以到达的所有位置
//	POINT ptCell:棋子位置
//	SOUI::SList<POINT> *pLstCell:棋子可到达位置表
//********************************************************************
int CJunQiLayout::GetChessmanTravelSpace(POINT ptCell, SOUI::SList<POINT> *pLstCell) const
{
	if(m_junQiMap.GetCellType(ptCell)==CP_PODIUM) 
		return 0;
	CELLCHESS cc=m_cellChess[ptCell.y][ptCell.x];
	ASSERT(cc.pos);//保证目标位置有棋
	pLstCell->RemoveAll();
	const SOUI::SList<CHESSMAN> *pLstTroop=GetTroopList(cc.nSide);
	CHESSMAN cm=pLstTroop->GetAt(cc.pos);

	if(cm.nOffRank==OR_MINE) return 0; //地雷不能动

	if(cm.nOffRank==OR_SAPPER||(cm.nOffRank==OR_BOMB&&m_dwBombFly))//工兵,炸弹满天飞
		return GetSapperTravelSpace(ptCell,pLstCell);

	const char	*pcAdj=m_junQiMap.GetCellAdjList(ptCell);
	for(char i=0;i<pcAdj[24];i++)
	{
		POINT pt={ADJPOSX(pcAdj,i),ADJPOSY(pcAdj,i)};
		if(IsValidMoveDest(cc.nSide,pt))
			pLstCell->AddTail(pt);
	}
	//查找铁路上的位置
	POINT pt;
	RAILWAYPOS rwps[3];
	int rws=m_junQiMap.GetCellRailwayPos(ptCell,rwps);
	if(rws!=0)
	{
		int i,j;
		for(i=0;i<rws;i++)
		{
			const CJunQiMap::RAILWAY *pRailway=m_junQiMap.GetRailway(rwps[i].nRailway);
			for(j=rwps[i].nIndex+1;j<pRailway->nCells;j++)
			{
				pt=pRailway->arrCellPt[j];
				if(IsValidMoveDest(cc.nSide,pt))
					AddCellDistinct(pLstCell,pt);
				if(m_cellChess[pt.y][pt.x].pos)
					break;
			}
			for(j=rwps[i].nIndex-1;j>=0;j--)
			{
				pt=pRailway->arrCellPt[j];
				if(IsValidMoveDest(cc.nSide,pt))
					AddCellDistinct(pLstCell,pt);
				if(m_cellChess[pt.y][pt.x].pos)
					break;
			}
		}
	}
	return pLstCell->GetCount();
}

int CJunQiLayout::GetChessmanTravelSpace(POINT ptCell, std::vector<POINT> *pLstCell) const
{
	SOUI::SList<POINT> lst;
	GetChessmanTravelSpace(ptCell,&lst);
	pLstCell->clear();
	pLstCell->resize(lst.GetCount());
	int i=0;
	SPOSITION pos=lst.GetHeadPosition();
	while(pos)
	{
		(*pLstCell)[i++]=lst.GetNext(pos);
	}
	return pLstCell->size();
}

//******************************************************************
//	将一个指定点加入到点集中，并保证当前点集中不存在指定点
//	return :TRUE-成功加入，FALSE-该点已经存在
//******************************************************************
BOOL CJunQiLayout::AddCellDistinct(SOUI::SList<POINT> *pLstCell, POINT ptCell) const
{
	POINT tmp;
	SPOSITION pos=pLstCell->GetHeadPosition();
	while(pos)
	{
		tmp=pLstCell->GetNext(pos);
		if(CJunQiMap::PtEqual(tmp,ptCell)) 	return FALSE;
	}
	pLstCell->AddTail(ptCell);
	return TRUE;
}


//**************************************************************
//	判断目标棋格是否是有较的目标
//**************************************************************
BOOL CJunQiLayout::IsValidMoveDest(int nSide, POINT ptCell) const
{
	if(m_cellChess[ptCell.y][ptCell.x].pos==NULL) 
		return TRUE;//目标棋格为空
	int nCellType=m_junQiMap.GetCellType(ptCell);
	if(nCellType==CP_BARBACK) 
		return FALSE;//是行营
	if(nSide==m_cellChess[ptCell.y][ptCell.x].nSide) 
		return FALSE;//被己方占据
	if(m_dwPlayers==4)
	{
		if((nSide%2)==(m_cellChess[ptCell.y][ptCell.x].nSide%2)) //被自己对家占据
			return FALSE;
	}
	if(nCellType==CP_PODIUM)
	{
		int nPodiumSide=GetPodiumSide(ptCell);
		if(nSide==nPodiumSide) return FALSE;	//自己的大本营不能进
		if(m_dwPlayers==4
			&&(nSide%2)==(nPodiumSide%2))		//对家的大本营不能进
			return FALSE;
	}
	return TRUE;
}

//*****************************************************
//	判断是否为有较的棋子
//*****************************************************
BOOL CJunQiLayout::IsValidCell(int nSide,POINT ptCell) const
{
	if(m_junQiMap.GetCellType(ptCell)==CP_PODIUM) 
		return FALSE;//大本营
	if(m_cellChess[ptCell.y][ptCell.x].pos==NULL) 
		return FALSE;//目标棋格为空
	if(m_cellChess[ptCell.y][ptCell.x].nSide!=nSide) 
		return FALSE;//不是操作方的棋
	SPOSITION pos=m_cellChess[ptCell.y][ptCell.x].pos;
	const SOUI::SList<CHESSMAN> *pLstChess=GetTroopList(nSide);
	const CHESSMAN &cm=pLstChess->GetAt(pos);
	if(cm.nOffRank==11)//地雷 
		return FALSE;
	return TRUE;
}

//***********************************************************
//	根据方位得到部队
//***********************************************************
const SOUI::SList<CHESSMAN> * CJunQiLayout::GetTroopList(int nSide) const
{
	ASSERT(nSide>=SIDE_BOTTOM&&nSide<=SIDE_RIGHT);
	return m_lstTroops+nSide;
}

SOUI::SList<CHESSMAN> * CJunQiLayout::GetTroopList2(int nSide){
	ASSERT(nSide>=SIDE_BOTTOM&&nSide<=SIDE_RIGHT);
	return m_lstTroops+nSide;
}
//********************************************************
//	比较两个棋子的大小
//	int nOffRank1, int nOffRank2:两个棋子的官职
//********************************************************
int CJunQiLayout::ChessmanCompare(int nOffRank1, int nOffRank2) const
{
	if( nOffRank1==OR_BOMB
		|| nOffRank2==OR_BOMB
		|| nOffRank1==nOffRank2)
	{
		return PUT_BOMB;
	}else if(nOffRank2==OR_MINE)
	{
		if(nOffRank1==OR_SAPPER)//工兵挖地雷
			return PUT_EAT;
		else if(m_dwMineAsBomb)
			return PUT_BOMB;
		else
			return PUT_BEEAT;
	}else if(nOffRank2==OR_FLAG)
	{
		return PUT_EAT;
	}else
	{
		int nCmp=nOffRank1-nOffRank2;
		if(nCmp>0)
			return PUT_EAT;
        else if (nCmp <0)
			return PUT_BEEAT;
        else // if(nCmp==0)
            return PUT_BOMB;
	}
}

//*****************************************************
//	拿掉一颗棋子
//	return:原位置棋子的大小
//*****************************************************
int CJunQiLayout::RemoveChessman(POINT ptCell)
{
	if(!m_cellChess[ptCell.y][ptCell.x].pos) return OR_NULL;
	SOUI::SList<CHESSMAN> *pTroopList=GetTroopList2(m_cellChess[ptCell.y][ptCell.x].nSide);
	CHESSMAN cm=pTroopList->GetAt(m_cellChess[ptCell.y][ptCell.x].pos);
	pTroopList->RemoveAt(m_cellChess[ptCell.y][ptCell.x].pos);
	m_cellChess[ptCell.y][ptCell.x].pos=NULL;
	m_cellChess[ptCell.y][ptCell.x].nSide=-1;
	return cm.nOffRank;
}

//****************************************************
//	改变棋子位置
//****************************************************
void CJunQiLayout::ChangeChessmanPos(POINT ptCell1, POINT ptCell2)
{
	ASSERT(m_cellChess[ptCell1.y][ptCell1.x].pos 
		&& !m_cellChess[ptCell2.y][ptCell2.x].pos);

	int nSide=m_cellChess[ptCell1.y][ptCell1.x].nSide;
	SPOSITION pos=m_cellChess[ptCell1.y][ptCell1.x].pos;
	CHESSMAN &pcm=m_lstTroops[nSide].GetAt(pos);
	pcm.ptCell=ptCell2;

	m_cellChess[ptCell1.y][ptCell1.x].nSide=-1;
	m_cellChess[ptCell1.y][ptCell1.x].pos=NULL;
	m_cellChess[ptCell2.y][ptCell2.x].nSide=nSide;
	m_cellChess[ptCell2.y][ptCell2.x].pos=pos;
}

//****************************************************
//	交换两个棋子位置，只在布局阶段使用
//****************************************************
BOOL CJunQiLayout::ExchangeChessman(POINT ptCell1, POINT ptCell2)
{
	if(CJunQiMap::PtEqual(ptCell1,ptCell2)) 
		return FALSE;
	int nRank1=GetCellChessOffRank(ptCell1);
	int nRank2=GetCellChessOffRank(ptCell2);
	if(nRank1==OR_NULL||nRank2==OR_NULL) 
		return FALSE;
    int nSide1 = GetCellChess(ptCell1).nSide;
    int nSide2 = GetCellChess(ptCell2).nSide;
	if(nSide1!=nSide2) 
		return FALSE;
	if(IsValidCellForChessman(ptCell1,nRank2)!=LE_OK ||
		IsValidCellForChessman(ptCell2,nRank1)!=LE_OK)
	{
		return FALSE;
	}
	SPOSITION pos1=m_cellChess[ptCell1.y][ptCell1.x].pos;
	SPOSITION pos2=m_cellChess[ptCell2.y][ptCell2.x].pos;
	if(!pos1 || !pos2) 
		return FALSE;
	SOUI::SList<CHESSMAN> *pLstChess=GetTroopList2(nSide1);
	CHESSMAN &cm1= pLstChess->GetAt(pos1);
	CHESSMAN &cm2= pLstChess->GetAt(pos2);
	cm1.ptCell=ptCell2;
	cm2.ptCell=ptCell1;
	m_cellChess[ptCell1.y][ptCell1.x].pos=pos2;
	m_cellChess[ptCell2.y][ptCell2.x].pos=pos1;

	return TRUE;
}

//********************************************************
//	获取在铁路上的工兵的漫游空间
//********************************************************
int CJunQiLayout::GetSapperTravelSpace(POINT ptCell, SOUI::SList<POINT> *pLstCell) const
{
	BYTE byFlagMetrix[17][17]={0};
	//通过递归方式获取工兵在铁路上可以到达的棋格
	int nSide=m_cellChess[ptCell.y][ptCell.x].nSide;
	GetSapperTravelSpaceRecursion(nSide,ptCell,pLstCell,(BYTE*)byFlagMetrix);

	const char	*pcAdj=m_junQiMap.GetCellAdjList(ptCell);
	for(char i=0;i<pcAdj[24];i++)
	{
		POINT pt={ADJPOSX(pcAdj,i),ADJPOSY(pcAdj,i)};
		if(ADJPATHTYPE(pcAdj,i)==PT_ROAD && IsValidMoveDest(nSide,pt))
			pLstCell->AddTail(pt);
	}

	return pLstCell->GetCount();
}

//********************************************************
//	获取在铁路上的工兵的漫游空间,private
//	int nSide:要走的棋子所在的方
//	POINT ptCell:入口位置
//	SOUI::SList<POINT> *pLstCell:返回位置表
//	BYTE *pbyFlagMetrix:处理标志,一个字节分成高4位与低4位两部分,
//						高4位批示该位置是否已经加入返回表,同时如果不是合法到达位置也要置位,
//						低4位只在该位置没有棋子时使用,指示该位置已经已经被当成入口处理过
//********************************************************
void CJunQiLayout::GetSapperTravelSpaceRecursion(int nSide,POINT ptCell,SOUI::SList<POINT> *pLstCell,BYTE *pbyFlagMetrix) const
{
	FLAGDATA(pbyFlagMetrix,ptCell.y,ptCell.x)=0x01;
	const char	*pcAdj=m_junQiMap.GetCellAdjList(ptCell);
	for(char i=0;i<pcAdj[24];i++)
	{
		if(ADJPATHTYPE(pcAdj,i)==PT_ROAD) continue;		//通过公路到达，不需要处理
		POINT pt={ADJPOSX(pcAdj,i),ADJPOSY(pcAdj,i)};
		if(FLAGDATA(pbyFlagMetrix,pt.y,pt.x)&0x01) continue;//如果已经被当作入口处理过，则不需要处理
		if(m_cellChess[pt.y][pt.x].pos)	
		{//有棋
			if(FLAGDATA(pbyFlagMetrix,pt.y,pt.x)&0x10) continue;
			FLAGDATA(pbyFlagMetrix,pt.y,pt.x)&=0x10;
			if(IsValidMoveDest(nSide,pt))
				pLstCell->AddTail(pt);
		}else
		{//没有棋子
			if(!(FLAGDATA(pbyFlagMetrix,pt.y,pt.x)&0x10))
			{
				FLAGDATA(pbyFlagMetrix,pt.y,pt.x)&=0x10;
				pLstCell->AddTail(pt);//没有棋子,不需要判断是不是合法位置
			}
			if(!(FLAGDATA(pbyFlagMetrix,pt.y,pt.x)&0x01))//只有没有棋子的地方才可以作为递归的入口
				GetSapperTravelSpaceRecursion(nSide,pt,pLstCell,pbyFlagMetrix);
		}
	}
}

#ifdef INFINITY
#undef INFINITY
#endif
#define INFINITY 0xffffffff

//*****************************************************************
//	采用Dijkstra算法产生最短路径
//	DWORD adjMatrix[AC_RWSSIZE][AC_RWSSIZE]:基础邻接矩阵
//	int nBegin:起点的ID
//	int nEnd: 终点的ID
//	return:TRUE-找到最短路径,FALSE-起点与终点不连通
//	更新记录:
//	2005-1-28:huangjx 增加路径途经交叉点个数判断，以保证在存在多条路径的情况下选择一条交点数最少的路径
//	2005-2-18:huangjx 优化计算途经交叉点算法,更改路径序列存储方式
//*****************************************************************
BOOL CJunQiLayout::FindShortestPath(
								  DWORD adjMatrix[AC_RWSSIZE][AC_RWSSIZE], 
								  int nBegin, 
								  int nEnd,
								  SOUI::SList<POINT> *pLstPath
								  ) const
{
	BOOL	bFind=FALSE;
	BOOL	bFinal[AC_RWSSIZE]={0};		//节点所属集合标志
	DWORD	dwDistance[AC_RWSSIZE];
	DWORD	dwCrosses[AC_RWSSIZE];		//用来记录每一个顶点到起点途经交叉点,以便在两条长度相同的最短路径中选择一条经过交点最少的路径
	int		nPathes[AC_RWSSIZE][AC_RWSSIZE];//用来记录任意点到nBegin的最短路径经过的点,{nPathes[v][0],nPahtes[v][nPathes[v][AC_RWSSIZE]-1]}=各路径点顺序ID
	int		nPathSize[AC_RWSSIZE]={0};	//每一条路径所包含的点数

	memset(dwCrosses,0xff,AC_RWSSIZE*sizeof(DWORD));

	//step1:查找所有和起点相连的点
	for(int v=0;v<AC_RWSSIZE;v++)
	{
		dwDistance[v]=adjMatrix[nBegin][v];
		if(dwDistance[v]!=INFINITY)
		{//初始状态下如果点v到nBegin有弧则认为这两点都在最短路径上，因为它们是一条路径的起点及终点
			dwCrosses[v]=1;

			nPathSize[v]=2;
			nPathes[v][0]=nBegin;
			nPathes[v][1]=v;
		}
	}
	dwDistance[nBegin]=0,dwCrosses[nBegin]=0,bFinal[nBegin]=TRUE;

	nPathSize[nBegin]=1;
	nPathes[nBegin][0]=nBegin;
	//TRACE("Find Path:begin point=(%d,%d)\n",g_ptJQMapRailway[nBegin].x,g_ptJQMapRailway[nBegin].y);

	//step2:最多重复AC_RWSSIZE-1次查找其它关键点
	int nCount=0;
	while(nCount<AC_RWSSIZE-1)
	{
		//step2.1:查找下一个到起点最短的点
		int nGet=-1;
		DWORD dwMinCross=INFINITY,dwMinDis=INFINITY;
		for(int v=0;v<AC_RWSSIZE;v++)
		{
			if(bFinal[v] || dwDistance[v]==INFINITY) continue;
			if(dwDistance[v]>dwMinDis) continue;
			if(dwDistance[v]==dwMinDis && dwCrosses[v]>=dwMinCross) continue;
			nGet=v;
			dwMinDis=dwDistance[v];
			dwMinCross=dwCrosses[v];
		}
		if(nGet==-1) break;
		bFinal[nGet]=TRUE;
		bFind=(nGet==nEnd);
		if(bFind) break;
		//step2.2:更新其它未找出但与新找出点相连点到起点的最短路径及距离
		for(int v=0;v<AC_RWSSIZE;v++)
		{
			if(bFinal[v] || adjMatrix[nGet][v]==INFINITY) continue;
			BOOL bExchange=FALSE;
			if(dwMinDis+adjMatrix[nGet][v]<dwDistance[v])
				bExchange=TRUE;
			else if(dwMinDis+adjMatrix[nGet][v]==dwDistance[v])
			{
				if(dwCrosses[v]>dwMinCross+1)
					bExchange=TRUE;
			}
			if(bExchange)
			{
				dwDistance[v]=dwMinDis+adjMatrix[nGet][v];
				memcpy(nPathes[v],nPathes[nGet],AC_RWSSIZE*sizeof(int));
				nPathSize[v]=nPathSize[nGet];
				nPathes[v][nPathSize[v]]=v;
				nPathSize[v]++;
				dwCrosses[v]=dwMinCross+1;
			}
		}
		nCount++;
	}
	if(pLstPath)
	{
		for(int i=0;i<nPathSize[nEnd];i++)
			pLstPath->AddTail(CJunQiMap::GetRailwayPos(nPathes[nEnd][i]));
	}
	return bFind;
}

//---------------------------------------------------------
//	采用邻接表数据结构计算最短路径算法
//	采用Dijkstra算法产生最短路径
//	DWORD adjMatrix[AC_RWSSIZE][AC_RWSSIZE]:基础邻接矩阵
//	int nBegin:起点的ID
//	int nEnd: 终点的ID
//	return:TRUE-找到最短路径,FALSE-起点与终点不连通
//	更新记录:
//	2005-1-28:huangjx 增加路径途经交叉点个数判断，以保证在存在多条路径的情况下选择一条交点数最少的路径
//	2005-2-18:huangjx 优化计算途经交叉点算法,更改路径序列存储方式
//	2005-2-24:huangjx 将图的数据结构从邻接矩阵模式改成邻接表模式，可以提高速度
//---------------------------------------------------------
BOOL CJunQiLayout::FindShortestPath(
								  ADJACENT *adjacent[AC_RWSSIZE], 
								  int		nAdjCount[AC_RWSSIZE],
								  int nBeginID, 
								  int nEndID,
								  SOUI::SList<POINT> *pLstPath
								  ) const
{
	BOOL	bFind=FALSE;
	BOOL	bFinal[AC_RWSSIZE]={0};		//节点所属集合标志
	DWORD	dwDistance[AC_RWSSIZE];
	DWORD	dwCrosses[AC_RWSSIZE];		//用来记录每一个顶点到起点途经交叉点,以便在两条长度相同的最短路径中选择一条经过交点最少的路径
	int		nPathes[AC_RWSSIZE][AC_RWSSIZE];//用来记录任意点到nBegin的最短路径经过的点,{nPathes[v][0],nPahtes[v][nPathes[v][AC_RWSSIZE]-1]}=各路径点顺序ID
	int		nPathSize[AC_RWSSIZE]={0};	//每一条路径所包含的点数

	memset(dwCrosses,0xff,AC_RWSSIZE*sizeof(DWORD));
	memset(dwDistance,0xff,AC_RWSSIZE*sizeof(DWORD));

	//step1:查找所有和起点相连的点
	for(int i=0;i<nAdjCount[nBeginID];i++)
	{//初始状态下如果点v到nBegin有弧则认为这两点都在最短路径上，因为它们是一条路径的起点及终点
		int nAdjID=adjacent[nBeginID][i].nID;
		dwDistance[nAdjID]=adjacent[nBeginID][i].dwDis;
		dwCrosses[nAdjID]=1;
		nPathSize[nAdjID]=2;
		nPathes[nAdjID][0]=nBeginID;
		nPathes[nAdjID][1]=nAdjID;
	}
	dwDistance[nBeginID]=0,dwCrosses[nBeginID]=0,bFinal[nBeginID]=TRUE;

	nPathSize[nBeginID]=1;
	nPathes[nBeginID][0]=nBeginID;
	//TRACE("Find Path:begin point=(%d,%d)\n",g_ptJQMapRailway[nBegin].x,g_ptJQMapRailway[nBegin].y);

	//step2:最多重复AC_RWSSIZE-1次查找其它关键点
	int nCount=0;
	while(nCount<AC_RWSSIZE-1)
	{
		//step2.1:查找下一个到起点最短的点
		int nGet=-1;
		DWORD dwMinCross=INFINITY,dwMinDis=INFINITY;
		for(int v=0;v<AC_RWSSIZE;v++)
		{
			if(bFinal[v] || dwDistance[v]==INFINITY) continue;
			if(dwDistance[v]>dwMinDis) continue;
			if(dwDistance[v]==dwMinDis && dwCrosses[v]>=dwMinCross) continue;
			nGet=v;
			dwMinDis=dwDistance[v];
			dwMinCross=dwCrosses[v];
		}
		if(nGet==-1) break;
		bFinal[nGet]=TRUE;
		bFind=(nGet==nEndID);
		if(bFind) break;
		//step2.2:更新其它未找出但与新找出点相连点到起点的最短路径及距离
		for(int i=0;i<nAdjCount[nGet];i++)
		{
			int nAdjID=adjacent[nGet][i].nID;
			if(bFinal[nAdjID]) continue;
			DWORD dwSegDis=adjacent[nGet][i].dwDis;

			BOOL bExchange=FALSE;
			if(dwMinDis+dwSegDis<dwDistance[nAdjID])
				bExchange=TRUE;
			else if(dwMinDis+dwSegDis==dwDistance[nAdjID])
			{
				if(dwCrosses[nAdjID]>dwMinCross+1)
					bExchange=TRUE;
			}
			if(bExchange)
			{
				dwDistance[nAdjID]=dwMinDis+dwSegDis;
				memcpy(nPathes[nAdjID],nPathes[nGet],AC_RWSSIZE*sizeof(int));
				nPathSize[nAdjID]=nPathSize[nGet];
				nPathes[nAdjID][nPathSize[nAdjID]]=nAdjID;
				nPathSize[nAdjID]++;
				dwCrosses[nAdjID]=dwMinCross+1;
			}
		}
		nCount++;
	}
	if(pLstPath)
	{
		for(int i=0;i<nPathSize[nEndID];i++)
			pLstPath->AddTail(CJunQiMap::GetRailwayPos(nPathes[nEndID][i]));
	}
	return bFind;
}

//*********************************************************
//	查找工兵的行走路径
//	remark: 使用Dijkstra算法,
//			在使用该方法前应先将终点的棋子暂时隐藏
//*********************************************************
void CJunQiLayout::FindSapperPath(POINT ptBegin, POINT ptEnd,SOUI::SList<POINT> *pLstPath) const
{
	//为了方便构造邻接矩阵，将起点与终点的棋子暂时屏蔽
	SPOSITION posSave1=m_cellChess[ptBegin.y][ptBegin.x].pos;
	SPOSITION posSave2=m_cellChess[ptEnd.y][ptEnd.x].pos;
	m_cellChess[ptBegin.y][ptBegin.x].pos=0;
	m_cellChess[ptEnd.y][ptEnd.x].pos=0;
	//构造邻接表
	ADJACENT *adjacent[AC_RWSSIZE];
	int		 nAdjCount[AC_RWSSIZE]={0};
	for(int i=0;i<AC_RWSSIZE;i++)
	{
		adjacent[i]=new ADJACENT[4];
		POINT   ptCell=CJunQiMap::GetRailwayPos(i);
		const char	*pcAdj=m_junQiMap.GetCellAdjList(ptCell);
		for(char c=0;c<pcAdj[24];c++)
		{
			if(ADJPATHTYPE(pcAdj,c)!=PT_RAILWAY) continue;
			POINT pt={ADJPOSX(pcAdj,c),ADJPOSY(pcAdj,c)};
			if(m_cellChess[ptCell.y][ptCell.x].pos) continue;

			DWORD dwDis=1;
			if(ptCell.y==pt.y)
				dwDis=abs(ptCell.x-pt.x);
			else if(ptCell.x==pt.x)
				dwDis=abs(ptCell.y-pt.y);
			adjacent[i][nAdjCount[i]].dwDis=dwDis;
			adjacent[i][nAdjCount[i]].nID=CJunQiMap::RWSPtToIndex(pt);
			nAdjCount[i]++;
		}
	}
	//恢复起点与终点的棋子信息
	m_cellChess[ptBegin.y][ptBegin.x].pos=posSave1;
	m_cellChess[ptEnd.y][ptEnd.x].pos=posSave2;
	FindShortestPath(adjacent,nAdjCount,CJunQiMap::RWSPtToIndex(ptBegin),CJunQiMap::RWSPtToIndex(ptEnd),pLstPath);
	for(int i=0;i<AC_RWSSIZE;i++) 
		delete []adjacent[i];
}

BOOL CJunQiLayout::InitLayout(int nSide, const int nMat[6][5])
{
	//check for layout validity
	for(int y=0;y<6;y++) for(int x=0;x<5;x++)
	{
		if(nMat[y][x] == OR_NULL) 
			continue;
        POINT ptCell = { x + 6, y }; 
		if(IsValidCellForChessman(ptCell,nMat[y][x])!=LE_OK)
			return FALSE;
	}

	//clear the troop list
	SOUI::SList<CHESSMAN> * pLstTroop=GetTroopList2(nSide);
	pLstTroop->RemoveAll();

	for(int y=0;y<6;y++) for(int x=0;x<5;x++)
	{
		if(nMat[y][x] == OR_NULL) 
			continue;
		POINT ptCell = {x+6, y};
		if(nSide != 0){
			ptCell=CJunQiMap::RotateCoordinate(ptCell,SIDE_BOTTOM,nSide);
		}
		CHESSMAN cm;
		cm.bShow=TRUE;
		cm.nOffRank = nMat[y][x];
		cm.ptCell = ptCell;
		cm.nID = pLstTroop->GetCount(); 
		m_cellChess[ptCell.y][ptCell.x].pos=pLstTroop->AddTail(cm);
		m_cellChess[ptCell.y][ptCell.x].nSide=nSide;
	}
	return TRUE;
}

//*********************************************************
//	利用布阵图信息初始化玩家的布局
//	int nSide:玩家的方位
//	BYTE *pbyLayout:布阵图信息,13个字节
//*********************************************************
BOOL CJunQiLayout::InitLayout(int nSide, const BYTE byLayout[13])
{
	int nMat[6][5] = {0};
	int i=0;
	for(int y=5;y>=0;y--) for(int x=0;x<5;x++)
	{
        if (m_junQiMap.GetCellType(6+x, y) == CP_BARBACK)
            continue;
		int idx = i/2;
		if(i%2==0)
			nMat[y][x] = (byLayout[idx]>>4) & 0x0f;
		else
			nMat[y][x] = byLayout[idx] & 0x0f;
		i++;
	}
	 return InitLayout(nSide,nMat);
}


//********************************************************
//	为指定方构造一个布阵图
//	int nSide:指定的方位
//	BYTE *pbyLayout:布阵图的返回数据
//	return :TRUE-成功,FALSE-失败
//	remark:只有指定方的数据完整才能成功
//********************************************************
BOOL CJunQiLayout::MakeLayout(int nSide, BYTE byLayout[13]) const
{
	const SOUI::SList<CHESSMAN> * pLstTroop=GetTroopList(nSide);
	if(pLstTroop->GetCount()!=25) return FALSE;

	POINT ptArmyPositions[25];
	CJunQiMap::GetArmyPositions(nSide,ptArmyPositions);
	for(int i=0;i<25;i++)
	{
		int	nOffRank=pLstTroop->GetAt(m_cellChess[ptArmyPositions[i].y][ptArmyPositions[i].x].pos).nOffRank;
		if(i%2==0)
            byLayout[i / 2] = nOffRank << 4;
		else
            byLayout[i / 2] |= nOffRank;
	}
	return TRUE;
}

//*****************************************************************
//	将一个棋子放到指定的棋格
//	POINT ptCell:目标棋格
//	int nOffRank:棋子
//	int nSide:棋子所属方位
//*****************************************************************
void CJunQiLayout::SetCellChessman(POINT ptCell, int nOffRank, int nSide,BOOL bShow)
{
	ASSERT(nOffRank!=OR_NULL);
	ASSERT(m_cellChess[ptCell.y][ptCell.x].pos==NULL);
	SOUI::SList<CHESSMAN> *pTroopList=GetTroopList2(nSide);
	CHESSMAN cm;
	cm.nOffRank=nOffRank;
	cm.ptCell=ptCell;
	cm.bShow=bShow;
	cm.nID = pTroopList->GetCount();
	m_cellChess[ptCell.y][ptCell.x].pos=pTroopList->AddTail(cm);
	m_cellChess[ptCell.y][ptCell.x].nSide=nSide;
}

//************************************************************
//	获取棋子的大小
//************************************************************
int CJunQiLayout::GetCellChessOffRank(POINT ptCell) const
{
	if(!m_cellChess[ptCell.y][ptCell.x].pos) return OR_NULL;
	const SOUI::SList<CHESSMAN> *pTroopList=GetTroopList(m_cellChess[ptCell.y][ptCell.x].nSide);
	CHESSMAN cm=pTroopList->GetAt(m_cellChess[ptCell.y][ptCell.x].pos);
	return cm.nOffRank;
}

//************************************************************
//	比较两个棋子的大小，同MoveChessmanServer
//************************************************************
int CJunQiLayout::GetCmpResult(POINT ptCell1, POINT ptCell2) const
{
	if(!m_cellChess[ptCell2.y][ptCell2.x].pos)	
		return PUT_MOVE;

	SPOSITION pos=m_cellChess[ptCell1.y][ptCell1.x].pos;
	SPOSITION posEnemy=m_cellChess[ptCell2.y][ptCell2.x].pos;
	const SOUI::SList<CHESSMAN> *pLstChess1=GetTroopList(m_cellChess[ptCell1.y][ptCell1.x].nSide);
	const SOUI::SList<CHESSMAN> *pLstChess2=GetTroopList(m_cellChess[ptCell2.y][ptCell2.x].nSide);
	int nOffRank1=pLstChess1->GetAt(pos).nOffRank;
	int nOffRank2=pLstChess2->GetAt(posEnemy).nOffRank;
	return ChessmanCompare(nOffRank1,nOffRank2);
}

void CJunQiLayout::MoveChessmanClient(POINT ptCell1, POINT ptCell2,int nResult)
{
	switch(nResult)
	{
	case PUT_MOVE:
		ChangeChessmanPos(ptCell1,ptCell2);
		break;
	case PUT_EAT:
		RemoveChessman(ptCell2);
		ChangeChessmanPos(ptCell1,ptCell2);
		break;
	case PUT_BEEAT:
		RemoveChessman(ptCell1);
		break;
	case PUT_BOMB:
		RemoveChessman(ptCell1);
		RemoveChessman(ptCell2);
		break;
	}
}

//*************************************************************
//	服务器使用的棋子移动接口
//	POINT ptCell1:棋子起始位置
//	POINT ptCell2：棋子目标位置
//	return:棋子比较结果
//	remark:由服务器端调用
//*************************************************************
int CJunQiLayout::MoveChessmanServer(POINT ptCell1, POINT ptCell2)
{
	int nResult=GetCmpResult(ptCell1,ptCell2);
	MoveChessmanClient(ptCell1,ptCell2,nResult);
	return nResult;
}


//***************************************************
//	翻开棋子
//	POINT ptCell:棋子位置;
//	int nOffRank:棋子大小
//	return:棋子所属的方位
//***************************************************
int CJunQiLayout::ViewChessman(POINT ptCell,int nOffRank)
{
	CELLCHESS cc=m_cellChess[ptCell.y][ptCell.x];
	ASSERT(cc.pos);
	SOUI::SList<CHESSMAN> *plstTroop=GetTroopList2(cc.nSide);
	CHESSMAN &cm=plstTroop->GetAt(cc.pos);
	cm.nOffRank=nOffRank;
	return cc.nSide;
}

//*******************************************************
//	玩家战败，将该玩家的棋子从棋盘上拿掉
//*******************************************************
void CJunQiLayout::PlayerLose(int nSide)
{
	SOUI::SList<CHESSMAN> *pLstTroop=GetTroopList2(nSide);
	SPOSITION pos=pLstTroop->GetHeadPosition();
	CHESSMAN cm;
	while(pos)
	{
		cm=pLstTroop->GetNext(pos);
		m_cellChess[cm.ptCell.y][cm.ptCell.x].pos=NULL;
		m_cellChess[cm.ptCell.y][cm.ptCell.x].nSide=-1;
	}
	pLstTroop->RemoveAll();
}

BOOL CJunQiLayout::IsPlayerLost(int nSide) const{
	return m_lstTroops[nSide].GetCount()==0;
}

//*****************************************************
//	删除布局
//******************************************************
void CJunQiLayout::DeleteLayout()
{
	for(int y=0;y<MAT_WIDTH;y++)	for(int x=0;x<MAT_WIDTH;x++)
	{
		m_cellChess[y][x].nSide=-1;
		m_cellChess[y][x].pos=NULL;
	}
	for(int i=0;i<4;i++) m_lstTroops[i].RemoveAll();
}

//******************************************************
//	判断一个玩家是否还有可以自由移动的棋子
//******************************************************
BOOL CJunQiLayout::HasFreeChessman(int nSide) const
{
	const SOUI::SList<CHESSMAN> *plstTroop=GetTroopList(nSide);
	SPOSITION pos=plstTroop->GetHeadPosition();
	CHESSMAN cm;
	while(pos)
	{
		cm=plstTroop->GetNext(pos);
		if(IsChessmanFree(cm.ptCell)) return TRUE;
	}
	return FALSE;
}

//***********************************************************
//	判断一个棋子是否可以自由移动
//***********************************************************
BOOL CJunQiLayout::IsChessmanFree(POINT ptCell) const
{
	if(m_junQiMap.GetCellType(ptCell)==CP_PODIUM) 
		return FALSE;
	if(GetCellChessOffRank(ptCell)==11) 
		return FALSE;//地雷不能移动
	int nSide=m_cellChess[ptCell.y][ptCell.x].nSide;

	const char *pcAdj=m_junQiMap.GetCellAdjList(ptCell);
	for(char i=0;i<pcAdj[24];i++)
	{
		POINT pt={ADJPOSX(pcAdj,i),ADJPOSY(pcAdj,i)};
		if(IsValidMoveDest(nSide,pt)) 
			return TRUE;
	}
	return FALSE;
}

int CJunQiLayout::GetPodiumSide(POINT ptPodium) const
{
	ASSERT(m_junQiMap.GetCellType(ptPodium)==CP_PODIUM);
	if(ptPodium.x==0)
		return 0;
	else if(ptPodium.y==0)
		return 1;
	else if(ptPodium.x==16)
		return 2;
	else
		return 3;
}


//************************************************************
//	BOOL bSapper:是否为工兵
//	POINT ptCell1[in]:棋子起始位置
//	POINT ptCell2[in]：棋子目标位置
//	SOUI::SList<POINT> *pLstPath[out]:棋子走子路径
//************************************************************
void CJunQiLayout::GetMoveCMPath(BOOL bSapper, POINT ptCell1, POINT ptCell2, SOUI::SList<POINT> *pLstPath) const
{
	int		irw1=CJunQiMap::RWSPtToIndex(ptCell1);
	int     irw2=CJunQiMap::RWSPtToIndex(ptCell2);
	pLstPath->RemoveAll();
	if(irw1!=-1&&irw2!=-1)
	{
		if(bSapper)//工兵
		{
			FindSapperPath(ptCell1,ptCell2,pLstPath);
		}else
		{
			int nDestRailway=-1,index=0;
			int idxBegin,idxEnd;
			while(nDestRailway==-1&&index<18)
			{
				BOOL bGetBegin=FALSE,bGetEnd=FALSE;
				const CJunQiMap::RAILWAY *pRailway=m_junQiMap.GetRailway(index);
				for(int j=0;j<pRailway->nCells;j++)
				{
					if(CJunQiMap::PtEqual(pRailway->arrCellPt[j],ptCell1))
						idxBegin=j,bGetBegin=TRUE;
					else if(CJunQiMap::PtEqual(pRailway->arrCellPt[j],ptCell2))
						idxEnd=j,bGetEnd=TRUE;
				}
				if(bGetBegin&&bGetEnd)
					nDestRailway=index;
				else
					index++;
			}
            if (nDestRailway != -1)
            {
                int nStep = idxEnd > idxBegin ? 1 : -1;
                index = idxBegin - nStep;
                while (index != idxEnd)
                {
                    index += nStep;
                    pLstPath->AddTail(m_junQiMap.GetRailway(nDestRailway)->arrCellPt[index]);
                }
            }
		}
	}else if(pLstPath)
	{
		pLstPath->AddTail(ptCell1);
		int cellType2 = m_junQiMap.GetCellType(ptCell2);
		if(cellType2 != CP_NULL){
			if(m_junQiMap.IsCellAdjecent(ptCell1,ptCell2)){
				pLstPath->AddTail(ptCell2);
			}
		}
	}
}

void CJunQiLayout::GetMoveCMPath(BOOL bSapper, POINT ptCell1, POINT ptCell2, std::vector<POINT> *pLstPath) const
{
	SOUI::SList<POINT> lst;
	GetMoveCMPath(bSapper,ptCell1,ptCell2,&lst);
	pLstPath->clear();
	pLstPath->resize(lst.GetCount());
	int i=0;
	SPOSITION pos=lst.GetHeadPosition();
	while(pos)
	{
		(*pLstPath)[i++]=lst.GetNext(pos);
	}
}

BOOL CJunQiLayout::ClearLayout(int nSide){
	SOUI::SList<CHESSMAN> * pLstTroop=GetTroopList2(nSide);
	for(SOUI::SPOSITION pos=pLstTroop->GetHeadPosition();pos;){
		CHESSMAN cm=pLstTroop->GetNext(pos);
		m_cellChess[cm.ptCell.y][cm.ptCell.x].pos=NULL;
		m_cellChess[cm.ptCell.y][cm.ptCell.x].nSide=-1;
	}
	pLstTroop->RemoveAll();
	return TRUE;
}

//********************************************************
//	为指定方构造一个布阵图
//	int nSide:指定的方位
//	int nMatStone[][17]:布阵图的返回数据
//	return :TRUE-成功,FALSE-失败
//	remark:只有指定方的数据完整才能成功
//********************************************************
BOOL CJunQiLayout::MakeLayout(int nSide,int nMatStone[][17]) const
{
	for(int y=0;y<17;y++) for(int x=0;x<17;x++)
	{
		if(m_cellChess[y][x].nSide==nSide)
		{
			const SOUI::SList<CHESSMAN> *pLstChess=GetTroopList(nSide);
			nMatStone[y][x]=nSide*100+pLstChess->GetAt(m_cellChess[y][x].pos).nOffRank;
		}else
		{
			nMatStone[y][x]=OR_NULL;
		}
	}
	return TRUE;
}

//*******************************************************
//	获取一方的军旗的位置
//	int nSide:方位
//	return:军旗坐标
//*******************************************************
POINT CJunQiLayout::GetFlagPosition(int nSide)
{
	if(GetCellChessOffRank(kJQFlagPos[nSide][0])==OR_FLAG)
		return kJQFlagPos[nSide][0];
	else
		return kJQFlagPos[nSide][1];		
}

//*******************************************************
//	使用布阵图矩阵初始化棋局
//	remark:nMatStone[y][x]=nSide*100+OFFRANK
//*******************************************************
void CJunQiLayout::InitLayout(const int nMatStone[][MAT_WIDTH],int nSide/*=-1*/)
{
	for(int y=0;y<MAT_WIDTH;y++) for(int x=0;x<MAT_WIDTH;x++)
	{
		if(!nMatStone[y][x]) continue;
		int nTmpSide=nMatStone[y][x]/100;
		if(nSide!=-1&&nSide!=nTmpSide) continue;
		POINT pt={x,y};
		SetCellChessman(pt,nMatStone[y][x]%100,nTmpSide);
	}
}

//--------------------------------------------------
//	反转单个棋子
//--------------------------------------------------
BOOL CJunQiLayout::ConvertChess(POINT ptCell)
{
	SPOSITION pos=m_cellChess[ptCell.y][ptCell.x].pos;
	if(!pos) return FALSE;
	int nSide=m_cellChess[ptCell.y][ptCell.x].nSide;
	SOUI::SList<CHESSMAN> *pLstChess=GetTroopList2(nSide);
	CHESSMAN &cm=pLstChess->GetAt(pos);
	cm.bShow=!cm.bShow;
	return TRUE;
}

//---------------------------------------------------
//	反转一方的棋，以当前输入的棋子为标准
//	return:返回反转显示的方位
//---------------------------------------------------
int CJunQiLayout::ConvertASide(POINT ptCell)
{
	SPOSITION pos=m_cellChess[ptCell.y][ptCell.x].pos;
	if(!pos) return -1;
	int nSide=m_cellChess[ptCell.y][ptCell.x].nSide;
	SOUI::SList<CHESSMAN> *pLstChess=GetTroopList2(nSide);
	CHESSMAN &cm=pLstChess->GetAt(pos);
	BOOL bShow=!cm.bShow;
	pos=pLstChess->GetHeadPosition();
	while(pos)
	{
		pLstChess->GetNext(pos).bShow=bShow;		
		pLstChess->GetNext(pos);		
	}
	return nSide;
}

BOOL CJunQiLayout::IsChessShow(POINT ptCell) const
{
	ASSERT(m_cellChess[ptCell.y][ptCell.x].pos);
	SOUI::SPOSITION pos=m_cellChess[ptCell.y][ptCell.x].pos;
	const SOUI::SList<CHESSMAN> *pLstChess=GetTroopList(m_cellChess[ptCell.y][ptCell.x].nSide);
	const CHESSMAN &cm=pLstChess->GetAt(pos);
	return cm.bShow;
}

BOOL CJunQiLayout::IsSapper(POINT ptCell) const
{
	CHESSMAN cm=GetCellChessMan(ptCell);
	return cm.nOffRank == OR_SAPPER || (cm.nOffRank == OR_BOMB && m_dwBombFly);
}

BOOL CJunQiLayout::IsValidMove(POINT ptFrom,POINT ptTo,int nSide) const
{
	CELLCHESS ccFrom = GetCellChess(ptFrom);
	if(ccFrom.nSide != nSide) 
		return FALSE;
	CHESSMAN cm=GetCellChessMan(ptFrom);
	if(cm.nOffRank==-1) 
		return FALSE;
	if((cm.nOffRank == OR_MINE && !m_dwMineAsBomb) || cm.nOffRank == OR_FLAG)
		return FALSE;
	CELLCHESS ccTo = GetCellChess(ptTo);
	if(ccTo.pos){
		if(ccTo.nSide == nSide) 
			return FALSE;
		if(m_dwPlayers == 4 && (nSide%2) == (ccTo.nSide%2))
			return FALSE;
		if(m_junQiMap.GetCellType(ptTo) == CP_BARBACK)
			return FALSE;
	}
	
	std::vector<POINT> lstPath;
	GetMoveCMPath(IsSapper(ptFrom),ptFrom,ptTo,&lstPath);
	BOOL bValid = FALSE;
	for (const auto& pt : lstPath)
	{
		if (pt.x == ptTo.x && pt.y == ptTo.y)
		{
			bValid = TRUE;
			break;
		}
	}
	return bValid;
}

CHESSMAN CJunQiLayout::GetCellChessMan(POINT ptCell) const
{
	CHESSMAN ret={-1,0};
	if(!m_cellChess[ptCell.y][ptCell.x].pos) return ret;
	SOUI::SPOSITION pos=m_cellChess[ptCell.y][ptCell.x].pos;
	const SOUI::SList<CHESSMAN> *pLstChess=GetTroopList(m_cellChess[ptCell.y][ptCell.x].nSide);
	return pLstChess->GetAt(pos);
}

CELLCHESS CJunQiLayout::GetCellChess(POINT ptCell) const
{
    return m_cellChess[ptCell.y][ptCell.x];
}

/**
 * 判断目标格子是否对某方有效
 * 1、军旗只能话大本营
 * 2、地雷只能放在最后两排
 * 3、炸弹不能话在第一排
 * 4、所有棋子不能话在行营
 */
LayoutError CJunQiLayout::IsValidCellForChessman(POINT ptCell, int nRank) const
{
	const int nSide = 0;
	if(m_junQiMap.GetCellType(ptCell)==CP_BARBACK) 
		return LE_INVALID_CELL;
    switch(nRank){
		case OR_FLAG:
			return (CJunQiMap::PtEqual(ptCell,kJQFlagPos[nSide][0])||
				CJunQiMap::PtEqual(ptCell,kJQFlagPos[nSide][1]))
				? LE_OK : LE_INVALID_FLAG;
		case OR_MINE:
		    if(m_dwMineAsBomb) 
				return IsValidCellForChessman(ptCell,OR_BOMB);
			return ptCell.y<2 ? LE_OK : LE_INVALID_MINE;
		case OR_BOMB:
			return ptCell.y<5 ? LE_OK : LE_INVALID_BOMB;
		default:
			return LE_OK;
	}
}