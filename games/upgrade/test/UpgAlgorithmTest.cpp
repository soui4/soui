#include "UpgAlgorithm.h"
#include <gtest/gtest.h>

// 测试获取牌的花色
TEST(UpgAlgorithmTest, GetCardColor) {
    // 普通牌测试
    EXPECT_EQ(COLOR_DIAMOND, CUpgAlgorithm::GetCardColor(0));   // 方块A
    EXPECT_EQ(COLOR_CLUB, CUpgAlgorithm::GetCardColor(13));     // 梅花A
    EXPECT_EQ(COLOR_HEART, CUpgAlgorithm::GetCardColor(26));    // 红桃A
    EXPECT_EQ(COLOR_SPADE, CUpgAlgorithm::GetCardColor(39));    // 黑桃A
    
    // 王牌测试
    EXPECT_EQ(COLOR_SKING, CUpgAlgorithm::GetCardColor(52));    // 小王
    EXPECT_EQ(COLOR_BKING, CUpgAlgorithm::GetCardColor(53));    // 大王
}

// 测试获取牌的值
TEST(UpgAlgorithmTest, GetCardValue) {
    // 普通牌测试
    EXPECT_EQ(0, CUpgAlgorithm::GetCardValue(0));   // 方块A
    EXPECT_EQ(1, CUpgAlgorithm::GetCardValue(1));   // 方块2
    EXPECT_EQ(12, CUpgAlgorithm::GetCardValue(12)); // 方块K
    EXPECT_EQ(0, CUpgAlgorithm::GetCardValue(13));  // 梅花A
    EXPECT_EQ(12, CUpgAlgorithm::GetCardValue(25)); // 梅花K
    
    // 王牌测试
    EXPECT_EQ(0, CUpgAlgorithm::GetCardValue(52));  // 小王
    EXPECT_EQ(1, CUpgAlgorithm::GetCardValue(53));  // 大王
}

// 测试牌型判断
TEST(UpgAlgorithmTest, GetCardType) {
    // 单牌
    int single_card[] = { 5 };
    EXPECT_EQ(CARD_TYPE_SINGLE, CUpgAlgorithm::GetCardType(single_card, 1, COLOR_HEART, 1, false));
    
    // 对子 (相同值不同花色)
    int pair_cards[] = { 18, 18 }; // 梅花6和梅花6 (values are 5)
    EXPECT_EQ(CARD_TYPE_PAIR, CUpgAlgorithm::GetCardType(pair_cards, 2, COLOR_HEART, 1, false));

    // 三张 (相同值不同花色) - 使用黑桃6而不是红桃6，因为红桃是主花色
    int triple_cards[] = { 5, 18, 44 }; // 方块6、梅花6和黑桃6 (values are 5)
    EXPECT_EQ(CARD_TYPE_INVALID, CUpgAlgorithm::GetCardType(triple_cards, 3, COLOR_HEART, 1, false));
    
    // 拖拉机测试 (连续对子)
    int tractor_cards[] = { 5, 5, 6, 6 }; // 方块6,方块6,方块7,方块7 (values 5-5-6-6)
    EXPECT_EQ(CARD_TYPE_TRACTOR, CUpgAlgorithm::GetCardType(tractor_cards, 4, COLOR_HEART, 1, false));


    // 测试两张不同value的主牌不是对子
    int two_main_cards[] = { 26, 1 }; // 红桃A,方块2（主花色为红桃，value不同）
    EXPECT_EQ(CARD_TYPE_SINGLE, CUpgAlgorithm::GetCardType(two_main_cards, 2, COLOR_HEART, 8, true));
    EXPECT_EQ(CARD_TYPE_INVALID, CUpgAlgorithm::GetCardType(two_main_cards, 2, COLOR_HEART, 8, false));
}

// 测试首家出牌验证
TEST(UpgAlgorithmTest, ValidateFirstPlay) {
    // 测试1: 有效的单一牌型 - 对子
    int valid_pair[] = { 5, 5 }; // 方块6,方块6
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFirstPlay(valid_pair, 2, COLOR_HEART, 1, false));

    // 测试2: 无效的首家出牌 (空牌)
    EXPECT_EQ(PUT_CARD_RESULT_INVALID_TYPE,
              CUpgAlgorithm::ValidateFirstPlay(nullptr, 0, COLOR_HEART, 1, false));

    // 测试3: 有效的甩牌 - 多个单张（同花色）
    int throw_singles[] = { 0, 1, 3, 5 }; // 方块A,2,4,6（同花色，不连续）
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFirstPlay(throw_singles, 4, COLOR_HEART, 8, false));

    // 测试4: 有效的甩牌 - 多个对子（同花色，不连续）
    int throw_pairs[] = { 0, 0, 2, 2, 5, 5 }; // 方块A,A,3,3,6,6（3个对子，不连续）
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFirstPlay(throw_pairs, 6, COLOR_HEART, 8, false));

    // 测试5: 有效的甩牌 - 混合对子和单张（同花色）
    int throw_mixed[] = { 0, 0, 1, 3 }; // 方块A,A,2,4（1个对子+2个单张）
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFirstPlay(throw_mixed, 4, COLOR_HEART, 8, false));

    // 测试6: 无效的甩牌 - 不同花色
    int invalid_throw[] = { 0, 13, 2, 15 }; // 方块A,梅花A,方块3,梅花3（不同花色）
    EXPECT_EQ(PUT_CARD_RESULT_INVALID_TYPE,
              CUpgAlgorithm::ValidateFirstPlay(invalid_throw, 4, COLOR_HEART, 8, false));

    // 测试7: 有效的拖拉机（单一牌型）
    int tractor[] = { 0, 0, 1, 1 }; // 方块A,A,2,2（拖拉机）
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFirstPlay(tractor, 4, COLOR_HEART, 8, false));

    // 测试8: 有效的甩牌 - 主牌多个对子
    int main_throw[] = { 26, 26, 27, 27, 29, 29 }; // 红桃A,A,2,2,4,4（主花色，3个对子）
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFirstPlay(main_throw, 6, COLOR_HEART, 8, false));
}

