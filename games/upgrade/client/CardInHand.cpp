// CCardInHand.cpp: implementation of the CCardInHand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CardInHand.h"
#include "upgcard.h"
#include <helper/slog.h>
#include <valueAnimator/SPropertyAnimator.h>
#include "UpgradeGame.h"
#include <UpgAlgorithm.h>
#define kLogTag "CardInHand"


int CCardInHand::InsertCard(SSpriteCard *pCard)
{
    int nRet = 0;
    int nCard = pCard->GetCard();
    if (nCard == -1)
    {
        m_lstCard.AddTail(pCard);
        nRet = m_lstCard.GetCount();
    }else{
        BOOL bInserted = FALSE;
        for (SPOSITION it = m_lstCard.GetHeadPosition(); it;)
        {
            SPOSITION pos = it;
            auto pCard2 = m_lstCard.GetNext(it);
            int nValue = pCard2->GetCard();
            if (CUpgCard::ClientCompare(nValue,nCard,m_pGame->GetLevel(),m_mainColor, m_pGame->GetProps()[PI_RULE_2_MAIN])>0)
            {
                m_lstCard.InsertBefore(pos, pCard);
                bInserted = TRUE;
                break;
            }
            nRet++;
        }
        if (!bInserted)
        {
            
            m_lstCard.AddTail(pCard);
            nRet = m_lstCard.GetCount() - 1;
        }
    }
    int iLayer = kCardLayer_Base;
    for (SPOSITION it = m_lstCard.GetHeadPosition(); it;)
    {
        auto pCard2 = m_lstCard.GetNext(it);
        pCard2->SetLayer(iLayer++); // change layer
    }
    return nRet;
}

void CCardInHand::ReceiveCard(SSpriteCard *pCard)
{
    InsertCard(pCard);
    // auto adjust current cards position for the new inserted card.
    AdjustCardsPosition();
}

void CCardInHand::ReceiveCards(SList<SAutoRefPtr<SSpriteCard> >  & lstCards)
{
    for (SPOSITION it = lstCards.GetHeadPosition(); it;)
    {
        SSpriteCard *pCard = lstCards.GetNext(it);
        InsertCard(pCard);
    }
    // auto adjust current cards position for the new inserted card.
    AdjustCardsPosition();
}

/**
 * @brief 选中指定区域内的牌
 * @param rc 选中区域
 * @remark 由于右边的牌会盖住左边的牌，计算时需要从右向左遍历
*/
void CCardInHand::SelectCardForRect(const CRect &rc)
{
    CRect rcTest = rc;
    if (rc.bottom == rc.top)
        rcTest.bottom = rcTest.top + 1; //make sure rcTest is not empty
    for (SPOSITION it = m_lstCard.GetTailPosition(); it;)
    {
        SSpriteCard *pCard = m_lstCard.GetPrev(it);
        CRect rcCard = pCard->GetClientRect();
        CRect rcIntersect = rcCard & rcTest;
        if (!rcIntersect.IsRectEmpty())
        {
            pCard->SetSelected(TRUE);
            rcTest.right = rcCard.left;
            if(rcTest.right <= rcTest.left)
                break;
        }
    }
}

void CCardInHand::ClearSelect()
{
    for (SPOSITION it = m_lstCard.GetHeadPosition(); it;)
    {
        SSpriteCard *pCard = m_lstCard.GetNext(it);
        pCard->SetSelected(FALSE);
    }
}

void CCardInHand::GetSelectedCards(SList<SAutoRefPtr<SSpriteCard> >& lstCards)
{
    for (SPOSITION it = m_lstCard.GetHeadPosition(); it;)
    {
        SSpriteCard *pCard = m_lstCard.GetNext(it);
        if (pCard->IsSelected())
        {
            lstCards.AddTail(pCard);
        }
    }
}

