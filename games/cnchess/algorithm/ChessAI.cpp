#include "stdafx.h"
#include "ChessAI.h"
#include "ChessBook.h"
#include <algorithm>
#include <vector>
#include <stdlib.h>
#include <mutex>
#include <chrono>

// ---------------------------------------------------------------------------
// 置换表(Transposition Table) + Zobrist 哈希
// 缓存搜索结果, 供相同局面再次到达时直接命中, 减少重复搜索并改进着法排序。
// 机器人 AI 在线程池中并发走子, 故用互斥量保护共享表, 避免数据竞争。
// ---------------------------------------------------------------------------
namespace
{
    enum { TT_BOUND_EXACT = 0, TT_BOUND_LOWER = 1, TT_BOUND_UPPER = 2 };

    const int              TT_SIZE = 1 << 16;             // 65536 项
    const unsigned long long TT_MASK = TT_SIZE - 1;

    struct TTEntry
    {
        unsigned long long key;   // 完整 Zobrist 键, 用于校验命中(0 表示空槽)
        int                depth; // 该条目的搜索深度
        int                bound; // 结点边界: 精确/下界/上界
        int                score; // 相对当前行棋方的分值
        int                bx1, by1, bx2, by2; // 最佳着法(用于着法排序)
    };
    TTEntry    g_tt[TT_SIZE];
    std::mutex g_ttLock;

    // Zobrist 随机表(按下标 = 棋子值低 4 位), 行棋方异或项; 首次使用前一次性初始化
    unsigned long long g_zob[10][9][16];
    const unsigned long long g_zobSide = 0x9d39247e33776d41ULL;
    std::once_flag g_zobOnce;

    void InitZobrist()
    {
        // 固定种子 LCG, 生成伪随机的 64 位 Zobrist 数(无需真正加密随机)
        unsigned long long s = 0x123456789abcdef0ULL;
        for (int y = 0; y < 10; y++)
            for (int x = 0; x < 9; x++)
                for (int c = 0; c < 16; c++)
                {
                    s = s * 2862933555777941757ULL + 3037000493ULL;
                    g_zob[y][x][c] = s;
                }
    }

    // 计算某局面的 Zobrist 键(黑方行棋时额外异或 g_zobSide 以区分行棋方)
    unsigned long long ComputeKey(const CChessLayout &layout)
    {
        std::call_once(g_zobOnce, InitZobrist);
        unsigned long long k = 0;
        for (int y = 0; y < 10; y++)
            for (int x = 0; x < 9; x++)
            {
                CHESSMAN cs = layout.m_chesses[y][x];
                if (cs != CHSMAN_NULL)
                    k ^= g_zob[y][x][cs & 0x0F];
            }
        if (layout.m_actSide == CS_BLACK)
            k ^= g_zobSide;
        return k;
    }

    // -------------------------------------------------------------------------
    // 杀手着法表: [ply][0/1] 记录在某层导致 beta 剪枝的安静的着法, 供同层其它局面
    // 优先尝试, 进一步提升着法排序与剪枝效率。
    // 每次搜索在机器人线程池的单一线程内进行, 用 thread_local 隔离, 既避免跨线程
    // 数据竞争, 也保证不同对局的杀手学习互不干扰。
    // -------------------------------------------------------------------------
    const int KILLER_MAX_PLY = 64;
    thread_local MOVESTEP g_killers[KILLER_MAX_PLY][2];

    void StoreKiller(int ply, const MOVESTEP &m)
    {
        if (m.pt1.x < 0 || ply >= KILLER_MAX_PLY)
            return;
        MOVESTEP (&k)[2] = g_killers[ply];
        if (k[0].pt1.x == m.pt1.x && k[0].pt1.y == m.pt1.y &&
            k[0].pt2.x == m.pt2.x && k[0].pt2.y == m.pt2.y)
            return; // 已存在则不再上浮
        k[1] = k[0];
        k[0] = m;
    }