// 测试跟牌验证
TEST(UpgAlgorithmTest, ValidateFollowPlay) {
    // 测试1: 正确的跟牌 - 单牌
    int lead_cards1[] = { 5 };  // 方块6
    int follow_cards1[] = { 6 }; // 方块7
    int hand_cards1[] = { 6, 7, 27, 28 }; // 手牌有方块7,8和红桃2,3
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFollowPlay(lead_cards1, 1, follow_cards1, 1, hand_cards1, 4, COLOR_HEART, 8, false));

    // 测试2: 错误的跟牌（张数不符）
    EXPECT_EQ(PUT_CARD_RESULT_WRONG_COUNT,
              CUpgAlgorithm::ValidateFollowPlay(lead_cards1, 1, follow_cards1, 2, hand_cards1, 4, COLOR_HEART, 8, false));

    // 测试3: 正确的对子跟牌
    int lead_pair[] = { 5, 5 };  // 方块6,6（对子）
    int follow_pair[] = { 6, 6 }; // 方块7,7（对子）
    int hand_pair[] = { 6, 6, 7, 27 }; // 手牌有方块7,7,8和红桃2
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFollowPlay(lead_pair, 2, follow_pair, 2, hand_pair, 4, COLOR_HEART, 8, false));

    // 测试4: 正确的主牌跟牌
    int lead_main_cards[] = { 27 };  // 红桃2（主牌）
    int follow_main_cards[] = { 28 }; // 红桃3（主牌）
    int hand_main_cards[] = { 28, 29, 30 }; // 手牌有红桃3,4,5
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFollowPlay(lead_main_cards, 1, follow_main_cards, 1, hand_main_cards, 3, COLOR_HEART, 8, false));

    // 测试5: 有同花色对子但没出对子（应该失败）
    int lead_pair_cards2[] = { 0, 0 }; // 方块A,A（对子，副牌）
    int follow_wrong_cards[] = { 1, 2 }; // 方块2,3（两张单牌，不是对子）
    int hand_with_pair2[] = { 1, 2, 3, 3 }; // 手牌有方块2,3,4,4（有对子4,4）
    EXPECT_EQ(PUT_CARD_RESULT_CANNOT_FOLLOW,
              CUpgAlgorithm::ValidateFollowPlay(lead_pair_cards2, 2, follow_wrong_cards, 2, hand_with_pair2, 4, COLOR_HEART, 8, false));

    // 测试6: 手中同花色牌不足，全部出了（应该成功）
    int lead_cards6[] = { 0, 0 }; // 方块A,A（对子）
    int follow_cards6[] = { 1, 27 }; // 方块2 + 红桃2（手中只有1张方块）
    int hand_cards6[] = { 1, 27, 28 }; // 手牌只有1张方块，其他是红桃（主牌）
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFollowPlay(lead_cards6, 2, follow_cards6, 2, hand_cards6, 3, COLOR_HEART, 8, false));

    // 测试7: 有同花色但没全出（应该失败）
    // value: 0=A, 1=2, 2=3, ...
    int lead_cards7[] = { 1, 1 }; // 方块2,2（对子）
    int follow_cards7[] = { 27, 28 }; // 红桃2,3（主牌）
    int hand_cards7[] = { 2, 27, 28 }; // 手牌有方块3，但没出
    EXPECT_EQ(PUT_CARD_RESULT_WRONG_COLOR,
              CUpgAlgorithm::ValidateFollowPlay(lead_cards7, 2, follow_cards7, 2, hand_cards7, 3, COLOR_HEART, 8, false));

    // 测试8: 有拖拉机但没出拖拉机（应该失败）
    int lead_tractor[] = { 1, 1, 2, 2 }; // 方块2,2,3,3（拖拉机）
    int follow_wrong_tractor[] = { 3, 3, 4, 5 }; // 方块4,4,5,6（一对+两单）
    int hand_with_tractor[] = { 3, 3, 4, 5, 6, 6 }; // 手牌有方块4,4,5,6,7,7（有拖拉机4,4,5,5但没出，还有拖拉机6,6,7,7）
    EXPECT_EQ(PUT_CARD_RESULT_CANNOT_FOLLOW,
              CUpgAlgorithm::ValidateFollowPlay(lead_tractor, 4, follow_wrong_tractor, 4, hand_with_tractor, 6, COLOR_HEART, 8, false));

    // 测试9: 手中没有对应牌型时，可以出任意组合（垫牌）- 应该成功
    int lead_pair_cards9[] = { 0, 0 }; // 方块A,A（对子）
    int follow_no_pair[] = { 1, 2 }; // 方块2,3（两张单牌，不是对子）
    int hand_no_pair[] = { 1, 2, 3 }; // 手牌有方块2,3,4（没有对子）
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFollowPlay(lead_pair_cards9, 2, follow_no_pair, 2, hand_no_pair, 3, COLOR_HEART, 8, false));

    {
    // 测试10: 首家出拖拉机，手中没有拖拉机但有对子，可以出任意组合 - 应该成功
    int lead_tractor10[] = { 1, 1, 2, 2 }; // 方块2,2,3,3（拖拉机）
    int follow_only_pair[] = { 3, 3, 5, 6 }; // 方块4,4,6,7（一对+两单）
    int hand_only_pair[] = { 3, 3, 5, 6, 3+13, 5+13 }; // 手牌只有方块3,3,4,5（有对子但没有拖拉机）
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFollowPlay(lead_tractor10, 4, follow_only_pair, 4, hand_only_pair, 6, COLOR_HEART, 8, false));
    }
    {
    // 测试10.2: 首家出拖拉机，手中没有拖拉机但有对子不出 - 应该失败
    int lead_tractor10[] = { 1, 1, 2, 2 }; // 方块2,2,3,3（拖拉机）
    int follow_only_pair[] = {5, 6, 7, 8 }; // 不出对子
    int hand_only_pair[] = { 3, 3, 5, 6, 7,8, 3+13, 5+13 }; // 手牌只有方块3,3,4,5（有对子但没有拖拉机）
    EXPECT_EQ(PUT_CARD_RESULT_CANNOT_FOLLOW,
              CUpgAlgorithm::ValidateFollowPlay(lead_tractor10, 4, follow_only_pair, 4, hand_only_pair, 8, COLOR_HEART, 4, false));
    }
    // 测试11: 首家出对子，手中有更大对子但出小对子 - 应该成功
    int lead_pair11[] = { 1, 1 }; // 方块2,2（对子）
    int follow_pair11[] = { 0, 0 }; // 方块A,A（对子，更小）
    int hand_pair11[] = { 0, 0, 2, 2 }; // 手牌有方块A,A,3,3（有两个对子）
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFollowPlay(lead_pair11, 2, follow_pair11, 2, hand_pair11, 4, COLOR_HEART, 8, false));
              
    // 测试12: 首家出对子，手中有主牌对子但出副牌对子 - 应该成功
    int lead_pair12[] = { 1, 1 }; // 方块2,2（对子）
    int follow_pair12[] = { 2, 2 }; // 方块3,3（副牌对子）
    int hand_pair12[] = { 2, 2, 28, 28 }; // 手牌有方块3,3和红桃3,3（主牌对子）
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFollowPlay(lead_pair12, 2, follow_pair12, 2, hand_pair12, 4, COLOR_HEART, 8, false));
              
    // 测试13: 首家出单张，手中有同花色更大单张但出小单张 - 应该成功
    int lead_single13[] = { 1 }; // 方块2（单张）
    int follow_single13[] = { 0 }; // 方块A（单张，更小）
    int hand_single13[] = { 0, 2 }; // 手牌有方块A,3（有更大单张）
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFollowPlay(lead_single13, 1, follow_single13, 1, hand_single13, 2, COLOR_HEART, 8, false));
              
    // 测试14: 首家出单张，手中有主牌但出副牌 - 应该成功
    int lead_single14[] = { 1 }; // 方块2（单张）
    int follow_single14[] = { 2 }; // 方块3（副牌单张）
    int hand_single14[] = { 2, 28 }; // 手牌有方块3和红桃3（主牌）
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFollowPlay(lead_single14, 1, follow_single14, 1, hand_single14, 2, COLOR_HEART, 8, false));
              
    // 测试15: 首家出拖拉机，手中有更大拖拉机但出小拖拉机 - 应该成功
    int lead_tractor15[] = { 11, 11, 12, 12 }; // 方块Q,Q,K,K（拖拉机）
    int follow_tractor15[] = { 9, 9, 10, 10 }; // 方块10,10,J,J（拖拉机，更小）
    int hand_tractor15[] = { 9, 9, 10, 10, 11, 11, 12, 12 }; // 手牌有方块10,10,J,J,Q,Q,K,K（有更大拖拉机）
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFollowPlay(lead_tractor15, 4, follow_tractor15, 4, hand_tractor15, 8, COLOR_HEART, 8, false));
              
    // 测试16: 首家出对子，手中有对子但花色不符且无同花色牌 - 应该成功（垫牌）
    int lead_pair16[] = { 0, 0 }; // 方块A,A（对子）
    int follow_pair16[] = { 14, 14 }; // 梅花A,A（不同花色对子）
    int hand_pair16[] = { 14, 14, 27, 28 }; // 手牌有梅花A,A和红桃2,3
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFollowPlay(lead_pair16, 2, follow_pair16, 2, hand_pair16, 4, COLOR_HEART, 8, false));
              
    // 测试17: 首家出拖拉机，手中有部分同花色牌但不构成拖拉机 - 应该成功（垫牌）
    int lead_tractor17[] = { 11, 11, 12, 12 }; // 方块Q,Q,K,K（拖拉机）
    int follow_tractor17[] = { 27, 28, 29, 30 }; // 红桃2,3,4,5（主牌）
    int hand_tractor17[] = { 9, 27, 28, 29, 30 }; // 手牌有方块10和红桃2,3,4,5
    EXPECT_EQ(PUT_CARD_RESULT_WRONG_COLOR,
              CUpgAlgorithm::ValidateFollowPlay(lead_tractor17, 4, follow_tractor17, 4, hand_tractor17, 5, COLOR_HEART, 8, false));
              
    // 测试18: 首家出单张，手中既有同花色又有主牌，但只出了主牌 - 应该失败
    int lead_single18[] = { 1 }; // 方块2（单张）
    int follow_single18[] = { 28 }; // 红桃3（主牌）
    int hand_single18[] = { 2, 28 }; // 手牌有方块3和红桃3
    EXPECT_EQ(PUT_CARD_RESULT_WRONG_COLOR,
              CUpgAlgorithm::ValidateFollowPlay(lead_single18, 1, follow_single18, 1, hand_single18, 2, COLOR_HEART, 8, false));
              
    // 测试19: 首家出对子，手中有同花色对子和主牌对子，但只出了主牌对子 - 应该失败
    int lead_pair19[] = { 1, 1 }; // 方块2,2（对子）
    int follow_pair19[] = { 28, 28 }; // 红桃3,3（主牌对子）
    int hand_pair19[] = { 2, 2, 28, 28 }; // 手牌有方块3,3和红桃3,3
    EXPECT_EQ(PUT_CARD_RESULT_WRONG_COLOR,
              CUpgAlgorithm::ValidateFollowPlay(lead_pair19, 2, follow_pair19, 2, hand_pair19, 4, COLOR_HEART, 8, false));
              
              
    // 测试21: 首家出对子，手中有更大对子但没出 - 应该成功
    int lead_pair21[] = { 1, 1 }; // 方块2,2（对子）
    int follow_pair21[] = { 2, 2 }; // 方块3,3（对子）
    int hand_pair21[] = { 2, 2, 5, 5 }; // 手牌有方块3,3,6,6（有更大对子方块6,6但没出）
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFollowPlay(lead_pair21, 2, follow_pair21, 2, hand_pair21, 4, COLOR_HEART, 8, false));
              
    // 测试22: 首家出拖拉机，手中有更大拖拉机但没出 - 应该成功
    int lead_tractor22[] = { 9, 9, 10, 10 }; // 方块10,10,J,J（拖拉机）
    int follow_tractor22[] = { 11, 11, 12, 12 }; // 方块Q,Q,K,K（拖拉机）
    int hand_tractor22[] = { 11, 11, 12, 12, 0, 0, 1, 1 }; // 手牌有方块Q,Q,K,K,A,A,2,2
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS,
              CUpgAlgorithm::ValidateFollowPlay(lead_tractor22, 4, follow_tractor22, 4, hand_tractor22, 8, COLOR_HEART, 8, false));
              
    // 测试23: 首家出单张，手中有同花色但出了不同花色 - 应该失败
    int lead_single23[] = { 1 }; // 方块2（单张）
    int follow_single23[] = { 14 }; // 梅花A（不同花色）
    int hand_single23[] = { 2, 14 }; // 手牌有方块3,梅花A（有同花色方块3但没出）
    EXPECT_EQ(PUT_CARD_RESULT_WRONG_COLOR,
              CUpgAlgorithm::ValidateFollowPlay(lead_single23, 1, follow_single23, 1, hand_single23, 2, COLOR_HEART, 8, false));
              
    // 测试24: 首家出对子，手中有同花色对子但出了不同花色对子 - 应该失败
    int lead_pair24[] = { 1, 1 }; // 方块2,2（对子）
    int follow_pair24[] = { 14, 14 }; // 梅花A,A（不同花色对子）
    int hand_pair24[] = { 2, 2, 14, 14 }; // 手牌有方块3,3,梅花A,A（有同花色对子方块3,3但没出）
    EXPECT_EQ(PUT_CARD_RESULT_WRONG_COLOR,
              CUpgAlgorithm::ValidateFollowPlay(lead_pair24, 2, follow_pair24, 2, hand_pair24, 4, COLOR_HEART, 8, false));
}

