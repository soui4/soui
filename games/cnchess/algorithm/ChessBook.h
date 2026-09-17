#pragma once
#include "ChessLayout.h"
#include <map>
#include <vector>

// 简易开局库: 以 "红在上"(红方王宫在低 y 侧, 即 kDefaultLayout 方向)为规范坐标系存储常见开局。
// 命中时返回一手推荐走法(pt1/pt2 为实际棋盘坐标); 非开局阶段或未命中返回 false。
// 仅供 AI 在开局阶段补充经验, 中后盘仍交由搜索引擎。
class CChessOpeningBook
{
public:
    // 探测开局库。命中且处于开局阶段返回 true 并给出走法(实际棋盘坐标), 否则返回 false。
    static bool Probe(const CChessLayout &layout, POINT &pt1, POINT &pt2);

    // 规范化(红在上)后整个棋盘 + 行棋方 的 FNV-1a 64 位哈希(构建期也需使用, 置为 public)
    static unsigned long long Hash(const CChessLayout &layout);
};