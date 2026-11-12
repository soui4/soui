// 长将 / 长捉（重复走子）判负相关测试
//
// 背景（对应工单两个 bug 之一）：默认长将 / 长捉步限为 3 步（server/PropBag.cpp
// 中 max_jiang / max_catch 配置默认 3）。用户反馈"机器人长将了不止 3 步"，
// 即怀疑重复走子限制没有真正生效。
//
// 本文件覆盖：
//   1) GetFarthestRepeat 的步限阈值边界（2 步不算、3 步算）；
//   2) 机器人带历史搜索时，在步限内允许将军、到达步限时必须改用合规着法；
//   3) 搜索引擎不得修改调用方的局面与历史（每步在独立线程执行，数据需本地副本）；
//   4) 被"保护"的将/帅同样会被判定为长将（GetProtecter 对将帅恒返回 0）；
//   5) 复现用户场景：连续将军时，若历史正确传入，机器人应在第 3 步改用非将军着法。
//
// 说明：判定口径由 CChessLayout::GetFarthestRepeat 统一提供，服务器
// CCnChess::GetFarthestRepeat 与机器人过滤 RootMoveViolatesRepeat 共用同一实现，
// 保证"判负口径"与"规避口径"一致。

#include <gtest/gtest.h>
#include <windows.h>
#include <tchar.h>
#include <Chessman.h>
#include <ChessLayout.h>
#include <ChessAI.h>
#include <cnchessProtocol.h>

namespace
{
    // 构造空棋盘
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
            if (moves[i].x == pt2.x && moves[i].y == pt2.y)
                return true;
        return false;
    }

    // 记录一步历史着法并推进局面
    void PlayMove(CChessLayout &layout, std::vector<MOVESTEP> &history,
                  int x1, int y1, int x2, int y2)
    {
        POINT pt1 = { x1, y1 };
        POINT pt2 = { x2, y2 };
        MOVESTEP ms = layout.Move(pt1, pt2);
        history.push_back(ms);
    }

    // 无效走法哨兵（搜索引擎在无合法着法时返回 pt<0）
    bool IsInvalidMove(const MOVESTEP &m)
    {
        return m.pt1.x < 0 || m.pt1.y < 0;
    }

    // 走完候选着法后是否触发长将 / 长捉判负（与机器人过滤、服务器判定同一口径）
    bool TriggersRepeatLoss(CChessLayout &layout, const std::vector<MOVESTEP> &history,
                            const MOVESTEP &candidate, int nMaxJiang, int nMaxCatch)
    {
        CChessLayout sim;
        sim.Copy(&layout);
        MOVESTEP played = sim.Move(candidate.pt1, candidate.pt2);

        std::vector<MOVESTEP> withCandidate = history;
        withCandidate.push_back(played);

        CHESSMAN chsEnemy = CHSMAN_NULL;
        int nRepeat = (int)CChessLayout::GetFarthestRepeat(sim, withCandidate, chsEnemy);
        if (chsEnemy == CHSMAN_RED_JIANG || chsEnemy == CHSMAN_BLK_JIANG)
            return nRepeat >= nMaxJiang;
        if (chsEnemy != CHSMAN_NULL)
            return nRepeat >= nMaxCatch;
        return false;
    }

    // 标准长将局面：黑将(4,0)、红帅(5,9)、红车(3,5)。
    // 红车可沿第 4 列将黑将，黑将退到 (3,0) 后再沿第 3 列被将，形成连续将军。
    void MakeLongCheckBoard(int board[10][9])
    {
        ClearBoard(board);
        board[0][4] = CHSMAN_BLK_JIANG; // (4,0)
        board[9][5] = CHSMAN_RED_JIANG; // (5,9)
        board[5][3] = CHSMAN_RED_JU;    // (3,5)
    }

    // 标准长捉局面：黑将(4,0)、红帅(5,9)、红车(1,4)、无根黑车(0,0)。
    void MakeLongCatchBoard(int board[10][9])
    {
        ClearBoard(board);
        board[0][4] = CHSMAN_BLK_JIANG; // (4,0)
        board[9][5] = CHSMAN_RED_JIANG; // (5,9)
        board[4][1] = CHSMAN_RED_JU;    // (1,4)
        board[0][0] = CHSMAN_BLK_JU;    // (0,0) 无根
    }

    // 追加一个"长将"循环：红车(3,5)->(4,5) 第4列将军，黑将(4,0)->(3,0) 躲避，
    // 红车(4,5)->(3,5) 第3列再将军，黑将(3,0)->(4,0) 回到原位。
    // 结束后局面回到初始，红方行棋；红方两步将军分别落在 (4,5)/(3,5) 两个不同落点。
    void AppendLongCheckCycle(CChessLayout &layout, std::vector<MOVESTEP> &history)
    {
        PlayMove(layout, history, 3, 5, 4, 5); // 红车将军（第4列）
        PlayMove(layout, history, 4, 0, 3, 0); // 黑将躲避
        PlayMove(layout, history, 4, 5, 3, 5); // 红车再将军（第3列）
        PlayMove(layout, history, 3, 0, 4, 0); // 黑将回位
    }

    // 追加一个"长捉"循环：红车(1,4)->(0,4) 追无根黑车(0,0)，黑车(0,0)->(1,0) 躲，
    // 红车(0,4)->(1,4) 回位，黑车(1,0)->(0,0) 回位。结束后局面回到初始。
    void AppendLongCatchCycle(CChessLayout &layout, std::vector<MOVESTEP> &history)
    {
        PlayMove(layout, history, 1, 4, 0, 4); // 红车追捉（落点 0,4）
        PlayMove(layout, history, 0, 0, 1, 0); // 黑车躲到 (1,0)
        PlayMove(layout, history, 0, 4, 1, 4); // 红车回位（落点 1,4）
        PlayMove(layout, history, 1, 0, 0, 0); // 黑车回位到 (0,0)
    }
}

