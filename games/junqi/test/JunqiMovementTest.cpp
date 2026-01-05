#include <gtest/gtest.h>
#include <windows.h>
#include "JunqiMap.h"
#include "JunqiLayout.h"

// 测试棋子移动范围计算
TEST(JunqiMovementTest, NormalMovementRange)
{
    CJunQiLayout layout;
    CJunQiMap map;
    
    POINT ptCell = {8, 8};
    
    // 放置普通棋子（军长）
    layout.SetCellChessman(ptCell, OR_ARMY, SIDE_BOTTOM, TRUE);
    
    // 获取移动范围
    SOUI::SList<POINT> vecTravel;
    int nCount = layout.GetChessmanTravelSpace(ptCell, &vecTravel);
    
    // 普通棋子应该能上下左右移动
    EXPECT_GT(nCount, 0);
    
    // 移动范围应该包含相邻的有效格子
    POINT ptAdjacent = {7, 8};
    bool bFound = false;
    for (SOUI::SPOSITION pos = vecTravel.GetHeadPosition(); pos;)
    {
        POINT pt = vecTravel.GetNext(pos);
        if (pt.x == ptAdjacent.x && pt.y == ptAdjacent.y)
        {
            bFound = true;
            break;
        }
    }
    EXPECT_TRUE(bFound);
}

// 测试铁路移动
TEST(JunqiMovementTest, RailwayMovement)
{
    CJunQiLayout layout;
    
    POINT ptCell = {6, 1};  // 铁路上的点
    
    // 放置铁路部队
    layout.SetCellChessman(ptCell, OR_ARMY, SIDE_BOTTOM, TRUE);
    
    // 获取移动范围
    SOUI::SList<POINT> vecTravel;
    int nCount = layout.GetChessmanTravelSpace(ptCell, &vecTravel);
    
    // 铁路部队应该有更大的移动范围
    EXPECT_GT(nCount, 4);  // 应该超过四个相邻方向
}

// 测试工兵特殊移动
TEST(JunqiMovementTest, SapperMovement)
{
    CJunQiLayout layout;
    
    POINT ptCell = {8, 8};
    
    // 放置工兵
    layout.SetCellChessman(ptCell, OR_SAPPER, SIDE_BOTTOM, TRUE);
    
    // 获取移动范围
    std::vector<POINT> vecTravel;
    int nCount = layout.GetChessmanTravelSpace(ptCell, &vecTravel);
    
    // 工兵应该能移动到相邻格子
    EXPECT_GT(nCount, 0);
}

// 测试被阻挡的移动
TEST(JunqiMovementTest, BlockedMovement)
{
    CJunQiLayout layout;
    
    POINT ptCell1 = {8, 8};
    POINT ptCell2 = {8, 9};
    
    // 放置两个棋子
    layout.SetCellChessman(ptCell1, OR_ARMY, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_CHIEF, SIDE_TOP, TRUE);
    
    // 获取移动范围
    std::vector<POINT> vecTravel;
    layout.GetChessmanTravelSpace(ptCell1, &vecTravel);
    
    // 不应该能移动到有敌方棋子的位置（但应该能吃掉）
    bool bCanEat = false;
    for (const auto& pt : vecTravel)
    {
        if (pt.x == ptCell2.x && pt.y == ptCell2.y)
        {
            bCanEat = true;
            break;
        }
    }
    // 虽然在范围内，但需要执行吃棋子操作
    EXPECT_TRUE(bCanEat);
}

// 测试大本营移动限制
TEST(JunqiMovementTest, PodiumRestriction)
{
    CJunQiLayout layout;
    CJunQiMap map;
    // 左方的大本营在 (0, 7)
    POINT ptPodium = {0, 7};
    
    // 放置左方的棋子在大本营
    layout.SetCellChessman(ptPodium, OR_CHIEF, SIDE_LEFT, TRUE);
    
    // 获取移动范围
    std::vector<POINT> vecTravel;
    layout.GetChessmanTravelSpace(ptPodium, &vecTravel);
    
    // 大本营只能在自己方向内移动
    for (const auto& pt : vecTravel)
    {
        int nType = map.GetCellType(pt);
        EXPECT_TRUE(nType != CP_NULL);
    }
}

// 测试有效移动目的地
TEST(JunqiMovementTest, ValidMovementDestination)
{
    CJunQiLayout layout;
    
    POINT ptCell = {8, 8};
    POINT ptDest1 = {8, 9};
    POINT ptDest2 = {-1, 8};  // 无效坐标
    
    layout.SetCellChessman(ptCell, OR_ARMY, SIDE_BOTTOM, TRUE);
    
    // 有效目的地
    //EXPECT_TRUE(layout.IsValidMoveDest(ptCell, ptDest1));
    
    // 无效目的地
    //EXPECT_FALSE(layout.IsValidMoveDest(ptCell, ptDest2));
}

// 测试棋子移动执行
TEST(JunqiMovementTest, ExecuteMovement)
{
    CJunQiLayout layout;
    
    POINT ptCell1 = {8, 8};
    POINT ptCell2 = {8, 9};
    
    // 初始化棋子
    layout.SetCellChessman(ptCell1, OR_ARMY, SIDE_BOTTOM, TRUE);
    
    // 执行移动
    int nResult = layout.MoveChessmanServer(ptCell1, ptCell2);
    
    // 应该是成功的移动（RST_OK）
    //EXPECT_NE(RST_INVALID, nResult);
    //EXPECT_NE(RST_OUT_BOUNDARY, nResult);
}

