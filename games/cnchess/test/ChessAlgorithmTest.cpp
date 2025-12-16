#include <gtest/gtest.h>
#include "../algorithm/ChessAlgorithm.h"

// 测试棋盘初始化
TEST(ChessAlgorithmTest, BoardInitialization)
{
    CChessAlgorithm chess;
    
    // 检查初始状态下棋盘是否正确设置
    // 红方棋子检查
    EXPECT_EQ(CChessAlgorithm::PIECE_RED_ROOK, chess.GetPiece(0, 0));
    EXPECT_EQ(CChessAlgorithm::PIECE_RED_KNIGHT, chess.GetPiece(1, 0));
    EXPECT_EQ(CChessAlgorithm::PIECE_RED_BISHOP, chess.GetPiece(2, 0));
    EXPECT_EQ(CChessAlgorithm::PIECE_RED_ADVISOR, chess.GetPiece(3, 0));
    EXPECT_EQ(CChessAlgorithm::PIECE_RED_KING, chess.GetPiece(4, 0));
    EXPECT_EQ(CChessAlgorithm::PIECE_RED_ADVISOR, chess.GetPiece(5, 0));
    EXPECT_EQ(CChessAlgorithm::PIECE_RED_BISHOP, chess.GetPiece(6, 0));
    EXPECT_EQ(CChessAlgorithm::PIECE_RED_KNIGHT, chess.GetPiece(7, 0));
    EXPECT_EQ(CChessAlgorithm::PIECE_RED_ROOK, chess.GetPiece(8, 0));
    
    EXPECT_EQ(CChessAlgorithm::PIECE_RED_CANNON, chess.GetPiece(1, 2));
    EXPECT_EQ(CChessAlgorithm::PIECE_RED_CANNON, chess.GetPiece(7, 2));
    
    // 检查红方兵的位置
    for (int i = 0; i < 9; i += 2) {
        EXPECT_EQ(CChessAlgorithm::PIECE_RED_PAWN, chess.GetPiece(i, 3));
    }
    
    // 黑方棋子检查
    EXPECT_EQ(CChessAlgorithm::PIECE_BLACK_ROOK, chess.GetPiece(0, 9));
    EXPECT_EQ(CChessAlgorithm::PIECE_BLACK_KNIGHT, chess.GetPiece(1, 9));
    EXPECT_EQ(CChessAlgorithm::PIECE_BLACK_BISHOP, chess.GetPiece(2, 9));
    EXPECT_EQ(CChessAlgorithm::PIECE_BLACK_ADVISOR, chess.GetPiece(3, 9));
    EXPECT_EQ(CChessAlgorithm::PIECE_BLACK_KING, chess.GetPiece(4, 9));
    EXPECT_EQ(CChessAlgorithm::PIECE_BLACK_ADVISOR, chess.GetPiece(5, 9));
    EXPECT_EQ(CChessAlgorithm::PIECE_BLACK_BISHOP, chess.GetPiece(6, 9));
    EXPECT_EQ(CChessAlgorithm::PIECE_BLACK_KNIGHT, chess.GetPiece(7, 9));
    EXPECT_EQ(CChessAlgorithm::PIECE_BLACK_ROOK, chess.GetPiece(8, 9));
    
    EXPECT_EQ(CChessAlgorithm::PIECE_BLACK_CANNON, chess.GetPiece(1, 7));
    EXPECT_EQ(CChessAlgorithm::PIECE_BLACK_CANNON, chess.GetPiece(7, 7));
    
    // 检查黑方卒的位置
    for (int i = 0; i < 9; i += 2) {
        EXPECT_EQ(CChessAlgorithm::PIECE_BLACK_PAWN, chess.GetPiece(i, 6));
    }
    
    // 检查空位
    EXPECT_EQ(CChessAlgorithm::PIECE_NONE, chess.GetPiece(0, 1));
    EXPECT_EQ(CChessAlgorithm::PIECE_NONE, chess.GetPiece(4, 1));
    EXPECT_EQ(CChessAlgorithm::PIECE_NONE, chess.GetPiece(8, 1));
}

// 测试帅/将的移动规则
TEST(ChessAlgorithmTest, KingMovement)
{
    CChessAlgorithm chess;
    
    // 测试红帅在九宫格内的移动
    // 红帅只能在九宫格内移动，每次只能移动一格
    EXPECT_TRUE(chess.MovePiece(4, 0, 3, 0)); // 横向移动
    EXPECT_TRUE(chess.MovePiece(3, 0, 3, 1)); // 纵向移动
    EXPECT_TRUE(chess.MovePiece(3, 1, 4, 1)); // 横向移动
    
    // 测试试图移出九宫格的情况
    EXPECT_FALSE(chess.MovePiece(4, 1, 2, 1)); // 试图横向移动两格（超出九宫格范围）
    EXPECT_FALSE(chess.MovePiece(4, 1, 4, 3)); // 试图纵向移动两格（超出九宫格范围）
    
    // 测试黑将在九宫格内的移动
    // 注意：这里需要模拟黑方回合
    // 由于简单起见，我们直接测试移动验证函数
    
    // 测试黑将的移动
    // 首先移动几步让黑方获得回合权（模拟）
    chess.MovePiece(4, 1, 4, 0); // 红方移动回去
    chess.MovePiece(4, 9, 3, 9); // 黑将横向移动
    chess.MovePiece(4, 0, 4, 1); // 红帅横向移动
    chess.MovePiece(3, 9, 3, 8); // 黑将纵向移动
}