// ---- 步限阈值边界：长将 ----
// 低于步限（仅 2 次将军，且落点不同 => 尚无同落点重复）：不应判负。
// 到达步限（第 3 次将军，落点回到第一次 => 重复）:应判负。
TEST(ChessLongCheckTest, LongCheckRepeatThresholdBoundary)
{
    const int nMaxJiang = 3; // 与 PropBag 配置默认 max_jiang 一致

    // 低于步限：历史只有 1 次将军 + 黑将躲避，候选为第 2 次将军（落点不同）。
    {
        int board[10][9];
        MakeLongCheckBoard(board);
        CChessLayout layout;
        layout.InitLayout(board, CS_RED);
        std::vector<MOVESTEP> history;

        PlayMove(layout, history, 3, 5, 4, 5); // 红车将军（落点 4,5）
        PlayMove(layout, history, 4, 0, 3, 0); // 黑将躲避

        // 候选：红车(4,5)->(3,5) 第二次将军（落点 3,5，与第一次不同）
        POINT ptFrom = {4, 5};
        POINT ptTo = {3, 5};
        MOVESTEP candidate = layout.Move(ptFrom, ptTo);
        history.push_back(candidate);

        CHESSMAN chsEnemy = CHSMAN_NULL;
        UINT nRepeat = CChessLayout::GetFarthestRepeat(layout, history, chsEnemy);
        EXPECT_EQ(CHSMAN_BLK_JIANG, chsEnemy);
        EXPECT_LT((int)nRepeat, nMaxJiang)
            << "2 次将军（落点不同，尚未形成同落点重复）不应触发长将判负";
        EXPECT_FALSE((int)nRepeat >= nMaxJiang);
    }

    // 到达步限：历史含 1 个完整循环（2 次将军，落点 4,5 / 3,5），候选为第 3 次将军
    // （落点回到 4,5，与第一次相同 => 重复）。
    {
        int board[10][9];
        MakeLongCheckBoard(board);
        CChessLayout layout;
        layout.InitLayout(board, CS_RED);
        std::vector<MOVESTEP> history;

        AppendLongCheckCycle(layout, history); // 红车两次将军 + 黑将两次躲避

        // 候选：红车(3,5)->(4,5) 第三次将军（落点 4,5，与第一次相同）
        POINT ptFrom = {3, 5};
        POINT ptTo = {4, 5};
        MOVESTEP candidate = layout.Move(ptFrom, ptTo);
        history.push_back(candidate);

        CHESSMAN chsEnemy = CHSMAN_NULL;
        UINT nRepeat = CChessLayout::GetFarthestRepeat(layout, history, chsEnemy);
        EXPECT_EQ(CHSMAN_BLK_JIANG, chsEnemy);
        EXPECT_GE((int)nRepeat, nMaxJiang)
            << "连续 3 次将军且第 3 次落点与第 1 次相同 => 应触发长将判负";
        EXPECT_TRUE((int)nRepeat >= nMaxJiang);
    }
}