BOOL CCardInHand::PutCard(SList<SAutoRefPtr<SSpriteCard> > &lstSelected)
{
    BOOL bRet = FALSE;
    for(SPOSITION it = m_lstCard.GetHeadPosition(); it;){
        SPOSITION pos = it;
        SSpriteCard *pCard = m_lstCard.GetNext(it);
        if (pCard->IsSelected())
        {
            lstSelected.AddTail(pCard);
            m_lstCard.RemoveAt(pos);
        }
    }
    if(lstSelected.IsEmpty())
        return FALSE;
    AdjustCardsPosition();
    return TRUE;
}

BOOL CCardInHand::PutCard(int * pCards, int nCount, SList<SAutoRefPtr<SSpriteCard> > &lstCard)
{
    if(nCount>m_lstCard.GetCount())
        return FALSE;
    for (size_t i = 0; i < nCount; i++)
    {
        SSpriteCard *pCard = m_lstCard.RemoveHead();
        pCard->SetCard(pCards[i]);
        lstCard.AddTail(pCard);
    }
    return TRUE;
}

BOOL CCardInHand::PutCard2(int * pCards, int nCount, SList<SAutoRefPtr<SSpriteCard> > &lstCard)
{
    if(nCount>m_lstCard.GetCount())
        return FALSE;
    for (int i = 0; i < nCount; i++)
    {
        BOOL bFound = FALSE;
        for(SPOSITION it = m_lstCard.GetHeadPosition(); it;){
            SPOSITION pos = it;
            SSpriteCard *pCard = m_lstCard.GetNext(it);
            if(pCard->GetCard()==pCards[i]){
                m_lstCard.RemoveAt(pos);
                lstCard.AddTail(pCard);
                bFound = TRUE;
                break;
            }
        }
        if(!bFound)
            return FALSE;
    }
    AdjustCardsPosition();
    return TRUE;
}

void CCardInHand::AdjustCardsPosition()
{
    int allWid = m_szCard.cx + m_lstCard.GetCount() * kCardInterval_Horz;
    int allHei = m_szCard.cy + m_lstCard.GetCount() * kCardInterval_Vert;
    int idx = 0;
    for (SPOSITION it = m_lstCard.GetHeadPosition(); it; idx++)
    {
        IWindow *pCard2 = m_lstCard.GetNext(it);
        AnchorPos pos = m_posBase;
        if (m_bHorz)
        {
            pos.x.fSize = idx * kCardInterval_Horz - allWid / 2 + m_szCard.cx / 2;
        }
        else
        {
            pos.y.fSize += idx * kCardInterval_Vert - allHei / 2 + m_szCard.cy / 2;
        }
        Util::MoveSpriteTo(pCard2, pos, SPEED_POSTCARD);
    }
}

void CCardInHand::Clear()
{
    for(SPOSITION it = m_lstCard.GetHeadPosition(); it;){
        m_lstCard.GetNext(it)->Destroy();
    }
    m_lstCard.RemoveAll();
    m_mainColor = -1;
}

static void ShuffleCard(SPOSITION *nCards,int nCardCount)
{
    if(nCardCount<=1)
        return;
	for(int i=0;i<nCardCount-1;i++)
	{
		//从后续的牌中随机选择一张牌
		int nExchange=rand()%(nCardCount-i-1)+i+1;
		//交换
		SPOSITION t=nCards[i];
		nCards[i]=nCards[nExchange];
		nCards[nExchange]=t;
	}
}

/**
 * @brief 随机选择牌
 * @param nCards 选择的牌数
 * @param bClear 是否清空之前的选中
 * @param nColor 选择的花色,nColor=-1表示任意花色
 * @param b2ConstMain 是否常主
 * @remark 牌堆中已经选中的牌不能被再次选中，优先从指定的花色中选择，不存在指定花色的牌时，从任意花色中选择
 */
