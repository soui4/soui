// JunQiMap.cpp: implementation of the CJunQiMap class.
//
//////////////////////////////////////////////////////////////////////
#include "windows.h"
#include "JunqiMap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define BORDER_AV		5

static const POINT kJQMapRailway[AC_RWSSIZE]=
{
	{6,1},{7,1},{8,1},{9,1},{10,1},{6,2},{10,2},{6,3},{10,3},{6,4},{10,4},{6,5},{7,5},{8,5},{9,5},{10,5},
	{1,6},{1,7},{1,8},{1,9},{1,10},{2,6},{2,10},{3,6},{3,10},{4,6},{4,10},{5,6},{5,7},{5,8},{5,9},{5,10},
	{6,11},{7,11},{8,11},{9,11},{10,11},{6,12},{10,12},{6,13},{10,13},{6,14},{10,14},{6,15},{7,15},{8,15},{9,15},{10,15},
	{11,6},{11,7},{11,8},{11,9},{11,10},{12,6},{12,10},{13,6},{13,10},{14,6},{14,10},{15,6},{15,7},{15,8},{15,9},{15,10},
	{6,6},{6,8},{6,10},{8,6},{8,8},{8,10},{10,6},{10,8},{10,10},	
};


//----------------------------------------------------------
//	获取一方的所有摆子位置，包括兵站和大本营
//----------------------------------------------------------
void CJunQiMap::GetArmyPositions(int nSide, POINT ptPos[25])
{
	static POINT ptSideBottom[25]={ {6,5},{7,5},{8,5},{9,5},{10,5},	//line1
									{6,4},		{8,4},		{10,4},	//line2
									{6,3},{7,3},	  {9,3},{10,3},	//line3
									{6,2},		{8,2},		{10,2},	//line4
									{6,1},{7,1},{8,1},{9,1},{10,1},	//line5
									{6,0},{7,0},{8,0},{9,0},{10,0},	//line6
								};
	if(nSide==0) 
		memcpy(ptPos,ptSideBottom,sizeof(POINT)*25);
	else
	{
		for(int i=0;i<25;i++)
			ptPos[i]=CJunQiMap::RotateCoordinate(ptSideBottom[i],0,nSide);
	}
}

POINT CJunQiMap::GetRailwayPos(int idx){
	return kJQMapRailway[idx];
}

BOOL CJunQiMap::PtEqual(POINT pt1,POINT pt2)
{
	return (pt1.x==pt2.x && pt1.y==pt2.y);
}

int CJunQiMap::RWSPtToIndex(POINT ptCell)
{
	for(int i=0;i<AC_RWSSIZE;i++)
		if(CJunQiMap::PtEqual(ptCell,kJQMapRailway[i])) 
			return i;
	return -1;
}

BOOL CJunQiMap::IsCellAdjecent(POINT pt1,POINT pt2) const {
	
	const char * pcAdj=m_cAdjacent[pt1.y][pt1.x];
	for(char i=0;i<pcAdj[24];i++)
	{
		POINT pt={ADJPOSX(pcAdj,i),ADJPOSY(pcAdj,i)};
		if(CJunQiMap::PtEqual(pt,pt2)) 
			return TRUE;
	}
	return FALSE;
}

CJunQiMap::CJunQiMap()
{
	InitCell();
	InitPath();
	InitRailway();
	MakeCellAdjList();
}

CJunQiMap::~CJunQiMap()
{

}