// ---- 步限阈值边界：长捉 ----
TEST(ChessLongCheckTest, LongCatchRepeatThresholdBoundary)
{
    const int nMaxCatch = 3; // 与 PropBag 配置默认 max_catch 一致

    // 低于步限：历史仅 1 次追捉 + 黑车躲避，候选为第 2 次追捉（落点不同）。
    {
        int board[10][9];
        MakeLongCatchBoard(board);
        CChessLayout layout;
        layout.InitLayout(board, CS_RED);
        std::vector<MOVESTEP> history;

        PlayMove(layout, history, 1, 4, 0, 4); // 红车追捉（落点 0,4）
        PlayMove(layout, history, 0, 0, 1, 0); // 黑车躲避

        POINT ptFrom = {0, 4};
        POINT ptTo = {1, 4};
        MOVESTEP candidate = layout.Move(ptFrom, ptTo); // 第2次（落点 1,4）
        history.push_back(candidate);

        CHESSMAN chsEnemy = CHSMAN_NULL;
        UINT nRepeat = CChessLayout::GetFarthestRepeat(layout, history, chsEnemy);
        EXPECT_EQ(CHSMAN_BLK_JU, chsEnemy);
        EXPECT_LT((int)nRepeat, nMaxCatch)
            << "2 次追捉（落点不同）不应触发长捉判负";
    }

    // 到达步限：历史含 1 个完整循环（2 次追捉），候选为第 3 次追捉（落点回到第一次）。
    {
        int board[10][9];
        MakeLongCatchBoard(board);
        CChessLayout layout;
        layout.InitLayout(board, CS_RED);
        std::vector<MOVESTEP> history;

        AppendLongCatchCycle(layout, history);

        POINT ptFrom = {1, 4};
        POINT ptTo = {0, 4};
        MOVESTEP candidate = layout.Move(ptFrom, ptTo); // 第3次（落点 0,4）
        history.push_back(candidate);

        CHESSMAN chsEnemy = CHSMAN_NULL;
        UINT nRepeat = CChessLayout::GetFarthestRepeat(layout, history, chsEnemy);
        EXPECT_EQ(CHSMAN_BLK_JU, chsEnemy);
        EXPECT_GE((int)nRepeat, nMaxCatch)
            << "连续 3 次追捉且第 3 次落点与第 1 次相同 => 应触发长捉判负";
    }
}

// ---- 机器人：步限内允许将军（边界下方应被允许）----
// 历史只有 1 次将军时，第 2 次将军仍在步限内，过滤不应把它判为违规。
TEST(ChessLongCheckTest, RobotAllowedToCheckUnderLimit)
{
    const int nMaxJiang = 3;
    const int nMaxCatch = 3;

    int board[10][9];
    MakeLongCheckBoard(board);
    CChessLayout layout;
    layout.InitLayout(board, CS_RED);
    std::vector<MOVESTEP> history;

    PlayMove(layout, history, 3, 5, 4, 5); // 红车第一次将军
    PlayMove(layout, history, 4, 0, 3, 0); // 黑将躲避

    // 第 2 次将军的候选着法（落点不同，未到步限）
    MOVESTEP candidate;
    POINT ptFrom = {4, 5};
    POINT ptTo = {3, 5};
    candidate.pt1 = ptFrom;
    candidate.pt2 = ptTo;
    candidate.enemy = layout.m_chesses[3][5];
    candidate.nEnemyID = layout.m_nChsID[3][5];

    // 过滤口径：第 2 次将军不应被判为违规（步限为 3）
    EXPECT_FALSE(TriggersRepeatLoss(layout, history, candidate, nMaxJiang, nMaxCatch))
        << "步限内（仅 2 次）的将军应是被允许的";

    // 搜索本身也不应修改调用方的局面与历史
    CHESSMAN before[10][9];
    for (int y = 0; y < 10; y++)
        for (int x = 0; x < 9; x++)
            before[y][x] = layout.m_chesses[y][x];
    size_t nHistoryBefore = history.size();

    MOVESTEP best = CChessAI::SearchBestMove(layout, 2, 0, history, nMaxJiang, nMaxCatch);
    ASSERT_FALSE(IsInvalidMove(best));

    for (int y = 0; y < 10; y++)
        for (int x = 0; x < 9; x++)
            EXPECT_EQ(before[y][x], layout.m_chesses[y][x]);
    EXPECT_EQ(nHistoryBefore, history.size());
}

