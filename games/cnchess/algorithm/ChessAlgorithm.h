#pragma once

/**
 * @brief 象棋算法类
 * 
 * 实现中国象棋的基本规则和玩法逻辑
 */
class CChessAlgorithm
{
public:
    // 棋子类型枚举
    enum PIECE_TYPE {
        PIECE_NONE = 0,    // 空位
        
        // 红方棋子
        PIECE_RED_KING,    // 帅
        PIECE_RED_ADVISOR, // 士
        PIECE_RED_BISHOP,  // 象
        PIECE_RED_KNIGHT,  // 马
        PIECE_RED_ROOK,    // 车
        PIECE_RED_CANNON,  // 炮
        PIECE_RED_PAWN,    // 兵
        
        // 黑方棋子
        PIECE_BLACK_KING,  // 将
        PIECE_BLACK_ADVISOR, // 士
        PIECE_BLACK_BISHOP,  // 象
        PIECE_BLACK_KNIGHT,  // 马
        PIECE_BLACK_ROOK,    // 车
        PIECE_BLACK_CANNON,  // 炮
        PIECE_BLACK_PAWN,    // 卒
    };
    
    // 棋盘尺寸
    static const int BOARD_WIDTH = 9;
    static const int BOARD_HEIGHT = 10;
    
public:
    CChessAlgorithm();
    ~CChessAlgorithm();
    
    // 初始化棋盘
    void InitBoard();
    
    // 获取指定位置的棋子
    PIECE_TYPE GetPiece(int x, int y) const;
    
    // 移动棋子
    bool MovePiece(int fromX, int fromY, int toX, int toY);
    
    // 检查是否将军
    bool IsCheck(bool isRed) const;
    
    // 检查是否将死
    bool IsCheckmate(bool isRed) const;
    
private:
    // 检查移动是否合法
    bool IsValidMove(int fromX, int fromY, int toX, int toY) const;
    
    // 检查是否在九宫格内
    bool IsInPalace(int x, int y, bool isRed) const;
    
    // 检查直线路径上是否有阻挡
    bool HasObstacleStraight(int fromX, int fromY, int toX, int toY) const;
    
    // 检查斜线路径上是否有阻挡
    bool HasObstacleDiagonal(int fromX, int fromY, int toX, int toY) const;
    
private:
    PIECE_TYPE m_board[BOARD_WIDTH][BOARD_HEIGHT]; // 棋盘状态
    bool m_isRedTurn; // 是否轮到红方走棋
};