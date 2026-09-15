/**
 * @file ChsAIEngine.cpp
 * @brief 中国象棋 alpha-beta 搜索引擎实现
 *
 * 移植自 jie65535/ChineseChess 的 ChineseChess.Core/AI 模块
 * (https://github.com/jie65535/ChineseChess, MIT License, Copyright (c) 2021)。
 * 算法语义与 C# 原版保持一致, 差异仅在:
 *  - Span/stackalloc 换成定长数组
 *  - CancellationToken 换成纯时间预算控制(调用方为阻塞式单次搜索)
 *  - AlphaBeta 增加 ply 上限保护(防止将军延伸链无限递归导致杀手表越界)
 */
#include "ChsAIEngine.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

namespace ChsAI
{
    // =========================================================================
    // Zobrist 哈希(移植自 Zobrist.cs): 每个 (棋子编码 x 格子) 一个 64 位随机数,
    // 外加行棋方异或项。用确定性种子, 便于调试与置换表稳定。
    // =========================================================================
    namespace
    {
        uint64_t g_zobPiece[14][SearchBoard::Size];
        uint64_t g_zobSide = 0;
        struct SZobristInit
        {
            SZobristInit()
            {
                // 固定种子的 64 位 LCG(C++ 静态初始化线程安全, 只执行一次)
                uint64_t s = 0x5A17B0A6ULL;
                for (int p = 0; p < 14; p++)
                    for (int sq = 0; sq < SearchBoard::Size; sq++)
                    {
                        s = s * 6364136223846793005ULL + 1442695040888963407ULL;
                        g_zobPiece[p][sq] = s;
                    }
                s = s * 6364136223846793005ULL + 1442695040888963407ULL;
                g_zobSide = s;
            }
        };
        void InitZobrist()
        {
            static SZobristInit s_init;
        }

        /** 棋子编码(±1..±7)映射到 0..13 */
        inline int PieceIndex(int pieceVal)
        {
            return pieceVal > 0 ? pieceVal - 1 : 6 - pieceVal;
        }
        inline uint64_t PieceAt(int pieceVal, int sq)
        {
            if (pieceVal == 0) return 0ULL;
            return g_zobPiece[PieceIndex(pieceVal)][sq];
        }

        // 生成器用的方向常量
        const int kOrthDc[4] = { 1, -1, 0, 0 };
        const int kOrthDr[4] = { 0, 0, 1, -1 };
        const int kDiagDc[4] = { 1, 1, -1, -1 };
        const int kDiagDr[4] = { 1, -1, 1, -1 };
        const int kKnightDc[8] = { 1, 1, -1, -1, 2, 2, -2, -2 };
        const int kKnightDr[8] = { 2, -2, 2, -2, 1, -1, 1, -1 };
    }

    // =========================================================================
    // SearchBoard
    // =========================================================================
    void SearchBoard::CloneFrom(const SearchBoard &src)
    {
        memcpy(Squares, src.Squares, sizeof(Squares));
        SideToMove = src.SideToMove;
        RedKingSq = src.RedKingSq;
        BlackKingSq = src.BlackKingSq;
        ZobristKey = src.ZobristKey;
    }

    uint64_t SearchBoard::ComputeZobristFromScratch()
    {
        InitZobrist();
        uint64_t key = 0;
        for (int sq = 0; sq < Size; sq++)
        {
            int p = Squares[sq];
            if (p != 0) key ^= PieceAt(p, sq);
        }
        if (SideToMove == Black) key ^= g_zobSide;
        return key;
    }

    void SearchBoard::Make(const SearchMove &m)
    {
        int from = m.From;
        int to = m.To;
        int piece = Squares[from];
        int captured = Squares[to];
        // 注: Piece/Captured 由生成器已填好; 此处直接用盘面值保证一致性
        // (与 C# 版 Make(ref m) 回填字段不同, C++ 侧 SearchMove 按值传递, 无需回填)

        ZobristKey ^= PieceAt(piece, from);
        if (captured != 0) ZobristKey ^= PieceAt(captured, to);

        Squares[to] = (int8_t)piece;
        Squares[from] = 0;

        ZobristKey ^= PieceAt(piece, to);

        if (piece == King) RedKingSq = to;
        else if (piece == -King) BlackKingSq = to;
        else if (captured == King) RedKingSq = -1;
        else if (captured == -King) BlackKingSq = -1;

        SideToMove = -SideToMove;
        ZobristKey ^= g_zobSide;
    }

    void SearchBoard::Unmake(const SearchMove &m)
    {
        int from = m.From;
        int to = m.To;
        int piece = m.Piece;
        int captured = m.Captured;

        SideToMove = -SideToMove;
        ZobristKey ^= g_zobSide;

        ZobristKey ^= PieceAt(piece, to);

        Squares[from] = (int8_t)piece;
        Squares[to] = (int8_t)captured;

        ZobristKey ^= PieceAt(piece, from);
        if (captured != 0) ZobristKey ^= PieceAt(captured, to);

        if (piece == King) RedKingSq = from;
        else if (piece == -King) BlackKingSq = from;
        if (captured == King) RedKingSq = to;
        else if (captured == -King) BlackKingSq = to;
    }