// 测试赢家判定
TEST(UpgAlgorithmTest, DetermineWinner) {
    // 简单测试：首家出黑桃A，其他人出较小的副牌（级牌是10，避开小牌）
    int player0_cards[] = { 51 }; // 黑桃K (3*13 + 12)
    int player1_cards[] = { 38 }; // 红桃K (2*13 + 12)
    int player2_cards[] = { 25 }; // 梅花K (1*13 + 12)
    int player3_cards[] = { 12 }; // 方块K (0*13 + 12)

    const int* cards_played[4] = { player0_cards, player1_cards, player2_cards, player3_cards };
    int cards_count[4] = { 1, 1, 1, 1 };

    // 首家应该是赢家（黑桃是主花色，级牌是10）
    EXPECT_EQ(0, CUpgAlgorithm::DetermineWinner(cards_played, cards_count, COLOR_SPADE, 8, false, 0));

    // 如果红桃是主花色，player1获胜因为红桃K是主牌
    EXPECT_EQ(1, CUpgAlgorithm::DetermineWinner(cards_played, cards_count, COLOR_HEART, 8, false, 0));
    
    // 测试主牌获胜的情况
    int player0_cards2[] = { 26 }; // 红桃A (主牌)
    int player1_cards2[] = { 0 };  // 方块A (副牌)
    const int* cards_played2[4] = { player0_cards2, player1_cards2, player1_cards2, player1_cards2 };
    int cards_count2[4] = { 1, 1, 1, 1 };
    
    // 主牌应该获胜
    EXPECT_EQ(0, CUpgAlgorithm::DetermineWinner(cards_played2, cards_count2, COLOR_HEART, 1, false, 0));

    // 测试牌型不匹配的情况：首家出对子，其他玩家出单牌（避开级牌10）
    int player0_pair[] = { 1, 1 }; // 方块2,2（对子）
    int player1_single[] = { 12, 11 }; // 方块K,Q（两张单牌，不是对子）
    int player2_single[] = { 10, 9 }; // 方块10,9（两张单牌）
    int player3_single[] = { 7, 6 }; // 方块8,7（两张单牌，避开级牌10）
    const int* cards_played3[4] = { player0_pair, player1_single, player2_single, player3_single };
    int cards_count3[4] = { 2, 2, 2, 2 };

    // 首家应该获胜，因为其他玩家牌型不匹配
    EXPECT_EQ(0, CUpgAlgorithm::DetermineWinner(cards_played3, cards_count3, COLOR_HEART, 8, false, 0));

    // 测试牌型匹配的情况：首家出对子，其他玩家也出对子
    // value: 0=A, 1=2, 2=3, 3=4, 4=5
    int player0_pair2[] = { 1, 1 }; // 方块2,2（对子）
    int player1_pair2[] = { 2, 2 }; // 方块3,3（对子，更大）
    int player2_pair2[] = { 3, 3 }; // 方块4,4（对子，更大）
    int player3_pair2[] = { 4, 4 }; // 方块5,5（对子，最大）
    const int* cards_played4[4] = { player0_pair2, player1_pair2, player2_pair2, player3_pair2 };
    int cards_count4[4] = { 2, 2, 2, 2 };

    // player3应该获胜，因为方块5,5最大
    EXPECT_EQ(3, CUpgAlgorithm::DetermineWinner(cards_played4, cards_count4, COLOR_HEART, 8, false, 0));

    // 测试拖拉机的情况：首家出拖拉机，其他玩家出单牌或对子
    // value: 0=A, 1=2, 2=3, 3=4, 4=5, 5=6, 6=7, 7=8, 8=9
    int player0_tractor[] = { 1, 1, 2, 2 }; // 方块2,2,3,3（拖拉机）
    int player1_singles[] = { 3, 4, 5, 6 }; // 方块4,5,6,7（单牌）
    int player2_pairs[] = { 7, 7, 9, 9 }; // 方块8,8,10,10（两个对子，但不是拖拉机，因为8和10不相邻,级牌是9）
    int player3_pairs2[] = { 5, 5, 6, 6 }; // 方块6,6,7,7（拖拉机）
    const int* cards_played5[4] = { player0_tractor, player1_singles, player2_pairs, player3_pairs2 };
    int cards_count5[4] = { 4, 4, 4, 4 };

    // player2应该获胜，因为当级牌为9时方块8,8,10,10最大6,6,7,7比方块2,2,3,3大
    EXPECT_EQ(2, CUpgAlgorithm::DetermineWinner(cards_played5, cards_count5, COLOR_HEART, 8, false, 0));

    // player3应该获胜，因为当级牌为不9时方块6,6,7,7比方块2,2,3,3大
    EXPECT_EQ(3, CUpgAlgorithm::DetermineWinner(cards_played5, cards_count5, COLOR_HEART, 4, false, 0));


    // 测试BUG修复：首家出副牌对子，其他玩家出主牌但不是对子
    int player0_side_pair[] = { 2, 2 }; // 方块3,3（副牌对子）
    int player1_main_singles[] = { 27, 28 }; // 红桃2,3（主牌，但是两张单牌，不是对子）
    int player2_side_pair2[] = { 3, 3 }; // 方块4,4（副牌对子）
    int player3_side_pair3[] = { 4, 4 }; // 方块5,5（副牌对子）
    const int* cards_played6[4] = { player0_side_pair, player1_main_singles, player2_side_pair2, player3_side_pair3 };
    int cards_count6[4] = { 2, 2, 2, 2 };

    // player3应该获胜（方块5,5最大），player1虽然出了主牌但牌型不匹配，不能获胜
    EXPECT_EQ(3, CUpgAlgorithm::DetermineWinner(cards_played6, cards_count6, COLOR_HEART, 8, false, 0));

    // 测试2为常主时的情况
    player0_side_pair[0] = player0_side_pair[1] = 1; // 方块2,2
    EXPECT_EQ(0, CUpgAlgorithm::DetermineWinner(cards_played6, cards_count6, COLOR_HEART, 8, true, 0));

    // 测试：首家出副牌对子，其他玩家出主牌对子（牌型匹配）
    int player0_side_pair_b[] = { 1, 1 }; // 方块2,2（副牌对子）
    int player1_main_pair[] = { 26, 26 }; // 红桃A,A（主牌对子）
    int player2_side_pair_b2[] = { 2, 2 }; // 方块3,3（副牌对子）
    int player3_side_pair_b3[] = { 3, 3 }; // 方块4,4（副牌对子）
    const int* cards_played7[4] = { player0_side_pair_b, player1_main_pair, player2_side_pair_b2, player3_side_pair_b3 };
    int cards_count7[4] = { 2, 2, 2, 2 };

    // player1应该获胜（主牌对子大于副牌对子，且牌型匹配）
    EXPECT_EQ(1, CUpgAlgorithm::DetermineWinner(cards_played7, cards_count7, COLOR_HEART, 8, false, 0));

    // 测试：首家出主牌对子，其他玩家出级牌非对子
    {
        int player0_main_pair_b[] = { 8, 8 }; // 方块9,9（主牌对子）
        int player1_main_pair[] = { 7, 7 }; // 方块8,8（主牌对子）
        int player2_side_pair_b2[] = { 4, 4+13 }; // 方块5,梅花5
        int player3_side_pair_b3[] = { 3, 3 }; // 方块4,4（副牌对子）
        const int* cards_played7[4] = { player0_main_pair_b, player1_main_pair, player2_side_pair_b2, player3_side_pair_b3 };
        int cards_count7[4] = { 2, 2, 2, 2 };

        // player0应该获胜
        EXPECT_EQ(0, CUpgAlgorithm::DetermineWinner(cards_played7, cards_count7, COLOR_DIAMOND, 4, true, 0));

    }
    //测试：首家AA77 对家有对KK的情况，期望甩牌失败
    {
        int player0_cards[] = { 0, 0, 6, 6 }; // 方块A,A,7,7
        PutCardResult result = CUpgAlgorithm::ValidateFirstPlay(player0_cards,4, COLOR_HEART, 4, true);
        EXPECT_EQ(PUT_CARD_RESULT_SUCCESS, result);
        int cards_count7[4] = { 4, 2, 2, 2 };
        int otherHands[3][25] = { 
            {12,12},// 方块K,K
            {1,1},// 方块2,2
            {2,2}// 方块3,3
         };
        int throwFailedCard[25];
        int nFailedCount=CUpgAlgorithm::CheckThrowFailed(player0_cards, 4, otherHands, cards_count7, COLOR_HEART, 4, true, throwFailedCard);
        EXPECT_EQ(2,nFailedCount);
     }
     //测试：首家副牌甩牌，后面主牌杀牌，期望主牌杀牌赢
     {
        int player0_side[] ={ 0, 12 }; // 方块A,K
        int player1_side[] = { 6, 6 }; // 方块7,7
        int player2_main[] = { 3+13, 8+13 }; // 梅花4,9 (主牌)
        int player3_main[] = { 4+13, 8+13 }; // 梅花5,9 (主牌)
        const int* cards_played7[4] = { player0_side, player1_side, player2_main, player3_main };
        int cards_count7[4] = { 2, 2, 2, 2 };
        // player3应该获胜，因为梅花5比梅花4大
        EXPECT_EQ(3, CUpgAlgorithm::DetermineWinner(cards_played7, cards_count7, COLOR_CLUB, 4, true, 0));
     }
     //测试：首家副牌对子，后面主牌对子，期望副牌对子大
     {
        int player0_side[] ={ 0, 0, 9,9, 5,5 }; // 方块A,A,10,10,6,6
        int player1_side[] = { 13, 14, 15, 16, 17,6+13,6+13 }; // 梅花2,3,4,5,7,7
        int player2_main[] = { 1+26, 2+26, 3+26, 5+26, 6+26,6+26 }; // 红桃2,3,4,5,7,7(主牌)
        int player3_other[] = { 2, 3, 2+13, 3+13, 2+26, 3+26 }; // 方块2,3,方块2,3,方块2,3(其他玩家)
        const int* cards_played7[4] = { player0_side, player1_side, player2_main, player3_other };
        int cards_count7[4] = { 6, 6, 6, 6 };
        // player0应该获胜
        EXPECT_EQ(0, CUpgAlgorithm::DetermineWinner(cards_played7, cards_count7, COLOR_HEART, 4, true, 0));
     }
}

