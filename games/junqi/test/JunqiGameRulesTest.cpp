#include <gtest/gtest.h>
#include <windows.h>
#include "JunqiMap.h"
#include "JunqiLayout.h"

// 测试棋子免死（指挥部和旗子）
TEST(JunqiGameRulesTest, ImmunePieces)
{
    CJunQiLayout layout;
    
    POINT ptCell1 = {8, 8};
    POINT ptCell2 = {8, 9};
    
    // 放置军旗和敌军
    layout.SetCellChessman(ptCell1, OR_FLAG, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_CHIEF, SIDE_TOP, TRUE);
    
    // 军旗应该被吃掉（而不是免死）
    int nResult = layout.GetCmpResult(ptCell2, ptCell1);
    EXPECT_EQ(PUT_EAT, nResult);
}

// 测试地雷规则
TEST(JunqiGameRulesTest, MineRules)
{
    CJunQiLayout layout;
    
    // 案例1: 非工兵踩地雷
    POINT ptCell1 = {8, 8};
    POINT ptCell2 = {8, 9};
    
    layout.SetCellChessman(ptCell1, OR_ARMY, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_MINE, SIDE_TOP, TRUE);
    
    int nResult = layout.GetCmpResult(ptCell1, ptCell2);
    EXPECT_EQ(PUT_BEEAT, nResult);  // 被吃
    
    // 案例2: 工兵踩地雷
    layout.SetCellChessman(ptCell1, OR_SAPPER, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_MINE, SIDE_TOP, TRUE);
    
    nResult = layout.GetCmpResult(ptCell1, ptCell2);
    EXPECT_EQ(PUT_EAT, nResult);  // 吃掉
    
    // 案例3: 地雷遇到司令
    layout.SetCellChessman(ptCell1, OR_MINE, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_CHIEF, SIDE_TOP, TRUE);
    
    nResult = layout.GetCmpResult(ptCell1, ptCell2);
    EXPECT_EQ(PUT_BEEAT, nResult);  // 被吃
}

// 测试炸弹规则
TEST(JunqiGameRulesTest, BombRules)
{
    CJunQiLayout layout;
    layout.SetProperty(4, 1, 0);  // 启用炸弹满天飞
    
    POINT ptCell1 = {8, 8};
    POINT ptCell2 = {8, 9};
    
    // 炸弹 vs 司令
    layout.SetCellChessman(ptCell1, OR_BOMB, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_CHIEF, SIDE_TOP, TRUE);
    
    int nResult = layout.GetCmpResult(ptCell1, ptCell2);
    EXPECT_EQ(PUT_BOMB, nResult);  // 同归于尽
    
    // 炸弹 vs 工兵
    layout.SetCellChessman(ptCell1, OR_BOMB, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_SAPPER, SIDE_TOP, TRUE);
    
    nResult = layout.GetCmpResult(ptCell1, ptCell2);
    EXPECT_EQ(PUT_BOMB, nResult);  // 同归于尽
    
    // 炸弹 vs 地雷
    layout.SetCellChessman(ptCell1, OR_BOMB, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_MINE, SIDE_TOP, TRUE);
    
    nResult = layout.GetCmpResult(ptCell1, ptCell2);
    EXPECT_EQ(PUT_BOMB, nResult);  // 同归于尽
}

// 测试军衔比较完整规则
TEST(JunqiGameRulesTest, RankComparison)
{
    CJunQiLayout layout;
    
    // 完整的军衔顺序测试
    std::vector<int> rankOrder = {
        OR_CHIEF,
        OR_ARMY,
        OR_DIVISION,
        OR_BRIGADIER,
        OR_COLONEL,
        OR_BATTALION,
        OR_COMPANY,
        OR_PLATOON
    };
    
    // 从强到弱，每个应该比下一个强
    for (int i = 0; i < (int)rankOrder.size() - 1; i++)
    {
        int result = layout.ChessmanCompare(rankOrder[i], rankOrder[i + 1]);
        EXPECT_GT(result, 0);  // 强棋子应该返回正值
    }
    
    // 反向比较
    for (int i = 0; i < (int)rankOrder.size() - 1; i++)
    {
        int result = layout.ChessmanCompare(rankOrder[i + 1], rankOrder[i]);
        EXPECT_LT(result, 0);  // 弱棋子应该返回负值
    }
}