    void SearchBoard::MakeNullMove()
    {
        SideToMove = -SideToMove;
        ZobristKey ^= g_zobSide;
    }

    void SearchBoard::UnmakeNullMove()
    {
        SideToMove = -SideToMove;
        ZobristKey ^= g_zobSide;
    }

    bool SearchBoard::IsSideInCheck(int color) const
    {
        int kingSq = color == Red ? RedKingSq : BlackKingSq;
        if (kingSq < 0) return true;
        return IsSquareAttacked(kingSq, -color);
    }

    bool SearchBoard::IsSquareAttacked(int sq, int byColor) const
    {
        int sqCol = Col(sq);
        int sqRow = Row(sq);

        // 兵/卒: 正前方一格 + 过河后的横向两格
        int pawnVal = byColor * Pawn;
        int srcRow = sqRow - byColor; // 红: 来自南(行号小的一侧); 黑: 来自北
        if (srcRow >= 0 && srcRow < Height)
        {
            if (Squares[Index(sqCol, srcRow)] == pawnVal) return true;
        }
        bool sidewaysOk = byColor > 0 ? sqRow >= 5 : sqRow <= 4;
        if (sidewaysOk)
        {
            if (sqCol > 0 && Squares[Index(sqCol - 1, sqRow)] == pawnVal) return true;
            if (sqCol < Width - 1 && Squares[Index(sqCol + 1, sqRow)] == pawnVal) return true;
        }

        // 将/帅一步邻接(仅当被攻击格在攻击方九宫内才有意义)
        int kingVal = byColor * King;
        if (InPalace(sqCol, sqRow, byColor))
        {
            if (sqCol > 0 && Squares[Index(sqCol - 1, sqRow)] == kingVal) return true;
            if (sqCol < Width - 1 && Squares[Index(sqCol + 1, sqRow)] == kingVal) return true;
            if (sqRow > 0 && Squares[Index(sqCol, sqRow - 1)] == kingVal) return true;
            if (sqRow < Height - 1 && Squares[Index(sqCol, sqRow + 1)] == kingVal) return true;
        }

        // 飞将: 被攻击格上是己方将帅, 且与对方将帅同列、中间无子
        int defenderKing = -byColor * King;
        if (Squares[sq] == defenderKing)
        {
            int otherKingSq = byColor == Red ? RedKingSq : BlackKingSq;
            if (otherKingSq >= 0 && Col(otherKingSq) == sqCol)
            {
                int r1 = (sqRow < Row(otherKingSq) ? sqRow : Row(otherKingSq)) + 1;
                int r2 = (sqRow > Row(otherKingSq) ? sqRow : Row(otherKingSq)) - 1;
                bool clear = true;
                for (int r = r1; r <= r2; r++)
                {
                    if (Squares[Index(sqCol, r)] != 0) { clear = false; break; }
                }
                if (clear) return true;
            }
        }

        // 士/仕: 仅九宫内对角一步
        int mandarinVal = byColor * Mandarin;
        if (InPalace(sqCol, sqRow, byColor))
        {
            for (int i = 0; i < 4; i++)
            {
                int dc = (i & 1) == 0 ? -1 : 1;
                int dr = (i & 2) == 0 ? -1 : 1;
                int c = sqCol + dc, r = sqRow + dr;
                if (c < 0 || c >= Width || r < 0 || r >= Height) continue;
                if (!InPalace(c, r, byColor)) continue;
                if (Squares[Index(c, r)] == mandarinVal) return true;
            }
        }

        // 象/相: 不过河, 象眼无子
        bool sqOnAttackerSide = byColor > 0 ? sqRow <= 4 : sqRow >= 5;
        if (sqOnAttackerSide)
        {
            int elephantVal = byColor * Elephant;
            const int dcs[4] = { -2, -2, 2, 2 };
            const int drs[4] = { -2, 2, -2, 2 };
            for (int i = 0; i < 4; i++)
            {
                int dc = dcs[i], dr = drs[i];
                int c = sqCol + dc, r = sqRow + dr;
                if (c < 0 || c >= Width || r < 0 || r >= Height) continue;
                if (Squares[Index(c, r)] != elephantVal) continue;
                int eyeC = sqCol + dc / 2, eyeR = sqRow + dr / 2;
                if (Squares[Index(eyeC, eyeR)] == 0) return true;
            }
        }

        // 马: 蹩马腿在 sq + (sign(dc), sign(dr)) 处
        int knightVal = byColor * Knight;
        for (int i = 0; i < 8; i++)
        {
            int c = sqCol + kKnightDc[i], r = sqRow + kKnightDr[i];
            if (c < 0 || c >= Width || r < 0 || r >= Height) continue;
            if (Squares[Index(c, r)] != knightVal) continue;
            int legC = sqCol + (kKnightDc[i] > 0 ? 1 : (kKnightDc[i] < 0 ? -1 : 0));
            int legR = sqRow + (kKnightDr[i] > 0 ? 1 : (kKnightDr[i] < 0 ? -1 : 0));
            if (Squares[Index(legC, legR)] == 0) return true;
        }

        // 车/炮: 直线扫描(第一个挡子为车则攻击, 第二个挡子为炮则攻击)
        int rookVal = byColor * Rook;
        int cannonVal = byColor * Cannon;
        const int kDdc[4] = { 1, -1, 0, 0 };
        const int kDdr[4] = { 0, 0, 1, -1 };
        for (int i = 0; i < 4; i++)
        {
            int dc = kDdc[i], dr = kDdr[i];
            int c = sqCol + dc, r = sqRow + dr;
            bool foundFirst = false;
            while (c >= 0 && c < Width && r >= 0 && r < Height)
            {
                int p = Squares[Index(c, r)];
                if (p != 0)
                {
                    if (!foundFirst)
                    {
                        if (p == rookVal) return true;
                        foundFirst = true;
                    }
                    else
                    {
                        if (p == cannonVal) return true;
                        break;
                    }
                }
                c += dc; r += dr;
            }
        }

        return false;
    }