// 测试牌的比较值
TEST(UpgAlgorithmTest, GetCardCompareValue) {
    // 普通牌
    EXPECT_LT(CUpgAlgorithm::GetCardCompareValue(0, COLOR_HEART, 1, false),  // 方块A
              CUpgAlgorithm::GetCardCompareValue(1, COLOR_HEART, 1, false)); // 方块2
    
    // 主花色牌应该比副牌大
    EXPECT_LT(CUpgAlgorithm::GetCardCompareValue(0, COLOR_HEART, 1, false),   // 方块A (副牌)
              CUpgAlgorithm::GetCardCompareValue(26, COLOR_HEART, 1, false));  // 红桃A (主牌)
    
    // 王牌应该是最大的
    EXPECT_LT(CUpgAlgorithm::GetCardCompareValue(26, COLOR_DIAMOND, 1, false),  // 红桃A (主牌)
              CUpgAlgorithm::GetCardCompareValue(52, COLOR_DIAMOND, 1, false));  // 小王
    
    EXPECT_LT(CUpgAlgorithm::GetCardCompareValue(52, COLOR_DIAMOND, 1, false),  // 小王
              CUpgAlgorithm::GetCardCompareValue(53, COLOR_DIAMOND, 1, false));  // 大王
              
    // 当前级别为2时，同花色的级牌（红桃2）应大于非级牌的主牌（如红桃3）
    EXPECT_LT(CUpgAlgorithm::GetCardCompareValue(27, COLOR_HEART, 0, false),   // 红桃2 (主牌)
              CUpgAlgorithm::GetCardCompareValue(26, COLOR_HEART, 0, false));   // 红桃A (级牌，levelCard=0表示打A)

    // 测试A的比较值：A(value=0)应该大于K(value=12)
    // 副牌：方块A > 方块K
    EXPECT_GT(CUpgAlgorithm::GetCardCompareValue(0, COLOR_HEART, 8, false),    // 方块A (card=0, value=0)
              CUpgAlgorithm::GetCardCompareValue(12, COLOR_HEART, 8, false));  // 方块K (card=12, value=12)

    // 主牌：红桃A > 红桃K（主花色为红桃，级牌为10）
    EXPECT_GT(CUpgAlgorithm::GetCardCompareValue(26, COLOR_HEART, 8, false),   // 红桃A (card=26, value=0)
              CUpgAlgorithm::GetCardCompareValue(38, COLOR_HEART, 8, false));  // 红桃K (card=38, value=12)
}

// 额外完整场景测试：顺子、王、主牌对子和拖拉机比较
// 注意：牌值映射为 value: 0=A, 1=2, 2=3, ..., 12=K
TEST(UpgAlgorithmTest, AdditionalRoundScenarios) {
    // 顺子比较：player1 有更大的顺子
    // 数组中的数字是牌序号（card），其 value = card % 13
    // {5,6,7} 对应的 value 为 {5,6,7} -> 实际牌点为 {6,7,8}
    int p0_seq[] = { 5, 6, 7 };   // 方块6,7,8
    int p1_seq[] = { 6, 7, 8 };   // 方块7,8,9 (更大)
    int p2_seq[] = { 4, 5, 6 };   // 方块5,6,7 (较小)
    int p3_seq[] = { 5, 6, 7 };   // 同 p0
    const int* seq_played[4] = { p0_seq, p1_seq, p2_seq, p3_seq };
    int seq_counts[4] = { 3, 3, 3, 3 };
    EXPECT_EQ(1, CUpgAlgorithm::DetermineWinner(seq_played, seq_counts, COLOR_DIAMOND, 8, false, 0));

    // 王比较：小王 vs 大王
    int p0_joker[] = { 52 }; // 小王
    int p1_joker[] = { 0 };  // 无效/副牌小，不能赢
    int p2_joker[] = { 53 }; // 大王 -> 胜者
    int p3_joker[] = { 1 };
    const int* joker_played[4] = { p0_joker, p1_joker, p2_joker, p3_joker };
    int joker_counts[4] = { 1, 1, 1, 1 };
    EXPECT_EQ(2, CUpgAlgorithm::DetermineWinner(joker_played, joker_counts, COLOR_HEART, 8, false, 0));

    // 主牌对子优先：首家出主牌对子，其他人出副牌更大但不是主牌
    int leader_main_pair[] = { 26, 26 }; // 红桃A, A (主花色, card%13 == 0 -> A)
    int other_side_pair[] = { 12, 12 };  // 方块K,K（副牌）
    const int* mainpair_played[4] = { leader_main_pair, other_side_pair, other_side_pair, other_side_pair };
    int mainpair_counts[4] = { 2, 2, 2, 2 };
    EXPECT_EQ(0, CUpgAlgorithm::DetermineWinner(mainpair_played, mainpair_counts, COLOR_HEART, 8, false, 0));

    // 拖拉机比较：player3 的拖拉机最大
    // 注意这里的 card 值 -> value: 0=A,1=2,2=3,... 所以需按照 value 判断连续性
    int p0_tr[] = { 1,1,2,2 };   // 方块2,2,3,3
    int p1_tr[] = { 2,2,3,3 };   // 方块3,3,4,4
    int p2_tr[] = { 0,0,1,1 };   // 方块A,A,2,2 (最小)
    int p3_tr[] = { 5,5,6,6 };   // 方块6,6,7,7 (最大)
    const int* tr_played[4] = { p0_tr, p1_tr, p2_tr, p3_tr };
    int tr_counts[4] = { 4,4,4,4 };
    EXPECT_EQ(3, CUpgAlgorithm::DetermineWinner(tr_played, tr_counts, COLOR_DIAMOND, 8, false, 0));
}