void CCardInHand::SelectCardRandom(int nCards, BOOL bClear,int nColor, BOOL b2ConstMain)
{
    if(bClear) 
        ClearSelect();
    SPOSITION cardPos[33],colorPos[33];
    int i = 0, j=0;
    for(SPOSITION it = m_lstCard.GetHeadPosition(); it;){
        SPOSITION pos = it;
        auto card = m_lstCard.GetNext(it);
        if(card->IsSelected())
            continue;
        int color = CUpgCard::ClientGetColor2(card->GetCard(),m_mainColor,m_pGame->GetLevel(),b2ConstMain);
        if(nColor == color)
            colorPos[j++] = pos;
        else
            cardPos[i++] = pos;
    }
    SASSERT(i+j>=nCards);
    ShuffleCard(colorPos,j);
    for(int k=0;k<j && k<nCards;k++){
        SSpriteCard *pCard = m_lstCard.GetAt(colorPos[k]);
        pCard->SetSelected(TRUE);
    }
    nCards -= j;
    if(nCards>0){
        ShuffleCard(cardPos,i);
        for(int k=0;k<i && k<nCards;k++){
            SSpriteCard *pCard = m_lstCard.GetAt(cardPos[k]);
            pCard->SetSelected(TRUE);
        }
    }
}

void CCardInHand::SetSelectable(BOOL bSelectable)
{
    for (SPOSITION it = m_lstCard.GetHeadPosition(); it;)
    {
        SSpriteCard *pCard = m_lstCard.GetNext(it);
        pCard->SetSelectable(bSelectable);
    }
}

SIZE CCardInHand::GetCardSize() const
{
    return m_szCard;
}


int CCardInHand::TestReport(int nColor)
{
	SPOSITION pos=m_lstCard.GetHeadPosition();
	int nRet=0;
	int nDestCard=CUpgCard::ClientGetCard(nColor,m_pGame->GetLevel());
	while(pos)
	{
        SSpriteCard *card = m_lstCard.GetNext(pos);
		if(nDestCard==card->GetCard()) 
            nRet++;
	}
    int nReportColor = CUpgCard::ClientGetColor(m_pGame->m_curShowMain.nCard[0]);
	if(nReportColor!=-1)
	{
		if(m_pGame->m_curShowMain.index==m_pGame->m_iSelfIndex)
		{//自己报的牌只能再报当前色
			if(nColor!=nReportColor)
				return 0;
			else if(nRet==1)//如果手上还有一张相同花色则可以定主
				return 2;
		}
		if(nRet!=2)
			return 0;
		if(nRet==m_pGame->m_curShowMain.nCount)
		{
			int nCurColor=nReportColor;
			if(nCurColor<COLOR_SPADE)
				nCurColor=COLOR_SPADE;
			if(nColor>nCurColor)
				return nRet;
			else
				return 0;
		}
	}else
	{
		if((nRet==1) && (nColor>COLOR_SPADE)) //大小王必须是对子
			return 0;
	}

	return nRet;
}

BOOL CCardInHand::ReportColor(int nCard,BOOL bCouple, SList<SAutoRefPtr<SSpriteCard> > &pCards)
{
	SPOSITION pos=m_lstCard.GetHeadPosition();
	while(pos)
	{
		auto card=m_lstCard.GetAt(pos);
		if(card->GetCard()==nCard)
		{
			if(bCouple)
			{
				SPOSITION pos1=pos;
				m_lstCard.GetNext(pos);
				auto card2=m_lstCard.GetAt(pos);
				if(card2->GetCard()==nCard)
				{
                    pCards.AddTail((SSpriteCard*)card);
                    pCards.AddTail((SSpriteCard*)card2);
                    m_lstCard.RemoveAt(pos);
                    m_lstCard.RemoveAt(pos1);
                    AdjustCardsPosition();
					return TRUE;
				}else
				{
					return FALSE;
				}
			}else
			{
                pCards.AddTail((SSpriteCard*)card);
                m_lstCard.RemoveAt(pos);
                AdjustCardsPosition();
				return TRUE;
			}
		}
		m_lstCard.GetNext(pos);
	}
	return FALSE;
}

struct AniData{
    int nAniSpeed;
    SList<SPOSITION> lstToUp;   //副主
    SList<SPOSITION> lstToLeft;

    SList<SPOSITION> lstToUp2; //正2
    SList<SPOSITION> lstToLeft2;
    
    SList<SPOSITION> lstToUp3; //正级牌
    SList<SPOSITION> lstToLeft3;
};

