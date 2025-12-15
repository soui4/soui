// CardRecieve.cpp: implementation of the CCardRecieve class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CardRecieve.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCardRecieve::CCardRecieve()
    : m_nCount(0)
{

}

CCardRecieve::~CCardRecieve()
{

}

void CCardRecieve::Init(const int *pnCards, int nSelfIndex, int nFirstIndex)
{
	memcpy(m_nCards,pnCards,25*sizeof(int));
	m_nCount=25;
	m_nFirstIndex=nFirstIndex;
	m_nToSeat=nFirstIndex;
	m_nSelfIndex=nSelfIndex;
}

#define PLAYER_COUNT 4
BOOL CCardRecieve::FatchCard(int &nCard, int &iToSeat)
{
	BOOL bFinish=FALSE;
	if(m_nCount==0 && m_nToSeat==m_nFirstIndex)
		return FALSE;
	if(m_nToSeat==m_nSelfIndex)
	{
		nCard=m_nCards[--m_nCount];
	}else
	{
		nCard=-1;
	}
	iToSeat=m_nToSeat;
	m_nToSeat=(m_nToSeat+1)%PLAYER_COUNT;
	return TRUE;
}
