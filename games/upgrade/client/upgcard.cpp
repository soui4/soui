#include "stdafx.h"
#include "upgcard.h"
#include <UpgAlgorithm.h>

int CUpgCard::ClientCompare(int nCard1, int nCard2,int nLevel,int nMainColor,BOOL b2ConstMain)
{	
	if(nCard1==nCard2) return 0;
	if(nCard1==-1) return -1;
	if(nCard2==-1) return 1;

	BOOL bKing1=(nCard1>=52);
	BOOL bKing2=(nCard2>=52);
	if(bKing1&&bKing2) return nCard1-nCard2;
	if(bKing1&&!bKing2) return 1;
	if(!bKing1&&bKing2) return -1;

	int nColor1=nCard1/13;
	int nValue1=nCard1%13;
	int nColor2=nCard2/13;
	int nValue2=nCard2%13;
	
	if(nColor1==nColor2)
	{
		if(nValue1==nLevel) return 1;
		if(nValue2==nLevel) return -1;
		int nValue1A=(nValue1+12)%13;
		int nValue2A=(nValue2+12)%13;
		if(b2ConstMain && nValue1A==0) return 1;
		if(b2ConstMain && nValue2A==0) return -1;
		return 	nValue1A-nValue2A;	
	}else
	{
		if(nColor1==nMainColor) nColor1+=4;	//将主花色做一下转换
		if(nColor2==nMainColor) nColor2+=4;

		if(nValue1==nValue2) return nColor1-nColor2;

		if(nValue1==nLevel) return 1;
		if(nValue2==nLevel) return -1;

		int nValue1A=(nValue1+12)%13;
		int nValue2A=(nValue2+12)%13;
		if(b2ConstMain && nValue1A==0) return 1;
		if(b2ConstMain && nValue2A==0) return -1;
		
		return nColor1-nColor2;
	}
}

int CUpgCard::ClientGetCard(int nColor, int nValue)
{
	if(nColor==COLOR_SKING) return 52;
	if(nColor==COLOR_BKING) return 53;
	return nColor*13+nValue;
}

int CUpgCard::ClientGetValue(int nCard){
	if(nCard == 52 || nCard == 53) 
		return 13;//king
	return nCard%13;
}

int CUpgCard::ClientGetColor(int nCard)
{
	if(nCard == -1)
		return -1;
	if(nCard==52) return COLOR_SKING;
	if(nCard==53) return COLOR_BKING;
	return nCard/13;
}

//计算牌的花色，正主、副主是一个花色
int CUpgCard::ClientGetColor2(int nCard,int nMainColor,int nLevelCard,BOOL b2ConstMain)
{
	if(nCard>=52) return COLOR_MAIN;
	int nColor=ClientGetColor(nCard);
	if(nColor==nMainColor) return COLOR_MAIN;
	int nValue=nCard%13;
	if(nValue==nLevelCard) return COLOR_MAIN;
	if(nValue==1 && b2ConstMain) return COLOR_MAIN;
	return nColor;
}

//如果一张牌是主牌，调用它来计算牌值，以便确定两个主牌是否相邻
//２是常主，牌值范围：0~12,14+15,16+17,20,21
//２不是常主，牌值范围：0~12,16+17,20,21
int CUpgCard::ClientGetMainValue(int nCard,int nMainColor,int nLevelCard,BOOL b2ConstMain)
{
	int nRet=0;
	if(nCard>=52)
	{
		nRet=nCard-52+20;
	}else
	{
		int nColor=ClientGetColor(nCard);
		int nValue=(nCard%13+12)%13;//调整为2->A的顺序
		if(nValue==((nLevelCard+12)%13))
		{//是级牌
			nRet=16;
			if(nColor==nMainColor) nRet++;
		}else if(nValue==0 && b2ConstMain)
		{//是常主的2
			nRet=14;
			if(nColor==nMainColor) nRet++;
		}else
		{
			nRet=nValue;
		}
	}
	return nRet;
}

