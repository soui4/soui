#include "UpgAlgorithm.h"
#include <algorithm>
#include <vector>
#include <map>
#include <cstring>
#include <assert.h>
void CUpgAlgorithm::InitCard(int nCards[108])
{ 
	//104张普通牌
	for(int i=0;i<52;i++)
	{
        nCards[2*i] = i;
        nCards[2 * i + 1] = i;
	}
	//4张大小王
	for(int i=52;i<54;i++)
	{
        nCards[104 + 2 * (i - 52)] = i;
        nCards[104 + 2 * (i - 52) + 1] = i;
	}
}

// 洗牌
void CUpgAlgorithm::ShuffleCard(int *pCard,int nCardCount)
{
	if(nCardCount<=1)
		return;
	for(int i=0;i<nCardCount-1;i++)
	{
		//从后续的牌中随机选择一张牌
		int nExchange=rand()%(nCardCount-i-1)+i+1;
		//交换
		int t=pCard[i];
		pCard[i]=pCard[nExchange];
		pCard[nExchange]=t;
	}
}

// 获取指定花色的牌
int CUpgAlgorithm::GetColorCards(const int* cards, int count, int color, int mainColor, int levelCard, bool is2ConstMain, int colorCards[MAX_CARDS])
{
    int ret = 0;
    for (int i = 0; i < count; i++) {
        int cardMainValue = GetMainValue(cards[i], mainColor, levelCard, is2ConstMain);
        int cardColor = GetCardColor(cards[i]);

        // 判断是否是指定花色
        bool isTargetColor = false;
        if (color == COLOR_MAIN) {
            // 主牌
            isTargetColor = (cardMainValue >= 0);
        } else {
            // 副牌
            isTargetColor = (cardColor == color && cardMainValue < 0);
        }

        if (isTargetColor) {
            colorCards[ret++] = cards[i];
        }
    }
    return ret;
}

int CUpgAlgorithm::BuildCard(int nColor,int nValue)
{
    if (nColor == COLOR_SKING) 
        return 52;
    if (nColor == COLOR_BKING) 
        return 53;
    return nColor * 13 + nValue;
}

// 获取牌的花色
int CUpgAlgorithm::GetCardColor(int card)
{
    if (card >= 52)
        return card - 52 + COLOR_SKING; // 王牌
    return card / 13; // 普通牌
}

// 获取牌的值
int CUpgAlgorithm::GetCardValue(int card)
{
    if (card >= 52)
        return card - 52; // 王牌值
    return card % 13; // 普通牌值
}

// 获取牌的主牌值
int CUpgAlgorithm::GetMainValue(int card, int mainColor, int levelCard, bool is2ConstMain)
{
    int color = GetCardColor(card);
    int value = GetCardValue(card);

    // 大小王永远是主牌
    if (color == COLOR_SKING || color == COLOR_BKING)
        return value;

    // 级牌（所有花色的级牌都是主牌）
    if (value == levelCard) {
        return value;
    }
    if (is2ConstMain && value == 1) { // 2为常主
        return value;
    }
    // 主花色牌
    if (color == mainColor)
        return value;

    // 副牌
    return -1;
}

// 判断两张牌是否相邻
bool CUpgAlgorithm::IsAdjacent(int card1, int card2, int mainColor, int levelCard, bool is2ConstMain)
{
    int val1 = GetCardCompareValue(card1, mainColor, levelCard, is2ConstMain);
    int val2 = GetCardCompareValue(card2, mainColor, levelCard, is2ConstMain);
    
    return (val1 + 1 == val2) || (val2 + 1 == val1);
}

enum{
    CMP_VALUE_BKING = 100,
    CMP_VALUE_SKING = 99,
    CMP_VALUE_MAIN = 50,
};
// 获取牌的比较值
int CUpgAlgorithm::GetCardCompareValue(int card, int mainColor, int levelCard, bool is2ConstMain)
{
    int color = GetCardColor(card);
    int value = GetCardValue(card);

    // 大王最大
    if (color == COLOR_BKING)
        return CMP_VALUE_BKING;

    // 小王次之
    if (color == COLOR_SKING)
        return CMP_VALUE_SKING;

    int mainValue = GetMainValue(card, mainColor, levelCard, is2ConstMain);
    int value2 = (value + 12) % 13;
    int levelCardValue2 = (levelCard + 12) % 13;
    //当级牌为A时，levelCard=0, 所有value都将>levelCard
    //当级牌为K时，levelCard=12，为了让QQAA能连续，需要将A的value2-1
    if(value2>levelCardValue2){
        //当牌值比级牌大时，-1, 以便和更小的牌在比较的值上连续, value2的范围为0-11
        value2--;
    }
    // 主牌
    if (mainValue >= 0) {
        // 级牌（所有花色的级牌）
        int upLevels = 11+2;
        if(is2ConstMain && levelCard != 1){
            upLevels = 11+4;
        }
        if (value == levelCard) {
            // 主花色的级牌最大（除了大小王）
            if (color == mainColor)
                 return CMP_VALUE_MAIN + upLevels;
            else
                return CMP_VALUE_MAIN + upLevels - 1;
        }else if(value == 1 && is2ConstMain){
            if (color == mainColor)
                return CMP_VALUE_MAIN + upLevels - 2;
            else
                return CMP_VALUE_MAIN + upLevels - 3;
        }
        return CMP_VALUE_MAIN + value2;
    }else{
        // 副牌
        return value2;
    }
}