    // =========================================================================
    // MoveGenerator
    // =========================================================================
    namespace
    {
        inline void Add(SearchMove *buf, int &count, int from, int to, int piece, int captured, bool capturesOnly)
        {
            if (capturesOnly && captured == 0) return;
            buf[count++] = SearchMove(from, to, piece, captured);
        }

        void GenKing(const SearchBoard &b, int sq, int color, SearchMove *buf, int &count, bool capOnly)
        {
            int sCol = SearchBoard::Col(sq), sRow = SearchBoard::Row(sq);
            int piece = b.Squares[sq];
            for (int i = 0; i < 4; i++)
            {
                int c = sCol + kOrthDc[i], r = sRow + kOrthDr[i];
                if (!SearchBoard::InPalace(c, r, color)) continue;
                int t = SearchBoard::Index(c, r);
                int target = b.Squares[t];
                if (target != 0 && SearchBoard::ColorOf(target) == color) continue;
                Add(buf, count, sq, t, piece, target, capOnly);
            }
        }

        void GenMandarin(const SearchBoard &b, int sq, int color, SearchMove *buf, int &count, bool capOnly)
        {
            int sCol = SearchBoard::Col(sq), sRow = SearchBoard::Row(sq);
            int piece = b.Squares[sq];
            for (int i = 0; i < 4; i++)
            {
                int c = sCol + kDiagDc[i], r = sRow + kDiagDr[i];
                if (!SearchBoard::InPalace(c, r, color)) continue;
                int t = SearchBoard::Index(c, r);
                int target = b.Squares[t];
                if (target != 0 && SearchBoard::ColorOf(target) == color) continue;
                Add(buf, count, sq, t, piece, target, capOnly);
            }
        }

        void GenElephant(const SearchBoard &b, int sq, int color, SearchMove *buf, int &count, bool capOnly)
        {
            int sCol = SearchBoard::Col(sq), sRow = SearchBoard::Row(sq);
            int piece = b.Squares[sq];
            for (int i = 0; i < 4; i++)
            {
                int dc = kDiagDc[i] * 2, dr = kDiagDr[i] * 2;
                int c = sCol + dc, r = sRow + dr;
                if (c < 0 || c >= SearchBoard::Width || r < 0 || r >= SearchBoard::Height) continue;
                if (!SearchBoard::OnOwnSide(r, color)) continue; // 象不过河
                // 象眼
                int eye = SearchBoard::Index(sCol + kDiagDc[i], sRow + kDiagDr[i]);
                if (b.Squares[eye] != 0) continue;
                int t = SearchBoard::Index(c, r);
                int target = b.Squares[t];
                if (target != 0 && SearchBoard::ColorOf(target) == color) continue;
                Add(buf, count, sq, t, piece, target, capOnly);
            }
        }

        void GenKnight(const SearchBoard &b, int sq, int color, SearchMove *buf, int &count, bool capOnly)
        {
            int sCol = SearchBoard::Col(sq), sRow = SearchBoard::Row(sq);
            int piece = b.Squares[sq];
            for (int i = 0; i < 8; i++)
            {
                int dc = kKnightDc[i], dr = kKnightDr[i];
                int c = sCol + dc, r = sRow + dr;
                if (c < 0 || c >= SearchBoard::Width || r < 0 || r >= SearchBoard::Height) continue;
                // 蹩马腿: 从源格朝 (sign(dc), sign(dr)) 一步
                int legC, legR;
                if (dr == 2 || dr == -2)
                {
                    legC = sCol;
                    legR = sRow + dr / 2;
                }
                else
                {
                    legC = sCol + dc / 2;
                    legR = sRow;
                }
                if (b.Squares[SearchBoard::Index(legC, legR)] != 0) continue;
                int t = SearchBoard::Index(c, r);
                int target = b.Squares[t];
                if (target != 0 && SearchBoard::ColorOf(target) == color) continue;
                Add(buf, count, sq, t, piece, target, capOnly);
            }
        }