// 测试特殊情况：级牌处理
TEST(UpgAlgorithmTest, LevelCardHandling) {
    // 当A是级牌时，非主花色的两张A虽然都是主牌，但不同花色不能构成对子
    int level_2_cards[] = { 0, 13 }; // 方块A和梅花A（主花色为红桃，级牌为A，levelCard=0）
    int card_type = CUpgAlgorithm::GetCardType(level_2_cards, 2, COLOR_HEART, 0, false);
    EXPECT_EQ(CARD_TYPE_SINGLE, card_type); // 不是对子

    // 当6是级牌时，非主花色的两张6虽然都是主牌，但不同花色不能构成对子
    int level_6_cards[] = { 4, 17 }; // 方块5和梅花5（主花色为黑桃，级牌为5，levelCard=4）
    card_type = CUpgAlgorithm::GetCardType(level_6_cards, 2, COLOR_SPADE, 4, false);
    EXPECT_EQ(CARD_TYPE_SINGLE, card_type); // 不是对子

    // 同花色的级牌可以构成对子
    int same_color_level_cards[] = { 4, 4 }; // 方块5,5（主花色为黑桃，级牌为5，levelCard=4）
    card_type = CUpgAlgorithm::GetCardType(same_color_level_cards, 2, COLOR_SPADE, 4, false);
    EXPECT_EQ(CARD_TYPE_PAIR, card_type); // 是对子
}

// 测试相邻牌判断
TEST(UpgAlgorithmTest, IsAdjacent) {
    // 普通相邻副牌（级牌是10，不影响小牌）
    // value: 0=A, 1=2, 2=3, ..., 11=Q, 12=K
    EXPECT_TRUE(CUpgAlgorithm::IsAdjacent(1, 2, COLOR_HEART, 8, false)); // 方块2和方块3（级牌是10）

    // 不相邻牌
    EXPECT_FALSE(CUpgAlgorithm::IsAdjacent(1, 3, COLOR_HEART, 8, false)); // 方块2和方块4

    // 相邻的主牌
    EXPECT_TRUE(CUpgAlgorithm::IsAdjacent(27, 28, COLOR_HEART, 8, false)); // 红桃2和红桃3（级牌是10）

    // K和A相邻
    EXPECT_TRUE(CUpgAlgorithm::IsAdjacent(12, 0, COLOR_HEART, 8, false)); // 方块K和方块A

    // 级牌的特殊情况 - 不同花色的级牌相邻（比较值连续）
    EXPECT_TRUE(CUpgAlgorithm::IsAdjacent(0, 26, COLOR_HEART, 0, false)); // 方块A和红桃A（都是级牌A，比较值连续）
}

// 测试必出牌提示功能
TEST(UpgAlgorithmTest, GetMustPlayCards) {
    // 测试手牌中有主牌的情况，首家出主牌（级牌是10，避开小牌）
    int hand_cards1[] = { 0, 2, 26, 52 }; // 方块A, 方块3, 红桃A(主牌), 小王(主牌)
    int lead_cards1[] = { 38 }; // 红桃K(主牌)
    int must_play_cards1[10];
    int must_count1 = CUpgAlgorithm::GetMustPlayCards(hand_cards1, 4, lead_cards1, 1, COLOR_HEART, 8, false, must_play_cards1);

    // 应该必须出主牌（红桃A和小王）
    EXPECT_EQ(2, must_count1);
    bool has_heart_2 = false;
    bool has_small_joker = false;
    for (int i = 0; i < must_count1; i++) {
        if (must_play_cards1[i] == 26) has_heart_2 = true;
        if (must_play_cards1[i] == 52) has_small_joker = true;
    }
    EXPECT_TRUE(has_heart_2);
    EXPECT_TRUE(has_small_joker);

    // 测试手牌中有同花色副牌的情况，首家出副牌（级牌是10）
    int hand_cards2[] = { 0, 2, 38, 27 }; // 方块A, 方块3, 红桃K, 红桃2
    int lead_cards2[] = { 3 }; // 方块4
    int must_play_cards2[10];
    int must_count2 = CUpgAlgorithm::GetMustPlayCards(hand_cards2, 4, lead_cards2, 1, COLOR_HEART, 8, false, must_play_cards2);

    // 应该必须出方块牌（方块A和方块3）
    EXPECT_EQ(2, must_count2);
    bool has_diamond_2 = false;
    bool has_diamond_4 = false;
    for (int i = 0; i < must_count2; i++) {
        if (must_play_cards2[i] == 0) has_diamond_2 = true;
        if (must_play_cards2[i] == 2) has_diamond_4 = true;
    }
    EXPECT_TRUE(has_diamond_2);
    EXPECT_TRUE(has_diamond_4);
    
    // 测试没有同花色副牌但有主牌的情况（级牌是10）
    int hand_cards3[] = { 38, 27, 52 }; // 红桃K, 红桃2, 小王
    int lead_cards3[] = { 3 }; // 方块4
    int must_play_cards3[10];
    int must_count3 = CUpgAlgorithm::GetMustPlayCards(hand_cards3, 3, lead_cards3, 1, COLOR_HEART, 8, false, must_play_cards3);

    // 应该必须出主牌（红桃K, 红桃2和小王）
    EXPECT_EQ(3, must_count3);
    bool has_heart_A = false;
    bool has_heart_3 = false;
    has_small_joker = false;
    for (int i = 0; i < must_count3; i++) {
        if (must_play_cards3[i] == 38) has_heart_A = true;
        if (must_play_cards3[i] == 27) has_heart_3 = true;
        if (must_play_cards3[i] == 52) has_small_joker = true;
    }
    EXPECT_TRUE(has_heart_A);
    EXPECT_TRUE(has_heart_3);
    EXPECT_TRUE(has_small_joker);
    

    
    // 测试2为常主时的情况，手中有不同花色的2
    int hand_cards6[] = { 1, 14, 27, 40 }; // 方块2, 梅花2, 红桃2, 黑桃2（都是主牌）
    int lead_cards6[] = { 2, 2 }; // 方块3,3（对子）
    int must_play_cards6[10];
    int must_count6 = CUpgAlgorithm::GetMustPlayCards(hand_cards6, 4, lead_cards6, 2, COLOR_HEART, 8, true, must_play_cards6);
    
    // 应该必须出所有主牌2（不同花色的2都是主牌）
    EXPECT_EQ(4, must_count6);
    
    // 测试首家出单张，但手中既有同花色又有主牌的情况
    int hand_cards7[] = { 1, 2, 27, 28 }; // 方块2,3,红桃2,3
    int lead_cards7[] = { 4 }; // 方块5
    int must_play_cards7[10];
    int must_count7 = CUpgAlgorithm::GetMustPlayCards(hand_cards7, 4, lead_cards7, 1, COLOR_HEART, 8, false, must_play_cards7);
    
    // 应该必须出同花色牌（方块2,3）
    EXPECT_EQ(2, must_count7);
    bool has_diamond_cards = false;
    for (int i = 0; i < must_count7; i++) {
        if (must_play_cards7[i] == 1 || must_play_cards7[i] == 2) has_diamond_cards = true;
    }
    EXPECT_TRUE(has_diamond_cards);
    
    // 测试首家出对子，但手中有主牌对子和副牌对子的情况
    int hand_cards8[] = { 1, 1, 27, 27 }; // 方块2,2,红桃2,2
    int lead_cards8[] = { 3, 3 }; // 方块4,4（对子）
    int must_play_cards8[10];
    int must_count8 = CUpgAlgorithm::GetMustPlayCards(hand_cards8, 4, lead_cards8, 2, COLOR_HEART, 8, false, must_play_cards8);
    
    // 应该必须出同花色对子（方块2,2）
    EXPECT_EQ(2, must_count8);
    bool has_same_suit_pair = false;
    for (int i = 0; i < must_count8; i++) {
        if (must_play_cards8[i] == 1) has_same_suit_pair = true;
    }
    EXPECT_TRUE(has_same_suit_pair);
}