// 判断牌型
int CUpgAlgorithm::GetCardType(const int* cards, int count, int mainColor, int levelCard, bool is2ConstMain)
{
    if (count <= 0)
        return CARD_TYPE_INVALID;
        
    if (count == 1)
        return CARD_TYPE_SINGLE;
    bool firstIsMain = (GetMainValue(cards[0], mainColor, levelCard, is2ConstMain) >= 0);
    int firstColor = GetCardColor(cards[0]);
    for(int i = 1; i < count; i++){
        if(firstIsMain)
        {
            if(GetMainValue(cards[i], mainColor, levelCard, is2ConstMain) < 0)
                return CARD_TYPE_INVALID;
        }else{
            if(firstColor != GetCardColor(cards[i]))
                return CARD_TYPE_INVALID;
        }
    }
    // 创建按原始值排序的副本（用于顺子和拖拉机的连续性检查）
    std::vector<int> sortedCards(cards, cards + count);
    std::sort(sortedCards.begin(), sortedCards.end(), [&](int a, int b) {
        // 按原始牌值排序，保留游戏中的自然顺序
        int valA = GetCardCompareValue(a, mainColor, levelCard, is2ConstMain);
        int valB = GetCardCompareValue(b, mainColor, levelCard, is2ConstMain);
        if (valA != valB) {
            return valA < valB;
        }
        // 同值时按花色排序（保持稳定）
        return GetCardColor(a) < GetCardColor(b);
    });

    std::vector<int> sortedValues;
    for (int i = 0; i < count; i++) {
        sortedValues.push_back(GetCardCompareValue(sortedCards[i], mainColor, levelCard, is2ConstMain));
    }
        
    // 检查是否为拖拉机（连续对子）
    if (count >= 4 && count % 2 == 0) {
        bool isTractor = true;
        for (int i = 0; i < (int)sortedValues.size(); i += 2) {
            // 每两个必须相等
            if (sortedValues[i] != sortedValues[i+1]) {
                isTractor = false;
                break;
            }
            // 与下一个对子必须连续
            if (i + 2 < (int)sortedValues.size()) {
                if (sortedValues[i+1] + 1 != sortedValues[i+2]) {
                    isTractor = false;
                    break;
                }
            }
        }

        if (isTractor)
            return CARD_TYPE_TRACTOR;
    }
    // 检查是否为对子或更多相同牌
    bool hasSingle = false;
    bool hasPair = false;
    for (int i = 1; i < count; i++)
    {
        if (sortedCards[i] == sortedCards[i - 1])
        {
            hasPair = true;
            i++;
        }
        else
        {
            hasSingle = true;
        }
    }
    if (hasSingle && hasPair)
        return CARD_TYPE_MIX;
    else if (hasPair)
        return CARD_TYPE_PAIR;
    else // if (hasSingle)
        return CARD_TYPE_SINGLE;
}

