#ifndef __UPGALGORITHM_H__
#define __UPGALGORITHM_H__

#include <stdint.h>

// 牌花色定义
typedef enum {
    COLOR_DIAMOND = 0,  // 方块
    COLOR_CLUB,         // 梅花
    COLOR_HEART,        // 红桃
    COLOR_SPADE,        // 黑桃
    COLOR_SKING,        // 小王
    COLOR_BKING,        // 大王
    COLOR_MAIN = 10     // 主花色
} CARDCOLOR;

// 牌型定义
typedef enum {
    CARD_TYPE_INVALID = 0,  // 无效牌型
    CARD_TYPE_SINGLE = 1,      // 单牌
    CARD_TYPE_PAIR,            // 对子
    CARD_TYPE_MIX,             // 混合对子和单张
    CARD_TYPE_TRACTOR,         // 拖拉机
} CARDTYPE;

// 出牌结果
typedef enum {
    PUT_CARD_RESULT_SUCCESS = 0,     // 成功
    PUT_CARD_RESULT_INVALID_TYPE,    // 无效牌型
    PUT_CARD_RESULT_WRONG_COUNT,     // 张数不符
    PUT_CARD_RESULT_WRONG_COLOR,     // 花色不符
    PUT_CARD_RESULT_CANNOT_FOLLOW,   // 无法跟牌
} PutCardResult;

/**
 * 升级游戏算法类
 * 实现出牌规则判断、牌型识别、胜负判定等核心算法
 */
class CUpgAlgorithm
{
public:

    static const int MAX_CARDS = 25;  // 最大手牌数量

    /**
     * 洗牌
     * @param pCard 牌数组
     * @param nCardCount 牌数量
     * @return 无
     */
    static void ShuffleCard(int *pCard,int nCardCount);

    /**
     * 初始化牌数组
     * @param cards 牌数组
     */
    static void InitCard(int cards[108]);

    /**
     * 获取指定花色的牌
     * @param cards 牌数组
     * @param count 牌数量
     * @param color 花色
     * @param mainColor 主花色
     * @param levelCard 级牌
     * @param is2ConstMain 2是否为常主
     * @param colorCards 指定花色的牌（输出参数）
     * @return 指定花色的牌数量
     */
    static int GetColorCards(const int* cards, int count, int color, int mainColor, int levelCard, bool is2ConstMain, int colorCards[MAX_CARDS]);

    /**
     * 判断牌型
     * @param cards 牌数组
     * @param count 牌数量
     * @param mainColor 主花色
     * @param levelCard 级牌
     * @param is2ConstMain 2是否为常主
     * @return 牌型
     */
    static int GetCardType(const int* cards, int count, int mainColor, int levelCard, bool is2ConstMain);

    /**
     * 判断首家出牌是否符合规则
     * 支持单一牌型（单张、对子、拖拉机等）和甩牌（多个对子、多个单张等）
     * @param cards 牌数组
     * @param count 牌数量
     * @param mainColor 主花色
     * @param levelCard 级牌
     * @param is2ConstMain 2是否为常主
     * @return 出牌结果
     */
    static PutCardResult ValidateFirstPlay(const int* cards, int count, int mainColor, int levelCard, bool is2ConstMain);

    /**
     * 判断跟牌是否符合规则
     * @param leadCards 首家出的牌
     * @param leadCount 首家出牌数量
     * @param followCards 跟牌
     * @param followCount 跟牌数量
     * @param handCards 跟牌者的手牌（包含已出的牌）
     * @param handCount 跟牌者的手牌数量
     * @param mainColor 主花色
     * @param levelCard 级牌
     * @param is2ConstMain 2是否为常主
     * @return 出牌结果
     */
    static PutCardResult ValidateFollowPlay(const int* leadCards, int leadCount,
                                            const int* followCards, int followCount,
                                            const int* handCards, int handCount,
                                            int mainColor, int levelCard, bool is2ConstMain);