        void GenRook(const SearchBoard &b, int sq, int color, SearchMove *buf, int &count, bool capOnly)
        {
            int sCol = SearchBoard::Col(sq), sRow = SearchBoard::Row(sq);
            int piece = b.Squares[sq];
            for (int i = 0; i < 4; i++)
            {
                int dc = kOrthDc[i], dr = kOrthDr[i];
                int c = sCol + dc, r = sRow + dr;
                while (c >= 0 && c < SearchBoard::Width && r >= 0 && r < SearchBoard::Height)
                {
                    int t = SearchBoard::Index(c, r);
                    int target = b.Squares[t];
                    if (target == 0)
                    {
                        Add(buf, count, sq, t, piece, 0, capOnly);
                    }
                    else
                    {
                        if (SearchBoard::ColorOf(target) != color)
                            Add(buf, count, sq, t, piece, target, capOnly);
                        break;
                    }
                    c += dc; r += dr;
                }
            }
        }

        void GenCannon(const SearchBoard &b, int sq, int color, SearchMove *buf, int &count, bool capOnly)
        {
            int sCol = SearchBoard::Col(sq), sRow = SearchBoard::Row(sq);
            int piece = b.Squares[sq];
            for (int i = 0; i < 4; i++)
            {
                int dc = kOrthDc[i], dr = kOrthDr[i];
                int c = sCol + dc, r = sRow + dr;
                bool hopped = false;
                while (c >= 0 && c < SearchBoard::Width && r >= 0 && r < SearchBoard::Height)
                {
                    int t = SearchBoard::Index(c, r);
                    int target = b.Squares[t];
                    if (!hopped)
                    {
                        if (target == 0)
                        {
                            Add(buf, count, sq, t, piece, 0, capOnly);
                        }
                        else
                        {
                            hopped = true;
                        }
                    }
                    else
                    {
                        if (target != 0)
                        {
                            if (SearchBoard::ColorOf(target) != color)
                                Add(buf, count, sq, t, piece, target, capOnly);
                            break;
                        }
                    }
                    c += dc; r += dr;
                }
            }
        }

        void GenPawn(const SearchBoard &b, int sq, int color, SearchMove *buf, int &count, bool capOnly)
        {
            int sCol = SearchBoard::Col(sq), sRow = SearchBoard::Row(sq);
            int piece = b.Squares[sq];
            int forwardR = sRow + color; // 红 +1 黑 -1
            if (forwardR >= 0 && forwardR < SearchBoard::Height)
            {
                int t = SearchBoard::Index(sCol, forwardR);
                int target = b.Squares[t];
                if (target == 0 || SearchBoard::ColorOf(target) != color)
                    Add(buf, count, sq, t, piece, target, capOnly);
            }
            bool acrossRiver = color > 0 ? sRow >= 5 : sRow <= 4;
            if (acrossRiver)
            {
                if (sCol > 0)
                {
                    int t = SearchBoard::Index(sCol - 1, sRow);
                    int target = b.Squares[t];
                    if (target == 0 || SearchBoard::ColorOf(target) != color)
                        Add(buf, count, sq, t, piece, target, capOnly);
                }
                if (sCol < SearchBoard::Width - 1)
                {
                    int t = SearchBoard::Index(sCol + 1, sRow);
                    int target = b.Squares[t];
                    if (target == 0 || SearchBoard::ColorOf(target) != color)
                        Add(buf, count, sq, t, piece, target, capOnly);
                }
            }
        }
    }

    int MoveGenerator::Generate(const SearchBoard &board, int color, SearchMove *buf, bool capturesOnly)
    {
        int count = 0;
        for (int sq = 0; sq < SearchBoard::Size; sq++)
        {
            int piece = board.Squares[sq];
            if (piece == 0) continue;
            if (SearchBoard::ColorOf(piece) != color) continue;
            int type = SearchBoard::TypeOf(piece);

            switch (type)
            {
            case SearchBoard::King:     GenKing(board, sq, color, buf, count, capturesOnly); break;
            case SearchBoard::Mandarin: GenMandarin(board, sq, color, buf, count, capturesOnly); break;
            case SearchBoard::Elephant: GenElephant(board, sq, color, buf, count, capturesOnly); break;
            case SearchBoard::Knight:   GenKnight(board, sq, color, buf, count, capturesOnly); break;
            case SearchBoard::Rook:     GenRook(board, sq, color, buf, count, capturesOnly); break;
            case SearchBoard::Cannon:   GenCannon(board, sq, color, buf, count, capturesOnly); break;
            case SearchBoard::Pawn:     GenPawn(board, sq, color, buf, count, capturesOnly); break;
            default: break;
            }
        }
        return count;
    }

