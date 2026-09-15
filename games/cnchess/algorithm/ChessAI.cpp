#include "stdafx.h"
#include "ChessAI.h"
#include "ChessBook.h"
#include "ChsAIEngine.h"
#include <vector>
#include <stdlib.h>

// ---------------------------------------------------------------------------
// CChessAI -> ChsAI 搜索引擎适配层
// 公开接口与旧版完全一致(见 ChessAI.h), 内部搜索算法已整体替换为
// jie65535/ChineseChess(MIT) 移植版引擎(ChsAIEngine.h/.cpp)。
// ---------------------------------------------------------------------------

namespace
{
    // 每线程一个引擎实例: 机器人 AI 在线程池中并发走子(最多 16 线程),
    // 引擎含置换表等有状态资源, 按 thread_local 隔离既避免加锁,
    // 也让同一池线程连续走子时复用置换表(每次搜索开启新代次老化旧条目)。
    // TT 取 2^18 条(约 4MB/线程), 16 线程上限约 64MB, 与旧共享表同量级。
    ChsAI::SearchEngine &GetEngine()
    {
        static thread_local ChsAI::SearchEngine s_engine(18);
        return s_engine;
    }

    // -------------------------------------------------------------------------
    // 朝向判定与坐标映射
    // 搜索引擎规范坐标系: 红方宫在低行(row 0..2), 红兵向高行进攻。
    // CChessLayout 的默认布局(kDefaultLayout)红方在低 y 侧, 但经 InitLayout/
    // ConvertLayout 后可能整体翻转, 故按红帅实际位置自动判定:
    //   红帅在低 y 侧 -> 直接映射 row=y; 否则垂直翻转 row=9-y。
    // 列方向不翻转(棋规左右对称, 两种取法等价, 恒取 col=x 最简单)。
    // -------------------------------------------------------------------------
    bool DetectRedTop(const CChessLayout &layout)
    {
        for (int y = 0; y < 10; y++)
            for (int x = 0; x < 9; x++)
                if (layout.m_chesses[y][x] == CHSMAN_RED_JIANG)
                    return y <= 4;
        // 无红帅(残局构造场景): 依黑将位置反推 —— 黑将在低 y 侧即红在底
        for (int y = 0; y < 10; y++)
            for (int x = 0; x < 9; x++)
                if (layout.m_chesses[y][x] == CHSMAN_BLK_JIANG)
                    return y >= 5;
        return true;
    }

    inline int LayoutRowToSearch(int y, bool bRedTop) { return bRedTop ? y : 9 - y; }
    inline int SearchRowToLayout(int row, bool bRedTop) { return bRedTop ? row : 9 - row; }

    // CChessLayout -> 搜索棋盘
    void LayoutToSearchBoard(const CChessLayout &layout, bool bRedTop, ChsAI::SearchBoard &sb)
    {
        for (int i = 0; i < ChsAI::SearchBoard::Size; i++)
            sb.Squares[i] = 0;
        sb.RedKingSq = -1;
        sb.BlackKingSq = -1;

        for (int y = 0; y < 10; y++)
        {
            for (int x = 0; x < 9; x++)
            {
                CHESSMAN cs = layout.m_chesses[y][x];
                if (cs == CHSMAN_NULL) continue;

                int type;   // SearchBoard::King..Pawn
                int color;  // +1 红 / -1 黑
                switch (cs)
                {
                case CHSMAN_RED_JIANG: type = ChsAI::SearchBoard::King;     color = ChsAI::SearchBoard::Red;   break;
                case CHSMAN_RED_SHI:   type = ChsAI::SearchBoard::Mandarin; color = ChsAI::SearchBoard::Red;   break;
                case CHSMAN_RED_XIANG: type = ChsAI::SearchBoard::Elephant; color = ChsAI::SearchBoard::Red;   break;
                case CHSMAN_RED_MA:    type = ChsAI::SearchBoard::Knight;   color = ChsAI::SearchBoard::Red;   break;
                case CHSMAN_RED_JU:    type = ChsAI::SearchBoard::Rook;     color = ChsAI::SearchBoard::Red;   break;
                case CHSMAN_RED_PAO:   type = ChsAI::SearchBoard::Cannon;   color = ChsAI::SearchBoard::Red;   break;
                case CHSMAN_RED_BING:  type = ChsAI::SearchBoard::Pawn;     color = ChsAI::SearchBoard::Red;   break;
                case CHSMAN_BLK_JIANG: type = ChsAI::SearchBoard::King;     color = ChsAI::SearchBoard::Black; break;
                case CHSMAN_BLK_SHI:   type = ChsAI::SearchBoard::Mandarin; color = ChsAI::SearchBoard::Black; break;
                case CHSMAN_BLK_XIANG: type = ChsAI::SearchBoard::Elephant; color = ChsAI::SearchBoard::Black; break;
                case CHSMAN_BLK_MA:    type = ChsAI::SearchBoard::Knight;   color = ChsAI::SearchBoard::Black; break;
                case CHSMAN_BLK_JU:    type = ChsAI::SearchBoard::Rook;     color = ChsAI::SearchBoard::Black; break;
                case CHSMAN_BLK_PAO:   type = ChsAI::SearchBoard::Cannon;   color = ChsAI::SearchBoard::Black; break;
                case CHSMAN_BLK_BING:  type = ChsAI::SearchBoard::Pawn;     color = ChsAI::SearchBoard::Black; break;
                default: continue; // 未知编码, 跳过(不应出现)
                }

                int row = LayoutRowToSearch(y, bRedTop);
                int sq = ChsAI::SearchBoard::Index(x, row);
                sb.Squares[sq] = (int8_t)(type * color);
                if (type == ChsAI::SearchBoard::King)
                {
                    if (color > 0) sb.RedKingSq = sq;
                    else sb.BlackKingSq = sq;
                }
            }
        }

        sb.SideToMove = (layout.m_actSide == CS_RED) ? ChsAI::SearchBoard::Red
                                                     : ChsAI::SearchBoard::Black;
        sb.ZobristKey = sb.ComputeZobristFromScratch();
    }

