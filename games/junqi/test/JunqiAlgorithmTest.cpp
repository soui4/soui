#include <gtest/gtest.h>
#include <windows.h>
#include "JunqiMap.h"
#include "JunqiLayout.h"

// 测试棋盘初始化
TEST(JunqiAlgorithmTest, MapInitialization)
{
    CJunQiMap map;
    
    // 检查棋格类型初始化
    EXPECT_EQ(CP_CORPS, map.GetCellType(6, 6));      // 兵站
    EXPECT_EQ(CP_BARBACK, map.GetCellType(7, 2));    // 兵营
    EXPECT_EQ(CP_PODIUM, map.GetCellType(0, 7));     // 大本营
    EXPECT_EQ(CP_NULL, map.GetCellType(7, 7));       // 无效位置
}

// 测试铁路查询
TEST(JunqiAlgorithmTest, RailwayQuery)
{
    CJunQiMap map;
    
    POINT ptCell = {6, 1};
    EXPECT_TRUE(map.IsCellInRailway(ptCell));        // 铁路上的点
    
    ptCell = {7, 2};
    EXPECT_FALSE(map.IsCellInRailway(ptCell));       // 非铁路点
}

// 测试坐标旋转
TEST(JunqiAlgorithmTest, CoordinateRotation)
{
    CJunQiMap map;
    
    POINT pt = {8, 8};  // 中心点
    
    // 同一方位应该返回相同的坐标
    POINT rotated = CJunQiMap::RotateCoordinate(pt, 0, 0);
    EXPECT_EQ(pt.x, rotated.x);
    EXPECT_EQ(pt.y, rotated.y);
    
    // 旋转 90 度四次应该回到原点
    POINT p = pt;
    for (int i = 0; i < 4; i++)
    {
        p = CJunQiMap::RotateCoordinate(p, i, (i + 1) % 4);
    }
    EXPECT_EQ(8, p.x);
    EXPECT_EQ(8, p.y);
}

// 测试游戏布局初始化
TEST(JunqiAlgorithmTest, LayoutInitialization)
{
    CJunQiLayout layout;
    
    // 初始化一个简单的布局
    int nMatStone[25][17] = {0};  // 25 个位置，17列
    
    // 为25个位置的SIDE_BOTTOM位置设置一些棋子
    nMatStone[0][SIDE_BOTTOM] = OR_FLAG;      // 军旗
    nMatStone[1][SIDE_BOTTOM] = OR_CHIEF;     // 司令
    nMatStone[2][SIDE_BOTTOM] = OR_ARMY;      // 军长
    
    layout.InitLayout(nMatStone, SIDE_BOTTOM);  // 初始化SIDE_BOTTOM方向
    
    // 检查棋子是否正确放置
    POINT ptFlag = layout.GetFlagPosition(SIDE_BOTTOM);
    EXPECT_NE(-1, ptFlag.x);
    EXPECT_NE(-1, ptFlag.y);
}

// 测试棋子放置和查询
TEST(JunqiAlgorithmTest, ChessmanPlacement)
{
    CJunQiLayout layout;
    
    POINT ptCell = {8, 8};
    
    // 放置一个军长
    layout.SetCellChessman(ptCell, OR_ARMY, SIDE_BOTTOM, TRUE);
    
    // 验证棋子被放置
    EXPECT_EQ(OR_ARMY, layout.GetCellChessOffRank(ptCell));
    EXPECT_EQ(SIDE_BOTTOM, layout.ConvertASide(ptCell));
}

// 测试棋子比较规则
TEST(JunqiAlgorithmTest, ChessmanComparison)
{
    CJunQiLayout layout;
    
    // 司令最强
    EXPECT_GT(layout.ChessmanCompare(OR_CHIEF, OR_ARMY), 0);
    
    // 军长比师长强
    EXPECT_GT(layout.ChessmanCompare(OR_ARMY, OR_DIVISION), 0);
    
    // 同等级应该返回 0
    EXPECT_EQ(layout.ChessmanCompare(OR_ARMY, OR_ARMY), 0);
    
    // 工兵比排长强
    EXPECT_LT(layout.ChessmanCompare(OR_SAPPER, OR_PLATOON), 0);
}

// 测试棋子移动结果
TEST(JunqiAlgorithmTest, MovementResult)
{
    CJunQiLayout layout;
    
    POINT ptCell1 = {6, 8};
    POINT ptCell2 = {8, 8};
    
    // 放置两个棋子
    layout.SetCellChessman(ptCell1, OR_ARMY, SIDE_BOTTOM, TRUE);      // 军长
    layout.SetCellChessman(ptCell2, OR_DIVISION, SIDE_TOP, TRUE);     // 师长
    
    // 军长应该吃掉师长
    int nResult = layout.GetCmpResult(ptCell1, ptCell2);
    EXPECT_EQ(PUT_EAT, nResult);
}