// 判断首家出牌是否符合规则
PutCardResult CUpgAlgorithm::ValidateFirstPlay(const int* cards, int count, int mainColor, int levelCard, bool is2ConstMain)
{
    if (count <= 0 || cards == nullptr)
        return PUT_CARD_RESULT_INVALID_TYPE;

    // 检查是否为单一有效牌型
    int cardType = GetCardType(cards, count, mainColor, levelCard, is2ConstMain);
    if (cardType != 0)
        return PUT_CARD_RESULT_SUCCESS; // 单一牌型有效

    // 如果不是单一牌型，检查是否为有效的甩牌
    // 甩牌规则：可以同时出多个对子或多个单张（即使不连续）

    // 创建牌的副本并排序
    std::vector<int> sortedCards(cards, cards + count);
    std::sort(sortedCards.begin(), sortedCards.end(), [mainColor, levelCard, is2ConstMain](int a, int b) {
        return GetCardCompareValue(a, mainColor, levelCard, is2ConstMain) <
               GetCardCompareValue(b, mainColor, levelCard, is2ConstMain);
    });

    // 检查是否所有牌都是同一花色
    int firstColor;
    int firstMainValue = GetMainValue(sortedCards[0], mainColor, levelCard, is2ConstMain);
    if (firstMainValue >= 0) {
        firstColor = COLOR_MAIN;
    } else {
        firstColor = GetCardColor(sortedCards[0]);
    }

    for (int i = 1; i < count; i++) {
        int currentMainValue = GetMainValue(sortedCards[i], mainColor, levelCard, is2ConstMain);
        int currentColor;
        if (currentMainValue >= 0) {
            currentColor = COLOR_MAIN;
        } else {
            currentColor = GetCardColor(sortedCards[i]);
        }

        if (currentColor != firstColor)
            return PUT_CARD_RESULT_INVALID_TYPE; // 甩牌必须同花色
    }

    // 检查是否为有效的甩牌组合
    // 情况1：全部是单张（每张牌都不同）
    bool allSingles = true;
    for (int i = 0; i < count - 1; i++) {
        if (sortedCards[i] == sortedCards[i + 1]) {
            allSingles = false;
            break;
        }
    }

    if (allSingles)
        return PUT_CARD_RESULT_SUCCESS; // 多个单张甩牌有效

    // 情况2：全部是对子（每个对子可以不连续）
    // 只有在牌数是偶数时才检查是否全部是对子
    if (count % 2 == 0) {
        bool allPairs = true;
        for (int i = 0; i < count; i += 2) {
            int value1 = GetCardValue(sortedCards[i]);
            int value2 = GetCardValue(sortedCards[i + 1]);
            bool isMain1 = (GetMainValue(sortedCards[i], mainColor, levelCard, is2ConstMain) >= 0);
            bool isMain2 = (GetMainValue(sortedCards[i + 1], mainColor, levelCard, is2ConstMain) >= 0);

            // 每两张必须是对子（值相同，且都是主牌或都是副牌）
            if (value1 != value2 || isMain1 != isMain2) {
                allPairs = false;
                break;
            }
        }

        if (allPairs)
            return PUT_CARD_RESULT_SUCCESS; // 多个对子甩牌有效
    }

    // 情况3：混合单张和对子的甩牌
    // 检查是否可以分解为若干对子和若干单张
    std::vector<bool> used(count, false);
    int pairCount = 0;
    int singleCount = 0;

    // 先找出所有对子
    for (int i = 0; i < count - 1; i++) {
        if (used[i]) continue;

        for (int j = i + 1; j < count; j++) {
            if (used[j]) continue;

            int value1 = GetCardValue(sortedCards[i]);
            int value2 = GetCardValue(sortedCards[j]);
            bool isMain1 = (GetMainValue(sortedCards[i], mainColor, levelCard, is2ConstMain) >= 0);
            bool isMain2 = (GetMainValue(sortedCards[j], mainColor, levelCard, is2ConstMain) >= 0);

            if (value1 == value2 && isMain1 == isMain2) {
                used[i] = true;
                used[j] = true;
                pairCount++;
                break;
            }
        }
    }

    // 剩余的都是单张
    for (int i = 0; i < count; i++) {
        if (!used[i])
            singleCount++;
    }

    // 如果可以分解为对子和单张的组合，则有效
    if (pairCount > 0 || singleCount > 0)
        return PUT_CARD_RESULT_SUCCESS;

    return PUT_CARD_RESULT_INVALID_TYPE;
}

// 辅助函数：获取指定花色的所有牌
static int GetSameColorCards(const int* cards, int count, int targetColor, int mainColor, int levelCard, bool is2ConstMain, int* colorCards)
{
    int colorCount = 0;
    for (int i = 0; i < count; i++) {
        int cardMainValue = CUpgAlgorithm::GetMainValue(cards[i], mainColor, levelCard, is2ConstMain);
        int cardColor = CUpgAlgorithm::GetCardColor(cards[i]);

        int currentColor;
        if (cardMainValue >= 0) {
            currentColor = COLOR_MAIN;
        } else {
            currentColor = cardColor;
        }

        if (currentColor == targetColor) {
            colorCards[colorCount++] = cards[i];
        }
    }
    return colorCount;
}

// 辅助函数：统计指定牌型的数量
static int CountCardType(const int* cards, int count, int typeLen, int mainColor, int levelCard, bool is2ConstMain)
{
    if (count < typeLen * 2) return 0;

    // 提取对子
    int pairs[26], pairCount = 0;
    int i = 0;
    while (i < count - 1) {
        if (cards[i] == cards[i + 1]) {
            pairs[pairCount++] = cards[i];
            i += 2;
        } else {
            i++;
        }
    }

    if (typeLen == 1) {
        // 对子数量
        return pairCount;
    }

    // 统计连续对子（拖拉机）的数量
    if (pairCount < typeLen) return 0;

    // 按比较值排序对子
    for (int i = 0; i < pairCount - 1; i++) {
        for (int j = 0; j < pairCount - i - 1; j++) {
            int val1 = CUpgAlgorithm::GetCardCompareValue(pairs[j], mainColor, levelCard, is2ConstMain);
            int val2 = CUpgAlgorithm::GetCardCompareValue(pairs[j + 1], mainColor, levelCard, is2ConstMain);
            if (val1 < val2) {
                int temp = pairs[j];
                pairs[j] = pairs[j + 1];
                pairs[j + 1] = temp;
            }
        }
    }

    // 统计连续对子
    int tractorCount = 0;
    i = 0;
    while (i <= pairCount - typeLen) {
        bool isContinuous = true;
        for (int j = 0; j < typeLen - 1; j++) {
            if (!CUpgAlgorithm::IsAdjacent(pairs[i + j], pairs[i + j + 1], mainColor, levelCard, is2ConstMain)) {
                isContinuous = false;
                break;
            }
        }

        if (isContinuous) {
            tractorCount++;
            i += typeLen;
        } else {
            i++;
        }
    }

    return tractorCount;
}

