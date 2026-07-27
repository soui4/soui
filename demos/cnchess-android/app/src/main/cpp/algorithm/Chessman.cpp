#include "stdafx.h"
#include "Chessman.h"

CChsMoveGenerator::CChsMoveGenerator()
{
}

CChsMoveGenerator::~CChsMoveGenerator(void)
{
}


//棋盘上下对调
void CChsMoveGenerator::ConvertLayout(CHESSMAN chsmLayout[10][9])
{
	for(int y=0;y<5;y++) for(int x=0;x<9;x++)
	{
		CHESSMAN csTmp=chsmLayout[9-y][8-x];
		chsmLayout[9-y][8-x]=chsmLayout[y][x];
		chsmLayout[y][x]=csTmp;
	}
}

void CChsMoveGenerator::ExchangeColor(CHESSMAN chsmLayout[10][9])
{
	for(int y=0;y<10;y++) for(int x=0;x<9;x++)
	{
		switch(CHSMANSIDE(chsmLayout[y][x]))
		{
		case CS_RED:chsmLayout[y][x]=(CHESSMAN)(chsmLayout[y][x]+7);break;
		case CS_BLACK:chsmLayout[y][x]=(CHESSMAN)(chsmLayout[y][x]-7);break;
		default:break;
		}
	}
}

int  CChsMoveGenerator::GetPossiableMoves(const CHESSMAN chsmLayout[10][9],BOOL bTopRed,POINT ptFocus,POINT ptMoves[MAX_MOVES])
{
	CHESSMAN tmpLayout[10][9];
	memcpy(tmpLayout,chsmLayout,10*9*sizeof(CHESSMAN));//备分布局，所有操作在备分中进行
	if(!bTopRed) ExchangeColor(tmpLayout);//颜色交换一下

	CHESSMAN chs=tmpLayout[ptFocus.y][ptFocus.x];
	if(chs==CHSMAN_NULL) return 0;//保证当前位置有棋子
	if(CHSMANSIDE(chs)==CS_BLACK)
	{//当发现是黑方行棋时，将棋盘中的棋子方向对调一下。
		ConvertLayout(tmpLayout);
		ExchangeColor(tmpLayout);
		ptFocus.x=8-ptFocus.x;
		ptFocus.y=9-ptFocus.y;
		int nRet=GetPossiableMoves(tmpLayout,TRUE,ptFocus,ptMoves);
		//将返回值调整方向
		for(int i=0;i<nRet;i++) ptMoves[i].x=8-ptMoves[i].x,ptMoves[i].y=9-ptMoves[i].y;
		return nRet;
	}
	//获得移动区(红方移动，红方在下)
	assert(CHSMANSIDE(chs)==CS_RED);
	int nRet=0;
	switch(chs)
	{
	case CHSMAN_RED_JIANG:nRet=GetMoves_Jiang(tmpLayout,ptFocus,ptMoves);	break;
	case CHSMAN_RED_JU:nRet=GetMoves_Ju(tmpLayout,ptFocus,ptMoves);	break;
	case CHSMAN_RED_MA:nRet=GetMoves_Ma(tmpLayout,ptFocus,ptMoves);	break;
	case CHSMAN_RED_PAO:nRet=GetMoves_Pao(tmpLayout,ptFocus,ptMoves);	break;
	case CHSMAN_RED_SHI:nRet=GetMoves_Shi(tmpLayout,ptFocus,ptMoves);	break;
	case CHSMAN_RED_XIANG:nRet=GetMoves_Xiang(tmpLayout,ptFocus,ptMoves);	break;
	case CHSMAN_RED_BING:nRet=GetMoves_Bing(tmpLayout,ptFocus,ptMoves);	break;
	}
	//检查目标位置是否有己方棋子
	int nValid=0;
	for(int i=0;i<nRet;i++)
	{
		CHESSMAN cs=tmpLayout[ptMoves[i].y][ptMoves[i].x];
		if(cs==CHSMAN_NULL || CHSMANSIDE(cs)!=CS_RED)
		{
			ptMoves[nValid++]=ptMoves[i];
		}
	}
	return nValid;
}



