#pragma once

enum{
	MAX_MOVES=	19,		//最多一个棋只可能有17个走法
    MAX_CHESS_COUNT = 32, //最多32个棋子
};

typedef enum tagCHESSMAN{
	//红方棋子,帅车马炮士相兵
	CHSMAN_RED_JIANG=0,	
	CHSMAN_RED_JU,
	CHSMAN_RED_MA,
	CHSMAN_RED_PAO,
	CHSMAN_RED_SHI,
	CHSMAN_RED_XIANG,
	CHSMAN_RED_BING,
	//黑方棋子,将车马炮士相卒
	CHSMAN_BLK_JIANG=7,
	CHSMAN_BLK_JU,
	CHSMAN_BLK_MA,
	CHSMAN_BLK_PAO,
	CHSMAN_BLK_SHI,
	CHSMAN_BLK_XIANG,
	CHSMAN_BLK_BING,
	//无棋状态
	CHSMAN_NULL=0xFF,
}CHESSMAN;

typedef enum tagChessSide{
	CS_RED=0,
	CS_BLACK=1,
	CS_NEUTRAL=CHSMAN_NULL/7,
	CS_NULL=-1
} CHSSIDE;
#define CHSMANSIDE(chsman) (CHSSIDE)((chsman)/7)


class CChsMoveGenerator
{
public:
	CChsMoveGenerator();
	virtual ~CChsMoveGenerator();

	/**
	 * @brief 获取当前棋子可能的走法
	 * @param chsmLayout [in] 棋盘布局
	 * @param bTopRed [in] 是否上部为红方
	 * @param ptFocus [in] 待移动棋子
	 * @param ptMoves [out] 可移动范围
	 * @return 可移动范围数
	 * @details 车、炮最多可以产生19种走法
	 */
	static int  GetPossiableMoves(const CHESSMAN chsmLayout[10][9],BOOL bTopRed,POINT ptFocus,POINT ptMoves[MAX_MOVES]);
	
	//!将红方与黑方棋子调换
	static void ConvertLayout(CHESSMAN chsmLayout[10][9]);
	//!互换棋子颜色
	static void ExchangeColor(CHESSMAN chsmLayout[10][9]);
protected:
	//7种棋子的走棋判断
	static int GetMoves_Jiang(const CHESSMAN chsLayout[10][9],POINT ptFocus,POINT ptMoves[19]);
	static int GetMoves_Ju(const CHESSMAN chsLayout[10][9],POINT ptFocus,POINT ptMoves[19]);
	static int GetMoves_Ma(const CHESSMAN chsLayout[10][9],POINT ptFocus,POINT ptMoves[19]);
	static int GetMoves_Pao(const CHESSMAN chsLayout[10][9],POINT ptFocus,POINT ptMoves[19]);
	static int GetMoves_Shi(const CHESSMAN chsLayout[10][9],POINT ptFocus,POINT ptMoves[19]);
	static int GetMoves_Xiang(const CHESSMAN chsLayout[10][9],POINT ptFocus,POINT ptMoves[19]);
	static int GetMoves_Bing(const CHESSMAN chsLayout[10][9],POINT ptFocus,POINT ptMoves[19]);
};