// 测试工兵吃地雷
TEST(JunqiAlgorithmTest, SapperEatsMine)
{
    CJunQiLayout layout;
    
    POINT ptCell1 = {6, 8};
    POINT ptCell2 = {8, 8};
    
    // 放置工兵和地雷
    layout.SetCellChessman(ptCell1, OR_SAPPER, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_MINE, SIDE_TOP, TRUE);
    
    // 工兵应该吃掉地雷
    int nResult = layout.GetCmpResult(ptCell1, ptCell2);
    EXPECT_EQ(PUT_EAT, nResult);
}

// 测试炸弹效果
TEST(JunqiAlgorithmTest, BombEffect)
{
    CJunQiLayout layout;
    layout.SetProperty(4, 1, 0);  // 启用炸弹满天飞
    
    POINT ptCell1 = {6, 8};
    POINT ptCell2 = {8, 8};
    
    // 放置炸弹和军长
    layout.SetCellChessman(ptCell1, OR_BOMB, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_ARMY, SIDE_TOP, TRUE);
    
    // 应该同归于尽
    int nResult = layout.GetCmpResult(ptCell1, ptCell2);
    EXPECT_EQ(PUT_BOMB, nResult);
}

// 测试军旗位置查询
TEST(JunqiAlgorithmTest, FlagPosition)
{
    CJunQiLayout layout;
    
    // 初始化时每个方向应该有默认的军旗位置
    for (int side = 0; side < 4; side++)
    {
        POINT ptFlag = layout.GetFlagPosition(side);
        EXPECT_NE(-1, ptFlag.x);
        EXPECT_NE(-1, ptFlag.y);
    }
}

// 测试有效格子判断
TEST(JunqiAlgorithmTest, ValidCellCheck)
{
    CJunQiLayout layout;
    
    // 普通兵站对所有方都有效
    POINT ptCorps = {8, 8};
    EXPECT_TRUE(layout.IsValidCell(SIDE_BOTTOM, ptCorps));
    EXPECT_TRUE(layout.IsValidCell(SIDE_TOP, ptCorps));
    
    // 大本营只对特定方有效
    POINT ptPodium = {0, 7};  // 左方的大本营
    EXPECT_TRUE(layout.IsValidCell(SIDE_LEFT, ptPodium));
    EXPECT_FALSE(layout.IsValidCell(SIDE_BOTTOM, ptPodium));
}

// 测试棋子移除
TEST(JunqiAlgorithmTest, ChessmanRemoval)
{
    CJunQiLayout layout;
    
    POINT ptCell = {8, 8};
    layout.SetCellChessman(ptCell, OR_ARMY, SIDE_BOTTOM, TRUE);
    
    // 验证棋子存在
    EXPECT_EQ(OR_ARMY, layout.GetCellChessOffRank(ptCell));
    
    // 移除棋子
    int nRank = layout.RemoveChessman(ptCell);
    EXPECT_EQ(OR_ARMY, nRank);
    
    // 验证棋子已移除
    EXPECT_EQ(OR_NULL, layout.GetCellChessOffRank(ptCell));
}

// 测试棋子交换
TEST(JunqiAlgorithmTest, ChessmanExchange)
{
    CJunQiLayout layout;
    
    POINT ptCell1 = {8, 8};
    POINT ptCell2 = {8, 9};
    
    layout.SetCellChessman(ptCell1, OR_ARMY, SIDE_BOTTOM, TRUE);
    layout.SetCellChessman(ptCell2, OR_CHIEF, SIDE_TOP, TRUE);
    
    // 交换
    layout.ExchangeChessman(ptCell1, ptCell2);
    
    // 验证交换
    EXPECT_EQ(OR_CHIEF, layout.GetCellChessOffRank(ptCell1));
    EXPECT_EQ(SIDE_TOP, layout.ConvertASide(ptCell1));
    
    EXPECT_EQ(OR_ARMY, layout.GetCellChessOffRank(ptCell2));
    EXPECT_EQ(SIDE_BOTTOM, layout.ConvertASide(ptCell2));
}

// 测试显示转换
TEST(JunqiAlgorithmTest, ChessmanShowToggle)
{
    CJunQiLayout layout;
    
    POINT ptCell = {8, 8};
    layout.SetCellChessman(ptCell, OR_ARMY, SIDE_BOTTOM, TRUE);
    
    // 初始状态应该显示
    EXPECT_TRUE(layout.IsChessShow(ptCell));
    
    // 转换显示状态
    layout.ConvertChess(ptCell);
    EXPECT_FALSE(layout.IsChessShow(ptCell));
    
    // 再转换一次
    layout.ConvertChess(ptCell);
    EXPECT_TRUE(layout.IsChessShow(ptCell));
}