    /**
     * 比较一轮牌的大小，确定赢家
     * @param cardsPlayed 四家出的牌，按座位顺序排列
     * @param cardsCount 每家出牌数量
     * @param mainColor 主花色
     * @param levelCard 级牌
     * @param is2ConstMain 2是否为常主
     * @param firstPlayer 首家座位号
     * @return 赢家座位号
     */
    static int DetermineWinner(const int* cardsPlayed[4], const int cardsCount[4], int mainColor, int levelCard, bool is2ConstMain, int firstPlayer);

    /**
     * 根据首家出牌，从手牌中找出必须出的牌
     * @param handCards 手牌
     * @param handCount 手牌数量
     * @param leadCards 首家出的牌
     * @param leadCount 首家出牌数量
     * @param mainColor 主花色
     * @param levelCard 级牌
     * @param is2ConstMain 2是否为常主
     * @param mustPlayCards 必须出的牌（输出参数）
     * @return 必须出的牌数量
     */
    static int GetMustPlayCards(const int* handCards, int handCount, const int* leadCards, int leadCount, int mainColor, int levelCard, bool is2ConstMain, int* mustPlayCards);

    /**
     * 根据花色和值构造牌
     * @param nColor 花色
     * @param nValue 牌值
     * @return 牌值
     */
    static int BuildCard(int nColor,int nValue);

    /**
     * 获取牌的花色
     * @param card 牌值
     * @return 花色
     */
    static int GetCardColor(int card);

    /**
     * 获取牌的值
     * @param card 牌值
     * @return 牌值
     */
    static int GetCardValue(int card);

    /**
     * 获取牌的比较值
     * @param card 牌值
     * @param mainColor 主花色
     * @param levelCard 级牌
     * @param is2ConstMain 2是否为常主
     * @return 比较值
     */
    static int GetCardCompareValue(int card, int mainColor, int levelCard, bool is2ConstMain);

    /**
     * 判断两张牌是否相邻
     * @param card1 牌1
     * @param card2 牌2
     * @param mainColor 主花色
     * @param levelCard 级牌
     * @param is2ConstMain 2是否为常主
     * @return 是否相邻
     */
    static bool IsAdjacent(int card1, int card2, int mainColor, int levelCard, bool is2ConstMain);

    /**
     * 获取牌的主牌值
     * @param card 牌值
     * @param mainColor 主花色
     * @param levelCard 级牌
     * @param is2ConstMain 2是否为常主
     * @return 主牌值
     */
    static int GetMainValue(int card, int mainColor, int levelCard, bool is2ConstMain);

    /**
     * 检测首家甩牌是否失败，如果失败则返回最小的必须出的牌型
     * @param playCards 首家出的牌
     * @param playCount 首家出牌数量
     * @param otherHands 其他三家的手牌（按座位顺序）
     * @param otherCounts 其他三家的手牌数量
     * @param mainColor 主花色
     * @param levelCard 级牌
     * @param is2ConstMain 2是否为常主
     * @param failedCards 甩牌失败时必须出的牌（输出参数）
     * @return 甩牌失败时必须出的牌数量，0表示甩牌成功
     */
    static int CheckThrowFailed(const int* playCards, int playCount,
                                const int* otherHands[3], const int otherCounts[3],
                                int mainColor, int levelCard, bool is2ConstMain,
                                int* failedCards);

    static int CheckThrowFailed(const int* playCards, int playCount,
                                const int otherHands[3][25], const int otherCounts[3],
                                int mainColor, int levelCard, bool is2ConstMain,
                                int* failedCards);


    /**
     * 抽取牌
     * @param pnSour 源牌（输入参数）
     * @param nSour 源牌数量
     * @param pnDest 目标牌（输出参数）
     * @param nDest 目标牌数量
     * @param bTest 是否测试，测试时不会改变源牌
     * @return 是否成功
     */
    static bool ExtractCards(int *pnSour, int nSour, int *pnDest, int nDest,bool bTest);

    /**
     * 对牌进行排序
     * @param cards 牌数组
     * @param count 牌数量
     * @param mainColor 主花色
     * @param levelCard 级牌
     * @param is2ConstMain 2是否为常主
     * @return 无
     */
    static void SortCards(int* cards, int count, int mainColor, int levelCard, bool is2ConstMain);
};

#endif // __UPGALGORITHM_H__