// 测试甩牌失败检测
TEST(UpgAlgorithmTest, CheckThrowFailed) {
    // 测试1: 甩牌成功 - 单张最大
    // value: 0=A, 1=2, 2=3, ..., 11=Q, 12=K
    // 首家出方块Q,K,A（单张），其他家没有更大的方块
    int play_cards1[] = { 11, 12, 0 }; // 方块Q,K,A
    const int other_hands1[3][25] = {
         { 1 }, // 方块2
         { 14 },// 梅花2
        { 26 } // 红桃A
         };
    int other_counts1[3] = { 1, 1, 1 };
    int failed_cards1[26];

    int result1 = CUpgAlgorithm::CheckThrowFailed(play_cards1, 3, other_hands1, other_counts1,
                                                   COLOR_HEART, 8, false, failed_cards1);
    EXPECT_EQ(0, result1); // 甩牌成功

    // 测试2: 甩牌失败 - 单张被大过
    // 首家出方块2,3,4（单张），其他家有方块A
    int play_cards2[] = { 1, 2, 3 }; // 方块2,3,4
    const int other_hands2[3][25] = {
        { 0 }, // 方块A（比较值最大）
        { 14 }, // 梅花2
        { 26 }, // 红桃A
         };
    int other_counts2[3] = { 1, 1, 1 };
    int failed_cards2[26];

    int result2 = CUpgAlgorithm::CheckThrowFailed(play_cards2, 3, other_hands2, other_counts2,
                                                   COLOR_HEART, 8, false, failed_cards2);
    EXPECT_EQ(1, result2); // 甩牌失败，返回1张牌
    EXPECT_EQ(1, failed_cards2[0]); // 最小的单张是方块2

    // 测试3: 甩牌成功 - 对子最大
    // 首家出方块Q,Q,K,K（两对），其他家没有更大的连对
    int play_cards3[25] = { 11, 11, 12, 12 }; // 方块Q,Q,K,K
    const int other_hands3[3][25] = {
        { 1, 1 }, // 方块2,2
        { 14, 15 }, // 梅花2,3
        { 26 },// 红桃A
        };
    int other_counts3[3] = { 2, 2, 1 };
    int failed_cards3[26];

    int result3 = CUpgAlgorithm::CheckThrowFailed(play_cards3, 4, other_hands3, other_counts3,
                                                   COLOR_HEART, 8, false, failed_cards3);
    EXPECT_EQ(0, result3); // 甩牌成功

    // 测试4: 甩牌失败 - 混合牌型（对子+单张），对子被大过
    // 首家出方块3,3,4,4,5（两对+一单），其他家有方块6,6,7,7
    int play_cards4[] = { 1, 1, 2, 2, 3 }; // 方块2,2,3,3,4
    const int other_hands4[3][25] = {
        { 4, 4, 5, 5 }, // 方块5,5,6,6（更大的连对）
{ 14 }, // 梅花4
{ 26 }, // 红桃A
 };
    int other_counts4[3] = { 4, 1, 1 };
    int failed_cards4[26];

    int result4 = CUpgAlgorithm::CheckThrowFailed(play_cards4, 5, other_hands4, other_counts4,
                                                   COLOR_HEART, 8, false, failed_cards4);
    EXPECT_EQ(4, result4); // 甩牌失败，返回4张牌（两对）
    // 验证返回的是最小的连对（从大到小排序，每个重复两次）
    // tractorCards = {2, 1}（从大到小），返回 {2, 2, 1, 1}
    EXPECT_EQ(2, failed_cards4[0]); // 方块3
    EXPECT_EQ(2, failed_cards4[1]); // 方块3
    EXPECT_EQ(1, failed_cards4[2]); // 方块2
    EXPECT_EQ(1, failed_cards4[3]); // 方块2

    // 测试5: 甩牌成功 - 纯拖拉机不检查
    // 首家出方块3,3,4,4（纯拖拉机），即使其他家有更大的也不算失败
    int play_cards5[] = { 1, 1, 2, 2 }; // 方块2,2,3,3
    const int other_hands5[3][25] = { 
        { 10, 10, 11, 11 }, // 方块J,J,Q,Q
        { 14 }, // 梅花2
        { 26 }, // 红桃A
    };
    int other_counts5[3] = { 4, 1, 1 };
    int failed_cards5[26];

    int result5 = CUpgAlgorithm::CheckThrowFailed(play_cards5, 4, other_hands5, other_counts5,
                                                   COLOR_HEART, 8, false, failed_cards5);
    EXPECT_EQ(0, result5); // 纯拖拉机不检查，甩牌成功

    // 测试6: 甩牌失败 - 混合牌型（对子+单张）
    // 首家出方块3,3,4（一对+一单），其他家有更大的单张
    int play_cards6[] = { 1, 1, 2 }; // 方块2,2,3
    const int other_hands6[3][25] = { 
        { 12 }, // 方块K（更大的单张）
{ 14 }, // 梅花2
{ 26 } // 红桃A
 };
    int other_counts6[3] = { 1, 1, 1 };
    int failed_cards6[26];

    int result6 = CUpgAlgorithm::CheckThrowFailed(play_cards6, 3, other_hands6, other_counts6,
                                                   COLOR_HEART, 8, false, failed_cards6);
    EXPECT_EQ(1, result6); // 甩牌失败，返回最小的单张
    EXPECT_EQ(2, failed_cards6[0]); // 最小的单张是方块3

    // 测试7: BUG重现 - AAK甩牌应该成功
    // value: 0=A, 1=2, ..., 12=K
    // 首家出方块A,A,K（两个A和一个K），其他家没有更大的方块
    int play_cards7[] = { 51, 39, 39 }; // 黑桃K,黑桃K,黑桃K (实际上应该是方块A,A,K)
    const int other_hands7[3][25] = {
        { 1, 2 }, // 方块2,3
        { 14 }, // 梅花2
        { 26 } // 红桃A
    };
    int other_counts7[3] = { 2, 1, 1 };
    int failed_cards7[26];

    int result7 = CUpgAlgorithm::CheckThrowFailed(play_cards7, 3, other_hands7, other_counts7,
                                                   COLOR_HEART, 8, false, failed_cards7);
    EXPECT_EQ(0, result7); // AAK是最大的，甩牌应该成功

    int result7_2 = CUpgAlgorithm::ValidateFirstPlay(play_cards7, 3, COLOR_HEART, 8, true);
    EXPECT_EQ(PUT_CARD_RESULT_SUCCESS, result7_2); // AAK是最大的，甩牌应该成功

    // 测试8: 甩牌失败 - 混合牌型（对子+单张），对子被大过
    // 首家出方块4,4,A（一对+一单），其他玩家有更大的对子方块6,6
    int play_cards8[] = { 10, 10, 0 }; // 方块4,4,A (value: 3=4, 0=A)
    const int other_hands8[3][25] = {
        { 11, 11 }, // 方块6,6（更大的对子）
        { 3+13 },   // 梅花4
        { 0+26 }    // 红桃A
    };
    int other_counts8[3] = { 2, 1, 1 };
    int failed_cards8[26];

    int result8 = CUpgAlgorithm::CheckThrowFailed(play_cards8, 3, other_hands8, other_counts8,
                                                  COLOR_HEART, 8, false, failed_cards8);
    EXPECT_EQ(2, result8); // 甩牌失败，返回2张牌（对子）
    EXPECT_EQ(10, failed_cards8[0]); // 方块4
    EXPECT_EQ(10, failed_cards8[1]); // 方块4

    {
    int play_cards8[] = { 10, 10, 12 }; // 方块J,J,K 
    const int other_hands8[3][25] = {
        { 11, 12 , 0}, // 方块Q,K, A（更大的单张）
        { 3+13 },   // 梅花4
        { 0+26 }    // 红桃A
    };
    int other_counts8[3] = { 3, 1, 1 };
    int failed_cards8[26];

    int result8 = CUpgAlgorithm::CheckThrowFailed(play_cards8, 3, other_hands8, other_counts8,
                                                  COLOR_HEART, 8, false, failed_cards8);
    EXPECT_EQ(1, result8); // 甩牌失败，返回1张牌（单张）
    EXPECT_EQ(12, failed_cards8[0]); // 方块K
    }

    {
        //test
        int play_cards8[] = { 4, 4, 52, 52 }; // 方块J,J,K
        const int other_hands8[3][25] = {
            { 53, 53, 0 }, // 方块Q,K, A（更大的单张）
            { 3 + 13 },    // 梅花4
            { 0 + 26 }     // 红桃A
        };
        int other_counts8[3] = { 3, 1, 1 };
        int failed_cards8[26];

        int result8 = CUpgAlgorithm::CheckThrowFailed(play_cards8, 4, other_hands8, other_counts8, COLOR_DIAMOND, 4, false, failed_cards8);
        EXPECT_EQ(2, result8);           // 甩牌失败，返回1张牌（单张）
        EXPECT_EQ(4, failed_cards8[0]); // 方块K
        EXPECT_EQ(4, failed_cards8[1]);  // 方块K
    }
}