// ---- 机器人：到达步限时必须改用合规着法（复现用户"长将不止3步"场景）----
// 历史已含 2 次将军（1 个完整循环），第 3 次将军会触发长将判负；搜索引擎必须
// 排除该着法并返回不会立刻判负的合规着法。这直接验证"长将步限是否生效"。
TEST(ChessLongCheckTest, RobotAvoidsLongCheckAtLimit)
{
    const int nMaxJiang = 3;
    const int nMaxCatch = 3;

    int board[10][9];
    MakeLongCheckBoard(board);
    CChessLayout layout;
    layout.InitLayout(board, CS_RED);
    std::vector<MOVESTEP> history;

    AppendLongCheckCycle(layout, history); // 红车两次将军 + 黑将两次躲避

    CHESSMAN before[10][9];
    for (int y = 0; y < 10; y++)
        for (int x = 0; x < 9; x++)
            before[y][x] = layout.m_chesses[y][x];
    size_t nHistoryBefore = history.size();

    MOVESTEP best = CChessAI::SearchBestMove(layout, 2, 0, history, nMaxJiang, nMaxCatch);
    ASSERT_FALSE(IsInvalidMove(best)) << "应能找到合规的替代着法";

    // 不应继续走第 3 次将军（落点 4,5）
    EXPECT_FALSE(best.pt1.x == 3 && best.pt1.y == 5
                 && best.pt2.x == 4 && best.pt2.y == 5)
        << "机器人不应继续长将";

    // 返回着法走完后不能立即触发长将 / 长捉判负
    EXPECT_FALSE(TriggersRepeatLoss(layout, history, best, nMaxJiang, nMaxCatch))
        << "机器人返回的着法不应立刻构成长将判负";

    // 搜索引擎必须在本地副本上工作，不修改调用方的局面与历史
    for (int y = 0; y < 10; y++)
        for (int x = 0; x < 9; x++)
            EXPECT_EQ(before[y][x], layout.m_chesses[y][x]);
    EXPECT_EQ(nHistoryBefore, history.size());
}

// ---- 机器人：长捉步限同样生效 ----
TEST(ChessLongCheckTest, RobotAvoidsLongCatchAtLimit)
{
    const int nMaxJiang = 3;
    const int nMaxCatch = 3;

    int board[10][9];
    MakeLongCatchBoard(board);
    CChessLayout layout;
    layout.InitLayout(board, CS_RED);
    std::vector<MOVESTEP> history;

    AppendLongCatchCycle(layout, history); // 红车两次追捉 + 黑车两次躲避

    MOVESTEP best = CChessAI::SearchBestMove(layout, 2, 0, history, nMaxJiang, nMaxCatch);
    ASSERT_FALSE(IsInvalidMove(best)) << "应能找到合规的替代着法";

    // 不应继续第 3 次追捉（落点 0,4）
    EXPECT_FALSE(best.pt1.x == 1 && best.pt1.y == 4
                 && best.pt2.x == 0 && best.pt2.y == 4)
        << "机器人不应继续长捉";

    EXPECT_FALSE(TriggersRepeatLoss(layout, history, best, nMaxJiang, nMaxCatch))
        << "机器人返回的着法不应立刻构成长捉判负";
}

// ---- 被"保护"的将/帅同样应判定为长将 ----
// GetProtecter 对将/帅恒返回 0，因此即使黑将旁有士保护，连续将军仍应被判定为长将。
// 复现用户场景：黑将(4,0) 由 (5,1) 的士保护，验证保护不导致长将漏判。
TEST(ChessLongCheckTest, LongCheckDetectedEvenWhenKingProtected)
{
    const int nMaxJiang = 3;

    int board[10][9];
    MakeLongCheckBoard(board);
    board[1][5] = CHSMAN_BLK_SHI; // (5,1) 士，斜线保护 (4,0)，且不挡第4列/第3列将军线
    CChessLayout layout;
    layout.InitLayout(board, CS_RED);
    std::vector<MOVESTEP> history;

    AppendLongCheckCycle(layout, history); // 红车两次将军 + 黑将两次躲避

    POINT ptFrom = {3, 5};
    POINT ptTo = {4, 5};
    MOVESTEP candidate = layout.Move(ptFrom, ptTo); // 第3次将军
    history.push_back(candidate);

    CHESSMAN chsEnemy = CHSMAN_NULL;
    UINT nRepeat = CChessLayout::GetFarthestRepeat(layout, history, chsEnemy);
    EXPECT_EQ(CHSMAN_BLK_JIANG, chsEnemy);
    EXPECT_GE((int)nRepeat, nMaxJiang)
        << "即使将/帅有保护子，连续 3 次将军仍应判定为长将";
}

