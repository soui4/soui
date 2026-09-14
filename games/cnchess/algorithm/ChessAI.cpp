#include "stdafx.h"
#include "ChessAI.h"
#include <algorithm>
#include <vector>
#include <stdlib.h>

// 局面评估与搜索的常量
const int CChessAI::KR_VAL[7] = { 10000, 900, 400, 450, 200, 200, 100 }; // 将车马炮士相兵
const int CChessAI::INF = 1000000000;
const int CChessAI::CHECKMATE = 100000;

int CChessAI::LevelToDepth(int nLevel)
{
    switch (nLevel)
    {
    case 1: return ROBOT_AI_DEPTH_BEGINNER; // 初级
    case 2: return ROBOT_AI_DEPTH_MEDIUM;   // 中级
    case 3: return ROBOT_AI_DEPTH_ADVANCED; // 高级
    default: return ROBOT_AI_DEPTH_BEGINNER;
    }
}

int CChessAI::PieceValue(CHESSMAN chs)
{
    return KR_VAL[chs % 7];
}

// 检测 (tx,ty) 是否被 bySide 一方沿 (dx,dy) 直线方向攻击(车/将对面/炮)。
// 返回 true 表示该方向上存在 bySide 的车、正对将或架一炮的炮能攻击到 (tx,ty)。
bool CChessAI::IsSquareAttacked(const CChessLayout &layout, int tx, int ty, CHSSIDE bySide)
{
    static const int kDir[4][2] = { { 0, -1 }, { 0, 1 }, { -1, 0 }, { 1, 0 } };
    const CHESSMAN kJu   = (bySide == CS_RED) ? CHSMAN_RED_JU   : CHSMAN_BLK_JU;
    const CHESSMAN kJi   = (bySide == CS_RED) ? CHSMAN_RED_JIANG: CHSMAN_BLK_JIANG;
    const CHESSMAN kPao  = (bySide == CS_RED) ? CHSMAN_RED_PAO  : CHSMAN_BLK_PAO;
    const CHESSMAN kMa   = (bySide == CS_RED) ? CHSMAN_RED_MA   : CHSMAN_BLK_MA;
    const CHESSMAN kBing = (bySide == CS_RED) ? CHSMAN_RED_BING : CHSMAN_BLK_BING;

    // 1) 车 / 将(对面) / 炮：沿横竖直线扫描
    for (int d = 0; d < 4; d++)
    {
        int x = tx, y = ty;
        bool bScreened = false;
        for (;;)
        {
            x += kDir[d][0];
            y += kDir[d][1];
            if (x < 0 || x > 8 || y < 0 || y > 9) break;
            CHESSMAN cs = layout.m_chesses[y][x];
            if (cs == CHSMAN_NULL) continue;

            if (!bScreened)
            {
                if (CHSMANSIDE(cs) != bySide)
                {
                    // 第一颗棋子是对方的，作为炮架
                    bScreened = true;
                    continue;
                }
                // 第一颗是 bySide 的棋子
                if (cs == kJu) return true;                 // 车直接攻击
                if (kDir[d][0] == 0 && cs == kJi) return true; // 将帅对面(同列通视)
                break; // 其它 bySide 棋子挡路，该方向无效
            }
            else
            {
                // 已有一颗炮架，返回时需要是 bySide 的炮
                if (CHSMANSIDE(cs) == bySide)
                    return (cs == kPao);
                break; // 出现两颗非目标侧棋子
            }
        }
    }

    // 2) 马：骑士步，注意蹩马腿
    static const int kHorse[8][2] = {
        { 1, 2 }, { 2, 1 }, { 2, -1 }, { 1, -2 },
        { -1, -2 }, { -2, -1 }, { -2, 1 }, { -1, 2 }
    };
    for (int i = 0; i < 8; i++)
    {
        int hx = tx + kHorse[i][0];
        int hy = ty + kHorse[i][1];
        if (hx < 0 || hx > 8 || hy < 0 || hy > 9) continue;
        if (layout.m_chesses[hy][hx] != kMa) continue;
        // 计算蹩马腿位置：马朝目标先行一格正交的落脚点
        int legX = hx, legY = hy;
        if (abs(kHorse[i][0]) > abs(kHorse[i][1]))
            legX = hx - (kHorse[i][0] > 0 ? 1 : -1);
        else
            legY = hy - (kHorse[i][1] > 0 ? 1 : -1);
        if (layout.m_chesses[legY][legX] == CHSMAN_NULL)
            return true;
    }

    // 3) 兵：向前一格 + 过河后左右一格
    if (bySide == CS_RED)
    {
        if (ty - 1 >= 0 && layout.m_chesses[ty - 1][tx] == kBing) return true; // 红兵向下
        if (ty >= 5) // 横向攻击的兵已过河
        {
            if (tx - 1 >= 0 && layout.m_chesses[ty][tx - 1] == kBing) return true;
            if (tx + 1 <= 8 && layout.m_chesses[ty][tx + 1] == kBing) return true;
        }
    }
    else
    {
        if (ty + 1 <= 9 && layout.m_chesses[ty + 1][tx] == kBing) return true; // 黑兵向上
        if (ty <= 4)
        {
            if (tx - 1 >= 0 && layout.m_chesses[ty][tx - 1] == kBing) return true;
            if (tx + 1 <= 8 && layout.m_chesses[ty][tx + 1] == kBing) return true;
        }
    }

    return false;
}