    // 搜索着法 -> MOVESTEP(布局坐标), enemy/nEnemyID 取自走子前的盘面快照
    MOVESTEP SearchMoveToStep(const CChessLayout &layout, bool bRedTop, uint16_t code)
    {
        MOVESTEP ms;
        int fromSq = ChsAI::SearchMove::FromOf(code);
        int toSq = ChsAI::SearchMove::ToOf(code);
        ms.pt1.x = ChsAI::SearchBoard::Col(fromSq);
        ms.pt1.y = SearchRowToLayout(ChsAI::SearchBoard::Row(fromSq), bRedTop);
        ms.pt2.x = ChsAI::SearchBoard::Col(toSq);
        ms.pt2.y = SearchRowToLayout(ChsAI::SearchBoard::Row(toSq), bRedTop);
        ms.enemy = layout.m_chesses[ms.pt2.y][ms.pt2.x];
        ms.nEnemyID = layout.m_nChsID[ms.pt2.y][ms.pt2.x];
        return ms;
    }

    MOVESTEP MakeInvalidStep()
    {
        MOVESTEP invalid;
        invalid.pt1.x = invalid.pt1.y = invalid.pt2.x = invalid.pt2.y = -1;
        invalid.enemy = CHSMAN_NULL;
        invalid.nEnemyID = 0;
        return invalid;
    }
}

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

    // 拷贝搜索棋盘快照: 搜索全程在快照上运行, 不触碰调用方的 layout
    const bool bRedTop = DetectRedTop(layout);
    ChsAI::SearchBoard sb;
    LayoutToSearchBoard(layout, bRedTop, sb);
    const int searchColor = sb.SideToMove;

    // 根节点先做合法着法普查: 无合法着法(将死/困毙/无子)直接返回无效走法,
    // 与旧版语义一致(判负信号由上层据此发出)
    {
        ChsAI::SearchMove rootMoves[ChsAI::MoveGenerator::MaxBuffer];
        int nRoot = ChsAI::MoveGenerator::Generate(sb, searchColor, rootMoves);
        int nLegal = 0;
        for (int i = 0; i < nRoot && nLegal == 0; i++)
        {
            sb.Make(rootMoves[i]);
            if (!sb.IsSideInCheck(searchColor))
                nLegal++;
            sb.Unmake(rootMoves[i]);
        }
        if (nLegal == 0)
            return MakeInvalidStep();
    }

    ChsAI::SearchEngine &engine = GetEngine();
    ChsAI::SearchResult result = engine.Search(sb, nDepth, nTimeMs);

    if (result.BestMove == 0)
    {
        // 预算过小, 连第 1 层都没算完: 不计时间补算 1 层, 保证永远返回合法着法
        // (根节点合法着法普查已确认至少存在 1 个, 此处必然得到有效着法)
        result = engine.Search(sb, 1, 0);
        if (result.BestMove == 0)
            return MakeInvalidStep(); // 理论上不可达, 双保险
    }

    // 严格采用搜索引擎的最佳着法(与 C# 原版一致)。
    // 注意: 不要在根节点做"等分随机挑选"——根节点子搜窗口随 alpha 收窄后,
    // fail-high 返回的边界值会与最佳分"伪同分"(实测自对弈中 64% 的等分候选
    // 为伪等分, 真实分值可差数百), 随机/优先吃子挑选会系统性劣化棋力。
    return SearchMoveToStep(layout, bRedTop, result.BestMove);
}