// ---- 复现用户报告：连续将军序列，历史正确传入时机器人应在步限处停止 ----
// 模拟"机器人连将"：每一步都把上一手纳入历史，验证第 3 手不会是将军（步限生效）。
TEST(ChessLongCheckTest, RobotLongCheckSequenceStopsAtLimit)
{
    const int nMaxJiang = 3;
    const int nMaxCatch = 3;

    int board[10][9];
    MakeLongCheckBoard(board);
    CChessLayout layout;
    layout.InitLayout(board, CS_RED);
    std::vector<MOVESTEP> history;

    // 第 1、2 手将军（构成 1 个完整循环，红方两次将军）
    AppendLongCheckCycle(layout, history);

    // 第 3 手：带完整历史搜索，机器人必须把第 3 次将军判为违规并改用他着
    MOVESTEP best = CChessAI::SearchBestMove(layout, 2, 0, history, nMaxJiang, nMaxCatch);
    ASSERT_FALSE(IsInvalidMove(best));

    // 关键断言：返回的不是第 3 次将军落点（4,5）
    EXPECT_FALSE(best.pt1.x == 3 && best.pt1.y == 5
                 && best.pt2.x == 4 && best.pt2.y == 5)
        << "复现用户场景：连续将军到第 3 步时必须停止，不得继续长将";

    EXPECT_FALSE(TriggersRepeatLoss(layout, history, best, nMaxJiang, nMaxCatch));
}

// ---- 攻击子顺带攻着其它无根子时，长将判定不能失效 ----
// 旧实现要求"攻击范围内恰好只有一个无根子"，因此红车在将军的同时若还威胁着另一个
// 无根黑子（实战里车、炮非常常见），nNoRootChs != 1 会直接 break，长将永远数不到
// 3 步 —— 这正是"机器人长将了不止 3 步"的真实原因。
// 本用例在长将局面下额外放一个无根黑马(0,5)，红车沿第 5 行顺带攻击它。
TEST(ChessLongCheckTest, LongCheckDetectedWhenCheckerAlsoAttacksUnrootedPiece)
{
    const int nMaxJiang = 3;

    int board[10][9];
    MakeLongCheckBoard(board);
    board[5][0] = CHSMAN_BLK_MA; // (0,5) 无根黑马：红车在第 5 行上顺带攻着它

    CChessLayout layout;
    layout.InitLayout(board, CS_RED);
    std::vector<MOVESTEP> history;

    AppendLongCheckCycle(layout, history); // 红车两次将军 + 黑将两次躲避

    POINT ptFrom = {3, 5};
    POINT ptTo = {4, 5};
    MOVESTEP candidate = layout.Move(ptFrom, ptTo); // 第 3 次将军
    history.push_back(candidate);

    CHESSMAN chsEnemy = CHSMAN_NULL;
    UINT nRepeat = CChessLayout::GetFarthestRepeat(layout, history, chsEnemy);
    EXPECT_EQ(CHSMAN_BLK_JIANG, chsEnemy)
        << "将军判定应优先认将/帅, 不能被顺带攻击的无根子顶掉";
    EXPECT_GE((int)nRepeat, nMaxJiang)
        << "将军同时攻着另一个无根子时, 长将仍必须被数出来";
}

// ---- 同上局面的机器人过滤：必须仍然在第 3 步改着 ----
TEST(ChessLongCheckTest, RobotAvoidsLongCheckWhenCheckerAlsoAttacksUnrootedPiece)
{
    const int nMaxJiang = 3;
    const int nMaxCatch = 3;

    int board[10][9];
    MakeLongCheckBoard(board);
    board[5][0] = CHSMAN_BLK_MA;

    CChessLayout layout;
    layout.InitLayout(board, CS_RED);
    std::vector<MOVESTEP> history;

    AppendLongCheckCycle(layout, history);

    MOVESTEP best = CChessAI::SearchBestMove(layout, 2, 0, history, nMaxJiang, nMaxCatch);
    ASSERT_FALSE(IsInvalidMove(best));

    EXPECT_FALSE(best.pt1.x == 3 && best.pt1.y == 5
                 && best.pt2.x == 4 && best.pt2.y == 5)
        << "机器人不应在顺带攻击其它无根子时继续长将";

    EXPECT_FALSE(TriggersRepeatLoss(layout, history, best, nMaxJiang, nMaxCatch));
}