// 测试工兵排名
TEST(JunqiGameRulesTest, SapperRank)
{
    CJunQiLayout layout;
    
    // 工兵应该比士兵弱
    EXPECT_LT(layout.ChessmanCompare(OR_SAPPER, OR_PLATOON), 0);
    
    // 但工兵能吃地雷
    POINT ptCell1 = {8, 8};
    POINT ptCell2 = {8, 9};
    
    layout.SetCellChessman(ptCell1, OR_SAPPER, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_MINE, SIDE_TOP, TRUE);
    
    int nResult = layout.GetCmpResult(ptCell1, ptCell2);
    EXPECT_EQ(PUT_EAT, nResult);
}

// 测试司令无人能敌（除了炸弹）
TEST(JunqiGameRulesTest, ChiefUnbeatable)
{
    CJunQiLayout layout;
    
    // 司令 vs 任何棋子
    for (int rank = OR_ARMY; rank < OR_SAPPER; rank++)
    {
        if (rank != OR_FLAG && rank != OR_CHIEF)
        {
            EXPECT_GT(layout.ChessmanCompare(OR_CHIEF, rank), 0);
            EXPECT_LT(layout.ChessmanCompare(rank, OR_CHIEF), 0);
        }
    }
}

// 测试相同军衔
TEST(JunqiGameRulesTest, SameRank)
{
    CJunQiLayout layout;
    
    // 相同军衔应该打平
    EXPECT_EQ(layout.ChessmanCompare(OR_ARMY, OR_ARMY), 0);
    EXPECT_EQ(layout.ChessmanCompare(OR_PLATOON, OR_PLATOON), 0);
    EXPECT_EQ(layout.ChessmanCompare(OR_CHIEF, OR_CHIEF), 0);
}

// 测试军旗免战
TEST(JunqiGameRulesTest, FlagImmune)
{
    CJunQiLayout layout;
    
    // 军旗被任何棋子攻击时不应该进行比较
    int nResult = layout.ChessmanCompare(OR_FLAG, OR_ARMY);
    EXPECT_EQ(nResult, 0);
    
    nResult = layout.ChessmanCompare(OR_ARMY, OR_FLAG);
    EXPECT_EQ(nResult, 0);
}

// 测试棋子免显（隐藏）
TEST(JunqiGameRulesTest, HiddenPieces)
{
    CJunQiLayout layout;
    
    POINT ptCell = {8, 8};
    layout.SetCellChessman(ptCell, OR_ARMY, SIDE_BOTTOM, TRUE);
    
    // 初始状态显示
    EXPECT_TRUE(layout.IsChessShow(ptCell));
    
    // 隐藏
    layout.ConvertChess(ptCell);
    EXPECT_FALSE(layout.IsChessShow(ptCell));
    
    // 隐藏状态下获取棋子排名仍然应该工作
    int nRank = layout.GetCellChessOffRank(ptCell);
    EXPECT_EQ(nRank, OR_ARMY);
}

// 测试棋子自由度
TEST(JunqiGameRulesTest, ChessmanFreedom)
{
    CJunQiLayout layout;
    
    POINT ptCell = {8, 8};
    layout.SetCellChessman(ptCell, OR_CHIEF, SIDE_BOTTOM, TRUE);
    
    // 初始状态下棋子应该是自由的
    bool bFree = layout.IsChessmanFree(ptCell);
    EXPECT_TRUE(bFree);
    
    // 放置被吃的标记后（如果有这样的机制）
    // 棋子可能不再自由
}

// 测试游戏结束条件：军旗被捕
TEST(JunqiGameRulesTest, GameEndCondition_FlagCaptured)
{
    CJunQiLayout layout;
    
    POINT ptFlag = {0, 0};
    POINT ptAttacker = {0, 1};
    
    layout.SetCellChessman(ptFlag, OR_FLAG, SIDE_TOP, TRUE);
    layout.SetCellChessman(ptAttacker, OR_ARMY, SIDE_BOTTOM, TRUE);
    
    // 攻击军旗
    int nResult = layout.MoveChessmanServer(ptAttacker, ptFlag);
    
    // 应该导致游戏结束（军旗被吃掉）
    EXPECT_TRUE((nResult & 0xFF) == PUT_EAT);
    
    // 检查是否返回游戏结束信号
    bool bLose = layout.IsPlayerLost(SIDE_TOP);
    EXPECT_TRUE(bLose);
}