    // -------------------------------------------------------------------------
    // 思考时间预算控制(thread_local: 每次走子仅作用于当前搜索线程, 与线程池并发安全)。
    // 用"每 256 个节点看一次真实时钟"的方式降低计时开销; 预算耗尽即置停搜标记,
    // 向上快速放弃当前迭代, 由根层丢弃未算完的结果并采用上一完整层的最佳着法。
    // -------------------------------------------------------------------------
    struct STimeCtrl
    {
        bool     enabled;      // 本次搜索是否受时间预算约束
        bool     stop;         // 停搜标记(预算耗尽)
        long long deadlineMs;  // steady_clock 毫秒截止时刻
        long long nodeCount;   // 已搜索节点数(驱动周期性查表)
    };
    thread_local STimeCtrl g_time = { false, false, 0, 0 };

    long long NowMs()
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    }

    // 预算检查: 返回 true 表示应停止搜索
    bool CheckTime()
    {
        if (!g_time.enabled)
            return false;
        if (g_time.stop)
            return true;
        if ((++g_time.nodeCount & 255) == 0 && NowMs() >= g_time.deadlineMs)
            g_time.stop = true;
        return g_time.stop;
    }
}

// 局面评估与搜索的常量
const int CChessAI::KR_VAL[7] = { 10000, 900, 400, 450, 200, 200, 100 }; // 将车马炮士相兵
const int CChessAI::INF = 1000000000;
const int CChessAI::CHECKMATE = 100000;

// ---------------------------------------------------------------------------
// 各棋子位置评分表(棋子类型 0..6 = 将车马炮士相兵)。
// 坐标系: 行 r 表示"本方从第0行向对方推进"(r=0 本方王宫, r=9 对方底线), 列 x 为 0..8。
// 值以"兵=100"为基准, 用于补充纯子力, 提升 AI 的开局站位与子力配合。
// ---------------------------------------------------------------------------
static const int kJuPos[10][9] = {
    { -6, 0, 0, 0, 0, 0, 0, 0, -6 },
    { 2, 4, 4, 4, 4, 4, 4, 4, 2 },
    { 8, 10, 10, 10, 10, 10, 10, 10, 8 },
    { 14, 16, 16, 16, 16, 16, 16, 16, 14 },
    { 20, 20, 20, 20, 22, 20, 20, 20, 20 },
    { 22, 24, 24, 24, 24, 24, 24, 24, 22 },
    { 24, 24, 24, 24, 24, 24, 24, 24, 24 },
    { 24, 26, 26, 26, 26, 26, 26, 26, 24 },
    { 22, 24, 24, 24, 24, 24, 24, 24, 22 },
    { 10, 12, 14, 16, 18, 16, 14, 12, 10 }
};

static const int kMaPos[10][9] = {
    { 90, 90, 90, 96, 90, 96, 90, 90, 90 },
    { 90, 96, 103, 90, 96, 90, 103, 96, 90 },
    { 92, 98, 99, 103, 99, 103, 99, 98, 92 },
    { 90, 96, 103, 99, 98, 99, 103, 96, 90 },
    { 82, 88, 95, 98, 100, 98, 95, 88, 82 },
    { 78, 84, 88, 92, 96, 92, 88, 84, 78 },
    { 68, 76, 82, 84, 90, 84, 82, 76, 68 },
    { 62, 70, 74, 78, 82, 78, 74, 70, 62 },
    { 58, 62, 66, 70, 74, 70, 66, 62, 58 },
    { 56, 58, 60, 64, 68, 64, 60, 58, 56 }
};

static const int kPaoPos[10][9] = {
    { 6, 4, 0, -10, -12, -10, 0, 4, 6 },
    { 2, 2, 0, -4, -14, -4, 0, 2, 2 },
    { 2, 2, 0, -10, -8, -10, 0, 2, 2 },
    { 0, 0, -2, 4, 10, 4, -2, 0, 0 },
    { 0, 0, 0, 2, 8, 2, 0, 0, 0 },
    { -2, 0, 4, 2, 6, 2, 4, 0, -2 },
    { 0, 0, 0, 2, 4, 2, 0, 0, 0 },
    { 4, 0, 8, 6, 10, 6, 8, 0, 4 },
    { 0, 2, 4, 6, 6, 6, 4, 2, 0 },
    { 0, 0, 2, 6, 6, 6, 2, 0, 0 }
};