// 测试2为常主的处理
TEST(UpgAlgorithmTest, Const2AsMain) {
    // 测试2为常主时，所有花色的2都是主牌
    int diamond_2 = 1;  // 方块2 (0*13 + 1)
    int club_2 = 14;    // 梅花2 (1*13 + 1)
    int heart_2 = 27;   // 红桃2 (2*13 + 1)
    int spade_2 = 40;   // 黑桃2 (3*13 + 1)

    // 主花色是红桃，级牌是10（value=8），2为常主
    EXPECT_GE(CUpgAlgorithm::GetMainValue(diamond_2, COLOR_HEART, 8, true), 0);
    EXPECT_GE(CUpgAlgorithm::GetMainValue(club_2, COLOR_HEART, 8, true), 0);
    EXPECT_GE(CUpgAlgorithm::GetMainValue(heart_2, COLOR_HEART, 8, true), 0);
    EXPECT_GE(CUpgAlgorithm::GetMainValue(spade_2, COLOR_HEART, 8, true), 0);

    // 测试比较值：大王 > 小王 > 级牌 > 常主2 > 其他主牌
    int big_king = 53;
    int small_king = 52;
    int main_level = 9+26;  // 红桃10（主花色级牌）
    int side_level = 9;   // 方块10（副花色级牌）
    int main_2 = heart_2; // 红桃2（主花色常主2）
    int side_2 = diamond_2; // 方块2（副花色常主2）
    int normal_main = 28; // 红桃3（普通主牌）
    int normal_main_A= 0+26; // 方块A（普通主牌）
    int val_big_king = CUpgAlgorithm::GetCardCompareValue(big_king, COLOR_HEART, 9, true);
    int val_small_king = CUpgAlgorithm::GetCardCompareValue(small_king, COLOR_HEART, 9, true);
    int val_main_level = CUpgAlgorithm::GetCardCompareValue(main_level, COLOR_HEART, 9, true);
    int val_side_level = CUpgAlgorithm::GetCardCompareValue(side_level, COLOR_HEART, 9, true);
    int val_main_2 = CUpgAlgorithm::GetCardCompareValue(main_2, COLOR_HEART, 9, true);
    int val_side_2 = CUpgAlgorithm::GetCardCompareValue(side_2, COLOR_HEART, 9, true);
    int val_normal_main = CUpgAlgorithm::GetCardCompareValue(normal_main, COLOR_HEART, 9, true);
    int val_normal_main_A = CUpgAlgorithm::GetCardCompareValue(normal_main_A, COLOR_HEART, 9, true);

    // 验证大小关系
    EXPECT_GT(val_big_king, val_small_king);
    EXPECT_GT(val_small_king, val_main_level);
    EXPECT_GT(val_main_level, val_side_level);
    EXPECT_GT(val_side_level, val_main_2);
    EXPECT_GT(val_main_2, val_side_2);
    EXPECT_GT(val_side_2, val_normal_main);
    EXPECT_GT(val_side_2, val_normal_main_A);

    EXPECT_TRUE(CUpgAlgorithm::IsAdjacent(side_2, normal_main_A, COLOR_HEART, 9, true));
    EXPECT_TRUE(CUpgAlgorithm::IsAdjacent(main_2, side_2, COLOR_HEART, 9, true));
    EXPECT_TRUE(CUpgAlgorithm::IsAdjacent(side_level, main_2, COLOR_HEART, 9, true));
    EXPECT_TRUE(CUpgAlgorithm::IsAdjacent(main_level, side_level, COLOR_HEART, 9, true));
    // 测试2为常主时，不同花色的2不相邻（用于拖拉机判断）
    EXPECT_FALSE(CUpgAlgorithm::IsAdjacent(heart_2, diamond_2, COLOR_CLUB, 9, true));

    // 测试级牌是2时，2为常主的逻辑（2既是级牌又是常主）
    // 此时2应该按级牌处理，而不是常主
    int val_2_as_level = CUpgAlgorithm::GetCardCompareValue(heart_2, COLOR_HEART, 1, true);
    int val_2_as_const = CUpgAlgorithm::GetCardCompareValue(heart_2, COLOR_HEART, 8, true);
    // 作为级牌时应该和作为常主时相等
    EXPECT_EQ(val_2_as_level, val_2_as_const);

    // 测试2为常主时，不同花色的2虽然都是主牌，但不能组成对子
    int const2_pair[] = { diamond_2, club_2 }; // 方块2和梅花2
    int card_type = CUpgAlgorithm::GetCardType(const2_pair, 2, COLOR_HEART, 8, true);
    EXPECT_EQ(CARD_TYPE_SINGLE, card_type); // 不是对子

    // 测试2为常主时，同花色的2可以组成对子
    int same_color_const2_pair[] = { diamond_2, diamond_2 }; // 方块2,2
    card_type = CUpgAlgorithm::GetCardType(same_color_const2_pair, 2, COLOR_HEART, 8, true);
    EXPECT_EQ(CARD_TYPE_PAIR, card_type); // 是对子

    // 测试2为常主时，不同花色的2不能组成拖拉机
    int const2_not_tractor[] = { diamond_2, diamond_2, club_2, club_2 }; // 方块2,2,梅花2,2
    card_type = CUpgAlgorithm::GetCardType(const2_not_tractor, 4, COLOR_HEART, 8, true);
    EXPECT_EQ(CARD_TYPE_PAIR, card_type); // 对子

    card_type = CUpgAlgorithm::GetCardType(const2_not_tractor, 4, COLOR_CLUB, 8, true);
    EXPECT_EQ(CARD_TYPE_TRACTOR, card_type); // 拖拉机,正副2组成拖拉机

    // 测试普通主牌可以组成拖拉机（红桃3,3,4,4）
    int normal_tractor[] = { 28, 28, 29, 29 }; // 红桃3,3,4,4
    card_type = CUpgAlgorithm::GetCardType(normal_tractor, 4, COLOR_HEART, 8, true);
    EXPECT_EQ(CARD_TYPE_TRACTOR, card_type);

    // 测试2为常主时，2不是副牌
    int diamond_3 = 2; // 方块3
    EXPECT_LT(CUpgAlgorithm::GetMainValue(diamond_3, COLOR_HEART, 8, true), 0); // 方块3是副牌
    EXPECT_GE(CUpgAlgorithm::GetMainValue(diamond_2, COLOR_HEART, 8, true), 0); // 方块2是主牌
}