//将军的移动
int CChsMoveGenerator::GetMoves_Jiang(const CHESSMAN chsLayout[10][9],POINT ptFocus,POINT ptMoves[19])
{
	int nRet=0;
	assert(CHSMAN_RED_JIANG==chsLayout[ptFocus.y][ptFocus.x]);
	//向左移动
	if(ptFocus.x>=4)
	{
		ptMoves[nRet].x=ptFocus.x-1;
		ptMoves[nRet].y=ptFocus.y;
		nRet++;
	}
	if(ptFocus.x<=4)
	{
		ptMoves[nRet].x=ptFocus.x+1;
		ptMoves[nRet].y=ptFocus.y;
		nRet++;
	}
	//纵向移动
	if(ptFocus.y>=1)
	{
		ptMoves[nRet].x=ptFocus.x;
		ptMoves[nRet].y=ptFocus.y-1;
		nRet++;
	}
	if(ptFocus.y<=1)
	{
		ptMoves[nRet].x=ptFocus.x;
		ptMoves[nRet].y=ptFocus.y+1;
		nRet++;
	}
	//将帅对面检测
    POINT ptEnemyKing = { -1, -1 };
	for(int y=7;y<10;y++) for(int x=3;x<6;x++)
	{
		if(chsLayout[y][x]==CHSMAN_BLK_JIANG)
		{
			ptEnemyKing.y=y,ptEnemyKing.x=x;
			break;
		}
	}
	if(ptEnemyKing.x==ptFocus.x)
	{
		BOOL bBlocked=FALSE;
		for(int y=ptFocus.y+1;y<ptEnemyKing.y;y++)
		{
			if(chsLayout[y][ptFocus.x]!=CHSMAN_NULL)
			{
				bBlocked=TRUE;
				break;
			}
		}
		if(!bBlocked)
		{
			ptMoves[nRet++]=ptEnemyKing;
		}
	}
	return nRet;
}

//车的移动
int CChsMoveGenerator::GetMoves_Ju(const CHESSMAN chsLayout[10][9],POINT ptFocus,POINT ptMoves[19])
{
	int nRet=0;
	assert(CHSMAN_RED_JU==chsLayout[ptFocus.y][ptFocus.x]);
	//横向移动
	POINT pt=ptFocus;
	while(pt.x<8)
	{
		ptMoves[nRet].x=pt.x+1;
		ptMoves[nRet].y=pt.y;
		nRet++;
		pt.x++;
		if(chsLayout[pt.y][pt.x]!=CHSMAN_NULL) break;
	}
	pt=ptFocus;
	while(pt.x>0)
	{
		ptMoves[nRet].x=pt.x-1;
		ptMoves[nRet].y=pt.y;
		nRet++;
		pt.x--;
		if(chsLayout[pt.y][pt.x]!=CHSMAN_NULL) break;
	}
	//纵向移动
	pt=ptFocus;
	while(pt.y<9)
	{
		ptMoves[nRet].x=pt.x;
		ptMoves[nRet].y=pt.y+1;
		nRet++;
		pt.y++;
		if(chsLayout[pt.y][pt.x]!=CHSMAN_NULL) break;
	}
	pt=ptFocus;
	while(pt.y>0)
	{
		ptMoves[nRet].x=pt.x;
		ptMoves[nRet].y=pt.y-1;
		nRet++;
		pt.y--;
		if(chsLayout[pt.y][pt.x]!=CHSMAN_NULL) break;
	}
	return nRet;
}