// 判断跟牌是否符合规则
PutCardResult CUpgAlgorithm::ValidateFollowPlay(const int* leadCards, int leadCount,
                                                const int* followCards, int followCount,
                                                const int* handCards, int handCount,
                                                int mainColor, int levelCard, bool is2ConstMain)
{
    // 检查牌数是否一致
    if (leadCount != followCount)
        return PUT_CARD_RESULT_WRONG_COUNT;

    // 获取首家出牌的花色
    int leadMainValue = GetMainValue(leadCards[0], mainColor, levelCard, is2ConstMain);
    int leadColor;
    if (leadMainValue >= 0) {
        leadColor = COLOR_MAIN;
    } else {
        leadColor = GetCardColor(leadCards[0]);
    }

    // 获取跟牌者手中同花色的牌（包括已出的牌）
    int sameColorCards[27];
    int sameColorCount = GetSameColorCards(handCards, handCount, leadColor, mainColor, levelCard, is2ConstMain, sameColorCards);

    // 检查跟牌花色
    int followSameColorCount = 0;
    for (int i = 0; i < followCount; i++) {
        int followMainValue = GetMainValue(followCards[i], mainColor, levelCard, is2ConstMain);
        int followCardColor;
        if (followMainValue >= 0) {
            followCardColor = COLOR_MAIN;
        } else {
            followCardColor = GetCardColor(followCards[i]);
        }

        if (followCardColor == leadColor) {
            followSameColorCount++;
        }
    }

    // 如果手中同花色牌数量大于等于出牌数量，必须全部出同花色
    if (sameColorCount >= followCount) {
        if (followSameColorCount != followCount) {
            return PUT_CARD_RESULT_WRONG_COLOR;
        }

        // 获取首家出牌的牌型
        int leadCardType = GetCardType(leadCards, leadCount, mainColor, levelCard, is2ConstMain);

        // 检查各种牌型是否满足要求
        // 从最大的牌型（拖拉机）开始检查
        if (leadCardType == CARD_TYPE_TRACTOR || leadCardType == CARD_TYPE_PAIR) {
            // 计算首家出牌中的拖拉机/对子长度
            int leadPairCount = 0;
            int i = 0;
            while (i < leadCount - 1) {
                if (leadCards[i] == leadCards[i + 1]) {
                    leadPairCount++;
                    i += 2;
                } else {
                    i++;
                }
            }

            // 计算拖拉机长度
            int tractorLen = leadPairCount;
            if (leadCardType == CARD_TYPE_TRACTOR) {
                // 已经是拖拉机，长度就是对子数量
            } else if (leadCardType == CARD_TYPE_PAIR) {
                tractorLen = 1;
            }

            // 检查从最大牌型到对子的所有牌型
            for (int typeLen = tractorLen; typeLen >= 1; typeLen--) {
                int leadTypeCount = CountCardType(leadCards, leadCount, typeLen, mainColor, levelCard, is2ConstMain);
                int handTypeCount = CountCardType(sameColorCards, sameColorCount, typeLen, mainColor, levelCard, is2ConstMain);
                int followTypeCount = CountCardType(followCards, followCount, typeLen, mainColor, levelCard, is2ConstMain);

                // 如果首家出了这种牌型，且手中有足够的这种牌型，但跟牌中没有足够的这种牌型
                if (leadTypeCount > 0 && handTypeCount > 0 && followTypeCount < handTypeCount && followTypeCount < leadTypeCount)
                {
                    return PUT_CARD_RESULT_CANNOT_FOLLOW;
                }
            }
        }
        else if (leadCardType == 0) {
            // 甩牌：leadCardType == 0 表示多个不连续的对子、多个单张或混合对子和单张
            // 需要验证跟牌的结构与首家相同
            
            // 统计首家甩牌的结构
            int leadPairCount = 0, leadSingleCount = 0;
            for (int i = 0; i < leadCount; ) {
                if (i + 1 < leadCount && leadCards[i] == leadCards[i + 1]) {
                    leadPairCount++;
                    i += 2;
                } else {
                    leadSingleCount++;
                    i++;
                }
            }
            
            // 统计跟牌的结构
            int followPairCount = 0, followSingleCount = 0;
            for (int i = 0; i < followCount; ) {
                if (i + 1 < followCount && followCards[i] == followCards[i + 1]) {
                    followPairCount++;
                    i += 2;
                } else {
                    followSingleCount++;
                    i++;
                }
            }
            
            // 验证跟牌结构与首家相同
            if (leadPairCount != followPairCount || leadSingleCount != followSingleCount) {
                return PUT_CARD_RESULT_CANNOT_FOLLOW;
            }
        }

        // 注意：不需要检查跟牌牌型是否有效
        // 因为在跟牌时，如果手中没有对应的牌型，玩家可以出任意组合（垫牌）
        // 前面已经检查了必须出的牌型（对子、拖拉机等）和甩牌结构
    } else {
        // 手中同花色牌数量不足，必须全部出同花色牌
        if (followSameColorCount != sameColorCount) {
            return PUT_CARD_RESULT_WRONG_COLOR;
        }

        // 手中同花色牌不足时，不需要检查牌型（可以垫其他花色的牌）
    }

    return PUT_CARD_RESULT_SUCCESS;
}