//************************************************
//	初始化棋盘的格子
//***********************************************
void CJunQiMap::InitCell()
{
	memset(m_nCells,0xff,sizeof(int)*17*17);//
	int x,y;
	//将所有有效位置设为兵站
	for(x=6;x<11;x++)
	{
		for(y=0;y<17;y++)
		{
			m_nCells[x][y]=CP_CORPS;
			m_nCells[y][x]=CP_CORPS;
		}
	}
	//去掉交叉位置的16个兵站
	m_nCells[7][6]=m_nCells[9][6]=CP_NULL;
	m_nCells[6][7]=m_nCells[7][7]=m_nCells[8][7]=m_nCells[9][7]=m_nCells[10][7]=CP_NULL;
	m_nCells[7][8]=m_nCells[9][8]=CP_NULL;
	m_nCells[6][9]=m_nCells[7][9]=m_nCells[8][9]=m_nCells[9][9]=m_nCells[10][9]=CP_NULL;
	m_nCells[7][10]=m_nCells[9][10]=CP_NULL;
	//8个大本营
	m_nCells[0][7]=m_nCells[0][9]=CP_PODIUM;
	m_nCells[7][0]=m_nCells[9][0]=CP_PODIUM;
	m_nCells[16][7]=m_nCells[16][9]=CP_PODIUM;
	m_nCells[7][16]=m_nCells[9][16]=CP_PODIUM;
	//20个兵营
	m_nCells[7][2]=m_nCells[2][7]=CP_BARBACK;
	m_nCells[7][4]=m_nCells[4][7]=CP_BARBACK;
	m_nCells[8][3]=m_nCells[3][8]=CP_BARBACK;
	m_nCells[9][2]=m_nCells[2][9]=CP_BARBACK;
	m_nCells[9][4]=m_nCells[4][9]=CP_BARBACK;
	m_nCells[12][7]=m_nCells[7][12]=CP_BARBACK;
	m_nCells[12][9]=m_nCells[9][12]=CP_BARBACK;
	m_nCells[13][8]=m_nCells[8][13]=CP_BARBACK;
	m_nCells[14][7]=m_nCells[7][14]=CP_BARBACK;
	m_nCells[14][9]=m_nCells[9][14]=CP_BARBACK;
}

void CJunQiMap::SetPathInfo(PACMPATH pPath,int x1,int y1,int x2,int y2,int nPathType)
{
	pPath->ptCellID1.x=x1;
	pPath->ptCellID1.y=y1;
	pPath->ptCellID2.x=x2;
	pPath->ptCellID2.y=y2;
	pPath->nPathType = nPathType;
}