void CCardInHand::MoveCards(SList<SPOSITION> &lstToUp, SList<SPOSITION> &lstToLeft, int nSpeed){
    //move toup list right and down
    int nLeftCount = lstToLeft.GetCount();
    for (SPOSITION it = lstToUp.GetHeadPosition(); it;)
    {
        SPOSITION pos = lstToUp.GetNext(it);
        auto pCard = m_lstCard.GetAt(pos);
        Util::EndAnimator(pCard);
        ILayoutParam *pParam = (ILayoutParam*)pCard->GetLayoutParam();
        SASSERT(pParam->IsClass(SAnchorLayoutParam::GetClassName()));
        SAnchorLayoutParamStruct * pParamStruct = (SAnchorLayoutParamStruct*)pParam->GetRawData();
        AnchorPos fromPos = pParamStruct->pos;
        AnchorPos toPos1 = fromPos;
        toPos1.x.fSize += kCardInterval_Horz*nLeftCount;
        AnchorPos toPos2 = toPos1;
        toPos2.y.fSize += m_szCard.cy;
        AnchorPos poss[] = {
            fromPos,
            toPos1,
            toPos2
	    };
        SAutoRefPtr<IValueAnimator> pAnimator(SPropertyAnimator::ofPosition(pCard, LayoutProperty::POSITION, poss, ARRAYSIZE(poss), sizeof(AnchorPos)), FALSE);
        pAnimator->setDuration(nSpeed*2);
        Util::StartAnimator(pAnimator,pCard);
    }
    //move toleft list left
    int nUpCount = lstToUp.GetCount();
    for (SPOSITION it = lstToLeft.GetHeadPosition(); it;)
    {
        SPOSITION pos = lstToLeft.GetNext(it);
        auto pCard = m_lstCard.GetAt(pos);
        Util::EndAnimator(pCard);
        Util::OffsetSprite(pCard, -nUpCount*kCardInterval_Horz, 0, nSpeed);
    }
}

void CCardInHand::onAnimationEnd(IValueAnimator * pAnimator)
{
    AniData *pData = (AniData*)pAnimator->GetUserData();
    if(pData->lstToUp.GetCount() && pData->lstToLeft.GetCount())
        MoveCards(pData->lstToUp, pData->lstToLeft, pData->nAniSpeed);
    if(pData->lstToUp2.GetCount() && pData->lstToLeft2.GetCount())
        MoveCards(pData->lstToUp2, pData->lstToLeft2, pData->nAniSpeed);
    if(pData->lstToUp3.GetCount() && pData->lstToLeft3.GetCount())
        MoveCards(pData->lstToUp3, pData->lstToLeft3, pData->nAniSpeed);
    //rearange spritecard list order, layer of card will be updated in InsertCard
    SList<SAutoRefPtr<SSpriteCard> > lstNew;
    lstNew.Swap(m_lstCard);
    for (SPOSITION it = lstNew.GetHeadPosition(); it;)
    {
        auto pCard = lstNew.GetNext(it);
        InsertCard(pCard);
    }
    delete pData;
}