// 测试游戏结束条件：所有移动棋子被吃
TEST(JunqiGameRulesTest, GameEndCondition_AllPieces)
{
    CJunQiLayout layout;
    
    // 创建简化局面（只有军旗和一个棋子）
    POINT ptFlag = {0, 0};
    POINT ptArmy = {0, 1};
    
    layout.SetCellChessman(ptFlag, OR_FLAG, SIDE_TOP, TRUE);
    layout.SetCellChessman(ptArmy, OR_ARMY, SIDE_TOP, TRUE);
    
    // 检查初始状态：还有活着的棋子
    EXPECT_FALSE(layout.IsPlayerLost(SIDE_TOP));
    
    // 移除军长
    layout.RemoveChessman(ptArmy);
    
    // 现在只有军旗，应该输了（军旗无法移动）
    EXPECT_TRUE(layout.IsPlayerLost(SIDE_TOP));
}

// 测试方向转换
TEST(JunqiGameRulesTest, SideConversion)
{
    CJunQiLayout layout;
    
    POINT ptCell = {8, 8};
    layout.SetCellChessman(ptCell, OR_ARMY, SIDE_BOTTOM, TRUE);
    
    // 获取该棋子的方向
    int nSide = layout.ConvertASide(ptCell);
    EXPECT_EQ(nSide, SIDE_BOTTOM);
    
    // 更改同位置的棋子
    layout.RemoveChessman(ptCell);
    layout.SetCellChessman(ptCell, OR_CHIEF, SIDE_TOP, TRUE);
    
    nSide = layout.ConvertASide(ptCell);
    EXPECT_EQ(nSide, SIDE_TOP);
}

// 测试棋子类型查询
TEST(JunqiGameRulesTest, ChessTypeQuery)
{
    CJunQiLayout layout;
    
    POINT ptCell1 = {8, 8};
    POINT ptCell2 = {8, 9};
    POINT ptCell3 = {8, 10};
    
    layout.SetCellChessman(ptCell1, OR_ARMY, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_CHIEF, SIDE_BOTTOM, TRUE);
    
    // ptCell3 是空的
    
    // 检查棋子状态
    EXPECT_TRUE(layout.IsValidCell(SIDE_BOTTOM, ptCell1));
    EXPECT_TRUE(layout.IsValidCell(SIDE_BOTTOM, ptCell2));
    EXPECT_TRUE(layout.IsValidCell(SIDE_BOTTOM, ptCell3));
    
    // 检查棋子是否存在
    EXPECT_EQ(OR_ARMY, layout.GetCellChessOffRank(ptCell1));
    EXPECT_EQ(OR_CHIEF, layout.GetCellChessOffRank(ptCell2));
    EXPECT_EQ(OR_NULL, layout.GetCellChessOffRank(ptCell3));
}


// 测试指挥部位置查询
TEST(JunqiGameRulesTest, PodiumQuery)
{
    CJunQiLayout layout;
    
    // 左方的大本营在 (0, 7)
    POINT ptPodium = {0, 7};
    
    // 该位置应该属于左方
    int nSide = layout.GetPodiumSide(ptPodium);
    EXPECT_EQ(SIDE_LEFT, nSide);
}

// 测试格子类型查询
TEST(JunqiGameRulesTest, CellTypeQuery)
{
    CJunQiMap layout;
    
    // 兵站应该在内部
    POINT ptCorps = {8, 8};
    int nType = layout.GetCellType(ptCorps);
    EXPECT_EQ(CP_CORPS, nType);
    
    // 兵营应该在四个角
    POINT ptBarback = {7, 2};
    nType = layout.GetCellType(ptBarback);
    EXPECT_EQ(CP_BARBACK, nType);
    
    // 大本营在角落
    POINT ptPodium = {0, 7};
    nType = layout.GetCellType(ptPodium);
    EXPECT_EQ(CP_PODIUM, nType);
}