    // =========================================================================
    // Evaluation
    // =========================================================================
    const int Evaluation::PieceValue[8] =
    {
        0,      // 0: 空(未用)
        10000,  // 1: 将/帅(仅必杀计算引用, 常规评估不计)
        120,    // 2: 士/仕
        120,    // 3: 象/相
        270,    // 4: 马
        600,    // 5: 车
        285,    // 6: 炮
        30,     // 7: 卒/兵(基准值, 过河后由位置表大幅加成)
    };

    // ====== 分兵种位置表(红方视角, [row][col] 共 90 项; 黑方按行镜像取值) ======
    namespace
    {
        // 兵卒: 跨河后大幅升值, 进入对方腹地最佳
        const int kPawnPST[SearchBoard::Size] =
        {
            0,0,0,0,0,0,0,0,0,   // row 0
            0,0,0,0,0,0,0,0,0,   // row 1
            0,0,0,0,0,0,0,0,0,   // row 2
            -2,0,4,0,6,0,4,0,-2, // row 3 (兵起始线)
            -2,0,8,0,8,0,8,0,-2, // row 4
            14,18,20,26,28,26,20,18,14, // row 5 (刚过河)
            22,30,32,34,38,34,32,30,22, // row 6
            28,36,42,46,50,46,42,36,28, // row 7
            32,42,48,54,58,54,48,42,32, // row 8 (近九宫)
            18,22,26,30,32,30,26,22,18, // row 9 (敌方底线)
        };

        // 马: 中心强, 被困边角弱
        const int kKnightPST[SearchBoard::Size] =
        {
            -4,4,4,6,4,6,4,4,-4,
            4,8,16,12,4,12,16,8,4,
            4,12,18,18,12,18,18,12,4,
            6,8,16,16,16,16,16,8,6,
            8,12,18,16,20,16,18,12,8,
            12,16,20,20,24,20,20,16,12,
            14,18,24,28,30,28,24,18,14,
            16,22,30,32,34,32,30,22,16,
            14,20,28,30,32,30,28,20,14,
            6,14,16,20,20,20,16,14,6,
        };

        // 车: 到处都强, 向前推进略加分; 中路、肋道更优
        const int kRookPST[SearchBoard::Size] =
        {
            -2,8,4,12,16,12,4,8,-2,
            8,12,16,18,18,18,16,12,8,
            6,10,12,14,14,14,12,10,6,
            6,8,10,16,16,16,10,8,6,
            8,12,16,18,18,18,16,12,8,
            10,14,18,20,20,20,18,14,10,
            14,18,20,22,22,22,20,18,14,
            16,20,22,24,26,24,22,20,16,
            16,20,22,26,28,26,22,20,16,
            14,18,20,24,26,24,20,18,14,
        };

        // 炮: 中线和宫顶炮位重要, 避免低效边路
        const int kCannonPST[SearchBoard::Size] =
        {
            6,4,0,-10,-12,-10,0,4,6,
            2,2,0,-4,-14,-4,0,2,2,
            2,2,0,-10,-8,-10,0,2,2,
            0,0,-2,4,10,4,-2,0,0,
            0,0,0,4,8,4,0,0,0,
            -2,0,4,6,10,6,4,0,-2,
            0,0,0,2,8,2,0,0,0,
            -4,-4,0,4,10,4,0,-4,-4,
            -6,-6,0,4,12,4,0,-6,-6,
            -4,-4,0,8,16,8,0,-4,-4,
        };

        // 将/帅: 留在九宫, 最好窝在底线中位
        const int kKingPST[SearchBoard::Size] =
        {
            0,0,0,12,18,12,0,0,0,
            0,0,0,4,10,4,0,0,0,
            0,0,0,-6,-8,-6,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
        };

        // 士: 仅九宫合法格有微小区分(中心比四角略优)
        const int kMandarinPST[SearchBoard::Size] =
        {
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,4,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
        };

        // 象: 典型三七象、田字眼略优, 象心略弱
        const int kElephantPST[SearchBoard::Size] =
        {
            0,0,4,0,0,0,4,0,0,
            0,0,0,0,0,0,0,0,0,
            2,0,0,0,6,0,0,0,2,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,
        };

        const int * const kPSTByType[8] =
        {
            NULL,           // 0
            kKingPST,       // 1
            kMandarinPST,   // 2
            kElephantPST,   // 3
            kKnightPST,     // 4
            kRookPST,       // 5
            kCannonPST,     // 6
            kPawnPST,       // 7
        };
    }

    int Evaluation::Evaluate(const SearchBoard &board)
    {
        int score = 0;
        for (int sq = 0; sq < SearchBoard::Size; sq++)
        {
            int p = board.Squares[sq];
            if (p == 0) continue;
            int type = SearchBoard::TypeOf(p);
            int idx = p > 0 ? sq : MirrorSq(sq);
            int v = PieceValue[type] + kPSTByType[type][idx];
            score += p > 0 ? v : -v;
        }
        return board.SideToMove > 0 ? score : -score;
    }