static const int kJiangPos[10][9] = {
    { 0, 0, 0, 2, 12, 2, 0, 0, 0 },
    { 0, 0, 0, -4, -8, -4, 0, 0, 0 },
    { 0, 0, 0, -10, -16, -10, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

static const int kShiPos[10][9] = {
    { 0, 0, 0, 4, 0, 4, 0, 0, 0 },
    { 0, 0, 0, 0, -2, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 8, 0, 0, 0, 0 },
    { 0, 0, 0, 2, 0, 2, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

static const int kXiangPos[10][9] = {
    { 0, 0, 6, 0, 0, 0, 6, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 4, 0, 0, 0, 8, 0, 0, 0, 4 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 6, 0, 4, 0, 6, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

static const int kBingPos[10][9] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 10, 14, 20, 24, 26, 24, 20, 14, 10 },
    { 16, 22, 30, 36, 40, 36, 30, 22, 16 },
    { 44, 54, 64, 70, 76, 70, 64, 54, 44 },
    { 54, 60, 66, 70, 78, 70, 66, 60, 54 },
    { 60, 66, 70, 72, 76, 72, 70, 66, 60 },
    { 64, 68, 70, 70, 72, 70, 70, 68, 64 },
    { 62, 66, 66, 66, 64, 66, 66, 66, 62 },
    { 56, 60, 62, 62, 62, 62, 62, 60, 56 },
    { 50, 56, 58, 58, 60, 58, 58, 56, 50 }
};

static const int * const kPosTables[7] = {
    (const int *)kJiangPos,   // 0 将
    (const int *)kJuPos,      // 1 车
    (const int *)kMaPos,      // 2 马
    (const int *)kPaoPos,     // 3 炮
    (const int *)kShiPos,     // 4 士
    (const int *)kXiangPos,   // 5 相
    (const int *)kBingPos     // 6 兵
};

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
                // 将死即止:AI 不允许吃掉对方将(吃将/飞将),将死由"无合法着法"判定
                CHESSMAN dst = layout.m_chesses[pt2.y][pt2.x];
                if (dst == CHSMAN_RED_JIANG || dst == CHSMAN_BLK_JIANG)
                    continue;
                POINT ptBegin = { x, y };
                CHESSMAN enemy = dst;
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

            // 位置评分: 按"本方王宫在第0行"的规范坐标系查表补充站位与子力配合
            int type = cs % 7;
            const int (*posTable)[9] = (const int (*)[9])kPosTables[type];
            int tr = (s == CS_RED) ? y : (9 - y); // 红方视角行(黑方垂直镜像)
            v += posTable[tr][x];

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
    if (CheckTime())
        return 0; // 预算耗尽, 快速放弃(该值随后被根层整体丢弃)
    if (depth <= 0)
        return Quiesce(layout, alpha, beta, ply);

    unsigned long long key = ComputeKey(layout);
    int tx1 = -1, ty1 = 0, tx2 = 0, ty2 = 0; // 命中时给出的最佳着法候选
    {
        std::lock_guard<std::mutex> lk(g_ttLock);
        const TTEntry &e = g_tt[key & TT_MASK];
        if (e.key == key)
        {
            if (e.depth >= depth)
            {
                if (e.bound == TT_BOUND_EXACT)          return e.score;
                if (e.bound == TT_BOUND_LOWER && e.score >= beta) return e.score;
                if (e.bound == TT_BOUND_UPPER && e.score <= alpha) return e.score;
            }
            tx1 = e.bx1; ty1 = e.by1; tx2 = e.bx2; ty2 = e.by2;
        }
    }

    std::vector<MOVESTEP> moves;
    GenerateLegalMoves(layout, moves);
    if (moves.empty())
        return -(CHECKMATE - ply); // 当前行棋方无子可走(将死或困毙)，判负

    // 着法排序: 置换表最佳 > 吃子(按被吃子力) > 杀手着法 > 其它, 以收紧 alpha-beta 剪枝
    const MOVESTEP (&kills)[2] = g_killers[ply];
    std::stable_sort(moves.begin(), moves.end(), [&](const MOVESTEP &a, const MOVESTEP &b) {
        int sa = 0, sb = 0;
        if (tx1 >= 0 && a.pt1.x == tx1 && a.pt1.y == ty1 && a.pt2.x == tx2 && a.pt2.y == ty2)
            sa += 1000000;
        if (tx1 >= 0 && b.pt1.x == tx1 && b.pt1.y == ty1 && b.pt2.x == tx2 && b.pt2.y == ty2)
            sb += 1000000;
        if (a.enemy != CHSMAN_NULL) sa += 100000 + PieceValue(a.enemy);
        if (b.enemy != CHSMAN_NULL) sb += 100000 + PieceValue(b.enemy);
        for (int s = 0; s < 2; s++)
        {
            if (a.pt1.x == kills[s].pt1.x && a.pt1.y == kills[s].pt1.y &&
                a.pt2.x == kills[s].pt2.x && a.pt2.y == kills[s].pt2.y)
                sa += 10000 - s * 1000;
            if (b.pt1.x == kills[s].pt1.x && b.pt1.y == kills[s].pt1.y &&
                b.pt2.x == kills[s].pt2.x && b.pt2.y == kills[s].pt2.y)
                sb += 10000 - s * 1000;
        }
        return sa > sb;
    });

    int origAlpha = alpha;
    int best = -INF;
    MOVESTEP bestMove = moves[0];
    for (size_t i = 0; i < moves.size(); i++)
    {
        MOVESTEP ms = layout.Move(moves[i].pt1, moves[i].pt2);
        int score = -Negamax(layout, depth - 1, -beta, -alpha, ply + 1);
        layout.UndoMove(ms);
        if (score > best) { best = score; bestMove = moves[i]; }
        if (best > alpha) alpha = best;
        if (alpha >= beta)
        {
            // 该安静着法导致剪枝, 记为本层杀手着法供同层其它局面优先尝试
            if (moves[i].enemy == CHSMAN_NULL)
                StoreKiller(ply, moves[i]);
            break;
        }
    }

    // 判定结点边界: 未提升上界(fail-low)存上界, 剪枝(fail-high)存下界, 否则精确
    int bound;
    if (best <= origAlpha)      bound = TT_BOUND_UPPER;
    else if (best >= beta)      bound = TT_BOUND_LOWER;
    else                        bound = TT_BOUND_EXACT;

    // 将死/困毙分值随 ply 变化, 不参与缓存, 避免跨深度复用导致距离失真
    if (best > -(CHECKMATE - 200) && best < (CHECKMATE - 200))
    {
        std::lock_guard<std::mutex> lk(g_ttLock);
        TTEntry &e = g_tt[key & TT_MASK];
        e.key = key; e.depth = depth; e.bound = bound; e.score = best;
        e.bx1 = bestMove.pt1.x; e.by1 = bestMove.pt1.y;
        e.bx2 = bestMove.pt2.x; e.by2 = bestMove.pt2.y;
    }
    return best;
}

// 静态搜索(quiescence): 在搜索深度归零后只延展吃子着法, 缓解"水平线效应"——
// 防止临近搜索边界时, 对手紧接着的"吃子/反吃子"连续着法被误判为对己方有利。
int CChessAI::Quiesce(CChessLayout &layout, int alpha, int beta, int ply)
{
    if (CheckTime())
        return 0; // 预算耗尽, 快速放弃(该值随后被根层整体丢弃)

    // 先作静态结点评估(stand-pat), 若已超出 beta 则无需搜索吃子
    int stand = Evaluate(layout);
    if (stand >= beta)
        return beta;
    if (stand > alpha)
        alpha = stand;

    // 生成全部合法着法, 仅保留吃子着法(GenerateLegalMoves 已校验不送将、不含吃将/飞将)
    std::vector<MOVESTEP> all;
    GenerateLegalMoves(layout, all);
    std::vector<MOVESTEP> caps;
    for (size_t i = 0; i < all.size(); i++)
        if (all[i].enemy != CHSMAN_NULL)
            caps.push_back(all[i]);
    if (caps.empty())
        return alpha;

    // 按被吃子力降序, 提升 alpha-beta 剪枝效率
    std::sort(caps.begin(), caps.end(), [](const MOVESTEP &a, const MOVESTEP &b) {
        return PieceValue(a.enemy) > PieceValue(b.enemy);
    });

    for (size_t i = 0; i < caps.size(); i++)
    {
        MOVESTEP ms = layout.Move(caps[i].pt1, caps[i].pt2);
        int score = -Quiesce(layout, -beta, -alpha, ply + 1);
        layout.UndoMove(ms);
        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }
    return alpha;
}

MOVESTEP CChessAI::SearchBestMove(CChessLayout &layout, int nDepth)
{
    return SearchBestMove(layout, nDepth, 0); // 0 == 不限时, 纯迭代加深
}

MOVESTEP CChessAI::SearchBestMove(CChessLayout &layout, int nDepth, int nTimeMs)
{
    // 开局阶段先查经验数据库, 命中则直接采用(补充开局经验, 避免搜索浪费)
    POINT book1, book2;
    if (CChessOpeningBook::Probe(layout, book1, book2))
    {
        MOVESTEP ms;
        ms.pt1 = book1;
        ms.pt2 = book2;
        ms.enemy = layout.m_chesses[book2.y][book2.x];
        ms.nEnemyID = layout.m_nChsID[book2.y][book2.x];
        return ms;
    }

    std::vector<MOVESTEP> moves;
    GenerateLegalMoves(layout, moves);

    MOVESTEP invalid;
    invalid.pt1.x = invalid.pt1.y = invalid.pt2.x = invalid.pt2.y = -1;
    invalid.enemy = CHSMAN_NULL;
    invalid.nEnemyID = 0;
    if (moves.empty()) return invalid;

    // 设定本线程本次搜索的时间预算(nTimeMs>0 时), ≤0 表示不限时
    const bool prevEnabled = g_time.enabled;
    if (nTimeMs > 0)
    {
        g_time.enabled = true;
        g_time.stop = false;
        g_time.nodeCount = 0;
        g_time.deadlineMs = NowMs() + nTimeMs;
    }
    else
    {
        g_time.enabled = false;
        g_time.stop = false;
    }

    // 迭代加深: 从 1 递增到 nDepth 逐层加深。
    // 浅层搜索结果会预热置换表与杀手着法, 并把上一层的最佳着法提到根节点最前,
    // 使深层搜索获得更好的着法排序与剪枝。
    std::vector<MOVESTEP> completed;
    for (int d = 1; d <= nDepth; d++)
    {
        // 用上一"已完整算完"层的最佳着法为根节点排序, 提升本轮剪枝
        if (d > 1 && !completed.empty())
        {
            const MOVESTEP &hint = completed[0];
            for (size_t i = 0; i < moves.size(); i++)
            {
                if (moves[i].pt1.x == hint.pt1.x && moves[i].pt1.y == hint.pt1.y &&
                    moves[i].pt2.x == hint.pt2.x && moves[i].pt2.y == hint.pt2.y)
                {
                    std::swap(moves[0], moves[i]);
                    break;
                }
            }
        }
        std::vector<MOVESTEP> cand;
        SearchRoot(layout, moves, d, -INF, INF, cand);
        if (g_time.stop)
            break; // 该层未算完, 丢弃, 采用上一个完整层的结果
        completed = cand;
    }

    // 恢复线程级的上一状态(正常是未启用)
    g_time.enabled = prevEnabled;
    g_time.stop = false;

    if (completed.empty())
    {
        // 预算过小, 连第 1 层都没算完: 忽略预算强制补算一层, 保证永远返回合法着法
        g_time.enabled = false;
        g_time.stop = false;
        SearchRoot(layout, moves, 1, -INF, INF, completed);
        if (completed.empty())
            completed.push_back(moves[0]);
        g_time.enabled = prevEnabled;
    }
    // 多个等优走法随机取一，增加棋风变化
    return completed[rand() % (int)completed.size()];
}

int CChessAI::SearchRoot(CChessLayout &layout, std::vector<MOVESTEP> &moves,
                         int depth, int alpha, int beta, std::vector<MOVESTEP> &candidates)
{
    int best = -INF;
    candidates.clear();
    for (size_t i = 0; i < moves.size(); i++)
    {
        MOVESTEP ms = layout.Move(moves[i].pt1, moves[i].pt2);
        int score = -Negamax(layout, depth - 1, -beta, -alpha, 1);
        layout.UndoMove(ms);
        if (g_time.stop)
            break; // 预算耗尽: 放弃本迭代, 已填的部分候选交由调用方丢弃
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
    return best;
}