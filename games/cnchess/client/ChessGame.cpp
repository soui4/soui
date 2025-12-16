#include "stdafx.h"
#include "ChessGame.h"
#include "../algorithm/ChessAlgorithm.h"

ChessGame::ChessGame(CMainDlg* pMainDlg)
    : m_pMainDlg(pMainDlg)
    , m_pGameBoard(NULL)
{
}

ChessGame::~ChessGame()
{
}

void ChessGame::Init(SWindow *pGameBoard)
{
    m_pGameBoard = pGameBoard;
    
    // 创建棋盘网格布局
    SWindow* pChessBoard = new SWindow();
    pChessBoard->SetAttribute(L"layout", L"grid");
    pChessBoard->SetAttribute(L"rows", L"10");
    pChessBoard->SetAttribute(L"cols", L"9");
    pChessBoard->SetAttribute(L"width", L"540");
    pChessBoard->SetAttribute(L"height", L"600");
    pChessBoard->SetAttribute(L"hcenter", L"1");
    pChessBoard->SetAttribute(L"vcenter", L"1");
    
    // 创建棋盘格子
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            SWindow* pCell = new SWindow();
            pCell->SetAttribute(L"width", L"60");
            pCell->SetAttribute(L"height", L"60");
            
            // 设置棋盘背景
            if ((row + col) % 2 == 0) {
                pCell->SetAttribute(L"colorBkgnd", L"#F5DEB3"); // 米色
            } else {
                pCell->SetAttribute(L"colorBkgnd", L"#DEB887"); // 棕色
            }
            
            // 添加点击事件
            pCell->SetAttribute(L"name", SStringT().Format(_T("cell_%d_%d"), col, row));
            
            pChessBoard->InsertChild(pCell);
        }
    }
    
    pGameBoard->InsertChild(pChessBoard);
    
    // TODO: 添加棋子到棋盘上
}