    bool Evaluation::HasMajorPiece(const SearchBoard &board, int color)
    {
        for (int sq = 0; sq < SearchBoard::Size; sq++)
        {
            int p = board.Squares[sq];
            if (p == 0) continue;
            if (SearchBoard::ColorOf(p) != color) continue;
            int type = SearchBoard::TypeOf(p);
            if (type == SearchBoard::Rook || type == SearchBoard::Cannon
                || type == SearchBoard::Knight || type == SearchBoard::Pawn)
                return true;
        }
        return false;
    }

    // =========================================================================
    // TranspositionTable
    // =========================================================================
    TranspositionTable::TranspositionTable(int sizeBits)
        : m_mask(0), m_generation(0)
    {
        int size = 1 << sizeBits;
        m_entries = new Entry[size];
        memset(m_entries, 0, sizeof(Entry) * size);
        m_mask = (uint64_t)(size - 1);
    }

    TranspositionTable::~TranspositionTable()
    {
        delete[] m_entries;
    }

    void TranspositionTable::NewGeneration()
    {
        m_generation++;
    }

    void TranspositionTable::Store(uint64_t key, int depth, int score, uint16_t bestMove, TTBound flag, int ply)
    {
        Entry &e = m_entries[key & m_mask];
        // 替换策略: 优先保留同代深度更大的, 否则替换
        if (e.Key == key && e.Depth > depth && e.Generation == m_generation)
            return;
        int adj = score;
        if (adj >= Evaluation::MateInMaxPly) adj += ply;
        else if (adj <= -Evaluation::MateInMaxPly) adj -= ply;
        e.Key = key;
        e.Score = (int16_t)adj;
        e.BestMove = bestMove;
        e.Depth = (uint8_t)depth;
        e.Flag = (uint8_t)flag;
        e.Generation = m_generation;
    }

    bool TranspositionTable::Probe(uint64_t key, int ply, int &depth, int &score, uint16_t &move, TTBound &flag) const
    {
        const Entry &e = m_entries[key & m_mask];
        if (e.Key != key)
        {
            depth = 0; score = 0; move = 0; flag = TT_NONE;
            return false;
        }
        depth = e.Depth;
        int s = e.Score;
        if (s >= Evaluation::MateInMaxPly) s -= ply;
        else if (s <= -Evaluation::MateInMaxPly) s += ply;
        score = s;
        move = e.BestMove;
        flag = (TTBound)e.Flag;
        return true;
    }

    // =========================================================================
    // SearchEngine
    // =========================================================================
    namespace
    {
        long long NowMs()
        {
            // 不引 <chrono>(保持最小依赖), 用 clock() 的 CLOCKS_PER_SEC 换算;
            // Windows/POSIX 下 clock() 均为进程 CPU 时间, 精度足够节点级计时。
            // 时间预算的真实墙钟检查同样用它, 单线程阻塞搜索场景语义等价。
            return (long long)clock() * 1000LL / (long long)CLOCKS_PER_SEC;
        }
    }

    SearchEngine::SearchEngine(int ttSizeBits)
        : m_board(), m_tt(ttSizeBits), m_nodes(0), m_startMs(0), m_timeLimitMs(0), m_stop(false)
        , m_rootTieCount(0)
    {
        memset(m_killers, 0, sizeof(m_killers));
        memset(m_history, 0, sizeof(m_history));
        memset(m_rootTies, 0, sizeof(m_rootTies));
    }

    SearchEngine::~SearchEngine()
    {
    }

    SearchResult SearchEngine::Search(const SearchBoard &root, int maxDepth, int timeLimitMs)
    {
        m_board.CloneFrom(root);
        m_nodes = 0;
        m_startMs = NowMs();
        m_timeLimitMs = timeLimitMs;
        m_stop = false;
        m_tt.NewGeneration();
        memset(m_killers, 0, sizeof(m_killers));
        memset(m_history, 0, sizeof(m_history));

        uint16_t bestMove = 0;
        int bestScore = 0;
        int completedDepth = 0;

        for (int depth = 1; depth <= maxDepth; depth++)
        {
            uint16_t iterBest = bestMove;
            int score = AlphaBetaRoot(depth, iterBest);
            // C# 版在时间到时抛 OperationCanceledException 中断本层;
            // C++ 版以 m_stop 标记等价语义: 本层未算完即丢弃, 采用上一个完整层结果
            if (m_stop)
                break;
            bestMove = iterBest;
            bestScore = score;
            completedDepth = depth;

            if (bestScore >= Evaluation::MateInMaxPly || bestScore <= -Evaluation::MateInMaxPly) break;
            if (TimeUp(true)) break;
        }

        SearchResult result;
        result.BestMove = bestMove;
        result.Score = bestScore;
        result.Depth = completedDepth;
        result.Nodes = m_nodes;
        result.ElapsedMs = NowMs() - m_startMs;

        // 根等分候选来自最近一个"完整算完"的层(中途超时的层不会写入);
        // 若从未完整算完任何层, RootTieCount 保持 0
        if (m_rootTieCount > 0)
        {
            memcpy(result.RootTies, m_rootTies, sizeof(uint16_t) * m_rootTieCount);
            result.RootTieCount = m_rootTieCount;
        }
        return result;
    }

