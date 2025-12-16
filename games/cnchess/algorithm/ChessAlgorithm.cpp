#include "ChessAlgorithm.h"
#include <cstring>

CChessAlgorithm::CChessAlgorithm()
{
    InitBoard();
}

CChessAlgorithm::~CChessAlgorithm()
{
}

void CChessAlgorithm::InitBoard()
{
    // 初始化棋盘为空
    memset(m_board, 0, sizeof(m_board));
    
    // 设置红方棋子
    m_board[0][0] = PIECE_RED_ROOK;      // 车
    m_board[1][0] = PIECE_RED_KNIGHT;    // 马
    m_board[2][0] = PIECE_RED_BISHOP;    // 象
    m_board[3][0] = PIECE_RED_ADVISOR;   // 士
    m_board[4][0] = PIECE_RED_KING;      // 帅
    m_board[5][0] = PIECE_RED_ADVISOR;   // 士
    m_board[6][0] = PIECE_RED_BISHOP;    // 象
    m_board[7][0] = PIECE_RED_KNIGHT;    // 马
    m_board[8][0] = PIECE_RED_ROOK;      // 车
    
    m_board[1][2] = PIECE_RED_CANNON;    // 炮
    m_board[7][2] = PIECE_RED_CANNON;    // 炮
    
    // 兵
    for (int i = 0; i < 9; i += 2) {
        m_board[i][3] = PIECE_RED_PAWN;
    }
    
    // 设置黑方棋子
    m_board[0][9] = PIECE_BLACK_ROOK;      // 车
    m_board[1][9] = PIECE_BLACK_KNIGHT;    // 马
    m_board[2][9] = PIECE_BLACK_BISHOP;    // 象
    m_board[3][9] = PIECE_BLACK_ADVISOR;   // 士
    m_board[4][9] = PIECE_BLACK_KING;      // 将
    m_board[5][9] = PIECE_BLACK_ADVISOR;   // 士
    m_board[6][9] = PIECE_BLACK_BISHOP;    // 象
    m_board[7][9] = PIECE_BLACK_KNIGHT;    // 马
    m_board[8][9] = PIECE_BLACK_ROOK;      // 车
    
    m_board[1][7] = PIECE_BLACK_CANNON;    // 炮
    m_board[7][7] = PIECE_BLACK_CANNON;    // 炮
    
    // 卒
    for (int i = 0; i < 9; i += 2) {
        m_board[i][6] = PIECE_BLACK_PAWN;
    }
    
    m_isRedTurn = true; // 红方先行
}

CChessAlgorithm::PIECE_TYPE CChessAlgorithm::GetPiece(int x, int y) const
{
    if (x < 0 || x >= BOARD_WIDTH || y < 0 || y >= BOARD_HEIGHT)
        return PIECE_NONE;
    
    return m_board[x][y];
}

bool CChessAlgorithm::MovePiece(int fromX, int fromY, int toX, int toY)
{
    // 检查坐标有效性
    if (fromX < 0 || fromX >= BOARD_WIDTH || fromY < 0 || fromY >= BOARD_HEIGHT ||
        toX < 0 || toX >= BOARD_WIDTH || toY < 0 || toY >= BOARD_HEIGHT)
        return false;
    
    // 获取棋子
    PIECE_TYPE piece = m_board[fromX][fromY];
    if (piece == PIECE_NONE)
        return false;
    
    // 检查是否是当前玩家的回合
    bool isRedPiece = (piece >= PIECE_RED_KING && piece <= PIECE_RED_PAWN);
    if (m_isRedTurn != isRedPiece)
        return false;
    
    // 检查移动是否合法
    if (!IsValidMove(fromX, fromY, toX, toY))
        return false;
    
    // 执行移动
    m_board[toX][toY] = piece;
    m_board[fromX][fromY] = PIECE_NONE;
    
    // 切换回合
    m_isRedTurn = !m_isRedTurn;
    
    return true;
}