//**********************************************************
//	填充路段信息
//**********************************************************
void CJunQiMap::InitPath()
{
	int i=0;
	//6 row
	SetPathInfo(m_acmPathes+i++, 6, 0, 7, 0,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 7, 0, 8, 0,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 8, 0, 9, 0,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 9, 0,10, 0,PT_ROAD);

	SetPathInfo(m_acmPathes+i++, 6, 1, 7, 1,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++, 7, 1, 8, 1,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++, 8, 1, 9, 1,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++, 9, 1,10, 1,PT_RAILWAY);

	SetPathInfo(m_acmPathes+i++, 6, 2, 7, 2,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 7, 2, 8, 2,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 8, 2, 9, 2,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 9, 2,10, 2,PT_ROAD);

	SetPathInfo(m_acmPathes+i++, 6, 3, 7, 3,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 7, 3, 8, 3,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 8, 3, 9, 3,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 9, 3,10, 3,PT_ROAD);

	SetPathInfo(m_acmPathes+i++, 6, 4, 7, 4,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 7, 4, 8, 4,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 8, 4, 9, 4,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 9, 4,10, 4,PT_ROAD);

	SetPathInfo(m_acmPathes+i++, 6, 5, 7, 5,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++, 7, 5, 8, 5,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++, 8, 5, 9, 5,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++, 9, 5,10, 5,PT_RAILWAY);
	//5 col
	SetPathInfo(m_acmPathes+i++, 6, 0, 6, 1,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 6, 1, 6, 2,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++, 6, 2, 6, 3,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++, 6, 3, 6, 4,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++, 6, 4, 6, 5,PT_RAILWAY);
	
	SetPathInfo(m_acmPathes+i++, 7, 0, 7, 1,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 7, 1, 7, 2,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 7, 2, 7, 3,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 7, 3, 7, 4,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 7, 4, 7, 5,PT_ROAD);

	SetPathInfo(m_acmPathes+i++, 8, 0, 8, 1,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 8, 1, 8, 2,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 8, 2, 8, 3,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 8, 3, 8, 4,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 8, 4, 8, 5,PT_ROAD);

	SetPathInfo(m_acmPathes+i++, 9, 0, 9, 1,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 9, 1, 9, 2,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 9, 2, 9, 3,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 9, 3, 9, 4,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 9, 4, 9, 5,PT_ROAD);

	SetPathInfo(m_acmPathes+i++,10, 0,10, 1,PT_ROAD);
	SetPathInfo(m_acmPathes+i++,10, 1,10, 2,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++,10, 2,10, 3,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++,10, 3,10, 4,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++,10, 4,10, 5,PT_RAILWAY);

	//16条斜线
	SetPathInfo(m_acmPathes+i++, 7, 2, 6, 1,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 7, 2, 8, 1,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 7, 2, 6, 3,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 7, 2, 8, 3,PT_ROAD);

	SetPathInfo(m_acmPathes+i++, 9, 2, 8, 1,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 9, 2,10, 1,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 9, 2, 8, 3,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 9, 2,10, 3,PT_ROAD);

	SetPathInfo(m_acmPathes+i++, 7, 4, 6, 3,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 7, 4, 8, 3,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 7, 4, 6, 5,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 7, 4, 8, 5,PT_ROAD);

	SetPathInfo(m_acmPathes+i++, 9, 4, 8, 3,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 9, 4,10, 3,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 9, 4, 8, 5,PT_ROAD);
	SetPathInfo(m_acmPathes+i++, 9, 4,10, 5,PT_ROAD);

	SetPathInfo(m_acmPathes+i++, 6, 5, 6, 6,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++, 8, 5, 8, 6,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++,10, 5,10, 6,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++, 6, 6, 8, 6,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++, 8, 6,10, 6,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++, 8, 6, 8, 8,PT_RAILWAY);
	int j,nSideCount=i;
	for(j=0;j<nSideCount;j++)
	{
		ACMPATH path=m_acmPathes[j];
		path.ptCellID1=RotateCoordinate(path.ptCellID1,2,3);
		path.ptCellID2=RotateCoordinate(path.ptCellID2,2,3);
		m_acmPathes[i++]=path;
	}
	for(j=0;j<nSideCount;j++)
	{
		ACMPATH path=m_acmPathes[j];
		path.ptCellID1=RotateCoordinate(path.ptCellID1,2,0);
		path.ptCellID2=RotateCoordinate(path.ptCellID2,2,0);
		m_acmPathes[i++]=path;
	}
	for(j=0;j<nSideCount;j++)
	{
		ACMPATH path=m_acmPathes[j];
		path.ptCellID1=RotateCoordinate(path.ptCellID1,2,1);
		path.ptCellID2=RotateCoordinate(path.ptCellID2,2,1);
		m_acmPathes[i++]=path;
	}
	//拐角
	SetPathInfo(m_acmPathes+i++, 6, 5, 5, 6,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++,10, 5,11, 6,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++,11,10,10,11,PT_RAILWAY);
	SetPathInfo(m_acmPathes+i++, 5,10, 6,11,PT_RAILWAY);
	
}