//马的移动
int CChsMoveGenerator::GetMoves_Ma(const CHESSMAN chsLayout[10][9],POINT ptFocus,POINT ptMoves[19])
{
	int nRet=0;
	assert(CHSMAN_RED_MA==chsLayout[ptFocus.y][ptFocus.x]);
	//上方
	if(ptFocus.y<=7 && chsLayout[ptFocus.y+1][ptFocus.x]==CHSMAN_NULL)
	{
		if(ptFocus.x>=1 )
		{
			ptMoves[nRet].x=ptFocus.x-1;
			ptMoves[nRet].y=ptFocus.y+2;
			nRet++;
		}
		if(ptFocus.x<=7)
		{
			ptMoves[nRet].x=ptFocus.x+1;
			ptMoves[nRet].y=ptFocus.y+2;
			nRet++;
		}		
	}
	//下方
	if(ptFocus.y>=2 && chsLayout[ptFocus.y-1][ptFocus.x]==CHSMAN_NULL)
	{
		if(ptFocus.x>=1 )
		{
			ptMoves[nRet].x=ptFocus.x-1;
			ptMoves[nRet].y=ptFocus.y-2;
			nRet++;
		}
		if(ptFocus.x<=7)
		{
			ptMoves[nRet].x=ptFocus.x+1;
			ptMoves[nRet].y=ptFocus.y-2;
			nRet++;
		}		
	}
	//左方
	if(ptFocus.x>=2 && chsLayout[ptFocus.y][ptFocus.x-1]==CHSMAN_NULL)
	{
		if(ptFocus.y>=1 )
		{
			ptMoves[nRet].x=ptFocus.x-2;
			ptMoves[nRet].y=ptFocus.y-1;
			nRet++;
		}
		if(ptFocus.y<=8)
		{
			ptMoves[nRet].x=ptFocus.x-2;
			ptMoves[nRet].y=ptFocus.y+1;
			nRet++;
		}		
	}
	//右方
	if(ptFocus.x<=6 && chsLayout[ptFocus.y][ptFocus.x+1]==CHSMAN_NULL)
	{
		if(ptFocus.y>=1 )
		{
			ptMoves[nRet].x=ptFocus.x+2;
			ptMoves[nRet].y=ptFocus.y-1;
			nRet++;
		}
		if(ptFocus.y<=8)
		{
			ptMoves[nRet].x=ptFocus.x+2;
			ptMoves[nRet].y=ptFocus.y+1;
			nRet++;
		}		
	}
	return nRet;
}

//炮的移动
int CChsMoveGenerator::GetMoves_Pao(const CHESSMAN chsLayout[10][9],POINT ptFocus,POINT ptMoves[19])
{
	int nRet=0;
	assert(CHSMAN_RED_PAO==chsLayout[ptFocus.y][ptFocus.x]);
	//向右移动
	POINT pt=ptFocus;
	while(pt.x<8)
	{
		if(chsLayout[pt.y][pt.x+1]!=CHSMAN_NULL){pt.x++; break;}
		ptMoves[nRet].x=pt.x+1;
		ptMoves[nRet].y=pt.y;
		nRet++;
		pt.x++;
	}
	while(pt.x<8)
	{
		if(chsLayout[pt.y][pt.x+1]==CHSMAN_NULL)
		{
			pt.x++;
		}else
		{
			ptMoves[nRet].x=pt.x+1;
			ptMoves[nRet].y=pt.y;
			nRet++;
			break;
		}
	}
	//向左移动
	pt=ptFocus;
	while(pt.x>0)
	{
		if(chsLayout[pt.y][pt.x-1]!=CHSMAN_NULL){pt.x--; break;}
		ptMoves[nRet].x=pt.x-1;
		ptMoves[nRet].y=pt.y;
		nRet++;
		pt.x--;
	}
	while(pt.x>0)
	{
		if(chsLayout[pt.y][pt.x-1]==CHSMAN_NULL)
		{
			pt.x--;
		}else
		{
			ptMoves[nRet].x=pt.x-1;
			ptMoves[nRet].y=pt.y;
			nRet++;
			break;
		}
	}
	//向上移动
	pt=ptFocus;
	while(pt.y<9)
	{
		if(chsLayout[pt.y+1][pt.x]!=CHSMAN_NULL){pt.y++; break;}
		ptMoves[nRet].x=pt.x;
		ptMoves[nRet].y=pt.y+1;
		nRet++;
		pt.y++;
	}
	while(pt.y<9)
	{
		if(chsLayout[pt.y+1][pt.x]==CHSMAN_NULL)
		{
			pt.y++;
		}else
		{
			ptMoves[nRet].x=pt.x;
			ptMoves[nRet].y=pt.y+1;
			nRet++;
			break;
		}
	}
	//向下移动
	pt=ptFocus;
	while(pt.y>0)
	{
		if(chsLayout[pt.y-1][pt.x]!=CHSMAN_NULL){pt.y--; break;}
		ptMoves[nRet].x=pt.x;
		ptMoves[nRet].y=pt.y-1;
		nRet++;
		pt.y--;
	}
	while(pt.y>0)
	{
		if(chsLayout[pt.y-1][pt.x]==CHSMAN_NULL)
		{
			pt.y--;
		}else
		{
			ptMoves[nRet].x=pt.x;
			ptMoves[nRet].y=pt.y-1;
			nRet++;
			break;
		}
	}
	return nRet;
}

