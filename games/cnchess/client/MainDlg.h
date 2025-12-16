#pragma once

#include <helper/SDpiHelper.hpp>
#include "ChessGame.h"

class CMainDlg : public SHostWnd
               , public SDpiHandler<CMainDlg>
{
public:
    CMainDlg();
    ~CMainDlg();

    void OnClose();
    void OnMaximize();
    void OnRestore();
    void OnMinimize();
    void OnSize(UINT nType, CSize size);
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

protected:
    void OnScaleChanged(int nScale) override;
    
protected:
    //soui消息
    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_close", OnClose)
        EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
        EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
        EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
        CHAIN_EVENT_MAP_MEMBER(*m_pChessGame)
    EVENT_MAP_END2(SHostWnd)
        
    //HostWnd真实窗口消息处理
    BEGIN_MSG_MAP_EX(CMainDlg)
        CHAIN_MSG_MAP(SDpiHandler<CMainDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        CHAIN_MSG_MAP_MEMBER(*m_pChessGame)
        CHAIN_MSG_MAP(SHostWnd)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

private:
    ChessGame* m_pChessGame;  // 象棋游戏核心逻辑
};