// 更多 DetermineWinner 测试用例
TEST(UpgAlgorithmTest, MoreDetermineWinnerTests) {
    // 测试：首家出主牌单张，其他玩家出副牌对子
    int player0_main_single[] = { 26 }; // 红桃A (主牌)
    int player1_side_pair[] = { 0, 0 }; // 方块A,A (副牌对子)
    int player2_side_single[] = { 1 }; // 方块2 (副牌单张)
    int player3_side_single2[] = { 2 }; // 方块3 (副牌单张)
    const int* cards_played[4] = { player0_main_single, player1_side_pair, player2_side_single, player3_side_single2 };
    int cards_count[4] = { 1, 2, 1, 1 };

    // 主牌应该获胜
    EXPECT_EQ(0, CUpgAlgorithm::DetermineWinner(cards_played, cards_count, COLOR_HEART, 8, false, 0));

    // 测试：首家出拖拉机，其他玩家出单张和对子（混合牌型）
    int player0_tractor[] = { 1, 1, 2, 2 }; // 方块2,2,3,3 (拖拉机)
    int player1_pair[] = { 3, 3 }; // 方块4,4 (对子)
    int player2_single[] = { 4 }; // 方块5 (单张)
    int player3_pair2[] = { 0, 0 }; // 方块A,A (对子)
    const int* cards_played3[4] = { player0_tractor, player1_pair, player2_single, player3_pair2 };
    int cards_count3[4] = { 4, 2, 1, 2 };

    // 首家应该获胜（拖拉机牌型且最大）
    EXPECT_EQ(0, CUpgAlgorithm::DetermineWinner(cards_played3, cards_count3, COLOR_HEART, 8, false, 0));

    // 测试：首家出小主牌对子，其他玩家出大主牌对子
    int player0_main_pair_small[] = { 27, 27 }; // 红桃2,2 (主牌对子)
    int player1_main_pair_large[] = { 28, 28 }; // 红桃3,3 (主牌对子，更大)
    int player2_side_pair[] = { 1, 1 }; // 方块2,2 (副牌对子)
    int player3_side_pair2[] = { 2, 2 }; // 方块3,3 (副牌对子)
    const int* cards_played4[4] = { player0_main_pair_small, player1_main_pair_large, player2_side_pair, player3_side_pair2 };
    int cards_count4[4] = { 2, 2, 2, 2 };

    // player1应该获胜（主牌对子最大）
    EXPECT_EQ(1, CUpgAlgorithm::DetermineWinner(cards_played4, cards_count4, COLOR_HEART, 8, false, 0));

    // 测试：首家出主牌拖拉机，其他玩家出副牌拖拉机
    int player0_main_tractor[] = { 27, 27, 28, 28 }; // 红桃2,2,3,3 (主牌拖拉机)
    int player1_side_tractor[] = { 1, 1, 2, 2 }; // 方块2,2,3,3 (副牌拖拉机)
    int player2_main_single[] = { 26 }; // 红桃A (主牌单张)
    int player3_side_single[] = { 0 }; // 方块A (副牌单张)
    const int* cards_played5[4] = { player0_main_tractor, player1_side_tractor, player2_main_single, player3_side_single };
    int cards_count5[4] = { 4, 4, 1, 1 };

    // 首家应该获胜（主牌拖拉机最大）
    EXPECT_EQ(0, CUpgAlgorithm::DetermineWinner(cards_played5, cards_count5, COLOR_HEART, 8, false, 0));

    // 测试：首家出王，其他玩家出主牌和副牌
    int player0_joker[] = { 53 }; // 大王
    int player1_small_joker[] = { 52 }; // 小王
    int player2_main_pair[] = { 28, 28 }; // 红桃3,3 (主牌对子)
    int player3_side_tractor[] = { 1, 1, 2, 2 }; // 方块2,2,3,3 (副牌拖拉机)
    const int* cards_played7[4] = { player0_joker, player1_small_joker, player2_main_pair, player3_side_tractor };
    int cards_count7[4] = { 1, 1, 2, 4 };

    // 首家应该获胜（大王最大）
    EXPECT_EQ(0, CUpgAlgorithm::DetermineWinner(cards_played7, cards_count7, COLOR_HEART, 8, false, 0));

    // 测试：首家出小王，其他玩家出大王
    int player0_small_joker2[] = { 52 }; // 小王
    int player1_big_joker[] = { 53 }; // 大王
    int player2_main_single2[] = { 26 }; // 红桃A (主牌)
    int player3_side_single5[] = { 0 }; // 方块A (副牌)
    const int* cards_played8[4] = { player0_small_joker2, player1_big_joker, player2_main_single2, player3_side_single5 };
    int cards_count8[4] = { 1, 1, 1, 1 };

    // player1应该获胜（大王比小王大）
    EXPECT_EQ(1, CUpgAlgorithm::DetermineWinner(cards_played8, cards_count8, COLOR_HEART, 8, false, 0));
    
    // 测试：混合牌型（对子+单张），首家被主牌单张杀牌
    // 首家甩牌：方块2,2,3 （一对+一单）
    // player1 杀牌：红桃4 (主牌单张)
    // 其他玩家垫牌
    int player0_mixed1[] = { 1, 1, 2 }; // 方块2,2,3 (一对+一单)
    int player1_kill1[] = { 2+26,2+26, 8 }; // 红桃3+3 (主牌)+级牌9
    int player2_filler1[] = { 12, 13, 15 }; // 方块K (垫牌)
    int player3_filler2[] = { 11, 5, 8 }; // 方块Q (垫牌)
    const int* cards_played9[4] = { player0_mixed1, player1_kill1, player2_filler1, player3_filler2 };
    int cards_count9[4] = { 3, 3, 3, 3 }; // 四家出牌数量应该一样多
    
    // player1应该获胜（主牌杀牌）
    EXPECT_EQ(1, CUpgAlgorithm::DetermineWinner(cards_played9, cards_count9, COLOR_HEART, 8, false, 0));
    
    // 测试：混合牌型（对子+单张），首家被更大对子杀牌
    // 首家甩牌：方块2,2,3 （一对+一单）
    // player1 杀牌：方块6,6 (更大的对子)
    // 其他玩家垫牌
    int player0_mixed2[] = { 1, 1, 2 }; // 方块2,2,3 (一对+一单)
    int player1_kill2[] = { 5, 5, 9 }; // 方块6,6，10
    int player2_filler3[] = { 12, 12, 15 }; // 方块K,K+ 其它花色
    int player3_filler4[] = { 11, 11, 18 }; // 方块Q,Q+其它花色
    const int* cards_played10[4] = { player0_mixed2, player1_kill2, player2_filler3, player3_filler4 };
    int cards_count10[4] = { 3, 3, 3, 3 }; // 四家出牌数量应该一样多
    
    // player1应该获胜（更大的对子杀牌）
    EXPECT_EQ(1, CUpgAlgorithm::DetermineWinner(cards_played10, cards_count10, COLOR_HEART, 8, false, 0));

    
    // 测试：混合牌型（对子+单张），首家被拖拉机杀牌
    // 首家甩牌：方块2,2,3 （一对+一单）
    // player1 杀牌：方块6,6,7,7 (副牌拖拉机)
    // 其他玩家垫牌
    int player0_mixed4[] = { 1, 1, 2 }; // 方块2,2,3 (一对+一单)
    int player1_kill4[] = { 5, 5, 6, 6 }; // 方块6,6,7,7 (副牌拖拉机)
    int player2_filler7[] = { 12, 12, 12 }; // 方块K,K,K (垫牌)
    int player3_filler8[] = { 11, 11, 11 }; // 方块Q,Q,Q (垫牌)
    const int* cards_played12[4] = { player0_mixed4, player1_kill4, player2_filler7, player3_filler8 };
    int cards_count12[4] = { 3, 4, 3, 3 }; // 四家出牌数量不一致，这种情况不应该出现，需要重新设计测试用例
    
    // 重新设计测试：混合牌型（对子+单张），首家被拖拉机杀牌（确保出牌数量一致）
    // 首家甩牌：方块2,2,3,3 （两对）
    // player1 杀牌：方块6,6,7,7 (副牌拖拉机)
    // 其他玩家垫牌
    int player0_mixed5[] = { 1, 1, 2, 2 }; // 方块2,2,3,3 (两对)
    int player1_kill5[] = { 5, 5, 6, 6 }; // 方块6,6,7,7 (副牌拖拉机)
    int player2_filler9[] = { 12, 12, 12, 12 }; // 方块K,K,K,K (垫牌)
    int player3_filler10[] = { 11, 11, 11, 11 }; // 方块Q,Q,Q,Q (垫牌)
    const int* cards_played13[4] = { player0_mixed5, player1_kill5, player2_filler9, player3_filler10 };
    int cards_count13[4] = { 4, 4, 4, 4 }; // 四家出牌数量应该一样多
    
    // player1应该获胜（拖拉机杀牌）
    EXPECT_EQ(1, CUpgAlgorithm::DetermineWinner(cards_played13, cards_count13, COLOR_HEART, 8, false, 0));
    
    // 测试：混合牌型（对子+单张），首家被主牌拖拉机杀牌
    // 首家甩牌：方块2,2,3,3 （两对）
    // player1 杀牌：红桃4,4,5,5 (主牌拖拉机)
    // 其他玩家垫牌
    int player0_mixed6[] = { 1, 1, 2, 2 }; // 方块2,2,3,3 (两对)
    int player1_kill6[] = { 28, 28, 29, 29 }; // 红桃3,3,4,4 (主牌拖拉机)
    int player2_filler11[] = { 12, 12, 12, 12 }; // 方块K,K,K,K (垫牌)
    int player3_filler12[] = { 11, 11, 11, 11 }; // 方块Q,Q,Q,Q (垫牌)
    const int* cards_played14[4] = { player0_mixed6, player1_kill6, player2_filler11, player3_filler12 };
    int cards_count14[4] = { 4, 4, 4, 4 }; // 四家出牌数量应该一样多
    
    // player1应该获胜（主牌拖拉机杀牌）
    EXPECT_EQ(1, CUpgAlgorithm::DetermineWinner(cards_played14, cards_count14, COLOR_HEART, 8, false, 0));
}

// 首家甩牌的更多场景：成功与失败（强制出最小牌）
TEST(UpgAlgorithmTest, ShuaiLeaderScenarios) {
    // 场景1：甩牌成功（单张）——其他家没有更大的单张
    int play1[] = { 10, 11, 9 }; // 方块J,Q,10 (三张单牌，均为副牌)
    const int other1[3][25] = {
        { 1 }, { 2 }, { 3 }
    };
    const int other1_count[3] = { 1, 1, 1 };
    int failed1[26];
    int r1 = CUpgAlgorithm::CheckThrowFailed(play1, 3, other1, other1_count, COLOR_DIAMOND, 8, false, failed1);
    EXPECT_EQ(0, r1);

    // 场景2：甩牌失败（多个单张），强制出最小单张
    int play2[] = { 2, 3 }; // 方块3,4（两张单张）
    const int other2[3][25] = {
        { 12 }, // 对手有方块K，可以压过
        { },
        { }
    };
    const int other2_count[3] = { 1, 0, 0 };
    int failed2[26];
    int r2 = CUpgAlgorithm::CheckThrowFailed(play2, 2, other2, other2_count, COLOR_DIAMOND, 8, false, failed2);
    EXPECT_EQ(1, r2);
    // 最小单张应为 play2 中的较小者（card 2）
    EXPECT_EQ(2, failed2[0]);

    // 场景3：甩牌失败（纯对子但不是拖拉机），返回最小对子
    int play3[] = { 3, 3, 5, 5 }; // 两对：方块4,4 和 方块6,6
    const int other3[3][25] = {
        { 6, 6 }, // 有更大的对子方块7,7
        { },
        { }
    };
    const int other3_count[3] = { 2, 0, 0 };
    int failed3[26];
    int r3 = CUpgAlgorithm::CheckThrowFailed(play3, 4, other3, other3_count, COLOR_DIAMOND, 8, false, failed3);
    EXPECT_EQ(2, r3);
    EXPECT_EQ(3, failed3[0]);
    EXPECT_EQ(3, failed3[1]);

    // 场景4：甩牌失败（含对子且存在最小拖拉机），其他家有更大的拖拉机
    int play4[] = { 1,1,2,2,7 }; // 含对子 1,2 以及一张单张
    const int other4[3][25] = {
        { 3,3,4,4 }, // 对家有连续对子 3,3,4,4 可构成更大的拖拉机
        { },
        { }
    };
    const int other4_count[3] = { 4, 0, 0 };
    int failed4[26];
    int r4 = CUpgAlgorithm::CheckThrowFailed(play4, 5, other4, other4_count, COLOR_DIAMOND, 8, false, failed4);
    // 只要返回大于0，且返回的数量为 tractorLen*2（这里预期为2*2=4）
    EXPECT_GT(r4, 0);
    EXPECT_EQ(4, r4);
}