//********************************************************
//	初始化铁路信息
//********************************************************
void CJunQiMap::InitRailway()
{
	static const POINT railway1[]={{ 1, 6},{ 1, 7},{ 1, 8},{ 1, 9},{ 1,10}};
	static const POINT railway2[]={{15, 6},{15, 7},{15, 8},{15, 9},{15,10}};
	static const POINT railway3[]={{ 5, 6},{ 5, 7},{ 5, 8},{ 5, 9},{ 5,10}};
	static const POINT railway4[]={{11, 6},{11, 7},{11, 8},{11, 9},{11,10}};
	static const POINT railway5[]={{ 6, 1},{ 6, 2},{ 6, 3},{ 6, 4},{ 6, 5},{ 6, 6},{ 6, 8},{ 6,10},{ 6,11},{ 6,12},{ 6,13},{ 6,14},{ 6,15}};
	static const POINT railway6[]={{10, 1},{10, 2},{10, 3},{10, 4},{10, 5},{10, 6},{10, 8},{10,10},{10,11},{10,12},{10,13},{10,14},{10,15}};
	static const POINT railway7[]={{ 8, 5},{ 8, 6},{ 8, 8},{ 8,10},{ 8,11}};

	static const POINT railwayA[]={{ 6, 1},{ 7, 1},{ 8, 1},{ 9, 1},{10, 1}};
	static const POINT railwayB[]={{ 6,15},{ 7,15},{ 8,15},{ 9,15},{10,15}};
	static const POINT railwayC[]={{ 6, 5},{ 7, 5},{ 8, 5},{ 9, 5},{10, 5}};
	static const POINT railwayD[]={{ 6,11},{ 7,11},{ 8,11},{ 9,11},{10,11}};
	static const POINT railwayE[]={{ 1, 6},{ 2, 6},{ 3, 6},{ 4, 6},{ 5, 6},{ 6, 6},{ 8, 6},{10, 6},{11, 6},{12, 6},{13, 6},{14, 6},{15, 6}};
	static const POINT railwayF[]={{ 1,10},{ 2,10},{ 3,10},{ 4,10},{ 5,10},{ 6,10},{ 8,10},{10,10},{11,10},{12,10},{13,10},{14,10},{15,10}};
	static const POINT railwayG[]={{ 5, 8},{ 6, 8},{ 8, 8},{10, 8},{11, 8}};

	static const POINT railwayL1[]={{1,6},{2,6},{3,6},{4,6},{5,6},{6,5},{6,4},{6,3},{6,2},{6,1}};
	static const POINT railwayL2[]={{10,1},{10,2},{10,3},{10,4},{10,5},{11,6},{12,6},{13,6},{14,6},{15,6}};
	static const POINT railwayL3[]={{1,10},{2,10},{3,10},{4,10},{5,10},{6,11},{6,12},{6,13},{6,14},{6,15}};
	static const POINT railwayL4[]={{10,15},{10,14},{10,13},{10,12},{10,11},{11,10},{12,10},{13,10},{14,10},{15,10}};
	
	m_railways[0].nCells=5;
	memcpy(m_railways[0].arrCellPt,railway1,5*sizeof(POINT));
	m_railways[1].nCells=5;
	memcpy(m_railways[1].arrCellPt,railway2,5*sizeof(POINT));
	m_railways[2].nCells=5;
	memcpy(m_railways[2].arrCellPt,railway3,5*sizeof(POINT));
	m_railways[3].nCells=5;
	memcpy(m_railways[3].arrCellPt,railway4,5*sizeof(POINT));
	m_railways[4].nCells=13;
	memcpy(m_railways[4].arrCellPt,railway5,13*sizeof(POINT));
	m_railways[5].nCells=13;
	memcpy(m_railways[5].arrCellPt,railway6,13*sizeof(POINT));
	m_railways[6].nCells=5;
	memcpy(m_railways[6].arrCellPt,railway7,5*sizeof(POINT));

	m_railways[7].nCells=5;
	memcpy(m_railways[7].arrCellPt,railwayA,5*sizeof(POINT));
	m_railways[8].nCells=5;
	memcpy(m_railways[8].arrCellPt,railwayB,5*sizeof(POINT));
	m_railways[9].nCells=5;
	memcpy(m_railways[9].arrCellPt,railwayC,5*sizeof(POINT));
	m_railways[10].nCells=5;
	memcpy(m_railways[10].arrCellPt,railwayD,5*sizeof(POINT));
	m_railways[11].nCells=13;
	memcpy(m_railways[11].arrCellPt,railwayE,13*sizeof(POINT));
	m_railways[12].nCells=13;
	memcpy(m_railways[12].arrCellPt,railwayF,13*sizeof(POINT));
	m_railways[13].nCells=5;
	memcpy(m_railways[13].arrCellPt,railwayG,5*sizeof(POINT));

	m_railways[14].nCells=10;
	memcpy(m_railways[14].arrCellPt,railwayL1,10*sizeof(POINT));
	m_railways[15].nCells=10;
	memcpy(m_railways[15].arrCellPt,railwayL2,10*sizeof(POINT));
	m_railways[16].nCells=10;
	memcpy(m_railways[16].arrCellPt,railwayL3,10*sizeof(POINT));
	m_railways[17].nCells=10;
	memcpy(m_railways[17].arrCellPt,railwayL4,10*sizeof(POINT));

}