void CCardInHand::SetMainColor(int mainColor,int nAniSpeed)
{
    SetSelectable(TRUE);
    m_mainColor = mainColor;
    if(m_mainColor>=0&&m_mainColor<COLOR_SPADE){
        //调整花色顺序，先将主花色抬起，再移动其它牌。
        SAutoRefPtr<IValueAnimator> animatorFirst;
        SList<SPOSITION> lstToUp;
        SList<SPOSITION> lstToLeft;
        {//调整副主的位置
            for (SPOSITION it = m_lstCard.GetHeadPosition(); it;)
            {
                SPOSITION pos = it;
                auto pCard = m_lstCard.GetNext(it);
                int nCard = pCard->GetCard();
                if(CUpgCard::ClientGetColor2(nCard,-1, m_pGame->GetLevel(), m_pGame->m_dwProps[PI_RULE_2_MAIN])>COLOR_SPADE)//主牌
                    break;
                int nColor = CUpgCard::ClientGetColor(nCard);
                if(nColor==m_mainColor){
                    lstToUp.AddTail(pos);
                }else if(nColor>m_mainColor && nColor<COLOR_SKING){
                    lstToLeft.AddTail(pos);
                }
            }
        }
        SList<SPOSITION> lstToUp2;
        SList<SPOSITION> lstToLeft2;
        if(m_pGame->m_dwProps[PI_RULE_2_MAIN]){
            //调整正2的位置
            for (SPOSITION it = m_lstCard.GetHeadPosition(); it;)
            {
                SPOSITION pos = it;
                auto pCard = m_lstCard.GetNext(it);
                int nCard = pCard->GetCard();
                if(CUpgCard::ClientGetValue(nCard)==1){
                    if(CUpgCard::ClientGetColor(nCard)==m_mainColor){
                        lstToUp2.AddTail(pos);
                    }else if(CUpgCard::ClientGetColor(nCard)>m_mainColor){
                        lstToLeft2.AddTail(pos);
                    }
                }
            }
        }
        SList<SPOSITION> lstToUp3;
        SList<SPOSITION> lstToLeft3;
        if(m_pGame->GetLevel() != 1 || m_pGame->m_dwProps[PI_RULE_2_MAIN] == 0){//调整级牌的位置将正级牌抬起
            for (SPOSITION it = m_lstCard.GetHeadPosition(); it;)
            {
                SPOSITION pos = it;
                auto pCard = m_lstCard.GetNext(it);
                int nCard = pCard->GetCard();
                if(CUpgCard::ClientGetValue(nCard)==m_pGame->GetLevel()){
                    if(CUpgCard::ClientGetColor(nCard)==m_mainColor){
                        lstToUp3.AddTail(pos);
                    }else if(CUpgCard::ClientGetColor(nCard)>m_mainColor){
                        lstToLeft3.AddTail(pos);
                    }
                }
            }
        }
        int nCount = lstToLeft.GetCount() + lstToUp.GetCount() + lstToLeft2.GetCount() + lstToUp2.GetCount() + lstToLeft3.GetCount() + lstToUp3.GetCount();
        if (nCount > 0)
        {
            if(lstToUp.GetCount() && lstToLeft.GetCount()){
                for (SPOSITION it = lstToUp.GetHeadPosition(); it;)
                {
                    SPOSITION pos = lstToUp.GetNext(it);
                    auto pCard = m_lstCard.GetAt(pos);
                    auto animator = Util::OffsetSprite(pCard, 0, -m_szCard.cy, SPEED_POSTCARD);
                    if (!animatorFirst)
                    {
                        animatorFirst = animator;
                        animator->addListener(this);
                    }
                }
            }
            if(lstToUp2.GetCount() && lstToLeft2.GetCount()){
                for(SPOSITION it = lstToUp2.GetHeadPosition(); it;)
                {
                    SPOSITION pos = lstToUp2.GetNext(it);
                    auto pCard = m_lstCard.GetAt(pos);
                    auto animator = Util::OffsetSprite(pCard, 0, -m_szCard.cy, SPEED_POSTCARD);
                    if (!animatorFirst)
                    {
                        animatorFirst = animator;
                        animator->addListener(this);
                    }
                }
            }
            if(lstToUp3.GetCount() && lstToLeft3.GetCount()){
                for(SPOSITION it = lstToUp3.GetHeadPosition(); it;)
                {
                    SPOSITION pos = lstToUp3.GetNext(it);
                    auto pCard = m_lstCard.GetAt(pos);
                    auto animator = Util::OffsetSprite(pCard, 0, -m_szCard.cy, nAniSpeed);
                    if (!animatorFirst)
                    {
                        animatorFirst = animator;
                        animator->addListener(this);
                    }
                }
            }
            if(animatorFirst){
                AniData *pData = new AniData;
                pData->nAniSpeed = nAniSpeed;
                pData->lstToLeft.Swap(lstToLeft);
                pData->lstToUp.Swap(lstToUp);
                pData->lstToLeft2.Swap(lstToLeft2);
                pData->lstToUp2.Swap(lstToUp2);
                pData->lstToLeft3.Swap(lstToLeft3);
                pData->lstToUp3.Swap(lstToUp3);
                animatorFirst->SetUserData(pData);
            }
        }
    }
}
SStringT CCardInHand::PrintCardID()
{
    SStringT str;
    for (SPOSITION it = m_lstCard.GetHeadPosition(); it;)
    {
        SSpriteCard *pCard = m_lstCard.GetNext(it);
        str += SStringT().Format(_T("%d,"), pCard->GetID());
    }
    return str;
}

