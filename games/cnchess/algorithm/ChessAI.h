#pragma once
#include "ChessLayout.h"
#include <vector>

// 机器人搜索深度(对应智力等级)
#define ROBOT_AI_DEPTH_BEGINNER 2   // 初级
#define ROBOT_AI_DEPTH_MEDIUM   3   // 中级
#define ROBOT_AI_DEPTH_ADVANCED 4   // 高级

/**
 * @brief 中国象棋 AI 引擎
 *
 * 采用 negamax(负极大) + alpha-beta 剪枝算法，通过搜索深度分档实现
 * 初/中/高三级智力水平。不依赖 UI 层，可在服务端与客户端共用。
 */
class CChessAI
{
public:
    /**
     * @brief 将智力等级转换为搜索深度
     * @param nLevel 智力等级(1=初级,2=中级,3=高级)
     * @return 搜索深度，非法等级返回初级深度
     */
    static int LevelToDepth(int nLevel);

    /**
     * @brief 计算当前行棋方的最佳走法
     * @param layout  当前棋局(调用后其内部状态可能改变，仅取 ptBegin/ptEnd 结果)
     * @param nDepth  搜索深度
     * @return 最佳走法；当无合法走法时返回{pt1={-1,-1},pt2={-1,-1},...}
     */
    static MOVESTEP SearchBestMove(CChessLayout &layout, int nDepth);

protected:
    // negamax 搜索
    static int Negamax(CChessLayout &layout, int depth, int alpha, int beta, int ply);
    // 局面评估(正值表示当前走棋方占优)
    static int Evaluate(const CChessLayout &layout);
    // 生成当前走棋方的全部合法走法
    static void GenerateLegalMoves(CChessLayout &layout, std::vector<MOVESTEP> &out);
    // 指定方是否被将军 == 检测己方将是否被对手攻击
    static bool IsJiangInCheck(const CChessLayout &layout, CHSSIDE side);
    // 检测 (tx,ty) 是否被 bySide 一方攻击
    static bool IsSquareAttacked(const CChessLayout &layout, int tx, int ty, CHSSIDE bySide);
    // 吃子价值
    static int PieceValue(CHESSMAN chs);

private:
    static const int KR_VAL[7];
    static const int INF;
    static const int CHECKMATE;
};