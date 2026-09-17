/**
 * @file ChsAIEngine.h
 * @brief 自包含的中国象棋 alpha-beta 搜索引擎(可独立于 UI/平台使用)
 *
 * 移植自开源项目 jie65535/ChineseChess 的 ChineseChess.Core/AI 模块
 * (https://github.com/jie65535/ChineseChess, MIT License, Copyright (c) 2021)。
 * 按 MIT 许可要求保留上述版权与许可声明。
 *
 * 原实现为 C#; 本移植保持算法语义不变:
 *  - 迭代加深 + PVS(alpha-beta)搜索, 空着裁剪, 将军延伸
 *  - Zobrist 哈希置换表(深度优先替换 + 代次老化)
 *  - 杀手着法 / 历史启发的着法排序
 *  - 静态搜索(吃子延展)缓解水平线效应
 *  - 物质分 + 分兵种位置表的静态评估
 *
 * 棋盘坐标约定与原版一致: 90 格一维数组, row = sq/9, col = sq%9,
 * 红方在 row 0..4(宫在 row 0..2), 向 row 增大方向进攻; 黑方反之。
 * 棋子编码: 0 空, +1..+7 红方(将士象马车炮卒顺序见 TypeXXX 常量), 负数为黑方。
 *
 * 本文件只依赖标准 C++ 头, 不依赖 stdafx/windows.h, 可在
 * MSVC / MinGW / NDK / OHOS clang 等任意工具链下编译。
 */
#ifndef __CHSAI_ENGINE_H__
#define __CHSAI_ENGINE_H__

#include <stdint.h>

namespace ChsAI
{
    /** 搜索用紧凑走子 */
    struct SearchMove
    {
        uint8_t From;      /**< 起点格(0..89) */
        uint8_t To;        /**< 终点格(0..89) */
        int8_t  Piece;     /**< 被移动的棋子编码(撤销走子用) */
        int8_t  Captured;  /**< 被吃子编码, 0 表示无吃子 */
        int     Score;     /**< 排序得分(不参与正确性) */

        SearchMove() : From(0), To(0), Piece(0), Captured(0), Score(0) {}
        SearchMove(int from, int to, int piece, int captured)
            : From((uint8_t)from), To((uint8_t)to)
            , Piece((int8_t)piece), Captured((int8_t)captured), Score(0) {}

        /** 压缩为 16 位编码(起点高 8 位 + 终点低 8 位), 0 表示无效着法 */
        uint16_t Encode() const { return (uint16_t)((From << 8) | To); }
        static int FromOf(uint16_t code) { return code >> 8; }
        static int ToOf(uint16_t code) { return code & 0xFF; }
    };

    /** 搜索专用紧凑棋盘(移植自 SearchBoard.cs) */
    class SearchBoard
    {
    public:
        static const int Width = 9;
        static const int Height = 10;
        static const int Size = Width * Height;

        // 棋子类型常量(正为红, 负为黑)
        static const int Empty = 0;
        static const int King = 1;      // 将/帅
        static const int Mandarin = 2;  // 士/仕
        static const int Elephant = 3;  // 象/相
        static const int Knight = 4;    // 马
        static const int Rook = 5;      // 车
        static const int Cannon = 6;    // 炮
        static const int Pawn = 7;      // 卒/兵

        static const int Red = 1;
        static const int Black = -1;

        int8_t  Squares[Size]; /**< 90 格棋子编码 */
        int     SideToMove;    /**< 当前行棋方: +1 红 / -1 黑 */
        int     RedKingSq;     /**< 红帅所在格, -1 表示已被吃 */
        int     BlackKingSq;   /**< 黑将所在格, -1 表示已被吃 */
        uint64_t ZobristKey;   /**< 增量维护的局面哈希 */

        SearchBoard() : SideToMove(Red), RedKingSq(-1), BlackKingSq(-1), ZobristKey(0)
        {
            for (int i = 0; i < Size; i++) Squares[i] = 0;
        }

        static int Index(int col, int row) { return row * Width + col; }
        static int Col(int sq) { return sq % Width; }
        static int Row(int sq) { return sq / Width; }
        static int ColorOf(int piece) { return piece > 0 ? Red : (piece < 0 ? Black : 0); }
        static int TypeOf(int piece) { return piece > 0 ? piece : -piece; }

        /** 完整复制棋盘 */
        void CloneFrom(const SearchBoard &src);

        /** 由当前盘面从头计算 Zobrist 键 */
        uint64_t ComputeZobristFromScratch();

        /** 走子(增量更新哈希与将帅位置) */
        void Make(const SearchMove &m);
        /** 撤销走子 */
        void Unmake(const SearchMove &m);
        /** 空着(仅换行棋方) */
        void MakeNullMove();
        void UnmakeNullMove();

        /** 指定一方是否正被将军(将帅不在盘上视为被将) */
        bool IsSideInCheck(int color) const;
        /** 指定格是否被 byColor 一方攻击(含飞将照面判定) */
        bool IsSquareAttacked(int sq, int byColor) const;

        static bool InPalace(int col, int row, int color)
        {
            if (col < 3 || col > 5) return false;
            return color > 0 ? (row >= 0 && row <= 2) : (row >= 7 && row <= 9);
        }
        static bool OnOwnSide(int row, int color) { return color > 0 ? row <= 4 : row >= 5; }
    };