int CCardInHand::AutoSelectCard(const int * pnCardFirst, int nFirstCards)
{
    int nFirstColor = CUpgCard::ClientGetColor2(pnCardFirst[0],m_mainColor,m_pGame->GetLevel(), m_pGame->m_dwProps[PI_RULE_2_MAIN]);
	//取得自己手中指定花色的所有牌
	SPOSITION pColorCardPos[33];
    int nColorCards = GetColorCard(nFirstColor,pColorCardPos);
	int nRet=0;
	if(nColorCards<=nFirstCards)
	{
		for(int i=0;i<nColorCards;i++)
        {
            auto pCard = m_lstCard.GetAt(pColorCardPos[i]);
            pCard->SetSelected(TRUE);
        }
		nRet=nColorCards;
	}else
	{
        int pColorCard[33];
        for(int i=0;i<nColorCards;i++)
        {
            auto pCard = m_lstCard.GetAt(pColorCardPos[i]);
            pColorCard[i] = pCard->GetCard();
        }
		int nTestType=2;
		while(TRUE)//循环检测所有牌型
		{
			//检测首家出牌的指定牌型数
			int nType1=CUpgCard::ClientGetCardType(pnCardFirst,nFirstCards,nTestType,m_mainColor,m_pGame->GetLevel(),m_pGame->m_dwProps[PI_RULE_2_MAIN] );
			if(nType1==0) break;//首家没有出该牌型，检测完成
			//检测手中的没有首家出的牌型
			int nType3=CUpgCard::ClientGetCardType(pColorCard,nColorCards,nTestType,m_mainColor,m_pGame->GetLevel(),m_pGame->m_dwProps[PI_RULE_2_MAIN]);
			if(nType3==0) break;//玩家手中没有指定类型的牌
			if(nType3<=nType1)
			{
				int nIndex[33]={0};
				nRet=CUpgCard::ClientGetTypeCard(pColorCard,nColorCards,nTestType,nIndex,m_mainColor,m_pGame->GetLevel(),m_pGame->m_dwProps[PI_RULE_2_MAIN]);
				for(int i=0;i<nRet;i++)
				{
                    auto pCard = m_lstCard.GetAt(pColorCardPos[nIndex[i]]);
                    pCard->SetSelected(TRUE);
				}
				break;
			}
			nTestType+=2;
		}		
	}
	return nRet;
}


//计算手中一门牌，主牌为一门牌
int CCardInHand::GetColorCard(int nFirstColor,SPOSITION pColorCard[33])
{
    int i = 0;
    for(SPOSITION it = m_lstCard.GetHeadPosition(); it;){
        SPOSITION pos = it;
        auto pCard = m_lstCard.GetNext(it);
        if(CUpgCard::ClientGetColor2(pCard->GetCard(),m_mainColor,m_pGame->GetLevel(),m_pGame->m_dwProps[PI_RULE_2_MAIN])==nFirstColor)
            pColorCard[i++] = pos;
    }
    return i;
}


//选择的牌是否为指定的一门牌
BOOL CCardInHand::IsSelectedCardSameColor(int nColor,int *pCard,int nCards) const
{
	for(int i=0;i<nCards;i++)
	{
		int tColor=CUpgCard::ClientGetColor2(pCard[i],m_mainColor,m_pGame->GetLevel(),m_pGame->m_dwProps[PI_RULE_2_MAIN]);
		if(nColor!=tColor) return FALSE;
	}
	return TRUE;
}