    bool SearchEngine::TimeUp(bool soft) const
    {
        if (m_timeLimitMs <= 0) return false;
        long long elapsed = NowMs() - m_startMs;
        // 软停时(完成一层后): 超过预算的 60% 即停, 避免下一层吞超
        if (soft) return elapsed >= (m_timeLimitMs * 60) / 100;
        return elapsed >= m_timeLimitMs;
    }

    void SearchEngine::CheckTime()
    {
        if ((m_nodes & 4095) == 0 && TimeUp(false))
            m_stop = true;
    }

    int SearchEngine::AlphaBetaRoot(int depth, uint16_t &bestMove)
    {
        int alpha = -Inf, beta = Inf;
        int ply = 0;
        int color = m_board.SideToMove;

        SearchMove moves[MoveGenerator::MaxBuffer];
        int nMoves = MoveGenerator::Generate(m_board, color, moves);
        OrderMoves(moves, nMoves, bestMove, ply);

        int legalCount = 0;
        int bestScore = -Inf;
        uint16_t iterBest = bestMove;

        // 根节点等分候选: 仅在本层完整算完时由 Search() 外部读取,
        // 故用局部数组暂存, 循环结束后不回填(避免中途超时残留脏数据)
        uint16_t ties[MoveGenerator::MaxBuffer];
        int tieCount = 0;

        for (int i = 0; i < nMoves; i++)
        {
            SearchMove m = moves[i];
            m_board.Make(m);
            if (m_board.IsSideInCheck(color))
            {
                m_board.Unmake(m);
                continue;
            }
            legalCount++;

            int score = -AlphaBeta(depth - 1, -beta, -alpha, ply + 1, true);
            m_board.Unmake(m);

            if (m_stop)
                return -Inf; // 本层未算完, 结果作废

            if (score > bestScore)
            {
                bestScore = score;
                iterBest = m.Encode();
                tieCount = 0;
                ties[tieCount++] = iterBest;
                if (score > alpha)
                {
                    alpha = score;
                    if (alpha >= beta) break;
                }
            }
            else if (score == bestScore)
            {
                if (tieCount < MoveGenerator::MaxBuffer)
                    ties[tieCount++] = m.Encode();
            }
        }

        if (legalCount == 0)
        {
            // 无子可动: 被困毙或已被绝杀
            return -Evaluation::MateValue + ply;
        }
        bestMove = iterBest;

        // 等分候选写入成员缓冲, 供 Search() 返回给上层做棋风取舍
        memcpy(m_rootTies, ties, sizeof(uint16_t) * tieCount);
        m_rootTieCount = tieCount;

        m_tt.Store(m_board.ZobristKey, depth, bestScore, iterBest, TT_EXACT, ply);
        return bestScore;
    }

    int SearchEngine::AlphaBeta(int depth, int alpha, int beta, int ply, bool allowNull)
    {
        m_nodes++;
        CheckTime();
        if (m_stop)
            return 0;

        int color = m_board.SideToMove;
        bool inCheck = m_board.IsSideInCheck(color);
        if (inCheck) depth++; // 被将军延伸

        // ply 上限保护: 将军延伸链可能令深度不再递减, 防止杀手表越界/栈溢出
        if (ply >= MaxPly - 2)
            return Quiescence(alpha, beta, ply);

        if (depth <= 0) return Quiescence(alpha, beta, ply);

        // 置换表探查
        uint64_t key = m_board.ZobristKey;
        uint16_t ttMove = 0;
        int ttDepth; int ttScore; uint16_t ttMv; TTBound ttFlag;
        if (m_tt.Probe(key, ply, ttDepth, ttScore, ttMv, ttFlag))
        {
            ttMove = ttMv;
            if (ttDepth >= depth)
            {
                if (ttFlag == TT_EXACT) return ttScore;
                if (ttFlag == TT_LOWER && ttScore >= beta) return ttScore;
                if (ttFlag == TT_UPPER && ttScore <= alpha) return ttScore;
            }
        }

        // 空着裁剪(己方还有大子且不在被将军状态才安全)
        if (allowNull && !inCheck && depth >= 3 && Evaluation::HasMajorPiece(m_board, color)
            && beta < Evaluation::MateInMaxPly && beta > -Evaluation::MateInMaxPly)
        {
            int R = depth >= 6 ? 3 : 2;
            m_board.MakeNullMove();
            int nullScore = -AlphaBeta(depth - 1 - R, -beta, -beta + 1, ply + 1, false);
            m_board.UnmakeNullMove();
            if (m_stop)
                return 0;
            if (nullScore >= beta)
            {
                if (nullScore >= Evaluation::MateInMaxPly) nullScore = beta;
                return nullScore;
            }
        }

        SearchMove moves[MoveGenerator::MaxBuffer];
        int nMoves = MoveGenerator::Generate(m_board, color, moves);
        OrderMoves(moves, nMoves, ttMove, ply);

        int legalCount = 0;
        int bestScore = -Inf;
        uint16_t bestMv = 0;
        TTBound bound = TT_UPPER;

        for (int i = 0; i < nMoves; i++)
        {
            SearchMove m = moves[i];
            m_board.Make(m);
            if (m_board.IsSideInCheck(color))
            {
                m_board.Unmake(m);
                continue;
            }
            legalCount++;

            int score;
            if (legalCount == 1)
            {
                score = -AlphaBeta(depth - 1, -beta, -alpha, ply + 1, true);
            }
            else
            {
                score = -AlphaBeta(depth - 1, -alpha - 1, -alpha, ply + 1, true);
                if (score > alpha && score < beta)
                    score = -AlphaBeta(depth - 1, -beta, -alpha, ply + 1, true);
            }
            m_board.Unmake(m);

            if (m_stop)
                return 0;

            if (score > bestScore)
            {
                bestScore = score;
                bestMv = m.Encode();
                if (score > alpha)
                {
                    alpha = score;
                    bound = TT_EXACT;
                    if (alpha >= beta)
                    {
                        if (m.Captured == 0)
                        {
                            // 安静着法导致剪枝: 记杀手着法并累计历史启发(深度平方加权)
                            if (m_killers[ply][0] != bestMv)
                            {
                                m_killers[ply][1] = m_killers[ply][0];
                                m_killers[ply][0] = bestMv;
                            }
                            m_history[PieceIndex(m.Piece)][m.To] += depth * depth;
                        }
                        bound = TT_LOWER;
                        break;
                    }
                }
            }
        }

        if (legalCount == 0)
        {
            // 无子可动 = 输(中国象棋无和棋判定)
            return -Evaluation::MateValue + ply;
        }

        m_tt.Store(key, depth, bestScore, bestMv, bound, ply);
        return bestScore;
    }

