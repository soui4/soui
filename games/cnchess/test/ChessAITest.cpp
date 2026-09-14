#include <gtest/gtest.h>
#include <windows.h>
#include <tchar.h>
#include <Chessman.h>
#include <ChessLayout.h>
#include <ChessAI.h>
#include <cnchessProtocol.h>

namespace
{
    // 构造一个空棋盘，默认 m_actSide = CS_RED(轮到红方走棋)
    // InitLayout 参数为 const int[10][9]，故用 int 存储棋盘
    void ClearBoard(int layout[10][9])
    {
        for (int y = 0; y < 10; y++)
            for (int x = 0; x < 9; x++)
                layout[y][x] = CHSMAN_NULL;
    }

    // 判断 (pt1,pt2) 是否是该棋子当前的一个合法走法
    bool IsInPieceMoves(CChessLayout &layout, POINT pt1, POINT pt2)
    {
        POINT moves[MAX_MOVES];
        int n = layout.GetPossiableMoves(pt1.x, pt1.y, moves);
        for (int i = 0; i < n; i++)
        {
            if (moves[i].x == pt2.x && moves[i].y == pt2.y)
                return true;
        }
        return false;
    }

    // 判断指定方是否被将军
    bool IsChecked(const CChessLayout &layout, CHSSIDE side)
    {
        CChsLytState state(const_cast<CChessLayout *>(&layout));
        state.UpdateState();
        return state.IsJiangJun(side);
    }

    // 记录某棋盘状态，用于验证搜索前后状态被正确恢复
    void SaveBoard(CHESSMAN dst[10][9], const CChessLayout &layout)
    {
        for (int y = 0; y < 10; y++)
            for (int x = 0; x < 9; x++)
                dst[y][x] = layout.m_chesses[y][x];
    }

    bool BoardsEqual(const CHESSMAN a[10][9], const CHESSMAN b[10][9])
    {
        for (int y = 0; y < 10; y++)
            for (int x = 0; x < 9; x++)
                if (a[y][x] != b[y][x])
                    return false;
        return true;
    }

    // 无效走法哨兵
    bool IsInvalidMove(const MOVESTEP &m)
    {
        return m.pt1.x < 0 || m.pt1.y < 0;
    }
}

// 智力等级 -> 搜索深度 映射
TEST(ChessAITest, LevelToDepthMapping)
{
    EXPECT_EQ(4, CChessAI::LevelToDepth(ROBOT_LEVEL_BEGINNER)); // 初级
    EXPECT_EQ(5, CChessAI::LevelToDepth(ROBOT_LEVEL_MEDIUM));   // 中级
    EXPECT_EQ(6, CChessAI::LevelToDepth(ROBOT_LEVEL_ADVANCED)); // 高级
    EXPECT_EQ(4, CChessAI::LevelToDepth(0));                    // 非法等级回退初级
    EXPECT_EQ(4, CChessAI::LevelToDepth(99));
}

// 默认开局下，三个难度均能给出一个有效、且不送王的走法
TEST(ChessAITest, DefaultBoardValidMoves)
{
    for (int lvl = ROBOT_LEVEL_BEGINNER; lvl <= ROBOT_LEVEL_ADVANCED; lvl++)
    {
        CChessLayout layout;
        layout.InitLayout(NULL, CS_RED);

        CHESSMAN before[10][9];
        SaveBoard(before, layout);

        MOVESTEP best = CChessAI::SearchBestMove(layout, CChessAI::LevelToDepth(lvl));

        // 必须能找到合法走法
        EXPECT_FALSE(IsInvalidMove(best)) << "level=" << lvl;
        if (IsInvalidMove(best))
            continue;

        // 起点必须有本方可移动的棋子，终点必须是该棋子的合法落点
        EXPECT_EQ(CS_RED, CHSMANSIDE(layout.m_chesses[best.pt1.y][best.pt1.x]));
        EXPECT_TRUE(IsInPieceMoves(layout, best.pt1, best.pt2)) << "level=" << lvl;

        // 走完后不能把自己送入将军
        CChessLayout l2;
        l2.Copy(&layout);
        l2.Move(best.pt1, best.pt2);
        EXPECT_FALSE(IsChecked(l2, CS_RED)) << "level=" << lvl;

        // 搜索内部执行了走子/悔子，棋盘必须被完整恢复
        EXPECT_TRUE(BoardsEqual(before, layout.m_chesses)) << "level=" << lvl;
    }
}

// 轮到行棋的一方无子可走(全部棋子被吃光)时返回无效走法
TEST(ChessAITest, NoPiecesReturnsInvalid)
{
    int board[10][9];
    ClearBoard(board);
    board[3][9] = CHSMAN_BLK_JIANG; // 仅黑方有将

    CChessLayout layout;
    layout.InitLayout(board, CS_RED); // 红方无子，红方先行

    MOVESTEP best = CChessAI::SearchBestMove(layout, CChessAI::LevelToDepth(ROBOT_LEVEL_MEDIUM));
    EXPECT_TRUE(IsInvalidMove(best));
}

// 被将军时，AI 必须选择一个不送王的防守走法
TEST(ChessAITest, AvoidsStayingInCheck)
{
    int board[10][9];
    ClearBoard(board);
    board[4][1] = CHSMAN_RED_JIANG; // 红将
    board[0][0] = CHSMAN_RED_JU;    // 红方另有车
    board[3][9] = CHSMAN_BLK_JIANG; // 黑将
    board[4][9] = CHSMAN_BLK_JU;    // 黑车沿第4列将军

    CChessLayout layout;
    layout.InitLayout(board, CS_RED);

    // 先确认构造的局面确实将军
    EXPECT_TRUE(IsChecked(layout, CS_RED));

    MOVESTEP best = CChessAI::SearchBestMove(layout, CChessAI::LevelToDepth(ROBOT_LEVEL_MEDIUM));
    EXPECT_FALSE(IsInvalidMove(best));
    if (IsInvalidMove(best))
        return;

    // AI 给出的走法必须解除将军
    CChessLayout l2;
    l2.Copy(&layout);
    l2.Move(best.pt1, best.pt2);
    EXPECT_FALSE(IsChecked(l2, CS_RED));
}

// 搜索不会破坏原始棋盘状态(可用于多次调用/复局面)
TEST(ChessAITest, SearchDoesNotMutateLayout)
{
    CChessLayout layout;
    layout.InitLayout(NULL, CS_RED);

    CHESSMAN before[10][9];
    CHESSMAN idBefore[10][9];
    SaveBoard(before, layout);
    for (int y = 0; y < 10; y++)
        for (int x = 0; x < 9; x++)
            idBefore[y][x] = (CHESSMAN)layout.m_nChsID[y][x];

    // 连续多次搜索，结果保持一致(非破坏性)
    CChessAI::SearchBestMove(layout, 3);
    CChessAI::SearchBestMove(layout, 3);
    CChessAI::SearchBestMove(layout, 3);

    EXPECT_TRUE(BoardsEqual(before, layout.m_chesses));
    for (int y = 0; y < 10; y++)
        for (int x = 0; x < 9; x++)
            EXPECT_EQ((int)idBefore[y][x], layout.m_nChsID[y][x]);
}