/**
 * 比较两手牌，返回1表示cards1大，返回-1表示cards2大，两手牌相同时返回0。这里不再比较牌型
 */
static int CompareCards(const int *cards1, const int *cards2, int count, int mainColor, int levelCard, bool is2ConstMain)
{
    // card1 and card2 should been sorted at first
    for (int i = 0; i < count; i++)
    {
        int compareValue1 = CUpgAlgorithm::GetCardCompareValue(cards1[i], mainColor, levelCard, is2ConstMain);
        int compareValue2 = CUpgAlgorithm::GetCardCompareValue(cards2[i], mainColor, levelCard, is2ConstMain);
        if (compareValue1 != compareValue2)
        {
            return compareValue1 - compareValue2;
        }
    }
    return 0;
}

// 比较一轮牌的大小，确定赢家
int CUpgAlgorithm::DetermineWinner(const int* cardsPlayed[4], const int cardsCount[4], int mainColor, int levelCard, bool is2ConstMain, int firstPlayer)
{
    if (firstPlayer < 0 || firstPlayer >= 4)
        return -1;
    int sorderedCards[4][25];   // 按比较值从大到小排序的牌
    for(int i = 0; i < 4; i++){
        memcpy(sorderedCards[i], cardsPlayed[i], cardsCount[i] * sizeof(int));
        std::sort(sorderedCards[i], sorderedCards[i] + cardsCount[i], [&](int a, int b){
            int compareValue1 = CUpgAlgorithm::GetCardCompareValue(a, mainColor, levelCard, is2ConstMain);
            int compareValue2 = CUpgAlgorithm::GetCardCompareValue(b, mainColor, levelCard, is2ConstMain);
            return compareValue1 > compareValue2;
        });
    }
    int count  = cardsCount[firstPlayer];
    // 获取首家出的牌型和花色
    int leadCardType = GetCardType(sorderedCards[firstPlayer], count, mainColor, levelCard, is2ConstMain);
    if(leadCardType == CARD_TYPE_MIX){
        //分离首家的对子和单张
        int pairs[26], pc = 0;
        int singles[26], sc = 0;
        int *pCard = sorderedCards[firstPlayer];
        for(int i = 0; i < count; i++){
            if(i < count - 1 && pCard[i] == pCard[i + 1]){
                pairs[pc] = pCard[i];
                pairs[pc+1] = pCard[i];
                pc +=2;
                i++;
            }else{
                singles[sc++] = sorderedCards[firstPlayer][i];
            }
        }

        int iWinner = firstPlayer;
        for(int i=0; i< 3;i++){
            int iNext = (i + firstPlayer + 1) % 4;
            int pairs2[26], pc2 = 0;
            int singles2[26], sc2 = 0;
            int *pCard2 = sorderedCards[iNext];
            for(int j = 0; j < cardsCount[iNext]; j++){
                if(pc2<pc && j < cardsCount[iNext] - 1 && pCard2[j] == pCard2[j + 1]){
                    pairs2[pc2] = pCard2[j];
                    pairs2[pc2+1] = pCard2[j];
                    pc2 +=2;
                    j++;
                }else{
                    singles2[sc2++] = pCard2[j];
                }
            }
            if(pc2<pc)
                continue;
            int pcmp = (pc==0)?0:CompareCards(pairs, pairs2, pc, mainColor, levelCard, is2ConstMain);
            if(pcmp == 0)
                pcmp = sc==0?0:CompareCards(singles, singles2, sc, mainColor, levelCard, is2ConstMain);
            if(pcmp < 0){
                iWinner = iNext;
                memcpy(pairs, pairs2, pc * sizeof(int));
                memcpy(singles, singles2, sc * sizeof(int));
            }
        }
        return iWinner;
    }
    int iWinner = firstPlayer;
    for (int i = 0; i < 3; i++)
    {
        int iNext = (i + firstPlayer + 1) % 4;
        int *pCardNext = sorderedCards[iNext];
        int nextCardType = GetCardType(pCardNext, count, mainColor, levelCard, is2ConstMain);
        if(nextCardType != leadCardType)
            continue;
        int pcmp=CompareCards(sorderedCards[iWinner], pCardNext, count, mainColor, levelCard, is2ConstMain);
        if (pcmp < 0)
        {
            iWinner = iNext;
        }
    }
    return iWinner;
}