//测试两张牌是否相邻
//nMainColor:0~5
//nLevelCard:0->12=A->K
BOOL CUpgCard::ClientIsAdjcent(int nCard1, int nCard2,int nMainColor,int nLevelCard,BOOL b2ConstMain)
{
	SASSERT(nMainColor>=0 && nMainColor<=5);
	int nColor1=ClientGetColor2(nCard1,nMainColor,nLevelCard,b2ConstMain);
	int nColor2=ClientGetColor2(nCard2,nMainColor,nLevelCard,b2ConstMain);
	if(nColor1!=nColor2) return FALSE;
	if(nColor1!=COLOR_MAIN)
	{//不是主花色,两张牌不会是级牌，也不会是常主的2
		int nValue1=(nCard1%13+12)%13;//调整为2->A的顺序
		int nValue2=(nCard2%13+12)%13;//调整为2->A的顺序
		if(abs(nValue1-nValue2)==1) 
			return TRUE;

		if((nValue1+nValue2)==(((nLevelCard+12)%13)*2) && abs(nValue1-nValue2)==2)//两张牌之间夹着级牌
			return TRUE;
		else
			return FALSE;
	}else
	{//主花色
		int nValue1=ClientGetMainValue(nCard1,nMainColor,nLevelCard,b2ConstMain);
		int nValue2=ClientGetMainValue(nCard2,nMainColor,nLevelCard,b2ConstMain);
		if(abs(nValue1-nValue2)==1)
			return TRUE;
		if(nValue1>12 || nValue2>12) 
			return FALSE;//如果是正主则不存在中间夹级牌的问题

		if((nValue1+nValue2)==(((nLevelCard+12)%13)*2) && abs(nValue1-nValue2)==2)//两张牌之间夹着级牌
			return TRUE;	
		else
			return FALSE;
	}
}

//计算牌型,nType=2,4,6,8等2-对子,4-2拖,6-3拖...
// return:指定的类型的数量
int CUpgCard::ClientGetCardType(const int *pCard,int nCards,int nType,int nMainColor,int nLevelCard,int b2ConstMain)
{
	int nRet=0;
	int i=0;
	while(i<nCards-1)
	{
		if(pCard[i]==pCard[i+1])
		{//找到第一个对子
			int card=pCard[i];
			i+=2;
			BOOL bFind=TRUE;
			for(int j=1;j<nType/2;j++)
			{//查找符合要求的后续对子
				while(TRUE)
				{
					if(!ClientIsAdjcent(card,pCard[i],nMainColor,nLevelCard,b2ConstMain))
					{
						bFind=FALSE;
						i++;
						break;
					}
					i++;
					if(i>=nCards)
					{
						bFind=FALSE;
						break;
					}
					if( pCard[i]==pCard[i-1])
					{//找到一对与上一对相邻的牌
						card=pCard[i];
						//找到下一张牌值比较时不相同的牌
						while(i<nCards-1)
						{
							if(ClientCompare(card,pCard[i+1],nLevelCard,nMainColor,b2ConstMain)==0)
								i++;
							else
								break;
						}
						break;
					}
					i++;	
				}
				if(!bFind) break;
				if(i>=nCards) break;
			}
			if(bFind) nRet++;
		}else
		{
			i++;
		}
	}
	return nRet;
}

//计算指定牌型的牌
//int *pCard,int nCards:输入牌数据
//int nType:牌型
//int *pRet:输出缓冲区,输出的数据是pCard的索引号
int CUpgCard::ClientGetTypeCard(const int *pCard, int nCards, int nType,int *pRet,int nMainColor,int nLevelCard,BOOL b2ConstMain)
{
	int nRet=0;
	int i=0;
	int buf[25];
	while(i<nCards-1)
	{
		if(pCard[i]==pCard[i+1])
		{//找到第一个对子
			int card=pCard[i];
			buf[0]=i,buf[1]=i+1;
			i+=2;
			BOOL bFind=TRUE;
			for(int j=1;j<nType/2;j++)
			{//查找符合要求的后续对子
				while(TRUE)
				{
					if(!ClientIsAdjcent(card,pCard[i],nMainColor,nLevelCard,b2ConstMain))
					{
						bFind=FALSE;
						i++;
						break;
					}
					i++;
					if(i>=nCards)
					{
						bFind=FALSE;
						break;
					}
					if( pCard[i]==pCard[i-1])
					{//找到一对与上一对相邻的牌
						card=pCard[i];
						buf[j*2]=i-1,buf[j*2+1]=i;
						//找到下一张牌值比较时不相同的牌
						while(i<nCards-1)
						{
							if(ClientCompare(card,pCard[i+1],nLevelCard,nMainColor,b2ConstMain)==0)
								i++;
							else
								break;
						}
						break;
					}
					i++;	
				}
				if(!bFind) break;
				if(i>=nCards) break;
			}
			if(bFind)
			{
				memcpy(pRet+nRet,buf,nType*sizeof(int));
				nRet+=nType;
			}
		}else
		{
			i++;
		}
	}
	return nRet;
}