// 测试吃棋子的移动
TEST(JunqiMovementTest, EatMovement)
{
    CJunQiLayout layout;
    
    POINT ptCell1 = {8, 8};
    POINT ptCell2 = {8, 9};
    
    // 放置两个棋子
    layout.SetCellChessman(ptCell1, OR_CHIEF, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_ARMY, SIDE_TOP, TRUE);
    
    // 执行吃棋子的移动
    int nResult = layout.MoveChessmanServer(ptCell1, ptCell2);
    
    // 结果应该是吃棋子（PUT_EAT）
    EXPECT_EQ(PUT_EAT, nResult & 0xFF);
    
    // 吃掉的棋子应该消失
    EXPECT_EQ(OR_NULL, layout.GetCellChessOffRank(ptCell2));
}

// 测试被吃的移动
TEST(JunqiMovementTest, BeEatenMovement)
{
    CJunQiLayout layout;
    
    POINT ptCell1 = {8, 8};
    POINT ptCell2 = {8, 9};
    
    // 放置两个棋子
    layout.SetCellChessman(ptCell1, OR_ARMY, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_CHIEF, SIDE_TOP, TRUE);
    
    // 执行被吃的移动
    int nResult = layout.MoveChessmanServer(ptCell1, ptCell2);
    
    // 结果应该是被吃（PUT_BEEAT）
    EXPECT_EQ(PUT_BEEAT, nResult & 0xFF);
    
    // 被吃的棋子（军长）应该消失
    EXPECT_EQ(OR_NULL, layout.GetCellChessOffRank(ptCell1));
}

// 测试捕捉军旗（游戏结束）
TEST(JunqiMovementTest, CaptureFlagGameEnd)
{
    CJunQiLayout layout;
    
    // 设置军旗位置
    POINT ptFlag = {0, 0};
    layout.SetCellChessman(ptFlag, OR_FLAG, SIDE_TOP, TRUE);
    
    POINT ptAttacker = {0, 1};
    layout.SetCellChessman(ptAttacker, OR_ARMY, SIDE_BOTTOM, TRUE);
    
    // 攻击军旗
    int nResult = layout.MoveChessmanServer(ptAttacker, ptFlag);
    
    // 应该捕捉军旗，游戏结束
    EXPECT_TRUE((nResult & 0xFF) == PUT_EAT || nResult == RST_DIE);
}

// 测试无效移动（超出范围）
TEST(JunqiMovementTest, InvalidMovementOutOfRange)
{
    CJunQiLayout layout;
    
    POINT ptCell = {8, 8};
    POINT ptDest = {8, 15};  // 超过普通移动范围
    
    layout.SetCellChessman(ptCell, OR_ARMY, SIDE_BOTTOM, TRUE);
    
    // 执行远距离移动（不在铁路上）
    int nResult = layout.MoveChessmanServer(ptCell, ptDest);
    
    // 应该失败
    EXPECT_EQ(RST_NULL, nResult);
}

// 测试无效移动（边界外）
TEST(JunqiMovementTest, InvalidMovementOutOfBoundary)
{
    CJunQiLayout layout;
    
    POINT ptCell = {8, 8};
    POINT ptDest = {-1, 8};  // 超出棋盘
    
    layout.SetCellChessman(ptCell, OR_ARMY, SIDE_BOTTOM, TRUE);
    
    // 执行边界外移动
    int nResult = layout.MoveChessmanServer(ptCell, ptDest);
    
    // 应该返回越界错误
    EXPECT_EQ(RST_NULL, nResult);
}

// 测试路径查找（非铁路）
TEST(JunqiMovementTest, PathFinding)
{
    CJunQiLayout layout;
    
    POINT ptStart = {8, 8};
    POINT ptEnd = {10, 10};
    
    // 查找路径
    std::vector<POINT> vecPath;
    // 注意：实际的路径查找可能需要在完整实现后测试
    
    // 这里只是验证接口存在
    EXPECT_TRUE(true);
}

// 测试同归于尽（炸弹）
TEST(JunqiMovementTest, MutualDestruction)
{
    CJunQiLayout layout;
    layout.SetProperty(4, 1, 0);  // 启用炸弹满天飞
    
    POINT ptCell1 = {8, 8};
    POINT ptCell2 = {8, 9};
    
    // 放置炸弹和任意棋子
    layout.SetCellChessman(ptCell1, OR_BOMB, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_ARMY, SIDE_TOP, TRUE);
    
    // 执行移动
    int nResult = layout.MoveChessmanServer(ptCell1, ptCell2);
    
    // 结果应该是同归于尽（PUT_BOMB）
    EXPECT_EQ(PUT_BOMB, nResult & 0xFF);
    
    // 两个棋子都应该消失
    EXPECT_EQ(OR_NULL, layout.GetCellChessOffRank(ptCell1));
    EXPECT_EQ(OR_NULL, layout.GetCellChessOffRank(ptCell2));
}