    int SearchEngine::Quiescence(int alpha, int beta, int ply)
    {
        m_nodes++;
        CheckTime();
        if (m_stop)
            return 0;

        // 静态结点评估(stand-pat): 已超出 beta 则无需延展吃子
        int standPat = Evaluation::Evaluate(m_board);
        if (standPat >= beta) return beta;
        if (alpha < standPat) alpha = standPat;
        if (ply >= MaxPly - 1) return alpha;

        int color = m_board.SideToMove;
        SearchMove moves[MoveGenerator::MaxBuffer];
        int nMoves = MoveGenerator::Generate(m_board, color, moves, true);
        OrderCaptures(moves, nMoves);

        for (int i = 0; i < nMoves; i++)
        {
            SearchMove m = moves[i];
            m_board.Make(m);
            if (m_board.IsSideInCheck(color))
            {
                m_board.Unmake(m);
                continue;
            }
            int score = -Quiescence(-beta, -alpha, ply + 1);
            m_board.Unmake(m);

            if (m_stop)
                return 0;
            if (score >= beta) return beta;
            if (score > alpha) alpha = score;
        }
        return alpha;
    }

    void SearchEngine::OrderMoves(SearchMove *moves, int count, uint16_t ttMove, int ply)
    {
        for (int i = 0; i < count; i++)
        {
            SearchMove &m = moves[i];
            uint16_t code = m.Encode();
            int score;
            if (code == ttMove && ttMove != 0)
            {
                score = 10000000;
            }
            else if (m.Captured != 0)
            {
                int victim = m.Captured > 0 ? m.Captured : -m.Captured;
                int attacker = m.Piece > 0 ? m.Piece : -m.Piece;
                score = 1000000 + Evaluation::PieceValue[victim] * 16 - Evaluation::PieceValue[attacker];
            }
            else if (code == m_killers[ply][0])
            {
                score = 900000;
            }
            else if (code == m_killers[ply][1])
            {
                score = 800000;
            }
            else
            {
                score = m_history[PieceIndex(m.Piece)][m.To];
            }
            m.Score = score;
        }
        SortByScoreDescending(moves, count);
    }

    void SearchEngine::OrderCaptures(SearchMove *moves, int count)
    {
        for (int i = 0; i < count; i++)
        {
            SearchMove &m = moves[i];
            int victim = m.Captured > 0 ? m.Captured : -m.Captured;
            int attacker = m.Piece > 0 ? m.Piece : -m.Piece;
            m.Score = Evaluation::PieceValue[victim] * 16 - Evaluation::PieceValue[attacker];
        }
        SortByScoreDescending(moves, count);
    }

    void SearchEngine::SortByScoreDescending(SearchMove *moves, int count)
    {
        // 着法数通常在 30~80 之间, 插入排序足够
        for (int i = 1; i < count; i++)
        {
            SearchMove key = moves[i];
            int j = i - 1;
            while (j >= 0 && moves[j].Score < key.Score)
            {
                moves[j + 1] = moves[j];
                j--;
            }
            moves[j + 1] = key;
        }
    }
}