//指定的一门牌是否全被选择
BOOL CCardInHand::IsColorCardAllSelected(int nColor) const
{
	SPOSITION pos=m_lstCard.GetHeadPosition();
	while(pos)
	{
		auto card=m_lstCard.GetNext(pos);
		int tColor=CUpgCard::ClientGetColor2(card->GetCard(),m_mainColor,m_pGame->GetLevel(),m_pGame->m_dwProps[PI_RULE_2_MAIN]);
		if(tColor==nColor)
		{
            if(!card->IsSelected()) return FALSE;
		}
	}
	return TRUE;
}

void CCardInHand::SelectAllCard()
{
    for (SPOSITION it = m_lstCard.GetHeadPosition(); it;)
    {
        SSpriteCard *pCard = m_lstCard.GetNext(it);
        pCard->SetSelected(TRUE);
    }
}
//----------------------------------------------------------------------------
// CPutCard
//----------------------------------------------------------------------------
CPutCard::CPutCard(UpgradeGame *pGame):m_bShowPrevRound(FALSE),m_pGame(pGame)
{
    ISkinObj *pCard = GETSKIN(L"skin_card", 100);
    m_szCard = pCard->GetSkinSize();
}

CPutCard::~CPutCard()
{
}

void CPutCard::SetScoreCardPos(AnchorPos posScore)
{
    m_posScore = posScore;
}

void CPutCard::SetBase(AnchorPos posBottom, AnchorPos posLeft, AnchorPos posTop, AnchorPos posRight)
{
    m_posPutBase[0]=posBottom;
    m_posPutBase[1]=posLeft;
    m_posPutBase[2]=posTop;
    m_posPutBase[3]=posRight;
}

void CPutCard::SetAvatarPos(AnchorPos posBottom, AnchorPos posLeft, AnchorPos posTop, AnchorPos posRight)
{
    m_posAvatar[0]=posBottom;
    m_posAvatar[1]=posLeft;
    m_posAvatar[2]=posTop;
    m_posAvatar[3]=posRight;
}

void CPutCard::SetPutCard(int iSeat,SList<SAutoRefPtr<SSpriteCard> > &lstCard)
{
    m_lstCard[iSeat].AddTailList(&lstCard);
    SIZE szCard = m_szCard;
    int allWid = szCard.cx + m_lstCard[iSeat].GetCount() * CCardInHand::kCardInterval_Horz;
    int idx = 0;
    for (SPOSITION it = m_lstCard[iSeat].GetHeadPosition(); it;idx++)
    {
        SSpriteCard *pCard = m_lstCard[iSeat].GetNext(it);
        pCard->SetSelectable(FALSE);
        AnchorPos pos = m_posPutBase[iSeat];
        pCard->SetLayer(100 + idx);
        if(iSeat == 1){
            pos.x.fSize += idx * CCardInHand::kCardInterval_Horz;
        }
        else if (iSeat == 3)
        { // right player, reverse horz position
            pos.x.fSize -= allWid - idx * CCardInHand::kCardInterval_Horz;
        }
        else
        {
            pos.x.fSize += idx * CCardInHand::kCardInterval_Horz - allWid / 2 + szCard.cx / 2;
        }
        Util::MoveSpriteTo(pCard, pos, SPEED_POSTCARD);
    }
}


int CPutCard::GetCard(int nSeat, SList<SAutoRefPtr<SSpriteCard> > & lstCard)
{
    lstCard.Swap(m_lstCard[nSeat]);
    return lstCard.GetCount();
}


void CPutCard::Clear()
{
    for (int i = 0; i < PLAYER_COUNT; i++)
    {
        for (SPOSITION it = m_lstCard[i].GetHeadPosition(); it;)
        {
            SSpriteCard *pCard = m_lstCard[i].GetNext(it);
            pCard->Destroy();
        }
        m_lstCard[i].RemoveAll();
    }
    m_bShowPrevRound = FALSE;
}