//仕的移动
int CChsMoveGenerator::GetMoves_Shi(const CHESSMAN chsLayout[10][9],POINT ptFocus,POINT ptMoves[19])
{
	int nRet=0;
	assert(CHSMAN_RED_SHI==chsLayout[ptFocus.y][ptFocus.x]);
	if(ptFocus.x>=4 && ptFocus.y<=1) //测试左上方
	{
		ptMoves[nRet].x=ptFocus.x-1;
		ptMoves[nRet].y=ptFocus.y+1;
		nRet++;
	}
	if(ptFocus.x<=4 && ptFocus.y<=1) //测试右上方
	{
		ptMoves[nRet].x=ptFocus.x+1;
		ptMoves[nRet].y=ptFocus.y+1;
		nRet++;
	}
	if(ptFocus.x>=4 && ptFocus.y>=1) //测试左下方
	{
		ptMoves[nRet].x=ptFocus.x-1;
		ptMoves[nRet].y=ptFocus.y-1;
		nRet++;
	}
	if(ptFocus.x<=4 && ptFocus.y>=1) //测试右下方
	{
		ptMoves[nRet].x=ptFocus.x+1;
		ptMoves[nRet].y=ptFocus.y-1;
		nRet++;
	}	
	return nRet;
}

//象的移动
int CChsMoveGenerator::GetMoves_Xiang(const CHESSMAN chsLayout[10][9],POINT ptFocus,POINT ptMoves[19])
{
	int nRet=0;
	assert(CHSMAN_RED_XIANG==chsLayout[ptFocus.y][ptFocus.x]);
	if(ptFocus.x>=2 && ptFocus.y<=2) //测试左上方
	{
		if(chsLayout[ptFocus.y+1][ptFocus.x-1]==CHSMAN_NULL)
		{
			ptMoves[nRet].x=ptFocus.x-2;
			ptMoves[nRet].y=ptFocus.y+2;
			nRet++;
		}
	}
	if(ptFocus.x<=6 && ptFocus.y<=2) //测试右上方
	{
		if( chsLayout[ptFocus.y+1][ptFocus.x+1]==CHSMAN_NULL)
		{
			ptMoves[nRet].x=ptFocus.x+2;
			ptMoves[nRet].y=ptFocus.y+2;
			nRet++;
		}
	}
	if(ptFocus.x>=2 && ptFocus.y>=2) //测试左下方
	{
		if(chsLayout[ptFocus.y-1][ptFocus.x-1]==CHSMAN_NULL)
		{
			ptMoves[nRet].x=ptFocus.x-2;
			ptMoves[nRet].y=ptFocus.y-2;
			nRet++;
		}
	}
	if(ptFocus.x<=6 && ptFocus.y>=2) //测试右下方
	{
		if(chsLayout[ptFocus.y-1][ptFocus.x+1]==CHSMAN_NULL)
		{
			ptMoves[nRet].x=ptFocus.x+2;
			ptMoves[nRet].y=ptFocus.y-2;
			nRet++;
		}
	}
	return nRet;
}

//兵的移动
int CChsMoveGenerator::GetMoves_Bing(const CHESSMAN chsLayout[10][9],POINT ptFocus,POINT ptMoves[19])
{
	int nRet=0;
	assert(CHSMAN_RED_BING==chsLayout[ptFocus.y][ptFocus.x]);
	if(ptFocus.y<10-1)
	{
		ptMoves[nRet].x=ptFocus.x;
		ptMoves[nRet].y=ptFocus.y+1;
		nRet++;
	}
	if(ptFocus.y>=5)
	{
		if(ptFocus.x>0)
		{
			ptMoves[nRet].x=ptFocus.x-1;
			ptMoves[nRet].y=ptFocus.y;
			nRet++;
		}
		if(ptFocus.x<8)
		{
			ptMoves[nRet].x=ptFocus.x+1;
			ptMoves[nRet].y=ptFocus.y;
			nRet++;
		}
	}
	return nRet;
}