//********************************************************
//	构造每个棋格的邻接表
//********************************************************
void CJunQiMap::MakeCellAdjList()
{
	for(int y=0;y<17;y++) for(int x=0;x<17;x++) 
		m_cAdjacent[y][x][24]=0;
	for(int i=0;i<AC_PATHSIZE;i++)
	{
		POINT pt=m_acmPathes[i].ptCellID2;
		char *pAdj=m_cAdjacent[pt.y][pt.x];
		char cCount=pAdj[24];
		pAdj[cCount*3+0]=(char)m_acmPathes[i].ptCellID1.x;
		pAdj[cCount*3+1]=(char)m_acmPathes[i].ptCellID1.y;
		pAdj[cCount*3+2]=(char)m_acmPathes[i].nPathType;
		pAdj[24]++;

		pt=m_acmPathes[i].ptCellID1;
		pAdj=m_cAdjacent[pt.y][pt.x];
		cCount=pAdj[24];
		pAdj[cCount*3+0]=(char)m_acmPathes[i].ptCellID2.x;
		pAdj[cCount*3+1]=(char)m_acmPathes[i].ptCellID2.y;
		pAdj[cCount*3+2]=(char)m_acmPathes[i].nPathType;
		pAdj[24]++;
	}
}

//************************************************************
//	判断一个棋格是否在铁路上
//************************************************************
BOOL CJunQiMap::IsCellInRailway(POINT ptCell) const
{
	for(int i=0;i<18;i++)
	{
		for(int j=0;j<m_railways[i].nCells;j++)
		{
			if(PtEqual(ptCell,m_railways[i].arrCellPt[j])) return TRUE;
		}
	}
	return FALSE;
}

//****************************************************************
//	获取一个棋格在每条铁路上的信息
//	一个棋子最多同时在三条铁路上
//****************************************************************
int CJunQiMap::GetCellRailwayPos(POINT ptCell, RAILWAYPOS *prwps) const
{
	int nRet=0;
	for(int i=0;i<18;i++)
	{
		for(int j=0;j<m_railways[i].nCells;j++)
		{
			if(PtEqual(ptCell,m_railways[i].arrCellPt[j]))
			{
				if(prwps)
				{
					prwps[nRet].nRailway=i;
					prwps[nRet].nIndex=j;
				}
				nRet++;
			}
		}
	}
	return nRet;
}

//******************************************************
//	计算地图旋转的cos值
//	int nSeat[in]:两个位置之间的差
//******************************************************
int CJunQiMap::MapCos(int nSeats)
{
	switch(nSeats)
	{
	case 4:
	case 0: return 1;
	case -1:
	case 1: return 0;
	case -2:
	case 2: return -1;
	case -3:
	case 3: return 0;
	default:return 0xff;
	}
}

//******************************************************
//	计算地图旋转的sin值
//	int nSeat[in]:两个位置之间的差
//******************************************************
int CJunQiMap::MapSin(int nSeats)
{
	switch(nSeats)
	{
	case 4:
	case 0: return 0;
	case -1: return -1;
	case 1: return 1;
	case -2: 
	case 2: return 0;
	case -3: return 1;
	case 3: return -1;
	default:return 0xff;
	}
}

//***********************************************************
//	坐标旋转
//	POINT pt:原位置坐标
//	int nOldSeat:原来的方位
//	int nNewSeat:新方位
//	return :原位置在新方位的坐标
//	remark: pt应该是nOldSeat的坐标,另外注意屏幕坐标与标准坐标系的差异，
//			这关系到旋转角度的计算
//***********************************************************
POINT CJunQiMap::RotateCoordinate(POINT pt, int nOldSeat, int nNewSeat)
{
	POINT ptRet;
	int   nSeats=(nNewSeat-nOldSeat);
	ptRet.x=(pt.x-8)*MapCos(nSeats)-(pt.y-8)*MapSin(nSeats)+8;
	ptRet.y=(pt.x-8)*MapSin(nSeats)+(pt.y-8)*MapCos(nSeats)+8;
	return ptRet;
}

