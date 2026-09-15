#pragma once
#include "ChessLayout.h"
#include <vector>

// 机器人搜索深度(对应智力等级)
#define ROBOT_AI_DEPTH_BEGINNER 4   // 初级
#define ROBOT_AI_DEPTH_MEDIUM   5   // 中级
#define ROBOT_AI_DEPTH_ADVANCED 6   // 高级

/**
 * @brief 中国象棋 AI 引擎
 *
 * 算法移植自开源项目 jie65535/ChineseChess 的 ChineseChess.Core/AI 模块
 * (https://github.com/jie65535/ChineseChess, MIT License, Copyright (c) 2021):
 * 迭代加深 + PVS(alpha-beta) + 空着裁剪 + 置换表 + 杀手/历史启发 + 静态搜索,
 * 评估为物质分 + 分兵种位置表。
 * 具体实现见 ChsAIEngine.h/.cpp; 本类仅负责 CChessLayout(9x10 二维) 与
 * 搜索棋盘(90 格一维)之间的坐标映射与结果转换。
 *
 * 坐标映射: 依据红帅位置自动判定朝向 —— 红方在低 y 侧(开局库规范朝向)时直接
 * 行号映射; 红方在高 y 侧时按 row=9-y 垂直翻转, 保证搜索引擎内部恒为
 * "红方宫在低行、红兵向高行进攻"的规范坐标系。
 *
 * 不依赖 UI 层, 可在服务端与客户端共用; 线程安全: 并发调用安全
 * (搜索引擎按线程独立实例, 棋盘在调用方快照上运行, 不修改传入的 layout)。
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
     * @param layout  当前棋局(只读使用, 内部在快照上搜索, 不会修改其状态)
     * @param nDepth  搜索深度
     * @return 最佳走法；当无合法走法时返回{pt1={-1,-1},pt2={-1,-1},...}
     */
    static MOVESTEP SearchBestMove(CChessLayout &layout, int nDepth);
    // 带时间预算的版本: nTimeMs>0 时迭代加深受思考时间约束(层间软停+搜内硬停),
    // 到点即返回已完成的最深层较优着法; ≤0 等价于纯迭代加深。
    // 预算过小导致第 1 层都未算完时, 自动不计时间补算 1 层, 保证总返回合法着法。
    static MOVESTEP SearchBestMove(CChessLayout &layout, int nDepth, int nTimeMs);
};