// 根据首家出牌，从手牌中找出必须出的牌
int CUpgAlgorithm::GetMustPlayCards(const int* handCards, int handCount, const int* leadCards, int leadCount, int mainColor, int levelCard, bool is2ConstMain, int* mustPlayCards)
{
    if (handCount <= 0 || leadCount <= 0 || handCards == nullptr || leadCards == nullptr || mustPlayCards == nullptr)
        return 0;
        
    // 获取首家出牌的牌型和花色
    int leadCardType = GetCardType(leadCards, leadCount, mainColor, levelCard, is2ConstMain);
    int leadColor = GetCardColor(leadCards[0]);
    bool leadIsMain = (GetMainValue(leadCards[0], mainColor, levelCard, is2ConstMain) >= 0);
    
    // 如果首家出的是主牌花色，则必须出主牌
    if (leadIsMain) {
        int mainCardCount = 0;
        for (int i = 0; i < handCount; i++) {
            if (GetMainValue(handCards[i], mainColor, levelCard, is2ConstMain) >= 0) {
                mustPlayCards[mainCardCount++] = handCards[i];
            }
        }
        return mainCardCount;
    } else {
        // 如果首家出的是副牌花色，则优先出该花色的牌
        int sameColorCount = 0;
        for (int i = 0; i < handCount; i++) {
            // 检查是否为相同花色的副牌
            int handCardColor = GetCardColor(handCards[i]);
            bool handCardIsMain = (GetMainValue(handCards[i], mainColor, levelCard, is2ConstMain) >= 0);
            
            // 如果是相同花色的副牌
            if (handCardColor == leadColor && !handCardIsMain) {
                mustPlayCards[sameColorCount++] = handCards[i];
            }
        }
        
        // 如果有相同花色的副牌，则必须出这些牌
        if (sameColorCount > 0) {
            return sameColorCount;
        }
        
        // 如果没有相同花色的副牌，但有主牌，则必须出主牌
        int mainCardCount = 0;
        for (int i = 0; i < handCount; i++) {
            if (GetMainValue(handCards[i], mainColor, levelCard, is2ConstMain) >= 0) {
                mustPlayCards[mainCardCount++] = handCards[i];
            }
        }
        return mainCardCount;
    }
    
    // 默认情况下不需要必须出特定牌
    return 0;
}

// 辅助函数：获取指定花色的牌
static int GetColorCards(const int* cards, int count, int color, int mainColor, int levelCard, bool is2ConstMain, int* colorCards)
{
    int colorCount = 0;
    for (int i = 0; i < count; i++) {
        int cardMainValue = CUpgAlgorithm::GetMainValue(cards[i], mainColor, levelCard, is2ConstMain);
        int cardColor = CUpgAlgorithm::GetCardColor(cards[i]);

        // 判断是否是指定花色
        bool isTargetColor = false;
        if (color == COLOR_MAIN) {
            // 主牌
            isTargetColor = (cardMainValue >= 0);
        } else {
            // 副牌
            isTargetColor = (cardColor == color && cardMainValue < 0);
        }

        if (isTargetColor) {
            colorCards[colorCount++] = cards[i];
        }
    }
    return colorCount;
}

// 辅助函数：将牌分成对子和单张
static void SplitPairsAndSingles(const int* cards, int count, int* pairs, int* pairCount, int* singles, int* singleCount)
{
    *pairCount = 0;
    *singleCount = 0;

    int i = 0;
    while (i < count) {
        if (i < count - 1 && cards[i] == cards[i + 1]) {
            // 找到对子
            pairs[(*pairCount)++] = cards[i];
            i += 2;
        } else {
            // 单张
            singles[(*singleCount)++] = cards[i];
            i++;
        }
    }
}

// 辅助函数：提取最小的连续对子牌型
static int ExtractMinTractorType(const int* pairs, int pairCount, int mainColor, int levelCard, bool is2ConstMain, int* tractorCards)
{
    if (pairCount == 0) return 0;

    // 按比较值排序对子
    int sortedPairs[26];
    for (int i = 0; i < pairCount; i++) {
        sortedPairs[i] = pairs[i];
    }

    // 冒泡排序（从大到小）
    for (int i = 0; i < pairCount - 1; i++) {
        for (int j = 0; j < pairCount - i - 1; j++) {
            int val1 = CUpgAlgorithm::GetCardCompareValue(sortedPairs[j], mainColor, levelCard, is2ConstMain);
            int val2 = CUpgAlgorithm::GetCardCompareValue(sortedPairs[j + 1], mainColor, levelCard, is2ConstMain);
            if (val1 < val2) {
                int temp = sortedPairs[j];
                sortedPairs[j] = sortedPairs[j + 1];
                sortedPairs[j + 1] = temp;
            }
        }
    }

    // 找到最小的连续对子序列
    int minTractorLen = pairCount + 1;
    int minTractorIdx = -1;

    int i = 0;
    while (i < pairCount) {
        // 计算从i开始的连续对子长度
        int continueLen = 1;
        for (int j = i; j < pairCount - 1; j++) {
            if (CUpgAlgorithm::IsAdjacent(sortedPairs[j], sortedPairs[j + 1], mainColor, levelCard, is2ConstMain)) {
                continueLen++;
            } else {
                break;
            }
        }

        // 更新最小拖拉机
        if (continueLen <= minTractorLen) {
            minTractorLen = continueLen;
            minTractorIdx = i;
        }

        i += continueLen;
    }

    // 返回最小拖拉机的牌（从大到小排序）
    if (minTractorIdx >= 0) {
        for (int i = 0; i < minTractorLen; i++) {
            tractorCards[i] = sortedPairs[minTractorIdx + i];
        }
        return minTractorLen;
    }

    return 0;
}