bool CChessAI::IsJiangInCheck(const CChessLayout &layout, CHSSIDE side)
{
    CHESSMAN jing = (side == CS_RED) ? CHSMAN_RED_JIANG : CHSMAN_BLK_JIANG;
    for (int y = 0; y < 10; y++)
        for (int x = 0; x < 9; x++)
            if (layout.m_chesses[y][x] == jing)
                return IsSquareAttacked(layout, x, y, (CHSSIDE)(!side));
    // 将已不存在(被吃/正对吃掉)，视为失败方
    return true;
}

void CChessAI::GenerateLegalMoves(CChessLayout &layout, std::vector<MOVESTEP> &out)
{
    out.clear();
    CHSSIDE mover = layout.m_actSide;
    POINT ptMoves[MAX_MOVES];
    for (int y = 0; y < 10; y++)
    {
        for (int x = 0; x < 9; x++)
        {
            CHESSMAN cs = layout.m_chesses[y][x];
            if (cs == CHSMAN_NULL) continue;
            if (CHSMANSIDE(cs) != mover) continue;

            int n = layout.GetPossiableMoves(x, y, ptMoves);
            for (int i = 0; i < n; i++)
            {
                POINT pt2 = ptMoves[i];
                POINT ptBegin = { x, y };
                CHESSMAN enemy = layout.m_chesses[pt2.y][pt2.x];
                int nEnemyID = layout.m_nChsID[pt2.y][pt2.x];
                MOVESTEP ms = layout.Move(ptBegin, pt2);
                if (!IsJiangInCheck(layout, mover))
                {
                    MOVESTEP m;
                    m.pt1.x = x; m.pt1.y = y;
                    m.pt2 = pt2;
                    m.enemy = enemy;
                    m.nEnemyID = nEnemyID;
                    out.push_back(m);
                }
                layout.UndoMove(ms);
            }
        }
    }

    // 吃子优先排序，提升 alpha-beta 剪枝效率
    std::sort(out.begin(), out.end(), [](const MOVESTEP &a, const MOVESTEP &b) {
        return PieceValue(a.enemy) > PieceValue(b.enemy);
    });
}

int CChessAI::Evaluate(const CChessLayout &layout)
{
    CHSSIDE mover = layout.m_actSide;
    int nMy = 0, nEn = 0;
    for (int y = 0; y < 10; y++)
    {
        for (int x = 0; x < 9; x++)
        {
            CHESSMAN cs = layout.m_chesses[y][x];
            if (cs == CHSMAN_NULL) continue;
            int v = PieceValue(cs);
            CHSSIDE s = CHSMANSIDE(cs);
            if (s == CS_RED)
            {
                // 红兵过河加分
                if (cs == CHSMAN_RED_BING)
                    v += (y >= 5) ? 40 : ((y >= 3) ? (y - 2) * 10 : 0);
            }
            else if (s == CS_BLACK)
            {
                if (cs == CHSMAN_BLK_BING)
                    v += (y <= 4) ? 40 : ((y <= 6) ? (6 - y) * 10 : 0);
            }
            if (s == mover) nMy += v;
            else nEn += v;
        }
    }
    return nMy - nEn;
}

int CChessAI::Negamax(CChessLayout &layout, int depth, int alpha, int beta, int ply)
{
    if (depth <= 0)
        return Evaluate(layout);

    std::vector<MOVESTEP> moves;
    GenerateLegalMoves(layout, moves);
    if (moves.empty())
        return -(CHECKMATE - ply); // 当前行棋方无子可走(将死或困毙)，判负

    int best = -INF;
    for (size_t i = 0; i < moves.size(); i++)
    {
        MOVESTEP ms = layout.Move(moves[i].pt1, moves[i].pt2);
        int score = -Negamax(layout, depth - 1, -beta, -alpha, ply + 1);
        layout.UndoMove(ms);
        if (score > best) best = score;
        if (best > alpha) alpha = best;
        if (alpha >= beta) break;
    }
    return best;
}

MOVESTEP CChessAI::SearchBestMove(CChessLayout &layout, int nDepth)
{
    std::vector<MOVESTEP> moves;
    GenerateLegalMoves(layout, moves);

    MOVESTEP invalid;
    invalid.pt1.x = invalid.pt1.y = invalid.pt2.x = invalid.pt2.y = -1;
    invalid.enemy = CHSMAN_NULL;
    invalid.nEnemyID = 0;
    if (moves.empty()) return invalid;

    int alpha = -INF, beta = INF, best = -INF;
    std::vector<MOVESTEP> candidates;
    for (size_t i = 0; i < moves.size(); i++)
    {
        MOVESTEP ms = layout.Move(moves[i].pt1, moves[i].pt2);
        int score = -Negamax(layout, nDepth - 1, -beta, -alpha, 1);
        layout.UndoMove(ms);
        if (score > best)
        {
            best = score;
            candidates.clear();
            candidates.push_back(moves[i]);
        }
        else if (score == best)
        {
            candidates.push_back(moves[i]);
        }
        if (best > alpha) alpha = best;
        if (alpha >= beta) break;
    }
    // 多个等优走法随机取一，增加棋风变化
    return candidates[rand() % candidates.size()];
}