bool CChessAlgorithm::IsValidMove(int fromX, int fromY, int toX, int toY) const
{
    // 基本检查
    if (fromX == toX && fromY == toY)
        return false;
    
    PIECE_TYPE piece = m_board[fromX][fromY];
    PIECE_TYPE target = m_board[toX][toY];
    
    // 不能吃自己的棋子
    bool isRedPiece = (piece >= PIECE_RED_KING && piece <= PIECE_RED_PAWN);
    bool isRedTarget = (target >= PIECE_RED_KING && target <= PIECE_RED_PAWN);
    bool isBlackTarget = (target >= PIECE_BLACK_KING && target <= PIECE_BLACK_PAWN);
    
    if ((isRedPiece && isRedTarget) || (!isRedPiece && isBlackTarget))
        return false;
    
    // 根据棋子类型检查移动规则
    switch (piece) {
        case PIECE_RED_KING:
        case PIECE_BLACK_KING: { // 帅/将
            // 只能在九宫格内移动
            if (!IsInPalace(toX, toY, piece == PIECE_RED_KING))
                return false;
            
            // 只能直走一格
            int dx = abs(toX - fromX);
            int dy = abs(toY - fromY);
            if (!((dx == 1 && dy == 0) || (dx == 0 && dy == 1)))
                return false;
            
            break;
        }
        
        case PIECE_RED_ADVISOR:
        case PIECE_BLACK_ADVISOR: { // 士
            // 只能在九宫格内移动
            if (!IsInPalace(toX, toY, piece == PIECE_RED_ADVISOR))
                return false;
            
            // 只能斜走一格
            int dx = abs(toX - fromX);
            int dy = abs(toY - fromY);
            if (!(dx == 1 && dy == 1))
                return false;
            
            break;
        }
        
        case PIECE_RED_BISHOP:
        case PIECE_BLACK_BISHOP: { // 象
            // 只能斜走两格
            int dx = abs(toX - fromX);
            int dy = abs(toY - fromY);
            if (!(dx == 2 && dy == 2))
                return false;
            
            // 象眼不能被堵住
            int eyeX = (fromX + toX) / 2;
            int eyeY = (fromY + toY) / 2;
            if (m_board[eyeX][eyeY] != PIECE_NONE)
                return false;
            
            // 红象不能过河
            if (piece == PIECE_RED_BISHOP && toY > 4)
                return false;
            
            // 黑象不能过河
            if (piece == PIECE_BLACK_BISHOP && toY < 5)
                return false;
            
            break;
        }
        
        // 其他棋子的移动规则可以继续在这里实现...
        
        default:
            return false;
    }
    
    return true;
}

bool CChessAlgorithm::IsInPalace(int x, int y, bool isRed) const
{
    // 检查是否在九宫格范围内
    if (x < 3 || x > 5)
        return false;
    
    if (isRed) {
        // 红方九宫格 (0-2行)
        return (y >= 0 && y <= 2);
    } else {
        // 黑方九宫格 (7-9行)
        return (y >= 7 && y <= 9);
    }
}

bool CChessAlgorithm::HasObstacleStraight(int fromX, int fromY, int toX, int toY) const
{
    // 检查直线路径上是否有棋子阻挡
    if (fromX == toX) {
        // 垂直方向
        int minY = min(fromY, toY);
        int maxY = max(fromY, toY);
        for (int y = minY + 1; y < maxY; y++) {
            if (m_board[fromX][y] != PIECE_NONE)
                return true;
        }
    } else if (fromY == toY) {
        // 水平方向
        int minX = min(fromX, toX);
        int maxX = max(fromX, toX);
        for (int x = minX + 1; x < maxX; x++) {
            if (m_board[x][fromY] != PIECE_NONE)
                return true;
        }
    }
    return false;
}

bool CChessAlgorithm::HasObstacleDiagonal(int fromX, int fromY, int toX, int toY) const
{
    // 检查对角线路径上是否有棋子阻挡
    int dx = (toX - fromX) > 0 ? 1 : -1;
    int dy = (toY - fromY) > 0 ? 1 : -1;
    
    int x = fromX + dx;
    int y = fromY + dy;
    
    while (x != toX && y != toY) {
        if (m_board[x][y] != PIECE_NONE)
            return true;
        x += dx;
        y += dy;
    }
    
    return false;
}

bool CChessAlgorithm::IsCheck(bool isRed) const
{
    // 检查是否将军的实现
    // 这里只是一个简单的占位实现
    return false;
}

bool CChessAlgorithm::IsCheckmate(bool isRed) const
{
    // 检查是否将死的实现
    // 这里只是一个简单的占位实现
    return false;
}