    /** 伪合法走子生成器(移植自 MoveGenerator.cs): 不过滤自将, 由搜索在 Make 后用 IsSideInCheck 排除 */
    class MoveGenerator
    {
    public:
        /** 单节点着法缓冲上限(车最多 17 步, 全盘 44 子实测远小于该值) */
        static const int MaxBuffer = 128;

        /**
         * @brief 生成指定一方的伪合法走法
         * @param buf 输出缓冲(容量不小于 MaxBuffer)
         * @param capturesOnly true 时仅生成吃子着法(静态搜索用)
         * @return 生成的着法数
         */
        static int Generate(const SearchBoard &board, int color, SearchMove *buf, bool capturesOnly = false);
    };

    /** 静态局面评估(移植自 Evaluation.cs): 物质分 + 分兵种位置表 */
    class Evaluation
    {
    public:
        static const int PieceValue[8];   /**< 按类型索引的物质分(1=将..7=兵) */
        static const int MateValue = 30000;      /**< 绝杀分, 远高于任何盘面物质评估 */
        static const int MateInMaxPly = MateValue - 1024; /**< 达到该量级视为已见绝杀 */

        /** 评估局面, 返回相对当前行棋方的分值(越大越有利) */
        static int Evaluate(const SearchBoard &board);

        /** 除士象将外是否还有其它子(决定空着裁剪是否安全) */
        static bool HasMajorPiece(const SearchBoard &board, int color);

        /** 黑方格子按行镜像到红方视角 */
        static int MirrorSq(int sq)
        {
            int col = sq % SearchBoard::Width;
            int row = sq / SearchBoard::Width;
            return (SearchBoard::Height - 1 - row) * SearchBoard::Width + col;
        }
    };

    /** 置换表结点边界类型 */
    enum TTBound
    {
        TT_NONE = 0,
        TT_EXACT = 1, /**< 精确值 */
        TT_LOWER = 2, /**< 下界(fail-high 剪枝) */
        TT_UPPER = 3, /**< 上界(fail-low) */
    };

    /** 简化置换表(移植自 TranspositionTable.cs): 定容数组 + 低位索引, 深度优先替换 */
    class TranspositionTable
    {
    public:
        explicit TranspositionTable(int sizeBits = 18);
        ~TranspositionTable();
        /** 开启新代次(每次搜索调用, 老化旧条目) */
        void NewGeneration();
        void Store(uint64_t key, int depth, int score, uint16_t bestMove, TTBound flag, int ply);
        bool Probe(uint64_t key, int ply, int &depth, int &score, uint16_t &move, TTBound &flag) const;

    private:
        TranspositionTable(const TranspositionTable &);
        TranspositionTable &operator=(const TranspositionTable &);

        struct Entry
        {
            uint64_t Key;
            int16_t  Score;
            uint16_t BestMove;
            uint8_t  Depth;
            uint8_t  Flag;
            uint8_t  Generation;
            uint8_t  Pad;
        };

        Entry  *m_entries;
        uint64_t m_mask;
        uint8_t  m_generation;
    };

    /** 一次搜索的结果 */
    struct SearchResult
    {
        uint16_t BestMove;        /**< 最佳着法(压缩编码), 0 表示无合法着法 */
        int      Score;           /**< 最佳分值(相对根行棋方) */
        int      Depth;           /**< 完整算完的最大深度 */
        long long Nodes;          /**< 搜索节点数 */
        long long ElapsedMs;      /**< 耗时(毫秒) */

        SearchResult() : BestMove(0), Score(0), Depth(0), Nodes(0), ElapsedMs(0) {}
    };

    /** 迭代加深 alpha-beta + PVS 搜索引擎(移植自 SearchEngine.cs) */
    class SearchEngine
    {
    public:
        /**
         * @brief 构造引擎
         * @param ttSizeBits 置换表容量(2^n 条), 18 = 26 万条约 4MB;
         *                   每个引擎实例独占一份, 并发场景按线程各建一个实例
         */
        explicit SearchEngine(int ttSizeBits = 18);
        ~SearchEngine();

        /**
         * @brief 搜索最佳着法
         * @param root 根局面(不会被修改)
         * @param maxDepth 迭代加深最大深度
         * @param timeLimitMs 时间预算(毫秒), >0 时迭代间软停 + 搜内硬停; <=0 不限时
         */
        SearchResult Search(const SearchBoard &root, int maxDepth, int timeLimitMs);

    private:
        SearchEngine(const SearchEngine &);
        SearchEngine &operator=(const SearchEngine &);

        bool TimeUp(bool soft) const;
        void CheckTime();
        int AlphaBetaRoot(int depth, uint16_t &bestMove);
        int AlphaBeta(int depth, int alpha, int beta, int ply, bool allowNull);
        int Quiescence(int alpha, int beta, int ply);
        void OrderMoves(SearchMove *moves, int count, uint16_t ttMove, int ply);
        static void OrderCaptures(SearchMove *moves, int count);
        static void SortByScoreDescending(SearchMove *moves, int count);

        static const int Inf = 32000;
        static const int MaxPly = 64;

        SearchBoard        m_board;
        TranspositionTable m_tt;
        long long          m_nodes;
        long long          m_startMs;
        int                m_timeLimitMs;
        bool               m_stop;    /**< 时间预算耗尽的停搜标记 */

        uint16_t m_killers[MaxPly][2];
        int      m_history[14][SearchBoard::Size];
    };
}

#endif // __CHSAI_ENGINE_H__
