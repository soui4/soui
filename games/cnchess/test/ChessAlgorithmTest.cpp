#include <gtest/gtest.h>
#include <windows.h>
#include <tchar.h>
#include <Chessman.h>
#include <ChessLayout.h>


// 测试棋盘初始化
TEST(ChessAlgorithmTest, BoardInitialization)
{
    CChessLayout layout;
    CHESSMAN initLayout[10][9];
    
    // 初始化一个空棋盘
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 9; x++) {
            initLayout[y][x] = CHSMAN_NULL;
        }
    }
    
    // 设置一些测试棋子
    initLayout[0][4] = CHSMAN_RED_JIANG;  // 红帅
    initLayout[9][4] = CHSMAN_BLK_JIANG;  // 黑将
    
    layout.InitLayout(initLayout, CS_RED);
    
    // 检查初始状态下棋盘是否正确设置
    EXPECT_EQ(CHSMAN_RED_JIANG, layout.m_chesses[0][4]);
    EXPECT_EQ(CHSMAN_BLK_JIANG, layout.m_chesses[9][4]);
}

// 测试帅/将的移动规则
TEST(ChessAlgorithmTest, JiangMovement)
{
    CChessLayout layout;
    CHESSMAN initLayout[10][9];
    
    // 初始化一个空棋盘
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 9; x++) {
            initLayout[y][x] = CHSMAN_NULL;
        }
    }
    
    // 放置红帅和黑将
    initLayout[1][4] = CHSMAN_RED_JIANG;  // 红帅放在九宫格内
    initLayout[8][4] = CHSMAN_BLK_JIANG;  // 黑将放在九宫格内
    
    layout.InitLayout(initLayout, CS_RED);
    
    POINT moves[MAX_MOVES];
    
    // 测试红帅的移动
    int moveCount = layout.GetPossiableMoves(4, 1, moves);
    
    // 红帅应该能在九宫格内移动
    EXPECT_GT(moveCount, 0);
    
    // 检查是否所有移动都在九宫格内
    for (int i = 0; i < moveCount; i++) {
        if(moves[i].y == 8 &&  moves[i].x == 4) // 将不能吃将
            continue;
        // 九宫格范围：x: 3-5, y: 0-2
        EXPECT_GE(moves[i].x, 3);
        EXPECT_LE(moves[i].x, 5);
        EXPECT_GE(moves[i].y, 0);
        EXPECT_LE(moves[i].y, 2);
    }
}

// 测试车的移动规则
TEST(ChessAlgorithmTest, JuMovement)
{
    CChessLayout layout;
    CHESSMAN initLayout[10][9];
    
    // 初始化一个空棋盘
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 9; x++) {
            initLayout[y][x] = CHSMAN_NULL;
        }
    }
    
    // 放置一个红车在中心位置
    initLayout[4][4] = CHSMAN_RED_JU;
    
    layout.InitLayout(initLayout, CS_RED);
    
    POINT moves[MAX_MOVES];
    
    // 测试红车的移动
    int moveCount = layout.GetPossiableMoves(4, 4, moves);
    
    // 车应该能在没有阻挡的情况下沿直线移动
    EXPECT_GT(moveCount, 0);
    
    // 检查移动是否都是直线（同一行或同一列）
    for (int i = 0; i < moveCount; i++) {
        bool isHorizontal = (moves[i].y == 4);  // 同一行
        bool isVertical = (moves[i].x == 4);    // 同一列
        EXPECT_TRUE(isHorizontal || isVertical);
    }
}

// 测试马的移动规则
TEST(ChessAlgorithmTest, MaMovement)
{
    CChessLayout layout;
    CHESSMAN initLayout[10][9];
    
    // 初始化一个空棋盘
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 9; x++) {
            initLayout[y][x] = CHSMAN_NULL;
        }
    }
    
    // 放置一个红马
    initLayout[4][4] = CHSMAN_RED_MA;
    
    layout.InitLayout(initLayout, CS_RED);
    
    POINT moves[MAX_MOVES];
    
    // 测试红马的移动
    int moveCount = layout.GetPossiableMoves(4, 4, moves);
    
    // 马应该能走"日"字形状
    EXPECT_GT(moveCount, 0);
    
    // 检查马的移动是否符合"日"字规律
    bool foundValidMove = false;
    for (int i = 0; i < moveCount; i++) {
        int dx = abs(moves[i].x - 4);
        int dy = abs(moves[i].y - 4);
        
        // 马走"日"字：一个方向走1格，另一个方向走2格
        if ((dx == 1 && dy == 2) || (dx == 2 && dy == 1)) {
            foundValidMove = true;
        }
    }
    
    EXPECT_TRUE(foundValidMove);
}

