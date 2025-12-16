#pragma once

#include <sobject/Sobject.hpp>
#include <helper/obj-ref-impl.hpp>

// 前向声明
class CMainDlg;

/**
 * @brief 象棋游戏核心逻辑类
 * 
 * 该类负责处理象棋游戏的核心逻辑，包括：
 * 1. 游戏状态管理
 * 2. 玩家交互处理
 * 3. 游戏数据管理
 * 4. 与UI的交互
 */
class ChessGame : public TObjRefImpl<SObject>
{
    DEF_SOBJECT(SObject, L"chessgame")     // 定义SObject的类名和类别名
    
public:
    /**
     * @brief 构造函数
     * @param pMainDlg 主窗口指针
     */
    ChessGame(CMainDlg* pMainDlg);
    
    /**
     * @brief 析构函数
     */
    ~ChessGame();
    
    /**
     * @brief 初始化游戏
     * @param pGameBoard 游戏面板窗口指针
     */
    void Init(SWindow *pGameBoard);

protected:
    CMainDlg* m_pMainDlg;  // 主窗口指针
    SWindow* m_pGameBoard; // 游戏面板
};