// 测试士/仕的移动规则
TEST(ChessAlgorithmTest, AdvisorMovement)
{
    CChessAlgorithm chess;
    
    // 士只能在九宫格内斜向移动一格
    EXPECT_TRUE(chess.MovePiece(3, 0, 4, 1)); // 红仕斜向移动
    EXPECT_TRUE(chess.MovePiece(4, 1, 3, 0)); // 移动回去
    
    // 测试试图直走的情况（非法）
    EXPECT_FALSE(chess.MovePiece(3, 0, 3, 1)); // 直走是非法的
    
    // 测试试图移出九宫格的情况
    EXPECT_FALSE(chess.MovePiece(3, 0, 2, 1)); // 移动到九宫格外
}

// 测试象/相的移动规则
TEST(ChessAlgorithmTest, BishopMovement)
{
    CChessAlgorithm chess;
    
    // 相/象走"田"字，且不能过河
    // 红象初始位置在(2,0)和(6,0)
    
    // 测试红象的合法移动
    EXPECT_TRUE(chess.MovePiece(2, 0, 4, 2)); // 正常走田字
    
    // 移动回去
    chess.MovePiece(4, 2, 2, 0);
    
    // 测试试图过河的移动（非法）
    EXPECT_FALSE(chess.MovePiece(2, 0, 0, 2)); // 这种移动会过河，非法
    
    // 测试被阻挡的情况（象眼被堵）
    // 在(3,1)放置一个棋子阻挡象眼
    // 由于我们不能直接修改棋盘，这种测试需要更复杂的设置
}

// 测试车的移动规则
TEST(ChessAlgorithmTest, RookMovement)
{
    CChessAlgorithm chess;
    
    // 车可以横竖直线移动任意距离，直到遇到阻挡
    // 先清除路径上的棋子
    
    // 移动红方兵以便车可以移动
    chess.MovePiece(0, 3, 0, 4); // 移动兵
    
    // 测试车的横向移动
    EXPECT_TRUE(chess.MovePiece(0, 0, 1, 0)); // 车横向移动（吃掉马）
    
    // 移动回去
    chess.MovePiece(1, 0, 0, 0);
    
    // 测试车的纵向移动
    EXPECT_TRUE(chess.MovePiece(0, 0, 0, 1)); // 车纵向移动
}

// 测试马的移动规则
TEST(ChessAlgorithmTest, KnightMovement)
{
    CChessAlgorithm chess;
    
    // 马走"日"字，先直一格再斜一格
    // 需要先移动兵才能让马有路可走
    
    // 移动红方兵
    chess.MovePiece(1, 3, 1, 4);
    
    // 测试马的合法移动
    EXPECT_TRUE(chess.MovePiece(1, 0, 2, 2)); // 马走日字
    
    // 移动回去
    chess.MovePiece(2, 2, 1, 0);
    
    // 测试马腿被蹩的情况（非法）
    // 这种测试需要更复杂的设置
}

// 测试炮的移动规则
TEST(ChessAlgorithmTest, CannonMovement)
{
    CChessAlgorithm chess;
    
    // 炮移动时和车一样，横竖直线移动
    // 吃子时需要隔着一个棋子（炮架）
    
    // 测试炮的移动
    EXPECT_TRUE(chess.MovePiece(1, 2, 1, 1)); // 炮纵向移动
    
    // 移动回去
    chess.MovePiece(1, 1, 1, 2);
    
    // 测试炮的吃子需要炮架
    // 这种测试需要更复杂的设置
}

// 测试兵/卒的移动规则
TEST(ChessAlgorithmTest, PawnMovement)
{
    CChessAlgorithm chess;
    
    // 兵/卒只能向前走一格，过河后可以横向移动
    // 测试红兵向前移动
    EXPECT_TRUE(chess.MovePiece(0, 3, 0, 4)); // 红兵向前移动
    
    // 移动回去
    chess.MovePiece(0, 4, 0, 3);
    
    // 测试试图后退（非法）
    EXPECT_FALSE(chess.MovePiece(0, 3, 0, 2)); // 红兵试图后退
    
    // 测试试图横向移动（未过河时非法）
    EXPECT_FALSE(chess.MovePiece(0, 3, 1, 3)); // 红兵试图横向移动
}

// 测试将军检测
TEST(ChessAlgorithmTest, CheckDetection)
{
    CChessAlgorithm chess;
    
    // 简单测试将军检测功能
    // 这个测试需要实现具体的将军检测逻辑
    EXPECT_FALSE(chess.IsCheck(true));  // 红方未被将军
    EXPECT_FALSE(chess.IsCheck(false)); // 黑方未被将军
}

// 测试将死检测
TEST(ChessAlgorithmTest, CheckmateDetection)
{
    CChessAlgorithm chess;
    
    // 简单测试将死检测功能
    // 这个测试需要实现具体的将死检测逻辑
    EXPECT_FALSE(chess.IsCheckmate(true));  // 红方未被将死
    EXPECT_FALSE(chess.IsCheckmate(false)); // 黑方未被将死
}

// 测试回合交替
TEST(ChessAlgorithmTest, TurnAlternation)
{
    CChessAlgorithm chess;
    
    // 测试回合交替机制
    EXPECT_TRUE(chess.MovePiece(4, 0, 4, 1)); // 红方移动
    
    // 尝试再次移动红方棋子（应该失败）
    EXPECT_FALSE(chess.MovePiece(4, 1, 4, 2)); // 红方不能连续移动两次
    
    // 黑方移动
    chess.MovePiece(4, 9, 4, 8); // 黑方移动
    
    // 再次轮到红方
    EXPECT_TRUE(chess.MovePiece(4, 1, 4, 0)); // 红方移动
}