// 测试炮的移动规则
TEST(ChessAlgorithmTest, PaoMovement)
{
    CChessLayout layout;
    CHESSMAN initLayout[10][9];
    
    // 初始化一个空棋盘
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 9; x++) {
            initLayout[y][x] = CHSMAN_NULL;
        }
    }
    
    // 放置一个红炮
    initLayout[4][4] = CHSMAN_RED_PAO;
    
    layout.InitLayout(initLayout, CS_RED);
    
    POINT moves[MAX_MOVES];
    
    // 测试红炮的移动
    int moveCount = layout.GetPossiableMoves(4, 4, moves);
    
    // 炮应该能沿直线移动（不吃子时）
    EXPECT_GT(moveCount, 0);
    
    // 检查移动是否都是直线（同一行或同一列）
    for (int i = 0; i < moveCount; i++) {
        bool isHorizontal = (moves[i].y == 4);  // 同一行
        bool isVertical = (moves[i].x == 4);    // 同一列
        EXPECT_TRUE(isHorizontal || isVertical);
    }
}

// 测试士的移动规则
TEST(ChessAlgorithmTest, ShiMovement)
{
    CChessLayout layout;
    CHESSMAN initLayout[10][9];
    
    // 初始化一个空棋盘
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 9; x++) {
            initLayout[y][x] = CHSMAN_NULL;
        }
    }
    
    // 放置一个红士在九宫格中心
    initLayout[1][4] = CHSMAN_RED_SHI;
    
    layout.InitLayout(initLayout, CS_RED);
    
    POINT moves[MAX_MOVES];
    
    // 测试红士的移动
    int moveCount = layout.GetPossiableMoves(4, 1, moves);
    
    // 士应该只能斜着走一格，且不能走出九宫格
    EXPECT_GT(moveCount, 0);
    
    // 检查士的移动是否符合规则
    for (int i = 0; i < moveCount; i++) {
        int dx = abs(moves[i].x - 4);
        int dy = abs(moves[i].y - 1);
        
        // 士斜着走一格：dx=1且dy=1
        EXPECT_EQ(dx, 1);
        EXPECT_EQ(dy, 1);
        
        // 士不能走出九宫格：x: 3-5, y: 0-2
        EXPECT_GE(moves[i].x, 3);
        EXPECT_LE(moves[i].x, 5);
        EXPECT_GE(moves[i].y, 0);
        EXPECT_LE(moves[i].y, 2);
    }
}

// 测试相/象的移动规则
TEST(ChessAlgorithmTest, XiangMovement)
{
    CChessLayout layout;
    CHESSMAN initLayout[10][9];
    
    // 初始化一个空棋盘
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 9; x++) {
            initLayout[y][x] = CHSMAN_NULL;
        }
    }
    
    // 放置一个红相
    initLayout[4][4] = CHSMAN_RED_XIANG;
    
    layout.InitLayout(initLayout, CS_RED);
    
    POINT moves[MAX_MOVES];
    
    // 测试红相的移动
    int moveCount = layout.GetPossiableMoves(4, 4, moves);
    
    // 相应该走"田"字，且不能过河
    EXPECT_GT(moveCount, 0);
    
    // 检查相的移动是否符合规则
    for (int i = 0; i < moveCount; i++) {
        int dx = abs(moves[i].x - 4);
        int dy = abs(moves[i].y - 4);
        
        // 相走"田"字：dx=2且dy=2
        EXPECT_EQ(dx, 2);
        EXPECT_EQ(dy, 2);
        
        // 相不能过河：红相y坐标不能大于4
        EXPECT_LE(moves[i].y, 4);
    }
}

