#include "stdafx.h"
#include "ChessBook.h"
#include <vector>
#include <map>
#include <stdlib.h>

// 简易开局库实现。
// 开局线以"红在上"坐标系存储(即 kDefaultLayout: 红方在第0行, 黑方在底)。
// 构建时逐变例走子并校验合法性, 对每个可达局面记录其推荐着法(规范化坐标),
// 供 Probe 在运行时按规范化哈希查表。

namespace
{
    // 一手走法: 起(列,行) -> 落(列,行), 红在上坐标系
    struct BMove
    {
        int x1, y1, x2, y2;
    };

    // 中炮对屏风马(缓开车)
    const BMove kLineA[] = {
        { 7, 2, 4, 2 }, // 红 炮二平五
        { 1, 9, 2, 7 }, // 黑 马8进7
        { 7, 0, 6, 2 }, // 红 马二进三
        { 7, 9, 6, 7 }, // 黑 马2进3
        { 8, 0, 7, 0 }, // 红 车一平二
        { 0, 9, 1, 9 }, // 黑 车9平8
        { 2, 3, 2, 4 }, // 红 兵三进一
        { 6, 6, 6, 5 }, // 黑 卒3进1
        { 4, 3, 4, 4 }, // 红 兵五进一
        { 0, 6, 0, 5 }  // 黑 卒7进1
    };

    // 中炮对屏风马(黑先进右马)
    const BMove kLineB[] = {
        { 7, 2, 4, 2 }, // 红 炮二平五
        { 7, 9, 6, 7 }, // 黑 马2进3
        { 7, 0, 6, 2 }, // 红 马二进三
        { 1, 9, 2, 7 }, // 黑 马8进7
        { 8, 0, 7, 0 }, // 红 车一平二
        { 0, 9, 1, 9 }  // 黑 车9平8
    };

    // 顺手炮(直车)
    const BMove kLineC[] = {
        { 7, 2, 4, 2 }, // 红 炮二平五
        { 1, 7, 4, 7 }, // 黑 炮8平5
        { 7, 0, 6, 2 }, // 红 马二进三
        { 1, 9, 2, 7 }, // 黑 马8进7
        { 8, 0, 7, 0 }, // 红 车一平二
        { 0, 9, 1, 9 }, // 黑 车9平8
        { 1, 0, 2, 2 }, // 红 马八进七
        { 7, 9, 6, 7 }  // 黑 马2进3
    };

    // 仙人指路(对兵局)
    const BMove kLineD[] = {
        { 2, 3, 2, 4 }, // 红 兵七进一
        { 6, 6, 6, 5 }, // 黑 卒3进1
        { 7, 2, 4, 2 }, // 红 炮二平五
        { 1, 9, 2, 7 }, // 黑 马8进7
        { 7, 0, 6, 2 }, // 红 马二进三
        { 7, 9, 6, 7 }, // 黑 马2进3
        { 8, 0, 7, 0 }, // 红 车一平二
        { 0, 9, 1, 9 }  // 黑 车9平8
    };

    // 起马局
    const BMove kLineE[] = {
        { 1, 0, 2, 2 }, // 红 马八进七
        { 6, 6, 6, 5 }, // 黑 卒3进1
        { 2, 3, 2, 4 }, // 红 兵三进一
        { 7, 9, 6, 7 }, // 黑 马2进3
        { 7, 2, 4, 2 }, // 红 炮二平五
        { 1, 9, 2, 7 }  // 黑 马8进7
    };

    // 过宫炮
    const BMove kLineF[] = {
        { 7, 2, 5, 2 }, // 红 炮二平六
        { 1, 9, 2, 7 }, // 黑 马8进7
        { 7, 0, 6, 2 }, // 红 马二进三
        { 0, 9, 1, 9 }, // 黑 车9平8
        { 8, 0, 7, 0 }, // 红 车一平二
        { 6, 6, 6, 5 }, // 黑 卒3进1
        { 2, 3, 2, 4 }, // 红 兵三进一
        { 0, 6, 0, 5 }  // 黑 卒7进1
    };

    // 反宫马
    const BMove kLineG[] = {
        { 7, 2, 4, 2 }, // 红 炮二平五
        { 7, 9, 6, 7 }, // 黑 马2进3
        { 7, 0, 6, 2 }, // 红 马二进三
        { 1, 7, 5, 7 }, // 黑 炮8平6(反宫马)
        { 8, 0, 7, 0 }, // 红 车一平二
        { 1, 9, 2, 7 }, // 黑 马8进7
        { 2, 3, 2, 4 }, // 红 兵三进一
        { 6, 6, 6, 5 }  // 黑 卒3进1
    };

    // 五七炮
    const BMove kLineH[] = {
        { 7, 2, 4, 2 }, // 红 炮二平五
        { 1, 9, 2, 7 }, // 黑 马8进7
        { 7, 0, 6, 2 }, // 红 马二进三
        { 7, 9, 6, 7 }, // 黑 马2进3
        { 1, 2, 3, 2 }, // 红 炮八平七(五七炮)
        { 0, 9, 1, 9 }, // 黑 车9平8
        { 8, 0, 7, 0 }, // 红 车一平二
        { 6, 6, 6, 5 }  // 黑 卒3进1
    };

    struct BLine
    {
        const BMove *moves;
        int count;
    };