void CUpgAlgorithm::SortCards(int* cards, int count, int mainColor, int levelCard, bool is2ConstMain){
    std::sort(cards, cards + count, [mainColor, levelCard, is2ConstMain](int a, int b) {
        return GetCardCompareValue(a, mainColor, levelCard, is2ConstMain) >
               GetCardCompareValue(b, mainColor, levelCard, is2ConstMain);
    });
}

int CUpgAlgorithm::CheckThrowFailed(const int* playCards_, int playCount,
                                    const int otherHands[3][25], const int otherCounts[3],
                                    int mainColor, int levelCard, bool is2ConstMain,
                                    int* failedCards){
    const int * otherHandsPtr[3] = {otherHands[0], otherHands[1], otherHands[2]};             
    return CheckThrowFailed(playCards_, playCount, otherHandsPtr, otherCounts, mainColor, levelCard, is2ConstMain, failedCards);                       
}

// 检测首家甩牌是否失败
int CUpgAlgorithm::CheckThrowFailed(const int* playCards_, int playCount,
                                    const int* otherHands_[3], const int otherCounts[3],
                                    int mainColor, int levelCard, bool is2ConstMain,
                                    int* failedCards)
{
    // 首先判断出牌的花色
    int playColor = -1;
    bool allSameColor = true;

    for (int i = 0; i < playCount; i++) {
        int cardMainValue = GetMainValue(playCards_[i], mainColor, levelCard, is2ConstMain);
        int cardColor = GetCardColor(playCards_[i]);

        int currentColor;
        if (cardMainValue >= 0) {
            currentColor = COLOR_MAIN;
        } else {
            currentColor = cardColor;
        }

        if (playColor == -1) {
            playColor = currentColor;
        } else if (playColor != currentColor) {
            allSameColor = false;
            break;
        }
    }

    // 如果不是同一花色，不能甩牌
    if (!allSameColor) {
        return 0;
    }

    int playCards[26];
    memcpy(playCards, playCards_, playCount * sizeof(int));
    int otherHands[3][26];
    for (int i = 0; i < 3; i++) {
        memcpy(otherHands[i], otherHands_[i], otherCounts[i] * sizeof(int));
    }
    //sort input cards.
    SortCards(playCards, playCount, mainColor, levelCard, is2ConstMain);
    for (int i = 0; i < 3; i++) {
        SortCards(otherHands[i], otherCounts[i], mainColor, levelCard, is2ConstMain);
    }

    // 将出牌分成对子和单张
    int playPairs[26], playPairCount;
    int playSingles[26], playSingleCount;
    SplitPairsAndSingles(playCards, playCount, playPairs, &playPairCount, playSingles, &playSingleCount);

    // 获取其他三家的同花色牌
    int otherColorCards[3][26];
    int otherColorCounts[3];

    for (int i = 0; i < 3; i++) {
        otherColorCounts[i] = GetColorCards(otherHands[i], otherCounts[i], playColor,
                                            mainColor, levelCard, is2ConstMain, otherColorCards[i]);
    }

    // 检查对子/拖拉机是否被大过
    // 对于纯对子甩牌（所有都是对子），检查最小对子是否被击败
    // 但纯拖拉机（连续对子）不检查
    if (playPairCount > 0 && playPairCount * 2 == playCount) {
        // 检查是否是纯拖拉机（连续对子）
        // 纯拖拉机不检查甩牌失败
        if (GetCardType(playCards, playCount, mainColor, levelCard, is2ConstMain) != CARD_TYPE_TRACTOR) {
            // 非拖拉机的纯对子，检查最小对子是否被更大的对子击败
            // 找最小对子
            int minPairValue = GetCardCompareValue(playPairs[0], mainColor, levelCard, is2ConstMain);
            int minPairCard = playPairs[0];
            for (int i = 1; i < playPairCount; i++) {
                int value = GetCardCompareValue(playPairs[i], mainColor, levelCard, is2ConstMain);
                if (value < minPairValue) {
                    minPairValue = value;
                    minPairCard = playPairs[i];
                }
            }
            
            // 检查对家是否有比最小对子更大的对子
            for (int i = 0; i < 3; i++) {
                if (otherColorCounts[i] < 2) continue;
                
                int otherPairs[26], otherPairCount;
                int otherSingles[26], otherSingleCount;
                SplitPairsAndSingles(otherColorCards[i], otherColorCounts[i],
                                    otherPairs, &otherPairCount, otherSingles, &otherSingleCount);
                
                if (otherPairCount > 0) {
                    for (int j = 0; j < otherPairCount; j++) {
                        int value = GetCardCompareValue(otherPairs[j], mainColor, levelCard, is2ConstMain);
                        if (value > minPairValue) {
                            // 对家有更大的对子，甩牌失败，返回最小对子
                            failedCards[0] = minPairCard;
                            failedCards[1] = minPairCard;
                            return 2;
                        }
                    }
                }
            }
        }
    }
    
    // 只有在有对子且不是纯拖拉机时才检查
    if (playPairCount > 0 && playPairCount * 2 < playCount) {
        // 提取最小的拖拉机牌型
        int tractorCards[26];
        int tractorLen = ExtractMinTractorType(playPairs, playPairCount, mainColor, levelCard, is2ConstMain, tractorCards);

        if (tractorLen > 0) {
            // 检查其他家是否有更大的同样长度的拖拉机
            for (int i = 0; i < 3; i++) {
                if (otherColorCounts[i] < tractorLen * 2) continue;

                // 将其他家的牌分成对子
                int otherPairs[26], otherPairCount;
                int otherSingles[26], otherSingleCount;
                SplitPairsAndSingles(otherColorCards[i], otherColorCounts[i],
                                    otherPairs, &otherPairCount, otherSingles, &otherSingleCount);

                if (otherPairCount < tractorLen) continue;

                // 检查是否有tractorLen长度的连续对子
                // 按比较值排序对子
                for (int p = 0; p < otherPairCount - 1; p++) {
                    for (int q = 0; q < otherPairCount - p - 1; q++) {
                        int val1 = GetCardCompareValue(otherPairs[q], mainColor, levelCard, is2ConstMain);
                        int val2 = GetCardCompareValue(otherPairs[q + 1], mainColor, levelCard, is2ConstMain);
                        if (val1 < val2) {
                            int temp = otherPairs[q];
                            otherPairs[q] = otherPairs[q + 1];
                            otherPairs[q + 1] = temp;
                        }
                    }
                }

                // 查找连续对子
                int j = 0;
                while (j <= otherPairCount - tractorLen) {
                    bool isContinuous = true;
                    for (int k = 0; k < tractorLen - 1; k++) {
                        if (!IsAdjacent(otherPairs[j + k], otherPairs[j + k + 1], mainColor, levelCard, is2ConstMain)) {
                            isContinuous = false;
                            break;
                        }
                    }

                    if (isContinuous) {
                        // 比较最大牌
                        int myMaxValue = GetCardCompareValue(tractorCards[0], mainColor, levelCard, is2ConstMain);
                        int otherMaxValue = GetCardCompareValue(otherPairs[j], mainColor, levelCard, is2ConstMain);

                        if (otherMaxValue > myMaxValue) {
                            // 甩牌失败，返回拖拉机的所有牌（每张对子两张，从大到小排序）
                            int failedCount = 0;
                            for (int k = 0; k < tractorLen; k++) {
                                failedCards[failedCount++] = tractorCards[k];
                                failedCards[failedCount++] = tractorCards[k];
                            }
                            return failedCount;
                        }
                        break;
                    }
                    j++;
                }
            }
        }
    }

    // 检查单张是否被大过（只有在有对子或多个单张时才检查）
    if (playSingleCount > 0 && (playPairCount > 0 || playSingleCount > 1)) {
        // 找到最小的单张
        int minSingleValue = GetCardCompareValue(playSingles[0], mainColor, levelCard, is2ConstMain);
        int minSingleCard = playSingles[0];

        for (int i = 1; i < playSingleCount; i++) {
            int value = GetCardCompareValue(playSingles[i], mainColor, levelCard, is2ConstMain);
            if (value < minSingleValue) {
                minSingleValue = value;
                minSingleCard = playSingles[i];
            }
        }

        // 检查其他家是否有更大的单张
        for (int i = 0; i < 3; i++) {
            if (otherColorCounts[i] == 0) continue;

            // 将其他家的牌分成对子和单张
            int otherPairs[26], otherPairCount;
            int otherSingles[26], otherSingleCount;
            SplitPairsAndSingles(otherColorCards[i], otherColorCounts[i],
                                otherPairs, &otherPairCount, otherSingles, &otherSingleCount);

            // 如果其他家有单张，检查是否有比我方最小单张更大的
            if (otherSingleCount > 0) {
                for (int j = 0; j < otherSingleCount; j++) {
                    int value = GetCardCompareValue(otherSingles[j], mainColor, levelCard, is2ConstMain);
                    if (value > minSingleValue) {
                        // 其他家有更大的单张，甩牌失败
                        failedCards[0] = minSingleCard;
                        return 1;
                    }
                }
            }
        }
    }

    // 甩牌成功
    return 0;
}

bool CUpgAlgorithm::ExtractCards(int *pnSour, int nSour, int *pnDest, int nDest,bool bTest){
    if(nSour < nDest) 
        return false;
    // 创建临时数组存储源数据
    std::vector<int> temp(pnSour, pnSour + nSour);
    for(int i=0; i< nDest; i++){
        auto it = std::find(temp.begin(), temp.end(), pnDest[i]);
        if(it != temp.end()){
            temp.erase(it);
        }else{
            return false;
        }
    }
    // 如果成功且非测试模式，则更新源数据
    if(!bTest) {
        memcpy(pnSour, temp.data(), temp.size() * sizeof(int));
    }
    return true;
}