int CPutCard::TakeinCard(int nWinner)
{
    int ret = 0;
    int iBankerSeat = m_pGame->Index2Seat(m_pGame->GetBankerIndex());
    for (int i = 0; i < PLAYER_COUNT; i++)
    {
        //clear prev card
        for(SPOSITION it = m_lstPrevCard[i].GetHeadPosition(); it;)
        {
            SSpriteCard *pCard = m_lstPrevCard[i].GetNext(it);
            pCard->Destroy();
        }
        m_lstPrevCard[i].RemoveAll();
        m_bShowPrevRound = FALSE;

        for (SPOSITION pos = m_lstCard[i].GetHeadPosition(); pos;)
        {
            SPOSITION pos2 = pos;
            SSpriteCard *pCard = m_lstCard[i].GetNext(pos);
            int nCard = pCard->GetCard();
            
            //clone a new card
            SSpriteCard *pCardNew = new SSpriteCard;
            pCardNew->SetLayer(pCard->GetLayer()+100);
            pCardNew->SetVisible(FALSE,FALSE);
            pCardNew->SetSkin(pCard->GetSkin());
            pCardNew->SetCard(nCard);
            pCard->GetParent()->InsertChild(pCardNew);
            SAutoRefPtr<ILayoutParam> pParam(pCard->GetLayoutParam()->Clone(),FALSE);
            pCardNew->SetLayoutParam(pParam);
            pCardNew->SetAlpha(200);//透明度
            m_lstPrevCard[i].AddTail(pCardNew);

            nCard = nCard % 13;
            //5,10,K
            if ((nCard == 4 || nCard == 9 || nCard == 12) && (nWinner%2!=iBankerSeat%2))
            {
                //move score card to score board.
                m_lstScore.AddTail(pCard);
                pCard->SetLayer(m_lstScore.GetCount()+100);
                ret += nCard == 4? 5:10;
                AnchorPos pos = m_posScore;
                pos.x.fSize += m_lstScore.GetCount() * CCardInHand::kCardInterval_Horz;
                Util::MoveSpriteTo(pCard, pos, SPEED_POSTCARD);
            }else{
                //move to the winner avatar
                SAutoRefPtr<IValueAnimator> pAnim = Util::MoveAndHideSprite(pCard, m_posAvatar[nWinner], SPEED_POSTCARD);
                pAnim->addListener(this);
            }
            m_lstCard[i].RemoveAt(pos2);
        }
    }
    return ret;
}

void CPutCard::onAnimationEnd(IValueAnimator * pAnimator)
{
    IPropertyAnimator * pPropAnim = sobj_cast<IPropertyAnimator>(pAnimator);
    IWindow *pTarget = pPropAnim->GetTarget();
    pTarget->Destroy();
}

void CPutCard::ShowPrevRound(BOOL bShow)
{
    if(m_bShowPrevRound==bShow)
        return;
    m_bShowPrevRound=bShow;
    for (int i = 0; i < PLAYER_COUNT; i++)
    {
        for (SPOSITION it = m_lstCard[i].GetHeadPosition(); it;)
        {
            SSpriteCard *pCard = m_lstCard[i].GetNext(it);
            pCard->SetVisible(!bShow,TRUE);
        }
        for (SPOSITION it = m_lstPrevCard[i].GetHeadPosition(); it;)
        {
            SSpriteCard *pCard = m_lstPrevCard[i].GetNext(it);
            pCard->SetVisible(bShow,TRUE);
        }
    }
}

void CPutCard::ClearScoreCards()
{
    for (SPOSITION it = m_lstScore.GetHeadPosition(); it;)
    {
        SSpriteCard *pCard = m_lstScore.GetNext(it);
        pCard->Destroy();
    }
    m_lstScore.RemoveAll();
}

int CPutCard::GetFirstCard(int nSeat)
{
    if (!m_lstCard[nSeat].IsEmpty())
    {
        return m_lstCard[nSeat].GetHead()->GetCard();
    }
    return -1;
}

int CPutCard::GetCards(int nSeat, int *cards)
{
    int idx = 0;
    for (SPOSITION it = m_lstCard[nSeat].GetHeadPosition(); it;)
    {
        SSpriteCard *pCard = m_lstCard[nSeat].GetNext(it);
        cards[idx++] = pCard->GetCard();
    }
    return idx;
}
