// JunQiMap.h: interface for the CJunQiMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JUNQIMAP_H__8B300E84_F1E5_4B6C_8AF0_11AE73FD93B8__INCLUDED_)
#define AFX_JUNQIMAP_H__8B300E84_F1E5_4B6C_8AF0_11AE73FD93B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//棋格属性
#define CP_CORPS	0		//兵站
#define CP_BARBACK	1		//兵营
#define CP_PODIUM	2		//大本营
#define CP_NULL		-1		//无效位置

//道路类型
#define PT_NULL		-1		//无效道路
#define PT_ROAD		0		//公路
#define PT_RAILWAY	1		//铁路

//棋格在铁路上的位置
typedef struct tagRAILWAYPOS
{
	int nRailway;
	int nIndex;
}RAILWAYPOS;

#define AC_PATHSIZE	288		//道路数量
#define AC_RWSSIZE	73		//16*4+9 个在铁路上的兵营

#define FLAGDATA(pflagMetrix,x,y) (*(pflagMetrix+(17*x)+y))

//获取邻接表信息
#define ADJPOSX(pcAdj,index) (*(pcAdj+3*index+0))		//point.x
#define ADJPOSY(pcAdj,index) (*(pcAdj+3*index+1))		//point.y
#define ADJPATHTYPE(pcAdj,index) (*(pcAdj+3*index+2))	//path type

class CJunQiMap  
{
public:
	//用于保存铁路信息
	typedef struct tagRAILWAY
	{
		int		nCells;			//途经兵站个数
		POINT	arrCellPt[15];	//途经兵站位置
	}RAILWAY,*PRAILWAY;

	typedef struct tagACMPATH
	{
		POINT ptCellID1,ptCellID2;	//路段起点与终点ID
		int		nPathType;			//道路类型
	}ACMPATH,*PACMPATH;

public:
	CJunQiMap();
	virtual ~CJunQiMap();

public:
	static POINT RotateCoordinate(POINT pt,int nOldSeat,int nNewSeat);
	static BOOL PtEqual(POINT pt1,POINT pt2);
	static POINT GetRailwayPos(int iRailway);
	static int RWSPtToIndex(POINT ptCell);

	/**
	 * 获取某边兵营的兵站位置
	 * @param nSide: 0-bottom,1-left,2-top,3-right
	 * @param ptPos: 兵站位置 
	 */
	static void GetArmyPositions(int nSide, POINT ptPos[25]);

	int GetCellRailwayPos(POINT ptCell, RAILWAYPOS *prwps) const;
	BOOL IsCellInRailway(POINT ptCell) const;
	BOOL IsCellAdjecent(POINT pt1, POINT pt2) const;

	int	 GetCellType(POINT ptCell) const { return m_nCells[ptCell.y][ptCell.x];}
	int	 GetCellType(int x,int y) const { return m_nCells[y][x];}
	const char * GetCellAdjList(POINT ptCell) const { return m_cAdjacent[ptCell.y][ptCell.x];}
	const RAILWAY * GetRailway(int nRailway) const { return &m_railways[nRailway];}
private:
	int			m_nCells[17][17];		//棋格信息
	char		m_cAdjacent[17][17][25];//棋盘的邻接表,25=3*8+1,最多8个相邻点信息(x+y+PathType)+一字节结点数
	ACMPATH		m_acmPathes[AC_PATHSIZE];
	RAILWAY		m_railways[18];//18条铁路
private:
	static int MapSin(int nSeats);
	static int MapCos(int nSeats);
	void MakeCellAdjList();
	void InitRailway();
	void SetPathInfo(PACMPATH pPath,int x1,int y1,int x2,int y2,int nPathType);
	void InitPath();
	void InitCell();
};


#endif // !defined(AFX_JUNQIMAP_H__8B300E84_F1E5_4B6C_8AF0_11AE73FD93B8__INCLUDED_)
