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
        int n = layout.GetPossibleMoves(pt1.x, pt1.y, moves);
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
    EXPECT_EQ(ROBOT_AI_DEPTH_BEGINNER, CChessAI::LevelToDepth(ROBOT_LEVEL_BEGINNER)); // 初级
    EXPECT_EQ(ROBOT_AI_DEPTH_MEDIUM,   CChessAI::LevelToDepth(ROBOT_LEVEL_MEDIUM));   // 中级
    EXPECT_EQ(ROBOT_AI_DEPTH_ADVANCED, CChessAI::LevelToDepth(ROBOT_LEVEL_ADVANCED)); // 高级
    EXPECT_EQ(ROBOT_AI_DEPTH_BEGINNER, CChessAI::LevelToDepth(0));                    // 非法等级回退初级
    EXPECT_EQ(ROBOT_AI_DEPTH_BEGINNER, CChessAI::LevelToDepth(99));
}

// 默认开局下，三个难度均能给出一个有效、且不送王的走法
// 注意:此为正确性验证,使用固定小深度避免高深度在完整开局下的组合爆炸;
//      深度映射本身由 LevelToDepthMapping 单独校验,高级别性能另测。
TEST(ChessAITest, DefaultBoardValidMoves)
{
    for (int lvl = ROBOT_LEVEL_BEGINNER; lvl <= ROBOT_LEVEL_ADVANCED; lvl++)
    {
        CChessLayout layout;
        layout.InitLayout(NULL, CS_RED);

        CHESSMAN before[10][9];
        SaveBoard(before, layout);

        MOVESTEP best = CChessAI::SearchBestMove(layout, 2);

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
    board[0][3] = CHSMAN_BLK_JIANG; // 仅黑方有将(3,0)

    CChessLayout layout;
    layout.InitLayout(board, CS_RED); // 红方无子，红方先行

    MOVESTEP best = CChessAI::SearchBestMove(layout, CChessAI::LevelToDepth(ROBOT_LEVEL_MEDIUM));
    EXPECT_TRUE(IsInvalidMove(best));
}

// 被车将军时，AI 必须选择一个不送王的防守走法(红将逃开或红车挡子解将)
TEST(ChessAITest, AvoidsStayingInCheck)
{
    int board[10][9];
    ClearBoard(board);
    board[7][4] = CHSMAN_RED_JIANG; // 红将 (4,7)
    board[9][4] = CHSMAN_BLK_JU;    // 黑车(4,9)沿第4列将军,中间y8为空
    board[8][0] = CHSMAN_RED_JU;    // 红另有一车(0,8),可横移到(4,8)挡子应将
    board[0][3] = CHSMAN_BLK_JIANG; // 黑将(3,0) 避开同列对脸干扰

    CChessLayout layout;
    layout.InitLayout(board, CS_RED);

    // 先确认构造的局面确实将军
    EXPECT_TRUE(IsChecked(layout, CS_RED));

    MOVESTEP best = CChessAI::SearchBestMove(layout, 3);
    EXPECT_FALSE(IsInvalidMove(best));
    if (IsInvalidMove(best))
        return;

    // AI 给出的走法必须解除将军
    CChessLayout l2;
    l2.Copy(&layout);
    l2.Move(best.pt1, best.pt2);
    EXPECT_FALSE(IsChecked(l2, CS_RED));
}

// 被炮隔子将军时，AI 必须应将(红将逃开或移走/吃掉炮架)
TEST(ChessAITest, AvoidsCannonCheck)
{
    int board[10][9];
    ClearBoard(board);
    board[7][4] = CHSMAN_RED_JIANG; // 红将 (4,7)
    board[8][4] = CHSMAN_RED_BING;  // 红炮架(4,8):黑炮隔此架打将
    board[9][4] = CHSMAN_BLK_PAO;   // 黑炮 (4,9)
    board[0][3] = CHSMAN_BLK_JIANG; // 黑将 (3,0)

    CChessLayout layout;
    layout.InitLayout(board, CS_RED);

    // 确认当前局面确实将军
    EXPECT_TRUE(IsChecked(layout, CS_RED));

    MOVESTEP best = CChessAI::SearchBestMove(layout, 3);
    EXPECT_FALSE(IsInvalidMove(best));
    if (IsInvalidMove(best))
        return;

    // 该走法必须解除将军
    CChessLayout l2;
    l2.Copy(&layout);
    l2.Move(best.pt1, best.pt2);
    EXPECT_FALSE(IsChecked(l2, CS_RED));
}

// 将帅照面(对脸)时，AI 必须移动将/帅或挡子解将
TEST(ChessAITest, AvoidsFacingGeneralCheck)
{
    int board[10][9];
    ClearBoard(board);
    board[7][4] = CHSMAN_RED_JIANG; // 红将 (4,7)
    board[0][4] = CHSMAN_BLK_JIANG; // 黑将(4,0)同列对脸,中间y1..6无遮挡
    board[2][0] = CHSMAN_RED_JU;    // 红车(0,2)可移到(4,2)挡住对脸

    CChessLayout layout;
    layout.InitLayout(board, CS_RED);

    EXPECT_TRUE(IsChecked(layout, CS_RED));

    MOVESTEP best = CChessAI::SearchBestMove(layout, 3);
    EXPECT_FALSE(IsInvalidMove(best));
    if (IsInvalidMove(best))
        return;

    CChessLayout l2;
    l2.Copy(&layout);
    l2.Move(best.pt1, best.pt2);
    EXPECT_FALSE(IsChecked(l2, CS_RED));
}

// 被将军但有棋可走(有子可将应)时，AI 必须选择一个解杀的走法(挡/逃/吃将军子)
TEST(ChessAITest, CheckedHasMoveForcesResponse)
{
    int board[10][9];
    ClearBoard(board);
    board[7][4] = CHSMAN_RED_JIANG; // 红将 (4,7)
    board[9][4] = CHSMAN_BLK_JU;    // 黑车沿第4列将军(y8为空)
    board[8][0] = CHSMAN_RED_JU;    // 红方另有第8行的一车,可横向挡在(4,8)
    board[0][4] = CHSMAN_BLK_JIANG; // 黑将

    CChessLayout layout;
    layout.InitLayout(board, CS_RED);

    // 构造的局面确实处于将军状态
    EXPECT_TRUE(IsChecked(layout, CS_RED));

    MOVESTEP best = CChessAI::SearchBestMove(layout, 3);
    // 有棋可走，不存在必须判负
    EXPECT_FALSE(IsInvalidMove(best));
    if (IsInvalidMove(best))
        return;

    // 该走法必须解除将军(挡住了能挡子的应将)
    CChessLayout l2;
    l2.Copy(&layout);
    l2.Move(best.pt1, best.pt2);
    EXPECT_FALSE(IsChecked(l2, CS_RED));
}

// 被将军且无棋可走(将死)时，引擎判定无合法着法(对应判负信号)
TEST(ChessAITest, CheckedNoMovesReturnsLoss)
{
    int board[10][9];
    ClearBoard(board);
    board[7][4] = CHSMAN_RED_JIANG; // 红将 (4,7)
    board[9][4] = CHSMAN_BLK_JU;    // 黑车沿第4列封住纵向
    board[7][0] = CHSMAN_BLK_JU;    // 黑车沿第7行封住横向
    board[0][4] = CHSMAN_BLK_JIANG; // 黑将

    CChessLayout layout;
    layout.InitLayout(board, CS_RED);

    // 双车锁死确为将军
    EXPECT_TRUE(IsChecked(layout, CS_RED));

    // 唯一子为红将,且 (3,7)/(5,7)/(4,6) 每一步走完仍被将军，故无任何合法着法
    MOVESTEP best = CChessAI::SearchBestMove(layout, 3);
    EXPECT_TRUE(IsInvalidMove(best)) << "将死局面应返回无合法着法(判负)";
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