    const BLine kLines[] = {
        { kLineA, sizeof(kLineA) / sizeof(kLineA[0]) },
        { kLineB, sizeof(kLineB) / sizeof(kLineB[0]) },
        { kLineC, sizeof(kLineC) / sizeof(kLineC[0]) },
        { kLineD, sizeof(kLineD) / sizeof(kLineD[0]) },
        { kLineE, sizeof(kLineE) / sizeof(kLineE[0]) },
        { kLineF, sizeof(kLineF) / sizeof(kLineF[0]) },
        { kLineG, sizeof(kLineG) / sizeof(kLineG[0]) },
        { kLineH, sizeof(kLineH) / sizeof(kLineH[0]) }
    };

    std::map<unsigned long long, std::vector<std::pair<POINT, POINT> > > g_book;

    // 判断一手走法是否为该局面的合法着法(非规范化, 直接基于 layout 判定)
    bool IsLegalMove(CChessLayout &layout, int x1, int y1, int x2, int y2)
    {
        CHESSMAN cs = layout.m_chesses[y1][x1];
        if (cs == CHSMAN_NULL || CHSMANSIDE(cs) != layout.m_actSide)
            return false;
        POINT moves[MAX_MOVES];
        int n = layout.GetPossibleMoves(x1, y1, moves);
        for (int i = 0; i < n; i++)
            if (moves[i].x == x2 && moves[i].y == y2)
                return true;
        return false;
    }

    // 构建开局库: 以默认开局为起点, 逐变例走子, 遇到非法着法即截断该变例
    void BuildBook()
    {
        for (int li = 0; li < (int)(sizeof(kLines) / sizeof(kLines[0])); li++)
        {
            const BLine &line = kLines[li];
            CChessLayout layout;
            layout.InitLayout(NULL, CS_RED); // kDefaultLayout, 红在上
            for (int i = 0; i < line.count; i++)
            {
                const BMove &m = line.moves[i];
                if (!IsLegalMove(layout, m.x1, m.y1, m.x2, m.y2))
                    break; // 该变例后续不可用, 保留已记录的着法
                POINT p1 = { m.x1, m.y1 };
                POINT p2 = { m.x2, m.y2 };
                unsigned long long h = CChessOpeningBook::Hash(layout);
                g_book[h].push_back(std::make_pair(p1, p2));
                layout.Move(p1, p2);
            }
        }
    }

    // 文件级静态对象: 其构造函数在主线程、进入 main 之前单线程执行, 天然无并发
    struct BBoot
    {
        BBoot() { BuildBook(); }
    };
    const BBoot g_boot;

    // 线程局部伪随机: rand() 共享全局状态、非线程安全, 而开局库探测运行在机器人池线程上,
    // 必须按线程独立产生随机数, 避免跨线程数据竞争。
    thread_local unsigned long long g_seed = 0x9e3779b97f4a7c15ULL;
    int ThreadRand(int n)
    {
        g_seed = g_seed * 2862933555777941757ULL + 3037000493ULL;
        return (int)(g_seed >> 33) % n; // 取高位更随机
    }
}

unsigned long long CChessOpeningBook::Hash(const CChessLayout &layout)
{
    // 规范化: 若红方王在低行的下半区(y>=5), 则垂直翻转棋盘使其处于上半区
    int redKingY = 9, blueKingY = 0;
    for (int y = 0; y < 10; y++)
    {
        for (int x = 0; x < 9; x++)
        {
            CHESSMAN cs = layout.m_chesses[y][x];
            if (cs == CHSMAN_RED_JIANG)
                redKingY = y;
            else if (cs == CHSMAN_BLK_JIANG)
                blueKingY = y;
        }
    }
    bool flip = (redKingY >= 5);
    unsigned long long h = 0xcbf29ce484222325ULL; // FNV-1a 偏移基数
    for (int y = 0; y < 10; y++)
    {
        for (int x = 0; x < 9; x++)
        {
            int sy = flip ? 9 - y : y;
            CHESSMAN cs = layout.m_chesses[sy][x];
            h ^= cs;
            h *= 0x100000001b3ULL; // FNV 素数
        }
    }
    h ^= (layout.m_actSide == CS_RED) ? 1 : 2;
    h *= 0x100000001b3ULL;
    return h;
}

bool CChessOpeningBook::Probe(const CChessLayout &layout, POINT &pt1, POINT &pt2)
{
    // 仅开局阶段查书: 棋子明显减少(进入中盘/残局)则不适用
    int count = 0;
    int redKingY = 9;
    for (int y = 0; y < 10; y++)
    {
        for (int x = 0; x < 9; x++)
        {
            CHESSMAN cs = layout.m_chesses[y][x];
            if (cs == CHSMAN_NULL)
                continue;
            count++;
            if (cs == CHSMAN_RED_JIANG)
                redKingY = y;
        }
    }
    if (count < 26) // 中盘/残局, 交由搜索
        return false;

    std::map<unsigned long long, std::vector<std::pair<POINT, POINT> > >::const_iterator it =
        g_book.find(Hash(layout));
    if (it == g_book.end() || it->second.empty())
        return false;

    bool flip = (redKingY >= 5);
    std::pair<POINT, POINT> rec = it->second[ThreadRand((int)it->second.size())];
    pt1 = rec.first;
    pt2 = rec.second;
    if (flip) // 把规范化坐标还原为实际棋盘坐标
    {
        pt1.y = 9 - pt1.y;
        pt2.y = 9 - pt2.y;
    }
    return true;
}