// 测试兵/卒的移动规则
TEST(ChessAlgorithmTest, BingMovement)
{
    CChessLayout layout;
    CHESSMAN initLayout[10][9];
    
    // 初始化一个空棋盘
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 9; x++) {
            initLayout[y][x] = CHSMAN_NULL;
        }
    }
    
    // 放置一个红兵在起始位置
    initLayout[3][4] = CHSMAN_RED_BING;
    
    layout.InitLayout(initLayout, CS_RED);
    
    POINT moves[MAX_MOVES];
    
    // 测试红兵的移动（未过河）
    int moveCount = layout.GetPossiableMoves(4, 3, moves);
    
    // 兵未过河只能向前走
    EXPECT_EQ(moveCount, 1);
    
    // 检查移动是否正确
    if (moveCount > 0) {
        EXPECT_EQ(moves[0].x, 4);  // x坐标不变
        EXPECT_EQ(moves[0].y, 4);  // y坐标加1（向前）
    }
    
    // 测试过河后的红兵
    initLayout[3][4] = CHSMAN_NULL;
    initLayout[5][4] = CHSMAN_RED_BING;  // 放在河对面
    layout.InitLayout(initLayout, CS_RED);
    
    moveCount = layout.GetPossiableMoves(4, 5, moves);
    
    // 过河后兵可以向前或横向移动
    EXPECT_GE(moveCount, 1);
    
    // 检查移动是否符合规则
    bool foundForward = false;
    bool foundHorizontal = false;
    
    for (int i = 0; i < moveCount; i++) {
        if (moves[i].x == 4 && moves[i].y == 4) {
            foundForward = true;  // 向前移动
        }
        if ((moves[i].x == 3 && moves[i].y == 5) || 
            (moves[i].x == 5 && moves[i].y == 5)) {
            foundHorizontal = true;  // 横向移动
        }
    }
    
    EXPECT_TRUE(foundForward || foundHorizontal);
}

// 测试走棋功能
TEST(ChessAlgorithmTest, MoveFunction)
{
    CChessLayout layout;
    CHESSMAN initLayout[10][9];
    
    // 初始化一个空棋盘
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 9; x++) {
            initLayout[y][x] = CHSMAN_NULL;
        }
    }
    
    // 放置一个红车
    initLayout[4][4] = CHSMAN_RED_JU;
    
    layout.InitLayout(initLayout, CS_RED);
    
    // 执行一次移动
    MOVESTEP move = layout.Move({4, 4}, {4, 2});
    
    // 检查移动结果
    EXPECT_EQ(move.pt1.x, 4);
    EXPECT_EQ(move.pt1.y, 4);
    EXPECT_EQ(move.pt2.x, 4);
    EXPECT_EQ(move.pt2.y, 2);
    EXPECT_EQ(move.enemy, CHSMAN_NULL);
    
    // 检查棋盘状态
    EXPECT_EQ(layout.m_chesses[4][4], CHSMAN_NULL);  // 原位置为空
    EXPECT_EQ(layout.m_chesses[2][4], CHSMAN_RED_JU);  // 新位置为车
}

// 测试悔棋功能
TEST(ChessAlgorithmTest, UndoMove)
{
    CChessLayout layout;
    CHESSMAN initLayout[10][9];
    
    // 初始化一个空棋盘
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 9; x++) {
            initLayout[y][x] = CHSMAN_NULL;
        }
    }
    
    // 放置一个红车
    initLayout[4][4] = CHSMAN_RED_JU;
    
    layout.InitLayout(initLayout, CS_RED);
    
    // 执行一次移动
    MOVESTEP move = layout.Move({4, 4}, {4, 2});
    
    // 检查移动后的状态
    EXPECT_EQ(layout.m_chesses[4][4], CHSMAN_NULL);
    EXPECT_EQ(layout.m_chesses[2][4], CHSMAN_RED_JU);
    
    // 执行悔棋
    BOOL undoResult = layout.UndoMove(move);
    
    // 检查悔棋结果
    EXPECT_TRUE(undoResult);
    
    // 检查棋盘状态是否恢复
    EXPECT_EQ(layout.m_chesses[4][4], CHSMAN_RED_JU);  // 原位置恢复为车
    EXPECT_EQ(layout.m_chesses[2][4], CHSMAN_NULL);    // 新位置变为空
}

// 测试走棋描述功能
TEST(ChessAlgorithmTest, MoveDescription)
{
    CChessLayout layout;
    CHESSMAN initLayout[10][9];
    
    // 初始化一个空棋盘
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 9; x++) {
            initLayout[y][x] = CHSMAN_NULL;
        }
    }
    
    // 放置一个红帅
    initLayout[0][4] = CHSMAN_RED_JIANG;
    
    layout.InitLayout(initLayout, CS_RED);
    
    // 执行一次移动
    MOVESTEP move = layout.Move({4, 0}, {4, 1});
    
    TCHAR desc[100];
    layout.GetMoveDescription(move, desc);
    
    // 检查描述不为空
    EXPECT_GT(_tcslen(desc), 0);
}